#include "error.h"


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
  va_list args;
  va_start( args, mess);
  char str[4096];
  vsprintf(str, mess, args);
  _errorText = string(str);
  logger().log("Error: %s", _errorText.c_str() );
}



Error::Error(const Error &e){
  _errorText = e._errorText;
  _scError = e._scError;
}

Error::~Error(){
}

Error::operator const void*()const{
  if (_scError == SC_SUCCESS)
    return this;
  return 0;
}


///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////  protected  ////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////  private  ////////////////////////////////////////////

