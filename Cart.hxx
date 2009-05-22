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
      Loads cartridge data from the given filename, creating a 'single'
      cart.  The bankswitch type is autodetected if type is "".
      The filename should exist and be readable.
    */
    bool createSingle(const string& filename, const string& type = "");

    //////////////////////////////////////////////////////////////////
    //  The following two methods act as an iterator through all the
    //  sectors making up the current Cart.
    //////////////////////////////////////////////////////////////////
    /**
      Initializes the sector iterator to the beginning of the list,
      in preparation for multiple calls to writeNextSector().

      @return  The number of sectors that need to be written
    */
    uInt16 initSectors();

    /**
      Write the next sector in the iterator to the serial port,
      returning the actual sector number that was written.

      @return  The sector number written; an exception is thrown
               on any errors
    */
    uInt16 writeNextSector(SerialPort& port);

    /** Accessor and mutator for bankswitch type. */
    BSType getBSType()            { return myType; }
    void   setBSType(BSType type) { myType = type; }

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
      Write the given sector to the serial port; write() uses this to send
      data to the port in pieces.
    */
    bool downloadSector(uInt32 sector, SerialPort& port);

  private:
    uInt8  myCart[MAXCARTSIZE];
    uInt32 myCartSize;
    BSType myType;

    // The following keep track of progress of sector writes
    uInt16 myCurrentSector;
    uInt16 myNumSectors;

    bool myIsValid;
};

#endif
