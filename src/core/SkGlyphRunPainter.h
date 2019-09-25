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
#include "src/core/SkGlyphRun.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkTextBlobPriv.h"

#if SK_SUPPORT_GPU
#include "src/gpu/text/GrTextContext.h"
class GrColorSpaceInfo;
class GrRenderTargetContext;
#endif

class SkGlyphRunPainterInterface;
class SkStrikeSpec;

class SkStrikeCommon {
public:
    static SkVector PixelRounding(bool isSubpixel, SkAxisAlignment axisAlignment);

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
    SkGlyphRunListPainter(const SkSurfaceProps&, const GrColorSpaceInfo&);
    explicit SkGlyphRunListPainter(const GrRenderTargetContext& renderTargetContext);
#endif  // SK_SUPPORT_GPU

    class BitmapDevicePainter {
    public:
        virtual ~BitmapDevicePainter() = default;

        virtual void paintPaths(SkSpan<const SkPathPos> pathsAndPositions,
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

private:
    SkGlyphRunListPainter(const SkSurfaceProps& props, SkColorType colorType,
                          SkScalerContextFlags flags, SkStrikeForGPUCacheInterface* strikeCache);

    struct ScopedBuffers {
        ScopedBuffers(SkGlyphRunListPainter* painter, int size);
        ~ScopedBuffers();
        SkGlyphRunListPainter* fPainter;
    };

    ScopedBuffers SK_WARN_UNUSED_RESULT ensureBuffers(const SkGlyphRunList& glyphRunList);

    // TODO: Remove once I can hoist ensureBuffers above the list for loop in all cases.
    ScopedBuffers SK_WARN_UNUSED_RESULT ensureBuffers(const SkGlyphRun& glyphRun);

    /**
     *  @param fARGBPositions in source space
     *  @param fARGBGlyphsIDs the glyphs to process
     *  @param fGlyphPos used as scratch space
     *  @param maxSourceGlyphDimension the longest dimension of any glyph as if all fARGBGlyphsIDs
     *                                 were drawn in source space (as if viewMatrix were identity)
     */
    void processARGBFallback(SkScalar maxSourceGlyphDimension,
                             const SkPaint& runPaint,
                             const SkFont& runFont,
                             const SkMatrix& viewMatrix,
                             SkGlyphRunPainterInterface* process);

    static SkSpan<const SkPackedGlyphID> DeviceSpacePackedGlyphIDs(
            SkStrikeForGPU* strike,
            const SkMatrix& viewMatrix,
            const SkPoint& origin,
            int n,
            const SkGlyphID* glyphIDs,
            const SkPoint* positions,
            SkPoint* mappedPositions,
            SkPackedGlyphID* results);

    static SkSpan<const SkPackedGlyphID> SourceSpacePackedGlyphIDs(
            const SkPoint& origin,
            int n,
            const SkGlyphID* glyphIDs,
            const SkPoint* positions,
            SkPoint* mappedPositions,
            SkPackedGlyphID* results);

    // The props as on the actual device.
    const SkSurfaceProps fDeviceProps;
    // The props for when the bitmap device can't draw LCD text.
    const SkSurfaceProps fBitmapFallbackProps;
    const SkColorType fColorType;
    const SkScalerContextFlags fScalerContextFlags;

    SkStrikeForGPUCacheInterface* const fStrikeCache;

    int fMaxRunSize{0};
    SkAutoTMalloc<SkPoint> fPositions;
    SkAutoTMalloc<SkPackedGlyphID> fPackedGlyphIDs;
    SkAutoTMalloc<SkGlyphPos> fGlyphPos;

    std::vector<SkGlyphPos> fPaths;

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

    virtual void processDeviceMasks(SkSpan<const SkGlyphPos> masks,
                                    const SkStrikeSpec& strikeSpec) = 0;

    virtual void processSourcePaths(SkSpan<const SkGlyphPos> paths,
                                    const SkStrikeSpec& strikeSpec) = 0;

    virtual void processDevicePaths(SkSpan<const SkGlyphPos> paths) = 0;

    virtual void processSourceSDFT(SkSpan<const SkGlyphPos> masks,
                                   const SkStrikeSpec& strikeSpec,
                                   const SkFont& runFont,
                                   SkScalar minScale,
                                   SkScalar maxScale,
                                   bool hasWCoord) = 0;

    virtual void processSourceFallback(SkSpan<const SkGlyphPos> masks,
                                       const SkStrikeSpec& strikeSpec,
                                       bool hasW) = 0;

    virtual void processDeviceFallback(SkSpan<const SkGlyphPos> masks,
                                       const SkStrikeSpec& strikeSpec) = 0;

};

#endif  // SkGlyphRunPainter_DEFINED
