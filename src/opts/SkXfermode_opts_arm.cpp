#include "SkXfermode.h"
#include "SkXfermode_proccoeff.h"
#include "SkUtilsArm.h"

extern SkProcCoeffXfermode* SkPlatformXfermodeFactory_impl_neon(const ProcCoeff& rec,
                                                                SkXfermode::Mode mode);

extern SkXfermodeProc SkPlatformXfermodeProcFactory_impl_neon(SkXfermode::Mode mode);

SkProcCoeffXfermode* SkPlatformXfermodeFactory_impl(const ProcCoeff& rec,
                                                    SkXfermode::Mode mode) {
    return NULL;
}

SkXfermodeProc SkPlatformXfermodeProcFactory_impl(SkXfermode::Mode mode) {
    return NULL;
}

SkProcCoeffXfermode* SkPlatformXfermodeFactory(const ProcCoeff& rec,
                                               SkXfermode::Mode mode) {
    return SK_ARM_NEON_WRAP(SkPlatformXfermodeFactory_impl)(rec, mode);
}

SkXfermodeProc SkPlatformXfermodeProcFactory(SkXfermode::Mode mode) {
    return SK_ARM_NEON_WRAP(SkPlatformXfermodeProcFactory_impl)(mode);
}
