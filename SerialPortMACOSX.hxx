//===========================================================================
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

#include "bspf.hxx"

#ifndef __SERIALPORT_MACOSX_HXX
#define __SERIALPORT_MACOSX_HXX

#include	<sys/filio.h>
#include	<sys/ioctl.h>
#include	<CoreFoundation/CoreFoundation.h>

#include	<IOKit/IOKitLib.h>
#include	<IOKit/serial/IOSerialKeys.h>
#include	<IOKit/IOBSD.h>

#include "SerialPort.hxx"

/**
  Implement reading and writing from a serial port under MacOSX.

  @author  Stephen Anthony
*/
class SerialPortMACOSX : public SerialPort
{
  public:
    SerialPortMACOSX();
    virtual ~SerialPortMACOSX();

    /**
      Open the given serial port with the specified attributes.

      @param device  The name of the port
      @return  False on any errors, else true
    */
    bool openPort(const string& device);

    /**
      Close a previously opened serial port.
    */
    void closePort();

    /**
      Answers if the port is currently open and ready for I/O.

      @return  True if open and ready, else false
    */
    bool isOpen();

    /**
      Read byte(s) from the serial port.

      @param data  Destination for the byte(s) read from the port
      @return  The number of bytes read (-1 indicates error)
    */
    int readBytes(uInt8* data, uInt32 size = 1);

    /**
      Write byte(s) to the serial port.

      @param data  The byte(s) to write to the port
      @return  The number of bytes written (-1 indicates error)
    */
    int writeBytes(const uInt8* data, uInt32 size = 1);

    /**
      Wait for acknowledgment from the serial port; currently,
      this means retry a read 100 times while waiting ~500
      microseconds between each attempt.

      @return  The ACK read (0 indicates error)
    */
    uInt8 waitForAck(uInt32 wait = 500);

    /**
      Get all valid serial ports detected on this system.
    */
    const StringList& getPortNames();

  private:
    kern_return_t createSerialIterator(io_iterator_t* serialIterator);
    char* getRegistryString(io_object_t sObj, char* propName);

  private:
    // File descriptor for serial connection
    int myHandle;
};

#endif
