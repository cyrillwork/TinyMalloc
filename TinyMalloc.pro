
TARGET = TinyMalloc
TEMPLATE = lib

SOURCES = \
    src/malloc.c

INCLUDEPATH += include

#DEFINES += MEMORY_CHECK

HEADERS = \
    include/malloc.h

DESTDIR=./build

LIBS += -lpthread

