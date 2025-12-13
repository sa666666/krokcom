//============================================================================
//
//  K   K  RRRR    OOO   K   K   CCCC   OOO   M   M
//  K  K   R   R  O   O  K  K   C      O   O  MM MM
//  KKK    RRRR   O   O  KKK    C      O   O  M M M  "Krokodile Cart software"
//  K  K   R R    O   O  K  K   C      O   O  M   M
//  K   K  R  R    OOO   K   K   CCCC   OOO   M   M
//
// Copyright (c) 2009-2025 by Stephen Anthony <sa666666@gmail.com>
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
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
#include <QButtonGroup>
#include <QRegExp>
#include <QSettings>
#include <QCloseEvent>
#include <QMessageBox>
#include <QTimer>
#include <QTableWidget>
#include <QHeaderView>
#include <QDir>

#include <iostream>
#include <sstream>
#include <sstream>
using namespace std;

#include "AboutDialog.hxx"
#include "KrokComWindow.hxx"
#include "ui_krokcomwindow.h"
#include "CartDetector.hxx"
#include "Version.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
KrokComWindow::KrokComWindow(QWidget* parent)
  : QMainWindow(parent),
    ui(new Ui::KrokComWindow)
{
  // Create GUI
  ui->setupUi(this);

  // Create thread to find Krok cart
  // We use a thread so the UI isn't blocked
  myFindKrokThread = new FindKrokThread(myManager);

  // Set up signal/slot connections
  setupConnections();

  // The text part of the status bar
  myStatus = new QLabel();
  myStatus->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
  statusBar()->addPermanentWidget(myStatus, 1000);

  // The LED part of the status bar
  myLED = new QLabel();
  statusBar()->addPermanentWidget(myLED);

  // Deactivate bankswitch, download and verify until it makes sense to use them
  ui->romBSType->setDisabled(true);
  ui->downloadButton->setDisabled(true);  ui->actDownloadROM->setDisabled(true);
  ui->verifyButton->setDisabled(true);  ui->actVerifyROM->setDisabled(true);

  // Initialize settings
  QCoreApplication::setOrganizationName("KrokCom");
  QCoreApplication::setApplicationName("Krokodile Commander");
  readSettings();

  ///////////////////////////////////////////////////////////
  // Set up multicart table
  // The table has to be customized more than Designer will
  // allow, so we do almost everything here
  ///////////////////////////////////////////////////////////
  // Set fixed sizes for headers
  ui->mcartTable->horizontalHeader()->resizeSection(0, 140);
  ui->mcartTable->horizontalHeader()->resizeSection(2, 28);
  ui->mcartTable->horizontalHeader()->resizeSection(3, 28);
  ui->mcartTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  ui->mcartTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

  slotSetMCBSType(0);

  // By default, start looking for ROMs in the users' home directory
  myLastDir.setPath(QDir::home().absolutePath());

  // Store last ROM info in '$HOME/.KCLASTROM.bin'
  QString lastrom = QDir(QDir::home().absolutePath() + "/.KCLASTROM.bin").absolutePath();
  Cart::setLastRomFilePath(lastrom.toStdString());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
KrokComWindow::~KrokComWindow()
{
  if(myFindKrokThread)
  {
    myFindKrokThread->quit();
    delete myFindKrokThread;
  }
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
  connect(ui->actIncDownload, SIGNAL(triggered(bool)), this, SLOT(slotEnableIncDownload(bool)));
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

  // Other
  connect(myFindKrokThread, SIGNAL(finished()), this, SLOT(slotUpdateFindKrokStatus()));

  ///////////////////////////////////////////////////////////
  // 'ROM' tab
  ///////////////////////////////////////////////////////////
  // Buttons
  connect(ui->openRomButton, SIGNAL(clicked()), this, SLOT(slotOpenROM()));
  connect(ui->downloadButton, SIGNAL(clicked()), this, SLOT(slotDownloadROM()));
  connect(ui->verifyButton, SIGNAL(clicked()), this, SLOT(slotVerifyROM()));
  connect(ui->romBSType, SIGNAL(activated(const QString&)), this, SLOT(slotSetBSType(const QString&)));

  // Quick-select buttons
  myQPGroup = new QButtonGroup(this);
  myQPGroup->setExclusive(false);
  myQPGroup->addButton(ui->qp1Button, 1);   ui->qp1Button->installEventFilter(this);
  myQPGroup->addButton(ui->qp2Button, 2);   ui->qp2Button->installEventFilter(this);
  myQPGroup->addButton(ui->qp3Button, 3);   ui->qp3Button->installEventFilter(this);
  myQPGroup->addButton(ui->qp4Button, 4);   ui->qp4Button->installEventFilter(this);
  myQPGroup->addButton(ui->qp5Button, 5);   ui->qp5Button->installEventFilter(this);
  myQPGroup->addButton(ui->qp6Button, 6);   ui->qp6Button->installEventFilter(this);
  myQPGroup->addButton(ui->qp7Button, 7);   ui->qp7Button->installEventFilter(this);
  myQPGroup->addButton(ui->qp8Button, 8);   ui->qp8Button->installEventFilter(this);
  myQPGroup->addButton(ui->qp9Button, 9);   ui->qp9Button->installEventFilter(this);
  myQPGroup->addButton(ui->qp10Button, 10); ui->qp10Button->installEventFilter(this);
  myQPGroup->addButton(ui->qp11Button, 11); ui->qp11Button->installEventFilter(this);
  myQPGroup->addButton(ui->qp12Button, 12); ui->qp12Button->installEventFilter(this);
  myQPGroup->addButton(ui->qp13Button, 13); ui->qp13Button->installEventFilter(this);
  myQPGroup->addButton(ui->qp14Button, 14); ui->qp14Button->installEventFilter(this);
  myQPGroup->addButton(ui->qp15Button, 15); ui->qp15Button->installEventFilter(this);
  myQPGroup->addButton(ui->qp16Button, 16); ui->qp16Button->installEventFilter(this);
  connect(myQPGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(slotQPButtonClicked(QAbstractButton*)));

  ///////////////////////////////////////////////////////////
  // 'Multicart' tab
  ///////////////////////////////////////////////////////////
  connect(ui->mcartBSType, SIGNAL(activated(int)), this, SLOT(slotSetMCBSType(int)));
  connect(ui->mcartinfoOpenButton, SIGNAL(clicked()), this, SLOT(slotMCOpenInfoFile()));
  connect(ui->mcartinfoNewButton, SIGNAL(clicked()), this, SLOT(slotMCNewInfoFile()));
  connect(ui->mcartinfoSaveButton, SIGNAL(clicked()), this, SLOT(slotMCSaveInfoFile()));
  connect(ui->mcartTable, SIGNAL(cellChanged(int,int)), this, SLOT(slotCheckMCTable(int, int)));
  connect(ui->mcartOpenButton, SIGNAL(clicked()), this, SLOT(slotMCRomnameButton()));
  connect(ui->mcartCreateButton, SIGNAL(clicked()), this, SLOT(slotCreateMulticart()));
  connect(ui->mcartUpButton, SIGNAL(clicked()), this, SLOT(slotMCMoveUp()));
  connect(ui->mcartDownButton, SIGNAL(clicked()), this, SLOT(slotMCMoveDown()));
  connect(ui->mcartLoadDirButton, SIGNAL(clicked()), this, SLOT(slotMCAddFromDir()));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::readSettings()
{
  // Load settings
  QSettings s;

  s.beginGroup("MainWindow");
    myManager.setDefaultPort(s.value("krokport", "").toString().toStdString());
    int retrycount = s.value("retrycount", 0).toInt();
    if(retrycount == 0)       ui->actRetry0->setChecked(true);
    else if(retrycount == 1)  ui->actRetry1->setChecked(true);
    else if(retrycount == 2)  ui->actRetry2->setChecked(true);
    else if(retrycount == 3)  ui->actRetry3->setChecked(true);
    myCart.setRetry(retrycount);
    bool incremental = s.value("incremental", false).toBool();
    ui->actIncDownload->setChecked(incremental);
    myCart.setIncremental(incremental);
    ui->actAutoDownFileSelect->setChecked(s.value("autodownload", false).toBool());
    ui->actAutoVerifyDownload->setChecked(s.value("autoverify", false).toBool());
    ui->mcartTVType->setCurrentIndex(s.value("tvtype", 0).toInt());
  s.endGroup();

  s.beginGroup("QPButtons");
    assignToQPButton(ui->qp1Button, 1, s.value("button1", "").toString(), false);
    assignToQPButton(ui->qp2Button, 2, s.value("button2", "").toString(), false);
    assignToQPButton(ui->qp3Button, 3, s.value("button3", "").toString(), false);
    assignToQPButton(ui->qp4Button, 4, s.value("button4", "").toString(), false);
    assignToQPButton(ui->qp5Button, 5, s.value("button5", "").toString(), false);
    assignToQPButton(ui->qp6Button, 6, s.value("button6", "").toString(), false);
    assignToQPButton(ui->qp7Button, 7, s.value("button7", "").toString(), false);
    assignToQPButton(ui->qp8Button, 8, s.value("button8", "").toString(), false);
    assignToQPButton(ui->qp9Button, 9, s.value("button9", "").toString(), false);
    assignToQPButton(ui->qp10Button, 10, s.value("button10", "").toString(), false);
    assignToQPButton(ui->qp11Button, 11, s.value("button11", "").toString(), false);
    assignToQPButton(ui->qp12Button, 12, s.value("button12", "").toString(), false);
    assignToQPButton(ui->qp13Button, 13, s.value("button13", "").toString(), false);
    assignToQPButton(ui->qp14Button, 14, s.value("button14", "").toString(), false);
    assignToQPButton(ui->qp15Button, 15, s.value("button15", "").toString(), false);
    assignToQPButton(ui->qp16Button, 16, s.value("button16", "").toString(), false);
  s.endGroup();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::closeEvent(QCloseEvent* event)
{
  // Make sure we don't prematurely kill any running threads
  if(myFindKrokThread->isRunning())
  {
    event->ignore();
    return;
  }

  // Save settings
  QSettings s;

  s.beginGroup("MainWindow");
    s.setValue("krokport", QString(myManager.portName().c_str()));
    int retrycount = 0;
    if(ui->actRetry0->isChecked())       retrycount = 0;
    else if(ui->actRetry1->isChecked())  retrycount = 1;
    else if(ui->actRetry2->isChecked())  retrycount = 2;
    else if(ui->actRetry3->isChecked())  retrycount = 3;
    s.setValue("retrycount", retrycount);
    s.setValue("autodownload", ui->actAutoDownFileSelect->isChecked());
    s.setValue("autoverify", ui->actAutoVerifyDownload->isChecked());
    s.setValue("incremental", ui->actIncDownload->isChecked());
    s.setValue("tvtype", ui->mcartTVType->currentIndex());
  s.endGroup();

  event->accept();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool KrokComWindow::eventFilter(QObject* object, QEvent* event)
{
  // This is necessary because braindead QPushButtons don't return right-click events
  if(event->type() == QEvent::ContextMenu)
  {
    int id = 0;
    if(object == ui->qp1Button)       id = 1;
    else if(object == ui->qp2Button)  id = 2;
    else if(object == ui->qp3Button)  id = 3;
    else if(object == ui->qp4Button)  id = 4;
    else if(object == ui->qp5Button)  id = 5;
    else if(object == ui->qp6Button)  id = 6;
    else if(object == ui->qp7Button)  id = 7;
    else if(object == ui->qp8Button)  id = 8;
    else if(object == ui->qp9Button)  id = 9;
    else if(object == ui->qp10Button) id = 10;
    else if(object == ui->qp11Button) id = 11;
    else if(object == ui->qp12Button) id = 12;
    else if(object == ui->qp13Button) id = 13;
    else if(object == ui->qp14Button) id = 14;
    else if(object == ui->qp15Button) id = 15;
    else if(object == ui->qp16Button) id = 16;
    else return false;

    assignToQPButton(static_cast<QPushButton*>(object), id);
    return  true;
  }
  else
  {
    // pass the event on to the parent class
    return QMainWindow::eventFilter(object, event);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotConnectKrokCart()
{
  myStatus->setText("Searching for Krokodile Cart.");
  myLED->setPixmap(QPixmap(":icons/pics/ledoff.png"));

  // Start a thread to do this potentially time-consuming operation
  myFindKrokThread->start();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotUpdateFindKrokStatus()
{
  if(myManager.krokCartAvailable())
  {
    myKrokCartMessage = "\'";
    myKrokCartMessage.append(myManager.versionID().c_str());
    myKrokCartMessage.append("\' @ \'");
    myKrokCartMessage.append(myManager.portName().c_str());
    myKrokCartMessage.append("\'.");
    myLED->setPixmap(QPixmap(":icons/pics/ledon.png"));
  }
  else
  {
    myKrokCartMessage = "Krokodile Cart not found.";
    myLED->setPixmap(QPixmap(":icons/pics/ledoff.png"));
  }
  myStatus->setText(myKrokCartMessage);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotOpenROM()
{
  // Switch to 'ROM' tab
  ui->tabWidget->setCurrentIndex(0);

  QString location = ui->romFileEdit->text() != "" ?
    QFileInfo(ui->romFileEdit->text()).absolutePath() :
    myLastDir.absolutePath();
  QString file = QFileDialog::getOpenFileName(this,
    tr("Select ROM Image"), location, tr("Atari 2600 ROM Image (*.a26 *.bin *.rom)"));

  if(!file.isNull())
    loadROM(file);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotDownloadROM()
{
  if(myDownloadInProgress)
    return;

  ui->verifyButton->setDisabled(true);  ui->actVerifyROM->setDisabled(true);

  if(!myManager.krokCartAvailable())
  {
    myDownloadInProgress = false;
    myStatus->setText("Krokodile Cart not found.");
    return;
  }
  else if(!myCart.isValid())
  {
    myDownloadInProgress = false;
    statusMessage("Invalid cartridge.");
    return;
  }

  // Switch to 'ROM' tab
  ui->tabWidget->setCurrentIndex(0);
  myDownloadInProgress = true;

  // Write to serial port
  uInt16 sector = 0, numSectors = myCart.initSectors(true);
  QProgressDialog progress("Downloading ROM...", QString(), 0, numSectors, this);
  progress.setWindowIcon(QPixmap(":icons/pics/appicon.png"));
  progress.setWindowModality(Qt::WindowModal);
  progress.setMinimumDuration(0);
  progress.setValue(0);
  try
  {
    for(sector = 0; sector < numSectors; ++sector)
    {
      myCart.writeNextSector(myManager.port());
      progress.setValue(sector);
    }
  }
  catch(const char* msg)
  {
    cout << msg << std::endl;
  }
  myDownloadInProgress = false;

  progress.setValue(numSectors);
  if(myCart.finalizeSectors())
  {
    statusMessage(QString(myCart.message().c_str()));

    ui->verifyButton->setDisabled(false);  ui->actVerifyROM->setDisabled(false);

    // See if we should automatically verify the download
    if(ui->actAutoVerifyDownload->isChecked())
      slotVerifyROM();
  }
  else
    statusMessage(QString(myCart.message().c_str()));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotVerifyROM()
{
  if(myDownloadInProgress)
    return;

  if(!myManager.krokCartAvailable())
  {
    myDownloadInProgress = false;
    myStatus->setText("Krokodile Cart not found.");
    return;
  }
  else if(!myCart.isValid())
  {
    myDownloadInProgress = false;
    statusMessage("Invalid cartridge.");
    return;
  }

  // Switch to 'ROM' tab
  ui->tabWidget->setCurrentIndex(0);
  myDownloadInProgress = true;

  // Verify data previously written to serial port
  uInt16 sector = 0, numSectors = myCart.initSectors(false);
  QProgressDialog progress("Verifying ROM...", QString(), 0, numSectors, this);
  progress.setWindowIcon(QPixmap(":icons/pics/appicon.png"));
  progress.setWindowModality(Qt::WindowModal);
  progress.setMinimumDuration(0);
  progress.setValue(0);
  try
  {
    for(sector = 0; sector < numSectors; ++sector)
    {
      myCart.verifyNextSector(myManager.port());
      progress.setValue(sector);
    }
  }
  catch(const char* msg)
  {
    cout << msg << std::endl;
  }

  if(sector == numSectors)
  {
    progress.setValue(numSectors);
    statusMessage("Verified download of " + QString::number(sector) + " sectors.");
  }
  else
    statusMessage("Verify failure on sector " + QString::number(sector) + ".");

  myDownloadInProgress = false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotEnableIncDownload(bool enable)
{
  ui->actIncDownload->setChecked(enable);
  myCart.setIncremental(enable);
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
//FIXME cerr << "Sure you want to override type (yes/no/always)?\n";
      myCart.setBSType(selectedType);
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
        << "<p><b>Krokodile Commander for UNIX v" << KROK_VERSION << "</b></p>"
        << "<p>Copyright &copy; 2009-2025 <a href=\"mailto:sa666666@gmail.com\">Stephen Anthony</a><br>"
        << "Check for updates at <a href=\"https://github.com/sa666666/krokcom\">https://github.com/sa666666/krokcom</a><p>"
        << "<p>Based on the original <a href=\"http://www.arminvogl.de/KrokodileCartridge\">Windows version</a><br>"
        << "Copyright &copy; 2002-2009 <a href=\"mailto:Armin.Vogl@gmx.net\">Armin Vogl</a></p>"
        << "</center>"
        << "<p>This software is released under the GNU GPLv3, and includes items from the following projects:</p>"
        << "<ul>"
        << "<li>JKrokcom: Preliminary Java port of KrokCom</li>"
        << "<li><a href=\"http://harmony.atariage.com\">HarmonyCart</a>: UI code, icons and other images</li>"
        << "<li><a href=\"https://stella-emu.github.io\">Stella</a>: bankswitch autodetection code</li>"
        << "</ul>"
        << "<p>Special thanks go to the following people:"
        << "<p><ul>"
        << "<li>Jeff Johnson: Provided extended loan of a Krokodile Cart, allowing this software to be written</li>"
        << "<li>Joe Chiarelli: Monetary contribution in appreciation for the OSX port</li>"
        << "<li>Darrell Spice Jr: Provided preliminary C-based commandline implementation of KrokCom</li>"
        << "<li>Nathan Strum: Provided icon for KrokCom"
        << "<li>Armin Vogl: Author of the Windows version of KrokCom, and designer of the UI (I basically reused the UI from KrokCom for Windows)</li>"
        << "<li>Chris Walton: Provided preliminary Java-based commandline implementation of JKrokCom</li>"
        << "</ul></p>"

        << "<p>Version 1.3 (Dec. xx, 2025):</p>"
        << "<ul>"
        << "<li>Ported application to Qt5.</li>"
        << "</ul>"

        << "<p>Version 1.2 (Dec. 25, 2012):</p>"
        << "<ul>"
        << "<li>Fixed bugs in user interface (cut off text, progress bar not always appearing, etc).</li>"
        << "<li>Updated bankswitch autodetection code to latest from Stella 3.7.5.</li>"
        << "</ul>"

        << "<p>Version 1.1 (Dec. 6, 2009):</p>"
        << "<ul>"
        << "<li>Added a menu item for crediting people responsible for helping me "
        << "in releasing this software.</li>"
        << "<li>Native support for OSX Snow Leopard by including Qt 4.6 (the first "
        << "version to actually support OSX 10.6).</li>"
        << "</ul>"

        << "<p>Version 1.0 (Nov. 15, 2009):</p>"
        << "<ul>"
        << "<li>Initial release for Linux/UNIX and Mac OSX.</li>"
        << "</ul>"
        ;

  AboutDialog aboutdlg(this, "Info about Krokodile Commander for UNIX", about.str().c_str());
  aboutdlg.exec();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotQPButtonClicked(QAbstractButton* b)
{
  // Get the full path from the settings
  QPushButton* button = static_cast<QPushButton*>(b);
  int id = myQPGroup->id(b);
  QString key = "button" + QString::number(id);
  QSettings s;
  s.beginGroup("QPButtons");
    QString file = s.value(key, "").toString();
  s.endGroup();

  QFileInfo info(file);
  if(info.exists())
    loadROM(file);
  else if(file != "")
  {
    if(QMessageBox::Yes == QMessageBox::warning(this, "Warning",
      "This ROM no longer exists.  Do you wish to remove it\nfrom the QuickPick list?",
      QMessageBox::Yes, QMessageBox::No))
    {
      button->setText("");
      s.beginGroup("QPButtons");
        s.remove(key);
      s.endGroup();
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::loadROM(const QString& file, bool showmessage)
{
  if(file == "")
    return;

  ui->romBSType->setDisabled(true);
  ui->downloadButton->setDisabled(true);  ui->actDownloadROM->setDisabled(true);
  ui->verifyButton->setDisabled(true);  ui->actVerifyROM->setDisabled(true);

  // Create a cart from the given filename
  myCart.create(file.toStdString());
  if(showmessage) statusMessage(QString(myCart.message().c_str()));

  if(myCart.isValid())
  {
    ui->romFileEdit->setText(file);
    ui->romSizeLabel->setText(QString::number(myCart.getSize()) + " bytes");
    myDetectedBSType = myCart.getBSType();
    QString bstype = Bankswitch::typeToName(myDetectedBSType).c_str();
    int match = ui->romBSType->findText(bstype, Qt::MatchStartsWith);
    ui->romBSType->setCurrentIndex(match < ui->romBSType->count() && match >= 0 ? match : 0);
    ui->romBSType->setDisabled(false);
    ui->downloadButton->setDisabled(false);  ui->actDownloadROM->setDisabled(false);

    // See if we should automatically download
    if(ui->actAutoDownFileSelect->isChecked())
      slotDownloadROM();
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::assignToQPButton(QPushButton* button, int id)
{
  QString file = QFileDialog::getOpenFileName(this,
    tr("Select ROM Image"), myLastDir.absolutePath(), tr("Atari 2600 ROM Image (*.a26 *.bin *.rom)"));

  if(!file.isNull())
  {
    assignToQPButton(button, id, file, true);
    // Remember this location for the next time a file is selected
    myLastDir.setPath(file);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::assignToQPButton(QPushButton* button, int id,
                                     const QString& file, bool save)
{
  QFileInfo info(file);

  // Only add files that exist
  QString filename = info.fileName();
  if(filename == "")
    return;

  // Otherwise, add the file itself to the button, and the full path to settings
  button->setText(filename);

  if(save)
  {
    QString key = "button" + QString::number(id);
    QSettings s;
    s.beginGroup("QPButtons");
      s.setValue(key, info.canonicalFilePath());
    s.endGroup();
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::statusMessage(const QString& msg)
{
  // Show the message for a short time, then reset to the default message
  myStatus->setText(msg);
  QTimer::singleShot(4000, this, SLOT(slotShowDefaultMsg()));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotShowDefaultMsg()
{
  myStatus->setText(myKrokCartMessage);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotSetMCBSType(int id)
{
  // Change number of rows in multicart table based on id
  // Only do so if the # of rows has actually changed
  QString romname = "";
  int rows = 0, count = countMCEntries();
  switch(id)
  {
    case 0:  rows = 127;  romname = "/MC4K.a26";  break;  // 4K
    case 1:  rows = 63;   romname = "/MCF8.a26";  break;  // 8K
    case 2:  rows = 31;   romname = "/MCF6.a26";  break;  // 16K
    case 3:  rows = 15;   romname = "/MCF4.a26";  break;  // 32K
    default: return;
  }
  if(rows == ui->mcartTable->rowCount() && count > 0)
    return;

  // Warn if all the entries are about to be deleted
  if(count > 0)
  {
    if(QMessageBox::Yes != QMessageBox::warning(this, "Warning",
      "Changing the multicart type will delete all file selections.\nDo you really want to change the multicart type?",
      QMessageBox::Yes, QMessageBox::No))
    {
      // Set the type back to the previous value
      switch(ui->mcartTable->rowCount())
      {
        case 127:  ui->mcartBSType->setCurrentIndex(0);  break;  // 4K
        case 63:   ui->mcartBSType->setCurrentIndex(1);  break;  // 8K
        case 31:   ui->mcartBSType->setCurrentIndex(2);  break;  // 16K
        case 15:   ui->mcartBSType->setCurrentIndex(3);  break;  // 32K
      }
      return;
    }
  }

  clearMCContents(rows);

  // Set a default multicart name
  QString path = QDir::homePath() + romname;
  ui->mcartFileEdit->setText(QDir(path).absolutePath());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotMCOpenInfoFile()
{
  QString file = QFileDialog::getOpenFileName(this,
    "Select Multicart Info File", myLastDir.absolutePath(), "Multicart Info File (*.mul)");
  if(file.isNull())
    return;

  // Remember this location for the next time a file is selected
  QFileInfo info(file);

  QSettings s(info.absoluteFilePath(), QSettings::IniFormat);
  s.beginGroup("Images");
    ui->mcartBSType->setCurrentIndex(s.value("Type", 0).toInt());
    ui->mcartTVType->setCurrentIndex(s.value("TVType", 0).toInt());
    ui->mcartFileEdit->setText(s.value("BinPath", "").toString());

    int rows = 0, count = 0;
    switch(ui->mcartBSType->currentIndex())
    {
      case 0:  rows = 127;  break;  // 4K
      case 1:  rows = 63;   break;  // 8K
      case 2:  rows = 31;   break;  // 16K
      case 3:  rows = 15;   break;  // 32K
    }
    clearMCContents(rows);
    QString key, menuname, filename;
    for(int i = 1; i <= rows; ++i)
    {
      key.asprintf("Menu%03d", i);  menuname = s.value(key, "").toString();
      key.asprintf("File%03d", i);  filename = s.value(key, "").toString();
      setMCTableEntry(i-1, menuname, filename);
      if(menuname != "" && filename != "")
        ++count;
    }
  s.endGroup();

  // Add the file once we're sure it contains valid data
  myLastDir.setPath(info.absolutePath());
  ui->mcartinfoFileEdit->setText(file);
  statusMessage("Loaded " + QString::number(count) + " entries from multicart info file.");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotMCNewInfoFile()
{
  if(countMCEntries() > 0 && QMessageBox::Yes == QMessageBox::warning(this, "Warning",
     "Do you really want to create a new multicart info file?\nAll current selections will be lost!",
     QMessageBox::Yes, QMessageBox::No))
  {
    QString file = QFileDialog::getSaveFileName(this,
      "Select Multicart Info File", myLastDir.absolutePath(), "Multicart Info File (*.mul)");
    ui->mcartinfoFileEdit->setText(!file.isNull() ? file : "");

    int rows = 0;
    switch(ui->mcartBSType->currentIndex())
    {
      case 0:  rows = 127;  break;  // 4K
      case 1:  rows = 63;   break;  // 8K
      case 2:  rows = 31;   break;  // 16K
      case 3:  rows = 15;   break;  // 32K
      default: return;
    }
    clearMCContents(rows);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotMCSaveInfoFile()
{
  if(ui->mcartinfoFileEdit->text() == "")
  {
    QMessageBox::critical(this, "Missing Info File",
      "Select a filename for the Multicart Info File.");
    return;
  }
  else if(countMCEntries() == 0)
  {
    QMessageBox::critical(this, "Missing Entries",
      "You must add some entries before you can\nsave a Multicart Info File.");
    return;
  }

  QSettings s(QFileInfo(ui->mcartinfoFileEdit->text()).absoluteFilePath(), QSettings::IniFormat);
  if(!s.isWritable())
  {
    QMessageBox::critical(this, "Invalid Info File",
      "The Multicart Info filename you've\nchosen cannot be used.");
    return;
  }

  s.beginGroup("Images");
    s.setValue("Type", ui->mcartBSType->currentIndex());
    s.setValue("TVType", ui->mcartTVType->currentIndex());
    s.setValue("BinPath", ui->mcartFileEdit->text());

    int count = 0;
    QString menukey, filekey, menuname, filename;
    for(int i = 1; i <= ui->mcartTable->rowCount(); ++i)
    {
      menukey.asprintf("Menu%03d", i);
      filekey.asprintf("File%03d", i);
      getMCTableEntry(i-1, menuname, filename);
      s.setValue(menukey, menuname);
      s.setValue(filekey, filename);
      if(menuname != "" && filename != "")
        ++count;
    }
  s.endGroup();
  statusMessage("Saved " + QString::number(count) + " entries to multicart info file.");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotMCAddRomButtonClicked(int row)
{
  // Check if this ROM can be used for the currently selected multicart type
  int selected = ui->mcartBSType->currentIndex();
  if(selected == -1)
    return;
  BSType desired = BS_4K;
  QString size = "";
  switch(selected)
  {
    case 0:  desired = BS_4K;  size = "1/2/4K";  break;
    case 1:  desired = BS_F8;  size = "1/2/4K/8K (F8)";  break;
    case 2:  desired = BS_F6;  size = "1/2/4K/16K (F6)";  break;
    case 3:  desired = BS_F4;  size = "1/2/4K/32K (F4)";  break;
  }

  QString file = QFileDialog::getOpenFileName(this,
    "Select \'" + size + "\' ROM Image", myLastDir.absolutePath(), "Atari 2600 ROM Image (*.a26 *.bin *.rom)");

  if(file.isNull())
    return;

  // Remember this location for the next time a file is selected
  myLastDir.setPath(file);
  QFileInfo info(file);

  // All types can also store 4K images, as well as those specific
  // to the type
  BSType actual = CartDetector::autodetectType(info.absoluteFilePath().toStdString());
  if(!(actual == desired || actual == BS_4K))
  {
    QMessageBox::critical(this, "Invalid ROM",
      "Only \'" + size + "\' images are supported by\nthe selected multicart type.\nThe selected ROM was detected as \'" +
      QString(Bankswitch::typeToName(actual).c_str()) + "\'.");
    return;
  }

  // Add the entry
  setMCTableEntry(row, info.completeBaseName().toUpper().left(13), info.absoluteFilePath());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotMCDeleteRomButtonClicked(int row)
{
  setMCTableEntry(row, "", "");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotCheckMCTable(int row, int col)
{
  if(col != 0)
    return;

  // Make sure menu name entries are uppercase and max 13 characters
  QTableWidgetItem* menuItem = ui->mcartTable->item(row, col);
  if(menuItem)
  {
    QString menuName = menuItem->data(Qt::DisplayRole).toString().toUpper().left(13);
    menuItem->setData(Qt::DisplayRole, menuName);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotMCRomnameButton()
{
  QString location = ui->mcartFileEdit->text() != "" ?
    QFileInfo(ui->mcartFileEdit->text()).absolutePath() :
    myLastDir.absolutePath();
  QString file = QFileDialog::getOpenFileName(this,
    "Select Multicart ROM Image", location, "Atari 2600 ROM Image (*.a26 *.bin *.rom)");

  if(!file.isNull())
  {
    ui->mcartFileEdit->setText(file);
    // Remember this location for the next time a file is selected
    myLastDir.setPath(file);
  }

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotMCMoveUp()
{
  swapMCEntry(+1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotMCMoveDown()
{
  swapMCEntry(-1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::swapMCEntry(int offset)
{
  // Determine the two positions to swap
  int current = ui->mcartTable->currentRow();
  if(current < 0 || current >= ui->mcartTable->rowCount())
    return;
  offset = current - offset;
  if(offset < 0 || offset >= ui->mcartTable->rowCount())
    return;

  QString currentMenuName, currentFileName;
  getMCTableEntry(current, currentMenuName, currentFileName);

  QString offsetMenuName, offsetFileName;
  getMCTableEntry(offset, offsetMenuName, offsetFileName);

  // Swap the name strings themselves
  QString tmp = currentMenuName;
  currentMenuName = offsetMenuName;
  offsetMenuName = tmp;

  tmp = currentFileName;
  currentFileName = offsetFileName;
  offsetFileName = tmp;

  // Now swap the actual cell contents
  setMCTableEntry(current, currentMenuName, currentFileName);
  setMCTableEntry(offset, offsetMenuName, offsetFileName);

  // Select the new row
  ui->mcartTable->setCurrentCell(offset, ui->mcartTable->currentColumn());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotMCAddFromDir()
{
  int selected = ui->mcartBSType->currentIndex();
  BSType bstype = BS_NONE;
  switch(selected)
  {
    case 0:  bstype = BS_4K;  break;
    case 1:  bstype = BS_F8;  break;
    case 2:  bstype = BS_F6;  break;
    case 3:  bstype = BS_F4;  break;
    default: return;
  }

  // Adding a directory will erase all current entries; do we want that?
  if(countMCEntries() > 0)
  {
    if(QMessageBox::Yes != QMessageBox::warning(this, "Warning",
      "Loading ROMs from a folder will delete all file selections.\nDo you really want to continue?",
      QMessageBox::Yes, QMessageBox::No))
      return;
  }

  // Get a folder, and parse its contents according to the desired bankswitch type
  QString path = QFileDialog::getExistingDirectory(this,
    "Browse for \'" + QString(Bankswitch::typeToName(bstype).c_str()) +  "\' ROM folder",
    myLastDir.absolutePath(), QFileDialog::ShowDirsOnly);
  if(path.isNull())
    return;

  QStringList menuNames, fileNames;
  int count = getRomsFromFolder(path, menuNames, fileNames, bstype, ui->mcartTable->rowCount());

  // Add ROMs to list
  clearMCContents(ui->mcartTable->rowCount());
  for(int i = 0; i < count; ++i)
    setMCTableEntry(i, menuNames.at(i), fileNames.at(i));

  statusMessage("Added " + QString::number(count) + " \'" +
                QString(Bankswitch::typeToName(bstype).c_str()) + "\' roms.");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int KrokComWindow::countMCEntries() const
{
  int size = 0;
  QString menuName, fileName;
  for(int row = 0; row < ui->mcartTable->rowCount(); ++row)
  {
    // Check for inconsistent data
    getMCTableEntry(row, menuName, fileName);
    if(menuName != "" && fileName != "")
      ++size;
  }
  return size;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::setMCTableEntry(int row, const QString& menuName, const QString& fileName)
{
  QTableWidgetItem* menuItem = ui->mcartTable->item(row, 0);
  if(menuItem) menuItem->setData(Qt::DisplayRole, menuName);
  else         ui->mcartTable->setItem(row, 0, new QTableWidgetItem(menuName));

  QTableWidgetItem* fileItem = ui->mcartTable->item(row, 1);
  if(fileItem) fileItem->setData(Qt::DisplayRole, fileName);
  else         ui->mcartTable->setItem(row, 1, new QTableWidgetItem(fileName));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::getMCTableEntry(int row, QString& menuName, QString& fileName) const
{
  QTableWidgetItem* menuItem = ui->mcartTable->item(row, 0);
  menuName = menuItem ? menuItem->data(Qt::DisplayRole).toString() : "";

  QTableWidgetItem* fileItem = ui->mcartTable->item(row, 1);
  fileName = fileItem ? fileItem->data(Qt::DisplayRole).toString() : "";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::clearMCContents(int rows)
{
  ui->mcartTable->setRowCount(127);
  ui->mcartTable->clearContents();

  // Add 'open' and 'trash' buttons to each row
  QButtonGroup* mcartOpenGroup = new QButtonGroup(this);
  mcartOpenGroup->setExclusive(false);
  QButtonGroup* mcartDeleteGroup = new QButtonGroup(this);
  mcartDeleteGroup->setExclusive(false);
  for(int i = 0; i < 127; ++i)
  {
    QPushButton* ins = new QPushButton("...");
    ui->mcartTable->setCellWidget(i, 2, ins);
    mcartOpenGroup->addButton(ins, i);

    QPushButton* del = new QPushButton(QIcon(":icons/pics/delete.png"), "");
    ui->mcartTable->setCellWidget(i, 3, del);
    mcartDeleteGroup->addButton(del, i);
  }
  connect(mcartOpenGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotMCAddRomButtonClicked(int)));
  connect(mcartDeleteGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotMCDeleteRomButtonClicked(int)));

  ui->mcartTable->setRowCount(rows);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int KrokComWindow::getRomsFromFolder(QString folder, QStringList& menuNames,
                                     QStringList& fileNames, BSType type,
                                     int maxEntries)
{
  QDir dir(folder, "*.a26 *.bin *.rom", QDir::Name|QDir::IgnoreCase, QDir::Files);
  QFileInfoList files = dir.entryInfoList();

  int i = 0;
  while(i < files.size() && fileNames.size() < maxEntries)
  {
    BSType detected = CartDetector::autodetectType(files.at(i).absoluteFilePath().toStdString());
    // Valid ROM, add to list
    if(detected == type || detected == BS_4K)
    {
      fileNames.push_back(files.at(i).absoluteFilePath());
      menuNames.push_back(files.at(i).completeBaseName().toUpper().left(13));
    }
    ++i;
  }
  return fileNames.size();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KrokComWindow::slotCreateMulticart()
{
  StringList menuNames, fileNames;
  int missingMenu = 0, missingFile = 0;

  ///////////////////////////////////////////////////////////////////
  // ERROR checking ...
  // As much as possible, we want to check for errors at this level,
  // so the actual Cart class won't have to worry about it so much.
  ///////////////////////////////////////////////////////////////////

  // Scan the table, checking for missing menu and/or filenames
  // Otherwise, add valid entries to a list
  QString menuName, fileName;
  for(int row = 0; row < ui->mcartTable->rowCount(); ++row)
  {
    getMCTableEntry(row, menuName, fileName);

    // Check for inconsistent data
    if(menuName == "" && fileName == "")
      continue;
    else if(menuName == "" && fileName != "")
      ++missingMenu;
    else if(menuName != "" && fileName == "")
      ++missingFile;
    else
    {
      // Valid entry
      menuNames.push_back(menuName.toStdString());
      fileNames.push_back(fileName.toStdString());
    }
  }

  // Do we continue with invalid data?
  if(missingMenu > 0 || missingFile > 0)
  {
    QString message = "Multicart entries are inconsistent.\n";
    if(missingMenu) message += "Missing menu names: " + QString::number(missingMenu) + "\n";
    if(missingFile) message += "Missing file names: " + QString::number(missingFile) + "\n";
    message += "You must fix these entries before proceeding.";

    QMessageBox::critical(this, "Invalid Entries", message);
    return;
  }
  else if(menuNames.size() == 0)
  {
    QMessageBox::critical(this, "Invalid Entries", "You must add at least one ROM image.");
    return;
  }

  // See if a valid multicart name has been chosen
  QFileInfo info(ui->mcartFileEdit->text());
  if(ui->mcartFileEdit->text() == "")
  {
    QMessageBox::critical(this, "File error",
      QString("An invalid name has been chosen for your multicart rom.\n") +
      QString("Make sure you specify a name, and that you have\n") +
      QString("permission to save a file in that location."));
    return;
  }

  // Check if this ROM can be used for the currently selected multicart type
  int selected = ui->mcartBSType->currentIndex();
  BSType bstype = BS_NONE;
  switch(selected)
  {
    case 0:  bstype = BS_MC4K;  break;
    case 1:  bstype = BS_MCF8;  break;
    case 2:  bstype = BS_MCF6;  break;
    case 3:  bstype = BS_MCF4;  break;
  }
  bool ntsc = ui->mcartTVType->currentIndex() == 0;

  // Create a cart from the given data
  myCart.createMultiFile(menuNames, fileNames, bstype, ntsc,
                         ui->mcartFileEdit->text().toStdString());

  statusMessage(QString(myCart.message().c_str()));

  loadROM(ui->mcartFileEdit->text(), false);
}
