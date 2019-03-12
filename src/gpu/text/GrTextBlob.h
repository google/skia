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
#include "GrStrikeCache.h"
#include "GrTextTarget.h"
#include "text/GrTextContext.h"
#include "SkDescriptor.h"
#include "SkMaskFilterBase.h"
#include "SkOpts.h"
#include "SkPathEffect.h"
#include "SkPoint3.h"
#include "SkRectPriv.h"
#include "SkStrikeCache.h"
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
class GrTextBlob : public SkNVRefCnt<GrTextBlob>, public SkGlyphRunPainterInterface {
    struct Run;
public:
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(GrTextBlob);

    class VertexRegenerator;

    void generateFromGlyphRunList(const GrShaderCaps& shaderCaps,
                                  const GrTextContext::Options& options,
                                  const SkPaint& paint,
                                  SkScalerContextFlags scalerContextFlags,
                                  const SkMatrix& viewMatrix,
                                  const SkSurfaceProps& props,
                                  const SkGlyphRunList& glyphRunList,
                                  SkGlyphRunListPainter* glyphPainter);

    static sk_sp<GrTextBlob> Make(
            int glyphCount,
            int runCount,
            GrColor color,
            GrStrikeCache* strikeCache);

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

    int runCountLimit() const { return fRunCountLimit; }

    Run* pushBackRun() {
        SkASSERT(fRunCount < fRunCountLimit);

        // If there is more run, then connect up the subruns.
        if (fRunCount > 0) {
            SubRun& newRun = fRuns[fRunCount].fSubRunInfo.back();
            SubRun& lastRun = fRuns[fRunCount - 1].fSubRunInfo.back();
            newRun.setAsSuccessor(lastRun);
        }

        fRunCount++;
        return this->currentRun();
    }

    void setMinAndMaxScale(SkScalar scaledMin, SkScalar scaledMax) {
        // we init fMaxMinScale and fMinMaxScale in the constructor
        fMaxMinScale = SkMaxScalar(scaledMin, fMaxMinScale);
        fMinMaxScale = SkMinScalar(scaledMax, fMinMaxScale);
    }

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

    bool mustRegenerate(const SkPaint&, bool, const SkMaskFilterBase::BlurRec& blurRec,
                        const SkMatrix& viewMatrix, SkScalar x, SkScalar y);

    void flush(GrTextTarget*, const SkSurfaceProps& props,
               const GrDistanceFieldAdjustTable* distanceAdjustTable,
               const SkPaint& paint, const SkPMColor4f& filteredColor, const GrClip& clip,
               const SkMatrix& viewMatrix, SkScalar x, SkScalar y);

    void computeSubRunBounds(SkRect* outBounds, int runIndex, int subRunIndex,
                             const SkMatrix& viewMatrix, SkScalar x, SkScalar y,
                             bool needsGlyphTransform) {
        // We don't yet position distance field text on the cpu, so we have to map the vertex bounds
        // into device space.
        // We handle vertex bounds differently for distance field text and bitmap text because
        // the vertex bounds of bitmap text are in device space.  If we are flushing multiple runs
        // from one blob then we are going to pay the price here of mapping the rect for each run.
        const Run& run = fRuns[runIndex];
        const SubRun& subRun = run.fSubRunInfo[subRunIndex];
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

    ~GrTextBlob() override {
        for (int i = 0; i < fRunCountLimit; i++) {
            fRuns[i].~Run();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Internal test methods
    std::unique_ptr<GrDrawOp> test_makeOp(int glyphCount, uint16_t run, uint16_t subRun,
                                          const SkMatrix& viewMatrix, SkScalar x, SkScalar y,
                                          const SkPaint& paint, const SkPMColor4f& filteredColor,
                                          const SkSurfaceProps&, const GrDistanceFieldAdjustTable*,
                                          GrTextTarget*);

private:
    GrTextBlob(GrStrikeCache* strikeCache) : fStrikeCache{strikeCache} { }

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
        for (int i = 0; i < fRunCountLimit; i++) {
            fRuns[i].fSubRunInfo[0].init(fInitialViewMatrix, x, y);
        }
    }

    class SubRun {
    public:
        SubRun(Run* run, const SkAutoDescriptor& desc, GrColor color)
            : fColor{color}
            , fRun{run}
            , fDesc{desc} {}

        // When used with emplace_back, this constructs a SubRun from the last SubRun in an array.
        //SubRun(SkSTArray<1, SubRun>* subRunList)
        //    : fColor{subRunList->fromBack(1).fColor} { }

        void appendGlyph(GrGlyph* glyph, SkRect dstRect);

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

        uint32_t glyphCount() const { return fGlyphEndIndex - fGlyphStartIndex; }
        uint32_t glyphStartIndex() const { return fGlyphStartIndex; }
        uint32_t glyphEndIndex() const { return fGlyphEndIndex; }
        void setColor(GrColor color) { fColor = color; }
        GrColor color() const { return fColor; }
        void setMaskFormat(GrMaskFormat format) { fMaskFormat = format; }
        GrMaskFormat maskFormat() const { return fMaskFormat; }

        void setAsSuccessor(const SubRun& prev) {
            fGlyphStartIndex = prev.glyphEndIndex();
            fGlyphEndIndex = fGlyphStartIndex;

            fVertexStartIndex = prev.vertexEndIndex();
            fVertexEndIndex = fVertexStartIndex;

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
        void setDrawAsDistanceFields() { fFlags.drawAsSdf = true; }
        bool drawAsDistanceFields() const { return fFlags.drawAsSdf; }
        void setUseLCDText(bool useLCDText) { fFlags.useLCDText = useLCDText; }
        bool hasUseLCDText() const { return fFlags.useLCDText; }
        void setAntiAliased(bool antiAliased) { fFlags.antiAliased = antiAliased; }
        bool isAntiAliased() const { return fFlags.antiAliased; }
        void setHasWCoord(bool hasW) { fFlags.hasWCoord = hasW; }
        bool hasWCoord() const { return fFlags.hasWCoord; }
        void setNeedsTransform(bool needsTransform) { fFlags.needsTransform = needsTransform; }
        bool needsTransform() const { return fFlags.needsTransform; }
        void setFallback() { fFlags.argbFallback = true; }
        bool isFallback() { return fFlags.argbFallback; }

        const SkDescriptor* desc() const { return fDesc.getDesc(); }

    private:
        GrDrawOpAtlas::BulkUseTokenUpdater fBulkUseToken;
        sk_sp<GrTextStrike> fStrike;
        SkMatrix fCurrentViewMatrix;
        SkRect fVertexBounds = SkRectPriv::MakeLargestInverted();
        uint64_t fAtlasGeneration{GrDrawOpAtlas::kInvalidAtlasGeneration};
        size_t fVertexStartIndex{0};
        size_t fVertexEndIndex{0};
        uint32_t fGlyphStartIndex{0};
        uint32_t fGlyphEndIndex{0};
        SkScalar fX;
        SkScalar fY;
        GrColor fColor{GrColor_ILLEGAL};
        GrMaskFormat fMaskFormat{kA8_GrMaskFormat};
        struct {
            bool drawAsSdf:1;
            bool useLCDText:1;
            bool antiAliased:1;
            bool hasWCoord:1;
            bool needsTransform:1;
            bool argbFallback:1;
        } fFlags{false, false, false, false, false, false};
        Run* const fRun;
        const SkAutoDescriptor& fDesc;
    };  // SubRunInfo

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
        explicit Run(GrTextBlob* blob, GrColor color)
        : fBlob{blob}, fColor{color} {
            // To ensure we always have one subrun, we push back a fresh run here
            fSubRunInfo.emplace_back(this, fDescriptor, color);
        }

        // sets the last subrun of runIndex to use w values
        void setSubRunHasW(bool hasWCoord) {
            SubRun& subRun = this->fSubRunInfo.back();
            subRun.setHasWCoord(hasWCoord);
        }

        // inits the override descriptor on the current run.  All following subruns must use this
        // descriptor
        SubRun* initARGBFallback() {
            fARGBFallbackDescriptor.reset(new SkAutoDescriptor{});
            // Push back a new subrun to fill and set the override descriptor
            SubRun* subRun = this->pushBackSubRun(*fARGBFallbackDescriptor, fColor);
            subRun->setMaskFormat(kARGB_GrMaskFormat);
            subRun->setFallback();
            return subRun;
        }

        // Appends a glyph to the blob as a path only.
        void appendPathGlyph(
                const SkPath& path, SkPoint position, SkScalar scale, bool preTransformed);

        // Append a glyph to the sub run taking care to switch the glyph if needed.
        void switchSubRunIfNeededAndAppendGlyph(GrGlyph* glyph,
                                                const sk_sp<GrTextStrike>& strike,
                                                const SkRect& destRect,
                                                bool needsTransform);

        // Used when the glyph in the cache has the CTM already applied, therefore no transform
        // is needed during rendering.
        void appendDeviceSpaceGlyph(const sk_sp<GrTextStrike>& strike,
                                    const SkGlyph& skGlyph,
                                    SkPoint origin);

        // The glyph is oriented upright in the cache and needs to be transformed onto the screen.
        void appendSourceSpaceGlyph(const sk_sp<GrTextStrike>& strike,
                                    const SkGlyph& skGlyph,
                                    SkPoint origin,
                                    SkScalar textScale);

        void setupFont(const SkStrikeSpec& strikeSpec);

        void setRunFontAntiAlias(bool aa) {
            fAntiAlias = aa;
        }

        // sets the last subrun of runIndex to use distance field text
        void setSubRunHasDistanceFields(bool hasLCD, bool isAntiAlias, bool hasWCoord) {
            SubRun& subRun = fSubRunInfo.back();
            subRun.setUseLCDText(hasLCD);
            subRun.setAntiAliased(isAntiAlias);
            subRun.setDrawAsDistanceFields();
            subRun.setHasWCoord(hasWCoord);
        }

        SubRun* pushBackSubRun(const SkAutoDescriptor& desc, GrColor color) {
            // Forward glyph / vertex information to seed the new sub run
            SubRun& newSubRun = fSubRunInfo.emplace_back(this, desc, color);

            const SubRun& prevSubRun = fSubRunInfo.fromBack(1);

            // Forward glyph / vertex information to seed the new sub run
            newSubRun.setAsSuccessor(prevSubRun);
            return &newSubRun;
        }

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


        sk_sp<SkTypeface> fTypeface;
        SkSTArray<1, SubRun> fSubRunInfo;
        SkAutoDescriptor fDescriptor;

        // Effects from the paint that are used to build a SkScalerContext.
        sk_sp<SkPathEffect> fPathEffect;
        sk_sp<SkMaskFilter> fMaskFilter;

        // Distance field text cannot draw coloremoji, and so has to fall back.  However,
        // though the distance field text and the coloremoji may share the same run, they
        // will have different descriptors.  If fARGBFallbackDescriptor is non-nullptr, then it
        // will be used in place of the run's descriptor to regen texture coords
        std::unique_ptr<SkAutoDescriptor> fARGBFallbackDescriptor;

        SkTArray<PathGlyph> fPathGlyphs;

        bool fAntiAlias{false};   // needed mainly for rendering paths
        bool fInitialized{false};

        GrTextBlob* const fBlob;
        GrColor fColor;
    };  // Run

    std::unique_ptr<GrAtlasTextOp> makeOp(
            const SubRun& info, int glyphCount, uint16_t run, uint16_t subRun,
            const SkMatrix& viewMatrix, SkScalar x, SkScalar y, const SkIRect& clipRect,
            const SkPaint& paint, const SkPMColor4f& filteredColor, const SkSurfaceProps&,
            const GrDistanceFieldAdjustTable*, GrTextTarget*);

    // currentRun, startRun, and the process* calls are all used by the SkGlyphRunPainter, and
    // live in SkGlyphRunPainter.cpp file.
    Run* currentRun();

    void startRun(const SkGlyphRun& glyphRun, bool useSDFT) override;

    void processDeviceMasks(SkSpan<const SkGlyphPos> masks,
                            SkStrikeInterface* strike) override;

    void processSourcePaths(SkSpan<const SkGlyphPos> paths,
                            SkStrikeInterface* strike, SkScalar cacheToSourceScale) override;

    void processDevicePaths(SkSpan<const SkGlyphPos> paths) override;

    void processSourceSDFT(SkSpan<const SkGlyphPos> masks,
                           SkStrikeInterface* strike,
                           const SkFont& runFont,
                           SkScalar cacheToSourceScale,
                           SkScalar minScale,
                           SkScalar maxScale,
                           bool hasWCoord) override;

    void processSourceFallback(SkSpan<const SkGlyphPos> masks,
                               SkStrikeInterface* strike,
                               SkScalar cacheToSourceScale,
                               bool hasW) override;

    void processDeviceFallback(SkSpan<const SkGlyphPos> masks,
                               SkStrikeInterface* strike) override;

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

    // Lifetime: The GrStrikeCache is owned by and has the same lifetime as the GrRecordingContext.
    // The GrRecordingContext also owns the GrTextBlob cache which owns this GrTextBlob.
    GrStrikeCache* const fStrikeCache;
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
    SkScalar fMaxMinScale{-SK_ScalarMax};
    SkScalar fMinMaxScale{SK_ScalarMax};
    int fRunCount{0};
    int fRunCountLimit;
    uint8_t fTextType{0};
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
                      GrDeferredUploadTarget*, GrStrikeCache*, GrAtlasManager*,
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
    bool doRegen(Result*, bool regenPos, bool regenCol, bool regenTexCoords, bool regenGlyphs);

    GrResourceProvider* fResourceProvider;
    const SkMatrix& fViewMatrix;
    GrTextBlob* fBlob;
    GrDeferredUploadTarget* fUploadTarget;
    GrStrikeCache* fGlyphCache;
    GrAtlasManager* fFullAtlasManager;
    SkExclusiveStrikePtr* fLazyCache;
    Run* fRun;
    SubRun* fSubRun;
    GrColor fColor;
    SkScalar fTransX;
    SkScalar fTransY;

    uint32_t fRegenFlags = 0;
    int fCurrGlyph = 0;
    bool fBrokenRun = false;
};

#endif  // GrTextBlob_DEFINED
