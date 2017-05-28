#include "pukdialog.h"


PukDialog::PukDialog(QWidget* parent):QDialog(parent){
  _ui.setupUi( this );
  _ui.pukLabel->setText( tr(DEFAULT_TEXT_INSERT_PUK) );
  setWindowTitle( tr("Insert PUK") );
}


PukDialog::~PukDialog(){
  
}


int PukDialog::exec(const CardControlHandler::Pkcs15PinInfo& pinInfo) {
  _ui.pukLineEdit->clear();
  QString pukLengthTxt = pinInfo.isValid ? tr("[Min:%1  Max:%2]").arg(pinInfo.minLength).arg(pinInfo.maxLength) : "";
  _ui.pukLabel->setText( tr(DEFAULT_TEXT_INSERT_PUK) + pukLengthTxt );
  return QDialog::exec();
}

////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////  private  /////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////  SLOTS  /////////////////////////////////////////////




