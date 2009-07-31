# -------------------------------------------------
# Project created by QtCreator 2009-05-21T12:40:28
# -------------------------------------------------
TARGET = krokcom
TEMPLATE = app
SOURCES += main.cxx \
    KrokComWindow.cxx \
    Cart.cxx \
    CartDetector.cxx \
    SerialPortManager.cxx \
    SerialPortUNIX.cxx \
    SerialPortWin32.cxx
HEADERS += KrokComWindow.hxx \
    bspf.hxx \
    BSType.hxx \
    Cart.hxx \
    CartDetector.hxx \
    SerialPortManager.hxx \
    SerialPort.hxx \
    SerialPortUNIX.hxx \
    SerialPortWin32.hxx
FORMS += krokcomwindow.ui
RESOURCES += resources.qrc
