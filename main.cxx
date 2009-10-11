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

#include <QApplication>
#include <cstring>

#include "bspf.hxx"
#include "Cart.hxx"
#include "SerialPort.hxx"
#include "SerialPortManager.hxx"
#include "KrokComWindow.hxx"
#include "Version.hxx"

void runCommandlineApp(KrokComWindow& win, int ac, char* av[])
{
  string bstype = "", tvformat = "", romfile = "";

  // Parse commandline args
  for(int i = 1; i < ac; ++i)
  {
    if(strstr(av[i], "-bs=") == av[i])
      bstype = av[i]+4;
//    else if(...)         // add more options here
    else
      romfile = av[i];
  }

  SerialPortManager& manager = win.portManager();
  manager.connectKrokCart();
  if(manager.krokCartAvailable())
  {
    cout << "KrokCart: \'" << manager.versionID() << "\'"
         << " @ \'" << manager.portName() << "\'" << endl;
  }
  else
  {
    cout << "KrokCart not detected" << endl;
    return;
  }

  // Create a new cart for writing
  Cart cart;

  // Create a new single-load cart
  cart.create(romfile, bstype);

  // Write to serial port
  if(cart.isValid())
  {
    try
    {
      uInt16 numSectors = cart.initSectors();
      for(uInt16 sector = 0; sector < numSectors; ++sector)
      {
        uInt16 s = cart.writeNextSector(manager.port());
        cout << "Sector " << setw(4) << s << " successfully sent : "
             << setw(3) << (100*(sector+1)/numSectors) << "% complete" << endl;
      }
    }
    catch(const char* msg)
    {
      cout << msg << endl;
    }
  }
  else
    cout << "ERROR: Invalid cartridge, not written" << endl;
}


int main(int ac, char* av[])
{
  if(ac == 2 && !strcmp(av[1], "-help"))
  {
    cout << "Krokodile Commander for UNIX version " << KROK_VERSION << endl
         << endl
         << "Usage: krokcom [options ...] datafile" << endl
         << "       Run without any options or datafile to use the graphical frontend" << endl
         << "       Consult the manual for more in-depth information" << endl
         << endl
         << "Valid options are:" << endl
         << endl
//         << "  -bios       Treat the specified datafile as an EEPROM loader BIOS image" << endl
//         << "              Otherwise, the datafile is treated as a ROM image instead" << endl
         << "  -bs=[type]  Specify the bankswitching scheme for a ROM image (default is 'auto')" << endl
         << "  -help       Displays the message you're now reading" << endl
         << endl
         << "This software is Copyright (c) 2009 Stephen Anthony, and is released" << endl
         << "under the GNU GPL version 3." << endl
         << endl;
    return 0;
  }

  // The application and window needs to be created even if we're using
  // commandline mode, since the settings are controlled by a QSettings
  // object which needs a Qt context.
  QApplication app(ac, av);
  KrokComWindow win;

  if(ac == 1)  // Launch GUI
  {
    // Only start a 'connect' thread if we're in UI mode
    win.connectKrokCart();
    win.show();
    return app.exec();
  }
  else  // Assume we're working from the commandline
  {
    runCommandlineApp(win, ac, av);
  }

  return 0;
}
