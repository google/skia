/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGr_DEFINED
#define SkGr_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkColor.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "src/core/SkColorData.h"
#include "src/core/SkColorPriv.h"
#include "src/gpu/Blend.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrColor.h"
#include "src/gpu/ganesh/GrSamplerState.h"

#include <cstdint>
#include <memory>
#include <string_view>
#include <tuple>

class GrColorInfo;
class GrFragmentProcessor;
class GrPaint;
class GrRecordingContext;
class GrSurfaceProxy;
class GrSurfaceProxyView;
class SkBitmap;
class SkBlender;
class SkIDChangeListener;
class SkMatrix;
class SkPaint;
enum class GrColorType;
enum GrSurfaceOrigin : int;
struct SkIRect;

namespace skgpu { class UniqueKey; }
namespace skgpu::ganesh { class SurfaceDrawContext; }

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
bool SkPaintToGrPaint(skgpu::ganesh::SurfaceDrawContext*,
                      const SkPaint& skPaint,
                      const SkMatrix& ctm,
                      GrPaint* grPaint);

/** Replaces the SkShader (if any) on skPaint with the passed in GrFragmentProcessor, if not null.
    If null then it is assumed that the geometry processor is implementing a shader replacement.
    The processor should expect an unpremul input color and produce a premultiplied output color. */
bool SkPaintToGrPaintReplaceShader(skgpu::ganesh::SurfaceDrawContext*,
                                   const SkPaint& skPaint,
                                   const SkMatrix& ctm,
                                   std::unique_ptr<GrFragmentProcessor> shaderFP,
                                   GrPaint* grPaint);

/** Blends the SkPaint's shader (or color if no shader) with the color which specified via a
    GrOp's GrPrimitiveProcesssor. */
bool SkPaintToGrPaintWithBlend(skgpu::ganesh::SurfaceDrawContext* context,
                               const SkPaint& skPaint,
                               const SkMatrix& ctm,
                               SkBlender* primColorBlender,
                               GrPaint* grPaint);

////////////////////////////////////////////////////////////////////////////////
// Misc Sk to Gr type conversions

static_assert((int)skgpu::BlendCoeff::kZero == (int)SkBlendModeCoeff::kZero);
static_assert((int)skgpu::BlendCoeff::kOne == (int)SkBlendModeCoeff::kOne);
static_assert((int)skgpu::BlendCoeff::kSC == (int)SkBlendModeCoeff::kSC);
static_assert((int)skgpu::BlendCoeff::kISC == (int)SkBlendModeCoeff::kISC);
static_assert((int)skgpu::BlendCoeff::kDC == (int)SkBlendModeCoeff::kDC);
static_assert((int)skgpu::BlendCoeff::kIDC == (int)SkBlendModeCoeff::kIDC);
static_assert((int)skgpu::BlendCoeff::kSA == (int)SkBlendModeCoeff::kSA);
static_assert((int)skgpu::BlendCoeff::kISA == (int)SkBlendModeCoeff::kISA);
static_assert((int)skgpu::BlendCoeff::kDA == (int)SkBlendModeCoeff::kDA);
static_assert((int)skgpu::BlendCoeff::kIDA == (int)SkBlendModeCoeff::kIDA);
static_assert((int)SkBlendModeCoeff::kCoeffCount == 10);

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
 * Creates a new texture with mipmap levels and copies the baseProxy into the base layer.
 */
sk_sp<GrSurfaceProxy> GrCopyBaseMipMapToTextureProxy(GrRecordingContext*,
                                                     sk_sp<GrSurfaceProxy> baseProxy,
                                                     GrSurfaceOrigin origin,
                                                     std::string_view label,
                                                     skgpu::Budgeted = skgpu::Budgeted::kYes);
/**
 * Same as GrCopyBaseMipMapToTextureProxy but takes the src as a view and returns a view with same
 * origin and swizzle as the src view.
 */
GrSurfaceProxyView GrCopyBaseMipMapToView(GrRecordingContext*,
                                          GrSurfaceProxyView,
                                          skgpu::Budgeted = skgpu::Budgeted::kYes);

/*
 * Create a texture proxy from the provided bitmap and add it to the texture cache using the key
 * also extracted from the bitmap. If skgpu::Mipmapped is kYes a non-mipmapped result may be
 * returned if mipmapping isn't supported or for a 1x1 bitmap. If skgpu::Mipmapped is kNo it
 * indicates mipmaps aren't required but a previously created mipmapped texture may still be
 * returned. A color type is returned as color type conversion may be performed if there isn't a
 * texture format equivalent of the bitmap's color type.
 */
std::tuple<GrSurfaceProxyView, GrColorType> GrMakeCachedBitmapProxyView(
        GrRecordingContext*,
        const SkBitmap&,
        std::string_view label,
        skgpu::Mipmapped = skgpu::Mipmapped::kNo);

/**
 * Like above but always uploads the bitmap and never inserts into the cache. Unlike above, the
 * texture may be approx or scratch and budgeted or not.
 */
std::tuple<GrSurfaceProxyView, GrColorType> GrMakeUncachedBitmapProxyView(
        GrRecordingContext*,
        const SkBitmap&,
        skgpu::Mipmapped = skgpu::Mipmapped::kNo,
        SkBackingFit = SkBackingFit::kExact,
        skgpu::Budgeted = skgpu::Budgeted::kYes);

/**
 *  Our key includes the offset, width, and height so that bitmaps created by extractSubset()
 *  are unique.
 *
 *  The imageID is in the shared namespace (see SkNextID::ImageID())
 *      - SkBitmap/SkPixelRef
 *      - SkImage
 *      - SkImageGenerator
 */
void GrMakeKeyFromImageID(skgpu::UniqueKey* key, uint32_t imageID, const SkIRect& imageBounds);

/**
 * Makes a SkIDChangeListener from a skgpu::UniqueKey. The key will be invalidated in the resource
 * cache if the ID becomes invalid. This also modifies the key so that it will cause the listener
 * to be deregistered if the key is destroyed (to prevent unbounded listener growth when resources
 * are purged before listeners trigger).
 */
sk_sp<SkIDChangeListener> GrMakeUniqueKeyInvalidationListener(skgpu::UniqueKey*,
                                                              uint32_t contextID);

static inline bool GrValidCubicResampler(SkCubicResampler cubic) {
    return cubic.B >= 0 && cubic.C >= 0;
}

#endif
