#include "SkBlitRow.h"

// Platform impl of Platform_procs with no overrides

const SkBlitRow::Proc SkBlitRow::gPlatform_565_Procs[] = {
    // no dither
    NULL,   // S32_D565_Opaque,
    NULL,   // S32_D565_Blend,
    NULL,   // S32A_D565_Opaque,
    NULL,   // S32A_D565_Blend,
    
    // dither
    NULL,   // S32_D565_Opaque_Dither,
    NULL,   // S32_D565_Blend_Dither,
    NULL,   // S32A_D565_Opaque_Dither,
    NULL,   // S32A_D565_Blend_Dither
};

const SkBlitRow::Proc SkBlitRow::gPlatform_4444_Procs[] = {
    // no dither
    NULL,   // S32_D4444_Opaque,
    NULL,   // S32_D4444_Blend,
    NULL,   // S32A_D4444_Opaque,
    NULL,   // S32A_D4444_Blend,
    
    // dither
    NULL,   // S32_D4444_Opaque_Dither,
    NULL,   // S32_D4444_Blend_Dither,
    NULL,   // S32A_D4444_Opaque_Dither,
    NULL,   // S32A_D4444_Blend_Dither
};

const SkBlitRow::Proc32 SkBlitRow::gPlatform_Procs32[] = {
    NULL,   // S32_Opaque,
    NULL,   // S32_Blend,
    NULL,   // S32A_Opaque,
    NULL,   // S32A_Blend,
};

