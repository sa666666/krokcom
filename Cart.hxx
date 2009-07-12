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

#define MAXCARTSIZE 512*1024

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
    static bool createMultiFile(const string& romfile, const string& type,
                                const vector<string>& filenames);

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

    /** Accessor and mutator for bankswitch type. */
    BSType getBSType()            { return myType; }
    void   setBSType(BSType type) { myType = type; }

    /** Set number of write retries before bailing out. */
    void setRetry(int retry) { myRetry = retry; }

    /** Get the current cart size. */
    uInt32 getSize() { return myCartSize; }

    /** Was the ROM loaded correctly? */
    bool isValid() { return myIsValid; }

    /** Auxiliary method to autodetect the bankswitch type. */
    static BSType autodetectType(uInt8* data, uInt32 size);

  private:
    /**
      Read data from given file and place it in the given buffer.
      The bankswitch type is also autodetected here.
    */
    int readFile(const string& filename, uInt8* cartridge, uInt32 maxSize,
                 const string& type);

    /**
      Write the given sector to the serial port.
    */
    bool downloadSector(uInt32 sector, SerialPort& port);

    /**
      Read and verify the given sector from the serial port.
    */
    bool verifySector(uInt32 sector, SerialPort& port);

  private:
    uInt8  myCart[MAXCARTSIZE];
    uInt32 myCartSize;
    uInt32 myRetry;
    BSType myType;

    // The following keep track of progress of sector writes
    uInt16 myCurrentSector;
    uInt16 myNumSectors;

    bool myIsValid;
};

#endif
