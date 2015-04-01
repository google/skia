/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBitmapTextContext_DEFINED
#define GrBitmapTextContext_DEFINED

#include "GrTextContext.h"

#include "GrGeometryProcessor.h"
#include "GrMemoryPool.h"
#include "SkDescriptor.h"
#include "SkTHash.h"

class GrBatchTextStrike;
class GrPipelineBuilder;

/*
 * This class implements GrTextContext using standard bitmap fonts, and can also process textblobs.
 * TODO replace GrBitmapTextContext
 */
class GrBitmapTextContextB : public GrTextContext {
public:
    static GrBitmapTextContextB* Create(GrContext*, SkGpuDevice*, const SkDeviceProperties&);

    virtual ~GrBitmapTextContextB();

private:
    GrBitmapTextContextB(GrContext*, SkGpuDevice*, const SkDeviceProperties&);

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

    void init(GrRenderTarget*, const GrClip&, const GrPaint&, const SkPaint&,
              const SkIRect& regionClipBounds);

    /*
     * A BitmapTextBlob contains a fully processed SkTextBlob, suitable for nearly immediate drawing
     * on the GPU.  These are initially created with valid positions and colors, but invalid
     * texture coordinates.  The BitmapTextBlob itself has a few Blob-wide properties, and also
     * consists of a number of runs.  Runs inside a blob are flushed individually so they can be
     * reordered.
     *
     * The only thing(aside from a memcopy) required to flush a BitmapTextBlob is to ensure that
     * the GrAtlas will not evict anything the Blob needs.
     * TODO this is currently a bug
     */
    struct BitmapTextBlob : public SkRefCnt {
        // Each Run inside of the blob can have its texture coordinates regenerated if required.
        // To determine if regeneration is necessary, fAtlasGeneration is used.  If there have been
        // any evictions inside of the atlas, then we will simply regenerate Runs.  We could track
        // this at a more fine grained level, but its not clear if this is worth it, as evictions
        // should be fairly rare.
        // One additional point, each run can contain glyphs with any of the three mask formats.
        // We call these SubRuns.  Because a subrun must be a contiguous range, we have to create
        // a new subrun each time the mask format changes in a run.  In theory, a run can have as
        // many SubRuns as it has glyphs, ie if a run alternates between color emoji and A8.  In
        // practice, the vast majority of runs have only a single subrun.

        struct Run {
            Run() : fColor(GrColor_ILLEGAL), fInitialized(false) {
                fVertexBounds.setLargestInverted();
                // We insert the first subrun to gurantee a run always has atleast one subrun.
                // We do this to simplify things when we 'hand off' data from one subrun to the
                // next
                fSubRunInfo.push_back();
            }
            struct SubRunInfo {
                SubRunInfo()
                    : fAtlasGeneration(GrBatchAtlas::kInvalidAtlasGeneration)
                    , fGlyphStartIndex(0)
                    , fGlyphEndIndex(0)
                    , fVertexStartIndex(0)
                    , fVertexEndIndex(0) {}
                GrMaskFormat fMaskFormat;
                uint64_t fAtlasGeneration;
                uint32_t fGlyphStartIndex;
                uint32_t fGlyphEndIndex;
                size_t fVertexStartIndex;
                size_t fVertexEndIndex;
            };
            SkSTArray<1, SubRunInfo, true> fSubRunInfo;
            SkAutoDescriptor fDescriptor;
            SkAutoTUnref<SkTypeface> fTypeface;
            SkRect fVertexBounds;
            GrColor fColor;
            bool fInitialized;
        };

        struct BigGlyph {
            BigGlyph(const SkPath& path, int vx, int vy) : fPath(path), fVx(vx), fVy(vy) {}
            SkPath fPath;
            int fVx;
            int fVy;
        };

        SkTArray<BigGlyph> fBigGlyphs;
        SkMatrix fViewMatrix;
        SkScalar fX;
        SkScalar fY;
        SkPaint::Style fStyle;
        uint32_t fRunCount;

        // all glyph / vertex offsets are into these pools.
        unsigned char* fVertices;
        GrGlyph::PackedID* fGlyphIDs;
        Run* fRuns;

        static uint32_t Hash(const uint32_t& key) {
            return SkChecksum::Mix(key);
        }

        void operator delete(void* p) { sk_free(p); }
        void* operator new(size_t) {
            SkFAIL("All blobs are created by placement new.");
            return sk_malloc_throw(0);
        }

        void* operator new(size_t, void* p) { return p; }
        void operator delete(void* target, void* placement) {
            ::operator delete(target, placement);
        }
    };

    typedef BitmapTextBlob::Run Run;
    typedef Run::SubRunInfo PerSubRunInfo;

    BitmapTextBlob* CreateBlob(int glyphCount, int runCount);

    void appendGlyph(BitmapTextBlob*, int runIndex, GrGlyph::PackedID, int left, int top,
                     GrFontScaler*, const SkIRect& clipRect);
    void flush(GrDrawTarget*, BitmapTextBlob*, GrRenderTarget*, const GrPaint&, const GrClip&,
               const SkMatrix& viewMatrix, int paintAlpha);

    void internalDrawText(BitmapTextBlob*, int runIndex, SkGlyphCache*, const SkPaint&,
                          const SkMatrix& viewMatrix, const char text[], size_t byteLength,
                          SkScalar x, SkScalar y, const SkIRect& clipRect);
    void internalDrawPosText(BitmapTextBlob*, int runIndex, SkGlyphCache*, const SkPaint&,
                             const SkMatrix& viewMatrix,
                             const char text[], size_t byteLength,
                             const SkScalar pos[], int scalarsPerPosition,
                             const SkPoint& offset, const SkIRect& clipRect);

    // sets up the descriptor on the blob and returns a detached cache.  Client must attach
    inline SkGlyphCache* setupCache(Run*, const SkPaint&, const SkMatrix& viewMatrix);
    static inline bool MustRegenerateBlob(const BitmapTextBlob&, const SkPaint&,
                                          const SkMatrix& viewMatrix, SkScalar x, SkScalar y);
    void regenerateTextBlob(BitmapTextBlob* bmp, const SkPaint& skPaint, const SkMatrix& viewMatrix,
                            const SkTextBlob* blob, SkScalar x, SkScalar y,
                            SkDrawFilter* drawFilter, const SkIRect& clipRect);

    // TODO this currently only uses the public interface of SkTextBlob, however, I may need to add
    // functionality to it while looping over the runs so I'm putting this here for the time being.
    // If this lands in Chrome without changes, move it to SkTextBlob.
    static inline void BlobGlyphCount(int* glyphCount, int* runCount, const SkTextBlob*);

    GrBatchTextStrike* fCurrStrike;

    // TODO use real cache
    static void ClearCacheEntry(uint32_t key, BitmapTextBlob**);
    SkTHashMap<uint32_t, BitmapTextBlob*, BitmapTextBlob::Hash> fCache;

    friend class BitmapTextBatch;

    typedef GrTextContext INHERITED;
};

class GrTextStrike;

/*
 * This class implements GrTextContext using standard bitmap fonts
 */
class GrBitmapTextContext : public GrTextContext {
public:
    static GrBitmapTextContext* Create(GrContext*, SkGpuDevice*,  const SkDeviceProperties&);

    virtual ~GrBitmapTextContext() {}

private:
    GrTextStrike*                     fStrike;
    void*                             fVertices;
    int                               fCurrVertex;
    int                               fAllocVertexCount;
    int                               fTotalVertexCount;
    SkRect                            fVertexBounds;
    GrTexture*                        fCurrTexture;
    GrMaskFormat                      fCurrMaskFormat;
    SkAutoTUnref<const GrGeometryProcessor> fCachedGeometryProcessor;
    // Used to check whether fCachedEffect is still valid.
    uint32_t                          fEffectTextureUniqueID;
    SkMatrix                          fLocalMatrix;

    GrBitmapTextContext(GrContext*, SkGpuDevice*, const SkDeviceProperties&);

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

    void init(GrRenderTarget*, const GrClip&, const GrPaint&, const SkPaint&,
              const SkIRect& regionClipBounds);
    void appendGlyph(GrGlyph::PackedID, SkFixed left, SkFixed top, GrFontScaler*);
    bool uploadGlyph(GrGlyph*, GrFontScaler*);
    void flush();                 // automatically called by destructor
    void finish();
};

#endif
