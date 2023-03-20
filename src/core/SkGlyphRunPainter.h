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
#include "src/base/SkZip.h"
#include "src/core/SkScalerContext.h"

class SkColorSpace;
class SkDrawableGlyphBuffer;
namespace sktext { class GlyphRunList; }

// -- SkGlyphRunListPainterCPU ---------------------------------------------------------------------
class SkGlyphRunListPainterCPU {
public:
    class BitmapDevicePainter {
    public:
        BitmapDevicePainter() = default;
        BitmapDevicePainter(const BitmapDevicePainter&) = default;
        virtual ~BitmapDevicePainter() = default;

        virtual void paintMasks(SkZip<const SkGlyph*, SkPoint> accepted,
                                const SkPaint& paint) const = 0;
        virtual void drawBitmap(const SkBitmap&, const SkMatrix&, const SkRect* dstOrNull,
                                const SkSamplingOptions&, const SkPaint&) const = 0;
    };

    SkGlyphRunListPainterCPU(const SkSurfaceProps& props,
                             SkColorType colorType,
                             SkColorSpace* cs);

    void drawForBitmapDevice(
            SkCanvas* canvas, const BitmapDevicePainter* bitmapDevice,
            const sktext::GlyphRunList& glyphRunList, const SkPaint& paint,
            const SkMatrix& drawMatrix);
private:
    // The props as on the actual device.
    const SkSurfaceProps fDeviceProps;

    // The props for when the bitmap device can't draw LCD text.
    const SkSurfaceProps fBitmapFallbackProps;
    const SkColorType fColorType;
    const SkScalerContextFlags fScalerContextFlags;
};
#endif  // SkGlyphRunPainter_DEFINED
