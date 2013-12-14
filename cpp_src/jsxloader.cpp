#include <iostream>

#include <QtScript/QScriptValue>
#include <QtScript/QScriptContextInfo>
#ifndef QT_NO_SCRIPTTOOLS
#include <QtScriptTools/QScriptEngineDebugger>
#endif

#include "jsxloader.h"

static QScriptValue consoleLogForJS(QScriptContext* context, QScriptEngine* engine)
{
    QStringList list;
    for(int i=0; i<context->argumentCount(); ++i)
    {
        QScriptValue param(context->argument(i));
        list.append(param.toString());
    }
    std::cout << list.join(" ").toStdString().c_str() << std::endl;
    return engine->undefinedValue();
}

static QScriptValue consoleErrorForJS(QScriptContext* context, QScriptEngine* engine)
{
    QStringList list;
    for(int i=0; i<context->argumentCount(); ++i)
    {
        QScriptValue param(context->argument(i));
        list.append(param.toString());
    }
    std::cerr << list.join(" ").toStdString().c_str() << std::endl;
    return engine->undefinedValue();
}

static QScriptValue requireForJS(QScriptContext* context, QScriptEngine* engine)
{
    QString requiredPath;
    QString requiredDir;

    bool found = false;
    QString param = context->argument(0).toString();
    param.push_back(".js");
    QScriptValue global = engine->globalObject();
    QStringList paths = global.property("require").property("paths").toVariant().toStringList();

    QFileInfo requiredFileInfo(param);
    QScriptContextInfo contextInfo(context->parentContext());
    QString parentFileName(contextInfo.fileName());
    paths.push_front(QFileInfo(parentFileName).dir().path());

    foreach(QString includePath, paths)
    {
        qDebug() << includePath;
        QFileInfo includePathInfo(includePath);
        if (includePathInfo.exists())
        {
            QDir includePathDir(includePath);
            includePathDir.cd(requiredFileInfo.dir().path());
            requiredPath = QDir::cleanPath(includePathDir.absoluteFilePath(requiredFileInfo.fileName()));

            if(QFileInfo(requiredPath).exists())
            {
                qDebug() << requiredPath;
                found = true;
                requiredDir = includePathDir.path();
                break;
            }
        }
    }
    if (!found)
    {
        qDebug() << "require() file not found: " << param;
        return engine->undefinedValue();
    }
    QScriptValue modules = engine->globalObject().property("$MODULES");
    QScriptValue existedModule = modules.property(requiredPath);
    if (existedModule.isValid())
    {
        return existedModule;
    }
    QFile requireFile(requiredPath);
    requireFile.open(QIODevice::ReadOnly);

    QScriptContext* newContext = engine->pushContext();
    QScriptValue exportsObject = engine->newObject();
    newContext->activationObject().setProperty("exports", exportsObject);

    QScriptValue originalDir = global.property("__DIR__");
    QScriptValue originalPath = global.property("__JS_FILE__");

    QScriptValue dirpathobj = requiredDir;
    QScriptValue scriptpathobj = requiredPath;
    global.setProperty("__DIR__", dirpathobj);
    global.setProperty("__FILE__", scriptpathobj);

    engine->evaluate(requireFile.readAll(), requiredPath);

    global.setProperty("__DIR__", originalDir);
    global.setProperty("__FILE__", originalPath);

    exportsObject = newContext->activationObject().property("exports");
    engine->popContext();
    requireFile.close();
    modules.setProperty(requiredPath, exportsObject);
    return exportsObject;
}


JSXLoader::JSXLoader(QObject *parent, int removeparam, bool debugger) :
    QObject(parent), removeparam_(removeparam), debugger_(debugger)
{
}

void JSXLoader::runScript()
{
    QDir pluginDir(QCoreApplication::applicationDirPath());
    QDir scriptDir(QCoreApplication::applicationDirPath());

    std::cout << QCoreApplication::applicationDirPath().toStdString() << std::endl;

    if (resolveDir(pluginDir, scriptDir))
    {
        this->engine = new QScriptEngine();

        QString scriptPath = scriptDir.filePath("main.js");

        if (!scriptDir.exists("main.js"))
        {
            std::cerr << "Script file " << scriptPath.toStdString().c_str() << " is not found." << std::endl;
            emit finished();
            return;
        }


#ifndef QT_NO_SCRIPTTOOLS
        if (debugger_)
        {
            QScriptEngineDebugger *dbg = new QScriptEngineDebugger();
            dbg->attachTo(engine);
        }
#endif
        loadExtensions(pluginDir);
        addConsole();
        addRequire(scriptDir);
        addProcessModule(scriptPath);

        // Load Script and Run
        QScriptValue globalObject = engine->globalObject();
        QScriptValue qtGlobalObject = engine->newObject();

        QScriptValue dirpathobj = scriptDir.path();
        QScriptValue scriptpathobj = scriptPath;
        QScriptValue qappobj = engine->newQObject(qApp);

        QScriptValue argv = engine->newArray();
        argv.setProperty(0, QScriptValue(qApp->arguments().at(0)));
        argv.setProperty(1, QScriptValue(scriptPath));
        for (int i = 1; i < qApp->arguments().size(); i++) {
            argv.setProperty(i + 1, QScriptValue(qApp->arguments().at(i)));
        }
        qtGlobalObject.setProperty("dir", dirpathobj);
        qtGlobalObject.setProperty("jsfile", scriptpathobj);
        qtGlobalObject.setProperty("qApp", qappobj);
        qtGlobalObject.setProperty("argv", argv);

        globalObject.setProperty("qtGlobal", qtGlobalObject);
        QScriptValue process = engine->newObject();
        process.setProperty("argv", argv);
        globalObject.setProperty("process", process);

        QFile scriptFile(scriptPath);
        scriptFile.open(QIODevice::ReadOnly);
        QTextStream stream(&scriptFile);
        QString firstLine = stream.readLine();
        QString contents = stream.readAll();
        if (!firstLine.startsWith("#"))
        {
            contents = firstLine + "\n" + contents;
        }
        scriptFile.close();
        QScriptValue r = engine->evaluate(contents, scriptPath);
        if (engine->hasUncaughtException()) {
            QStringList backtrace = engine->uncaughtExceptionBacktrace();
            std::cerr << "    " << qPrintable(r.toString()) << std::endl
                      << qPrintable(backtrace.join("\n")) << std::endl << std::endl;
        }
    }
    emit finished();
}

void JSXLoader::loadExtensions(QDir &pluginDir)
{
    QCoreApplication* app = dynamic_cast<QCoreApplication*>(parent());
    std::cout << "library path: " << pluginDir.absolutePath().toStdString() << std::endl;
    app->addLibraryPath(pluginDir.absolutePath());
    QDir scriptDir(pluginDir.filePath("script"));
    QStringList files = scriptDir.entryList(QDir::Files | QDir::NoSymLinks);
    foreach (const QString &file, files)
    {
        if (file.startsWith("libqtscript_"))
        {
            QString modulename = file.left(file.indexOf('.'));
            modulename.remove(0, 12);
            QScriptValue ret = engine->importExtension(QString("qt.") + modulename);
            if (ret.isError())
            {
                qDebug() << "Error occured to load extionsion: " << ret.toVariant() << modulename;
            }
        }
    }
}

void JSXLoader::addConsole()
{
    QScriptValue globalObject = engine->globalObject();

    QScriptValue console = engine->newObject();
    globalObject.setProperty("console", console);
    QScriptValue consoleLog = engine->newFunction(consoleLogForJS);
    console.setProperty("log", consoleLog);
    console.setProperty("info", consoleLog);
    QScriptValue consoleError = engine->newFunction(consoleErrorForJS);
    console.setProperty("error", consoleError);
    console.setProperty("warn", consoleError);
}

void JSXLoader::addRequire(QDir& scriptDir)
{
    QScriptValue globalObject = engine->globalObject();

    QScriptValue modules = engine->newObject();
    globalObject.setProperty("$MODULES", modules);

    QScriptValue require = engine->newFunction(requireForJS);
    globalObject.setProperty("require", require);
    QScriptValue paths = engine->newArray();
    QScriptValue requirepush = paths.property("push");
    requirepush.call(paths, QScriptValueList() << QScriptValue(scriptDir.path()));
    require.setProperty("paths", paths);
}

void JSXLoader::addProcessModule(QString& scriptPath)
{
    QCoreApplication* app = dynamic_cast<QCoreApplication*>(parent());
    QScriptValue globalObject = engine->globalObject();

    QScriptValue process = engine->newObject();
    globalObject.setProperty("process", process);

    QScriptValue argvobj = engine->newArray();
    QScriptValue argvpush = argvobj.property("push");
    QStringList rawargv = app->arguments();
    QScriptValueList argv;
    argv << QScriptValue(rawargv.at(0));
    argv << QScriptValue(scriptPath);
    for (int i = removeparam_; i < rawargv.size(); i++)
    {
        argv << QScriptValue(rawargv.at(i));
    }
    argvpush.call(argvobj, argv);
    process.setProperty("argv", argvobj);
}

bool JSXLoader::resolveDir(QDir &pluginDir, QDir &scriptDir) const
{
#ifdef Q_OS_MACX
    if (pluginDir.dirName() == QLatin1String("MacOS"))
    {
        pluginDir.cdUp();
        scriptDir.cdUp();
        if (!pluginDir.cd("Resources"))
        {
            qDebug() << "Resources folder doesn't exists.";
            return false;
        }
        scriptDir.cd("Resources");
    }
    if (!pluginDir.cd("plugins"))
    {
        qDebug() << "Resources/plugins folder doesn't not exists.";
        return false;
    }
    if (!scriptDir.cd("js_src"))
    {
        qDebug() << "Resources/js_src folder doesn't not exists.";
    }
#elif Q_OS_UNIX
    if (pluginDir.dirName() == QLatin1String("bin"))
    {
        pluginDir.cdUp();
        scriptDir.cdUp();
    }
    if (pluginDir.exists("shared"))
    {
        pluginDir.cd("shared");
        scriptDir.cd("shared");
        QString appliName = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
        if (plugin.cd(appliName))
        {
            scriptDir.cd(appliName);
        }
    }
    if (!pluginDir.cd("plugins"))
    {
        qDebug() << "shared/plugins folder doesn't not exists.";
        return false;
    }
    if (!scriptDir.cd("js_src"))
    {
        qDebug() << "shared/js_src folder doesn't not exists.";
    }
#else // Windows
    if (pluginDir.dirName() == QLatin1String("debug") || pluginDir.dirName() == QLatin1String("release"))
    {
        pluginDir.cdUp();
        scriptDir.cdUp();
    }
    if (!pluginDir.cd("plugins"))
    {
        qDebug() << "plugins folder doesn't not exists.";
        return false;
    }
    if (!scriptDir.cd("js_src"))
    {
        qDebug() << "js_src folder doesn't not exists.";
    }
#endif

    return true;
}
