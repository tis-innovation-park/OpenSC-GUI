#include "mainwidget.h"

#include <QSharedMemory>

int main( int argc, char ** argv ){
  qRegisterMetaType<sc_reader_t>("sc_reader_t");
  qRegisterMetaType<sc_card_t>("sc_card_t");
  qRegisterMetaType<QTextCursor>("QTextCursor");
  qRegisterMetaType<Error>("Error");
  qRegisterMetaType<sc_pkcs15_auth_info_t>("sc_pkcs15_auth_info_t");
  qRegisterMetaType<CardControlHandler::PersonalData>("CardControlHandler::PersonalData");
  qRegisterMetaType<CardControlHandler::SerialData>("CardControlHandler::SerialData");
  qRegisterMetaType<X509CertificateHandler::X509CertificateData>("X509CertificateHandler::X509CertificateData");
  

//   QSharedMemory shared("6d6f02f0-ed13-4132-b21e-be746ed4a623");
//   if( !shared.create( 512, QSharedMemory::ReadWrite) )
//     exit(0);

  QApplication a( argc, argv );
  bool hideMainWin = false;
  if ( argc == 2 && string(argv[1]) == "startHidden" )
    hideMainWin = true;
  
  MainWidget* mw = new MainWidget();
  if ( !hideMainWin )
    mw->show();
  
  return a.exec();
}
