/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextUtils_DEFINED
#define GrTextUtils_DEFINED

#include "GrColor.h"
#include "SkColorFilter.h"
#include "SkPaint.h"
#include "SkScalar.h"
#include "SkGr.h"

class GrAtlasGlyphCache;
class GrAtlasTextBlob;
class GrAtlasTextStrike;
class GrClip;
class GrContext;
class GrRenderTargetContext;
class GrShaderCaps;
class SkGlyph;
class SkMatrix;
struct SkIRect;
struct SkPoint;
class SkGlyphCache;
class SkSurfaceProps;

/*
 * A class to house a bunch of common text utilities.  This class should *ONLY* have static
 * functions.  It is not a namespace only because we wish to friend SkPaint
 *
 */
class GrTextUtils {
public:
    class PaintWithFilteredColor {
    public:
        PaintWithFilteredColor(const SkPaint& paint) : fPaint(paint) {
            fFilteredSkColor = paint.getColor();
            if (paint.getColorFilter()) {
                fFilteredSkColor = paint.getColorFilter()->filterColor(fFilteredSkColor);
            }
            fFilteredGrColor = SkColorToPremulGrColor(fFilteredSkColor);
        }
        SkColor filteredSkColor() const { return fFilteredSkColor; }
        GrColor filteredPremulGrColor() const { return fFilteredGrColor; }

        const SkPaint& paint() const { return fPaint; }

    private:
        const SkPaint& fPaint;
        // This is the paint's color run through it's color filter, if present. This color should
        // be used except when rendering bitmap text, in which case each the bitmap must be filtered
        // in the fragment shader.
        SkColor fFilteredSkColor;
        SkColor fFilteredGrColor;
    };
    // Functions for appending BMP text to GrAtlasTextBlob
    static void DrawBmpText(GrAtlasTextBlob*, int runIndex,
                            GrAtlasGlyphCache*, const SkSurfaceProps&,
                            const SkPaint& runPaint, GrColor color, uint32_t scalerContextFlags,
                            const SkMatrix& viewMatrix, const char text[], size_t byteLength,
                            SkScalar x, SkScalar y);

    static void DrawBmpPosText(GrAtlasTextBlob*, int runIndex,
                               GrAtlasGlyphCache*, const SkSurfaceProps&,
                               const SkPaint& runPaint, GrColor color,
                               uint32_t scalerContextFlags, const SkMatrix& viewMatrix,
                               const char text[], size_t byteLength,
                               const SkScalar pos[], int scalarsPerPosition,
                               const SkPoint& offset);

    // functions for appending distance field text
    static bool CanDrawAsDistanceFields(const SkPaint& skPaint, const SkMatrix& viewMatrix,
                                        const SkSurfaceProps& props, const GrShaderCaps& caps);

    static void DrawDFText(GrAtlasTextBlob* blob, int runIndex,
                           GrAtlasGlyphCache*, const SkSurfaceProps&,
                           const SkPaint& runPaint, GrColor color, uint32_t scalerContextFlags,
                           const SkMatrix& viewMatrix,
                           const char text[], size_t byteLength,
                           SkScalar x, SkScalar y);

    static void DrawDFPosText(GrAtlasTextBlob* blob, int runIndex,
                              GrAtlasGlyphCache*, const SkSurfaceProps&,
                              const SkPaint& runPaint, GrColor color, uint32_t scalerContextFlags,
                              const SkMatrix& viewMatrix,
                              const char text[], size_t byteLength,
                              const SkScalar pos[], int scalarsPerPosition,
                              const SkPoint& offset);

    // Functions for drawing text as paths
    static void DrawTextAsPath(GrContext*, GrRenderTargetContext*, const GrClip& clip,
                               const SkPaint& runPaint, const SkMatrix& viewMatrix,
                               const char text[], size_t byteLength, SkScalar x, SkScalar y,
                               const SkIRect& clipBounds);

    static void DrawPosTextAsPath(GrContext* context,
                                  GrRenderTargetContext* rtc,
                                  const SkSurfaceProps& props,
                                  const GrClip& clip,
                                  const SkPaint&,
                                  const SkMatrix& viewMatrix,
                                  const char text[], size_t byteLength,
                                  const SkScalar pos[], int scalarsPerPosition,
                                  const SkPoint& offset, const SkIRect& clipBounds);

    static bool ShouldDisableLCD(const SkPaint& paint);

    static uint32_t FilterTextFlags(const SkSurfaceProps& surfaceProps, const SkPaint& paint);

private:
    static void InitDistanceFieldPaint(GrAtlasTextBlob* blob,
                                       SkPaint* skPaint,
                                       SkScalar* textRatio,
                                       const SkMatrix& viewMatrix);

    static void BmpAppendGlyph(GrAtlasTextBlob*, int runIndex, GrAtlasGlyphCache*,
                               GrAtlasTextStrike**, const SkGlyph&, int left, int top,
                               GrColor color, SkGlyphCache*);

    static bool DfAppendGlyph(GrAtlasTextBlob*, int runIndex, GrAtlasGlyphCache*,
                              GrAtlasTextStrike**, const SkGlyph&,
                              SkScalar sx, SkScalar sy, GrColor color,
                              SkGlyphCache* cache,
                              SkScalar textRatio, const SkMatrix& viewMatrix);
};

#endif
