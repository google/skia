
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkBlitMask.h"
#include "SkUtilsArm.h"
#include "SkBlitMask_opts_arm_neon.h"

SkBlitMask::ColorProc SkBlitMask::PlatformColorProcs(SkBitmap::Config dstConfig,
                                                     SkMask::Format maskFormat,
                                                     SkColor color) {
#if SK_ARM_NEON_IS_NONE
    return NULL;
#else
#if SK_ARM_NEON_IS_DYNAMIC
    if (!sk_cpu_arm_has_neon()) {
        return NULL;
    }
#endif
    if ((SkBitmap::kARGB_8888_Config == dstConfig) &&
        (SkMask::kA8_Format == maskFormat)) {
            return D32_A8_Factory_neon(color);
    }
#endif

    // We don't need to handle the SkMask::kLCD16_Format case as the default
    // LCD16 will call us through SkBlitMask::PlatformBlitRowProcs16()

    return NULL;
}

SkBlitMask::BlitLCD16RowProc SkBlitMask::PlatformBlitRowProcs16(bool isOpaque) {
    if (isOpaque) {
        return SK_ARM_NEON_WRAP(SkBlitLCD16OpaqueRow);
    } else {
        return SK_ARM_NEON_WRAP(SkBlitLCD16Row);
    }
}

SkBlitMask::RowProc SkBlitMask::PlatformRowProcs(SkBitmap::Config dstConfig,
                                                 SkMask::Format maskFormat,
                                                 RowFlags flags) {
    return NULL;
}
