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

#include <QFileDialog>
#include <QLabel>
#include <QPixmap>
#include <QString>
#include <QStatusBar>
#include <QTextEdit>
#include <QProgressDialog>

#include <iostream>
#include <sstream>
using namespace std;

//#include "pics/ledoff.xpm"

#include "KrokComWindow.hxx"
#include "ui_krokcomwindow.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
KrokComWindow::KrokComWindow(QWidget* parent)
  : QMainWindow(parent),
    ui(new Ui::KrokComWindow)
{
  // Create GUI
  ui->setupUi(this);

  // Set up signal/slot connections
  setupConnections();

  // The text part of the status bar
  myStatus = new QLabel();
  myStatus->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
  statusBar()->addPermanentWidget(myStatus, 1000);

  // The LED part of the status bar
  myLED = new QLabel();
  statusBar()->addPermanentWidget(myLED);

  // Find and connect to KrokCart
  slotConnectKrokCart();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
KrokComWindow::~KrokComWindow()
{
  delete ui;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::setupConnections()
{
  // Menus
  connect(ui->actSelectROM, SIGNAL(triggered()), this, SLOT(slotOpenROM()));
  connect(ui->actDownloadROM, SIGNAL(triggered()), this, SLOT(slotDownloadROM()));
  connect(ui->actVerifyROM, SIGNAL(triggered()), this, SLOT(slotVerifyROM()));
  connect(ui->actConnectKrokCart, SIGNAL(triggered()), this, SLOT(slotConnectKrokCart()));
  connect(ui->actQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
  connect(ui->actAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

  // Buttons
  connect(ui->openRomButton, SIGNAL(clicked()), this, SLOT(slotOpenROM()));
  connect(ui->downloadButton, SIGNAL(clicked()), this, SLOT(slotDownloadROM()));
  connect(ui->verifyButton, SIGNAL(clicked()), this, SLOT(slotVerifyROM()));

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotConnectKrokCart()
{
  myManager.connectKrokCart();
  if(myManager.krokCartAvailable())
  {
    QString about = "\'";
    about.append(myManager.versionID().c_str());
    about.append("\' @ \'");
    about.append(myManager.portName().c_str());
    about.append("\'");
    myStatus->setText(about);
//    QPixmap on(ledon_xpm);
//    myLED->setPixmap(on);
  }
  else
  {
    myStatus->setText("Krokodile Cartridge not found.");
//    QPixmap off(ledoff_xpm);
//    myLED->setPixmap(off);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotOpenROM()
{
  QString file = QFileDialog::getOpenFileName(this,
    tr("Select ROM Image"), "", tr("Atari 2600 ROM Image (*.bin *.a26)"));

  if(file.isNull())
    return;

  // Create a single-load cart
  myCart.createSingle(file.toStdString());

  if(myCart.isValid())
  {
    ui->romFileEdit->setText(file);
    ui->romSizeLabel->setText(QString::number(myCart.getSize()) + " bytes.");
    QString bstype = Bankswitch::typeToName(myCart.getBSType()).c_str();
    int match = ui->romBSType->findText(bstype, Qt::MatchStartsWith);
    ui->romBSType->setCurrentIndex(match < ui->romBSType->count() ? match : 0);
  }
  else
    myStatus->setText("Invalid cartridge.");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotDownloadROM()
{
  if(!myManager.krokCartAvailable())
  {
    myStatus->setText("Krokodile Cartridge not found.");
    return;
  }
  else if(!myCart.isValid())
  {
    myStatus->setText("Invalid cartridge.");
    return;
  }

  // Write to serial port
  uInt16 numSectors = myCart.initSectors();
  QProgressDialog progress("Downloading ROM...", QString(), 0, numSectors, this);
  progress.setWindowModality(Qt::WindowModal);
  progress.setMinimumDuration(0);
  try
  {
    for(uInt16 sector = 0; sector < numSectors; ++sector)
    {
      myCart.writeNextSector(myManager.port());
      progress.setValue(sector);
//      if(progress.wasCanceled())
//        break;
    }
  }
  catch(const char* msg)
  {
  }
  progress.setValue(numSectors);
  myStatus->setText("Cartridge downloaded.");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotVerifyROM()
{
}
