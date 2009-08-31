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

#ifndef __SERIALPORT_HXX
#define __SERIALPORT_HXX

#include "bspf.hxx"

#include <vector>
typedef vector<string> StringList;

/**
  This class provides an interface for a standard serial port.
  For now, this is used when connecting a Krokodile Cart,
  and as such it always uses 115200, 8n1, no flow control.

  @author  Stephen Anthony
*/
class SerialPort
{
  public:
    SerialPort() { }
    virtual ~SerialPort() { }

    /**
      Open the given serial port with the specified attributes.

      @param device  The name of the port
      @return  False on any errors, else true
    */
    virtual bool openPort(const string& device) = 0;

    /**
      Close a previously opened serial port.
    */
    virtual void closePort() = 0;

    /**
      Answers if the port is currently open and ready for I/O.

      @return  True if open and ready, else false
    */
    virtual bool isOpen() = 0;

    /**
      Read byte(s) from the serial port.

      @param data  Destination for the byte(s) read from the port
      @return  The number of bytes read (-1 indicates error)
    */
    virtual int readBytes(uInt8* data, uInt32 size = 1) = 0;

    /**
      Write byte(s) to the serial port.

      @param data  The byte(s) to write to the port
      @return  The number of bytes written (-1 indicates error)
    */
    virtual int writeBytes(const uInt8* data, uInt32 size = 1) = 0;

    /**
      Wait for acknowledgment from the serial port; currently,
      this means retry a read 100 times while waiting ~500
      microseconds between each attempt.

      @return  The ACK read (0 indicates error)
    */
    virtual uInt8 waitForAck(uInt32 wait = 500) = 0;

    /**
      Get all valid serial ports detected on this system.
    */
    virtual const StringList& getPortNames() = 0;

    /**
      Get/set ID string for this port.
    */
    const string& getID() { return myID; }
    void setID(const string& id) { myID = id; }

  protected:
    StringList myPortNames;
    string myID;
};

#endif