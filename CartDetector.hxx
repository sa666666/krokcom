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

#ifndef CART_DETECTOR_HXX
#define CART_DETECTOR_HXX

#include "bspf.hxx"
#include "BSType.hxx"

/**
  Auto-detect cart type.
  Based on auto-detection code from Stella (https://stella-emu.github.io)

  @author  Stephen Anthony
*/
class CartDetector
{
  public:
    /**
      Try to auto-detect the bankswitching type of the cartridge

      @param rom  The file containing the ROM image
      @return  The "best guess" for the cartridge type
    */
    static BSType autodetectType(const string& rom);

    /**
      Try to auto-detect the bankswitching type of the cartridge

      @param rom    The file containing the ROM image
      @param image  A pointer to the ROM image
      @param size   The size of the ROM image
      @return  The "best guess" for the cartridge type
    */
    static BSType autodetectType(const string& rom, const uInt8* image, uInt32 size);

    static void addRomInfo(const string& filename, BSType type, const uInt8* image, uInt32 size);
    static BSType getRomInfo(const string& filename, const uInt8* image, uInt32 size);

  private:
    /**
      Search the image for the specified byte signature

      @param image      A pointer to the ROM image
      @param imagesize  The size of the ROM image
      @param signature  The byte sequence to search for
      @param sigsize    The number of bytes in the signature
      @param minhits    The minimum number of times a signature is to be found

      @return  True if the signature was found at least 'minhits' time, else false
    */
    static bool searchForBytes(const uInt8* image, uInt32 imagesize,
                               const uInt8* signature, uInt32 sigsize,
                               uInt32 minhits);

    /**
      Returns true if the image is probably a SuperChip (256 bytes RAM)
    */
    static bool isProbablySC(const uInt8* image, uInt32 size);

    /**
      Returns true if the image is probably a 0840 bankswitching cartridge
    */
    static bool isProbably0840(const uInt8* image, uInt32 size);

    /**
      Returns true if the image is probably a 3E bankswitching cartridge
    */
    static bool isProbably3E(const uInt8* image, uInt32 size);

    /**
      Returns true if the image is probably a 3F bankswitching cartridge
    */
    static bool isProbably3F(const uInt8* image, uInt32 size);

    /**
      Returns true if the image is probably a 4A50 bankswitching cartridge
    */
    static bool isProbably4A50(const uInt8* image, uInt32 size);

    /**
      Returns true if the image is probably a CV bankswitching cartridge
    */
    static bool isProbablyCV(const uInt8* image, uInt32 size);

    /**
      Returns true if the image is probably a DPC+ bankswitching cartridge
    */
    static bool isProbablyDPCplus(const uInt8* image, uInt32 size);

    /**
      Returns true if the image is probably a E0 bankswitching cartridge
    */
    static bool isProbablyE0(const uInt8* image, uInt32 size);

    /**
      Returns true if the image is probably a E7 bankswitching cartridge
    */
    static bool isProbablyE7(const uInt8* image, uInt32 size);

    /**
      Returns true if the image is probably a EF bankswitching cartridge
    */
    static bool isProbablyEF(const uInt8* image, uInt32 size);

    /**
      Returns true if the image is probably an FE bankswitching cartridge
    */
    static bool isProbablyFE(const uInt8* image, uInt32 size);

    /**
      Returns true if the image is probably a SB bankswitching cartridge
    */
    static bool isProbablySB(const uInt8* image, uInt32 size);

    /**
      Returns true if the image is probably a UA bankswitching cartridge
    */
    static bool isProbablyUA(const uInt8* image, uInt32 size);

    /**
      Returns true if the image is probably an X07 bankswitching cartridge
    */
    static bool isProbablyX07(const uInt8* image, uInt32 size);
};

#endif
