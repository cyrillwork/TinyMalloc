
TEMPLATE = app
TARGET = 
DEPENDPATH += .

INCLUDEPATH += ../../

CONFIG += c++14
QMAKE_CXXFLAGS += -std=c++14

LIBS += ../../build/libTinyMalloc.so -Wl,-Bdynamic
DEFINES += USE_TINYMALLOC

# Input
SOURCES += main.cpp \
    profiler.cpp

HEADERS += \
    profiler.h

