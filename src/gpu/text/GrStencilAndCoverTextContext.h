/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStencilAndCoverTextContext_DEFINED
#define GrStencilAndCoverTextContext_DEFINED

#include "GrRenderTargetContext.h"
#include "GrStyle.h"
#include "SkDrawFilter.h"
#include "SkOpts.h"
#include "SkTHash.h"
#include "SkTInternalLList.h"
#include "SkTLList.h"
#include "SkTextBlob.h"
#include "ops/GrDrawPathOp.h"

class GrAtlasTextContext;
class GrTextStrike;
class GrPath;
class SkSurfaceProps;

/*
 * This class implements text rendering using stencil and cover path rendering
 * (by the means of GrOpList::drawPath).
 */
class GrStencilAndCoverTextContext {
public:
    static GrStencilAndCoverTextContext* Create(GrAtlasTextContext* fallbackTextContext);

    void drawText(GrContext*, GrRenderTargetContext* rtc, const GrClip&, const SkPaint&,
                  const SkMatrix& viewMatrix, const SkSurfaceProps&, const char text[],
                  size_t byteLength, SkScalar x, SkScalar y, const SkIRect& clipBounds);
    void drawPosText(GrContext*, GrRenderTargetContext*, const GrClip&, const SkPaint&,
                     const SkMatrix& viewMatrix, const SkSurfaceProps&, const char text[],
                     size_t byteLength, const SkScalar pos[], int scalarsPerPosition,
                     const SkPoint& offset, const SkIRect& clipBounds);
    void drawTextBlob(GrContext*, GrRenderTargetContext*, const GrClip&, const SkPaint&,
                      const SkMatrix& viewMatrix, const SkSurfaceProps&, const SkTextBlob*,
                      SkScalar x, SkScalar y,
                      SkDrawFilter*, const SkIRect& clipBounds);

    virtual ~GrStencilAndCoverTextContext();

private:
    GrStencilAndCoverTextContext(GrAtlasTextContext* fallbackTextContext);

    bool canDraw(const SkPaint& skPaint, const SkMatrix&) {
        return this->internalCanDraw(skPaint);
    }

    bool internalCanDraw(const SkPaint&);

    void uncachedDrawTextBlob(GrContext*, GrRenderTargetContext* rtc,
                              const GrClip& clip, const SkPaint& skPaint,
                              const SkMatrix& viewMatrix,
                              const SkSurfaceProps&,
                              const SkTextBlob* blob,
                              SkScalar x, SkScalar y,
                              SkDrawFilter* drawFilter,
                              const SkIRect& clipBounds);

    class FallbackBlobBuilder;

    class TextRun {
    public:
        TextRun(const SkPaint& fontAndStroke);
        ~TextRun();

        void setText(const char text[], size_t byteLength, SkScalar x, SkScalar y);

        void setPosText(const char text[], size_t byteLength, const SkScalar pos[],
                        int scalarsPerPosition, const SkPoint& offset);

        void draw(GrContext*, GrRenderTargetContext*, const GrClip&, const SkMatrix&,
                  const SkSurfaceProps&, SkScalar x, SkScalar y, const SkIRect& clipBounds,
                  GrAtlasTextContext* fallbackTextContext, const SkPaint& originalSkPaint) const;

        void releaseGlyphCache() const;

        size_t computeSizeInCache() const;

        GrAA aa() const { return fFont.isAntiAlias() ? GrAA::kYes : GrAA::kNo; }

    private:
        typedef GrDrawPathRangeOp::InstanceData InstanceData;

        SkGlyphCache* getGlyphCache() const;
        sk_sp<GrPathRange> createGlyphs(GrResourceProvider*) const;
        void appendGlyph(const SkGlyph&, const SkPoint&, FallbackBlobBuilder*);

        GrStyle                         fStyle;
        SkPaint                         fFont;
        SkScalar                        fTextRatio;
        float                           fTextInverseRatio;
        bool                            fUsingRawGlyphPaths;
        GrUniqueKey                     fGlyphPathsKey;
        int                             fTotalGlyphCount;
        sk_sp<InstanceData>             fInstanceData;
        int                             fFallbackGlyphCount;
        sk_sp<SkTextBlob>               fFallbackTextBlob;
        mutable SkGlyphCache*           fDetachedGlyphCache;
        mutable GrGpuResource::UniqueID fLastDrawnGlyphsID;
    };

    // Text blobs/caches.

    class TextBlob : public SkTLList<TextRun, 1> {
    public:
        typedef SkTArray<uint32_t, true> Key;

        static const Key& GetKey(const TextBlob* blob) { return blob->key(); }

        static uint32_t Hash(const Key& key) {
            SkASSERT(key.count() > 1); // 1-length keys should be using the blob-id hash map.
            return SkOpts::hash(key.begin(), sizeof(uint32_t) * key.count());
        }

        TextBlob(uint32_t blobId, const SkTextBlob* skBlob, const SkPaint& skPaint)
            : fKey(&blobId, 1) { this->init(skBlob, skPaint); }

        TextBlob(const Key& key, const SkTextBlob* skBlob, const SkPaint& skPaint)
            : fKey(key) {
            // 1-length keys are unterstood to be the blob id and must use the other constructor.
            SkASSERT(fKey.count() > 1);
            this->init(skBlob, skPaint);
        }

        const Key& key() const { return fKey; }

        size_t cpuMemorySize() const { return fCpuMemorySize; }

    private:
        void init(const SkTextBlob*, const SkPaint&);

        const SkSTArray<1, uint32_t, true>   fKey;
        size_t                               fCpuMemorySize;

        SK_DECLARE_INTERNAL_LLIST_INTERFACE(TextBlob);
    };

    const TextBlob& findOrCreateTextBlob(const SkTextBlob*, const SkPaint&);
    void purgeToFit(const TextBlob&);

    GrAtlasTextContext*                                       fFallbackTextContext;
    SkTHashMap<uint32_t, TextBlob*>                           fBlobIdCache;
    SkTHashTable<TextBlob*, const TextBlob::Key&, TextBlob>   fBlobKeyCache;
    SkTInternalLList<TextBlob>                                fLRUList;
    size_t                                                    fCacheSize;
};

#endif
