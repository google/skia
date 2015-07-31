/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasTextBlob_DEFINED
#define GrAtlasTextBlob_DEFINED

#include "GrBatchAtlas.h"
#include "GrBatchFontCache.h"
#include "GrColor.h"
#include "SkDescriptor.h"
#include "SkMaskFilter.h"
#include "GrMemoryPool.h"
#include "SkSurfaceProps.h"
#include "SkTInternalLList.h"

// With this flag enabled, the GrAtlasTextContext will, as a sanity check, regenerate every blob
// that comes in to verify the integrity of its cache
//#define CACHE_SANITY_CHECK // VERY SLOW

/*
 * A GrAtlasTextBlob contains a fully processed SkTextBlob, suitable for nearly immediate drawing
 * on the GPU.  These are initially created with valid positions and colors, but invalid
 * texture coordinates.  The GrAtlasTextBlob itself has a few Blob-wide properties, and also
 * consists of a number of runs.  Runs inside a blob are flushed individually so they can be
 * reordered.
 *
 * The only thing(aside from a memcopy) required to flush a GrAtlasTextBlob is to ensure that
 * the GrAtlas will not evict anything the Blob needs.
 *
 * Note: This struct should really be named GrCachedAtasTextBlob, but that is too verbose.
 *
 * *WARNING* If you add new fields to this struct, then you may need to to update AssertEqual
 */
struct GrAtlasTextBlob : public SkRefCnt {
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(GrAtlasTextBlob);

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
            SubRunInfo(const SubRunInfo& that)
                : fBulkUseToken(that.fBulkUseToken)
                , fStrike(SkSafeRef(that.fStrike.get()))
                , fAtlasGeneration(that.fAtlasGeneration)
                , fVertexStartIndex(that.fVertexStartIndex)
                , fVertexEndIndex(that.fVertexEndIndex)
                , fGlyphStartIndex(that.fGlyphStartIndex)
                , fGlyphEndIndex(that.fGlyphEndIndex)
                , fTextRatio(that.fTextRatio)
                , fMaskFormat(that.fMaskFormat)
                , fDrawAsDistanceFields(that.fDrawAsDistanceFields)
                , fUseLCDText(that.fUseLCDText) {
            }
            // Distance field text cannot draw coloremoji, and so has to fall back.  However,
            // though the distance field text and the coloremoji may share the same run, they
            // will have different descriptors.  If fOverrideDescriptor is non-NULL, then it
            // will be used in place of the run's descriptor to regen texture coords
            // TODO we could have a descriptor cache, it would reduce the size of these blobs
            // significantly, and then the subrun could just have a refed pointer to the
            // correct descriptor.
            GrBatchAtlas::BulkUseTokenUpdater fBulkUseToken;
            SkAutoTUnref<GrBatchTextStrike> fStrike;
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
            SubRunInfo& newSubRun = fSubRunInfo.push_back();
            SubRunInfo& prevSubRun = fSubRunInfo.fromBack(1);

            newSubRun.fGlyphStartIndex = prevSubRun.fGlyphEndIndex;
            newSubRun.fGlyphEndIndex = prevSubRun.fGlyphEndIndex;

            newSubRun.fVertexStartIndex = prevSubRun.fVertexEndIndex;
            newSubRun.fVertexEndIndex = prevSubRun.fVertexEndIndex;
            return newSubRun;
        }
        static const int kMinSubRuns = 1;
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

    GrAtlasTextBlob()
        : fMaxMinScale(-SK_ScalarMax)
        , fMinMaxScale(SK_ScalarMax)
        , fTextType(0) {}

    ~GrAtlasTextBlob() override {
        for (int i = 0; i < fRunCount; i++) {
            fRuns[i].~Run();
        }
    }

    static const Key& GetKey(const GrAtlasTextBlob& blob) {
        return blob.fKey;
    }

    static uint32_t Hash(const Key& key) {
        return SkChecksum::Murmur3(&key, sizeof(Key));
    }

    void operator delete(void* p) {
        GrAtlasTextBlob* blob = reinterpret_cast<GrAtlasTextBlob*>(p);
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

#ifdef CACHE_SANITY_CHECK
    static void AssertEqual(const GrAtlasTextBlob&, const GrAtlasTextBlob&);
    size_t fSize;
#endif
};

#endif
