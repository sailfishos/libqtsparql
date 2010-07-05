#####################################################################
# Main projectfile
#####################################################################

include(shared.pri)
CONFIG += ordered
TEMPLATE = subdirs

SUBDIRS = src examples

xclean.commands = rm -rf lib plugins
xclean.depends = clean

QMAKE_EXTRA_TARGETS += xclean

