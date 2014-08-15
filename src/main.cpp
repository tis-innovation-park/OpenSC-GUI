#include "mainwidget.h"

int main( int argc, char ** argv ){
  qRegisterMetaType<sc_reader_t>("sc_reader_t");
  qRegisterMetaType<sc_card_t>("sc_card_t");
  qRegisterMetaType<QTextCursor>("QTextCursor");
  qRegisterMetaType<Error>("Error");
  qRegisterMetaType<sc_pkcs15_auth_info_t>("sc_pkcs15_auth_info_t");
  qRegisterMetaType<CardControlHandler::PersonalData>("CardControlHandler::PersonalData");
  qRegisterMetaType<CardControlHandler::SerialData>("CardControlHandler::SerialData");
  qRegisterMetaType<X509CertificateHandler::X509CertificateData>("X509CertificateHandler::X509CertificateData");
  
  QApplication a( argc, argv );  
//   QSharedMemory mem("buergerkarteapp");
//   if(!mem.create(1)) {
//     QMessageBox::critical(0,"Instance detected!","Application is already running!\nApplication terminating...","Ok");
//     exit(0);
//   }
  
  MainWidget* mw = new MainWidget();
  mw->show();
  
  return a.exec();
}
