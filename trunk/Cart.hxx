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

#ifndef __CART_HXX
#define __CART_HXX

// 2048 sectors of 256 bytes each
#define MAXCARTSIZE 2048*256

#include <vector>

#include "bspf.hxx"

#include "BSType.hxx"
#include "SerialPort.hxx"

/**
 *
 * @author stephena
 */
class Cart
{
  public:
    /**
      Create a new Cart object, which can be used to create single
      cartridge or one consisting of many ROMs (aka multi-cart).
    */
    Cart();

  public:
    /**
      Loads cartridge data from the given filename, creating a cart.
      The bankswitch type is autodetected if type is "".
      The filename should exist and be readable.
    */
    bool create(const string& filename, const string& type = "");

    /**
      Creates a single ROM file comprised of the given files.
      This will be a 'multi-cart' ROM, consisting of each of the separate
      ROMs, all with the same bankswitching scheme.
    */
    bool createMultiFile(const StringList& menuNames, const StringList& fileNames,
                         BSType type, bool ntsc, const string& romfile = "");

    //////////////////////////////////////////////////////////////////
    //  The following two methods act as an iterator through all the
    //  sectors making up the current Cart.
    //////////////////////////////////////////////////////////////////
    /**
      Initializes the sector iterator to the beginning of the list,
      in preparation for multiple calls to writeNextSector() or
      verifyNextSector().

      NOTE: After calling initSectors(), DO NOT mix calls to
            writeNextSector() and verifyNextSector().

      @return  The number of sectors that need to be accessed
    */
    uInt16 initSectors();

    /**
      Write the next sector in the iterator to the serial port,
      returning the actual sector number that was written.

      NOTE: After calling initSectors(), DO NOT mix calls to
            writeNextSector() and verifyNextSector().

      @return  The sector number written; an exception is thrown
               on any errors
    */
    uInt16 writeNextSector(SerialPort& port);

    /**
      Read and verify the next sector in the iterator from the serial port,
      returning the actual sector number that was verified.

      NOTE: After calling initSectors(), DO NOT mix calls to
            writeNextSector() and verifyNextSector().

      @return  The sector number verified; an exception is thrown
               on any errors
    */
    uInt16 verifyNextSector(SerialPort& port);

    /**
      Return the current position of the sector iterator.

      @return  The current sector number
    */
    uInt16 currentSector() const { return myCurrentSector; }

    /** Accessor and mutator for bankswitch type. */
    BSType getBSType() const      { return myType; }
    void   setBSType(BSType type) { myType = type; }

    /** Accessor and mutator for incremental download. */
    bool getIncremental() const      { return myIncremental;   }
    void setIncremental(bool enable) { myIncremental = enable; }

    /** Set number of write retries before bailing out. */
    void setRetry(int retry) { myRetry = retry; }

    /** Get the current cart size. */
    uInt32 getSize() const { return myCartSize; }

    /** Was the ROM loaded correctly? */
    bool isValid() const { return myIsValid; }

    /** Get the most recent logged message. */
    const string& message() const { return myLogMessage; }

    static void setLastRomFilePath(const string& rom) { ourLastCart = rom; }

  private:
    /**
      Read data from given file and place it in the given buffer.

      @return  The number of bytes read (0 indicates error).
    */
    uInt32 readFile(const string& filename, uInt8* buffer, uInt32 maxSize) const;

    /**
      Write data from the given buffer to the given file.

      @return  The number of bytes written (0 indicates error).
    */
    uInt32 writeFile(const string& filename, uInt8* buffer, uInt32 size) const;

    /**
      Write the given sector to the serial port.
    */
    void padImage(uInt8* buffer, uInt32 bufsize, uInt32 requiredsize) const;

    /**
      Write the given sector to the serial port.
    */
    bool downloadSector(uInt32 sector, SerialPort& port) const;

    /**
      Read and verify the given sector from the serial port.
    */
    bool verifySector(uInt32 sector, SerialPort& port) const;

    /**
      Fill the buffer with the data read ...
    */
    void menuEntry(uInt8* buffer, const string& name) const;

  private:
    uInt8  myCart[MAXCARTSIZE];
    uInt32 myCartSize;
    uInt32 myRetry;
    BSType myType;
    bool   myIncremental;

    // The following keep track of progress of sector writes
    uInt16 myCurrentSector;
    uInt16 myNumSectors;
    bool myModifiedSectors[MAXCARTSIZE/256];

    bool myIsValid;
    string myLogMessage;

    static string ourLastCart;
};

#endif
