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

#ifndef __KROKCOM_WINDOW_HXX
#define __KROKCOM_WINDOW_HXX

#include <QMainWindow>
#include <QAction>
#include <QLabel>
#include <QProgressBar>

#include "Cart.hxx"
#include "SerialPortManager.hxx"
#include "ui_krokcomwindow.h"

namespace Ui
{
  class KrokComWindow;
}

class KrokComWindow : public QMainWindow
{
Q_OBJECT
  public:
    KrokComWindow(QWidget* parent = 0);
    ~KrokComWindow();

  private:
    void setupConnections();

  private slots:
    void slotConnectKrokCart();
    void slotOpenROM();
    void slotDownloadROM();
    void slotVerifyROM();
    void slotRetry(QAction* action);
    void slotSetBSType(const QString& text);


  private:
    Ui::KrokComWindow* ui;

    Cart myCart;
    SerialPortManager myManager;
    BSType myDetectedBSType;

    QLabel* myStatus;
    QLabel* myLED;
    QProgressBar* myProgress;
};

#endif
