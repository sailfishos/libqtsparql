#####################################################################
# Main projectfile
#####################################################################

include(shared.pri)
CONFIG += ordered
TEMPLATE = subdirs

SUBDIRS = src tests examples

xclean.commands = rm -rf lib plugins include
xclean.depends = clean

QMAKE_EXTRA_TARGETS += xclean

include(doc/doc.pri)

