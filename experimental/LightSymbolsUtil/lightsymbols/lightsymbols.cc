#include "lightsymbols.h"

LightSymbol::PLightSymbol LightSymbol::lsFrames[1000];
HANDLE LightSymbol::handleFrames[1000];
SZ* LightSymbol::fileNames;
bool LightSymbol::busted = false;


LightSymbol::LightSymbol(const char* sym, int fileId, int lineNumber) {
  while (busted) {
    busted = busted;
  }
  this->sym = sym;
  this->fileId = fileId;
  this->lineNumber = lineNumber;

  LightSymbol** container = getThreadFrameContainer();

  parentFrame = *container;
  *container = this; // shortcut for get+set current frame
}

LightSymbol::~LightSymbol() {

// assert  if (GetCurrentFrame() != this) {

  SetCurrentFrame(parentFrame);
}

bool LightSymbol::GetCallStack(char* sz, int len, const char* separator) {
  LightSymbol* ls = GetCurrentFrame();
  if (ls == 0) {
    return false;
  } else {
    return ls->GetCallStackCore(sz, len, separator);
  }
}

LightSymbol** LightSymbol::getThreadFrameContainer() {
  //pthread_t t = pthread_self();
  HANDLE h = (HANDLE)GetCurrentThreadId(); // f, keep handle so I don't have to recompie tyhe whole app; update toi DWORD one I really need changes in header file
  int i = 0; 
  while (handleFrames[i] != h && handleFrames[i] != NULL && i < 1000 - 1) {
    i++;
  }
  if (handleFrames[i] == h) {
    return &lsFrames[i];
  }
  handleFrames[i] = h;
  return &lsFrames[i];
}

bool LightSymbol::GetCallStackCore(char* sz, int len, const char* separator) const {
  if (busted) {
    return false;
  }
  if (fileNames == NULL) { // f multithreading synchr
    FILE* log = fopen("d:\\edisonn\\log.txt", "wt");

    if (log) { fprintf(log, "build\n");fflush(log); }
    
    char szLine[10000];
    FILE* file = fopen(getenv(LIGHT_SYMBOLS_FILE), "rt");
    if (file == NULL) {
      busted = true;
      return false;
    }

    const char* trimed;
      
    // count number of lines
    int id;
    int entries = 0;
    while (true) {
      id = -1;
      if (fscanf(file, "%i", &id) == 0) break;
      if (id == -1) break;
      if (entries <= id + 1) {
        entries = id + 1;
      }
      *szLine = '\0';
      fgets(szLine, 10000, file);
      trimed = trim(szLine);
    }

    fclose(file);
    file = fopen(getenv(LIGHT_SYMBOLS_FILE), "rt");
    if (file == NULL) {
      busted = true;
      return false; // f this
    }

    if (log) { fprintf(log, "entries: %i\n", entries);fflush(log); }

    SZ* __fileNames = new SZ[entries];

    while (true) {
      id = -1;
      if (fscanf(file, "%i", &id) == 0) break;
      if (id == -1) break;
      *szLine = '\0';
      fgets(szLine, 10000, file);
      trimed = trim(szLine);

      if (log) { fprintf(log, "%i, %s", id, trimed); }

      // ass u me the file is correct

      __fileNames[id] = new char[strlen(trimed) + 1];
      if (log) { fprintf(log, " - ");fflush(log); }
      strcpy(__fileNames[id], trimed);
      if (log) { fprintf(log, " _ \n");fflush(log); }
    }
    fclose(file);
    fileNames = __fileNames;
    if (log) { fclose(log); }
  }

  const LightSymbol* ls = this;
  char* szOut = sz;
  // f security
  while (ls != NULL && len > 1000) {
    sprintf(szOut, "%s, %s:%i%s", ls->sym, fileNames[ls->fileId], ls->lineNumber, separator);
    while (*szOut && len > 0) {
      szOut++;
      len--;
    }
    ls = ls->parentFrame;
  }

  int more = 0;
  while (ls != NULL) {
    ls = ls->parentFrame;
  }

  if (more > 0) {
    sprintf(szOut, " ... %i more frames. allocate more memory!", more);
  }

  return true;
}

LightSymbol* LightSymbol::GetCurrentFrame() {
  return *getThreadFrameContainer();
}

void LightSymbol::SetCurrentFrame(LightSymbol* ls) {
  *getThreadFrameContainer() = ls;
}

const char* LightSymbol::trim(char* sz) {
  if (sz == NULL) return NULL;
    
  while (*sz == ' ' || *sz == '\t' || *sz == '\r' || *sz == '\n' || *sz == ',')
    sz++;

  if (*sz == '\0') return sz;

  int len = strlen(sz);
  char* start = sz;
  sz = sz + (len - 1);

  while (sz >= start && (*sz == ' ' || *sz == '\t' || *sz == '\r' || *sz == '\n' || *sz == ',')) {
    *sz = '\0';
    sz--;
  }

  return start;
}
