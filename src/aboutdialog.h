#ifndef ABOUT_DIALOG_H
#define ABOUT_DIALOG_H


#include "ui_aboutdialog.h"
#include "cardcontrolhandler.h"


class AboutDialog : public QDialog {
  Q_OBJECT
  public:
    AboutDialog(QWidget *parent);
    ~AboutDialog();

  private:
    Ui_AboutDialog _ui;
    
  public slots:    
    void creditsButtonPressed();
    void licenseButtonPressed();
  
};

#endif
