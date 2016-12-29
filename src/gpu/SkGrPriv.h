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
#include "GrSamplerParams.h"
#include "SkImageInfo.h"
#include "SkMatrix.h"
#include "SkXfermodePriv.h"

class GrCaps;
class GrColorSpaceXform;
class GrContext;
class GrRenderTargetContext;
class GrFragmentProcessor;
class GrPaint;
class GrTexture;
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
                      GrRenderTargetContext*,
                      const SkPaint& skPaint,
                      const SkMatrix& viewM,
                      GrPaint* grPaint);

/** Same as above but ignores the SkShader (if any) on skPaint. */
bool SkPaintToGrPaintNoShader(GrContext* context,
                              GrRenderTargetContext* rtc,
                              const SkPaint& skPaint,
                              GrPaint* grPaint);

/** Replaces the SkShader (if any) on skPaint with the passed in GrFragmentProcessor. The processor
    should expect an unpremul input color and produce a premultiplied output color. There is
    no primitive color. */
bool SkPaintToGrPaintReplaceShader(GrContext*,
                                   GrRenderTargetContext*,
                                   const SkPaint& skPaint,
                                   sk_sp<GrFragmentProcessor> shaderFP,
                                   GrPaint* grPaint);

/** Blends the SkPaint's shader (or color if no shader) with the color which specified via a
    GrOp's GrPrimitiveProcesssor. Currently there is a bool param to indicate whether the
    primitive color is the dst or src color to the blend in order to work around differences between
    drawVertices and drawAtlas. */
bool SkPaintToGrPaintWithXfermode(GrContext* context,
                                  GrRenderTargetContext* rtc,
                                  const SkPaint& skPaint,
                                  const SkMatrix& viewM,
                                  SkBlendMode primColorMode,
                                  bool primitiveIsSrc,
                                  GrPaint* grPaint);

/** This is used when there is a primitive color, but the shader should be ignored. Currently,
    the expectation is that the primitive color will be premultiplied, though it really should be
    unpremultiplied so that interpolation is done in unpremul space. The paint's alpha will be
    applied to the primitive color after interpolation. */
inline bool SkPaintToGrPaintWithPrimitiveColor(GrContext* context, GrRenderTargetContext* rtc,
                                               const SkPaint& skPaint, GrPaint* grPaint) {
    return SkPaintToGrPaintWithXfermode(context, rtc, skPaint, SkMatrix::I(), SkBlendMode::kDst,
                                        false, grPaint);
}

/** This is used when there may or may not be a shader, and the caller wants to plugin a texture
    lookup.  If there is a shader, then its output will only be used if the texture is alpha8. */
bool SkPaintToGrPaintWithTexture(GrContext* context,
                                 GrRenderTargetContext* rtc,
                                 const SkPaint& paint,
                                 const SkMatrix& viewM,
                                 sk_sp<GrFragmentProcessor> fp,
                                 bool textureIsAlphaOnly,
                                 GrPaint* grPaint);

////////////////////////////////////////////////////////////////////////////////
// Sk to Gr Type conversions

static inline GrColor SkColorToPremulGrColor(SkColor c) {
    SkPMColor pm = SkPreMultiplyColor(c);
    unsigned r = SkGetPackedR32(pm);
    unsigned g = SkGetPackedG32(pm);
    unsigned b = SkGetPackedB32(pm);
    unsigned a = SkGetPackedA32(pm);
    return GrColorPackRGBA(r, g, b, a);
}

static inline GrColor SkColorToUnpremulGrColor(SkColor c) {
    unsigned r = SkColorGetR(c);
    unsigned g = SkColorGetG(c);
    unsigned b = SkColorGetB(c);
    unsigned a = SkColorGetA(c);
    return GrColorPackRGBA(r, g, b, a);
}

/** Transform an SkColor (sRGB bytes) to GrColor4f for the specified color space. */
GrColor4f SkColorToPremulGrColor4f(SkColor c, SkColorSpace* dstColorSpace);
GrColor4f SkColorToUnpremulGrColor4f(SkColor c, SkColorSpace* dstColorSpace);

/**
* As above, but with explicit control over the linearization and gamut xform steps.
* Typically used when you have easy access to a pre-computed xform.
*/
GrColor4f SkColorToPremulGrColor4f(SkColor c, bool gammaCorrect, GrColorSpaceXform* gamutXform);
GrColor4f SkColorToUnpremulGrColor4f(SkColor c, bool gammaCorrect, GrColorSpaceXform* gamutXform);

//////////////////////////////////////////////////////////////////////////////

GrPixelConfig SkImageInfo2GrPixelConfig(SkColorType, SkAlphaType, const SkColorSpace*,
                                        const GrCaps&);

static inline GrPixelConfig SkImageInfo2GrPixelConfig(const SkImageInfo& info, const GrCaps& caps) {
    return SkImageInfo2GrPixelConfig(info.colorType(), info.alphaType(), info.colorSpace(), caps);
}

GrSurfaceDesc GrImageInfoToSurfaceDesc(const SkImageInfo&, const GrCaps&);

bool GrPixelConfigToColorType(GrPixelConfig, SkColorType*);

/** When image filter code needs to construct a render target context to do intermediate rendering,
    we need a renderable pixel config. The source (SkSpecialImage) may not be in a renderable
    format, but we want to preserve the color space of that source. This picks an appropriate format
    to use. */
GrPixelConfig GrRenderableConfigForColorSpace(const SkColorSpace*);

GrSamplerParams::FilterMode GrSkFilterQualityToGrFilterMode(SkFilterQuality paintFilterQuality,
                                                            const SkMatrix& viewM,
                                                            const SkMatrix& localM,
                                                            bool* doBicubic);

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

GrTexture* GrGenerateMipMapsAndUploadToTexture(GrContext*, const SkBitmap&,
                                               SkColorSpace* dstColorSpace);

/**
 * Creates a new texture for the pixmap.
 */
GrTexture* GrUploadPixmapToTexture(GrContext*, const SkPixmap&, SkBudgeted budgeted);

/**
 * Creates a new texture populated with the mipmap levels.
 */
GrTexture* GrUploadMipMapToTexture(GrContext*, const SkImageInfo&, const GrMipLevel* texels,
                                   int mipLevelCount);

////////////////////////////////////////////////////////////////////////////////
/** Returns a texture representing the bitmap that is compatible with the GrSamplerParams. The
    texture is inserted into the cache (unless the bitmap is marked volatile) and can be
    retrieved again via this function. */
GrTexture* GrRefCachedBitmapTexture(GrContext*, const SkBitmap&, const GrSamplerParams&);

sk_sp<GrTexture> GrMakeCachedBitmapTexture(GrContext*, const SkBitmap&, const GrSamplerParams&);

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
