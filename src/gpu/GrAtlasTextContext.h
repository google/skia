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
#include "SkTextBlob.h"

#ifdef GR_TEST_UTILS
#include "GrBatchTest.h"
#endif

class GrDrawContext;
class GrDrawTarget;
class GrPipelineBuilder;
class GrTextBlobCache;
class SkGlyph;

/*
 * This class implements GrTextContext using standard bitmap fonts, and can also process textblobs.
 * TODO replace GrBitmapTextContext
 */
class GrAtlasTextContext : public GrTextContext {
public:
    static GrAtlasTextContext* Create(GrContext*, GrDrawContext*, const SkSurfaceProps&);

private:
    GrAtlasTextContext(GrContext*, GrDrawContext*, const SkSurfaceProps&);
    ~GrAtlasTextContext() override {}

    bool canDraw(const GrRenderTarget*, const GrClip&, const GrPaint&,
                 const SkPaint&, const SkMatrix& viewMatrix) override;

    void onDrawText(GrRenderTarget*, const GrClip&, const GrPaint&, const SkPaint&,
                    const SkMatrix& viewMatrix, const char text[], size_t byteLength,
                    SkScalar x, SkScalar y, const SkIRect& regionClipBounds) override;
    void onDrawPosText(GrRenderTarget*, const GrClip&, const GrPaint&,
                       const SkPaint&, const SkMatrix& viewMatrix,
                       const char text[], size_t byteLength,
                       const SkScalar pos[], int scalarsPerPosition,
                       const SkPoint& offset, const SkIRect& regionClipBounds) override;
    void drawTextBlob(GrRenderTarget*, const GrClip&, const SkPaint&,
                      const SkMatrix& viewMatrix, const SkTextBlob*, SkScalar x, SkScalar y,
                      SkDrawFilter*, const SkIRect& clipBounds) override;

    typedef GrAtlasTextBlob::Run Run;
    typedef Run::SubRunInfo PerSubRunInfo;

    inline bool canDrawAsDistanceFields(const SkPaint&, const SkMatrix& viewMatrix);
    GrAtlasTextBlob* setupDFBlob(int glyphCount, const SkPaint& origPaint,
                                const SkMatrix& viewMatrix, SkGlyphCache** cache,
                                SkPaint* dfPaint, SkScalar* textRatio);
    void bmpAppendGlyph(GrAtlasTextBlob*, int runIndex, const SkGlyph&, int left, int top,
                        GrColor color, GrFontScaler*, const SkIRect& clipRect);
    bool dfAppendGlyph(GrAtlasTextBlob*, int runIndex, const SkGlyph&, SkScalar sx, SkScalar sy,
                       GrColor color, GrFontScaler*, const SkIRect& clipRect, SkScalar textRatio,
                       const SkMatrix& viewMatrix);
    inline void appendGlyphPath(GrAtlasTextBlob*, GrGlyph*, GrFontScaler*, const SkGlyph&,
                                SkScalar x, SkScalar y);
    inline void appendGlyphCommon(GrAtlasTextBlob*, Run*, Run::SubRunInfo*,
                                  const SkRect& positions, GrColor color,
                                  size_t vertexStride, bool useVertexColor,
                                  GrGlyph*);

    inline void flushRunAsPaths(GrRenderTarget*,
                                const SkTextBlob::RunIterator&, const GrClip& clip,
                                const SkPaint&, SkDrawFilter*,
                                const SkMatrix& viewMatrix, const SkIRect& clipBounds, SkScalar x,
                                SkScalar y);
    inline GrBatch* createBatch(GrAtlasTextBlob*, const PerSubRunInfo&,
                                int glyphCount, int run, int subRun,
                                GrColor, SkScalar transX, SkScalar transY,
                                const SkPaint&);
    inline void flushRun(GrPipelineBuilder*, GrAtlasTextBlob*, int run, GrColor,
                         SkScalar transX, SkScalar transY, const SkPaint&);
    inline void flushBigGlyphs(GrAtlasTextBlob* cacheBlob, GrRenderTarget*,
                               const GrClip& clip, const SkPaint& skPaint,
                               SkScalar transX, SkScalar transY, const SkIRect& clipBounds);

    // We have to flush SkTextBlobs differently from drawText / drawPosText
    void flush(const SkTextBlob*, GrAtlasTextBlob*, GrRenderTarget*,
               const SkPaint&, const GrPaint&, SkDrawFilter*, const GrClip&,
               const SkMatrix& viewMatrix, const SkIRect& clipBounds, SkScalar x, SkScalar y,
               SkScalar transX, SkScalar transY);
    void flush(GrAtlasTextBlob*, GrRenderTarget*, const SkPaint&,
               const GrPaint&, const GrClip&, const SkIRect& clipBounds);

    // A helper for drawing BitmapText in a run of distance fields
    inline void fallbackDrawPosText(GrAtlasTextBlob*, int runIndex,
                                    GrRenderTarget*, const GrClip&,
                                    const GrPaint&,
                                    const SkPaint&, const SkMatrix& viewMatrix,
                                    const SkTDArray<char>& fallbackTxt,
                                    const SkTDArray<SkScalar>& fallbackPos,
                                    int scalarsPerPosition,
                                    const SkPoint& offset,
                                    const SkIRect& clipRect);

    void internalDrawBMPText(GrAtlasTextBlob*, int runIndex, SkGlyphCache*, const SkPaint&,
                             GrColor color, const SkMatrix& viewMatrix,
                             const char text[], size_t byteLength,
                             SkScalar x, SkScalar y, const SkIRect& clipRect);
    void internalDrawBMPPosText(GrAtlasTextBlob*, int runIndex, SkGlyphCache*, const SkPaint&,
                                GrColor color, const SkMatrix& viewMatrix,
                                const char text[], size_t byteLength,
                                const SkScalar pos[], int scalarsPerPosition,
                                const SkPoint& offset, const SkIRect& clipRect);

    void internalDrawDFText(GrAtlasTextBlob*, int runIndex, SkGlyphCache*, const SkPaint&,
                            GrColor color, const SkMatrix& viewMatrix,
                            const char text[], size_t byteLength,
                            SkScalar x, SkScalar y, const SkIRect& clipRect,
                            SkScalar textRatio,
                            SkTDArray<char>* fallbackTxt,
                            SkTDArray<SkScalar>* fallbackPos,
                            SkPoint* offset, const SkPaint& origPaint);
    void internalDrawDFPosText(GrAtlasTextBlob*, int runIndex, SkGlyphCache*, const SkPaint&,
                               GrColor color, const SkMatrix& viewMatrix,
                               const char text[], size_t byteLength,
                               const SkScalar pos[], int scalarsPerPosition,
                               const SkPoint& offset, const SkIRect& clipRect,
                               SkScalar textRatio,
                               SkTDArray<char>* fallbackTxt,
                               SkTDArray<SkScalar>* fallbackPos);

    // sets up the descriptor on the blob and returns a detached cache.  Client must attach
    inline static GrColor ComputeCanonicalColor(const SkPaint&, bool lcd);
    inline SkGlyphCache* setupCache(Run*, const SkPaint&, const SkMatrix* viewMatrix, bool noGamma);
    static inline bool MustRegenerateBlob(SkScalar* outTransX, SkScalar* outTransY,
                                          const GrAtlasTextBlob&, const SkPaint&,
                                          const SkMaskFilter::BlurRec&,
                                          const SkMatrix& viewMatrix, SkScalar x, SkScalar y);
    void regenerateTextBlob(GrAtlasTextBlob* bmp, const SkPaint& skPaint, GrColor,
                            const SkMatrix& viewMatrix,
                            const SkTextBlob* blob, SkScalar x, SkScalar y,
                            SkDrawFilter* drawFilter, const SkIRect& clipRect, GrRenderTarget*,
                            const GrClip&, const GrPaint&);
    inline static bool HasLCD(const SkTextBlob*);
    inline void initDistanceFieldPaint(GrAtlasTextBlob*, SkPaint*, SkScalar* textRatio,
                                       const SkMatrix&);

    // Test methods
    // TODO this is really ugly.  It'd be much nicer if positioning could be moved to batch
    inline GrAtlasTextBlob* createDrawTextBlob(GrRenderTarget*, const GrClip&, const GrPaint&,
                                              const SkPaint&, const SkMatrix& viewMatrix,
                                              const char text[], size_t byteLength,
                                              SkScalar x, SkScalar y,
                                              const SkIRect& regionClipBounds);
    inline GrAtlasTextBlob* createDrawPosTextBlob(GrRenderTarget*, const GrClip&, const GrPaint&,
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
        ~DistanceAdjustTable() { SkDELETE_ARRAY(fTable); }

        const SkScalar& operator[] (int i) const {
            return fTable[i];
        }

    private:
        void buildDistanceAdjustTable();

        SkScalar* fTable;
    };

    GrBatchTextStrike* fCurrStrike;
    GrTextBlobCache* fCache;
    SkAutoTUnref<DistanceAdjustTable> fDistanceAdjustTable;

    friend class GrTextBlobCache;
    friend class TextBatch;

#ifdef GR_TEST_UTILS
    BATCH_TEST_FRIEND(TextBlobBatch);
#endif

    typedef GrTextContext INHERITED;
};

#endif
