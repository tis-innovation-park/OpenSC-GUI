#include "x509certificatehandler.h"
#include <QTimer>


X509CertificateHandler::X509CertificateHandler() {  
}


X509CertificateHandler::~X509CertificateHandler() {

}

Error X509CertificateHandler::getX509DataFromCertificate(const sc_pkcs15_cert_t& cert, X509CertificateHandler::X509CertificateData* certData) {
  memset( certData, 0 , sizeof(X509CertificateHandler::X509CertificateData) );
  const u8 *encodedData = cert.data.value;
  X509 *x509Cert = 0;

  x509Cert = d2i_X509(NULL, &encodedData, cert.data.len);
  if( !x509Cert ) 
    return Error( SC_ERROR_OBJECT_NOT_VALID, "Could convert X509 certificate via openssl\n");
  
 
  //Gather Issuer Data
  int length = X509_NAME_entry_count( x509Cert->cert_info->issuer );
  for (int i = 0; i < length; i++) {
    X509_NAME_ENTRY *entry = X509_NAME_get_entry(x509Cert->cert_info->issuer , i);
    handleEntry( entry, certData );
  }
  
  
  //Gather Serial Data
  BIGNUM *bn = ASN1_INTEGER_to_BN(x509Cert->cert_info->serialNumber, NULL);
  char *hex = BN_bn2hex(bn);
  snprintf(certData->serialNumber, CERTIFICATE_TEXT_LENGTH, "%s", hex);
  BN_free(bn);
  OPENSSL_free(hex);
  //cout<<"SERIAL DATA"<<certData->serialNumber<<endl;
  
  
  //Gather Validity Data
//   cout<<"TIME 1: "<<x509Cert->cert_info->validity->notBefore->data<<endl;
//   cout<<"TIME 2: "<<x509Cert->cert_info->validity->notAfter->data<<endl;
  parseTimeTFromASN1Time(x509Cert->cert_info->validity->notBefore, &certData->validity.notBefore );
  parseTimeTFromASN1Time(x509Cert->cert_info->validity->notAfter, &certData->validity.notAfter );
//   printf ("Before time is: %s", ctime (&certData->validity.notBefore));
//   printf ("After time is: %s", ctime (&certData->validity.notAfter));
  
  certData->isValid = true;
  return SC_SUCCESS;
}


void X509CertificateHandler::handleEntry(const X509_NAME_ENTRY* entry, X509CertificateHandler::X509CertificateData* certData) {
  ASN1_OBJECT* entryObject = X509_NAME_ENTRY_get_object( (X509_NAME_ENTRY*)  entry );
    
  char buff[256];
  OBJ_obj2txt(buff, 255, entryObject, 0);
  unsigned char* value = ASN1_STRING_data(X509_NAME_ENTRY_get_data( (X509_NAME_ENTRY*) entry ) );
  cout<<"Entry: "<<buff<<"    Value: "<<value<<endl;
  
  string entryName = buff;
  if( entryName.find("countryName") != string::npos  )
    snprintf(certData->issuer.countryName, CERTIFICATE_TEXT_LENGTH, "%s", value);
  else if( entryName.find("organizationName" ) != string::npos   )
    snprintf(certData->issuer.organizationName, CERTIFICATE_TEXT_LENGTH, "%s", value);
  else if( entryName.find("organizationalUnitName" ) != string::npos   )
    snprintf(certData->issuer.organizationalUnitName, CERTIFICATE_TEXT_LENGTH, "%s", value);
  else if( entryName.find("commonName" ) != string::npos   )
    snprintf(certData->issuer.commonName, CERTIFICATE_TEXT_LENGTH, "%s", value);
}


Error X509CertificateHandler::parseTimeTFromASN1Time(const ASN1_TIME* time, time_t *timeOut ) {
  *timeOut = 0;
  struct tm t;
  const char* str = (const char*) time->data;
  size_t i = 0;

  memset(&t, 0, sizeof(t));

  if (time->type == V_ASN1_UTCTIME) {
    /* two digit year */
    t.tm_year = (str[i++] - '0') * 10;
    t.tm_year += (str[i++] - '0');
    if (t.tm_year < 70)
      t.tm_year += 100;
  } else if (time->type == V_ASN1_GENERALIZEDTIME) {/* four digit year */
    t.tm_year = (str[i++] - '0') * 1000;
    t.tm_year+= (str[i++] - '0') * 100;
    t.tm_year+= (str[i++] - '0') * 10;
    t.tm_year+= (str[i++] - '0');
    t.tm_year -= 1900;
  }
  
  t.tm_mon = (str[i++] - '0') * 10;
  t.tm_mon += (str[i++] - '0') - 1;
  t.tm_mday = (str[i++] - '0') * 10;
  t.tm_mday += (str[i++] - '0');
  t.tm_hour = (str[i++] - '0') * 10;
  t.tm_hour += (str[i++] - '0');
  t.tm_min = (str[i++] - '0') * 10;
  t.tm_min += (str[i++] - '0');
  t.tm_sec  = (str[i++] - '0') * 10;
  t.tm_sec += (str[i++] - '0');

  /* Note: we did not adjust the time based on time zone information */
  *timeOut = mktime(&t);
  return SC_SUCCESS;
}

