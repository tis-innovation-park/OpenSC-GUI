#include "aboutdialog.h"


AboutDialog::AboutDialog(QWidget* parent):QDialog(parent){
  _ui.setupUi( this );
  setWindowTitle( tr("About") );
  _ui.label->setText( _ui.label->text() + " " BUERGERKARTE_SW_VERSION );
  _ui.creditsButton->setText( tr("&Credits") );
  _ui.licenseButton->setText( tr("&License") );
  
  connect(_ui.creditsButton, SIGNAL(clicked()), this, SLOT(creditsButtonPressed()) );
  connect(_ui.licenseButton, SIGNAL(clicked()), this, SLOT(licenseButtonPressed()) );
  
  _ui.creditTextEdit->hide();
  _ui.licenseTextEdit->hide();
}


AboutDialog::~AboutDialog(){
  
}



////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////  private  /////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////  SLOTS  /////////////////////////////////////////////




void AboutDialog::creditsButtonPressed(){
  if ( _ui.creditTextEdit->isVisible() ) 
    _ui.creditTextEdit->hide();
  else
    _ui.creditTextEdit->show();
  _ui.licenseTextEdit->hide();
  
  _ui.aboutLabel->setVisible( !_ui.creditTextEdit->isVisible() );
}


void AboutDialog::licenseButtonPressed(){
  if ( _ui.licenseTextEdit->isVisible() ) 
    _ui.licenseTextEdit->hide();
  else
    _ui.licenseTextEdit->show();
  _ui.creditTextEdit->hide();
  
  _ui.aboutLabel->setVisible( !_ui.licenseTextEdit->isVisible() );
}
