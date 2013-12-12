
QString adjustPath(const QString &path)
{
    #ifdef Q_OS_UNIX
    #ifdef Q_OS_MAC
    if (!QDir::isAbsolutePath(path))
        return QCoreApplication::applicationDirPath()
               + QLatin1String("/../Resources/") + path;
    #else
    const QString pathInShareDir = QCoreApplication::applicationDirPath()
          + QLatin1String("/../share/")
          + QFileInfo(QCoreApplication::applicationFilePath()).fileName()
          + QLatin1Char('/') + path;
    if (QFileInfo(pathInShareDir).exists())
        return pathInShareDir;
    #endif
    #endif
    return path;
}



static QScriptValue consoleLogForJS(QScriptContext* context, QScriptEngine* engine)
{
    QStringList list;
    for(int i=0; i<context->argumentCount(); ++i)
    {
        QScriptValue param(context->argument(i));
        list.append(param.toString());
    }
    qDebug() << list.join(" ");
    return engine->undefinedValue();
}

static QScriptValue requireForJS(QScriptContext* context, QScriptEngine* engine)
{
    QString requiredPath;
    bool found = false;
    QString param = context->argument(0).toString();
    param.push_back(".js");
    QStringList paths = engine->globalObject().property("require").property("paths").toVariant().toStringList();

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
    else
        QFile requireFile(requiredPath);
        requireFile.open(QIODevice::ReadOnly);

        QScriptContext* newContext = engine->pushContext();
        QScriptValue exportsObject = engine->newObject();
        newContext->activationObject().setProperty("exports", exportsObject);
        engine->evaluate(requireFile.readAll(), requiredPath);
        exportsObject = newContext->activationObject().property("exports");
        engine->popContext();
        requireFile.close();
        modules.setProperty(requiredPath, exportsObject);
        return exportsObject;
    }
}

void init_qtscript(QScriptEngine& engine)
{
    QScriptValue globalObject = engine.globalObject();
    QScriptValue console = engine.newObject();
    globalObject.setProperty("console", console);
    QScriptValue consoleLog = engine.newFunction(consoleLogForJS);
    console.setProperty("log", consoleLog);

    QScriptValue modules = engine.newObject();
    globalObject.setProperty("$MODULES", modules);

    ScriptValue require = engine.newFunction(requireForJS);
    gobalObject.setProperty("require", require);
    ScriptValue paths = engine->newArray();
    QScriptValue push = paths.property("push");
    push.call(paths, QScriptValueList() << QScriptValue("/search/path/you/want/to/add"));
};

void registerQtComponents()
{
    QDir pluginDir(QApplication::applicationDirPath());
    #ifdef Q_OS_MAC
    pluginDir.cdUp();
    pluginDir.cd("Resources");
    #endif
    if (!pluginDir.cd("plugins")) {
        fprintf(stderr, "plugins folder does not exist -- did you build the bindings?\n");
        eturn(-1);
    }
    QStringList paths = app.libraryPaths();
    paths << pluginDir.absolutePath();
    app.setLibraryPaths(paths);

    QStringList extensions;
    extensions << "qt.core"
               << "qt.gui"
               << "qt.xml"
               << "qt.svg"
               << "qt.network"
               << "qt.sql"
               << "qt.opengl"
               << "qt.webkit"
               << "qt.xmlpatterns"
               << "qt.uitools";
}

foreach (const QString &ext, extensions) {
    QScriptValue ret = engine.importExtension(ext);
    if (ret.isError())
    {
        qDebug() << "Error occured to load extionsion: " << ret.toVariant() << ext;
    }
