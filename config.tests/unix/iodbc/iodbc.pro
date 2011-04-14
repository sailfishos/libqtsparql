SOURCES = iodbc.cpp
CONFIG -= qt dylib
mac:CONFIG -= app_bundle
LIBS += -liodbc
INCLUDEPATH += /usr/include/libiodbc
