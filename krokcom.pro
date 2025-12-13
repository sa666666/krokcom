TARGET = KrokCom
TEMPLATE = app

SOURCES += src/common/main.cxx \
    src/common/KrokComWindow.cxx \
    src/common/Cart.cxx \
    src/common/CartDetector.cxx \
    src/common/SerialPortManager.cxx \
    src/common/MD5.cxx \
    src/common/AboutDialog.cxx
HEADERS += src/common/KrokComWindow.hxx \
    src/common/bspf.hxx \
    src/common/BSType.hxx \
    src/common/Cart.hxx \
    src/common/CartDetector.hxx \
    src/common/SerialPortManager.hxx \
    src/common/SerialPort.hxx \
    src/common/FindKrokThread.hxx \
    src/common/Version.hxx \
    src/common/MultiCart.hxx \
    src/common/MD5.hxx \
    src/common/AboutDialog.hxx
FORMS += src/common/krokcomwindow.ui src/common/aboutdialog.ui

RESOURCES += src/resources.qrc
INCLUDEPATH += src/common
QT += widgets
MOC_DIR = obj
OBJECTS_DIR = obj
UI_DIR = obj

unix:!macx {
    DEFINES += BSPF_UNIX
    INCLUDEPATH += src/unix
    SOURCES += src/unix/SerialPortUNIX.cxx
    HEADERS += src/unix/SerialPortUNIX.hxx
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
    desktop.files = src/unix/krokcom.desktop
    icon.path = /usr/share/icons
    icon.files = src/unix/krokcom.png
    INSTALLS += target \
        icon \
        docs \
        desktop
    QMAKE_CXXFLAGS += -std=c++20
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter
}

macx {
    DEFINES += BSPF_MACOS
    INCLUDEPATH += src/macos
    SOURCES += src/macos/SerialPortMACOS.cxx
    HEADERS += src/macos/SerialPortMACOS.hxx
    LIBS += -framework CoreFoundation -framework IOKit
    ICON = src/macos/krokcom.icns
    QMAKE_CXXFLAGS += -std=c++20
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter -Wno-deprecated-declarations
}
