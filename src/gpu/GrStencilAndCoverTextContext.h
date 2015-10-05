/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStencilAndCoverTextContext_DEFINED
#define GrStencilAndCoverTextContext_DEFINED

#include "GrTextContext.h"
#include "GrDrawTarget.h"
#include "GrStrokeInfo.h"
#include "SkTHash.h"
#include "SkTInternalLList.h"
#include "SkTLList.h"

class GrTextStrike;
class GrPath;
class GrPathRange;
class SkSurfaceProps;

/*
 * This class implements text rendering using stencil and cover path rendering
 * (by the means of GrDrawTarget::drawPath).
 * This class exposes the functionality through GrTextContext interface.
 */
class GrStencilAndCoverTextContext : public GrTextContext {
public:
    static GrStencilAndCoverTextContext* Create(GrContext*, const SkSurfaceProps&);

    virtual ~GrStencilAndCoverTextContext();

private:
    GrStencilAndCoverTextContext(GrContext*, const SkSurfaceProps&);

    bool canDraw(const GrRenderTarget*, const GrClip&, const GrPaint&, const SkPaint& skPaint,
                 const SkMatrix&) override { return this->internalCanDraw(skPaint); }

    bool internalCanDraw(const SkPaint&);

    void onDrawText(GrDrawContext*, GrRenderTarget*, const GrClip&, const GrPaint&, const SkPaint&,
                    const SkMatrix& viewMatrix,
                    const char text[], size_t byteLength,
                    SkScalar x, SkScalar y, const SkIRect& clipBounds) override;
    void onDrawPosText(GrDrawContext*, GrRenderTarget*,
                       const GrClip&, const GrPaint&, const SkPaint&,
                       const SkMatrix& viewMatrix,
                       const char text[], size_t byteLength,
                       const SkScalar pos[], int scalarsPerPosition,
                       const SkPoint& offset, const SkIRect& clipBounds) override;
    void drawTextBlob(GrDrawContext*, GrRenderTarget*, const GrClip&, const SkPaint&,
                      const SkMatrix& viewMatrix, const SkTextBlob*, SkScalar x, SkScalar y,
                      SkDrawFilter*, const SkIRect& clipBounds) override;

    class FallbackBlobBuilder;

    class TextRun {
    public:
        TextRun(const SkPaint& fontAndStroke);
        ~TextRun();

        void setText(const char text[], size_t byteLength, SkScalar x, SkScalar y,
                     GrContext*, const SkSurfaceProps*);

        void setPosText(const char text[], size_t byteLength,
                        const SkScalar pos[], int scalarsPerPosition, const SkPoint& offset,
                        GrContext*, const SkSurfaceProps*);

        void draw(GrDrawContext*, GrPipelineBuilder*, GrColor, const SkMatrix&,
                  SkScalar x, SkScalar y, const SkIRect& clipBounds,
                  GrTextContext* fallbackTextContext, const SkPaint& originalSkPaint) const;

        int cpuMemorySize() const;

    private:
        GrPathRange* createGlyphs(GrContext*, SkGlyphCache*);

        void appendGlyph(const SkGlyph&, const SkPoint&, FallbackBlobBuilder*);

        GrStrokeInfo                     fStroke;
        SkPaint                          fFont;
        SkScalar                         fTextRatio;
        float                            fTextInverseRatio;
        bool                             fUsingRawGlyphPaths;
        int                              fTotalGlyphCount;
        SkAutoTUnref<GrPathRangeDraw>    fDraw;
        SkAutoTUnref<const SkTextBlob>   fFallbackTextBlob;
        mutable SkMatrix                 fLocalMatrixTemplate;
    };

    // Text blobs/caches.

    class TextBlob : public SkTLList<TextRun> {
    public:
        typedef SkTArray<uint32_t, true> Key;

        static const Key& GetKey(const TextBlob* blob) { return blob->key(); }

        static uint32_t Hash(const Key& key) {
            SkASSERT(key.count() > 1); // 1-length keys should be using the blob-id hash map.
            return SkChecksum::Murmur3(key.begin(), sizeof(uint32_t) * key.count());
        }

        TextBlob(uint32_t blobId, const SkTextBlob* skBlob, const SkPaint& skPaint,
                 GrContext* ctx, const SkSurfaceProps* props)
            : fKey(&blobId, 1) { this->init(skBlob, skPaint, ctx, props); }

        TextBlob(const Key& key, const SkTextBlob* skBlob, const SkPaint& skPaint,
                 GrContext* ctx, const SkSurfaceProps* props)
            : fKey(key) {
            // 1-length keys are unterstood to be the blob id and must use the other constructor.
            SkASSERT(fKey.count() > 1);
            this->init(skBlob, skPaint, ctx, props);
        }

        const Key& key() const { return fKey; }

        int cpuMemorySize() const { return fCpuMemorySize; }

    private:
        void init(const SkTextBlob*, const SkPaint&, GrContext*, const SkSurfaceProps*);

        const SkSTArray<1, uint32_t, true>   fKey;
        int                                  fCpuMemorySize;

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(TextBlob);
    };

    const TextBlob& findOrCreateTextBlob(const SkTextBlob*, const SkPaint&);
    void purgeToFit(const TextBlob&);

    SkTHashMap<uint32_t, TextBlob*>                           fBlobIdCache;
    SkTHashTable<TextBlob*, const TextBlob::Key&, TextBlob>   fBlobKeyCache;
    SkTInternalLList<TextBlob>                                fLRUList;
    int                                                       fCacheSize;

    typedef GrTextContext INHERITED;
};

#endif
