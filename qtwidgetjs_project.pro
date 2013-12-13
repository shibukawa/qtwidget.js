#-------------------------------------------------
#
# Project created by QtCreator 2012-12-09T21:58:31
#
#-------------------------------------------------

QT       += core gui widgets script scripttools

TARGET = qtwidgetjs
TEMPLATE = app
SOURCES +=  cpp_src/main.cpp \
            cpp_src/jsxloader.cpp

HEADERS +=  cpp_src/jsxloader.h

macx {
    CONFIG += app_bundle
    QMAKE_POST_LINK += $$quote(cp -R $${PWD}/js_src $${OUT_PWD}/$${TARGET}.app/Contents/Resources/);
    QMAKE_POST_LINK += $$quote(cp -R $${PWD}/plugins $${OUT_PWD}/$${TARGET}.app/Contents/Resources/);
}

OTHER_FILES += \
    js_src/main.js
