#ifndef ERROR_H
#define ERROR_H

#include "logger.h"
#include "errors.h"

typedef int SCError;

using namespace std;

class Error{
  public:
    Error();
    Error(SCError errorCode);
    Error(SCError errorCode, const char *mess, ...);
    Error(const Error&);
    virtual ~Error();

    bool hasError(){ return _scError != SC_SUCCESS; }
    bool operator !()const{ return _scError != SC_SUCCESS; }
    bool operator ==(SCError errorCode)const{ return _scError == errorCode; }
    bool operator !=(SCError errorCode)const{ return _scError != errorCode; }
    operator const void*()const;
    
    SCError scError(){ return _scError; }
    string getErrorText() { return _errorText; }
  private:
    SCError _scError;
    string _errorText;
};


#endif
