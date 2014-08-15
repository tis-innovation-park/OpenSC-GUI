#include "logger.h"

Logger& logger(){
  static Logger logger;
  return logger;
}
 


 
Logger::Logger() {
  _logTextEdit = 0;
}


void Logger::log(const char *mess, ... ) {
  QMutexLocker ml( &_loggerMutex );
  va_list args;
  va_start( args, mess);
  char str[4096];
  vsprintf(str, mess, args);
  va_end (args);
  string mess_str = string(str);
  printf("%s\n", str);
  
  if( _logTextEdit )
    emit(loggingRequest( QString(str) ));
}



void Logger::setLogWidget( QTextEdit *textedit ) { 
  QMutexLocker ml( &_loggerMutex );
  _logTextEdit = textedit; 
  
  if( _logTextEdit ) {
    connect( this, SIGNAL(loggingRequest(QString)), _logTextEdit, SLOT(insertPlainText(QString)), Qt::QueuedConnection);
  }
}