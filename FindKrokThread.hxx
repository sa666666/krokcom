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

#ifndef __FINDKROKTHREAD_HXX
#define __FINDKROKTHREAD_HXX

#include <QThread>

#include "SerialPortManager.hxx"

/**
  This class is a wrapper thread around the 'find KrokCart' functionality.
  Searching through all available serial ports and querying them can be a
  time-consuming operation, during which the UI would be unresponsive.
  Using a thread eliminates this UI lockup.

  @author  Stephen Anthony
*/
class FindKrokThread: public QThread
{
Q_OBJECT
  public:
    FindKrokThread(SerialPortManager& manager)
      : QThread(),
        myManager(manager)
    { }
    ~FindKrokThread() { }

  protected:
    void run() { myManager.connectKrokCart(); }

  private:
    SerialPortManager& myManager;
};

#endif // FINDKROKTHREAD_HXX
