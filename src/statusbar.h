#ifndef STATUS_BAR_H_
#define STATUS_BAR_H_

#include <QLabel>
#include <QStatusBar>

#include "cardcontrolhandler.h"
#include "opensc.h"

class StatusBar: public QStatusBar {
  Q_OBJECT
  public:
    StatusBar( CardControlHandler* scControl);
    ~StatusBar();

  private:
    QLabel *_readerStatusLabel, *_cardStatusLabel;
    CardControlHandler* _scControl;
  
  public slots:
    void cardReaderWasConnected( sc_reader_t scReader );
    void smartCardWasConnected( sc_card_t scCard );
    void cardReaderWasRemoved();
    void smartCardWasRemoved();
};

#endif
