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
#include <QAction>
#include <QActionGroup>
#include <QRegExp>
#include <QSettings>
#include <QCloseEvent>
#include <QMessageBox>

#include <iostream>
#include <sstream>
#include <sstream>
using namespace std;

//#include "pics/ledoff.xpm"

#include "KrokComWindow.hxx"
#include "ui_krokcomwindow.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
KrokComWindow::KrokComWindow(QWidget* parent)
  : QMainWindow(parent),
    ui(new Ui::KrokComWindow),
    myDetectedBSType(BS_NONE)
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

  // Deactivate download and verify until it makes sense to use them
  ui->downloadButton->setDisabled(true);  ui->actDownloadROM->setDisabled(true);
  ui->verifyButton->setDisabled(true);  ui->actVerifyROM->setDisabled(true);

  // Initialize settings
  QCoreApplication::setOrganizationName("KrokCom");
  QCoreApplication::setApplicationName("Krokodile Commander");
  readSettings();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
KrokComWindow::~KrokComWindow()
{
  delete ui;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::setupConnections()
{
  // File menu
  connect(ui->actSelectROM, SIGNAL(triggered()), this, SLOT(slotOpenROM()));
  connect(ui->actDownloadROM, SIGNAL(triggered()), this, SLOT(slotDownloadROM()));
  connect(ui->actVerifyROM, SIGNAL(triggered()), this, SLOT(slotVerifyROM()));
  connect(ui->actQuit, SIGNAL(triggered()), this, SLOT(close()));

  // Device menu
  connect(ui->actConnectKrokCart, SIGNAL(triggered()), this, SLOT(slotConnectKrokCart()));

  // Options menu
  QActionGroup* group = new QActionGroup(this);
  group->setExclusive(true);
  group->addAction(ui->actRetry0);
  group->addAction(ui->actRetry1);
  group->addAction(ui->actRetry2);
  group->addAction(ui->actRetry3);
  connect(group, SIGNAL(triggered(QAction*)), this, SLOT(slotRetry(QAction*)));

  // Help menu
  connect(ui->actAbout, SIGNAL(triggered()), this, SLOT(slotAbout()));
  connect(ui->actAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

  // Buttons
  connect(ui->openRomButton, SIGNAL(clicked()), this, SLOT(slotOpenROM()));
  connect(ui->downloadButton, SIGNAL(clicked()), this, SLOT(slotDownloadROM()));
  connect(ui->verifyButton, SIGNAL(clicked()), this, SLOT(slotVerifyROM()));
  connect(ui->romBSType, SIGNAL(activated(const QString&)), this, SLOT(slotSetBSType(const QString&)));

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::readSettings()
{
  // Load settings
  QSettings s;

  s.beginGroup("MainWindow");
    int retrycount = s.value("retrycount", 0).toInt();
    if(retrycount == 0)       ui->actRetry0->setChecked(true);
    else if(retrycount == 1)  ui->actRetry1->setChecked(true);
    else if(retrycount == 2)  ui->actRetry2->setChecked(true);
    else if(retrycount == 3)  ui->actRetry3->setChecked(true);
    myCart.setRetry(retrycount);
    ui->actAutoDownFileSelect->setChecked(s.value("autodownload", false).toBool());
    ui->actAutoVerifyDownload->setChecked(s.value("autoverify", false).toBool());
  s.endGroup();

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::closeEvent(QCloseEvent* event)
{
  // Save settings
  QSettings s;

  s.beginGroup("MainWindow");
    int retrycount = 0;
    if(ui->actRetry0->isChecked())       retrycount = 0;
    else if(ui->actRetry1->isChecked())  retrycount = 1;
    else if(ui->actRetry2->isChecked())  retrycount = 2;
    else if(ui->actRetry3->isChecked())  retrycount = 3;
    s.setValue("retrycount", retrycount);
    s.setValue("autodownload", ui->actAutoDownFileSelect->isChecked());
    s.setValue("autoverify", ui->actAutoVerifyDownload->isChecked());
  s.endGroup();

  event->accept();
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
    QPixmap off(":icons/pics/ledoff.png");
    myLED->setPixmap(off);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotOpenROM()
{
  ui->downloadButton->setDisabled(true);  ui->actDownloadROM->setDisabled(true);
  ui->verifyButton->setDisabled(true);  ui->actVerifyROM->setDisabled(true);

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
    myDetectedBSType = myCart.getBSType();
    QString bstype = Bankswitch::typeToName(myDetectedBSType).c_str();
    int match = ui->romBSType->findText(bstype, Qt::MatchStartsWith);
    ui->romBSType->setCurrentIndex(match < ui->romBSType->count() ? match : 0);
    ui->downloadButton->setDisabled(false);  ui->actDownloadROM->setDisabled(false);

    // See if we should automatically download
    if(ui->actAutoDownFileSelect->isChecked())
      slotDownloadROM();
  }
  else
    myStatus->setText("Invalid cartridge.");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotDownloadROM()
{
cerr << "download ROM\n";
  ui->verifyButton->setDisabled(true);  ui->actVerifyROM->setDisabled(true);

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
  ui->verifyButton->setDisabled(false);  ui->actVerifyROM->setDisabled(false);

  // See if we should automatically verify the download
  if(ui->actAutoVerifyDownload->isChecked())
      slotVerifyROM();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotVerifyROM()
{
cerr << "Verify download of ROM\n";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotRetry(QAction* action)
{
  if(action == ui->actRetry0)       myCart.setRetry(0);
  else if(action == ui->actRetry1)  myCart.setRetry(1);
  else if(action == ui->actRetry2)  myCart.setRetry(2);
  else if(action == ui->actRetry3)  myCart.setRetry(3);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotSetBSType(const QString& text)
{
  QRegExp regex("([a-zA-Z0-9]*)");
  regex.indexIn(text);
  QString t = regex.cap();
  BSType selectedType = Bankswitch::nameToType(regex.cap().toStdString());

  if(myCart.isValid())
  {
    if(myDetectedBSType != selectedType)
    {
cerr << "Sure you want to override type (yes/no/always)?\n";
    }
    else
      myCart.setBSType(myDetectedBSType);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotAbout()
{
  ostringstream about;
  about << "<center>"
        << "<p><b>Krokodile Commander for UNIX v0.1</b></p>"
        << "<p>Copyright &copy; 2009 Stephen Anthony<br>"
        << "<a href=\"mailto:stephena@users.sf.net\">stephena@users.sf.net</a></p>"
        << "<p><a href=\"http://krokcom.sf.net\">http://krokcom.sf.net</a><p>"
        << "<p>Based on the original Windows version<br>"
        << "Copyright &copy; 2002-2009 <a href=\"mailto:Armin.Vogl@gmx.net\">Armin Vogl</a>"
        << "<p>This software is released under the GNU GPLv2</p>"
        << "</center>";

  QMessageBox mb;
  mb.setWindowTitle("Info about Krokodile Commander");
  mb.setIconPixmap(QPixmap(":icons/pics/cart.png"));
  mb.setTextFormat(Qt::RichText);
  mb.setText(about.str().c_str());
  mb.exec();
}
