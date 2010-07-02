load(qttest_p4)
SOURCES  += tst_qsparql.cpp

QT = core sparql


!wince*:win32:LIBS += -lws2_32


wince*: {
   plugFiles.sources = ../../../plugins/sparqldrivers
   plugFiles.path    = .
   DEPLOYMENT += plugFiles
   LIBS += -lws2
}

