#include "SkXfermode.h"
#include "SkXfermode_proccoeff.h"

// The prototype below is for Clang
extern SkProcCoeffXfermode* SkPlatformXfermodeFactory(const ProcCoeff& rec,
                                                      SkXfermode::Mode mode);

SkProcCoeffXfermode* SkPlatformXfermodeFactory(const ProcCoeff& rec,
                                               SkXfermode::Mode mode) {
    return NULL;
}
