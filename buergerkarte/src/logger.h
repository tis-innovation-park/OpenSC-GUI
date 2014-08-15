#ifndef SC_LOGGER_H
#define SC_LOGGER_H

#include <fstream>

#include <QMutexLocker>
#include <QTextEdit>

using namespace std;

class Logger : public QObject  {
  Q_OBJECT
  public:
    void log(const char*, ...);
    void setLogWidget( QTextEdit *textedit );
  private:
    Logger();
    friend Logger& logger();   
    
    QMutex _loggerMutex;
    QTextEdit *_logTextEdit;
  signals:
    void loggingRequest( QString logText );
};

Logger& logger();
#endif


