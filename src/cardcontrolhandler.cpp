#include "cardcontrolhandler.h"


CardControlHandler::CardControlHandler() {  
  _scCard = NULL;
  _scReader = NULL;
  _scCtxt = NULL;
  memset( &_pkcs15ChangePinContext, 0 , sizeof(struct Pkcs15ChangePinContext) );
  memset( &_pkcs15UnblockPinContext, 0 , sizeof(struct Pkcs15UnblockPinContext) );
  resetPersonalData();
  resetSerialData();
  resetX509CertificationData();
}


CardControlHandler::~CardControlHandler() {

}


Error CardControlHandler::init() {
  Error err = intitScContext();
  if(err.hasError()) 
    return err;
      
  return connectCard( false );
}


void CardControlHandler::changePinPkcs15Request( const char *oldpin, const char *newpin ) {
  QMutexLocker ml( &_ccMutex );
  if( _pkcs15ChangePinContext.oldPin ) {
    free( _pkcs15ChangePinContext.oldPin  );
    _pkcs15ChangePinContext.oldPin = 0;
  }
  
  if( _pkcs15ChangePinContext.newPin ) {
    free( _pkcs15ChangePinContext.newPin  );
    _pkcs15ChangePinContext.newPin = 0;
  }
  
  _pkcs15ChangePinContext.pinChangeRequested = true;
  _pkcs15ChangePinContext.oldPin = (u8 *) strdup(oldpin);
  _pkcs15ChangePinContext.newPin = (u8 *) strdup(newpin);
}


void CardControlHandler::unblockPinPkcs15Request( const char *puk, const char *newpin ) {
  QMutexLocker ml( &_ccMutex );
  if( _pkcs15UnblockPinContext.puk ) {
    free( _pkcs15UnblockPinContext.puk  );
    _pkcs15UnblockPinContext.puk = 0;
  }
  
  if( _pkcs15UnblockPinContext.newPin ) {
    free( _pkcs15UnblockPinContext.newPin  );
    _pkcs15UnblockPinContext.newPin = 0;
  }
  
  _pkcs15UnblockPinContext.pinUnblockRequested = true;
  _pkcs15UnblockPinContext.puk = (u8 *) strdup(puk);
  _pkcs15UnblockPinContext.newPin = (u8 *) strdup(newpin);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////// protected  //////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
////////////////////////////// private  ////////////////////////////////////////


Error CardControlHandler::intitScContext() {
  if( _scCard ) {
    sc_disconnect_card( _scCard );
    _scCard = NULL;
  }
  if( _scCtxt ) {
    sc_release_context( _scCtxt );
    _scCtxt = NULL;
  }
  
  sc_context_param_t ctxParam;
  memset(&ctxParam, 0, sizeof(ctxParam));
  ctxParam.ver      = 0;
  ctxParam.app_name = "BuergerKarte";

  int err = sc_context_create(&_scCtxt, &ctxParam);
  if (err) {
    return Error( err, "Failed to establish context: %s\n", sc_strerror(err));
  } 
  return SC_SUCCESS;
}


Error CardControlHandler::connectCard( bool waitForCard ) {
  Error error;
  if( !_scCtxt && !(error = intitScContext()) ) {
    return error;
  }
  
  if( !_scReader && !(error = connectReader( waitForCard ))) {
      return error;
  }
  
  int err;
  if( sc_detect_card_presence( _scReader ) <= 0 ) {
    if ( waitForCard ) {
      unsigned int event;
      logger().log( "Waiting for a card to be inserted...\n");
      int err = sc_wait_for_event(_scCtxt, SC_EVENT_CARD_INSERTED, &_scReader, &event, -1, NULL);
      if ( err ) {
        return Error( err, "Error while waiting for a card: %s\n", sc_strerror(err));
      }
    } else {
      return Error(SC_ERROR_CARD_NOT_PRESENT); 
    }
  }

  logger().log("Connecting to card in reader %s...\n", _scReader->name);
  err = sc_connect_card(_scReader, &_scCard);
  if( err ) {
    _scCard = 0;
    return Error( err, "Failed to connect to card: %s\n", sc_strerror(err) );
  }

  logger().log("Using card driver %s.\n", _scCard->driver->name);

  err = sc_lock(_scCard);
  if (err) {
    sc_disconnect_card(_scCard);
    _scCard = 0;
    return Error( err, "Failed to lock card: %s\n", sc_strerror(err));
  }
  
  emit( smartCardConnected( *_scCard) );
  
  gatherPkcs15PinInfo();
  return SC_SUCCESS;
}


Error CardControlHandler::connectReader( bool waitForReader ) {
  struct sc_reader *foundReader = NULL;
  int err;
  if( sc_ctx_get_reader_count( _scCtxt ) == 0 ) { 
    if( !waitForReader ) {
      _scReader = 0;
      return Error( SC_ERROR_NO_READERS_FOUND, "No smart card readers found.\n");
    }
    
    unsigned int event;
    logger().log("Waiting for a reader to be attached...\n");
    err = sc_wait_for_event( _scCtxt, SC_EVENT_READER_ATTACHED, &foundReader, &event, -1, NULL);

    if ( err ) {
      return Error( err, "Error while waiting for a reader: %s\n", sc_strerror(err));
    }
    
    err = sc_ctx_detect_readers( _scCtxt );
    if ( err ) {
      return Error( err, "Error while refreshing readers: %s\n", sc_strerror(err));
    }
  }
  
  /* Automatically try to skip to a reader with a card if reader not specified */
  for (unsigned int i = 0; i < sc_ctx_get_reader_count(_scCtxt); i++) {
    foundReader = sc_ctx_get_reader(_scCtxt, i);
    if (sc_detect_card_presence( foundReader ) & SC_READER_CARD_PRESENT ) {
      logger().log("Using reader with a card: %s\n", foundReader->name);
      break;
    } else {
      foundReader = NULL;
    }
  }
    
  /* If no reader had a card, default to the first reader */
  if( !foundReader ) {
    foundReader = sc_ctx_get_reader(_scCtxt, 0);
  }
  
  _scReader = foundReader;  
  if ( !_scReader ) {
    return Error( SC_ERROR_NO_READERS_FOUND, "No valid readers found (%d reader(s) detected)\n", sc_ctx_get_reader_count(_scCtxt) );
  }
  
  emit( cardReaderConnected( *_scReader) );
  return SC_SUCCESS;
}


Error CardControlHandler::verifyCurrentSmartCard() {
  if( !_scReader && _scCard ) {
    cleanupSmartCard();
    return Error(SC_ERROR_CARD_NOT_PRESENT);
  }
  
  if( !_scCard )
    return Error(SC_ERROR_CARD_NOT_PRESENT);
  
  int cardFlags = sc_detect_card_presence( _scCard->reader );
  if( cardFlags == 0 || cardFlags & SC_READER_CARD_CHANGED ) {
    logger().log("Card %s has been removed from reader\n", _scCard->name);
    emit( smartCardRemoved(*_scCard) );
    resetPersonalData();
    cleanupSmartCard();
    return Error(SC_ERROR_CARD_NOT_PRESENT);
  } else if ( cardFlags < 0 ) {
    emit( smartCardRemoved(*_scCard) );
    resetPersonalData();
    cleanupSmartCard();
    return Error( SC_ERROR_CARD_NOT_PRESENT, "Error detecting card precence: %s\n", sc_strerror(cardFlags));
  } else {
    return SC_SUCCESS;
  }
}


Error CardControlHandler::verifyCurrentCardReader() {
  if( !_scReader ) {
    Error(SC_ERROR_NO_READERS_FOUND);
  }

  if( sc_ctx_detect_readers( _scCtxt ) == SC_ERROR_NO_READERS_FOUND || sc_ctx_get_reader_count( _scCtxt ) == 0 || !sc_ctx_get_reader_by_name( _scCtxt, _scReader->name ) ) { 
    logger().log("Reader %s has been removed\n", _scReader->name);
    if( _scCard )
      cleanupSmartCard(); 
    
    emit(cardReaderRemoved( *_scReader ));
    resetPersonalData();
    _scReader = 0;
    sc_release_context( _scCtxt );
    _scCtxt = 0;
    return Error(SC_ERROR_NO_READERS_FOUND);
  }
  return SC_SUCCESS;
}


Error CardControlHandler::cleanupSmartCard() {
  sc_unlock( _scCard );
  sc_disconnect_card( _scCard );
  _scCard = 0;
  return SC_SUCCESS;
}


Error CardControlHandler::gatherPkcs15PinInfo() {
  struct sc_pkcs15_card *p15card = NULL;
  sc_pkcs15_object_t *pin_obj = NULL;
  sc_pkcs15_auth_info_t *pinfo = NULL;
  
  int err = sc_pkcs15_bind(_scCard, NULL, &p15card);
  if( err ) {
    return Error( err, "PKCS#15 binding failed: %s\n", sc_strerror(err));
  }
  
  getPersonalDatafromCard( p15card );
  getSerialDatafromCard( p15card );
  getX509CertificateDatafromCard( p15card );
  
  //Getting pin information
  sc_pkcs15_object_t *objs[32];
  int res = sc_pkcs15_get_objects(p15card, SC_PKCS15_TYPE_AUTH_PIN, objs, 32);
  if (res < 0) {
    sc_pkcs15_unbind(p15card);
    return Error( err, "PIN code enumeration failed: %s\n", sc_strerror(res));
  } else if (res == 0) {
    sc_pkcs15_unbind(p15card);
    return Error( SC_ERROR_INVALID_PIN_REFERENCE, "No PIN codes found.\n");
  }
  pin_obj = objs[0];
  
//   for (unsigned i=0; i<res; i++ ){
//     sc_pkcs15_object_t *currObj = objs[i];
//     pinfo = (sc_pkcs15_auth_info_t *)currObj->data;
//     printf("%s\n", currObj->label );
//   }
// 
  pinfo = (sc_pkcs15_auth_info_t *) pin_obj->data;
  if (pinfo->auth_type != SC_PKCS15_PIN_AUTH_TYPE_PIN) {
    sc_pkcs15_unbind(p15card);
    return Error( SC_ERROR_INVALID_PIN_REFERENCE );
  }
  
  emit( pkcs15PinInfoGathered(*pinfo) );
  sc_pkcs15_unbind(p15card);  
  return SC_SUCCESS;
}


Error CardControlHandler::changePinPkcs15( const u8 *oldpin, const u8 *newpin) {
  struct sc_pkcs15_card *p15card = NULL;
  sc_pkcs15_object_t *pin_obj = NULL;
  sc_pkcs15_auth_info_t *pinfo = NULL;
  
  int err = sc_pkcs15_bind(_scCard, NULL, &p15card);
  if( err ) {
    return Error( err, "PKCS#15 binding failed: %s\n", sc_strerror(err));
  }
  
  //Getting pin information
  sc_pkcs15_object_t *objs[32];
  int res = sc_pkcs15_get_objects(p15card, SC_PKCS15_TYPE_AUTH_PIN, objs, 32);
  if (res < 0) {
    return Error( err, "PIN code enumeration failed: %s\n", sc_strerror(res));
  } else if (res == 0) {
    sc_pkcs15_unbind(p15card);
    return Error( SC_ERROR_INVALID_PIN_REFERENCE, "No PIN codes found.\n");
  }
  pin_obj = objs[0];

  pinfo = (sc_pkcs15_auth_info_t *) pin_obj->data;
  if (pinfo->auth_type != SC_PKCS15_PIN_AUTH_TYPE_PIN){ 
    sc_pkcs15_unbind(p15card);
    return Error( SC_ERROR_INVALID_PIN_REFERENCE );  
  }
 
  if (pinfo->tries_left != -1) {
    if (pinfo->tries_left != pinfo->max_tries) {
      if (pinfo->tries_left == 0) {
        sc_pkcs15_unbind(p15card);
        return Error( SC_ERROR_AUTH_METHOD_BLOCKED, "PIN code blocked!\n");
      } else {
        logger().log("%d PIN tries left.\n", pinfo->tries_left);
      }
    }
  }
  
  if (strlen( (const char*)oldpin) < pinfo->attrs.pin.min_length) {
    logger().log("PIN code too short, try again.\n");
    sc_pkcs15_unbind(p15card);
    return Error( SC_ERROR_PIN_CODE_INCORRECT);
  } else if ( strlen( (const char*)oldpin) > pinfo->attrs.pin.max_length) {
    logger().log("PIN code too long, try again.\n");
    sc_pkcs15_unbind(p15card);
    return Error( SC_ERROR_PIN_CODE_INCORRECT);
  }
  
  if (strlen( (const char*)newpin) < pinfo->attrs.pin.min_length) {
    logger().log("PIN code too short, try again.\n");
    sc_pkcs15_unbind(p15card);
    return Error( SC_ERROR_PIN_CODE_INCORRECT);
  } else if (strlen( (const char*)newpin) > pinfo->attrs.pin.max_length) {
    logger().log("PIN code too long, try again.\n");
    sc_pkcs15_unbind(p15card);
    return Error( SC_ERROR_PIN_CODE_INCORRECT);
  }
  
  Error error;
  err = sc_pkcs15_change_pin(p15card, pin_obj, (const u8*) oldpin, oldpin ? strlen( (const char*) oldpin) : 0 , (const u8*) newpin,  newpin ? strlen( (const char*)newpin) : 0);
  if (err == SC_ERROR_PIN_CODE_INCORRECT) {
    error = Error( err, "PIN code incorrect; tries left: %d\n", pinfo->tries_left);
  } else if (err) {
    error = Error( err, "PIN code change failed: %s\n", sc_strerror(err));
  } else {
    logger().log("PIN code changed successfully.\n");
  }
  
  sc_pkcs15_unbind(p15card);
  return error;
}


Error CardControlHandler::unblockPinPkcs15( const u8 *puk, const u8 *pin) {
  struct sc_pkcs15_card *p15card = NULL;
  struct sc_pkcs15_auth_info *pinfo = NULL;
  sc_pkcs15_object_t *pin_obj;
  
  int err = sc_pkcs15_bind(_scCard, NULL, &p15card);
  if( err ) {
    return Error( err, "PKCS#15 binding failed: %s\n", sc_strerror(err));
  }
  
  //Getting pin information
  sc_pkcs15_object_t *objs[32];
  int res = sc_pkcs15_get_objects(p15card, SC_PKCS15_TYPE_AUTH_PIN, objs, 32);
  if (res < 0) {
    return Error( res, "PIN code enumeration failed: %s\n", sc_strerror(res));
  } else if (res == 0) {
    sc_pkcs15_unbind(p15card);
    return Error( SC_ERROR_INVALID_PIN_REFERENCE, "No PIN codes found.\n");
  }
  pin_obj = objs[0];
  
  pinfo = (sc_pkcs15_auth_info_t *) pin_obj->data;
  if (pinfo->auth_type != SC_PKCS15_PIN_AUTH_TYPE_PIN) {
    sc_pkcs15_unbind(p15card);
    return Error(SC_ERROR_INVALID_PIN_REFERENCE);  
  }

  if (strlen( (const char*)puk) < pinfo->attrs.pin.min_length) {
    logger().log("PUK code too short, try again.\n");
    sc_pkcs15_unbind(p15card);
    return Error(SC_ERROR_PIN_CODE_INCORRECT);
  } else if (strlen( (const char*)puk) > pinfo->attrs.pin.max_length) {
    logger().log("PUK code too long, try again.\n");
    sc_pkcs15_unbind(p15card);
    return Error(SC_ERROR_PIN_CODE_INCORRECT);
  }
  
  if (strlen( (const char*)pin) < pinfo->attrs.pin.min_length) {
    logger().log("PIN code too short, try again.\n");
    sc_pkcs15_unbind(p15card);
    return Error(SC_ERROR_PIN_CODE_INCORRECT);
  } else if (strlen( (const char*)pin) > pinfo->attrs.pin.max_length) {
    logger().log("PIN code too long, try again.\n");
    sc_pkcs15_unbind(p15card);
    return Error(SC_ERROR_PIN_CODE_INCORRECT);
  }
  
  Error error;
  err = sc_pkcs15_unblock_pin(p15card, pin_obj, puk, puk ? strlen( (const char*) puk) : 0, pin, pin ? strlen( (const char*)pin) : 0);
  if (err == SC_ERROR_PIN_CODE_INCORRECT) {  
    error = Error( err, "PUK code incorrect; tries left: %d\n", pinfo->tries_left);
  } else if (err) {  
    error = Error( err, "PIN unblocking failed: %s\n", sc_strerror(err));
  } else {
   logger().log("PIN successfully unblocked.\n");
  }
  
  sc_pkcs15_unbind(p15card);
  return error;
}


Error CardControlHandler::getPersonalDatafromCard( sc_pkcs15_card *p15card ) {
  resetPersonalData();
  
  int r, i, count;
  struct sc_pkcs15_object *objs[32];
  struct sc_object_id oid;
  string filename = "EF_DatiPersonali";

  r = sc_pkcs15_get_objects(p15card, SC_PKCS15_TYPE_DATA_OBJECT, objs, 32);
  if (r < 0) {
    return Error( r, "Data object enumeration failed: %s\n", sc_strerror(r));
  }
  
  count = r;
  for (i = 0; i < count; i++) {
    struct sc_pkcs15_data_info *cinfo = (struct sc_pkcs15_data_info *) objs[i]->data;
    struct sc_pkcs15_data *data_object = NULL;
    
    if (!sc_format_oid(&oid, filename.c_str()))   {
      if (!sc_compare_oid(&oid, &cinfo->app_oid))
        continue;
    } else   {
      if (strcmp(filename.c_str(), cinfo->app_label) && strcmp(filename.c_str(), objs[i]->label))
        continue;
    }
    
    
    //DATA FIELD HAS BEEN FOUND
//     logger().log("Reading data object with label '%s'\n", filename.c_str());
//     logger().log("PATH %s \n", sc_print_path(&cinfo->path));
//     logger().log("APP LABEL %s \n", cinfo->app_label);
//     logger().log("DATA: %s \n", cinfo->data.value);

    //printf("Reading data object with label '%s'\n", filename.c_str());
    r = sc_pkcs15_read_data_object(p15card, cinfo, &data_object);
    if (r) {
      return Error( r, "Data object read failed: %s\n", sc_strerror(r));
    }
    u8* buff = data_object->data;
    size_t len = data_object->data_len;
     
    
    /*
    * Bytes 0-5 contain the ASCII encoding of the following TLV strcture's total size, in base 16.
    */
    const int personalDataMaxLen = len;
        
    unsigned int tlv_length_size = 6;
    char *fileData = (char*) &buff[tlv_length_size];
    int fileSize = hexToInt( (char*)buff, tlv_length_size);
    if(fileSize < 0)
      return Error( SC_ERROR_INVALID_DATA, "Personal Data file has invalid size %d.\n", fileSize);
    
    //Just to make sure we are not exceeding max data size
    if( fileSize > (int) personalDataMaxLen - (int) tlv_length_size )
      fileSize = personalDataMaxLen - tlv_length_size;
        
    int characterOffset=0; /* offset inside the file */
    int fieldNr; /* field number */

    for(fieldNr=0; fieldNr < pd_ExpatNotes+1; fieldNr++) {
      int fieldSize;
      
      /* Don't read beyond the allocated buffer */
      if(characterOffset > fileSize) 
        return Error( SC_ERROR_INVALID_DATA, "Max File Size exceeded while gathering personal data\n");

      fieldSize = hexToInt( (char*) &fileData[characterOffset], 2);
      if((fieldSize < 0) || (fieldSize + characterOffset > fileSize))
        return Error( SC_ERROR_INVALID_DATA, "Invalid Field Size detected while gathering personal data\n");

      characterOffset += 2;

      if(fieldSize >= (int) sizeof( _personalData.dataFields[fieldNr].value) )
        return Error( SC_ERROR_INVALID_DATA, "Field Size exceeds max defined field size\n");

      _personalData.dataFields[fieldNr].len = fieldSize;
      strncpy(_personalData.dataFields[fieldNr].value, &fileData[characterOffset], fieldSize);
      _personalData.dataFields[fieldNr].value[fieldSize] = '\0';
//       logger().log("%s\n",_personalData.dataFields[fieldNr].value);
      characterOffset += fieldSize;
    }
    _personalData.isValid = true;
    emit(personalDataGathered( _personalData ));
    sc_pkcs15_free_data_object(data_object);
    return SC_SUCCESS;
  }
  
  return Error( SC_ERROR_FILE_NOT_FOUND, "PersonalData could not be found.\n");  
}


Error CardControlHandler::getSerialDatafromCard( sc_pkcs15_card* p15card) {
  resetSerialData();
  
  string serialPath = CARD_SERIAL_PATH;
  sc_path_t scPath;
  u8 serial[CARD_SERIAL_LENGTH + 1];
  int bytes;
  
  sc_format_path(serialPath.c_str(), &scPath);
  bytes = readDataFromFile(p15card, &scPath, serial, CARD_SERIAL_LENGTH);
  if( bytes < 0 || bytes > CARD_SERIAL_LENGTH ) 
    return Error( SC_ERROR_INVALID_DATA, "Inavlid number of bytes have been read %d\n", bytes);  
  serial[bytes] = '\0';
  
  strncpy( _serialData.serial, (const char*) serial, sizeof(_serialData.serial) );
  _serialData.isValid = true;
  logger().log("CARD SERIAL: %s\n", _serialData.serial);
  emit(serialDataGathered( _serialData ));
  return SC_SUCCESS;
}


Error CardControlHandler::getX509CertificateDatafromCard(  sc_pkcs15_card *p15card  ) {
  resetX509CertificationData();
  int err = 0;
  int cns0_secenv = 0x01;
  string certPath = "3F0011001101";
  
  sc_pkcs15_cert_info_t info;
  sc_pkcs15_object_t obj;      
  sc_pkcs15_cert_t *cert;
  sc_path_t scCertPath; 
  sc_pkcs15_id_t certId;
  memset(&info, 0, sizeof(info));
  memset(&obj, 0, sizeof(obj));

  certId.len = 1;
  certId.value[0] = cns0_secenv;
  
  sc_format_path(certPath.c_str(), &scCertPath);

  
  //Check if the certificate file is available
  err = sc_select_file(p15card->card, &scCertPath, NULL);
  if (err != SC_SUCCESS) {
    return Error( err, "Could not find X509 certificate\n", sc_strerror(err));
  }

  info.id = certId;
  info.authority = 0;
  info.path = scCertPath;
  
  //Try to read the certificate
  err = sc_pkcs15_read_certificate(p15card, &info, &cert);
  if (err != SC_SUCCESS) {
    return Error( err, "Could not read X509 certificate\n", sc_strerror(err));
  }
  
  Error error = _x509CertificateHandler.getX509DataFromCertificate( *cert, &_x509Data );
  if( !error.hasError() )
    emit( x509CertificateDataGathered(_x509Data) );
  return error;
}


int CardControlHandler::readDataFromFile(const sc_pkcs15_card_t *p15card, const sc_path_t *path, u8 *buf, const size_t buflen) {
  int sc_res;
  sc_res = sc_select_file(p15card->card, path, NULL);
  if(sc_res != SC_SUCCESS)
    return sc_res;

  sc_res = sc_read_binary(p15card->card, 0, buf, buflen, 0);
  return sc_res;
}


int CardControlHandler::hexToInt(char *src, unsigned int len) {
    char hex[16];
    char *end;
    int res;

    if(len >= sizeof(hex) )
      return -1;
    
    strncpy(hex, src, len+1);
    hex[len] = '\0';
     
    res = strtol(hex, &end, 0x10);
     if(end != (char*)&hex[len])
             return -1;
    return res;
}


void CardControlHandler::resetPersonalData() {
  memset(&_personalData, 0 , sizeof(PersonalData) );
}

void CardControlHandler::resetSerialData() {
  memset(&_serialData, 0 , sizeof(SerialData) );
}

void CardControlHandler::resetX509CertificationData() {
  memset( &_x509Data, 0 , sizeof(X509CertificateHandler::X509CertificateData) );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// slots  ////////////////////////////////////////


void CardControlHandler::timerEvent(QTimerEvent *event) {
  Q_UNUSED(event)

  if( !_scCard ) {
    connectCard( true );
  }
  
  if( _scReader ) {
    verifyCurrentCardReader();
  }
  
  if( _scCard ) {
    verifyCurrentSmartCard();
  }
  
  if( _pkcs15UnblockPinContext.pinUnblockRequested ) {
    QMutexLocker ml( &_ccMutex );
    Error err = unblockPinPkcs15( _pkcs15UnblockPinContext.puk, _pkcs15UnblockPinContext.newPin );
    _pkcs15UnblockPinContext.pinUnblockRequested = false;
//     free(_pkcs15UnblockPinContext.newPin);
//     free(_pkcs15UnblockPinContext.puk);
    emit(pksc15PinUnblockDone(err));
  }
  
  if( _pkcs15ChangePinContext.pinChangeRequested ) {
    QMutexLocker ml( &_ccMutex );
    Error err = changePinPkcs15( _pkcs15ChangePinContext.oldPin, _pkcs15ChangePinContext.newPin );
    _pkcs15ChangePinContext.pinChangeRequested = false;
//     free(_pkcs15ChangePinContext.newPin);
//     free(_pkcs15ChangePinContext.oldPin);
    emit(pksc15PinChangeDone(err));
  }
}




