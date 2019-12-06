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
class GrTextBlob final : public SkNVRefCnt<GrTextBlob>, public SkGlyphRunPainterInterface {
public:
    class SubRun;
    class VertexRegenerator;
    using SubRunBufferSpec = std::tuple<uint32_t, uint32_t, size_t, size_t>;

    enum SubRunType {
        kDirectMask,
        kTransformedMask,
        kTransformedPath,
        kTransformedSDFT
    };

    struct Key {
        Key();
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

        bool operator==(const Key& other) const;
    };

    // Any glyphs that can't be rendered with the base or override descriptor
    // are rendered as paths
    struct PathGlyph {
        PathGlyph(const SkPath& path, SkPoint origin);
        SkPath fPath;
        SkPoint fOrigin;
    };

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
        void resetBulkUseToken();
        GrDrawOpAtlas::BulkUseTokenUpdater* bulkUseToken();
        void setStrike(sk_sp<GrTextStrike> strike);
        GrTextStrike* strike() const;
        sk_sp<GrTextStrike> refStrike() const;

        void setAtlasGeneration(uint64_t atlasGeneration);
        uint64_t atlasGeneration() const;

        size_t vertexStartIndex() const;
        uint32_t glyphCount() const;
        uint32_t glyphStartIndex() const;

        void setColor(GrColor color);
        GrColor color() const;

        GrMaskFormat maskFormat() const;

        const SkRect& vertexBounds() const;
        void joinGlyphBounds(const SkRect& glyphBounds);

        void init(const SkMatrix& viewMatrix, SkScalar x, SkScalar y);

        // This function assumes the translation will be applied before it is called again
        void computeTranslation(const SkMatrix& viewMatrix, SkScalar x, SkScalar y,
                                SkScalar* transX, SkScalar* transY);

        bool drawAsDistanceFields() const;
        bool drawAsPaths() const;
        bool needsTransform() const;
        bool hasW() const;

        // df properties
        void setUseLCDText(bool useLCDText);
        bool hasUseLCDText() const;
        void setAntiAliased(bool antiAliased);
        bool isAntiAliased() const;

        const SkStrikeSpec& strikeSpec() const;

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

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(GrTextBlob);

    // Change memory management to handle the data after GrTextBlob, but in the same allocation
    // of memory. Only allow placement new.
    void operator delete(void* p);
    void* operator new(size_t);
    void* operator new(size_t, void* p);

    ~GrTextBlob() override;

    // Make an empty GrTextBlob, with all the invariants set to make the right decisions when
    // adding SubRuns.
    static sk_sp<GrTextBlob> Make(int glyphCount,
                                  GrStrikeCache* strikeCache,
                                  const SkMatrix& viewMatrix,
                                  SkPoint origin,
                                  GrColor color,
                                  bool forceWForDistanceFields);

    void generateFromGlyphRunList(const GrShaderCaps& shaderCaps,
                                  const GrTextContext::Options& options,
                                  const SkPaint& paint,
                                  const SkMatrix& viewMatrix,
                                  const SkSurfaceProps& props,
                                  const SkGlyphRunList& glyphRunList,
                                  SkGlyphRunListPainter* glyphPainter);

    // Key manipulation functions
    void setupKey(const GrTextBlob::Key& key,
                  const SkMaskFilterBase::BlurRec& blurRec,
                  const SkPaint& paint);
    static const Key& GetKey(const GrTextBlob& blob);
    static uint32_t Hash(const Key& key);

    bool hasDistanceField() const;
    bool hasBitmap() const;
    bool hasPerspective() const;

    void setHasDistanceField();
    void setHasBitmap();
    void setMinAndMaxScale(SkScalar scaledMin, SkScalar scaledMax);

    static size_t GetVertexStride(GrMaskFormat maskFormat, bool hasWCoord);

    bool mustRegenerate(const SkPaint&, bool, const SkMaskFilterBase::BlurRec& blurRec,
                        const SkMatrix& viewMatrix, SkScalar x, SkScalar y);

    void flush(GrTextTarget*, const SkSurfaceProps& props,
               const GrDistanceFieldAdjustTable* distanceAdjustTable,
               const SkPaint& paint, const SkPMColor4f& filteredColor, const GrClip& clip,
               const SkMatrix& viewMatrix, SkScalar x, SkScalar y);

    void computeSubRunBounds(SkRect* outBounds, const SubRun& subRun,
                             const SkMatrix& viewMatrix, SkScalar x, SkScalar y,
                             bool needsGlyphTransform);

    // Normal text mask, SDFT, or color.
    struct Mask2DVertex {
        SkPoint devicePos;
        GrColor color;
        SkIPoint16 atlasPos;
    };
    struct ARGB2DVertex {
        SkPoint devicePos;
        SkIPoint16 atlasPos;
    };

    // Perspective SDFT or SDFT forced to 3D or perspective color.
    struct SDFT3DVertex {
        SkPoint3 devicePos;
        GrColor color;
        SkIPoint16 atlasPos;
    };
    struct ARGB3DVertex {
        SkPoint3 devicePos;
        SkIPoint16 atlasPos;
    };

    static const int kVerticesPerGlyph = 4;

    static void AssertEqual(const GrTextBlob&, const GrTextBlob&);

    // This function will only be called when we are generating a blob from scratch.
    // The color here is the GrPaint color, and it is used to determine whether we
    // have to regenerate LCD text blobs.
    // We use this color vs the SkPaint color because it has the color filter applied.
    void initReusableBlob(SkColor luminanceColor);

    const Key& key() const;
    size_t size() const;

    // Internal test methods
    std::unique_ptr<GrDrawOp> test_makeOp(int glyphCount,
                                          const SkMatrix& viewMatrix, SkScalar x, SkScalar y,
                                          const SkPaint& paint, const SkPMColor4f& filteredColor,
                                          const SkSurfaceProps&, const GrDistanceFieldAdjustTable*,
                                          GrTextTarget*);

    bool hasW(SubRunType type) const;

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
    enum TextType {
        kHasDistanceField_TextType = 0x1,
        kHasBitmap_TextType = 0x2,
    };

    struct StrokeInfo {
        SkScalar fFrameWidth;
        SkScalar fMiterLimit;
        SkPaint::Join fJoin;
    };

    GrTextBlob(size_t size,
               GrStrikeCache* strikeCache,
               const SkMatrix& viewMatrix,
               SkPoint origin,
               GrColor color,
               bool forceWForDistanceFields);

    std::unique_ptr<GrAtlasTextOp> makeOp(
            SubRun& info, int glyphCount,
            const SkMatrix& viewMatrix, SkScalar x, SkScalar y, const SkIRect& clipRect,
            const SkPaint& paint, const SkPMColor4f& filteredColor, const SkSurfaceProps&,
            const GrDistanceFieldAdjustTable*, GrTextTarget*);

    // Methods to satisfy SkGlyphRunPainterInterface
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
