# -------------------------------------------------
# Project created by QtCreator 2009-05-21T12:40:28
# -------------------------------------------------
TARGET = KrokCom
TEMPLATE = app
SOURCES += main.cxx \
    KrokComWindow.cxx \
    Cart.cxx \
    CartDetector.cxx \
    SerialPortManager.cxx \
    MD5.cxx
HEADERS += KrokComWindow.hxx \
    bspf.hxx \
    BSType.hxx \
    Cart.hxx \
    CartDetector.hxx \
    SerialPortManager.hxx \
    SerialPort.hxx \
    FindKrokThread.hxx \
    Version.hxx \
    MultiCart.hxx \
    MD5.hxx
FORMS += krokcomwindow.ui
RESOURCES += resources.qrc
unix:!macx { 
    SOURCES += SerialPortUNIX.cxx
    HEADERS += SerialPortUNIX.hxx
    TARGET = krokcom
    target.path = /usr/bin
    docs.path = /usr/share/doc/krokcom
    docs.files = docs/* \
        Announce.txt \
        Changes.txt \
        Copyright.txt \
        License.txt \
        Readme.txt
    arm.path = /usr/share/krokcom
    arm.files = arm/*
    desktop.path = /usr/share/applications
    desktop.files = unix/krokcom.desktop
    icon.path = /usr/share/icons
    icon.files = unix/krokcom.png
    INSTALLS += target \
        icon \
        docs \
        arm \
        desktop
}
macx { 
    SOURCES += SerialPortMACOSX.cxx
    HEADERS += SerialPortMACOSX.hxx
    LIBS += -framework \
        CoreFoundation \
        -framework \
        IOKit
    
    # ICON = macosx/Krok_icon.icns
    QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.5.sdk
    CONFIG += x86 \
        ppc
}
