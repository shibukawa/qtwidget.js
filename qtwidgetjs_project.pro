#-------------------------------------------------
#
# Project created by QtCreator 2012-12-09T21:58:31
#
#-------------------------------------------------

QT       += core gui

TARGET = qtwidgetjs_project
TEMPLATE = app

folder_01.source = js_src
folder_01.target = .
folder_02.source = plugins
folder_02.target = .
DEPLOYMENTFOLDERS = folder_01 folder_02


SOURCES +=  cpp_src/main.cpp \
            cpp_src/qtscript_support.cpp

HEADERS +=  cpp_src/qtscript_support.h

