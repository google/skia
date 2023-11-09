/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkScalarContext_win_dw_DEFINED
#define SkScalarContext_win_dw_DEFINED

#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTDArray.h"
#include "src/core/SkScalerContext.h"
#include "src/ports/SkTypeface_win_dw.h"

#include <dwrite.h>
#include <dwrite_2.h>
#include <dwrite_3.h>

class SkGlyph;
class SkDescriptor;

interface IDWritePaintReader;
struct DWRITE_PAINT_ELEMENT;

class SkScalerContext_DW : public SkScalerContext {
public:
    SkScalerContext_DW(sk_sp<DWriteFontTypeface>,
                       const SkScalerContextEffects&,
                       const SkDescriptor*);
    ~SkScalerContext_DW() override;

protected:
    GlyphMetrics generateMetrics(const SkGlyph&, SkArenaAlloc*) override;
    void generateImage(const SkGlyph&, void* imageBuffer) override;
    bool generatePath(const SkGlyph&, SkPath*) override;
    sk_sp<SkDrawable> generateDrawable(const SkGlyph&) override;
    void generateFontMetrics(SkFontMetrics*) override;

private:
    bool setAdvance(const SkGlyph&, SkVector*);

    struct ScalerContextBits {
        using value_type = uint16_t;
        static const constexpr value_type NONE   = 0;
        static const constexpr value_type DW     = 1;
        static const constexpr value_type DW_1   = 2;
        static const constexpr value_type PNG    = 3;
        static const constexpr value_type SVG    = 4;
        static const constexpr value_type COLR   = 5;
        static const constexpr value_type COLRv1 = 6;
        static const constexpr value_type PATH   = 7;
    };

    static void BilevelToBW(const uint8_t* SK_RESTRICT src, const SkGlyph& glyph, void* dst);

    template<bool APPLY_PREBLEND>
    static void GrayscaleToA8(const uint8_t* SK_RESTRICT src,
                              const SkGlyph& glyph, void* dst,
                              const uint8_t* table8);

    template<bool APPLY_PREBLEND>
    static void RGBToA8(const uint8_t* SK_RESTRICT src,
                        const SkGlyph& glyph, void* dst,
                        const uint8_t* table8);

    template<bool APPLY_PREBLEND, bool RGB>
    static void RGBToLcd16(const uint8_t* SK_RESTRICT src, const SkGlyph& glyph, void* dst,
                           const uint8_t* tableR, const uint8_t* tableG, const uint8_t* tableB);

    DWriteFontTypeface* getDWriteTypeface() {
        return static_cast<DWriteFontTypeface*>(this->getTypeface());
    }

    bool generateColorV1PaintBounds(SkMatrix*, SkRect*, IDWritePaintReader&, DWRITE_PAINT_ELEMENT const &);
    bool generateColorV1Metrics(const SkGlyph&, SkRect*);
    bool generateColorV1Image(const SkGlyph&, void* dst);
    bool drawColorV1Paint(SkCanvas&, IDWritePaintReader&, DWRITE_PAINT_ELEMENT const &);
    bool drawColorV1Image(const SkGlyph&, SkCanvas&);

    bool getColorGlyphRun(const SkGlyph&, IDWriteColorGlyphRunEnumerator**);
    bool generateColorMetrics(const SkGlyph&, SkRect*);
    bool generateColorImage(const SkGlyph&, void* dst);
    bool drawColorImage(const SkGlyph&, SkCanvas&);

    bool generateSVGMetrics(const SkGlyph&, SkRect*);
    bool generateSVGImage(const SkGlyph&, void* dst);
    bool drawSVGImage(const SkGlyph&, SkCanvas&);

    bool generatePngMetrics(const SkGlyph&, SkRect*);
    bool generatePngImage(const SkGlyph&, void* dst);
    bool drawPngImage(const SkGlyph&, SkCanvas&);

    bool generateDWMetrics(const SkGlyph&, DWRITE_RENDERING_MODE, DWRITE_TEXTURE_TYPE, SkRect*);
    const void* getDWMaskBits(const SkGlyph&, DWRITE_RENDERING_MODE, DWRITE_TEXTURE_TYPE);
    bool generateDWImage(const SkGlyph&, void* dst);

    SkTDArray<uint8_t> fBits;
    /** The total matrix without the text height scale. */
    SkMatrix fSkXform;
    /** The total matrix without the text height scale. */
    DWRITE_MATRIX fXform;
    /** The text size to render with. */
    SkScalar fTextSizeRender;
    /** The text size to measure with. */
    SkScalar fTextSizeMeasure;
    int fGlyphCount;
    DWRITE_RENDERING_MODE fRenderingMode;
    DWRITE_TEXTURE_TYPE fTextureType;
    DWRITE_MEASURING_MODE fMeasuringMode;
    DWRITE_TEXT_ANTIALIAS_MODE fAntiAliasMode;
    DWRITE_GRID_FIT_MODE fGridFitMode;
};

#endif
