include(../sparql-examples.pri)
TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
#QT += sparql # enable this later
QT += gui

# Input
FORMS += contact.ui list.ui add.ui main.ui
SOURCES += main.cpp
HEADERS += main.cpp
