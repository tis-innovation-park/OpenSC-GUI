#include "logger.h"

#include <QByteArray>

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
  va_start(args, mess);
  char str[1024];
  qvsnprintf(str, sizeof(str), mess, args);
  str[sizeof(str)-1] = '\0';
  va_end(args);

  printf("%s\n", str);
  if( _logTextEdit )
    emit(loggingRequest( QString::fromLocal8Bit(str) ));
}


void Logger::setLogWidget( QTextEdit *textedit ) { 
  QMutexLocker ml( &_loggerMutex );
  _logTextEdit = textedit; 
  
  if( _logTextEdit ) {
    connect( this, SIGNAL(loggingRequest(QString)), _logTextEdit, SLOT(insertPlainText(QString)), Qt::QueuedConnection);
  }
}
