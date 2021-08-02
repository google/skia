/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyphRunPainter_DEFINED
#define SkGlyphRunPainter_DEFINED

#include "include/core/SkSurfaceProps.h"
#include "src/core/SkDistanceFieldGen.h"
#include "src/core/SkGlyphBuffer.h"
#include "src/core/SkGlyphRun.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkTextBlobPriv.h"

#if SK_SUPPORT_GPU
#include "src/gpu/text/GrSDFTControl.h"
class GrColorInfo;
namespace skgpu { namespace v1 { class SurfaceDrawContext; }}
#endif

class SkGlyphRunPainterInterface;
class SkStrikeSpec;

// round and ignorePositionMask are used to calculate the subpixel position of a glyph.
// The per component (x or y) calculation is:
//
//   subpixelOffset = (floor((viewportPosition + rounding) & mask) >> 14) & 3
//
// where mask is either 0 or ~0, and rounding is either
// 1/2 for non-subpixel or 1/8 for subpixel.
struct SkGlyphPositionRoundingSpec {
    SkGlyphPositionRoundingSpec(bool isSubpixel, SkAxisAlignment axisAlignment);
    const SkVector halfAxisSampleFreq;
    const SkIPoint ignorePositionMask;
    const SkIPoint ignorePositionFieldMask;

private:
    static SkVector HalfAxisSampleFreq(bool isSubpixel, SkAxisAlignment axisAlignment);
    static SkIPoint IgnorePositionMask(bool isSubpixel, SkAxisAlignment axisAlignment);
    static SkIPoint IgnorePositionFieldMask(bool isSubpixel, SkAxisAlignment axisAlignment);
};

class SkStrikeCommon {
public:
    // An atlas consists of plots, and plots hold glyphs. The minimum a plot can be is 256x256.
    // This means that the maximum size a glyph can be is 256x256.
    static constexpr uint16_t kSkSideTooBigForAtlas = 256;
};

class SkGlyphRunListPainter {
public:
    // Constructor for SkBitmpapDevice.
    SkGlyphRunListPainter(const SkSurfaceProps& props,
                          SkColorType colorType,
                          SkColorSpace* cs,
                          SkStrikeForGPUCacheInterface* strikeCache);

#if SK_SUPPORT_GPU
    // The following two ctors are used exclusively by the GPU, and will always use the global
    // strike cache.
    SkGlyphRunListPainter(const SkSurfaceProps&, const GrColorInfo&);
    explicit SkGlyphRunListPainter(const skgpu::v1::SurfaceDrawContext&);
#endif  // SK_SUPPORT_GPU

    class BitmapDevicePainter {
    public:
        BitmapDevicePainter() = default;
        BitmapDevicePainter(const BitmapDevicePainter&) = default;
        virtual ~BitmapDevicePainter() = default;

        virtual void paintPaths(
                SkDrawableGlyphBuffer* drawables, SkScalar scale, SkPoint origin,
                const SkPaint& paint) const = 0;

        virtual void paintMasks(SkDrawableGlyphBuffer* drawables, const SkPaint& paint) const = 0;
        virtual void drawBitmap(const SkBitmap&, const SkMatrix&, const SkRect* dstOrNull,
                                const SkSamplingOptions&, const SkPaint&) const = 0;
    };

    void drawForBitmapDevice(
            const SkGlyphRunList& glyphRunList, const SkPaint& paint, const SkMatrix& deviceMatrix,
            const BitmapDevicePainter* bitmapDevice);

#if SK_SUPPORT_GPU
    // A nullptr for process means that the calls to the cache will be performed, but none of the
    // callbacks will be called.
    void processGlyphRun(const SkGlyphRun& glyphRun,
                         const SkMatrix& drawMatrix,
                         const SkPaint& drawPaint,
                         const GrSDFTControl& control,
                         SkGlyphRunPainterInterface* process,
                         const char* tag = nullptr);
#endif  // SK_SUPPORT_GPU

private:
    SkGlyphRunListPainter(const SkSurfaceProps& props, SkColorType colorType,
                          SkScalerContextFlags flags, SkStrikeForGPUCacheInterface* strikeCache);

    struct ScopedBuffers {
        ScopedBuffers(SkGlyphRunListPainter* painter, size_t size);
        ~ScopedBuffers();
        SkGlyphRunListPainter* fPainter;
    };

    ScopedBuffers SK_WARN_UNUSED_RESULT ensureBuffers(const SkGlyphRunList& glyphRunList);
    ScopedBuffers SK_WARN_UNUSED_RESULT ensureBuffers(const SkGlyphRun& glyphRun);

    // The props as on the actual device.
    const SkSurfaceProps fDeviceProps;
    // The props for when the bitmap device can't draw LCD text.
    const SkSurfaceProps fBitmapFallbackProps;
    const SkColorType fColorType;
    const SkScalerContextFlags fScalerContextFlags;

    SkStrikeForGPUCacheInterface* const fStrikeCache;

    SkDrawableGlyphBuffer fDrawable;
    SkSourceGlyphBuffer fRejects;
};

// SkGlyphRunPainterInterface are all the ways that Ganesh generates glyphs. The first
// distinction is between Device and Source.
// * Device - the data in the cache is scaled to the device. There is no transformation from the
//   cache to the screen.
// * Source - the data in the cache needs to be scaled from the cache to source space using the
//   factor cacheToSourceScale. When drawn the system must combine cacheToSourceScale and the
//   deviceView matrix to transform the cache data onto the screen. This allows zooming and
//   simple animation to reuse the same glyph data by just changing the transform.
//
// In addition to transformation type above, Masks, Paths, SDFT, and Fallback (or really the
// rendering method of last resort) are the different
// formats of data used from the cache.
class SkGlyphRunPainterInterface {
public:
    virtual ~SkGlyphRunPainterInterface() = default;

#if SK_GPU_V1
    virtual void processDeviceMasks(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    const SkStrikeSpec& strikeSpec) = 0;

    virtual void processSourceMasks(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    const SkStrikeSpec& strikeSpec) = 0;

    virtual void processSourcePaths(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                    const SkFont& runFont,
                                    const SkStrikeSpec& strikeSpec) = 0;

    virtual void processSourceSDFT(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                   const SkStrikeSpec& strikeSpec,
                                   const SkFont& runFont,
                                   SkScalar minScale,
                                   SkScalar maxScale) = 0;
#endif // SK_GPU_V1
};

#endif  // SkGlyphRunPainter_DEFINED
