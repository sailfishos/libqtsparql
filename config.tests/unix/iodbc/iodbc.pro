SOURCES = iodbc.cpp
CONFIG -= qt dylib
CONFIG += link_pkgconfig
PKGCONFIG += libiodbc
mac:CONFIG -= app_bundle
