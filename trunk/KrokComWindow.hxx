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

#ifndef __KROKCOM_WINDOW_HXX
#define __KROKCOM_WINDOW_HXX

#include <QMainWindow>
#include <QAction>
#include <QLabel>
#include <QProgressBar>
#include <QDir>

#include "Cart.hxx"
#include "SerialPortManager.hxx"
#include "FindKrokThread.hxx"
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

  public:
    SerialPortManager& portManager() { return myManager; }
    void connectKrokCart() { slotConnectKrokCart(); }

  protected:
    void closeEvent(QCloseEvent* event);
    bool eventFilter(QObject* object, QEvent* event);

  private:
    void setupConnections();
    void readSettings();
    void loadROM(const QString& file, bool showmessage = true);
    void assignToQPButton(QPushButton* button, int id);
    void assignToQPButton(QPushButton* button, int id, const QString& file, bool save);
    void swapMCEntry(int direction);
    int  countMCEntries() const;
    void setMCTableEntry(int row, const QString& menuName, const QString& fileName);
    void getMCTableEntry(int row, QString& menuName, QString& fileName) const;
    void clearMCContents(int rows);
    static int getRomsFromFolder(QString folder, QStringList& menuNames, QStringList& fileNames,
                                 BSType type, int maxEntries);

    void statusMessage(const QString& msg);

  private slots:
    void slotConnectKrokCart();
    void slotUpdateFindKrokStatus();

    void slotOpenROM();
    void slotDownloadROM();
    void slotVerifyROM();
    void slotEnableIncDownload(bool);
    void slotRetry(QAction*);
    void slotSetBSType(const QString&);
    void slotAbout();
    void slotQPButtonClicked(QAbstractButton* b);
    void slotShowDefaultMsg();

    void slotSetMCBSType(int id);
    void slotMCOpenInfoFile();
    void slotMCNewInfoFile();
    void slotMCSaveInfoFile();
    void slotMCAddRomButtonClicked(int row);
    void slotMCDeleteRomButtonClicked(int row);
    void slotCheckMCTable(int row, int col);
    void slotMCRomnameButton();
    void slotMCMoveUp();
    void slotMCMoveDown();
    void slotMCAddFromDir();

    void slotCreateMulticart();

  private:
    Ui::KrokComWindow* ui;
    FindKrokThread* myFindKrokThread;
    QButtonGroup* myQPGroup;

    Cart myCart;
    SerialPortManager myManager;
    BSType myDetectedBSType;

    QLabel* myStatus;
    QLabel* myLED;
    QProgressBar* myProgress;
    QDir myLastDir;

    QString myKrokCartMessage;
    bool myDownloadInProgress;
};

#endif
