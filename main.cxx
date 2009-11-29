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

#include "bspf_krok.hxx"
#include "Cart.hxx"
#include "SerialPort.hxx"
#include "SerialPortManager.hxx"
#include "KrokComWindow.hxx"
#include "Version.hxx"

void runCommandlineApp(KrokComWindow& win, int ac, char* av[])
{
  string bstype = "", romfile = "";
  bool incremental = false, autoverify = false;

  // Parse commandline args
  for(int i = 1; i < ac; ++i)
  {
    if(strstr(av[i], "-bs=") == av[i])
      bstype = av[i]+4;
    else if(!strcmp(av[i], "-id"))
      incremental = true;
    else if(!strcmp(av[i], "-av"))
      autoverify = true;
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
  cart.setIncremental(incremental);

  // Write to serial port
  if(cart.isValid())
  {
    try
    {
      cout << endl;
      uInt16 sector = 0, numSectors = cart.initSectors(true);
      while(sector < numSectors)
      {
        uInt16 lower = cart.currentSector();
        uInt16 upper = lower + BSPF_min(15, (int)(numSectors-sector-1));

        cout << "Sectors " << setw(4) << lower << " - " << setw(4) << upper << " | ";
        for(uInt16 col = 0; col < 16; ++col)
        {
          if(sector < numSectors)
          {
            cart.writeNextSector(manager.port());
            ++sector;
            cout << "." << flush;
          }
          else
            cout << " " << flush;
        }
        cout << " | successfully sent : " << setw(3) << (100*sector/numSectors) << "% complete" << endl;
      }
    }
    catch(const char* msg)
    {
      cout << msg << endl;
    }

    if(cart.finalizeSectors())
    {
      cout << cart.message() << endl;

      // See if we should automatically verify the download
      if(autoverify)
      {
        // It seems we must wait a while before attempting a verify
        manager.port().sleepMillis(100);
        try
        {
          uInt16 sector = 0, numSectors = cart.initSectors(false);
          while(sector < numSectors)
          {
            uInt16 lower = cart.currentSector();
            uInt16 upper = lower + BSPF_min(15, (int)(numSectors-sector-1));

            cout << endl << "Sectors " << setw(4) << lower << " - " << setw(4) << upper << " | ";
            for(uInt16 col = 0; col < 16; ++col)
            {
              if(sector < numSectors)
              {
                cart.verifyNextSector(manager.port());
                ++sector;
                cout << "." << flush;
              }
              else
                cout << " " << flush;
            }
            cout << " | successfully verified : " << setw(3) << (100*sector/numSectors) << "% complete" << endl;
          }
        }
        catch(const char* msg)
        {
          cout << msg << endl;
        }
      }
    }
    else
      cout << cart.message() << endl;
  }
  else
    cout << "ERROR: Invalid cartridge, not written" << endl;
}


int main(int ac, char* av[])
{
  if(ac == 2 && !strcmp(av[1], "-help"))
  {
    cout << "Krokodile Commander for UNIX version " << KROK_VERSION << endl
         << "  http://krokcom.sf.net" << endl
         << endl
         << "Usage: krokcom [options ...] datafile" << endl
         << "       Run without any options or datafile to use the graphical frontend" << endl
         << "       Consult the manual for more in-depth information" << endl
         << endl
         << "Valid options are:" << endl
         << endl
         << "  -bs=[type]  Specify the bankswitching scheme for a ROM image (default is 'auto')" << endl
         << "  -av         Automatically verify after a download is successfully completed" << endl
         << "  -id         Perform an incremental download (only download changes since last time)" << endl
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
