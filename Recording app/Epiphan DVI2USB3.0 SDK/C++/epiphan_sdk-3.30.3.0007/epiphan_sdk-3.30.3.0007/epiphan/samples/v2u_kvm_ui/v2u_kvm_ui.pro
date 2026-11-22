# -------------------------------------------------
# $Id: v2u_kvm_ui.pro 24919 2013-12-09 22:21:32Z pzeldin $
# -------------------------------------------------
CONFIG += qt thread debug_and_release
TARGET = v2u_kvm_ui
TEMPLATE = app
SOURCES += \
    main.cpp \
    framegrabber.cpp \
    grabmainwindow.cpp \
    grabthread.cpp \
    grabwidget.cpp \
    grabframe.cpp \
    grabevent.cpp \
    graboptions.cpp \
    ps2eventbuf.cpp \
    edid.cpp
HEADERS += \
    framegrabber.h \
    debugtrace.h \
    edid.h \
    grabmainwindow.h \
    grabthread.h \
    grabwidget.h \
    grabframe.h \
    grabevent.h \
    graboptions.h \
    ps2eventbuf.h \
    objectref.h
RESOURCES += v2u_kvm_ui.qrc
INCLUDEPATH += ../v2u_lib/include ../../frmgrab/include ../../include

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

unix {
    CPU = $$system(sh v2u_kvm_ui.arch)
    SRCDIR = $$system(sh v2u_kvm_ui.path)
    LIBS += -L$$SRCDIR/../../frmgrab/lib/linux/$$CPU -lfrmgrab -L$$SRCDIR/../../../opensource/slib -lslava -lz
    #LIBS += -L$$SRCDIR/../../frmgrab/build/debug -lfrmgrabd -L$$SRCDIR/../../../opensource/slib -lslavad -lz
}

win32:LIBS += -L../../frmgrab/lib/win/x86 -lfrmgrab

# You need to link against Carbon network on Mac OS X
macx:LIBS += -framework Carbon

OTHER_FILES += 
