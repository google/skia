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
#include "SkDrawFilter.h"
#include "SkGr.h"
#include "SkPaint.h"
#include "SkScalar.h"
#include "SkTLazy.h"
#include "SkTextBlobRunIterator.h"

class GrAtlasGlyphCache;
class GrAtlasTextBlob;
class GrAtlasTextStrike;
class GrClip;
class GrContext;
class GrPaint;
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
    class Paint {
    public:
        explicit Paint(const SkPaint* paint) : fPaint(paint) { this->initFilteredColor(); }

        SkColor filteredSkColor() const { return fFilteredSkColor; }
        GrColor filteredPremulGrColor() const { return fFilteredGrColor; }

        const SkPaint& skPaint() const { return *fPaint; }

        bool toGrPaint(GrMaskFormat, GrRenderTargetContext*, const SkMatrix& viewMatrix,
                       GrPaint*) const;

    protected:
        void initFilteredColor() {
            fFilteredSkColor = fPaint->getColor();
            if (fPaint->getColorFilter()) {
                fFilteredSkColor = fPaint->getColorFilter()->filterColor(fFilteredSkColor);
            }
            fFilteredGrColor = SkColorToPremulGrColor(fFilteredSkColor);
        }
        Paint() = default;
        const SkPaint* fPaint;
        // This is the paint's color run through it's color filter, if present. This color should
        // be used except when rendering bitmap text, in which case each the bitmap must be filtered
        // in the fragment shader.
        SkColor fFilteredSkColor;
        SkColor fFilteredGrColor;
    };

    class RunPaint : public Paint {
    public:
        RunPaint(const Paint* paint, SkDrawFilter* filter, const SkSurfaceProps& props)
                : fOriginalPaint(paint), fFilter(filter), fProps(props) {
            // Initially we represent the original paint.
            fPaint = &fOriginalPaint->skPaint();
            fFilteredSkColor = fOriginalPaint->filteredSkColor();
            fFilteredGrColor = fOriginalPaint->filteredPremulGrColor();
        }

        bool modifyForRun(const SkTextBlobRunIterator&);

    private:
        SkTLazy<SkPaint> fTempPaint;
        const Paint* fOriginalPaint;
        SkDrawFilter* fFilter;
        const SkSurfaceProps& fProps;
    };

    // Functions for appending BMP text to GrAtlasTextBlob
    static void DrawBmpText(GrAtlasTextBlob*, int runIndex, GrAtlasGlyphCache*,
                            const SkSurfaceProps&, const Paint& paint, uint32_t scalerContextFlags,
                            const SkMatrix& viewMatrix, const char text[], size_t byteLength,
                            SkScalar x, SkScalar y);

    static void DrawBmpPosText(GrAtlasTextBlob*, int runIndex, GrAtlasGlyphCache*,
                               const SkSurfaceProps&, const Paint& paint,
                               uint32_t scalerContextFlags, const SkMatrix& viewMatrix,
                               const char text[], size_t byteLength, const SkScalar pos[],
                               int scalarsPerPosition, const SkPoint& offset);

    // functions for appending distance field text
    static bool CanDrawAsDistanceFields(const SkPaint& paint, const SkMatrix& viewMatrix,
                                        const SkSurfaceProps& props, const GrShaderCaps& caps);

    static void DrawDFText(GrAtlasTextBlob* blob, int runIndex, GrAtlasGlyphCache*,
                           const SkSurfaceProps&, const Paint& paint, uint32_t scalerContextFlags,
                           const SkMatrix& viewMatrix, const char text[], size_t byteLength,
                           SkScalar x, SkScalar y);

    static void DrawDFPosText(GrAtlasTextBlob* blob, int runIndex, GrAtlasGlyphCache*,
                              const SkSurfaceProps&, const Paint& paint,
                              uint32_t scalerContextFlags, const SkMatrix& viewMatrix,
                              const char text[], size_t byteLength, const SkScalar pos[],
                              int scalarsPerPosition, const SkPoint& offset);

    // Functions for drawing text as paths
    static void DrawTextAsPath(GrContext*, GrRenderTargetContext*, const GrClip& clip,
                               const SkPaint& paint, const SkMatrix& viewMatrix, const char text[],
                               size_t byteLength, SkScalar x, SkScalar y,
                               const SkIRect& clipBounds);

    static void DrawPosTextAsPath(GrContext* context, GrRenderTargetContext* rtc,
                                  const SkSurfaceProps& props, const GrClip& clip,
                                  const SkPaint& paint, const SkMatrix& viewMatrix,
                                  const char text[], size_t byteLength, const SkScalar pos[],
                                  int scalarsPerPosition, const SkPoint& offset,
                                  const SkIRect& clipBounds);

    static bool ShouldDisableLCD(const SkPaint& paint);


private:
    static uint32_t FilterTextFlags(const SkSurfaceProps& surfaceProps, const SkPaint& paint);

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
