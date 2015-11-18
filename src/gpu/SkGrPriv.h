/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGrPriv_DEFINED
#define SkGrPriv_DEFINED

#include "GrTypes.h"
#include "GrBlend.h"
#include "SkImageInfo.h"
#include "SkMatrix.h"
#include "SkXfermode.h"

class GrCaps;
class GrContext;
class GrFragmentProcessor;
class GrPaint;
class GrTexture;
class GrTextureParams;
class GrUniqueKey;
class SkData;
class SkPaint;
class SkPixelRef;
struct SkIRect;

/**
 *  Our key includes the offset, width, and height so that bitmaps created by extractSubset()
 *  are unique.
 *
 *  The imageID is in the shared namespace (see SkNextID::ImageID())
 *      - SkBitmap/SkPixelRef
 *      - SkImage
 *      - SkImageGenerator
 *
 *  Note: width/height must fit in 16bits for this impl.
 */
void GrMakeKeyFromImageID(GrUniqueKey* key, uint32_t imageID, const SkIRect& imageBounds);

/** Call this after installing a GrUniqueKey on texture. It will cause the texture's key to be
    removed should the bitmap's contents change or be destroyed. */
void GrInstallBitmapUniqueKeyInvalidator(const GrUniqueKey& key, SkPixelRef* pixelRef);

/** Converts an SkPaint to a GrPaint for a given GrContext. The matrix is required in order
    to convert the SkShader (if any) on the SkPaint. The primitive itself has no color. */
bool SkPaintToGrPaint(GrContext*,
                      const SkPaint& skPaint,
                      const SkMatrix& viewM,
                      GrPaint* grPaint);

/** Same as above but ignores the SkShader (if any) on skPaint. */
bool SkPaintToGrPaintNoShader(GrContext* context,
                              const SkPaint& skPaint,
                              GrPaint* grPaint);

/** Replaces the SkShader (if any) on skPaint with the passed in GrFragmentProcessor. The processor
    should expect an unpremul input color and produce a premultiplied output color. There is
    no primitive color. */
bool SkPaintToGrPaintReplaceShader(GrContext*,
                                   const SkPaint& skPaint,
                                   const GrFragmentProcessor* shaderFP,
                                   GrPaint* grPaint);

/** Blends the SkPaint's shader (or color if no shader) with the color which specified via a
    GrBatch's GrPrimitiveProcesssor. Currently there is a bool param to indicate whether the
    primitive color is the dst or src color to the blend in order to work around differences between
    drawVertices and drawAtlas. */
bool SkPaintToGrPaintWithXfermode(GrContext* context,
                                  const SkPaint& skPaint,
                                  const SkMatrix& viewM,
                                  SkXfermode::Mode primColorMode,
                                  bool primitiveIsSrc,
                                  GrPaint* grPaint);

/** This is used when there is a primitive color, but the shader should be ignored. Currently,
    the expectation is that the primitive color will be premultiplied, though it really should be
    unpremultiplied so that interpolation is done in unpremul space. The paint's alpha will be
    applied to the primitive color after interpolation. */
inline bool SkPaintToGrPaintWithPrimitiveColor(GrContext* context, const SkPaint& skPaint,
                                               GrPaint* grPaint) {
    return SkPaintToGrPaintWithXfermode(context, skPaint, SkMatrix::I(), SkXfermode::kDst_Mode,
                                        false, grPaint);
}

/** This is used when there may or may not be a shader, and the caller wants to plugin a texture
    lookup.  If there is a shader, then its output will only be used if the texture is alpha8. */
bool SkPaintToGrPaintWithTexture(GrContext* context,
                                 const SkPaint& paint,
                                 const SkMatrix& viewM,
                                 const GrFragmentProcessor* fp,
                                 bool textureIsAlphaOnly,
                                 GrPaint* grPaint);

//////////////////////////////////////////////////////////////////////////////

GrSurfaceDesc GrImageInfoToSurfaceDesc(const SkImageInfo&);

bool GrPixelConfig2ColorAndProfileType(GrPixelConfig, SkColorType*, SkColorProfileType*);

/**
 *  If the compressed data in the SkData is supported (as a texture format, this returns
 *  the pixel-config that should be used, and sets outStartOfDataToUpload to the ptr into
 *  the data where the actual raw data starts (skipping any header bytes).
 *
 *  If the compressed data is not supported, this returns kUnknown_GrPixelConfig, and
 *  ignores outStartOfDataToUpload.
 */
GrPixelConfig GrIsCompressedTextureDataSupported(GrContext* ctx, SkData* data,
                                                 int expectedW, int expectedH,
                                                 const void** outStartOfDataToUpload);


/**
 * Creates a new texture for the bitmap. Does not concern itself with cache keys or texture params.
 * The bitmap must have CPU-accessible pixels. Attempts to take advantage of faster paths for
 * compressed textures and yuv planes.
 */
GrTexture* GrUploadBitmapToTexture(GrContext*, const SkBitmap&);

//////////////////////////////////////////////////////////////////////////////

GR_STATIC_ASSERT((int)kZero_GrBlendCoeff == (int)SkXfermode::kZero_Coeff);
GR_STATIC_ASSERT((int)kOne_GrBlendCoeff == (int)SkXfermode::kOne_Coeff);
GR_STATIC_ASSERT((int)kSC_GrBlendCoeff == (int)SkXfermode::kSC_Coeff);
GR_STATIC_ASSERT((int)kISC_GrBlendCoeff == (int)SkXfermode::kISC_Coeff);
GR_STATIC_ASSERT((int)kDC_GrBlendCoeff == (int)SkXfermode::kDC_Coeff);
GR_STATIC_ASSERT((int)kIDC_GrBlendCoeff == (int)SkXfermode::kIDC_Coeff);
GR_STATIC_ASSERT((int)kSA_GrBlendCoeff == (int)SkXfermode::kSA_Coeff);
GR_STATIC_ASSERT((int)kISA_GrBlendCoeff == (int)SkXfermode::kISA_Coeff);
GR_STATIC_ASSERT((int)kDA_GrBlendCoeff == (int)SkXfermode::kDA_Coeff);
GR_STATIC_ASSERT((int)kIDA_GrBlendCoeff == (int)SkXfermode::kIDA_Coeff);
GR_STATIC_ASSERT(SkXfermode::kCoeffCount == 10);

#define SkXfermodeCoeffToGrBlendCoeff(X) ((GrBlendCoeff)(X))

#endif
