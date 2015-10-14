#ifndef	re2c_globals_h
#define	re2c_globals_h

#include "tools/re2c/basics.h"

extern const char *fileName;
extern char *outputFileName;
extern int sFlag;
extern int bFlag;
extern int dFlag;
extern int iFlag;
extern int bUsedYYAccept;
extern unsigned int oline;
extern unsigned int maxFill;
extern int vFillIndexes;
extern unsigned char *vUsedLabels;
extern unsigned int vUsedLabelAlloc;

extern unsigned char asc2ebc[256];
extern unsigned char ebc2asc[256];

extern unsigned char *xlat, *talx;

char *mystrdup(const char *str);

#endif
