
TARGET = TinyMalloc
TEMPLATE = lib

SOURCES = \
    src/malloc.c

INCLUDEPATH += include


HEADERS = \
    include/malloc.h

DESTDIR=./build

LIBS += -lpthread

