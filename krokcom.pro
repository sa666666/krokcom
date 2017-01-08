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
    MD5.cxx \
    AboutDialog.cxx
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
    MD5.hxx \
    bspf_krok.hxx \
    AboutDialog.hxx
FORMS += krokcomwindow.ui \
    aboutdialog.ui
RESOURCES += resources.qrc
QT += widgets
unix:!macx { 
    INCLUDEPATH += unix
    DEFINES += HAVE_INTTYPES BSPF_UNIX
    SOURCES += unix/SerialPortUNIX.cxx
    HEADERS += unix/SerialPortUNIX.hxx
    TARGET = krokcom
    target.path = /usr/bin
    docs.path = /usr/share/doc/krokcom
    docs.files = docs/* \
        Announce.txt \
        Changes.txt \
        Copyright.txt \
        License.txt \
        Readme.txt
    desktop.path = /usr/share/applications
    desktop.files = unix/krokcom.desktop
    icon.path = /usr/share/icons
    icon.files = unix/krokcom.png
    INSTALLS += target \
        icon \
        docs \
        desktop
}
macx { 
    INCLUDEPATH += macosx
    DEFINES += HAVE_INTTYPES BSPF_MAC_OSX
    SOURCES += macosx/SerialPortMACOSX.cxx
    HEADERS += macosx/SerialPortMACOSX.hxx
    LIBS += -framework CoreFoundation -framework IOKit
    ICON = macosx/krokcom.icns
}
