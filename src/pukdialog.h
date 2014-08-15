#ifndef PUK_DIALOG_H
#define PUK_DIALOG_H


#include "ui_pukdialog.h"
#include "cardcontrolhandler.h"

#define DEFAULT_TEXT_INSERT_PUK "Please enter PUK:"

class PukDialog : public QDialog {
  Q_OBJECT
  public:
    PukDialog(QWidget *parent);
    ~PukDialog();

    QString getPuk() { return _ui.pukLineEdit->text(); }
    int exec(const CardControlHandler::Pkcs15PinInfo& pinInfo);
  private:
    Ui_PukDialog _ui;
};

#endif
