/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyphRunPainter_DEFINED
#define SkGlyphRunPainter_DEFINED

#include "include/core/SkColorType.h"
#include "include/core/SkSurfaceProps.h"
#include "src/core/SkDevice.h"
#include "src/core/SkScalerContext.h"

class SkGlyphRunList;

class SkStrikeCommon {
public:
    // An atlas consists of plots, and plots hold glyphs. The minimum a plot can be is 256x256.
    // This means that the maximum size a glyph can be is 256x256.
    inline static constexpr uint16_t kSkSideTooBigForAtlas = 256;
};

// -- SkGlyphRunListPainterCPU ---------------------------------------------------------------------
class SkGlyphRunListPainterCPU {
public:
    class BitmapDevicePainter {
    public:
        BitmapDevicePainter() = default;
        BitmapDevicePainter(const BitmapDevicePainter&) = default;
        virtual ~BitmapDevicePainter() = default;

        virtual void paintMasks(SkDrawableGlyphBuffer* accepted, const SkPaint& paint) const = 0;
        virtual void drawBitmap(const SkBitmap&, const SkMatrix&, const SkRect* dstOrNull,
                                const SkSamplingOptions&, const SkPaint&) const = 0;
    };

    SkGlyphRunListPainterCPU(const SkSurfaceProps& props,
                             SkColorType colorType,
                             SkColorSpace* cs);

    void drawForBitmapDevice(
            SkCanvas* canvas, const BitmapDevicePainter* bitmapDevice,
            const SkGlyphRunList& glyphRunList, const SkPaint& paint, const SkMatrix& drawMatrix);
private:
    // The props as on the actual device.
    const SkSurfaceProps fDeviceProps;

    // The props for when the bitmap device can't draw LCD text.
    const SkSurfaceProps fBitmapFallbackProps;
    const SkColorType fColorType;
    const SkScalerContextFlags fScalerContextFlags;
};

#if (SK_SUPPORT_GPU || defined(SK_GRAPHITE_ENABLED))
namespace sktext::gpu{
class SubRunList;
class SubRunAllocator;
}

class SkGlyphRunListPainter {
public:
    // A nullptr for subRunList means that no SubRuns will be created, but the code will go
    // through all the decisions and strike lookups.
    // N.B. The positionMatrix has already been translated to the glyph run list origin.
    static bool CategorizeGlyphRunList(const SkGlyphRunList& glyphRunList,
                                       const SkMatrix& positionMatrix,
                                       const SkPaint& drawPaint,
                                       SkStrikeDeviceInfo strikeDeviceInfo,
                                       SkStrikeForGPUCacheInterface* strikeCache,
                                       sktext::gpu::SubRunList* subRunList,
                                       sktext::gpu::SubRunAllocator* alloc,
                                       const char* tag = nullptr);
};
#endif  // SK_SUPPORT_GPU || defined(SK_GRAPHITE_ENABLED)
#endif  // SkGlyphRunPainter_DEFINED
