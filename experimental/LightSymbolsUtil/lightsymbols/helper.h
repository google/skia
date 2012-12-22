#include <stdlib.h>
#define CANVAS_PATH "CANVAS_PATH"

class SkFile {
  FILE* file;
  bool busted;
  char* sz;
  mutable int i;

public:
  SkFile(unsigned long id) {
    file = NULL;
    busted = false;
    sz = new char[100000];
    set(id);
    i = 100;
  }

  ~SkFile() {
    delete sz;
    if (file) {
      fclose(file);
    }
  }

  void set(unsigned long id) {
    if (busted) {
      return;
    }

    if (file == NULL) {
      char sz[10000];
      sprintf(sz, "%s\\%ul.callstacks.txt", getenv(CANVAS_PATH), id);
      file = fopen(sz, "a");
      if (file == NULL) {
        busted = true;
      }
      fprintf(file, "\n\n\nNEW SESSION, just coliding ids ... should generate a new file ideally ...  \n\n\n");
    }
  }

  void appendLine(const char* sz) const {
    if (busted) {
      return;
    }

    fprintf(file, "%s\n", sz);
  }

  void dump(bool flush = false) const {
    if (busted) {
      return;
    }

    LightSymbol::GetCallStack(sz, 100000, " >- ");
    appendLine(sz);

    i--;

    if (i < 0 || flush) {
      i = 100;
      fflush(file);
    }
  }
};
