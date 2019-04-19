/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGr_DEFINED
#define SkGr_DEFINED

#include "GrBlend.h"
#include "GrCaps.h"
#include "GrColor.h"
#include "GrSamplerState.h"
#include "GrTypes.h"
#include "SkBlendModePriv.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkColorData.h"
#include "SkFilterQuality.h"
#include "SkImageInfo.h"
#include "SkMatrix.h"
#include "SkVertices.h"

class GrCaps;
class GrColorSpaceInfo;
class GrColorSpaceXform;
class GrContext;
class GrFragmentProcessor;
class GrPaint;
class GrRecordingContext;
class GrResourceProvider;
class GrTextureProxy;
class GrUniqueKey;
class SkBitmap;
class SkData;
class SkPaint;
class SkPixelRef;
class SkPixmap;
struct SkIRect;

////////////////////////////////////////////////////////////////////////////////
// Color type conversions

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

/** Similar, but using SkPMColor4f. */
SkPMColor4f SkColorToPMColor4f(SkColor, const GrColorSpaceInfo&);

/** Converts an SkColor4f to the destination color space. */
SkColor4f SkColor4fPrepForDst(SkColor4f, const GrColorSpaceInfo&);

/** Returns true if half-floats are required to store the color in a vertex (and half-floats
    are supported). */
static inline bool SkPMColor4fNeedsWideColor(SkPMColor4f color, GrClampType clampType,
                                             const GrCaps& caps) {
    return GrClampType::kNone == clampType &&
        caps.halfFloatVertexAttributeSupport() &&
        !color.fitsInBytes();
}

////////////////////////////////////////////////////////////////////////////////
// Paint conversion

/** Converts an SkPaint to a GrPaint for a given GrRecordingContext. The matrix is required in order
    to convert the SkShader (if any) on the SkPaint. The primitive itself has no color. */
bool SkPaintToGrPaint(GrRecordingContext*,
                      const GrColorSpaceInfo& dstColorSpaceInfo,
                      const SkPaint& skPaint,
                      const SkMatrix& viewM,
                      GrPaint* grPaint);

/** Same as above but ignores the SkShader (if any) on skPaint. */
bool SkPaintToGrPaintNoShader(GrRecordingContext*,
                              const GrColorSpaceInfo& dstColorSpaceInfo,
                              const SkPaint& skPaint,
                              GrPaint* grPaint);

/** Replaces the SkShader (if any) on skPaint with the passed in GrFragmentProcessor. The processor
    should expect an unpremul input color and produce a premultiplied output color. There is
    no primitive color. */
bool SkPaintToGrPaintReplaceShader(GrRecordingContext*,
                                   const GrColorSpaceInfo& dstColorSpaceInfo,
                                   const SkPaint& skPaint,
                                   std::unique_ptr<GrFragmentProcessor> shaderFP,
                                   GrPaint* grPaint);

/** Blends the SkPaint's shader (or color if no shader) with the color which specified via a
    GrOp's GrPrimitiveProcesssor. */
bool SkPaintToGrPaintWithXfermode(GrRecordingContext*,
                                  const GrColorSpaceInfo& dstColorSpaceInfo,
                                  const SkPaint& skPaint,
                                  const SkMatrix& viewM,
                                  SkBlendMode primColorMode,
                                  GrPaint* grPaint);

/** This is used when there is a primitive color, but the shader should be ignored. Currently,
    the expectation is that the primitive color will be premultiplied, though it really should be
    unpremultiplied so that interpolation is done in unpremul space. The paint's alpha will be
    applied to the primitive color after interpolation. */
inline bool SkPaintToGrPaintWithPrimitiveColor(GrRecordingContext* context,
                                               const GrColorSpaceInfo& dstColorSpaceInfo,
                                               const SkPaint& skPaint,
                                               GrPaint* grPaint) {
    return SkPaintToGrPaintWithXfermode(context, dstColorSpaceInfo, skPaint, SkMatrix::I(),
                                        SkBlendMode::kDst, grPaint);
}

/** This is used when there may or may not be a shader, and the caller wants to plugin a texture
    lookup.  If there is a shader, then its output will only be used if the texture is alpha8. */
bool SkPaintToGrPaintWithTexture(GrRecordingContext*,
                                 const GrColorSpaceInfo& dstColorSpaceInfo,
                                 const SkPaint& skPaint,
                                 const SkMatrix& viewM,
                                 std::unique_ptr<GrFragmentProcessor> fp,
                                 bool textureIsAlphaOnly,
                                 GrPaint* grPaint);

////////////////////////////////////////////////////////////////////////////////
// Misc Sk to Gr type conversions

GrSurfaceDesc GrImageInfoToSurfaceDesc(const SkImageInfo&);
GrPixelConfig SkColorType2GrPixelConfig(const SkColorType);
GrPixelConfig SkImageInfo2GrPixelConfig(const SkImageInfo& info);

bool GrPixelConfigToColorType(GrPixelConfig, SkColorType*);

GrSamplerState::Filter GrSkFilterQualityToGrFilterMode(SkFilterQuality paintFilterQuality,
                                                       const SkMatrix& viewM,
                                                       const SkMatrix& localM,
                                                       bool sharpenMipmappedTextures,
                                                       bool* doBicubic);

//////////////////////////////////////////////////////////////////////////////

static inline GrPrimitiveType SkVertexModeToGrPrimitiveType(SkVertices::VertexMode mode) {
    switch (mode) {
        case SkVertices::kTriangles_VertexMode:
            return GrPrimitiveType::kTriangles;
        case SkVertices::kTriangleStrip_VertexMode:
            return GrPrimitiveType::kTriangleStrip;
        case SkVertices::kTriangleFan_VertexMode:
            break;
    }
    SK_ABORT("Invalid mode");
    return GrPrimitiveType::kPoints;
}

//////////////////////////////////////////////////////////////////////////////

GR_STATIC_ASSERT((int)kZero_GrBlendCoeff == (int)SkBlendModeCoeff::kZero);
GR_STATIC_ASSERT((int)kOne_GrBlendCoeff == (int)SkBlendModeCoeff::kOne);
GR_STATIC_ASSERT((int)kSC_GrBlendCoeff == (int)SkBlendModeCoeff::kSC);
GR_STATIC_ASSERT((int)kISC_GrBlendCoeff == (int)SkBlendModeCoeff::kISC);
GR_STATIC_ASSERT((int)kDC_GrBlendCoeff == (int)SkBlendModeCoeff::kDC);
GR_STATIC_ASSERT((int)kIDC_GrBlendCoeff == (int)SkBlendModeCoeff::kIDC);
GR_STATIC_ASSERT((int)kSA_GrBlendCoeff == (int)SkBlendModeCoeff::kSA);
GR_STATIC_ASSERT((int)kISA_GrBlendCoeff == (int)SkBlendModeCoeff::kISA);
GR_STATIC_ASSERT((int)kDA_GrBlendCoeff == (int)SkBlendModeCoeff::kDA);
GR_STATIC_ASSERT((int)kIDA_GrBlendCoeff == (int)SkBlendModeCoeff::kIDA);
//GR_STATIC_ASSERT(SkXfermode::kCoeffCount == 10);

////////////////////////////////////////////////////////////////////////////////
// Texture management

/** Returns a texture representing the bitmap that is compatible with the GrSamplerState. The
 *  texture is inserted into the cache (unless the bitmap is marked volatile) and can be
 *  retrieved again via this function.
 *  The 'scaleAdjust' in/out parameter will be updated to hold any rescaling that needs to be
 *  performed on the absolute texture coordinates (e.g., if the texture is resized out to
 *  the next power of two). It can be null if the caller is sure the bitmap won't be resized.
 */
sk_sp<GrTextureProxy> GrRefCachedBitmapTextureProxy(GrRecordingContext*,
                                                    const SkBitmap&,
                                                    const GrSamplerState&,
                                                    SkScalar scaleAdjust[2]);

/**
 * Creates a new texture with mipmap levels and copies the baseProxy into the base layer.
 */
sk_sp<GrTextureProxy> GrCopyBaseMipMapToTextureProxy(GrRecordingContext*,
                                                     GrTextureProxy* baseProxy);

/*
 * Create a texture proxy from the provided bitmap by wrapping it in an image and calling
 * GrMakeCachedImageProxy.
 */
sk_sp<GrTextureProxy> GrMakeCachedBitmapProxy(GrProxyProvider*, const SkBitmap& bitmap,
                                              SkBackingFit fit = SkBackingFit::kExact);

/*
 * Create a texture proxy from the provided 'srcImage' and add it to the texture cache
 * using the key also extracted from 'srcImage'.
 */
sk_sp<GrTextureProxy> GrMakeCachedImageProxy(GrProxyProvider*, sk_sp<SkImage> srcImage,
                                             SkBackingFit fit = SkBackingFit::kExact);

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
void GrInstallBitmapUniqueKeyInvalidator(const GrUniqueKey& key, uint32_t contextID,
                                         SkPixelRef* pixelRef);

#endif
