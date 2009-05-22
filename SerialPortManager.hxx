//============================================================================
//
//  K   K  RRRR    OOO   K   K   CCCC   OOO   M   M
//  K  K   R   R  O   O  K  K   C      O   O  MM MM
//  KKK    RRRR   O   O  KKK    C      O   O  M M M  "Krokodile Cart software"
//  K  K   R R    O   O  K  K   C      O   O  M   M
//  K   K  R  R    OOO   K   K   CCCC   OOO   M   M
//
// Copyright (c) 2009 by Stephen Anthony <stephena@users.sourceforge.net>
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id$
//============================================================================

#ifndef __SERIALPORTMANAGER_HXX
#define __SERIALPORTMANAGER_HXX

// TODO - this should come from the build environment
#define UNIX 1


#if defined(UNIX)
  #include "SerialPortUNIX.hxx"
#elif defined(MAC_OSX)
  #include "SerialPortMACOSX.hxx"
#endif

class SerialPortManager
{
  public:
    SerialPortManager();
    ~SerialPortManager();

    void connectKrokCart();
    bool krokCartAvailable();

    SerialPort& port();
    const string& portName();
    const string& versionID();

  private:
  #if defined(UNIX)
    SerialPortUNIX myPort;
  #elif defined(MAC_OSX)
    SerialPortMACOSX myPort;
  #else
    #error Unsupported platform!
  #endif

    bool myFoundKrokCart;
    string myPortName;
    string myVersionID;
};

#endif // __SERIALPORTMANAGER_HXX
