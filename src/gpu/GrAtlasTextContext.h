/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasTextContext_DEFINED
#define GrAtlasTextContext_DEFINED

#include "GrTextContext.h"

#include "GrBatchAtlas.h"
#include "GrBatchFontCache.h"
#include "GrGeometryProcessor.h"
#include "SkDescriptor.h"
#include "GrMemoryPool.h"
#include "SkMaskFilter.h"
#include "SkTextBlob.h"
#include "SkTInternalLList.h"

#ifdef GR_TEST_UTILS
#include "GrBatchTest.h"
#endif

class BitmapTextBatch;
class GrPipelineBuilder;
class GrTextBlobCache;

/*
 * This class implements GrTextContext using standard bitmap fonts, and can also process textblobs.
 * TODO replace GrBitmapTextContext
 */
class GrAtlasTextContext : public GrTextContext {
public:
    static GrAtlasTextContext* Create(GrContext*, SkGpuDevice*, const SkDeviceProperties&,
                                      bool enableDistanceFields);

private:
    GrAtlasTextContext(GrContext*, SkGpuDevice*, const SkDeviceProperties&,
                       bool enableDistanceFields);
    ~GrAtlasTextContext() override {}

    bool canDraw(const GrRenderTarget*, const GrClip&, const GrPaint&,
                 const SkPaint&, const SkMatrix& viewMatrix) override;

    void onDrawText(GrRenderTarget*, const GrClip&, const GrPaint&, const SkPaint&,
                    const SkMatrix& viewMatrix, const char text[], size_t byteLength,
                    SkScalar x, SkScalar y, const SkIRect& regionClipBounds) override;
    void onDrawPosText(GrRenderTarget*, const GrClip&, const GrPaint&, const SkPaint&,
                       const SkMatrix& viewMatrix,
                       const char text[], size_t byteLength,
                       const SkScalar pos[], int scalarsPerPosition,
                       const SkPoint& offset, const SkIRect& regionClipBounds) override;
    void drawTextBlob(GrRenderTarget*, const GrClip&, const SkPaint&,
                      const SkMatrix& viewMatrix, const SkTextBlob*, SkScalar x, SkScalar y,
                      SkDrawFilter*, const SkIRect& clipBounds) override;

    /*
     * A BitmapTextBlob contains a fully processed SkTextBlob, suitable for nearly immediate drawing
     * on the GPU.  These are initially created with valid positions and colors, but invalid
     * texture coordinates.  The BitmapTextBlob itself has a few Blob-wide properties, and also
     * consists of a number of runs.  Runs inside a blob are flushed individually so they can be
     * reordered.
     *
     * The only thing(aside from a memcopy) required to flush a BitmapTextBlob is to ensure that
     * the GrAtlas will not evict anything the Blob needs.
     */
    struct BitmapTextBlob : public SkRefCnt {
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(BitmapTextBlob);

        /*
         * Each Run inside of the blob can have its texture coordinates regenerated if required.
         * To determine if regeneration is necessary, fAtlasGeneration is used.  If there have been
         * any evictions inside of the atlas, then we will simply regenerate Runs.  We could track
         * this at a more fine grained level, but its not clear if this is worth it, as evictions
         * should be fairly rare.
         *
         * One additional point, each run can contain glyphs with any of the three mask formats.
         * We call these SubRuns.  Because a subrun must be a contiguous range, we have to create
         * a new subrun each time the mask format changes in a run.  In theory, a run can have as
         * many SubRuns as it has glyphs, ie if a run alternates between color emoji and A8.  In
         * practice, the vast majority of runs have only a single subrun.
         *
         * Finally, for runs where the entire thing is too large for the GrAtlasTextContext to
         * handle, we have a bit to mark the run as flusahable via rendering as paths.  It is worth
         * pointing. It would be a bit expensive to figure out ahead of time whether or not a run
         * can flush in this manner, so we always allocate vertices for the run, regardless of
         * whether or not it is too large.  The benefit of this strategy is that we can always reuse
         * a blob allocation regardless of viewmatrix changes.  We could store positions for these
         * glyphs.  However, its not clear if this is a win because we'd still have to either go the
         * glyph cache to get the path at flush time, or hold onto the path in the cache, which
         * would greatly increase the memory of these cached items.
         */
        struct Run {
            Run()
                : fColor(GrColor_ILLEGAL)
                , fInitialized(false)
                , fDrawAsPaths(false) {
                fVertexBounds.setLargestInverted();
                // To ensure we always have one subrun, we push back a fresh run here
                fSubRunInfo.push_back();
            }
            struct SubRunInfo {
                SubRunInfo()
                    : fAtlasGeneration(GrBatchAtlas::kInvalidAtlasGeneration)
                    , fVertexStartIndex(0)
                    , fVertexEndIndex(0)
                    , fGlyphStartIndex(0)
                    , fGlyphEndIndex(0)
                    , fDrawAsDistanceFields(false) {}
                // Distance field text cannot draw coloremoji, and so has to fall back.  However,
                // though the distance field text and the coloremoji may share the same run, they
                // will have different descriptors.  If fOverrideDescriptor is non-NULL, then it
                // will be used in place of the run's descriptor to regen texture coords
                // TODO we could have a descriptor cache, it would reduce the size of these blobs
                // significantly, and then the subrun could just have a refed pointer to the
                // correct descriptor.
                GrBatchAtlas::BulkUseTokenUpdater fBulkUseToken;
                uint64_t fAtlasGeneration;
                size_t fVertexStartIndex;
                size_t fVertexEndIndex;
                uint32_t fGlyphStartIndex;
                uint32_t fGlyphEndIndex;
                SkScalar fTextRatio; // df property
                GrMaskFormat fMaskFormat;
                bool fDrawAsDistanceFields; // df property
                bool fUseLCDText; // df property
            };

            SubRunInfo& push_back() {
                // Forward glyph / vertex information to seed the new sub run
                SubRunInfo& prevSubRun = fSubRunInfo.back();
                SubRunInfo& newSubRun = fSubRunInfo.push_back();
                newSubRun.fGlyphStartIndex = prevSubRun.fGlyphEndIndex;
                newSubRun.fGlyphEndIndex = prevSubRun.fGlyphEndIndex;

                newSubRun.fVertexStartIndex = prevSubRun.fVertexEndIndex;
                newSubRun.fVertexEndIndex = prevSubRun.fVertexEndIndex;
                return newSubRun;
            }
            static const int kMinSubRuns = 1;
            SkAutoTUnref<GrBatchTextStrike> fStrike;
            SkAutoTUnref<SkTypeface> fTypeface;
            SkRect fVertexBounds;
            SkSTArray<kMinSubRuns, SubRunInfo> fSubRunInfo;
            SkAutoDescriptor fDescriptor;
            SkAutoTDelete<SkAutoDescriptor> fOverrideDescriptor; // df properties
            GrColor fColor;
            bool fInitialized;
            bool fDrawAsPaths;
        };

        struct BigGlyph {
            BigGlyph(const SkPath& path, SkScalar vx, SkScalar vy)
                : fPath(path)
                , fVx(vx)
                , fVy(vy) {}
            SkPath fPath;
            SkScalar fVx;
            SkScalar fVy;
        };

        struct Key {
            Key() {
                sk_bzero(this, sizeof(Key));
            }
            uint32_t fUniqueID;
            // Color may affect the gamma of the mask we generate, but in a fairly limited way.
            // Each color is assigned to on of a fixed number of buckets based on its
            // luminance. For each luminance bucket there is a "canonical color" that
            // represents the bucket.  This functionality is currently only supported for A8
            SkColor fCanonicalColor;
            SkPaint::Style fStyle;
            SkPixelGeometry fPixelGeometry;
            bool fHasBlur;

            bool operator==(const Key& other) const {
                return 0 == memcmp(this, &other, sizeof(Key));
            }
        };

        struct StrokeInfo {
            SkScalar fFrameWidth;
            SkScalar fMiterLimit;
            SkPaint::Join fJoin;
        };

        enum TextType {
            kHasDistanceField_TextType = 0x1,
            kHasBitmap_TextType = 0x2,
        };

        // all glyph / vertex offsets are into these pools.
        unsigned char* fVertices;
        GrGlyph** fGlyphs;
        Run* fRuns;
        GrMemoryPool* fPool;
        SkMaskFilter::BlurRec fBlurRec;
        StrokeInfo fStrokeInfo;
        SkTArray<BigGlyph> fBigGlyphs;
        Key fKey;
        SkMatrix fViewMatrix;
        SkColor fPaintColor;
        SkScalar fX;
        SkScalar fY;

        // We can reuse distance field text, but only if the new viewmatrix would not result in
        // a mip change.  Because there can be multiple runs in a blob, we track the overall
        // maximum minimum scale, and minimum maximum scale, we can support before we need to regen
        SkScalar fMaxMinScale;
        SkScalar fMinMaxScale;
        int fRunCount;
        uint8_t fTextType;

        BitmapTextBlob()
            : fMaxMinScale(-SK_ScalarMax)
            , fMinMaxScale(SK_ScalarMax)
            , fTextType(0) {}

        ~BitmapTextBlob() override {
            for (int i = 0; i < fRunCount; i++) {
                fRuns[i].~Run();
            }
        }

        static const Key& GetKey(const BitmapTextBlob& blob) {
            return blob.fKey;
        }

        static uint32_t Hash(const Key& key) {
            return SkChecksum::Murmur3(&key, sizeof(Key));
        }

        void operator delete(void* p) {
            BitmapTextBlob* blob = reinterpret_cast<BitmapTextBlob*>(p);
            blob->fPool->release(p);
        }
        void* operator new(size_t) {
            SkFAIL("All blobs are created by placement new.");
            return sk_malloc_throw(0);
        }

        void* operator new(size_t, void* p) { return p; }
        void operator delete(void* target, void* placement) {
            ::operator delete(target, placement);
        }

        bool hasDistanceField() const { return SkToBool(fTextType & kHasDistanceField_TextType); }
        bool hasBitmap() const { return SkToBool(fTextType & kHasBitmap_TextType); }
        void setHasDistanceField() { fTextType |= kHasDistanceField_TextType; }
        void setHasBitmap() { fTextType |= kHasBitmap_TextType; }
    };

    typedef BitmapTextBlob::Run Run;
    typedef Run::SubRunInfo PerSubRunInfo;

    inline bool canDrawAsDistanceFields(const SkPaint&, const SkMatrix& viewMatrix);
    BitmapTextBlob* setupDFBlob(int glyphCount, const SkPaint& origPaint,
                                const SkMatrix& viewMatrix, SkGlyphCache** cache,
                                SkPaint* dfPaint, SkScalar* textRatio);
    void bmpAppendGlyph(BitmapTextBlob*, int runIndex, GrGlyph::PackedID, int left, int top,
                        GrColor color, GrFontScaler*, const SkIRect& clipRect);
    bool dfAppendGlyph(BitmapTextBlob*, int runIndex, GrGlyph::PackedID, SkScalar sx, SkScalar sy,
                       GrColor color, GrFontScaler*, const SkIRect& clipRect, SkScalar textRatio,
                       const SkMatrix& viewMatrix);
    inline void appendGlyphPath(BitmapTextBlob* blob, GrGlyph* glyph,
                                GrFontScaler* scaler, SkScalar x, SkScalar y);
    inline void appendGlyphCommon(BitmapTextBlob*, Run*, Run::SubRunInfo*,
                                  const SkRect& positions, GrColor color,
                                  size_t vertexStride, bool useVertexColor,
                                  GrGlyph*);

    inline void flushRunAsPaths(const SkTextBlob::RunIterator&, const SkPaint&, SkDrawFilter*,
                                const SkMatrix& viewMatrix, const SkIRect& clipBounds, SkScalar x,
                                SkScalar y);
    inline BitmapTextBatch* createBatch(BitmapTextBlob*, const PerSubRunInfo&,
                                        int glyphCount, int run, int subRun,
                                        GrColor, SkScalar transX, SkScalar transY,
                                        const SkPaint&);
    inline void flushRun(GrDrawTarget*, GrPipelineBuilder*, BitmapTextBlob*, int run, GrColor,
                         SkScalar transX, SkScalar transY, const SkPaint&);
    inline void flushBigGlyphs(BitmapTextBlob* cacheBlob, GrRenderTarget* rt,
                               const SkPaint& skPaint,
                               SkScalar transX, SkScalar transY, const SkIRect& clipBounds);

    // We have to flush SkTextBlobs differently from drawText / drawPosText
    void flush(GrDrawTarget*, const SkTextBlob*, BitmapTextBlob*, GrRenderTarget*, const SkPaint&,
               const GrPaint&, SkDrawFilter*, const GrClip&, const SkMatrix& viewMatrix,
               const SkIRect& clipBounds, SkScalar x, SkScalar y, SkScalar transX, SkScalar transY);
    void flush(GrDrawTarget*, BitmapTextBlob*, GrRenderTarget*, const SkPaint&,
               const GrPaint&, const GrClip&, const SkIRect& clipBounds);

    // A helper for drawing BitmapText in a run of distance fields
    inline void fallbackDrawPosText(BitmapTextBlob*, int runIndex,
                                    GrRenderTarget*, const GrClip&,
                                    const GrPaint&,
                                    const SkPaint&, const SkMatrix& viewMatrix,
                                    const SkTDArray<char>& fallbackTxt,
                                    const SkTDArray<SkScalar>& fallbackPos,
                                    int scalarsPerPosition,
                                    const SkPoint& offset,
                                    const SkIRect& clipRect);

    void internalDrawBMPText(BitmapTextBlob*, int runIndex, SkGlyphCache*, const SkPaint&,
                             GrColor color, const SkMatrix& viewMatrix,
                             const char text[], size_t byteLength,
                             SkScalar x, SkScalar y, const SkIRect& clipRect);
    void internalDrawBMPPosText(BitmapTextBlob*, int runIndex, SkGlyphCache*, const SkPaint&,
                                GrColor color, const SkMatrix& viewMatrix,
                                const char text[], size_t byteLength,
                                const SkScalar pos[], int scalarsPerPosition,
                                const SkPoint& offset, const SkIRect& clipRect);

    void internalDrawDFText(BitmapTextBlob*, int runIndex, SkGlyphCache*, const SkPaint&,
                            GrColor color, const SkMatrix& viewMatrix,
                            const char text[], size_t byteLength,
                            SkScalar x, SkScalar y, const SkIRect& clipRect,
                            SkScalar textRatio,
                            SkTDArray<char>* fallbackTxt,
                            SkTDArray<SkScalar>* fallbackPos,
                            SkPoint* offset, const SkPaint& origPaint);
    void internalDrawDFPosText(BitmapTextBlob*, int runIndex, SkGlyphCache*, const SkPaint&,
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
                                          const BitmapTextBlob&, const SkPaint&,
                                          const SkMaskFilter::BlurRec&,
                                          const SkMatrix& viewMatrix, SkScalar x, SkScalar y);
    void regenerateTextBlob(BitmapTextBlob* bmp, const SkPaint& skPaint, GrColor,
                            const SkMatrix& viewMatrix,
                            const SkTextBlob* blob, SkScalar x, SkScalar y,
                            SkDrawFilter* drawFilter, const SkIRect& clipRect, GrRenderTarget*,
                            const GrClip&, const GrPaint&);
    inline static bool HasLCD(const SkTextBlob*);
    inline void initDistanceFieldPaint(BitmapTextBlob*, SkPaint*, SkScalar* textRatio,
                                       const SkMatrix&);

    // Test methods
    // TODO this is really ugly.  It'd be much nicer if positioning could be moved to batch
    inline BitmapTextBlob* createDrawTextBlob(GrRenderTarget*, const GrClip&, const GrPaint&,
                                              const SkPaint&, const SkMatrix& viewMatrix,
                                              const char text[], size_t byteLength,
                                              SkScalar x, SkScalar y,
                                              const SkIRect& regionClipBounds);
    inline BitmapTextBlob* createDrawPosTextBlob(GrRenderTarget*, const GrClip&, const GrPaint&,
                                                 const SkPaint&, const SkMatrix& viewMatrix,
                                                 const char text[], size_t byteLength,
                                                 const SkScalar pos[], int scalarsPerPosition,
                                                 const SkPoint& offset,
                                                 const SkIRect& regionClipBounds);

    // Distance field text needs this table to compute a value for use in the fragment shader.
    // Because the GrAtlasTextContext can go out of scope before the final flush, this needs to be
    // refcnted and malloced
    struct DistanceAdjustTable : public SkNVRefCnt<DistanceAdjustTable> {
        DistanceAdjustTable(float gamma) { this->buildDistanceAdjustTable(gamma); }
        ~DistanceAdjustTable() { SkDELETE_ARRAY(fTable); }

        void buildDistanceAdjustTable(float gamma);

        SkScalar& operator[] (int i) {
            return fTable[i];
        }

        const SkScalar& operator[] (int i) const {
            return fTable[i];
        }

        SkScalar* fTable;
    };

    GrBatchTextStrike* fCurrStrike;
    GrTextBlobCache* fCache;
    bool fEnableDFRendering;
    SkAutoTUnref<DistanceAdjustTable> fDistanceAdjustTable;

    friend class GrTextBlobCache;
    friend class BitmapTextBatch;

#ifdef GR_TEST_UTILS
    BATCH_TEST_FRIEND(TextBlobBatch);
#endif

    typedef GrTextContext INHERITED;
};

#endif
