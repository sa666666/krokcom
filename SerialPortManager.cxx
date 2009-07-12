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

#include <cstring>

#include "SerialPortManager.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SerialPortManager::SerialPortManager()
  : myFoundKrokCart(false),
    myPortName(""),
    myVersionID("")
{
  myPort.closePort();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SerialPortManager::~SerialPortManager()
{
  myPort.closePort();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SerialPortManager::setDefaultPort(const string& port)
{
  myPortName = port;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SerialPortManager::connectKrokCart()
{
  myPort.closePort();

  myFoundKrokCart = false;
  myVersionID = "";

  // Find (and open) the KrokCart serial port

  // First try the port that was successful the last time
  if(myPortName != "" && connect(myPortName))
  {
    // myPortName already contains the correct name
  }
  else  // Search through all ports
  {
    const StringList& ports = myPort.getPortNames();
    for(uInt32 i = 0; i < ports.size(); ++i)
    {
      if(connect(ports[i]))
        break;
    }
  }

  // Re-initialize the port; make sure we start in a known state
  myPort.closePort();
  if(myFoundKrokCart)
    myPort.openPort(myPortName);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool SerialPortManager::connect(const string& device)
{
  if(myPort.openPort(device))
  {
    uInt8 tx[100];   // transmit buffer
    uInt8 rx[100];   // receive  buffer
    uInt8 ver[100];  // cart version info

    // -------------------------------------------------------------
    // Setup "Send Version Info" Command
    // -------------------------------------------------------------
    memset(rx, 0, 100);
    memset(tx, 0, 100);
    memset(ver, 0, 100);

    tx[0] = 1;  // Mark start of command
    tx[1] = 2;  // Command # for 'Send Version'

    // -------------------------------------------------------------
    // Search for KrokCart device
    // -------------------------------------------------------------
    if(myPort.writeBytes(tx, 2) < 0)
    {
      myPort.closePort();
      return false;
    }
    // Wait for device to respond to command (get ACK)
    rx[0] = myPort.waitForAck();

    // -------------------------------------------------------------
    // If ACK is not sent, the device is not present
    // -------------------------------------------------------------
    int VCnt = 0;
    int BytesRead = 0;
    do
    {
      BytesRead = myPort.readBytes(rx, 1);
      ver[VCnt++] = rx[0];
    }
    while(rx[0] != 0 && VCnt < 100 && BytesRead > 0);
    if(VCnt > 10)
      myPort.writeBytes(tx, 1);  // Send an Ack

    myPort.closePort();

    if(ver[0] != 0 && VCnt > 10)
    {
      myFoundKrokCart = true;
      myPortName = device;
      myVersionID = (const char*)ver;
      return true;
    }
  }

  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool SerialPortManager::krokCartAvailable()
{
  return myFoundKrokCart;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SerialPort& SerialPortManager::port()
{
  return myPort;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const string& SerialPortManager::portName()
{
  return myPortName;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const string& SerialPortManager::versionID()
{
  return myVersionID;
}
