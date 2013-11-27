#ifndef SkBlitMask_opts_arm_neon_DEFINED
#define SkBlitMask_opts_arm_neon_DEFINED

#include "SkColor.h"
#include "SkBlitMask.h"

extern SkBlitMask::ColorProc D32_A8_Factory_neon(SkColor color);

extern void SkBlitLCD16OpaqueRow_neon(SkPMColor dst[], const uint16_t src[],
                                      SkColor color, int width,
                                      SkPMColor opaqueDst);

extern void SkBlitLCD16Row_neon(SkPMColor dst[], const uint16_t src[],
                                SkColor color, int width, SkPMColor);

#endif // #ifndef SkBlitMask_opts_arm_neon_DEFINED
