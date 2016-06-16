/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkScalarContext_win_dw_DEFINED
#define SkScalarContext_win_dw_DEFINED

#include "SkScalar.h"
#include "SkScalerContext.h"
#include "SkTypeface_win_dw.h"
#include "SkTypes.h"

#include <dwrite.h>
#if SK_HAS_DWRITE_2_H
#include <dwrite_2.h>
#endif

class SkGlyph;
class SkDescriptor;

class SkScalerContext_DW : public SkScalerContext {
public:
    SkScalerContext_DW(DWriteFontTypeface*, const SkScalerContextEffects&, const SkDescriptor*);
    virtual ~SkScalerContext_DW();

protected:
    unsigned generateGlyphCount() override;
    uint16_t generateCharToGlyph(SkUnichar uni) override;
    void generateAdvance(SkGlyph* glyph) override;
    void generateMetrics(SkGlyph* glyph) override;
    void generateImage(const SkGlyph& glyph) override;
    void generatePath(const SkGlyph& glyph, SkPath* path) override;
    void generateFontMetrics(SkPaint::FontMetrics*) override;

private:
    const void* drawDWMask(const SkGlyph& glyph,
                           DWRITE_RENDERING_MODE renderingMode,
                           DWRITE_TEXTURE_TYPE textureType);

    HRESULT getBoundingBox(SkGlyph* glyph,
                           DWRITE_RENDERING_MODE renderingMode,
                           DWRITE_TEXTURE_TYPE textureType,
                           RECT* bbox);

    bool isColorGlyph(const SkGlyph& glyph);

#if SK_HAS_DWRITE_2_H
    bool getColorGlyphRun(const SkGlyph& glyph, IDWriteColorGlyphRunEnumerator** colorGlyph);

    void generateColorGlyphImage(const SkGlyph& glyph);
#endif

    SkTDArray<uint8_t> fBits;
    /** The total matrix without the text height scale. */
    SkMatrix fSkXform;
    /** The total matrix without the text height scale. */
    DWRITE_MATRIX fXform;
    /** The non-rotational part of total matrix without the text height scale.
     *  This is used to find the magnitude of gdi compatible advances.
     */
    DWRITE_MATRIX fGsA;
    /** The inverse of the rotational part of the total matrix.
     *  This is used to find the direction of gdi compatible advances.
     */
    SkMatrix fG_inv;
    /** The text size to render with. */
    SkScalar fTextSizeRender;
    /** The text size to measure with. */
    SkScalar fTextSizeMeasure;
    SkAutoTUnref<DWriteFontTypeface> fTypeface;
    int fGlyphCount;
    DWRITE_RENDERING_MODE fRenderingMode;
    DWRITE_TEXTURE_TYPE fTextureType;
    DWRITE_MEASURING_MODE fMeasuringMode;
#if SK_HAS_DWRITE_2_H
    SkTScopedComPtr<IDWriteFactory2> fFactory2;
    bool fIsColorFont;
#endif
};

#endif
