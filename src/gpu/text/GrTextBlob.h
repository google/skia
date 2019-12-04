/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextBlob_DEFINED
#define GrTextBlob_DEFINED

#include "include/core/SkPathEffect.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkSurfaceProps.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkOpts.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTInternalLList.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrDrawOpAtlas.h"
#include "src/gpu/text/GrStrikeCache.h"
#include "src/gpu/text/GrTextContext.h"
#include "src/gpu/text/GrTextTarget.h"

class GrAtlasManager;
class GrAtlasTextOp;
struct GrDistanceFieldAdjustTable;
struct GrGlyph;

class SkTextBlob;
class SkTextBlobRunIterator;

// With this flag enabled, the GrTextContext will, as a sanity check, regenerate every blob
// that comes in to verify the integrity of its cache
// This is of dubious value, and maybe should be removed. I checked it on 11/21/2019, and many
// tests failed.
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
 * *WARNING* If you add new fields to this struct, then you may need to to update AssertEqual
 */
class GrTextBlob : public SkNVRefCnt<GrTextBlob>, public SkGlyphRunPainterInterface {
public:
    class SubRun;
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(GrTextBlob);
    using SubRunBufferSpec = std::tuple<uint32_t, uint32_t, size_t, size_t>;

    class VertexRegenerator;

    void generateFromGlyphRunList(const GrShaderCaps& shaderCaps,
                                  const GrTextContext::Options& options,
                                  const SkPaint& paint,
                                  const SkMatrix& viewMatrix,
                                  const SkSurfaceProps& props,
                                  const SkGlyphRunList& glyphRunList,
                                  SkGlyphRunListPainter* glyphPainter);

    // Make an empty GrTextBlob, with all the invariants set to make the right decisions when
    // adding SubRuns.
    static sk_sp<GrTextBlob> Make(int glyphCount,
                                  GrStrikeCache* strikeCache,
                                  const SkMatrix& viewMatrix,
                                  SkPoint origin,
                                  GrColor color,
                                  bool forceWForDistanceFields);

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
    }

    void* operator new(size_t, void* p) { return p; }

    bool hasDistanceField() const { return SkToBool(fTextType & kHasDistanceField_TextType); }
    bool hasBitmap() const { return SkToBool(fTextType & kHasBitmap_TextType); }
    void setHasDistanceField() { fTextType |= kHasDistanceField_TextType; }
    void setHasBitmap() { fTextType |= kHasBitmap_TextType; }

    void setMinAndMaxScale(SkScalar scaledMin, SkScalar scaledMax) {
        // we init fMaxMinScale and fMinMaxScale in the constructor
        fMaxMinScale = SkMaxScalar(scaledMin, fMaxMinScale);
        fMinMaxScale = SkMinScalar(scaledMax, fMinMaxScale);
    }

    bool hasPerspective() const {
        return fInitialViewMatrix.hasPerspective();
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

    void computeSubRunBounds(SkRect* outBounds, const SubRun& subRun,
                             const SkMatrix& viewMatrix, SkScalar x, SkScalar y,
                             bool needsGlyphTransform);

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

    // This function will only be called when we are generating a blob from scratch.
    // The color here is the GrPaint color, and it is used to determine whether we
    // have to regenerate LCD text blobs.
    // We use this color vs the SkPaint color because it has the color filter applied.
    void initReusableBlob(SkColor luminanceColor) {
        fLuminanceColor = luminanceColor;
    }

    const Key& key() const { return fKey; }

    size_t size() const { return fSize; }

    ~GrTextBlob() override { }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Internal test methods
    std::unique_ptr<GrDrawOp> test_makeOp(int glyphCount,
                                          const SkMatrix& viewMatrix, SkScalar x, SkScalar y,
                                          const SkPaint& paint, const SkPMColor4f& filteredColor,
                                          const SkSurfaceProps&, const GrDistanceFieldAdjustTable*,
                                          GrTextTarget*);

public:
    // Any glyphs that can't be rendered with the base or override descriptor
    // are rendered as paths
    struct PathGlyph {
        PathGlyph(const SkPath& path, SkPoint origin)
                : fPath(path)
                , fOrigin(origin) {}
        SkPath fPath;
        SkPoint fOrigin;
    };

    enum SubRunType {
        kDirectMask,
        kTransformedMask,
        kTransformedPath,
        kTransformedSDFT
    };

    bool hasW(SubRunType type) const {
        if (type == kTransformedSDFT) {
            return this->hasPerspective() || fForceWForDistanceFields;
        } else if (type == kTransformedMask || type == kTransformedPath) {
            return this->hasPerspective();
        }

        // The viewMatrix is implicitly SkMatrix::I when drawing kDirectMask, because it is not
        // used.
        return false;
    }

    // Hold data to draw the different types of sub run. SubRuns are produced knowing all the
    // glyphs that are included in them.
    class SubRun {
    public:
        // SubRun for masks
        SubRun(SubRunType type,
               GrTextBlob* textBlob,
               const SkStrikeSpec& strikeSpec,
               GrMaskFormat format,
               const SubRunBufferSpec& bufferSpec,
               sk_sp<GrTextStrike>&& grStrike);

        // SubRun for paths
        SubRun(GrTextBlob* textBlob, const SkStrikeSpec& strikeSpec);

        void appendGlyphs(const SkZip<SkGlyphVariant, SkPoint>& drawables);

        // TODO when this object is more internal, drop the privacy
        void resetBulkUseToken() { fBulkUseToken.reset(); }
        GrDrawOpAtlas::BulkUseTokenUpdater* bulkUseToken() { return &fBulkUseToken; }
        void setStrike(sk_sp<GrTextStrike> strike) { fStrike = std::move(strike); }
        GrTextStrike* strike() const { return fStrike.get(); }
        sk_sp<GrTextStrike> refStrike() const { return fStrike; }

        void setAtlasGeneration(uint64_t atlasGeneration) { fAtlasGeneration = atlasGeneration;}
        uint64_t atlasGeneration() const { return fAtlasGeneration; }

        size_t vertexStartIndex() const { return fVertexStartIndex; }
        uint32_t glyphCount() const { return fGlyphEndIndex - fGlyphStartIndex; }
        uint32_t glyphStartIndex() const { return fGlyphStartIndex; }

        void setColor(GrColor color) { fColor = color; }
        GrColor color() const { return fColor; }

        GrMaskFormat maskFormat() const { return fMaskFormat; }

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

        bool drawAsDistanceFields() const { return fType == kTransformedSDFT; }
        bool drawAsPaths() const { return fType == kTransformedPath; }
        bool needsTransform() const {
            return fType == kTransformedPath
            || fType == kTransformedMask
            || fType == kTransformedSDFT;
        }
        bool hasW() const {
            return fBlob->hasW(fType);
        }

        // df properties
        void setUseLCDText(bool useLCDText) { fFlags.useLCDText = useLCDText; }
        bool hasUseLCDText() const { return fFlags.useLCDText; }
        void setAntiAliased(bool antiAliased) { fFlags.antiAliased = antiAliased; }
        bool isAntiAliased() const { return fFlags.antiAliased; }

        const SkStrikeSpec& strikeSpec() const { return fStrikeSpec; }

        const SubRunType fType;
        GrTextBlob* const fBlob;
        const GrMaskFormat fMaskFormat;
        const uint32_t fGlyphStartIndex;
        const uint32_t fGlyphEndIndex;
        const size_t fVertexStartIndex;
        const size_t fVertexEndIndex;
        const SkStrikeSpec fStrikeSpec;
        sk_sp<GrTextStrike> fStrike;
        struct {
            bool useLCDText:1;
            bool antiAliased:1;
        } fFlags{false, false};
        GrColor fColor;
        GrDrawOpAtlas::BulkUseTokenUpdater fBulkUseToken;
        SkRect fVertexBounds = SkRectPriv::MakeLargestInverted();
        uint64_t fAtlasGeneration{GrDrawOpAtlas::kInvalidAtlasGeneration};
        SkScalar fX;
        SkScalar fY;
        SkMatrix fCurrentViewMatrix;
        std::vector<PathGlyph> fPaths;
    };  // SubRun

    SubRun* makeSubRun(SubRunType type,
                       const SkZip<SkGlyphVariant, SkPoint>& drawables,
                       const SkStrikeSpec& strikeSpec,
                       GrMaskFormat format);

    void addSingleMaskFormat(
            SubRunType type,
            const SkZip<SkGlyphVariant, SkPoint>& drawables,
            const SkStrikeSpec& strikeSpec,
            GrMaskFormat format);

    void addMultiMaskFormat(
            SubRunType type,
            const SkZip<SkGlyphVariant, SkPoint>& drawables,
            const SkStrikeSpec& strikeSpec);

    void addSDFT(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                 const SkStrikeSpec& strikeSpec,
                 const SkFont& runFont,
                 SkScalar minScale,
                 SkScalar maxScale);

private:
    GrTextBlob(size_t size,
               GrStrikeCache* strikeCache,
               const SkMatrix& viewMatrix,
               SkPoint origin,
               GrColor color,
               bool forceWForDistanceFields);

    struct StrokeInfo {
        SkScalar fFrameWidth;
        SkScalar fMiterLimit;
        SkPaint::Join fJoin;
    };

    enum TextType {
        kHasDistanceField_TextType = 0x1,
        kHasBitmap_TextType = 0x2,
    };

    std::unique_ptr<GrAtlasTextOp> makeOp(
            SubRun& info, int glyphCount,
            const SkMatrix& viewMatrix, SkScalar x, SkScalar y, const SkIRect& clipRect,
            const SkPaint& paint, const SkPMColor4f& filteredColor, const SkSurfaceProps&,
            const GrDistanceFieldAdjustTable*, GrTextTarget*);

    void processDeviceMasks(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                            const SkStrikeSpec& strikeSpec) override;

    void processSourcePaths(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                            const SkFont& runFont,
                            const SkStrikeSpec& strikeSpec) override;

    void processSourceSDFT(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                           const SkStrikeSpec& strikeSpec,
                           const SkFont& runFont,
                           SkScalar minScale,
                           SkScalar maxScale) override;

    void processSourceMasks(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                            const SkStrikeSpec& strikeSpec) override;

    // Overall size of this struct plus vertices and glyphs at the end.
    const size_t fSize;

    // Lifetime: The GrStrikeCache is owned by and has the same lifetime as the GrRecordingContext.
    // The GrRecordingContext also owns the GrTextBlob cache which owns this GrTextBlob.
    GrStrikeCache* const fStrikeCache;

    // The initial view matrix and its inverse. This is used for moving additional draws of this
    // same text blob. We record the initial view matrix and initial offsets(x,y), because we
    // record vertex bounds relative to these numbers.  When blobs are reused with new matrices,
    // we need to return to source space so we can update the vertex bounds appropriately.
    const SkMatrix fInitialViewMatrix;
    const SkMatrix fInitialViewMatrixInverse;

    // Initial position of this blob. Used for calculating position differences when reusing this
    // blob.
    const SkPoint fInitialOrigin;

    // From the distance field options to force distance fields to have a W coordinate.
    const bool fForceWForDistanceFields;

    // The color of the text to draw for solid colors.
    const GrColor fColor;

    // Pool of bytes for vertex data.
    char* fVertices;
    // How much (in bytes) of the vertex data is used while accumulating SubRuns.
    size_t fVerticesCursor{0};
    // Pointers to every glyph that will be drawn.
    GrGlyph** fGlyphs;
    // Number of glyphs stored in fGlyphs while accumulating SubRuns.
    uint32_t fGlyphsCursor{0};

    // Assume one run per text blob.
    SkSTArray<1, SubRun> fSubRuns;

    SkMaskFilterBase::BlurRec fBlurRec;
    StrokeInfo fStrokeInfo;
    Key fKey;
    SkColor fLuminanceColor;


    // We can reuse distance field text, but only if the new view matrix would not result in
    // a mip change.  Because there can be multiple runs in a blob, we track the overall
    // maximum minimum scale, and minimum maximum scale, we can support before we need to regen
    SkScalar fMaxMinScale{-SK_ScalarMax};
    SkScalar fMinMaxScale{SK_ScalarMax};

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
    VertexRegenerator(GrResourceProvider*, GrTextBlob*,
            GrTextBlob::SubRun* subRun,
                      const SkMatrix& viewMatrix, SkScalar x, SkScalar y, GrColor color,
                      GrDeferredUploadTarget*, GrStrikeCache*, GrAtlasManager*);

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
    GrStrikeCache* fGrStrikeCache;
    GrAtlasManager* fFullAtlasManager;
    SkTLazy<SkBulkGlyphMetricsAndImages> fMetricsAndImages;
    SubRun* fSubRun;
    GrColor fColor;
    SkScalar fTransX;
    SkScalar fTransY;

    uint32_t fRegenFlags = 0;
    int fCurrGlyph = 0;
    bool fBrokenRun = false;
};

#endif  // GrTextBlob_DEFINED
