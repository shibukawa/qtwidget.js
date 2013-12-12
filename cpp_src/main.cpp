#include <QtGui/QApplication>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>
#ifndef QT_NO_SCRIPTTOOLS
#include <QtScriptTools/QScriptEngineDebugger>
#endif
#include "qtscript_support.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QScriptEngineDebugger* debugger = new QScriptEngineDebugger();
    debugger->attachTo(&engine);
    debugger->action(QScriptEngineDebugger::InterruptAction)->trigger();
    return a.exec();
}
