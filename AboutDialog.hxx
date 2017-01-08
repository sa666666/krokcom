//===========================================================================
//
//  K   K  RRRR    OOO   K   K   CCCC   OOO   M   M
//  K  K   R   R  O   O  K  K   C      O   O  MM MM
//  KKK    RRRR   O   O  KKK    C      O   O  M M M  "Krokodile Cart software"
//  K  K   R R    O   O  K  K   C      O   O  M   M
//  K   K  R  R    OOO   K   K   CCCC   OOO   M   M
//
// Copyright (c) 2009-2017 by Stephen Anthony <sa666666@gmail.com>
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#ifndef ABOUTDIALOG_HXX
#define ABOUTDIALOG_HXX

#include <QtWidgets/QDialog>

namespace Ui {
  class AboutDialog;
}

class AboutDialog : public QDialog
{
Q_OBJECT
  public:
    AboutDialog(QWidget* parent, const QString& title, const QString& info);
    ~AboutDialog();

  private:
    Ui::AboutDialog* m_ui;
};

#endif // ABOUTDIALOG_HXX
