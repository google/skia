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

    void onDrawText(GrDrawContext*, GrRenderTarget*, const GrClip&, const GrPaint&, const SkPaint&,
                    const SkMatrix& viewMatrix, const char text[], size_t byteLength,
                    SkScalar x, SkScalar y, const SkIRect& regionClipBounds) override;
    void onDrawPosText(GrDrawContext*, GrRenderTarget*, const GrClip&, const GrPaint&,
                       const SkPaint&, const SkMatrix& viewMatrix,
                       const char text[], size_t byteLength,
                       const SkScalar pos[], int scalarsPerPosition,
                       const SkPoint& offset, const SkIRect& regionClipBounds) override;
    void drawTextBlob(GrDrawContext*, GrRenderTarget*, const GrClip&, const SkPaint&,
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
    inline void appendGlyphPath(GrAtlasTextBlob*, GrGlyph*, GrFontScaler*, const SkGlyph&,
                                SkScalar x, SkScalar y, SkScalar scale = 1.0f,
                                bool applyVM = false);
    inline void appendGlyphCommon(GrAtlasTextBlob*, Run*, Run::SubRunInfo*,
                                  const SkRect& positions, GrColor color,
                                  size_t vertexStride, bool useVertexColor,
                                  GrGlyph*);

    inline void flushRunAsPaths(GrDrawContext*,
                                const SkTextBlobRunIterator&, const GrClip& clip,
                                const SkPaint&, SkDrawFilter*,
                                const SkMatrix& viewMatrix, const SkIRect& clipBounds, SkScalar x,
                                SkScalar y);
    inline GrDrawBatch* createBatch(GrAtlasTextBlob*, const PerSubRunInfo&,
                                    int glyphCount, int run, int subRun,
                                    GrColor, SkScalar transX, SkScalar transY,
                                    const SkPaint&);
    inline void flushRun(GrDrawContext*, GrPipelineBuilder*, GrAtlasTextBlob*, int run, GrColor,
                         SkScalar transX, SkScalar transY, const SkPaint&);
    inline void flushBigGlyphs(GrAtlasTextBlob* cacheBlob, GrDrawContext*,
                               const GrClip& clip, const SkPaint& skPaint,
                               SkScalar transX, SkScalar transY, const SkIRect& clipBounds);

    // We have to flush SkTextBlobs differently from drawText / drawPosText
    void flush(const SkTextBlob*, GrAtlasTextBlob*, GrDrawContext*, GrRenderTarget*,
               const SkPaint&, const GrPaint&, SkDrawFilter*, const GrClip&,
               const SkMatrix& viewMatrix, const SkIRect& clipBounds, SkScalar x, SkScalar y,
               SkScalar transX, SkScalar transY);
    void flush(GrAtlasTextBlob*, GrDrawContext*, GrRenderTarget*, const SkPaint&,
               const GrPaint&, const GrClip&, const SkIRect& clipBounds);

    // A helper for drawing BitmapText in a run of distance fields
    inline void fallbackDrawPosText(GrAtlasTextBlob*, int runIndex,
                                    const GrClip&, GrColor color,
                                    const SkPaint&, const SkMatrix& viewMatrix,
                                    const SkTDArray<char>& fallbackTxt,
                                    const SkTDArray<SkScalar>& fallbackPos,
                                    int scalarsPerPosition,
                                    const SkPoint& offset);

    void internalDrawBMPText(GrAtlasTextBlob*, int runIndex, SkGlyphCache*, const SkPaint&,
                             GrColor color, const SkMatrix& viewMatrix,
                             const char text[], size_t byteLength,
                             SkScalar x, SkScalar y);
    void internalDrawBMPPosText(GrAtlasTextBlob*, int runIndex, SkGlyphCache*, const SkPaint&,
                                GrColor color, const SkMatrix& viewMatrix,
                                const char text[], size_t byteLength,
                                const SkScalar pos[], int scalarsPerPosition,
                                const SkPoint& offset);

    void internalDrawDFText(GrAtlasTextBlob*, int runIndex, const SkPaint&,
                            GrColor color, const SkMatrix& viewMatrix,
                            const char text[], size_t byteLength,
                            SkScalar x, SkScalar y,
                            SkScalar textRatio,
                            SkTDArray<char>* fallbackTxt,
                            SkTDArray<SkScalar>* fallbackPos,
                            SkPoint* offset, const SkPaint& origPaint);
    void internalDrawDFPosText(GrAtlasTextBlob*, int runIndex, const SkPaint&,
                               GrColor color, const SkMatrix& viewMatrix,
                               const char text[], size_t byteLength,
                               const SkScalar pos[], int scalarsPerPosition,
                               const SkPoint& offset,
                               SkScalar textRatio,
                               SkTDArray<char>* fallbackTxt,
                               SkTDArray<SkScalar>* fallbackPos);

    // sets up the descriptor on the blob and returns a detached cache.  Client must attach
    inline static GrColor ComputeCanonicalColor(const SkPaint&, bool lcd);
    inline SkGlyphCache* setupCache(Run*, const SkPaint&, const SkMatrix* viewMatrix, bool noGamma);
    static inline bool MustRegenerateBlob(SkScalar* outTransX, SkScalar* outTransY,
                                          const GrAtlasTextBlob&, const SkPaint&, GrColor,
                                          const SkMaskFilter::BlurRec&,
                                          const SkMatrix& viewMatrix, SkScalar x, SkScalar y);
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
    inline GrAtlasTextBlob* createDrawTextBlob(const GrClip&, const GrPaint&,
                                               const SkPaint&, const SkMatrix& viewMatrix,
                                               const char text[], size_t byteLength,
                                               SkScalar x, SkScalar y,
                                               const SkIRect& regionClipBounds);
    inline GrAtlasTextBlob* createDrawPosTextBlob(const GrClip&, const GrPaint&,
                                                  const SkPaint&, const SkMatrix& viewMatrix,
                                                  const char text[], size_t byteLength,
                                                  const SkScalar pos[], int scalarsPerPosition,
                                                  const SkPoint& offset,
                                                  const SkIRect& regionClipBounds);

    // Distance field text needs this table to compute a value for use in the fragment shader.
    // Because the GrAtlasTextContext can go out of scope before the final flush, this needs to be
    // refcnted and malloced
    struct DistanceAdjustTable : public SkNVRefCnt<DistanceAdjustTable> {
        DistanceAdjustTable() { this->buildDistanceAdjustTable(); }
        ~DistanceAdjustTable() { delete[] fTable; }

        const SkScalar& operator[] (int i) const {
            return fTable[i];
        }

    private:
        void buildDistanceAdjustTable();

        SkScalar* fTable;
    };

    GrBatchTextStrike* fCurrStrike;
    GrTextBlobCache* fCache;
    SkAutoTUnref<const DistanceAdjustTable> fDistanceAdjustTable;

    friend class GrTextBlobCache;
    friend class GrAtlasTextBatch;

#ifdef GR_TEST_UTILS
    DRAW_BATCH_TEST_FRIEND(TextBlobBatch);
#endif

    typedef GrTextContext INHERITED;
};

#endif
