#ifndef __LIGHT_SYMBOLS__
#define __LIGHT_SYMBOLS__
#define LS_TRACE(functionName,fileId,lineNumber) LightSymbol __lstr(functionName,fileId,lineNumber);

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#include <pthread.h>
#include <windows.h>

typedef char* SZ;

#define LIGHT_SYMBOLS_FILE "LIGHT_SYMBOLS_FILE"

class LightSymbol {
  const char* sym;
  int fileId;
  int lineNumber;

  LightSymbol* parentFrame;

  typedef LightSymbol* PLightSymbol;

  static PLightSymbol lsFrames[1000];
  static HANDLE handleFrames[1000];
  static SZ* fileNames;
  static bool busted;

public:
  LightSymbol(const char* sym, int fileId, int lineNumber);

  ~LightSymbol();

  static bool GetCallStack(char* sz, int len, const char* separator);

private:

  static LightSymbol** getThreadFrameContainer();

  bool GetCallStackCore(char* sz, int len, const char* separator) const ;

  static LightSymbol* GetCurrentFrame() ;

  static void SetCurrentFrame(LightSymbol* ls) ;

  static const char* trim(char* sz) ;
};

#endif
