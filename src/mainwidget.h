#ifndef MAINWIDGET_H
#define MAINWIDGET_H


#include "logger.h"

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QMenuBar>
#include <QThread>
#include <QCloseEvent>
#include <QDateTimeEdit>

#include "config.h"
#include "pukdialog.h"
#include "aboutdialog.h"
#include "statusbar.h"

#include "ui_mainwidget.h"


using namespace std;

#define BUERGERKATRE_SW_VERSION 1.0

#define DEFAULT_TEXT_INSERT_CURRENT_PIN "Enter current Pin:"
#define DEFAULT_TEXT_INSERT_NEW_PIN "Enter new Pin:"
#define DEFAULT_TEXT_CONFIRM_NEW_PIN "Confirm new Pin:"
#define DEFAULT_TEXT_CHANGE_PIN_BUTTON "Change Pin"
#define DEFAULT_TEXT_UNBLOCK_PIN_BUTTON "Unblock Pin"


class MainWidget:public QMainWindow{
  Q_OBJECT
  public:
    MainWidget(QWidget *parent = NULL);
    ~MainWidget();

  protected:
    //Catches change Events, f.e. the application will be minimized to tray when the user minimzes the mainwindow
    virtual void changeEvent(QEvent *event);
    
    //Catches close Events so that the application will be minimized to tray when the user closes the mainwindow
    virtual void closeEvent(QCloseEvent *event);
    
    //Catches hide events to disable the minimize action in the trayIcon menu
    virtual void hideEvent(QHideEvent *event);
    
    //Catches hide events to enable the minimize action in the trayIcon menu
    virtual void showEvent(QShowEvent *event);
    
  private:
    //Sets up the tray icon and its context menu
    void setupTrayIcon();
    
    //Enables/Disables the button to change/unblock the pin according to the pin input LineEdits
    void updatePinChangeButtonState();
    
    //Returns true if the given pin is too short and the given pinInfo valid, otherwise false
    bool pinTooShort( const QString& pin, const CardControlHandler::Pkcs15PinInfo& pinInfo);
    
    //Returns true if the given pin is too long and the given pinInfo valid, otherwise false
    bool pinTooLong( const QString& pin, const CardControlHandler::Pkcs15PinInfo& pinInfo);
    
    void resetPersonalData();
    void resetSerialData();
    void resetX509CertificationData();
    
    Ui_MainWidget _ui;  //Graphical User Interface
    QPixmap *_logo;     //Logo displayed on top of the GUI
    AboutDialog *_aboutDialog;
  
    CardControlHandler *_scControl; //Handles access and operations on the smart card and the smart card reader
    QThread *_scThread; //Thread where the CardControlHandler is running
    
    CardControlHandler::Pkcs15PinInfo _pinInfo; //Contains information about the min and max pin length and if the data is valid
    bool _pinBlocked;   //Shows is the pin authentication method on the inserted card is blocked, will be reset if card or reader is removed
    
    CardControlHandler::PersonalData _personalData;
    CardControlHandler::SerialData _serialData;
    X509CertificateHandler::X509CertificateData _x509Data;
    
    //System Tray Icon
    QSystemTrayIcon *_trayIcon;
    QMenu *_trayIconMenu;  
    QAction *_openAction;             //Action to open the mainwindoow
    QAction *_aboutAction;            //Action to request version information
    QAction *_minimizeAction;         //Action to minimize the mainwindow to the tray
    QAction *_quitAction;             //Action to quit the application
    QAction *_enableDebugViewAction;  //Action to quit the application
    
    // Status
    StatusBar *_statusBar;      //Bar showing brief status information about the card reader / smart card
    QTextEdit *_textEdit;       //A Textfield to show Debug information
    QSize _logosize;
   
  public slots:
    //This slot is called when the user clicks on the about action in the tray icon menu, a message showing version infos will be displayed
    void aboutDialogActionTriggered();
    
    //This slot is called when the user requests to change the password, after checking the validity of the inserted pin information a change Pin request is sent to the smart card
    void changePasswordButtonClicked();

    //This slot is called when the smart card reader was connected, and displays a message to inform the user
    void cardReaderWasConnected( sc_reader_t scReader );
    
    //This slot is called when a smart card was connected, and displays a message to inform the user
    void smartCardWasConnected( sc_card_t scCard );
    
    //This slot is called when the current smart card reader was removed, and displays a message to inform the user
    void cardReaderWasRemoved();
    
    //This slot is called when the current smart card was connected, and displays a message to inform the user
    void smartCardWasRemoved();
    
    //This slot is called when the pin infomation is gathered after inserting a card
    void pkcs15PinInfoWasGathered(sc_pkcs15_auth_info_t pinInfo);
    
    //This slot will be called when a pin change request has been finished, and in case of an error it contains the relevant error context
    void pksc15PinChangeDone( Error err );
    
    //This slot will be called when a pin unblock request has been finished, and in case of an error it contains the relevant error context
    void pksc15PinUnblockDone( Error err );
    
    void personalDataGathered( CardControlHandler::PersonalData personalData);
    
    void serialDataGathered( CardControlHandler::SerialData serialData );
    
    void x509CertificateDataGathered(X509CertificateHandler::X509CertificateData x509Data);
    
  private slots:
    //---- Slots to monitor the user input and display according error hints ------
    void currentPinEditingFinished();
    void currentPinTextChanged( const QString& text );
    void newPinEditingFinished();
    void newPinTextChanged( const QString& text );
    void confirmNewPinEditingFinished();
    void confirmNewPinTextChanged( const QString& text );
    
    //---- Slots to show the mainwindow if it is in hidden or deactivated state -----
    void systemTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void openActionTriggered();
    void openAboutDialogTriggered();
    void openDebugDialogTriggered();
    void enableDebugViewActionTriggered();
    
    void updateCardInformation();
    void updateX509CertificateInformation();
};

#endif

