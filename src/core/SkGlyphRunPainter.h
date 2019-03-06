/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyphRunPainter_DEFINED
#define SkGlyphRunPainter_DEFINED

#include "SkDistanceFieldGen.h"
#include "SkGlyphRun.h"
#include "SkScalerContext.h"
#include "SkSurfaceProps.h"
#include "SkTextBlobPriv.h"

#if SK_SUPPORT_GPU
#include "text/GrTextContext.h"
class GrColorSpaceInfo;
class GrRenderTargetContext;
#endif

class SkGlyphRunPainterInterface;

class SkStrikeCommon {
public:
    static SkVector PixelRounding(bool isSubpixel, SkAxisAlignment axisAlignment);

    // This assumes that position has the appropriate rounding term applied.
    static SkIPoint SubpixelLookup(SkAxisAlignment axisAlignment, SkPoint position);

    // An atlas consists of plots, and plots hold glyphs. The minimum a plot can be is 256x256.
    // This means that the maximum size a glyph can be is 256x256.
    static constexpr uint16_t kSkSideTooBigForAtlas = 256;

    static bool GlyphTooBigForAtlas(const SkGlyph& glyph);
};

class SkGlyphRunListPainter {
public:
    // Constructor for SkBitmpapDevice.
    SkGlyphRunListPainter(const SkSurfaceProps& props,
                          SkColorType colorType,
                          SkColorSpace* cs,
                          SkStrikeCacheInterface* strikeCache);

#if SK_SUPPORT_GPU
    // The following two ctors are used exclusively by the GPU, and will always use the global
    // strike cache.
    SkGlyphRunListPainter(const SkSurfaceProps&, const GrColorSpaceInfo&);
    explicit SkGlyphRunListPainter(const GrRenderTargetContext& renderTargetContext);
#endif  // SK_SUPPORT_GPU

    struct PathAndPos {
        const SkPath* path;
        SkPoint position;
    };

    struct GlyphAndPos {
        const SkGlyph* glyph;
        SkPoint position;
    };

    class BitmapDevicePainter {
    public:
        virtual ~BitmapDevicePainter() = default;

        virtual void paintPaths(SkSpan<const PathAndPos> pathsAndPositions,
                                SkScalar scale,
                                const SkPaint& paint) const = 0;

        virtual void paintMasks(SkSpan<const SkMask> masks, const SkPaint& paint) const = 0;
    };

    void drawForBitmapDevice(
            const SkGlyphRunList& glyphRunList, const SkMatrix& deviceMatrix,
            const BitmapDevicePainter* bitmapDevice);

#if SK_SUPPORT_GPU
    // A nullptr for process means that the calls to the cache will be performed, but none of the
    // callbacks will be called.
    void processGlyphRunList(const SkGlyphRunList& glyphRunList,
                             const SkMatrix& viewMatrix,
                             const SkSurfaceProps& props,
                             bool contextSupportsDistanceFieldText,
                             const GrTextContext::Options& options,
                             SkGlyphRunPainterInterface* process);
#endif  // SK_SUPPORT_GPU

    // TODO: Make this the canonical check for Skia.
    static bool ShouldDrawAsPath(const SkPaint& paint, const SkFont& font, const SkMatrix& matrix);

private:
    SkGlyphRunListPainter(const SkSurfaceProps& props, SkColorType colorType,
                          SkScalerContextFlags flags, SkStrikeCacheInterface* strikeCache);

    struct ScopedBuffers {
        ScopedBuffers(SkGlyphRunListPainter* painter, int size);
        ~ScopedBuffers();
        SkGlyphRunListPainter* fPainter;
    };

    ScopedBuffers SK_WARN_UNUSED_RESULT ensureBuffers(const SkGlyphRunList& glyphRunList);

    // TODO: Remove once I can hoist ensureBuffers above the list for loop in all cases.
    ScopedBuffers SK_WARN_UNUSED_RESULT ensureBuffers(const SkGlyphRun& glyphRun);

    void processARGBFallback(SkScalar maxGlyphDimension,
                             const SkPaint& runPaint,
                             const SkFont& runFont,
                             const SkMatrix& viewMatrix,
                             SkScalar cacheToSourceScale,
                             SkGlyphRunPainterInterface* process);

    // The props as on the actual device.
    const SkSurfaceProps fDeviceProps;
    // The props for when the bitmap device can't draw LCD text.
    const SkSurfaceProps fBitmapFallbackProps;
    const SkColorType fColorType;
    const SkScalerContextFlags fScalerContextFlags;

    SkStrikeCacheInterface* const fStrikeCache;

    int fMaxRunSize{0};
    SkAutoTMalloc<SkPoint> fPositions;
    SkAutoTMalloc<GlyphAndPos> fGlyphPos;

    std::vector<GlyphAndPos> fPaths;

    // Vectors for tracking ARGB fallback information.
    std::vector<SkGlyphID> fARGBGlyphsIDs;
    std::vector<SkPoint>   fARGBPositions;
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

    virtual void startRun(const SkGlyphRun& glyphRun, bool useSDFT) = 0;

    virtual void processDeviceMasks(SkSpan<const SkGlyphRunListPainter::GlyphAndPos> masks,
                                    SkStrikeInterface* strike) = 0;

    virtual void processSourcePaths(SkSpan<const SkGlyphRunListPainter::GlyphAndPos> paths,
                                    SkStrikeInterface* strike, SkScalar cacheToSourceScale) = 0;

    virtual void processDevicePaths(SkSpan<const SkGlyphRunListPainter::GlyphAndPos> paths) = 0;

    virtual void processSourceSDFT(SkSpan<const SkGlyphRunListPainter::GlyphAndPos> masks,
                                   SkStrikeInterface* strike,
                                   const SkFont& runFont,
                                   SkScalar cacheToSourceScale,
                                   SkScalar minScale,
                                   SkScalar maxScale,
                                   bool hasWCoord) = 0;

    virtual void processSourceFallback(SkSpan<const SkGlyphRunListPainter::GlyphAndPos> masks,
                                       SkStrikeInterface* strike,
                                       SkScalar cacheToSourceScale,
                                       bool hasW) = 0;

    virtual void processDeviceFallback(SkSpan<const SkGlyphRunListPainter::GlyphAndPos> masks,
                                       SkStrikeInterface* strike) = 0;

};

#endif  // SkGlyphRunPainter_DEFINED
