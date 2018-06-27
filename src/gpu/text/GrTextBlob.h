/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextBlob_DEFINED
#define GrTextBlob_DEFINED

#include "GrColor.h"
#include "GrDrawOpAtlas.h"
#include "GrGlyphCache.h"
#include "GrTextUtils.h"
#include "SkDescriptor.h"
#include "SkMaskFilterBase.h"
#include "SkOpts.h"
#include "SkPathEffect.h"
#include "SkPoint3.h"
#include "SkRectPriv.h"
#include "SkSurfaceProps.h"
#include "SkTInternalLList.h"

class GrAtlasManager;
struct GrDistanceFieldAdjustTable;
struct GrGlyph;

class SkTextBlob;
class SkTextBlobRunIterator;

// With this flag enabled, the GrTextContext will, as a sanity check, regenerate every blob
// that comes in to verify the integrity of its cache
#define CACHE_SANITY_CHECK 0

/*
 * A GrTextBlob contains a fully processed SkTextBlob, suitable for nearly immediate drawing
 * on the GPU.  These are initially created with valid positions and colors, but invalid
 * texture coordinates.  The GrTextBlob itself has a few Blob-wide properties, and also
 * consists of a number of runs.  Runs inside a blob are flushed individually so they can be
 * reordered.
 *
 * The only thing(aside from a memcopy) required to flush a GrTextBlob is to ensure that
 * the GrAtlas will not evict anything the Blob needs.
 *
 * Note: This struct should really be named GrCachedAtasTextBlob, but that is too verbose.
 *
 * *WARNING* If you add new fields to this struct, then you may need to to update AssertEqual
 */
class GrTextBlob : public SkNVRefCnt<GrTextBlob> {
public:
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(GrTextBlob);

    class VertexRegenerator;

    static sk_sp<GrTextBlob> Make(int glyphCount, int runCount);

    /**
     * We currently force regeneration of a blob if old or new matrix differ in having perspective.
     * If we ever change that then the key must contain the perspectiveness when there are distance
     * fields as perspective distance field use 3 component vertex positions and non-perspective
     * uses 2.
     */
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
        uint32_t fScalerContextFlags;

        bool operator==(const Key& other) const {
            return 0 == memcmp(this, &other, sizeof(Key));
        }
    };

    void setupKey(const GrTextBlob::Key& key,
                  const SkMaskFilterBase::BlurRec& blurRec,
                  const SkPaint& paint) {
        fKey = key;
        if (key.fHasBlur) {
            fBlurRec = blurRec;
        }
        if (key.fStyle != SkPaint::kFill_Style) {
            fStrokeInfo.fFrameWidth = paint.getStrokeWidth();
            fStrokeInfo.fMiterLimit = paint.getStrokeMiter();
            fStrokeInfo.fJoin = paint.getStrokeJoin();
        }
    }

    static const Key& GetKey(const GrTextBlob& blob) {
        return blob.fKey;
    }

    static uint32_t Hash(const Key& key) {
        return SkOpts::hash(&key, sizeof(Key));
    }

    void operator delete(void* p) {
        ::operator delete(p);
    }

    void* operator new(size_t) {
        SK_ABORT("All blobs are created by placement new.");
        return sk_malloc_throw(0);
    }

    void* operator new(size_t, void* p) { return p; }

    bool hasDistanceField() const { return SkToBool(fTextType & kHasDistanceField_TextType); }
    bool hasBitmap() const { return SkToBool(fTextType & kHasBitmap_TextType); }
    void setHasDistanceField() { fTextType |= kHasDistanceField_TextType; }
    void setHasBitmap() { fTextType |= kHasBitmap_TextType; }

    int runCount() const { return fRunCount; }

    void push_back_run(int currRun) {
        SkASSERT(currRun < fRunCount);
        if (currRun > 0) {
            Run::SubRunInfo& newRun = fRuns[currRun].fSubRunInfo.back();
            Run::SubRunInfo& lastRun = fRuns[currRun - 1].fSubRunInfo.back();
            newRun.setAsSuccessor(lastRun);
        }
    }

    // sets the last subrun of runIndex to use distance field text
    void setSubRunHasDistanceFields(int runIndex, bool hasLCD, bool isAntiAlias, bool hasWCoord) {
        Run& run = fRuns[runIndex];
        Run::SubRunInfo& subRun = run.fSubRunInfo.back();
        subRun.setUseLCDText(hasLCD);
        subRun.setAntiAliased(isAntiAlias);
        subRun.setDrawAsDistanceFields();
        subRun.setHasWCoord(hasWCoord);
    }

    // sets the last subrun of runIndex to use w values
    void setSubRunHasW(int runIndex, bool hasWCoord) {
        Run& run = fRuns[runIndex];
        Run::SubRunInfo& subRun = run.fSubRunInfo.back();
        subRun.setHasWCoord(hasWCoord);
    }

    void setRunPaintFlags(int runIndex, uint16_t paintFlags) {
        fRuns[runIndex].fPaintFlags = paintFlags & Run::kPaintFlagsMask;
    }

    void setMinAndMaxScale(SkScalar scaledMax, SkScalar scaledMin) {
        // we init fMaxMinScale and fMinMaxScale in the constructor
        fMaxMinScale = SkMaxScalar(scaledMax, fMaxMinScale);
        fMinMaxScale = SkMinScalar(scaledMin, fMinMaxScale);
    }

    // inits the override descriptor on the current run.  All following subruns must use this
    // descriptor
    void initOverride(int runIndex) {
        Run& run = fRuns[runIndex];
        // Push back a new subrun to fill and set the override descriptor
        run.push_back();
        run.fOverrideDescriptor.reset(new SkAutoDescriptor);
    }

    SkExclusiveStrikePtr setupCache(int runIndex,
                                    const SkSurfaceProps& props,
                                    SkScalerContextFlags scalerContextFlags,
                                    const SkPaint& skPaint,
                                    const SkMatrix* viewMatrix);

    // Appends a glyph to the blob.  If the glyph is too large, the glyph will be appended
    // as a path.
    void appendGlyph(int runIndex,
                     const SkRect& positions,
                     GrColor color,
                     sk_sp<GrTextStrike> strike,
                     GrGlyph* glyph,
                     SkGlyphCache*, const SkGlyph& skGlyph,
                     SkScalar x, SkScalar y, SkScalar scale, bool preTransformed);

    // Appends a glyph to the blob as a path only.
    void appendPathGlyph(int runIndex, const SkPath& path,
                         SkScalar x, SkScalar y, SkScalar scale, bool preTransformed);

    static size_t GetVertexStride(GrMaskFormat maskFormat, bool hasWCoord) {
        switch (maskFormat) {
            case kA8_GrMaskFormat:
                return hasWCoord ? kGrayTextDFPerspectiveVASize : kGrayTextVASize;
            case kARGB_GrMaskFormat:
                return hasWCoord ? kColorTextPerspectiveVASize : kColorTextVASize;
            default:
                SkASSERT(!hasWCoord);
                return kLCDTextVASize;
        }
    }

    bool mustRegenerate(const GrTextUtils::Paint&, const SkMaskFilterBase::BlurRec& blurRec,
                        const SkMatrix& viewMatrix, SkScalar x, SkScalar y);

    void flush(GrTextUtils::Target*, const SkSurfaceProps& props,
               const GrDistanceFieldAdjustTable* distanceAdjustTable,
               const GrTextUtils::Paint& paint, const GrClip& clip,
               const SkMatrix& viewMatrix, const SkIRect& clipBounds, SkScalar x,
               SkScalar y);

    void computeSubRunBounds(SkRect* outBounds, int runIndex, int subRunIndex,
                             const SkMatrix& viewMatrix, SkScalar x, SkScalar y,
                             bool needsGlyphTransform) {
        // We don't yet position distance field text on the cpu, so we have to map the vertex bounds
        // into device space.
        // We handle vertex bounds differently for distance field text and bitmap text because
        // the vertex bounds of bitmap text are in device space.  If we are flushing multiple runs
        // from one blob then we are going to pay the price here of mapping the rect for each run.
        const Run& run = fRuns[runIndex];
        const Run::SubRunInfo& subRun = run.fSubRunInfo[subRunIndex];
        *outBounds = subRun.vertexBounds();
        if (needsGlyphTransform) {
            // Distance field text is positioned with the (X,Y) as part of the glyph position,
            // and currently the view matrix is applied on the GPU
            outBounds->offset(x - fInitialX, y - fInitialY);
            viewMatrix.mapRect(outBounds);
        } else {
            // Bitmap text is fully positioned on the CPU, and offset by an (X,Y) translate in
            // device space.
            SkMatrix boundsMatrix = fInitialViewMatrixInverse;

            boundsMatrix.postTranslate(-fInitialX, -fInitialY);

            boundsMatrix.postTranslate(x, y);

            boundsMatrix.postConcat(viewMatrix);
            boundsMatrix.mapRect(outBounds);

            // Due to floating point numerical inaccuracies, we have to round out here
            outBounds->roundOut(outBounds);
        }
    }

    // position + local coord
    static const size_t kColorTextVASize = sizeof(SkPoint) + sizeof(SkIPoint16);
    static const size_t kColorTextPerspectiveVASize = sizeof(SkPoint3) + sizeof(SkIPoint16);
    static const size_t kGrayTextVASize = sizeof(SkPoint) + sizeof(GrColor) + sizeof(SkIPoint16);
    static const size_t kGrayTextDFPerspectiveVASize =
            sizeof(SkPoint3) + sizeof(GrColor) + sizeof(SkIPoint16);
    static const size_t kLCDTextVASize = kGrayTextVASize;
    static const size_t kMaxVASize = kGrayTextDFPerspectiveVASize;
    static const int kVerticesPerGlyph = 4;

    static void AssertEqual(const GrTextBlob&, const GrTextBlob&);

    // The color here is the GrPaint color, and it is used to determine whether we
    // have to regenerate LCD text blobs.
    // We use this color vs the SkPaint color because it has the colorfilter applied.
    void initReusableBlob(SkColor luminanceColor, const SkMatrix& viewMatrix,
                          SkScalar x, SkScalar y) {
        fLuminanceColor = luminanceColor;
        this->setupViewMatrix(viewMatrix, x, y);
    }

    void initThrowawayBlob(const SkMatrix& viewMatrix, SkScalar x, SkScalar y) {
        this->setupViewMatrix(viewMatrix, x, y);
    }

    const Key& key() const { return fKey; }

    size_t size() const { return fSize; }

    ~GrTextBlob() {
        for (int i = 0; i < fRunCount; i++) {
            fRuns[i].~Run();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Internal test methods
    std::unique_ptr<GrDrawOp> test_makeOp(int glyphCount, uint16_t run, uint16_t subRun,
                                          const SkMatrix& viewMatrix, SkScalar x, SkScalar y,
                                          const GrTextUtils::Paint&, const SkSurfaceProps&,
                                          const GrDistanceFieldAdjustTable*,
                                          GrTextUtils::Target*);

private:
    GrTextBlob()
        : fMaxMinScale(-SK_ScalarMax)
        , fMinMaxScale(SK_ScalarMax)
        , fTextType(0) {}


    // This function will only be called when we are generating a blob from scratch. We record the
    // initial view matrix and initial offsets(x,y), because we record vertex bounds relative to
    // these numbers.  When blobs are reused with new matrices, we need to return to model space so
    // we can update the vertex bounds appropriately.
    void setupViewMatrix(const SkMatrix& viewMatrix, SkScalar x, SkScalar y) {
        fInitialViewMatrix = viewMatrix;
        if (!viewMatrix.invert(&fInitialViewMatrixInverse)) {
            fInitialViewMatrixInverse = SkMatrix::I();
        }
        fInitialX = x;
        fInitialY = y;

        // make sure all initial subruns have the correct VM and X/Y applied
        for (int i = 0; i < fRunCount; i++) {
            fRuns[i].fSubRunInfo[0].init(fInitialViewMatrix, x, y);
        }
    }

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
     * Finally, for runs where the entire thing is too large for the GrTextContext to
     * handle, we have a bit to mark the run as flushable via rendering as paths or as scaled
     * glyphs. It would be a bit expensive to figure out ahead of time whether or not a run
     * can flush in this manner, so we always allocate vertices for the run, regardless of
     * whether or not it is too large.  The benefit of this strategy is that we can always reuse
     * a blob allocation regardless of viewmatrix changes.  We could store positions for these
     * glyphs, however, it's not clear if this is a win because we'd still have to either go to the
     * glyph cache to get the path at flush time, or hold onto the path in the cache, which
     * would greatly increase the memory of these cached items.
     */
    struct Run {
        Run() : fPaintFlags(0)
              , fInitialized(false) {
            // To ensure we always have one subrun, we push back a fresh run here
            fSubRunInfo.push_back();
        }
        struct SubRunInfo {
            SubRunInfo()
                    : fAtlasGeneration(GrDrawOpAtlas::kInvalidAtlasGeneration)
                    , fVertexStartIndex(0)
                    , fVertexEndIndex(0)
                    , fGlyphStartIndex(0)
                    , fGlyphEndIndex(0)
                    , fColor(GrColor_ILLEGAL)
                    , fMaskFormat(kA8_GrMaskFormat)
                    , fFlags(0) {
                fVertexBounds = SkRectPriv::MakeLargestInverted();
            }
            SubRunInfo(const SubRunInfo& that)
                : fBulkUseToken(that.fBulkUseToken)
                , fStrike(SkSafeRef(that.fStrike.get()))
                , fCurrentViewMatrix(that.fCurrentViewMatrix)
                , fVertexBounds(that.fVertexBounds)
                , fAtlasGeneration(that.fAtlasGeneration)
                , fVertexStartIndex(that.fVertexStartIndex)
                , fVertexEndIndex(that.fVertexEndIndex)
                , fGlyphStartIndex(that.fGlyphStartIndex)
                , fGlyphEndIndex(that.fGlyphEndIndex)
                , fX(that.fX)
                , fY(that.fY)
                , fColor(that.fColor)
                , fMaskFormat(that.fMaskFormat)
                , fFlags(that.fFlags) {
            }

            // TODO when this object is more internal, drop the privacy
            void resetBulkUseToken() { fBulkUseToken.reset(); }
            GrDrawOpAtlas::BulkUseTokenUpdater* bulkUseToken() { return &fBulkUseToken; }
            void setStrike(sk_sp<GrTextStrike> strike) { fStrike = std::move(strike); }
            GrTextStrike* strike() const { return fStrike.get(); }
            sk_sp<GrTextStrike> refStrike() const { return fStrike; }

            void setAtlasGeneration(uint64_t atlasGeneration) { fAtlasGeneration = atlasGeneration;}
            uint64_t atlasGeneration() const { return fAtlasGeneration; }

            size_t byteCount() const { return fVertexEndIndex - fVertexStartIndex; }
            size_t vertexStartIndex() const { return fVertexStartIndex; }
            size_t vertexEndIndex() const { return fVertexEndIndex; }
            void appendVertices(size_t vertexStride) {
                fVertexEndIndex += vertexStride * kVerticesPerGlyph;
            }

            uint32_t glyphCount() const { return fGlyphEndIndex - fGlyphStartIndex; }
            uint32_t glyphStartIndex() const { return fGlyphStartIndex; }
            uint32_t glyphEndIndex() const { return fGlyphEndIndex; }
            void glyphAppended() { fGlyphEndIndex++; }
            void setColor(GrColor color) { fColor = color; }
            GrColor color() const { return fColor; }
            void setMaskFormat(GrMaskFormat format) { fMaskFormat = format; }
            GrMaskFormat maskFormat() const { return fMaskFormat; }

            void setAsSuccessor(const SubRunInfo& prev) {
                fGlyphStartIndex = prev.glyphEndIndex();
                fGlyphEndIndex = prev.glyphEndIndex();

                fVertexStartIndex = prev.vertexEndIndex();
                fVertexEndIndex = prev.vertexEndIndex();

                // copy over viewmatrix settings
                this->init(prev.fCurrentViewMatrix, prev.fX, prev.fY);
            }

            const SkRect& vertexBounds() const { return fVertexBounds; }
            void joinGlyphBounds(const SkRect& glyphBounds) {
                fVertexBounds.joinNonEmptyArg(glyphBounds);
            }

            void init(const SkMatrix& viewMatrix, SkScalar x, SkScalar y) {
                fCurrentViewMatrix = viewMatrix;
                fX = x;
                fY = y;
            }

            // This function assumes the translation will be applied before it is called again
            void computeTranslation(const SkMatrix& viewMatrix, SkScalar x, SkScalar y,
                                    SkScalar* transX, SkScalar* transY);

            // df properties
            void setDrawAsDistanceFields() { fFlags |= kDrawAsSDF_Flag; }
            bool drawAsDistanceFields() const { return SkToBool(fFlags & kDrawAsSDF_Flag); }
            void setUseLCDText(bool useLCDText) {
                fFlags = useLCDText ? fFlags | kUseLCDText_Flag : fFlags & ~kUseLCDText_Flag;
            }
            bool hasUseLCDText() const { return SkToBool(fFlags & kUseLCDText_Flag); }
            void setAntiAliased(bool antiAliased) {
                fFlags = antiAliased ? fFlags | kAntiAliased_Flag : fFlags & ~kAntiAliased_Flag;
            }
            bool isAntiAliased() const { return SkToBool(fFlags & kAntiAliased_Flag); }
            void setHasWCoord(bool hasW) {
                fFlags  = hasW ? (fFlags | kHasWCoord_Flag) : fFlags & ~kHasWCoord_Flag;
            }
            bool hasWCoord() const { return SkToBool(fFlags & kHasWCoord_Flag); }
            void setNeedsTransform(bool needsTransform) {
                fFlags  = needsTransform ? (fFlags | kNeedsTransform_Flag)
                                          : fFlags & ~kNeedsTransform_Flag;
            }
            bool needsTransform() const { return SkToBool(fFlags & kNeedsTransform_Flag); }

        private:
            enum Flag {
                kDrawAsSDF_Flag = 0x01,
                kUseLCDText_Flag = 0x02,
                kAntiAliased_Flag = 0x04,
                kHasWCoord_Flag = 0x08,
                kNeedsTransform_Flag = 0x10
            };

            GrDrawOpAtlas::BulkUseTokenUpdater fBulkUseToken;
            sk_sp<GrTextStrike> fStrike;
            SkMatrix fCurrentViewMatrix;
            SkRect fVertexBounds;
            uint64_t fAtlasGeneration;
            size_t fVertexStartIndex;
            size_t fVertexEndIndex;
            uint32_t fGlyphStartIndex;
            uint32_t fGlyphEndIndex;
            SkScalar fX;
            SkScalar fY;
            GrColor fColor;
            GrMaskFormat fMaskFormat;
            uint32_t fFlags;
        };  // SubRunInfo

        SubRunInfo& push_back() {
            // Forward glyph / vertex information to seed the new sub run
            SubRunInfo& newSubRun = fSubRunInfo.push_back();
            const SubRunInfo& prevSubRun = fSubRunInfo.fromBack(1);

            newSubRun.setAsSuccessor(prevSubRun);
            return newSubRun;
        }
        static const int kMinSubRuns = 1;
        sk_sp<SkTypeface> fTypeface;
        SkSTArray<kMinSubRuns, SubRunInfo> fSubRunInfo;
        SkAutoDescriptor fDescriptor;

        // Effects from the paint that are used to build a SkScalerContext.
        sk_sp<SkPathEffect> fPathEffect;
        sk_sp<SkMaskFilter> fMaskFilter;

        // Distance field text cannot draw coloremoji, and so has to fall back.  However,
        // though the distance field text and the coloremoji may share the same run, they
        // will have different descriptors.  If fOverrideDescriptor is non-nullptr, then it
        // will be used in place of the run's descriptor to regen texture coords
        std::unique_ptr<SkAutoDescriptor> fOverrideDescriptor; // df properties

        // Any glyphs that can't be rendered with the base or override descriptor
        // are rendered as paths
        struct PathGlyph {
            PathGlyph(const SkPath& path, SkScalar x, SkScalar y, SkScalar scale, bool preXformed)
                : fPath(path)
                , fX(x)
                , fY(y)
                , fScale(scale)
                , fPreTransformed(preXformed) {}
            SkPath fPath;
            SkScalar fX;
            SkScalar fY;
            SkScalar fScale;
            bool fPreTransformed;
        };

        SkTArray<PathGlyph> fPathGlyphs;

        struct {
            unsigned fPaintFlags : 16;   // needed mainly for rendering paths
            bool fInitialized : 1;
        };
        // the only flags we need to set
        static constexpr auto kPaintFlagsMask = SkPaint::kAntiAlias_Flag;
    };  // Run

    inline std::unique_ptr<GrAtlasTextOp> makeOp(
            const Run::SubRunInfo& info, int glyphCount, uint16_t run, uint16_t subRun,
            const SkMatrix& viewMatrix, SkScalar x, SkScalar y, const SkIRect& clipRect,
            const GrTextUtils::Paint&, const SkSurfaceProps&,
            const GrDistanceFieldAdjustTable*, GrTextUtils::Target*);

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
    char* fVertices;
    GrGlyph** fGlyphs;
    Run* fRuns;
    SkMaskFilterBase::BlurRec fBlurRec;
    StrokeInfo fStrokeInfo;
    Key fKey;
    SkMatrix fInitialViewMatrix;
    SkMatrix fInitialViewMatrixInverse;
    size_t fSize;
    SkColor fLuminanceColor;
    SkScalar fInitialX;
    SkScalar fInitialY;

    // We can reuse distance field text, but only if the new viewmatrix would not result in
    // a mip change.  Because there can be multiple runs in a blob, we track the overall
    // maximum minimum scale, and minimum maximum scale, we can support before we need to regen
    SkScalar fMaxMinScale;
    SkScalar fMinMaxScale;
    int fRunCount;
    uint8_t fTextType;
};

/**
 * Used to produce vertices for a subrun of a blob. The vertices are cached in the blob itself.
 * This is invoked each time a sub run is drawn. It regenerates the vertex data as required either
 * because of changes to the atlas or because of different draw parameters (e.g. color change). In
 * rare cases the draw may have to interrupted and flushed in the middle of the sub run in order to
 * free up atlas space. Thus, this generator is stateful and should be invoked in a loop until the
 * entire sub run has been completed.
 */
class GrTextBlob::VertexRegenerator {
public:
    /**
     * Consecutive VertexRegenerators often use the same SkGlyphCache. If the same instance of
     * SkAutoGlyphCache is reused then it can save the cost of multiple detach/attach operations of
     * SkGlyphCache.
     */
    VertexRegenerator(GrResourceProvider*, GrTextBlob*, int runIdx, int subRunIdx,
                      const SkMatrix& viewMatrix, SkScalar x, SkScalar y, GrColor color,
                      GrDeferredUploadTarget*, GrGlyphCache*, GrAtlasManager*,
                      SkExclusiveStrikePtr*);

    struct Result {
        /**
         * Was regenerate() able to draw all the glyphs from the sub run? If not flush all glyph
         * draws and call regenerate() again.
         */
        bool fFinished = true;

        /**
         * How many glyphs were regenerated. Will be equal to the sub run's glyph count if
         * fType is kFinished.
         */
        int fGlyphsRegenerated = 0;

        /**
         * Pointer where the caller finds the first regenerated vertex.
         */
        const char* fFirstVertex;
    };

    bool regenerate(Result*);

private:
    template <bool regenPos, bool regenCol, bool regenTexCoords, bool regenGlyphs>
    bool doRegen(Result*);

    GrResourceProvider* fResourceProvider;
    const SkMatrix& fViewMatrix;
    GrTextBlob* fBlob;
    GrDeferredUploadTarget* fUploadTarget;
    GrGlyphCache* fGlyphCache;
    GrAtlasManager* fFullAtlasManager;
    SkExclusiveStrikePtr* fLazyCache;
    Run* fRun;
    Run::SubRunInfo* fSubRun;
    GrColor fColor;
    SkScalar fTransX;
    SkScalar fTransY;

    uint32_t fRegenFlags = 0;
    int fCurrGlyph = 0;
    bool fBrokenRun = false;
};

#endif  // GrTextBlob_DEFINED
