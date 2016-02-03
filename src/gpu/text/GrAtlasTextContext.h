/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasTextContext_DEFINED
#define GrAtlasTextContext_DEFINED

#include "GrTextContext.h"

#include "GrAtlasTextBlob.h"
#include "GrDistanceFieldAdjustTable.h"
#include "GrGeometryProcessor.h"
#include "SkTextBlobRunIterator.h"

#ifdef GR_TEST_UTILS
#include "GrBatchTest.h"
#endif

class GrDrawBatch;
class GrDrawContext;
class GrDrawTarget;
class GrPipelineBuilder;
class GrTextBlobCache;
class SkGlyph;

/*
 * This class implements GrTextContext using standard bitmap fonts, and can also process textblobs.
 */
class GrAtlasTextContext : public GrTextContext {
public:
    static GrAtlasTextContext* Create(GrContext*, const SkSurfaceProps&);

private:
    GrAtlasTextContext(GrContext*, const SkSurfaceProps&);
    ~GrAtlasTextContext() override {}

    bool canDraw(const SkPaint&, const SkMatrix& viewMatrix) override;

    void onDrawText(GrDrawContext*, const GrClip&, const GrPaint&, const SkPaint&,
                    const SkMatrix& viewMatrix, const char text[], size_t byteLength,
                    SkScalar x, SkScalar y, const SkIRect& regionClipBounds) override;
    void onDrawPosText(GrDrawContext*, const GrClip&, const GrPaint&,
                       const SkPaint&, const SkMatrix& viewMatrix,
                       const char text[], size_t byteLength,
                       const SkScalar pos[], int scalarsPerPosition,
                       const SkPoint& offset, const SkIRect& regionClipBounds) override;
    void drawTextBlob(GrDrawContext*, const GrClip&, const SkPaint&,
                      const SkMatrix& viewMatrix, const SkTextBlob*, SkScalar x, SkScalar y,
                      SkDrawFilter*, const SkIRect& clipBounds) override;

    typedef GrAtlasTextBlob::Run Run;
    typedef Run::SubRunInfo PerSubRunInfo;

    inline bool canDrawAsDistanceFields(const SkPaint&, const SkMatrix& viewMatrix);
    GrAtlasTextBlob* setupDFBlob(int glyphCount, const SkPaint& origPaint,
                                 const SkMatrix& viewMatrix, SkPaint* dfPaint, 
                                 SkScalar* textRatio);
    void bmpAppendGlyph(GrAtlasTextBlob*, int runIndex, const SkGlyph&, int left, int top,
                        GrColor color, GrFontScaler*);
    bool dfAppendGlyph(GrAtlasTextBlob*, int runIndex, const SkGlyph&, SkScalar sx, SkScalar sy,
                       GrColor color, GrFontScaler*, SkScalar textRatio,
                       const SkMatrix& viewMatrix);

    // A helper for drawing BitmapText in a run of distance fields
    inline void fallbackDrawPosText(GrAtlasTextBlob*, int runIndex,
                                    GrColor color,
                                    const SkPaint&, const SkMatrix& viewMatrix,
                                    const SkTDArray<char>& fallbackTxt,
                                    const SkTDArray<SkScalar>& fallbackPos,
                                    int scalarsPerPosition,
                                    const SkPoint& offset);

    void internalDrawDFText(GrAtlasTextBlob*, int runIndex,
                            const SkPaint&,
                            GrColor color, const SkMatrix& viewMatrix,
                            const char text[], size_t byteLength,
                            SkScalar x, SkScalar y,
                            SkScalar textRatio, const SkPaint& origPaint);
    void internalDrawDFPosText(GrAtlasTextBlob*, int runIndex,
                               const SkPaint&,
                               GrColor color, const SkMatrix& viewMatrix,
                               const char text[], size_t byteLength,
                               const SkScalar pos[], int scalarsPerPosition,
                               const SkPoint& offset,
                               SkScalar textRatio,
                               const SkPaint& origPaint);

    // sets up the descriptor on the blob and returns a detached cache.  Client must attach
    inline static GrColor ComputeCanonicalColor(const SkPaint&, bool lcd);
    void regenerateTextBlob(GrAtlasTextBlob* bmp, const SkPaint& skPaint, GrColor,
                            const SkMatrix& viewMatrix,
                            const SkTextBlob* blob, SkScalar x, SkScalar y,
                            SkDrawFilter* drawFilter,
                            const GrClip&);
    inline static bool HasLCD(const SkTextBlob*);
    inline void initDistanceFieldPaint(GrAtlasTextBlob*, SkPaint*, SkScalar* textRatio,
                                       const SkMatrix&);

    // Test methods
    // TODO this is really ugly.  It'd be much nicer if positioning could be moved to batch
    inline GrAtlasTextBlob* createDrawTextBlob(const GrPaint&,
                                               const SkPaint&, const SkMatrix& viewMatrix,
                                               const char text[], size_t byteLength,
                                               SkScalar x, SkScalar y);
    inline GrAtlasTextBlob* createDrawPosTextBlob(const GrPaint&,
                                                  const SkPaint&, const SkMatrix& viewMatrix,
                                                  const char text[], size_t byteLength,
                                                  const SkScalar pos[], int scalarsPerPosition,
                                                  const SkPoint& offset);
    const GrDistanceFieldAdjustTable* dfAdjustTable() const { return fDistanceAdjustTable; }

    GrBatchTextStrike* fCurrStrike;
    GrTextBlobCache* fCache;
    SkAutoTUnref<const GrDistanceFieldAdjustTable> fDistanceAdjustTable;

#ifdef GR_TEST_UTILS
    DRAW_BATCH_TEST_FRIEND(TextBlobBatch);
#endif

    typedef GrTextContext INHERITED;
};

#endif
