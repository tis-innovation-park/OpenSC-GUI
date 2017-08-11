#include "error.h"

#include <QByteArray>

Error::Error(){
  _scError = SC_SUCCESS;
  _errorText = "";
}

Error::Error(SCError errorCode){
  _scError = errorCode;
  _errorText = "";
}

Error::Error(SCError errorCode, const char *mess, ...) {
  _scError = errorCode;

  if (errorCode == SC_ERROR_EVENT_TIMEOUT)
    return; // program end

  va_list args;
  va_start(args, mess);
  char str[1024];
  qvsnprintf(str, sizeof(str), mess, args);
  str[sizeof(str)-1] = '\0';
  va_end(args);

  _errorText = string(str);
  logger().log("Error: %s", str);
}


///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////  protected  ////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////  private  ////////////////////////////////////////////

