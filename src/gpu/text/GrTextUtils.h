/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextUtils_DEFINED
#define GrTextUtils_DEFINED

#include "GrColor.h"
#include "SkPaint.h"
#include "SkScalar.h"

class GrAtlasTextBlob;
class GrBatchFontCache;
class GrBatchTextStrike;
class GrClip;
class GrContext;
class GrDrawContext;
class GrFontScaler;
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
    // Functions for appending BMP text to GrAtlasTextBlob
    static void DrawBmpText(GrAtlasTextBlob*, int runIndex,
                            GrBatchFontCache*, const SkSurfaceProps&,
                            const SkPaint&,
                            GrColor, SkPaint::FakeGamma, const SkMatrix& viewMatrix,
                            const char text[], size_t byteLength,
                            SkScalar x, SkScalar y);

    static void DrawBmpPosText(GrAtlasTextBlob*, int runIndex,
                               GrBatchFontCache*, const SkSurfaceProps&, const SkPaint&,
                               GrColor, SkPaint::FakeGamma, const SkMatrix& viewMatrix,
                               const char text[], size_t byteLength,
                               const SkScalar pos[], int scalarsPerPosition,
                               const SkPoint& offset);

    // functions for appending distance field text
    static bool CanDrawAsDistanceFields(const SkPaint& skPaint, const SkMatrix& viewMatrix,
                                        const SkSurfaceProps& props, const GrShaderCaps& caps);

    static void DrawDFText(GrAtlasTextBlob* blob, int runIndex,
                           GrBatchFontCache*, const SkSurfaceProps&,
                           const SkPaint& skPaint, GrColor color, SkPaint::FakeGamma,
                           const SkMatrix& viewMatrix,
                           const char text[], size_t byteLength,
                           SkScalar x, SkScalar y);

    static void DrawDFPosText(GrAtlasTextBlob* blob, int runIndex,
                              GrBatchFontCache*, const SkSurfaceProps&, const SkPaint&,
                              GrColor color, SkPaint::FakeGamma, const SkMatrix& viewMatrix,
                              const char text[], size_t byteLength,
                              const SkScalar pos[], int scalarsPerPosition,
                              const SkPoint& offset);

    // Functions for drawing text as paths
    static void DrawTextAsPath(GrContext*, GrDrawContext*, const GrClip& clip,
                               const SkPaint& origPaint, const SkMatrix& viewMatrix,
                               const char text[], size_t byteLength, SkScalar x, SkScalar y,
                               const SkIRect& clipBounds);

    static void DrawPosTextAsPath(GrContext* context,
                                  GrDrawContext* dc,
                                  const SkSurfaceProps& props,
                                  const GrClip& clip,
                                  const SkPaint& origPaint, const SkMatrix& viewMatrix,
                                  const char text[], size_t byteLength,
                                  const SkScalar pos[], int scalarsPerPosition,
                                  const SkPoint& offset, const SkIRect& clipBounds);

    static bool ShouldDisableLCD(const SkPaint& paint);

    static GrFontScaler* GetGrFontScaler(SkGlyphCache* cache);
    static uint32_t FilterTextFlags(const SkSurfaceProps& surfaceProps, const SkPaint& paint);

private:
    static void InitDistanceFieldPaint(GrAtlasTextBlob* blob,
                                       SkPaint* skPaint,
                                       SkScalar* textRatio,
                                       const SkMatrix& viewMatrix);

    static void BmpAppendGlyph(GrAtlasTextBlob*, int runIndex, GrBatchFontCache*,
                               GrBatchTextStrike**, const SkGlyph&, int left, int top,
                               GrColor color, GrFontScaler*);

    static bool DfAppendGlyph(GrAtlasTextBlob*, int runIndex, GrBatchFontCache*,
                              GrBatchTextStrike**, const SkGlyph&,
                              SkScalar sx, SkScalar sy, GrColor color,
                              GrFontScaler* scaler,
                              SkScalar textRatio, const SkMatrix& viewMatrix);
};

#endif
