//===========================================================================
//
//  K   K  RRRR    OOO   K   K   CCCC   OOO   M   M
//  K  K   R   R  O   O  K  K   C      O   O  MM MM
//  KKK    RRRR   O   O  KKK    C      O   O  M M M  "Krokodile Cart software"
//  K  K   R R    O   O  K  K   C      O   O  M   M
//  K   K  R  R    OOO   K   K   CCCC   OOO   M   M
//
// Copyright (c) 2009-2025 by Stephen Anthony <sa666666@gmail.com>
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================


#ifndef SERIAL_PORT_MACOS_HXX
#define SERIAL_PORT_MACOS_HXX

#include <sys/filio.h>
#include <sys/ioctl.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/IOBSD.h>

#include <termios.h>

#include "bspf.hxx"
#include "SerialPort.hxx"

/**
  Implement reading and writing from a serial port under MacOSX.

  @author  Stephen Anthony
*/
class SerialPortMACOS : public SerialPort
{
  public:
    SerialPortMACOS();
    virtual ~SerialPortMACOS();

    /**
      Open the given serial port with the specified attributes.

      @param device  The name of the port
      @return  False on any errors, else true
    */
    bool openPort(const string& device) override;

    /**
      Close a previously opened serial port.
    */
    void closePort() override;

    /**
      Answers if the port is currently open and ready for I/O.

      @return  True if open and ready, else false
    */
    bool isOpen() override;

    /**
      Receives a buffer from the open com port. Returns all the characters
      ready (waits for up to 'n' milliseconds before accepting that no more
      characters are ready) or when the buffer is full. 'n' is system dependent,
      see SerialTimeout routines.

      @param answer    Buffer to hold the bytes read from the serial port
      @param max_size  The size of buffer pointed to by answer
      @return  The number of bytes read
    */
    uInt32 receiveBlock(void* answer, uInt32 max_size) override;

    /**
      Write block of bytes to the serial port.

      @param data  The byte(s) to write to the port
      @param size  The size of the block
      @return  The number of bytes written
    */
    uInt32 sendBlock(const void* data, uInt32 size) override;

    /**
      Sets (or resets) the timeout to the timout period requested.  Starts
      counting to this period.  This timeout support is a little odd in that
      the timeout specifies the accumulated deadtime waiting to read not the
      total time waiting to read. They should be close enough to the same for
      this use. Used by the serial input routines, the actual counting takes
      place in receiveBlock.

      @param timeout_milliseconds  The time in milliseconds to use for timeout
    */
    void setTimeout(uInt32 timeout_milliseconds) override;

    /**
      Empty the serial port buffers.  Cleans things to a known state.
    */
    void clearBuffers() override;

    /**
      Controls the modem lines to place the microcontroller into various
      states during the programming process.

      @param DTR  The state to set the DTR line to
      @param RTS  The state to set the RTS line to
    */
    void controlModemLines(bool DTR, bool RTS) override;

    /**
      Sleep the specified amount of time (in milliseconds).
    */
    void sleepMillis(uInt32 milliseconds) override;

    /**
      Get all valid serial ports detected on this system.
    */
    const StringList& getPortNames() override;

  private:
    kern_return_t createSerialIterator(io_iterator_t* serialIterator);
    const char* getRegistryString(io_object_t sObj, const char* propName);

  private:
    // File descriptor for serial connection
    int myHandle{0};

    struct termios myOldtio, myNewtio;
};

#endif
