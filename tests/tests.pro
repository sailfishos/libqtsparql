TEMPLATE = subdirs
SUBDIRS = auto

check.CONFIG = recursive
check.recurse = auto

QMAKE_EXTRA_TARGETS += check
