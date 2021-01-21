/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGr_DEFINED
#define SkGr_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSamplingOptions.h"
#include "include/gpu/GrTypes.h"
#include "include/private/SkColorData.h"
#include "src/core/SkBlendModePriv.h"
#include "src/gpu/GrBlend.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrSamplerState.h"

class GrCaps;
class GrColorInfo;
class GrColorSpaceXform;
class GrDirectContext;
class GrFragmentProcessor;
class GrPaint;
class GrRecordingContext;
class GrResourceProvider;
class GrTextureProxy;
class GrUniqueKey;
class SkBitmap;
class SkData;
class SkMatrix;
class SkMatrixProvider;
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
SkPMColor4f SkColorToPMColor4f(SkColor, const GrColorInfo&);

/** Converts an SkColor4f to the destination color space. */
SkColor4f SkColor4fPrepForDst(SkColor4f, const GrColorInfo&);

////////////////////////////////////////////////////////////////////////////////
// SkTileMode conversion

static constexpr GrSamplerState::WrapMode SkTileModeToWrapMode(SkTileMode tileMode) {
    switch (tileMode) {
        case SkTileMode::kClamp:  return GrSamplerState::WrapMode::kClamp;
        case SkTileMode::kDecal:  return GrSamplerState::WrapMode::kClampToBorder;
        case SkTileMode::kMirror: return GrSamplerState::WrapMode::kMirrorRepeat;
        case SkTileMode::kRepeat: return GrSamplerState::WrapMode::kRepeat;
    }
    SkUNREACHABLE;
}

////////////////////////////////////////////////////////////////////////////////
// Paint conversion

/** Converts an SkPaint to a GrPaint for a given GrRecordingContext. The matrix is required in order
    to convert the SkShader (if any) on the SkPaint. The primitive itself has no color. */
bool SkPaintToGrPaint(GrRecordingContext*,
                      const GrColorInfo& dstColorInfo,
                      const SkPaint& skPaint,
                      const SkMatrixProvider& matrixProvider,
                      GrPaint* grPaint);

/** Same as above but ignores the SkShader (if any) on skPaint. */
bool SkPaintToGrPaintNoShader(GrRecordingContext*,
                              const GrColorInfo& dstColorInfo,
                              const SkPaint& skPaint,
                              const SkMatrixProvider& matrixProvider,
                              GrPaint* grPaint);

/** Replaces the SkShader (if any) on skPaint with the passed in GrFragmentProcessor. The processor
    should expect an unpremul input color and produce a premultiplied output color. There is
    no primitive color. */
bool SkPaintToGrPaintReplaceShader(GrRecordingContext*,
                                   const GrColorInfo& dstColorInfo,
                                   const SkPaint& skPaint,
                                   const SkMatrixProvider& matrixProvider,
                                   std::unique_ptr<GrFragmentProcessor> shaderFP,
                                   GrPaint* grPaint);

/** Blends the SkPaint's shader (or color if no shader) with the color which specified via a
    GrOp's GrPrimitiveProcesssor. */
bool SkPaintToGrPaintWithBlend(GrRecordingContext*,
                               const GrColorInfo& dstColorInfo,
                               const SkPaint& skPaint,
                               const SkMatrixProvider& matrixProvider,
                               SkBlendMode primColorMode,
                               GrPaint* grPaint);

/** Blends the passed-in shader with a per-primitive color which must be setup as a vertex attribute
    using the specified SkBlendMode. */
bool SkPaintToGrPaintWithBlendReplaceShader(GrRecordingContext* context,
                                            const GrColorInfo& dstColorInfo,
                                            const SkPaint& skPaint,
                                            const SkMatrixProvider& matrixProvider,
                                            std::unique_ptr<GrFragmentProcessor> shaderFP,
                                            SkBlendMode primColorMode,
                                            GrPaint* grPaint);

/** This is used when there is a primitive color, but the shader should be ignored. Currently,
    the expectation is that the primitive color will be premultiplied, though it really should be
    unpremultiplied so that interpolation is done in unpremul space. The paint's alpha will be
    applied to the primitive color after interpolation. */
inline bool SkPaintToGrPaintWithPrimitiveColor(GrRecordingContext* context,
                                               const GrColorInfo& dstColorInfo,
                                               const SkPaint& skPaint,
                                               const SkMatrixProvider& matrixProvider,
                                               GrPaint* grPaint) {
    return SkPaintToGrPaintWithBlend(context, dstColorInfo, skPaint, matrixProvider,
                                     SkBlendMode::kDst, grPaint);
}

/** This is used when there may or may not be a shader, and the caller wants to plugin a texture
    lookup.  If there is a shader, then its output will only be used if the texture is alpha8. */
bool SkPaintToGrPaintWithTexture(GrRecordingContext*,
                                 const GrColorInfo& dstColorInfo,
                                 const SkPaint& skPaint,
                                 const SkMatrixProvider& matrixProvider,
                                 std::unique_ptr<GrFragmentProcessor> fp,
                                 bool textureIsAlphaOnly,
                                 GrPaint* grPaint);

////////////////////////////////////////////////////////////////////////////////
// Misc Sk to Gr type conversions

/**
 * Determines how to interpret SkFilterQuality given draw params and canvas state. If the returned
 * bool is true then bicubic filtering should be used (and the two other return values can be
 * ignored).
 */
std::tuple<GrSamplerState::Filter,
           GrSamplerState::MipmapMode,
           SkCubicResampler>
GrInterpretSamplingOptions(SkISize imageDims,
                           const SkSamplingOptions&,
                           const SkMatrix& viewM,
                           const SkMatrix& localM,
                           bool sharpenMipmappedTextures,
                           bool allowFilterQualityReduction);

//////////////////////////////////////////////////////////////////////////////

static_assert((int)kZero_GrBlendCoeff == (int)SkBlendModeCoeff::kZero);
static_assert((int)kOne_GrBlendCoeff == (int)SkBlendModeCoeff::kOne);
static_assert((int)kSC_GrBlendCoeff == (int)SkBlendModeCoeff::kSC);
static_assert((int)kISC_GrBlendCoeff == (int)SkBlendModeCoeff::kISC);
static_assert((int)kDC_GrBlendCoeff == (int)SkBlendModeCoeff::kDC);
static_assert((int)kIDC_GrBlendCoeff == (int)SkBlendModeCoeff::kIDC);
static_assert((int)kSA_GrBlendCoeff == (int)SkBlendModeCoeff::kSA);
static_assert((int)kISA_GrBlendCoeff == (int)SkBlendModeCoeff::kISA);
static_assert((int)kDA_GrBlendCoeff == (int)SkBlendModeCoeff::kDA);
static_assert((int)kIDA_GrBlendCoeff == (int)SkBlendModeCoeff::kIDA);
// static_assert(SkXfermode::kCoeffCount == 10);

////////////////////////////////////////////////////////////////////////////////
// Texture management

/**
 * Policies for how to create textures for SkImages (and SkBitmaps).
 */
enum class GrImageTexGenPolicy : int {
    // Choose the cheapest way to generate the texture. Use GrResourceCache if appropriate.
    kDraw,
    // Always make a new texture that is uncached and unbudgeted.
    kNew_Uncached_Unbudgeted,
    // Always make a new texture that is uncached and budgeted.
    kNew_Uncached_Budgeted
};

/**
 * Returns a view that wraps a texture representing the bitmap. The texture is inserted into the
 * cache (unless the bitmap is marked volatile and can be retrieved again via this function.
 * A MIP mapped texture may be returned even when GrMipmapped is kNo. The function will succeed
 * with a non-MIP mapped texture if GrMipmapped is kYes but MIP mapping is not supported.
 */
GrSurfaceProxyView GrRefCachedBitmapView(GrRecordingContext*, const SkBitmap&, GrMipmapped);

/**
 * Creates a new texture with mipmap levels and copies the baseProxy into the base layer.
 */
sk_sp<GrSurfaceProxy> GrCopyBaseMipMapToTextureProxy(GrRecordingContext*,
                                                     sk_sp<GrSurfaceProxy> baseProxy,
                                                     GrSurfaceOrigin origin,
                                                     SkBudgeted = SkBudgeted::kYes);
/**
 * Same as GrCopyBaseMipMapToTextureProxy but takes the src as a view and returns a view with same
 * origin and swizzle as the src view.
 */
GrSurfaceProxyView GrCopyBaseMipMapToView(GrRecordingContext*,
                                          GrSurfaceProxyView,
                                          SkBudgeted = SkBudgeted::kYes);

/*
 * Create a texture proxy from the provided bitmap and add it to the texture cache
 * using the key also extracted from 'bitmp'.
 */
GrSurfaceProxyView GrMakeCachedBitmapProxyView(GrRecordingContext*, const SkBitmap& bitmap);

/**
 *  Our key includes the offset, width, and height so that bitmaps created by extractSubset()
 *  are unique.
 *
 *  The imageID is in the shared namespace (see SkNextID::ImageID())
 *      - SkBitmap/SkPixelRef
 *      - SkImage
 *      - SkImageGenerator
 */
void GrMakeKeyFromImageID(GrUniqueKey* key, uint32_t imageID, const SkIRect& imageBounds);

/**
 * Makes a SkIDChangeListener from a GrUniqueKey. The key will be invalidated in the resource
 * cache if the ID becomes invalid. This also modifies the key so that it will cause the listener
 * to be deregistered if the key is destroyed (to prevent unbounded listener growth when resources
 * are purged before listeners trigger).
 */
sk_sp<SkIDChangeListener> GrMakeUniqueKeyInvalidationListener(GrUniqueKey*, uint32_t contextID);

constexpr SkCubicResampler kInvalidCubicResampler{-1.f, -1.f};

static inline bool GrValidCubicResampler(SkCubicResampler cubic) {
    return cubic.B >= 0 && cubic.C >= 0;
}

#endif
