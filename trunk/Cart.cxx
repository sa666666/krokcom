//============================================================================
//
//  K   K  RRRR    OOO   K   K   CCCC   OOO   M   M
//  K  K   R   R  O   O  K  K   C      O   O  MM MM
//  KKK    RRRR   O   O  KKK    C      O   O  M M M  "Krokodile Cart software"
//  K  K   R R    O   O  K  K   C      O   O  M   M
//  K   K  R  R    OOO   K   K   CCCC   OOO   M   M
//
// Copyright (c) 2009-2013 by Stephen Anthony <stephena@users.sf.net>
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id$
//============================================================================

#include <cstring>
#include <fstream>
#include <sstream>

#include "bspf_krok.hxx"

#include "BSType.hxx"
#include "Cart.hxx"
#include "MultiCart.hxx"
#include "CartDetector.hxx"
#include "SerialPort.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Cart::Cart()
  : myCartSize(0),
    myRetry(0),
    myType(BS_NONE),
    myIncremental(false),
    myCurrentSector(0),
    myNumSectors(0),
    myIsValid(false),
    myLogMessage("")
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cart::create(const string& filename, const string& type)
{
  // Get the cart image
  memset(myCart, 0, MAXCARTSIZE);
  myCartSize = readFile(filename, myCart, MAXCARTSIZE);

  // Auto-detect the bankswitch type
  if(myType == BS_AUTO || type == "")
  {
    myType = CartDetector::autodetectType(filename, myCart, myCartSize);
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
    case BS_DPCP:
    case BS_4A50:
    case BS_X07:
    case BS_SB:
    case BS_MC:
    {
      ostringstream out;
      out << "Bankswitch mode \'" << Bankswitch::typeToName(myType) << "\' is not supported.";
      myLogMessage = out.str();
      cout << myLogMessage << endl;
      return myIsValid = false;
    }
    default:
      break;
  }

  // Pad sub-4K images to minimum size
  if(myType == BS_4K && myCartSize < 4096)
  {
    padImage(myCart, myCartSize, 4096);
    myCartSize = 4096;
  }

  // 3F and 3E carts need the upper bank in uppermost part of the ROM
  if(myType == BS_3F || myType == BS_3E)
    for(int i = 0; i < 2048; i++)
      myCart[MAXCARTSIZE - 2048 + i] = myCart[myCartSize - 2048 + i];

  for(uInt32 i = 0; i < MAXCARTSIZE/256; ++i)
    myModifiedSectors[i] = true;

  myIsValid = myCartSize > 0;
  if(myIsValid)
    myLogMessage = "Cartridge is valid.";
  else
    myLogMessage = "Invalid cartridge.";

  cout << myLogMessage << endl;
  return myIsValid;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cart::createMultiFile(const StringList& menuNames, const StringList& fileNames,
                           BSType type, bool ntsc, const string& romfile)
{
  myIsValid = false;
  myLogMessage = "Invalid cartridge.";
  myCartSize = 0;
  myType = BS_NONE;
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
  const uInt8* menuPtr = NULL;
  switch(type)
  {
    case BS_MC4K:  romType = BS_4K;  menuPtr = MC_4KMenu;  size = 0;  break;
    case BS_MCF8:  romType = BS_F8;  menuPtr = MC_F8Menu;  size = 1;  break;
    case BS_MCF6:  romType = BS_F6;  menuPtr = MC_F6Menu;  size = 2;  break;
    case BS_MCF4:  romType = BS_F4;  menuPtr = MC_F4Menu;  size = 3;  break;
    default:
      myLogMessage = "Invalid multicart bankswitch scheme.";
      return false;
  }
  uInt8* imgbuf = new uInt8[MC_ByteSizes[size]];

  // Add the menu image
  memcpy(myCart, menuPtr, MC_ByteSizes[size]);

  // Set menu title and clear all menu entries
  menuEntry(menubuffer, "- KROKOCART -");
  for (int charpos = 0; charpos < 13; ++charpos)
    cart[MC_MenuOffset[size] + charpos] = menubuffer[charpos];
  for (int charpos = 13; charpos < (13 * 127); ++charpos)
    cart[MC_MenuOffset[size] + charpos] = 0;

  cart += MC_ByteSizes[size];
  myCartSize += MC_ByteSizes[size];

  // Scan through each item in the list(s)
  int numEntries = BSPF_min((int)menuNames.size(), MC_MaxEntries[size]);
  int validEntries = 0;
  for(int i = 0; i < numEntries; ++i)
  {
    int imgsize = readFile(fileNames[i], imgbuf, MC_ByteSizes[size]);
    BSType imgtype = CartDetector::autodetectType(fileNames[i], imgbuf, imgsize);

    if(imgtype == romType || imgtype == BS_4K)
    {
      if(imgsize < MC_ByteSizes[size])
        padImage(imgbuf, imgsize, MC_ByteSizes[size]);

      // Add the image
      memcpy(cart, imgbuf, MC_ByteSizes[size]);
      cart += MC_ByteSizes[size];        // Point to position for next cart
      myCartSize += MC_ByteSizes[size];  // Cart size increases
      ++validEntries;

      // Add menu entry
      menuEntry(menubuffer, menuNames[i]);
      for (int charpos = 0; charpos < 13; ++charpos)
        myCart[MC_MenuOffset[size] + ((validEntries) * 13) + charpos] =
            menubuffer[charpos];
    }
    else
      cout << "Multicart image " << i << " skipped; invalid bankswitch type \'"
           << Bankswitch::typeToName(imgtype) << "\'" << endl;
  }
  delete[] imgbuf;

  // Set PAL/NTSC
  cout << "Setting " << (ntsc ? "NTSC" : "PAL") << " multicart menu type." << endl;
  myCart[MC_MenuOffset[size] + 2046] = ntsc ? 128 : 0;

  // Set number of menu entries
  cout << "Multicart has " << validEntries << " menu entries." << endl;
  myCart[MC_MenuOffset[size] + 2047] = (uInt8)validEntries;

  myIsValid = validEntries > 0;

  // Save the image to an external file
  ostringstream buf;
  if(myIsValid)
  {
    if(romfile != "")
    {
      if(writeFile(romfile, myCart, myCartSize) == 0)
      {
        myLogMessage = "Couldn't open multicart output file.";
        return false;
      }

      // Add info for this ROM to the database, since autodetection won't know what it is
      CartDetector::addRomInfo(romfile, type, myCart, myCartSize);
    }

    buf << (ntsc ? "NTSC" : "PAL") << " multicart created with " << validEntries << " entries";
    if((int)menuNames.size() > validEntries)
      buf << " (skipped " << (menuNames.size() - validEntries) << ")";
    buf << ", size = " << myCartSize << ".";
    myLogMessage = buf.str();
  }
  else
  {
    buf << "Skipped " << (numEntries - validEntries) << " invalid entries, multicart not created.";
    myLogMessage = buf.str();
  }

  myType = type;
  return myIsValid;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 Cart::initSectors(bool downloadMode)
{
  myCurrentSector = 0;

  if(myIsValid)
  {
    myNumSectors = myCartSize / 256;        // the number of 256 byte sectors
    if(myType == BS_3F || myType == BS_3E)  // 3F and 3E add 8 more (2040 - 2047)
      myNumSectors += 8;

    for(uInt32 i = 0; i < MAXCARTSIZE/256; ++i)
      myModifiedSectors[i] = true;

    // Determine which sectors should be written to the KrokCart
    if(downloadMode)
    {
      ostringstream out;
      if(myIncremental)
      {
        // Read the last rom written
        uInt8 buffer[MAXCARTSIZE];
        memset(buffer, 0, MAXCARTSIZE);
        readFile(ourLastCart, buffer, MAXCARTSIZE, false);

        // Determine which 256 byte blocks differ
        int count = 0;
        uInt8 *cart = myCart, *buf = buffer;
        for(uInt32 i = 0; i < myCartSize/256; ++i, cart += 256, buf += 256)
        {
          myModifiedSectors[i] = memcmp(cart, buf, 256) != 0;
          if(myModifiedSectors[i])  ++count;
        }

        out << "Incremental download mode, " << count << " / "
            << (myCartSize/256) << " sectors are changed.";
        myLogMessage = out.str();
      }
      else
      {
        out << "Normal download mode, " << myNumSectors << " / "
            << (myCartSize/256) << " sectors are changed.";
        myLogMessage = out.str();
      }
      cout << myLogMessage << endl;
    }
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

  uInt16 sector = myCurrentSector;
  uInt32 retry = 0;

  // Only write the sector if it has changed
  if(!myIncremental || myModifiedSectors[sector])
  {
    bool status;
    while(!(status = downloadSector(sector, port)) && retry++ < myRetry)
      cout << "Write transmission of sector " <<  sector << " failed, retry " << retry << endl;
    if(!status)
      throw "write: failed max retries";
  }

  // Handle 3F and 3E carts, which are a little different from the rest
  // There are two ranges of sectors; the second starts once we past the
  // cart size
  myCurrentSector++;
  if((myType == BS_3F || myType == BS_3E) &&
      myCurrentSector == myCartSize / 256)
    myCurrentSector = 2040;

  return sector;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 Cart::verifyNextSector(SerialPort& port)
{
  if(!myIsValid)
    throw "verify: Invalid cart";
  else if(myCurrentSector == myNumSectors)
    throw "verify: All sectors already verified";

  uInt16 sector = myCurrentSector;
  uInt32 retry = 0;

  bool status;
  while(!(status = verifySector(sector, port)) && retry++ < myRetry)
    cout << "Read transmission of sector " <<  sector << " failed, retry " << retry << endl;
  if(!status)
    throw "verify: failed max retries";

  // Handle 3F and 3E carts, which are a little different from the rest
  // There are two ranges of sectors; the second starts once we past the
  // cart size
  myCurrentSector++;
  if((myType == BS_3F || myType == BS_3E) &&
      myCurrentSector == myCartSize / 256)
    myCurrentSector = 2040;

  return sector;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Cart::finalizeSectors()
{
  bool status = false;
  ostringstream out;
  if(myCurrentSector == myNumSectors)
  {
    if(myIncremental)
    {
      int count = 0;
      for(uInt32 i = 0; i < myCartSize/256; ++i)
        if(myModifiedSectors[i])  ++count;

      out << "Incremental download complete, wrote " << count << " / " << myNumSectors << " sectors.";
    }
    else
      out << "Download complete, wrote " << myNumSectors << " sectors.";

    // Write out the current ROM to use for comparison next time
    writeFile(ourLastCart, myCart, MAXCARTSIZE, false);

    status = true;
  }
  else
    out <<  "Download failure on sector " << myCurrentSector << ".";

  myLogMessage = out.str();
  return status;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 Cart::readFile(const string& filename, uInt8* buffer, uInt32 maxSize,
                      bool showmessage) const
{
  if(showmessage) cout << "Reading from file: \'" << filename << "\' ... ";

  // Read file into buffer
  ifstream in(filename.c_str(), ios::binary);
  if(!in)
    return 0;

  // Figure out how much data we should read
  in.seekg(0, ios::end);
  streampos length = in.tellg();
  in.seekg(0, ios::beg);
  uInt32 size = length > maxSize ? maxSize : (uInt32)length;

  in.read((char*)buffer, size);
  if(showmessage) cout << "read in " << size << " bytes" << endl;
  in.close();

  return size;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 Cart::writeFile(const string& filename, uInt8* buffer, uInt32 size,
                       bool showmessage) const
{
  if(showmessage) cout << "Writing to file: \'" << filename << "\' ... ";

  // Write to file from buffer
  ofstream out(filename.c_str(), ios::binary);
  if(!out)
    return 0;

  out.write((char*)buffer, size);
  if(showmessage) cout << "wrote out " << size << " bytes" << endl;
  out.close();

  return size;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Cart::padImage(uInt8* buffer, uInt32 bufsize, uInt32 requiredsize) const
{
  // Pad buffer to minimum size, aligning to power-of-2 boundary
  if(bufsize < requiredsize)
  {
    cout << "  Converting to " << (requiredsize/1024) << "K." << endl;

    // Determine power-of-2 boundary
    uInt32 power2 = 1;
    while(power2 < bufsize)
      power2 <<= 1;

    // Erase all garbage after the valid data
    memset(buffer+bufsize, 0, requiredsize-bufsize);

    // Lay down a copy of the valid buffer data at power-of-2 intervals
    uInt8* tmp_ptr = buffer + power2;
    for(uInt32 i = 1; i < requiredsize/power2; ++i, tmp_ptr += power2)
      memcpy(tmp_ptr, buffer, bufsize);
  }
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
  if(port.send(buffer, 262) != 262)
  {
    cout << "Transmission error in downloadSector" << endl;
    return false;
  }

  // Check return code of sector write
  uInt8 result = 0;
  port.receive(&result, 1);

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
  if(port.send(buffer, 5) != 5)
  {
    cout << "Write transmission error of command in verifySector" << endl;
    return false;
  }

  // Check return code of command write
  uInt8 result = 0;
  port.receive(&result, 1);

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
    if(port.receive(&data, 1) == 1)
      buffer[BytesRead++] = data;
  }
  while(BytesRead < 257);
  port.send(buffer, 1);  // Send an Ack

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
      for (charpos = (sizeof(MC_NameChars) - 1); charpos > 0; charpos --)
      {
        // Check for character in MCNameChars
        if (next == MC_NameChars[charpos])
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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string Cart::ourLastCart = "";
