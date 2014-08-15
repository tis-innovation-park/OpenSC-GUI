#include "statusbar.h"

StatusBar::StatusBar( CardControlHandler* scControl ) : QStatusBar() {  
  _scControl = scControl;
  
  connect( _scControl, SIGNAL(cardReaderConnected(sc_reader_t)), this, SLOT(cardReaderWasConnected(sc_reader_t)));
  connect( _scControl, SIGNAL(smartCardConnected(sc_card_t)), this, SLOT(smartCardWasConnected(sc_card_t)));
  connect( _scControl, SIGNAL(cardReaderRemoved(sc_reader_t)), this, SLOT(cardReaderWasRemoved()));
  connect( _scControl, SIGNAL(smartCardRemoved(sc_card_t)), this, SLOT(smartCardWasRemoved()));
  
  _readerStatusLabel = new QLabel( tr("Card Reader: Not Connected") );
  _readerStatusLabel->setAlignment( Qt::AlignLeft);
  
  _cardStatusLabel = new QLabel( tr("Card: No Card") );
  _cardStatusLabel->setAlignment( Qt::AlignRight);
  
  this->addPermanentWidget(_readerStatusLabel,1);
  this->addPermanentWidget(_cardStatusLabel,1);
  this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  
  connect( &_updateTimer, SIGNAL(timeout()), this, SLOT(updateStatus()));
  _updateTimer.start(500);
}

StatusBar::~StatusBar() {
  delete _readerStatusLabel;
  delete _cardStatusLabel;
}



void StatusBar::updateStatus() {
  
}

void StatusBar::cardReaderWasConnected(sc_reader_t scReader) {
  _readerStatusLabel->setText( QString(tr("Card Reader: %1")).arg(scReader.name) );
}

void StatusBar::smartCardWasConnected(sc_card_t scCard) {
  _cardStatusLabel->setText( QString(tr("Card: %1")).arg(scCard.name) );
}

void StatusBar::cardReaderWasRemoved() {
  _readerStatusLabel->setText( tr("Card Reader: Not Connected") );
  _cardStatusLabel->setText( tr("Card: No Card") );
}

void StatusBar::smartCardWasRemoved() {
  _cardStatusLabel->setText( tr("Card: No Card") );
}


