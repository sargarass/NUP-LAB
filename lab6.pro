TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    mutex.c \
    timer.c \
    mythread.c \
    veb.c \
    bin_tree.c \
    log2.c \
    bin_tree_allocator.c \
    seqlock.c \
    bin_tree_debugtools.c \
    rwlock.c
QMAKE_CFLAGS += -std=gnu11 -pthread -masm=intel -O3
QMAKE_LFLAGS += -lpthread

HEADERS += \
    mutex.h \
    timer.h \
    config.h \
    mythread.h \
    bin_tree.h \
    veb.h \
    include.h \
    bin_tree_allocator.h \
    seqlock.h \
    rwlock.h \
    ministl.h

OTHER_FILES += \
    also.txt
