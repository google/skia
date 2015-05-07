
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef SkGr_DEFINED
#define SkGr_DEFINED

#include <stddef.h>

// Gr headers
#include "GrTypes.h"
#include "GrContext.h"

// skia headers
#include "SkBitmap.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkRegion.h"
#include "SkClipStack.h"

////////////////////////////////////////////////////////////////////////////////
// Sk to Gr Type conversions

GR_STATIC_ASSERT((int)kZero_GrBlendCoeff == (int)SkXfermode::kZero_Coeff);
GR_STATIC_ASSERT((int)kOne_GrBlendCoeff  == (int)SkXfermode::kOne_Coeff);
GR_STATIC_ASSERT((int)kSC_GrBlendCoeff   == (int)SkXfermode::kSC_Coeff);
GR_STATIC_ASSERT((int)kISC_GrBlendCoeff  == (int)SkXfermode::kISC_Coeff);
GR_STATIC_ASSERT((int)kDC_GrBlendCoeff   == (int)SkXfermode::kDC_Coeff);
GR_STATIC_ASSERT((int)kIDC_GrBlendCoeff  == (int)SkXfermode::kIDC_Coeff);
GR_STATIC_ASSERT((int)kSA_GrBlendCoeff   == (int)SkXfermode::kSA_Coeff);
GR_STATIC_ASSERT((int)kISA_GrBlendCoeff  == (int)SkXfermode::kISA_Coeff);
GR_STATIC_ASSERT((int)kDA_GrBlendCoeff   == (int)SkXfermode::kDA_Coeff);
GR_STATIC_ASSERT((int)kIDA_GrBlendCoeff  == (int)SkXfermode::kIDA_Coeff);

#define sk_blend_to_grblend(X) ((GrBlendCoeff)(X))

///////////////////////////////////////////////////////////////////////////////

#include "SkColorPriv.h"

GrPixelConfig SkImageInfo2GrPixelConfig(SkColorType, SkAlphaType, SkColorProfileType);

static inline GrPixelConfig SkImageInfo2GrPixelConfig(const SkImageInfo& info) {
    return SkImageInfo2GrPixelConfig(info.colorType(), info.alphaType(), info.profileType());
}

bool GrPixelConfig2ColorAndProfileType(GrPixelConfig, SkColorType*, SkColorProfileType*);

static inline GrColor SkColor2GrColor(SkColor c) {
    SkPMColor pm = SkPreMultiplyColor(c);
    unsigned r = SkGetPackedR32(pm);
    unsigned g = SkGetPackedG32(pm);
    unsigned b = SkGetPackedB32(pm);
    unsigned a = SkGetPackedA32(pm);
    return GrColorPackRGBA(r, g, b, a);
}

static inline GrColor SkColor2GrColorJustAlpha(SkColor c) {
    U8CPU a = SkColorGetA(c);
    return GrColorPackRGBA(a, a, a, a);
}

////////////////////////////////////////////////////////////////////////////////

bool GrIsBitmapInCache(const GrContext*, const SkBitmap&, const GrTextureParams*);

GrTexture* GrRefCachedBitmapTexture(GrContext*, const SkBitmap&, const GrTextureParams*);

////////////////////////////////////////////////////////////////////////////////

// Converts a SkPaint to a GrPaint, ignoring the SkPaint's shader.
// Sets the color of GrPaint to the value of the parameter paintColor
// Callers may subsequently modify the GrPaint. Setting constantColor indicates
// that the final paint will draw the same color at every pixel. This allows
// an optimization where the color filter can be applied to the SkPaint's
// color once while converting to GrPaint and then ignored. TODO: Remove this
// bool and use the invariant info to automatically apply the color filter.
bool SkPaint2GrPaintNoShader(GrContext* context, GrRenderTarget*, const SkPaint& skPaint,
                             GrColor paintColor, bool constantColor, GrPaint* grPaint);

// This function is similar to skPaint2GrPaintNoShader but also converts
// skPaint's shader to a GrFragmentProcessor if possible.
// constantColor has the same meaning as in skPaint2GrPaintNoShader.
bool SkPaint2GrPaint(GrContext* context, GrRenderTarget*, const SkPaint& skPaint,
                     const SkMatrix& viewM, bool constantColor, GrPaint* grPaint);


SkImageInfo GrMakeInfoFromTexture(GrTexture* tex, int w, int h, bool isOpaque);

// Using the dreaded SkGrPixelRef ...
void GrWrapTextureInBitmap(GrTexture* src, int w, int h, bool isOpaque, SkBitmap* dst);

////////////////////////////////////////////////////////////////////////////////
// Classes

class SkGlyphCache;

////////////////////////////////////////////////////////////////////////////////

#endif
