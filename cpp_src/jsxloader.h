#ifndef JSXLOADER_H
#define JSXLOADER_H

#include <QtCore>
#include <QtScript/QScriptEngine>

class JSXLoader : public QObject
{
    Q_OBJECT

    int removeparam_;
    bool debugger_;
public:
    explicit JSXLoader(QObject *parent = 0, int removeparam = 1, bool debugger = false);

private:
    bool resolveDir(QDir& pluginDir, QDir& scriptDir) const;
    void loadExtensions(QDir& pluginDir);
    void addConsole();
    void addRequire(QDir& scriptDir);
    void addProcessModule(QString& scriptPath);

    QScriptEngine* engine;
    
signals:
    void finished();

public slots:
    void runScript();
};

#endif // JSXLOADER_H
