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
#include <fstream>

#include "bspf.hxx"

#include "BSType.hxx"
#include "Cart.hxx"
#include "CartDetector.hxx"
#include "SerialPort.hxx"


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cart::Cart()
  : myCartSize(0),
    myRetry(0),
    myType(BS_NONE),
    myCurrentSector(0),
    myNumSectors(0),
    myIsValid(false)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cart::createSingle(const string& filename, const string& type)
{
  memset(myCart, 0, MAXCARTSIZE);
  myCartSize = readFile(filename, myCart, MAXCARTSIZE, type);
  myIsValid = myCartSize > 0;
  return myIsValid;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 Cart::initSectors()
{
  myCurrentSector = 0;

  if(myIsValid)
  {
    myNumSectors = myCartSize / 256;        // the number of 256 byte sectors
    if(myType == BS_3F || myType == BS_3E)  // 3F and 3E add 8 more (2040 - 2047)
      myNumSectors += 8;
  }
  else
    myNumSectors = 0;

  return myNumSectors;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 Cart::writeNextSector(SerialPort& port)
{
  if(!myIsValid)
    throw "write: Invalid cart";
  else if(myCurrentSector == myNumSectors)
    throw "write: All sectors already written";

  // Handle 3F and 3E carts, which are a little different from the rest
  // There are two ranges of sectors; the second starts once we past the
  // cart size
  if((myType == BS_3F || myType == BS_3E) &&
      myCurrentSector == myCartSize / 256)
    myCurrentSector = 2040;

  uInt16 sector = myCurrentSector;
  uInt32 retry = 0;
  bool status;
  while(!(status = downloadSector(sector, port)) && retry++ < myRetry)
    cout << "Transmission of sector " <<  sector << " failed, retry " << retry << endl;
  if(!status)
    throw "write: failed max retries";

  myCurrentSector++;
  return sector;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BSType Cart::autodetectType(uInt8* data, uInt32 size)
{
  return CartDetector::autodetectType(data, size);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int Cart::readFile(const string& filename, uInt8* cartridge, uInt32 maxSize,
                   const string& type)
{
  cout << "Reading from file: \'" << filename << "\'" << endl;

  uInt32 minsize = (maxSize != MAXCARTSIZE) ? maxSize : 4096;

  // Read file into buffer
  ifstream in(filename.c_str(), ios::binary);
  if(!in)
    return 0;

  // Figure out how much data we should read
  in.seekg(0, ios::end);
  streampos length = in.tellg();
  in.seekg(0, ios::beg);
  uInt32 cartsize = length > maxSize ? maxSize : (uInt32)length;

  in.read((char*)cartridge, cartsize);
  cout << "Read in " << cartsize << " bytes" << endl;
  in.close();

  // Auto-detect the bankswitch type
  /* TODO - do we really need to consult stella.pro anymore??
  {   // find MD5 value
    MD5_CTX context;
    unsigned char digest[16];
    MD5Init (&context);
    MD5Update (&context, cartridge, cartsize);
    MD5Final (digest, &context);
    printf ("MD5 = ");
    MDPrint (digest);
  }
  */
  if(type == "")
  {
    myType = autodetectType(cartridge, cartsize);
    cout << "Bankswitch type: " << Bankswitch::typeToName(myType)
         << " (auto-detected)" << endl;
  }
  else
  {
    myType = Bankswitch::nameToType(type);
    cout << "Bankswitch type: " << Bankswitch::typeToName(myType)
         << " (WARNING: overriding auto-detection)" << endl;
  }
  switch(myType)
  {
    case BS_F0:
    case BS_E0:
    case BS_FE:
    case BS_AR:
    case BS_NONE:
      cout << "Warning - The Krokodile Cartridge does not support this type of bank switching" << endl;
      break;
    default:
      break;
  }

  // Pad buffer to minimum size
  if(cartsize < minsize)
  {
    cout << "  Converting to " << (minsize/1024) << "K." << endl;
    int i = 0;
    while(cartsize < minsize)
      cartridge[cartsize++] = cartridge[i++];
  }
  cout << endl;

  // 3F and 3E carts need the upper bank in uppermost part of the ROM
  if(myType == BS_3F || myType == BS_3E)
    for(int i = 0; i < 2048; i++)
      myCart[MAXCARTSIZE - 2048 + i] = myCart[cartsize - 2048 + i];

  return cartsize;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cart::downloadSector(uInt32 sector, SerialPort& port)
{
  uInt8 buffer[262];

  buffer[0] = 1;                             // Mark start of command
  buffer[1] = 0;                             // Command # for 'Download Sector'
  buffer[2] = (uInt8)((sector >> 8) & 0xff); // Sector # Hi-Byte
  buffer[3] = (uInt8)sector;                 // Sector # Lo-Byte
  buffer[4] = (uInt8)myType;                 // Bankswitching mode

  uInt8 chksum = 0;
  for(int i = 0; i < 256; i++)
    buffer[5+i] = myCart[(sector*256) + i];
  for(int i = 2; i < 261; i++)
    chksum ^= buffer[i];
  buffer[261] = chksum;

  // Write sector to serial port
  if(port.writeBytes(buffer, 262) != 262)
  {
    cout << "Transmission error in downloadSector" << endl;
    return false;
  }

  // Check return code of sector write
  uInt8 result = port.waitForAck();

  // Check return code
  if(result == 0x7c)
  {
    cout << "Checksum Error for sector " << sector << endl;
    return false;
  }
  else if(result == 0xff)
  {
    return true;
  }
  else
  {
    cout << "Undefined response " << (int)result << " for sector " << sector << endl;
    return false;
  }
}
