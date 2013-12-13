#include <QtCore>
#include <QApplication>
#include "jsxloader.h"
#include <iostream>

int main(int argc, char *argv[])
{
    bool gui = true;
    bool debugger = false;
    int removeparam = 1;
    for (int i = 1; i < argc; i++)
    {
        QString arg = argv[i];
        if (arg == "--console")
        {
            gui = false;
            removeparam++;
        }
        else if (arg == "--debugger")
        {
            debugger = true;
            std::cout << "Debugger" << std::endl;
            removeparam++;
        }
        else
        {
            break;
        }
    }
    QApplication* app;
    app = new QApplication(argc, argv);
    JSXLoader *loader = new JSXLoader(app, removeparam, debugger);
    QObject::connect(loader, SIGNAL(finished()), app, SLOT(quit()));
    QTimer::singleShot(0, loader, SLOT(runScript()));
    int result = app->exec();
    delete app;
    return result;
}
