//============================================================================
//
//  K   K  RRRR    OOO   K   K   CCCC   OOO   M   M
//  K  K   R   R  O   O  K  K   C      O   O  MM MM
//  KKK    RRRR   O   O  KKK    C      O   O  M M M  "Krokodile Cart software"
//  K  K   R R    O   O  K  K   C      O   O  M   M
//  K   K  R  R    OOO   K   K   CCCC   OOO   M   M
//
// Copyright (c) 2009-2020 by Stephen Anthony <sa666666@gmail.com>
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#ifndef SERIAL_PORT_MANAGER_HXX
#define SERIAL_PORT_MANAGER_HXX

#include "bspf.hxx"

#if defined(BSPF_MACOS)
  #include "SerialPortMACOSX.hxx"
#elif defined(BSPF_UNIX)
  #include "SerialPortUNIX.hxx"
#else
  #error Unsupported platform!
#endif

class SerialPortManager
{
  public:
    SerialPortManager();
    ~SerialPortManager();

    void setDefaultPort(const string& port);
    void connectKrokCart();
    bool krokCartAvailable() const;

    SerialPort& port();
    const string& portName() const;
    const string& versionID() const;

  private:
    bool connect(const string& device);

  private:
  #if defined(BSPF_MACOS)
    SerialPortMACOSX myPort;
  #elif defined(BSPF_UNIX)
    SerialPortUNIX myPort;
  #endif

    bool myFoundKrokCart{false};
    string myPortName;
    string myVersionID;
};

#endif // SERIAL_PORT_MANAGER_HXX
