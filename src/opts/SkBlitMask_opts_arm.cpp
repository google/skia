/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkBlitMask.h"
#include "SkUtilsArm.h"
#include "SkBlitMask_opts_arm_neon.h"

SkBlitMask::ColorProc SkBlitMask::PlatformColorProcs(SkColorType dstCT,
                                                     SkMask::Format maskFormat,
                                                     SkColor color) {
#if SK_ARM_NEON_IS_NONE
    return NULL;
#else
/* ** This has been disabled until we can diagnose and fix the SIGILL generated
   ** in the NEON code.  See http://skbug.com/2067 for details.
#if SK_ARM_NEON_IS_DYNAMIC
    if (!sk_cpu_arm_has_neon()) {
        return NULL;
    }
#endif
    if ((kN32_SkColorType == dstCT) &&
        (SkMask::kA8_Format == maskFormat)) {
            return D32_A8_Factory_neon(color);
    }
*/
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

SkBlitMask::RowProc SkBlitMask::PlatformRowProcs(SkColorType dstCT,
                                                 SkMask::Format maskFormat,
                                                 RowFlags flags) {
    return NULL;
}
