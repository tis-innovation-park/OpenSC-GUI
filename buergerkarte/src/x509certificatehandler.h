#ifndef X509_CERTIFICATE_HANDLER_H
#define X509_CERTIFICATE_HANDLER_H

#include <QTimer>

#include <iostream>

#include "error.h"
#include "opensc.h"
#include "cardctl.h"
#include "pkcs15.h"
#include "cards.h"

#include <openssl/x509v3.h>
#define CERTIFICATE_TEXT_LENGTH 256

using namespace std;


class X509CertificateHandler {
  public:
    struct X509CertificateData {
      char serialNumber[CERTIFICATE_TEXT_LENGTH];
      
      struct {
       char countryName[CERTIFICATE_TEXT_LENGTH];
       char organizationName[CERTIFICATE_TEXT_LENGTH];
       char organizationalUnitName[CERTIFICATE_TEXT_LENGTH];
       char commonName[CERTIFICATE_TEXT_LENGTH];
      } issuer;
      
      struct {
        time_t notBefore;
        time_t notAfter;
      } validity;
      
      bool isValid;
    };
    
    
    X509CertificateHandler();
    virtual ~X509CertificateHandler();
    
    Error getX509DataFromCertificate( const sc_pkcs15_cert_t& cert, X509CertificateData* certData );
    
  private:
    void handleEntry(const X509_NAME_ENTRY* entry, X509CertificateData* certData );
    Error parseTimeTFromASN1Time(const ASN1_TIME* time, time_t *timeOut);
};

#endif

