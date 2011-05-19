TEMPLATE = subdirs
SUBDIRS = auto

check.CONFIG = recursive
check.recurse = auto

memcheck.CONFIG = recursive
memcheck.recurse = auto

QMAKE_EXTRA_TARGETS += check memcheck
