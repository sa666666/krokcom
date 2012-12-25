//============================================================================
//
//  K   K  RRRR    OOO   K   K   CCCC   OOO   M   M
//  K  K   R   R  O   O  K  K   C      O   O  MM MM
//  KKK    RRRR   O   O  KKK    C      O   O  M M M  "Krokodile Cart software"
//  K  K   R R    O   O  K  K   C      O   O  M   M
//  K   K  R  R    OOO   K   K   CCCC   OOO   M   M
//
// Copyright (c) 2009-2013 by Stephen Anthony <stephena@users.sf.net>
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id$
//============================================================================

#ifndef __SERIALPORTMANAGER_HXX
#define __SERIALPORTMANAGER_HXX

#include "bspf_krok.hxx"

#if defined(BSPF_MAC_OSX)
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
    bool krokCartAvailable();

    SerialPort& port();
    const string& portName();
    const string& versionID();

  private:
    bool connect(const string& device);

  private:
  #if defined(BSPF_MAC_OSX)
    SerialPortMACOSX myPort;
  #elif defined(BSPF_UNIX)
    SerialPortUNIX myPort;
  #endif

    bool myFoundKrokCart;
    string myPortName;
    string myVersionID;
};

#endif // __SERIALPORTMANAGER_HXX
