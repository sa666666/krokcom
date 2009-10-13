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
    myIsValid(false),
    myLogMessage("")
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cart::create(const string& filename, const string& type)
{
  memset(myCart, 0, MAXCARTSIZE);
  myCartSize = readFile(filename, myCart, MAXCARTSIZE, type);
  myIsValid = myCartSize > 0;
  return myIsValid;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cart::createMultiFile(const string& romfile, BSType type, bool ntsc,
                           const StringList& menuNames, const StringList& fileNames)
{
  static int MCByteSizes[]  = {4096, 8192, 16384, 32768};
  static int MCMaxEntries[] = {127, 63, 31, 15};
  static int MCMenuOffset[] = {0, 4096, 12288, 28672};

  myIsValid = false;
  myLogMessage = "Invalid cartridge.";
  myCartSize = 0;
  memset(myCart, 0, MAXCARTSIZE);
  uInt8 *cart = myCart, menubuffer[13];

  // Rudimentary consistency check of lists
  if(menuNames.size() != fileNames.size())
  {
    myLogMessage = "Menu and file names don't match.";
    return false;
  }

  // Determine the bankswitch scheme each ROM should have
  BSType romType = BS_NONE;
  int size = 0;
  switch(type)
  {
    case BS_MC4K:  romType = BS_4K;  size = 0;  break;
    case BS_MCF8:  romType = BS_F8;  size = 1;  break;
    case BS_MCF6:  romType = BS_F6;  size = 2;  break;
    case BS_MCF4:  romType = BS_F4;  size = 3;  break;
    default:
      myLogMessage = "Invalid multicart bankswitch scheme.";
      return false;
  }

  // Add the menu image
  if(readFile("MC4K.bin", cart, MCByteSizes[size], "") == MCByteSizes[size])
  {
    // Set menu title and clear all menu entries
    menuEntry(menubuffer, "- KROKOCART -");
    for (int charpos = 0; charpos < 13; ++charpos)
      cart[MCMenuOffset[size] + charpos] = menubuffer[charpos];
    for (int charpos = 13; charpos < (13 * 127); ++charpos)
      cart[MCMenuOffset[size] + charpos] = 0;

    cart += MCByteSizes[size];
    myCartSize += MCByteSizes[size];
  }
  else
  {
    myLogMessage = "Couldn't find multicart menu image.";
    return false;
  }

  // Scan through each item in the list(s)
  int numEntries = BSPF_min((int)menuNames.size(), MCMaxEntries[size]);
  int delEntries = BSPF_max((int)menuNames.size() - numEntries, 0);
  int validEntries = 0;
  for(int i = 0; i < numEntries; ++i)
  {
    // First check if the size match
    if(readFile(fileNames[i], cart, MCByteSizes[size], "") == MCByteSizes[size])
    {
      // We also only want ROMs where the bankswitch scheme matches
      if(myType == romType)
      {
        cart += MCByteSizes[size];        // Point to position for next cart
        myCartSize += MCByteSizes[size];  // Cart size increases
        ++validEntries;

        // Add menu entry
        menuEntry(menubuffer, menuNames[i]);
        for (int charpos = 0; charpos < 13; ++charpos)
          myCart[MCMenuOffset[size] + ((validEntries) * 13) + charpos] =
              menubuffer[charpos];
      }
      else
        cout << "Multicart image at position " << i << " skipped." << endl;
    }
  }

  // Set PAL/NTSC
  cout << "Setting " << (ntsc ? "NTSC" : "PAL") << " multicart menu type." << endl;
  myCart[MCMenuOffset[size] + 2046] = ntsc ? 128 : 0;

  // Set number of menu entries
  cout << "Multicart has " << validEntries << " menu entries." << endl;
  myCart[MCMenuOffset[size] + 2047] = (uInt8)validEntries;

  myIsValid = myCartSize > 0;

  // Save the image to an external file
  if(myIsValid)
  {
    ofstream out(romfile.c_str(), ios::binary);
    if(!out)
    {
    }

    out.write((char*)myCart, myCartSize);
    cout << "Wrote out " << myCartSize << " bytes." << endl;
    out.close();
  }

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
    cout << "Write transmission of sector " <<  sector << " failed, retry " << retry << endl;
  if(!status)
    throw "write: failed max retries";

  myCurrentSector++;
  return sector;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 Cart::verifyNextSector(SerialPort& port)
{
  if(!myIsValid)
    throw "verify: Invalid cart";
  else if(myCurrentSector == myNumSectors)
    throw "verify: All sectors already verified";

  // Handle 3F and 3E carts, which are a little different from the rest
  // There are two ranges of sectors; the second starts once we past the
  // cart size
  if((myType == BS_3F || myType == BS_3E) &&
      myCurrentSector == myCartSize / 256)
    myCurrentSector = 2040;

  uInt16 sector = myCurrentSector;
  uInt32 retry = 0;
  bool status;
  while(!(status = verifySector(sector, port)) && retry++ < myRetry)
    cout << "Read transmission of sector " <<  sector << " failed, retry " << retry << endl;
  if(!status)
    throw "verify: failed max retries";

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
  if(myType == BS_AUTO || type == "")
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
    case BS_DPC:
    case BS_4A50:
    case BS_X07:
    case BS_SB:
    case BS_MC:
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
bool Cart::downloadSector(uInt32 sector, SerialPort& port) const
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
  if(port.sendBlock(buffer, 262) != 262)
  {
    cout << "Transmission error in downloadSector" << endl;
    return false;
  }

  // Check return code of sector write
  uInt8 result = 0;
  port.receiveBlock(&result, 1);

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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cart::verifySector(uInt32 sector, SerialPort& port) const
{
  uInt8 buffer[257];

  uInt8 chksum = 0;
  buffer[0] = 1;                             // Mark start of command
  buffer[1] = 1;                             // Command # for 'Read Sector'
  buffer[2] = (uInt8)((sector >> 8) & 0xff); // Sector # Hi-Byte
  buffer[3] = (uInt8)sector;                 // Sector # Lo-Byte
  chksum ^= buffer[2];
  chksum ^= buffer[3];
  buffer[4] = chksum;                        // Chksum

  // Write command to serial port
  if(port.sendBlock(buffer, 5) != 5)
  {
    cout << "Write transmission error of command in verifySector" << endl;
    return false;
  }

  // Check return code of command write
  uInt8 result = 0;
  port.receiveBlock(&result, 1);

  // Check return code
  if(result == 0x00)
  {
    cout << "Checksum Error for verify sector " << sector << endl;
    return false;
  }
  else if(result != 0xfe)
  {
    cout << "Undefined response " << (int)result << " for sector " << sector << endl;
    return false;
  }

  // Now it's safe to read the sector (256 data bytes + 1 chksum)
  int BytesRead = 0;
  do
  {
    uInt8 data = 0;
    if(port.receiveBlock(&data, 1) == 1)
      buffer[BytesRead++] = data;
  }
  while(BytesRead < 257);
  port.sendBlock(buffer, 1);  // Send an Ack

  // Make sure the data chksum matches
  chksum = 0;
  for(int i = 0; i < 256; ++i)
    chksum ^= buffer[i];
  if(chksum != buffer[256])
    return false;

  // Now that we have a valid sector read back from the device,
  // compare to the actual data to make sure they match
  return memcmp(myCart + sector*256, &buffer, 256) == 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cart::menuEntry(uInt8* menuentry, const string& menuname) const
{
  static uInt8 MCNameChars[] = {
    ' ','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q',
    'R','S','T','U','V','W','X','Y','Z','*','=','0','1','2','3','4','5','6',
    '7','8','9','_','(',')','-'
  };

  // Iterate through characters in name
  int charpos;
  char next;
  for (uInt32 stringpos = 0; stringpos < 13; stringpos ++)
  {
    // Pad with spaces once we have reached the end of the name
    if (stringpos >= menuname.length())
      charpos = 0;
    else 
    {
      // Retrieve next character from name 
      next = menuname[stringpos];
      for (charpos = (sizeof(MCNameChars) - 1); charpos > 0; charpos --)
      {
        // Check for character in MCNameChars
        if (next == MCNameChars[charpos])
          break;
      }
    }
    menuentry[stringpos] = (uInt8)(charpos * 5);  // Chars are 5 lines high
  }

  // Copy first character to last place (oddity of krok menu software)
  uInt8 first = menuentry[0];
  for (charpos = 1 ; charpos < 13; charpos ++)
    menuentry[charpos - 1] = menuentry[charpos];
  menuentry[12] = first;
}
