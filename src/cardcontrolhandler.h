#ifndef CARD_CONTROL_HANDLER_H
#define CARD_CONTROL_HANDLER_H

#include <iostream>

#include "error.h"
#include "opensc.h"
#include "cardctl.h"
#include "pkcs15.h"
#include "cards.h"

#include "x509certificatehandler.h"

using namespace std;

#define CARD_SERIAL_LENGTH 16
#define CARD_SERIAL_PATH "10001003"

#define POLLING_INTERVAL 250


class CardControlHandler : public QObject {
  Q_OBJECT
  public:
    enum PersonalDataFields {
      pd_IssuerCode = 0,
      pd_IssuingDate,
      pd_ExpiryDate,
      pd_LastName,
      pd_FirstName,
      pd_BirthDate,
      pd_Sex,
      pd_Height,
      pd_CodiceFiscale,
      pd_CitizenshipCode,
      pd_BirthTownshipCode,
      pd_BirthCountry,
      pd_BirthCertificate,
      pd_ResidenceTownshipCode,
      pd_ResidenceAddress,
      pd_ExpatNotes
    };
    
    struct PersonalData {
      struct {
        int len; 
        char value[256];
      } dataFields[pd_ExpatNotes+1];
      bool isValid;
    };
    
    struct SerialData {
      char serial[CARD_SERIAL_LENGTH + 1];
      bool isValid;
    };
    
    


    
    struct Pkcs15PinInfo {
      int minLength;
      int maxLength;
      bool isValid;
    };
    
    CardControlHandler();
    ~CardControlHandler();

    bool init();
    
    void changePinPkcs15Request( const char *oldpin, const char *newpin );
    void unblockPinPkcs15Request( const char *puk, const char *newpin );
    
  protected:
    sc_context_t *_scCtxt;
    sc_reader_t *_scReader;
    sc_card_t *_scCard;

    void timerEvent(QTimerEvent *event);
    
  private:
    struct Pkcs15ChangePinContext {
      bool pinChangeRequested;
      u8* oldPin;
      u8* newPin;
    } _pkcs15ChangePinContext;
    
    struct Pkcs15UnblockPinContext {
      bool pinUnblockRequested;
      u8* puk;
      u8* newPin;
    } _pkcs15UnblockPinContext;
    
    
    QMutex _ccMutex;
    
    Error initScContext();
    Error connectCard( bool waitForCard = true );
    Error connectReader( bool waitForReader = true );
    Error verifyCurrentSmartCard();
    Error verifyCurrentCardReader();
    Error cleanupScContext();
    Error cleanupSmartCard();
    Error gatherPkcs15PinInfo();
    
    Error changePinPkcs15( const u8 *oldpin, const u8 *newpin);
    Error unblockPinPkcs15( const u8 *puk, const u8 *pin);
    
 
      
    Error getPersonalDatafromCard( sc_pkcs15_card *p15card );
    Error getSerialDatafromCard( sc_pkcs15_card *p15card );
    Error getX509CertificateDatafromCard(  sc_pkcs15_card *p15card  );
    
    
    int readDataFromFile(const sc_pkcs15_card_t *p15card, const sc_path_t *path, u8 *buf, const size_t buflen);
    int hexToInt(const char *src, unsigned int len);
    
    
    PersonalData _personalData;
    SerialData _serialData;
    X509CertificateHandler::X509CertificateData _x509Data;
    X509CertificateHandler _x509CertificateHandler;
    
    void resetPersonalData();
    void resetSerialData();
    void resetX509CertificationData();
    
  public slots:
    void startPolling() { startTimer(POLLING_INTERVAL); }

  signals:
    void cardReaderConnected( sc_reader_t scReader );
    void smartCardConnected( sc_card_t scCard );
    void cardReaderRemoved( sc_reader_t scReader );
    void smartCardRemoved( sc_card_t scCard );
    void pkcs15PinInfoGathered(sc_pkcs15_auth_info_t pinInfo);
    void personalDataGathered(CardControlHandler::PersonalData personalData);
    void serialDataGathered(CardControlHandler::SerialData serialData);
    void x509CertificateDataGathered(X509CertificateHandler::X509CertificateData x509Data);
    void pksc15PinChangeDone( Error err );
    void pksc15PinUnblockDone( Error err );
};

#endif

