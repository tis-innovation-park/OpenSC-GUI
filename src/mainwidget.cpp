#include "mainwidget.h"


MainWidget::MainWidget(QWidget *parent) : QMainWindow(parent) {
  _ui.setupUi( this );
  _ui.passwordWidget->setEnabled( false );
  _ui.oldPinLabel->setText( tr(DEFAULT_TEXT_INSERT_CURRENT_PIN) );
  _ui.newPinLabel->setText( tr(DEFAULT_TEXT_INSERT_NEW_PIN) );
  _ui.confirmPinLabel->setText( tr(DEFAULT_TEXT_CONFIRM_NEW_PIN) );
  _ui.changePasswordButton->setText(tr(DEFAULT_TEXT_CHANGE_PIN_BUTTON));
  _ui.changePasswordButton->setEnabled( false );  
  
  _aboutDialog = new AboutDialog(this);
  _aboutDialog->close();
  
  QPalette palette = _ui.newPinInfoLabel->palette();
  palette.setColor(_ui.newPinInfoLabel->foregroundRole(), Qt::red);
  _ui.newPinInfoLabel->setPalette( palette );
  palette = _ui.currentPinInfoLabel->palette();
  palette.setColor(_ui.currentPinInfoLabel->foregroundRole(), Qt::red);
  _ui.currentPinInfoLabel->setPalette( palette );
  palette = _ui.confirmPinInfoLabel->palette();
  palette.setColor(_ui.confirmPinInfoLabel->foregroundRole(), Qt::red);
  _ui.confirmPinInfoLabel->setPalette( palette );
  
  setWindowTitle(tr("CNS Card Control"));
  setWindowIcon( QIcon(":/icons/provinz_wappen.png") );
  
  _textEdit = 0;
  _pinBlocked = false;
  memset( &_pinInfo, 0, sizeof(CardControlHandler::Pkcs15PinInfo));
  memset( &_personalData, 0, sizeof(CardControlHandler::PersonalData));
  memset( &_serialData, 0, sizeof(CardControlHandler::SerialData));
  memset( &_x509Data, 0 , sizeof(X509CertificateHandler::X509CertificateData) );
  
  
  updateCardInformation();
  updateX509CertificateInformation();

  _textEdit = new QTextEdit();
  logger().setLogWidget( _textEdit );
  _textEdit->hide();
  _textEdit->setWindowTitle(tr("Buergerkarte debug window") );
  
  setupTrayIcon();
  _trayIcon->show();
  
  _scThread = new QThread();
  _scControl = new CardControlHandler();
  _statusBar = new StatusBar( _scControl );
  this->setStatusBar( _statusBar );
  
  connect( _scControl, SIGNAL(cardReaderConnected(sc_reader_t)), this, SLOT(cardReaderWasConnected(sc_reader_t)), Qt::QueuedConnection);
  connect( _scControl, SIGNAL(smartCardConnected(sc_card_t)), this, SLOT(smartCardWasConnected(sc_card_t)), Qt::QueuedConnection);
  connect( _scControl, SIGNAL(cardReaderRemoved(sc_reader_t)), this, SLOT(cardReaderWasRemoved()), Qt::QueuedConnection);
  connect( _scControl, SIGNAL(smartCardRemoved(sc_card_t)), this, SLOT(smartCardWasRemoved()), Qt::QueuedConnection);
  connect( _scControl, SIGNAL(pksc15PinChangeDone(Error)), this, SLOT(pksc15PinChangeDone(Error)), Qt::QueuedConnection);
  connect( _scControl, SIGNAL(pksc15PinUnblockDone(Error)), this, SLOT(pksc15PinUnblockDone(Error)), Qt::QueuedConnection);
  connect( _ui.changePasswordButton, SIGNAL(clicked(bool)), this, SLOT(changePasswordButtonClicked()));
  connect( _scControl, SIGNAL(pkcs15PinInfoGathered(sc_pkcs15_auth_info_t)), this, SLOT(pkcs15PinInfoWasGathered(sc_pkcs15_auth_info_t)), Qt::QueuedConnection);
  connect( _scControl, SIGNAL(personalDataGathered(CardControlHandler::PersonalData)), this , SLOT(personalDataGathered(CardControlHandler::PersonalData)), Qt::QueuedConnection );
  connect( _scControl, SIGNAL(serialDataGathered(CardControlHandler::SerialData)), this , SLOT(serialDataGathered(CardControlHandler::SerialData)), Qt::QueuedConnection );
  connect( _scControl, SIGNAL(x509CertificateDataGathered(X509CertificateHandler::X509CertificateData)), this , SLOT(x509CertificateDataGathered(X509CertificateHandler::X509CertificateData)), Qt::QueuedConnection );

  
  connect( _ui.currentPasswordLineEdit, SIGNAL(editingFinished()), this, SLOT(currentPinEditingFinished()));
  connect( _ui.currentPasswordLineEdit, SIGNAL(textChanged(QString)), this, SLOT(currentPinTextChanged(QString)));
  connect( _ui.newPasswordLineEdit, SIGNAL(editingFinished()), this, SLOT(newPinEditingFinished()));
  connect( _ui.newPasswordLineEdit, SIGNAL(textChanged(QString)), this, SLOT(newPinTextChanged(QString)));
  connect( _ui.confirmPasswordLineEdit, SIGNAL(editingFinished()), this, SLOT(confirmNewPinEditingFinished()));
  connect( _ui.confirmPasswordLineEdit, SIGNAL(textChanged(QString)), this, SLOT(confirmNewPinTextChanged(QString)));
  
  //Initializing SmartCard Control Handler
  if( _scControl->init() ) 
     _ui.passwordWidget->setEnabled( true );
  
  _scControl->moveToThread( _scThread );
  _scThread->start();  
  QMetaObject::invokeMethod(_scControl, "startPolling", Qt::QueuedConnection);

  _logo = new QPixmap(":/icons/logo.png");
  _ui.logoLabel->setPixmap(*_logo);
  _ui.logoLabel->show();
  _ui.logoLabel->setScaledContents(true);
  _logosize = QSize(1094, 556);
  _logosize /= 3.0;
  _ui.logoLabel->setMinimumSize( _logosize );
  _ui.logoLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  
  _ui.tabWidget->setTabText(0, tr("Change pin") );
  _ui.tabWidget->setTabText(1, tr("Card information") );
  _ui.tabWidget->setTabText(2, tr("Authentication Certificate (X509)") );    
  _ui.changePasswordButton->setText( tr("&Change password") );
  
  this->setFixedSize(_logosize.width() + 100, _logosize.height() + 300);
}

MainWidget::~MainWidget() {
  logger().setLogWidget( 0 );
  _scThread->quit();
  _scThread->wait();
  delete _scThread;
  delete _scControl;
  delete _logo;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// protected  //////////////////////////////////////

//Catches change Events, f.e. the application will be minimized to tray when the user minimzes the mainwindow
void MainWidget::changeEvent(QEvent *event ) {
  QMainWindow::changeEvent(event);
  if(event->type() == QEvent::WindowStateChange) {
    if(isMinimized()) {
      this->hide();
    }
  }
}

//Catches close Events so that the application will be minimized to tray when the user closes the mainwindow
void MainWidget::closeEvent(QCloseEvent *event) {
  if (_trayIcon->isVisible()) {
//     QMessageBox::information(this, tr("Systray"), tr("The program will keep running in the "
//                                      "system tray. To terminate the program, "
//                                      "choose <b>Quit</b> in the context menu "
//                                      "of the system tray entry."));
    hide();
    event->ignore();
  }
}

//Catches hide events to disable the minimize action in the trayIcon menu
void MainWidget::hideEvent(QHideEvent *event) {
  _minimizeAction->setEnabled(false);
  QWidget::hideEvent(event);
}

//Catches hide events to enable the minimize action in the trayIcon menu
void MainWidget::showEvent(QShowEvent* event) {
  _minimizeAction->setEnabled( true );
  QWidget::showEvent(event);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////// private  ////////////////////////////////////////


//Sets up the tray icon and its context menu
void MainWidget::setupTrayIcon() {
  //Creating actions fro tray icon menu
  _minimizeAction = new QAction(tr("Mi&nimize"), this);
  connect(_minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));
  _openAction = new QAction(tr("&Open..."), this);
  connect(_openAction, SIGNAL(triggered()), this, SLOT(openActionTriggered()));
  _aboutAction = new QAction(tr("&About..."), this);
  connect(_aboutAction, SIGNAL(triggered()), _aboutDialog, SLOT(exec())); 
  _quitAction = new QAction(tr("&Quit"), this);
  connect(_quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
  _enableDebugViewAction = new QAction(tr("&Enable Debug View"), this);
  connect(_enableDebugViewAction, SIGNAL(triggered()), this, SLOT(enableDebugViewActionTriggered()));
  
  //Setting up tray icon context menu
  _trayIconMenu = new QMenu(this);
  _trayIconMenu->addAction(_openAction);
  _trayIconMenu->addAction(_aboutAction);
  _trayIconMenu->addSeparator();
  _trayIconMenu->addAction(_minimizeAction);
  _trayIconMenu->addSeparator();
  _trayIconMenu->addAction(_enableDebugViewAction);
  _trayIconMenu->addAction(_quitAction);

  //Creating tray icon
  _trayIcon = new QSystemTrayIcon(this);
  _trayIcon->setContextMenu(_trayIconMenu );
//   _trayIcon->setIcon( QIcon(":/icons/provinz_wappen.jpg") );
#ifndef WIN32  
  _trayIcon->setIcon( QIcon(":/icons/provinz_wappen.svg") );
#else
  _trayIcon->setIcon( QIcon(":/icons/provinz_wappen.png") );
#endif
  connect( _trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(systemTrayActivated(QSystemTrayIcon::ActivationReason)));
}

//Enables/Disables the button to change/unblock the pin according to the pin input LineEdits
void MainWidget::updatePinChangeButtonState() {
  if( (_ui.currentPasswordLineEdit->text() == "" && !_pinBlocked) || 
       _ui.newPasswordLineEdit->text() == "" || 
       _ui.confirmPasswordLineEdit->text() == "" || 
       _ui.newPasswordLineEdit->text() != _ui.confirmPasswordLineEdit->text() ) {
    _ui.changePasswordButton->setEnabled( false );
  } else {
    if( (!_pinBlocked && (pinTooLong(_ui.currentPasswordLineEdit->text(), _pinInfo) || pinTooShort(_ui.currentPasswordLineEdit->text(), _pinInfo))) ||
        pinTooLong(_ui.newPasswordLineEdit->text(), _pinInfo) || pinTooShort(_ui.newPasswordLineEdit->text(), _pinInfo) ||
        pinTooLong(_ui.confirmPasswordLineEdit->text(), _pinInfo) || pinTooShort(_ui.confirmPasswordLineEdit->text(), _pinInfo)   ) {
        
      _ui.changePasswordButton->setEnabled( false );
    } else {
      _ui.changePasswordButton->setEnabled( true );
    }
  }
}

//Returns true if the given pin is too short and the given pinInfo valid, otherwise false
bool MainWidget::pinTooShort(const QString& pin, const CardControlHandler::Pkcs15PinInfo& pinInfo) {
  if( !pinInfo.isValid )
    return false;
  
  return pin.size() < _pinInfo.minLength; 
}

//Returns true if the given pin is too long and the given pinInfo valid, otherwise false
bool MainWidget::pinTooLong(const QString& pin, const CardControlHandler::Pkcs15PinInfo& pinInfo) {
  if( !pinInfo.isValid )
    return false;
  
  return pin.size() > _pinInfo.maxLength; 
}

void MainWidget::resetPersonalData() {
  memset( &_personalData, 0, sizeof(CardControlHandler::PersonalData));
  updateCardInformation();
}


void MainWidget::resetSerialData() {
  memset( &_serialData, 0, sizeof(CardControlHandler::SerialData));
  updateCardInformation();
}

void MainWidget::resetX509CertificationData() {
  memset( &_x509Data, 0 , sizeof(X509CertificateHandler::X509CertificateData) );
  updateX509CertificateInformation();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// slots  ////////////////////////////////////////

void MainWidget::aboutDialogActionTriggered() {
  QString aboutInfo = QString(tr("Software Version: %1\nCompile Date: %2 %3\nUsing: %4")).arg(BUERGERKATRE_SW_VERSION).arg(__DATE__).arg(__TIME__).arg(PACKAGE_STRING);
  _trayIcon->showMessage(QString(tr("About Buegerkarte")), aboutInfo, QSystemTrayIcon::Information);
}


void MainWidget::changePasswordButtonClicked() {
  QString currentPassword = _ui.currentPasswordLineEdit->text();
  QString newPassword = _ui.newPasswordLineEdit->text();
  QString newPasswordConfirmed = _ui.confirmPasswordLineEdit->text();
  
  if( !_pinBlocked ) {
    if( currentPassword == "" ) {
      QMessageBox::warning(this, tr("Empty Pin field"), tr("Please enter current Pin"));
    } else if( _pinInfo.isValid && ( currentPassword.size() < _pinInfo.minLength || currentPassword.size() > _pinInfo.maxLength ) ) {
      QMessageBox::warning(this, tr("Incorrect Pin size"), tr("Current Pin must have between %1 and %2 characters").arg(_pinInfo.minLength).arg(_pinInfo.maxLength));
      _ui.currentPasswordLineEdit->clear();
      return;    
    }
  }
  
  if( newPassword == "" ) {
    QMessageBox::warning(this, tr("Empty Pin field"), tr("Please enter new Pin"));
    _ui.confirmPasswordLineEdit->clear();
    return;
  } else if( _pinInfo.isValid && ( newPassword.size() < _pinInfo.minLength || newPassword.size() > _pinInfo.maxLength ) ) {
    QMessageBox::warning(this, tr("Incorrect Pin size"), tr("New Pin must have between %1 and %2 characters").arg(_pinInfo.minLength).arg(_pinInfo.maxLength));
    _ui.newPasswordLineEdit->clear();
    _ui.confirmPasswordLineEdit->clear();
    return;    
  }
  
  if( newPasswordConfirmed == "" ) {
    QMessageBox::warning(this, tr("Empty Pin field"), tr("Please confirm new Pin"));
    return;
  }
  
  if( newPassword != newPasswordConfirmed ) {
    QMessageBox::warning(this, tr("Password missmath"), tr("Confirmed Pin does not math new Pin"));
    _ui.newPasswordLineEdit->clear();
    _ui.confirmPasswordLineEdit->clear();
    return;
  }
  
  if( _pinBlocked ) {
    //Pin authentication is blocked => Request Puk from user
    int result;
    PukDialog pukDialog( this );
    QString puk = "";
    while( (result = pukDialog.exec(_pinInfo)) > 0 ) {
      puk = pukDialog.getPuk();
      if( puk == "" ) {
        QMessageBox::warning(this, tr("Empty PUK field"), tr("Please enter PUK"));
        continue;
      } else if ( _pinInfo.isValid && ( puk.size() < _pinInfo.minLength || puk.size() > _pinInfo.maxLength ) ) {
        QMessageBox::warning(this, tr("Incorrect PUK size"), tr("PUK must have between %1 and %2 characters").arg(_pinInfo.minLength).arg(_pinInfo.maxLength));
        continue;
      }
      break;
    }
    
    if( result <= 0 ) {
      _ui.newPasswordLineEdit->clear();
      _ui.confirmPasswordLineEdit->clear();
      return;
    }
    _scControl->unblockPinPkcs15Request( puk.toStdString().c_str(), newPassword.toStdString().c_str()  );
  } else {
    _scControl->changePinPkcs15Request( currentPassword.toStdString().c_str(), newPassword.toStdString().c_str()  );
  }
}


//----------------------------------------------------------------------------------------
//---------------------- SLOTS FOR COMMUNICATION WITH SMART CARD -------------------------
//----------------------------------------------------------------------------------------

//This slot will be called when the smart card reader was connected, and displays a message to inform the user
void MainWidget::cardReaderWasConnected(sc_reader_t scReader) {
  QString aboutInfo = QString(tr("Card Reader was connected:\n Reader: %1")).arg(scReader.name);
  _trayIcon->showMessage(QString(tr("Card Reader Connected")), aboutInfo, QSystemTrayIcon::Information, 2000);
}

//This slot will be called when a smart card was connected, and displays a message to inform the user
void MainWidget::smartCardWasConnected(sc_card_t scCard) {
  QString aboutInfo = QString(tr("Smart Card was inserted:\n Card: %1")).arg(scCard.name);
  _trayIcon->showMessage(QString(tr("Smart Card Inserted")), aboutInfo, QSystemTrayIcon::Information, 2000);
  _ui.passwordWidget->setEnabled( true );
}

//This slot will be called when the current smart card reader was removed, and displays a message to inform the user
void MainWidget::cardReaderWasRemoved() {
  QString aboutInfo = QString(tr("Card Reader was removed!"));
  _trayIcon->showMessage(QString(tr("Card Reader removed")), aboutInfo, QSystemTrayIcon::Information, 1000);
  _ui.passwordWidget->setEnabled( false );
  
  memset( &_pinInfo, 0, sizeof(CardControlHandler::Pkcs15PinInfo));
  _ui.oldPinLabel->setText( tr(DEFAULT_TEXT_INSERT_CURRENT_PIN) );
  _ui.newPinLabel->setText( tr(DEFAULT_TEXT_INSERT_NEW_PIN) );
  _ui.confirmPinLabel->setText( tr(DEFAULT_TEXT_CONFIRM_NEW_PIN) );
  _ui.changePasswordButton->setText(tr(DEFAULT_TEXT_CHANGE_PIN_BUTTON));
  _pinBlocked = false;
  resetPersonalData();
  resetSerialData();
  resetX509CertificationData();
}

//This slot will be called when the current smart card was connected, and displays a message to inform the user
void MainWidget::smartCardWasRemoved(){
  QString aboutInfo = QString(tr("Smart Card was removed!"));
  _trayIcon->showMessage(QString(tr("Smart Card removed")), aboutInfo, QSystemTrayIcon::Information, 1000);
  _ui.passwordWidget->setEnabled( false );
  
  memset( &_pinInfo, 0, sizeof(CardControlHandler::Pkcs15PinInfo));
  _ui.oldPinLabel->setText( tr(DEFAULT_TEXT_INSERT_CURRENT_PIN) );
  _ui.newPinLabel->setText( tr(DEFAULT_TEXT_INSERT_NEW_PIN) );
  _ui.confirmPinLabel->setText( tr(DEFAULT_TEXT_CONFIRM_NEW_PIN) );
  _ui.changePasswordButton->setText(tr(DEFAULT_TEXT_CHANGE_PIN_BUTTON));
  _pinBlocked = false;
  resetPersonalData();
  resetSerialData();
  resetX509CertificationData();
}

//This slot is called when the pin infomation is gathered after inserting a card   
void MainWidget::pkcs15PinInfoWasGathered(sc_pkcs15_auth_info_t pinInfo) {
  _pinInfo.maxLength = pinInfo.attrs.pin.max_length;
  _pinInfo.minLength = pinInfo.attrs.pin.min_length;
  _pinInfo.isValid = true;
  
  _ui.oldPinLabel->setText( tr(DEFAULT_TEXT_INSERT_CURRENT_PIN) + QString(tr("[Min:%1  Max:%2]").arg(_pinInfo.minLength).arg(_pinInfo.maxLength)) );
  _ui.newPinLabel->setText( tr(DEFAULT_TEXT_INSERT_NEW_PIN) + QString(tr("[Min:%1  Max:%2]").arg(_pinInfo.minLength).arg(_pinInfo.maxLength) ) );
  _ui.confirmPinLabel->setText( tr(DEFAULT_TEXT_CONFIRM_NEW_PIN) + QString(tr("[Min:%1  Max:%2]").arg(_pinInfo.minLength).arg(_pinInfo.maxLength)) );
}

//This slot will be called when a pin change request has been finished, and in case of an error it contains the relevant error context    
void MainWidget::pksc15PinChangeDone(Error err) {
  if( err.hasError() ) { 
    QMessageBox::critical(this, tr("Error"), QString(tr("Error changing pin %1")).arg(sc_strerror(err.scError())));
    if( err.scError() == SC_ERROR_AUTH_METHOD_BLOCKED ) {
      QString pinLengthTxt = _pinInfo.isValid ? tr("[Min:%1  Max:%2]").arg(_pinInfo.minLength).arg(_pinInfo.maxLength) : "";
      _ui.changePasswordButton->setText(tr(DEFAULT_TEXT_UNBLOCK_PIN_BUTTON));
      _pinBlocked = true;
    }
  } else {
    QMessageBox::information(this, tr("Pin changed"), tr("Pin has been successfully changed")); 
  }
  _ui.currentPasswordLineEdit->clear();
  _ui.newPasswordLineEdit->clear();
  _ui.confirmPasswordLineEdit->clear();
}

//This slot will be called when a pin unblock request has been finished, and in case of an error it contains the relevant error context    
void MainWidget::pksc15PinUnblockDone(Error err) {
  if( err.hasError() ) { 
    QMessageBox::critical(this, tr("Error"), QString(tr("Error unblocking pin %1")).arg(sc_strerror(err.scError())));
  } else {
    QMessageBox::information(this, tr("Pin unblocked"), tr("Pin has been successfully unblocked")); 
    QString pinLengthTxt = _pinInfo.isValid ? tr("[Min:%1  Max:%2]").arg(_pinInfo.minLength).arg(_pinInfo.maxLength) : "";
    _ui.newPinLabel->setText( tr(DEFAULT_TEXT_INSERT_NEW_PIN)+ pinLengthTxt );
    _ui.confirmPinLabel->setText( tr(DEFAULT_TEXT_CONFIRM_NEW_PIN) + pinLengthTxt );
    _ui.changePasswordButton->setText(tr(DEFAULT_TEXT_CHANGE_PIN_BUTTON));
    _pinBlocked = false;
  }
  _ui.currentPasswordLineEdit->clear();
  _ui.newPasswordLineEdit->clear();
  _ui.confirmPasswordLineEdit->clear();
}

void MainWidget::personalDataGathered(CardControlHandler::PersonalData personalData) {
  memcpy( &_personalData, &personalData, sizeof(personalData) );
  updateCardInformation();
}

void MainWidget::serialDataGathered(CardControlHandler::SerialData serialData) {
  memcpy( &_serialData, &serialData, sizeof( serialData) );
  updateCardInformation();
}


void MainWidget::x509CertificateDataGathered(X509CertificateHandler::X509CertificateData x509Data) {
  memcpy( &_x509Data, &x509Data, sizeof(x509Data) );
  updateX509CertificateInformation();
}


//----------------------------------------------------------------------------------------
//--------------------------------  GENERAL SLOTS ----------------------------------------
//----------------------------------------------------------------------------------------


void MainWidget::currentPinEditingFinished() {
  updatePinChangeButtonState();
  if( _ui.currentPasswordLineEdit->text() != "" ) {
    QString oldPin = _ui.currentPasswordLineEdit->text();
    if( pinTooShort(oldPin, _pinInfo) ) {
      _ui.currentPinInfoLabel->setText(tr("Pin too short"));
    } else if( pinTooLong( oldPin, _pinInfo ) ){
      _ui.currentPinInfoLabel->setText(tr("Pin too long"));
    } else {
      _ui.currentPinInfoLabel->setText("");
    }
  }
}

void MainWidget::currentPinTextChanged( const QString& text ) {
  updatePinChangeButtonState();
  QString oldPin = _ui.currentPasswordLineEdit->text();
  if( oldPin == "" || ( !pinTooShort(oldPin, _pinInfo) && !pinTooLong(oldPin, _pinInfo) ))
    _ui.currentPinInfoLabel->setText("");
}

void MainWidget::newPinEditingFinished() {
  updatePinChangeButtonState();
  if( _ui.newPasswordLineEdit->text() != "" ) {
    QString newPin = _ui.newPasswordLineEdit->text();
    if( pinTooShort(newPin, _pinInfo) ) {
      _ui.newPinInfoLabel->setText(tr("Pin too short"));
    } else if( pinTooLong( newPin, _pinInfo ) ){
      _ui.newPinInfoLabel->setText(tr("Pin too long"));
    } else {
      _ui.newPinInfoLabel->setText("");
    }
  }
}

void MainWidget::newPinTextChanged( const QString& text ) {
  updatePinChangeButtonState();
  QString newPin = _ui.newPasswordLineEdit->text();
  if( newPin == "" || (!pinTooShort(newPin, _pinInfo) && !pinTooLong(newPin, _pinInfo)) )
    _ui.newPinInfoLabel->setText("");
  if( newPin != "" && _ui.confirmPasswordLineEdit->text() != "" ) {
    if( _ui.confirmPasswordLineEdit->text() == newPin )
      _ui.confirmPinInfoLabel->setText("");
    else
      _ui.confirmPinInfoLabel->setText("Does not match new pin");
  }
}

void MainWidget::confirmNewPinEditingFinished() {
  updatePinChangeButtonState();
  if( _ui.confirmPasswordLineEdit->text() != "" ) {
    QString confirmPin = _ui.confirmPasswordLineEdit->text();
    if( pinTooShort(confirmPin, _pinInfo) ) {
      _ui.confirmPinInfoLabel->setText(tr("Pin too short"));
    } else if( pinTooLong( confirmPin, _pinInfo ) ){
      _ui.confirmPinInfoLabel->setText(tr("Pin too long"));
    } else if( confirmPin != _ui.newPasswordLineEdit->text() ) {
      _ui.confirmPinInfoLabel->setText("Does not match new pin");
    } else {
      _ui.confirmPinInfoLabel->setText("");
    }
  }
}

void MainWidget::confirmNewPinTextChanged( const QString& text ) {
  updatePinChangeButtonState();
  QString confirmPin = _ui.confirmPasswordLineEdit->text();
  if( confirmPin == "" || (!pinTooShort(confirmPin, _pinInfo) && !pinTooLong(confirmPin, _pinInfo) ))
    _ui.confirmPinInfoLabel->setText(""); 
  if( confirmPin != _ui.newPasswordLineEdit->text() && confirmPin.size() > 0 ) {
      _ui.confirmPinInfoLabel->setText("Does not match new pin");
  }
}

void MainWidget::systemTrayActivated(QSystemTrayIcon::ActivationReason reason) {
  if( reason == QSystemTrayIcon::Trigger ) {
    this->activateWindow();
    showNormal();
  }
}

void MainWidget::openActionTriggered() {
  this->activateWindow();
  showNormal();
}


void MainWidget::enableDebugViewActionTriggered(){
  if ( _textEdit->isVisible() ) {
    _textEdit->hide();
    _enableDebugViewAction->setText( tr("&Enable debug view") );
  } else {
    _textEdit->show();
    _enableDebugViewAction->setText( tr("&Disable debug view") );
  }
}


void MainWidget::updateCardInformation() {
  if( !_serialData.isValid ) {
    _ui.serialNumberLineEdit->clear();
  } else {
    _ui.serialNumberLineEdit->setText( _serialData.serial );
  }
  
  if( !_personalData.isValid ) {
    _ui.cardInformationTab->setEnabled(false);
    _ui.issuingDateLineEdit->clear();
    _ui.expiringDateLineEdit->clear();
    _ui.nameLineEdit->clear();
    _ui.financialCodeLineEdit->clear();
    _ui.birthdateLineEdit->clear();
    _ui.sexLineEdit->clear();
    _ui.addressLineEdit->clear();
  } else {
    _ui.cardInformationTab->setEnabled(true);
    if( _personalData.dataFields[CardControlHandler::pd_IssuingDate].len >= 8 ) {
      QDate issuingDate = QDate::fromString( _personalData.dataFields[CardControlHandler::pd_IssuingDate].value, "ddMMyyyy" );
      _ui.issuingDateLineEdit->setText( issuingDate.toString("dd.MM.yyyy") );
    } else {
      _ui.issuingDateLineEdit->setText("Unknown");
    }
    
    if( _personalData.dataFields[CardControlHandler::pd_ExpiryDate].len >= 8 ) {
      QDate expiringDate = QDate::fromString( _personalData.dataFields[CardControlHandler::pd_ExpiryDate].value, "ddMMyyyy" );
      _ui.expiringDateLineEdit->setText( expiringDate.toString("dd.MM.yyyy") );
    } else {
      _ui.expiringDateLineEdit->setText("Unknown");
    }
    
    if( _personalData.dataFields[CardControlHandler::pd_BirthDate].len >= 8 ) {
      QDate birthdate = QDate::fromString( _personalData.dataFields[CardControlHandler::pd_BirthDate].value, "ddMMyyyy" );
      _ui.birthdateLineEdit->setText( birthdate.toString("dd.MM.yyyy") );
    } else {
      _ui.birthdateLineEdit->setText("Unknown");
    }
    
    if( _personalData.dataFields[CardControlHandler::pd_CodiceFiscale].len > 0 )
      _ui.financialCodeLineEdit->setText(_personalData.dataFields[CardControlHandler::pd_CodiceFiscale].value );
    else
      _ui.financialCodeLineEdit->setText("Unknown");
    
    if( _personalData.dataFields[CardControlHandler::pd_Sex].len > 0 ) {
      QString gender(_personalData.dataFields[CardControlHandler::pd_Sex].value );
      if( gender == "M" )
        _ui.sexLineEdit->setText("Male");
      else if( gender == "F" )
        _ui.sexLineEdit->setText("Female");
      else
      _ui.financialCodeLineEdit->setText("Unknown");
    } else {
      _ui.financialCodeLineEdit->setText("Unknown");
    }
    
    if( _personalData.dataFields[CardControlHandler::pd_FirstName].len > 0 || _personalData.dataFields[CardControlHandler::pd_LastName].len > 0 ) {
      _ui.nameLineEdit->setText( QString(_personalData.dataFields[CardControlHandler::pd_LastName].value) + " " + QString( _personalData.dataFields[CardControlHandler::pd_FirstName].value) );
    } else {
      _ui.nameLineEdit->setText("Unknown");  
    }
    
    if( _personalData.dataFields[CardControlHandler::pd_ResidenceAddress].len > 0 )
      _ui.addressLineEdit->setText(_personalData.dataFields[CardControlHandler::pd_ResidenceAddress].value );
    else
      _ui.addressLineEdit->setText("Unknown");
  }
}

void MainWidget::updateX509CertificateInformation() {
  if( !_x509Data.isValid ) {
    _ui.authenticationCertificateTab->setEnabled(false);
    _ui.x509SerialNumerLineEdit->clear();
    _ui.x509validNotBeforeLineEdit->clear();
    _ui.x509ValidNotAfterLineEdit->clear();
    _ui.x509IssuerCommonNameLineEdit->clear();
  } else {
    _ui.authenticationCertificateTab->setEnabled(true);
    if( string(_x509Data.serialNumber) != "" ) {
      _ui.x509SerialNumerLineEdit->setText( _x509Data.serialNumber );
    } else {
      _ui.x509SerialNumerLineEdit->setText( tr("Unknown") );
    }
    
    if( string(_x509Data.issuer.commonName) != "" ) {
      _ui.x509IssuerCommonNameLineEdit->setText( _x509Data.issuer.commonName );
    } else {
      _ui.x509IssuerCommonNameLineEdit->setText( tr("Unknown") );
    }
    
    if( _x509Data.validity.notBefore != 0 ) {
      _ui.x509validNotBeforeLineEdit->setText( ctime(&_x509Data.validity.notBefore) );
    } else {
      _ui.x509validNotBeforeLineEdit->setText( tr("Unknown") );
    }
    
    if( _x509Data.validity.notAfter != 0 ) {
      _ui.x509ValidNotAfterLineEdit->setText( ctime(&_x509Data.validity.notAfter) );
    } else {
      _ui.x509ValidNotAfterLineEdit->setText( tr("Unknown") );
    }
  }
}

