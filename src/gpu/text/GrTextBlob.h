/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextBlob_DEFINED
#define GrTextBlob_DEFINED

#include <limits>

#include "include/core/SkPoint3.h"
#include "include/core/SkRefCnt.h"
#include "src/core/SkGlyphRunPainter.h"
#include "src/core/SkIPoint16.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkOpts.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTLazy.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrDrawOpAtlas.h"

class GrAtlasManager;
class GrAtlasTextOp;
class GrDeferredUploadTarget;
class GrGlyph;
class GrStrikeCache;
class GrTextTarget;

class SkMatrixProvider;
class SkSurfaceProps;
class SkTextBlob;
class SkTextBlobRunIterator;


// A GrTextBlob contains a fully processed SkTextBlob, suitable for nearly immediate drawing
// on the GPU.  These are initially created with valid positions and colors, but invalid
// texture coordinates.
//
// A GrTextBlob contains a number of SubRuns that are created in the blob's arena. Each SubRun
// tracks its own GrGlyph* and vertex data. The memory is organized in the arena in the following
// way so that the pointers for the GrGlyph* and vertex data are known before creating the SubRun.
//
//  GrGlyph*... | vertexData... | SubRun | GrGlyph*... | vertexData... | SubRun  etc.
//
// In these classes, I'm trying to follow the convention about matrices and origins.
// * draw Matrix|Origin    - describes the current draw command.
// * initial Matrix|Origin - describes the matrix and origin the GrTextBlob was created with.
// * current Matrix|Origin - describes the matrix and origin that are currently in the SubRun's
//                           vertex data.
//
// When handling repeated drawing using the same GrTextBlob initial data are compared to drawing
// data to see if this blob can service this drawing. If it can, but small changes are needed to
// the vertex data, the current data of the SubRuns is adjusted to conform to the drawing data
// from the op using the VertexRegenerator.
//
class GrTextBlob final : public SkNVRefCnt<GrTextBlob>, public SkGlyphRunPainterInterface {
public:
    class SubRun;
    class VertexRegenerator;

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

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(GrTextBlob);

    // Change memory management to handle the data after GrTextBlob, but in the same allocation
    // of memory. Only allow placement new.
    void operator delete(void* p);
    void* operator new(size_t);
    void* operator new(size_t, void* p);

    ~GrTextBlob() override;

    // Make an empty GrTextBlob, with all the invariants set to make the right decisions when
    // adding SubRuns.
    static sk_sp<GrTextBlob> Make(const SkGlyphRunList& glyphRunList,
                                  const SkMatrix& drawMatrix);

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

    bool canReuse(const SkPaint& paint, const SkMaskFilterBase::BlurRec& blurRec,
                  const SkMatrix& drawMatrix, SkPoint drawOrigin);

    void insertOpsIntoTarget(GrTextTarget* target,
                             const SkSurfaceProps& props,
                             const SkPaint& paint,
                             const GrClip* clip,
                             const SkMatrixProvider& deviceMatrix,
                             SkPoint drawOrigin);

    static const int kVerticesPerGlyph = 4;

    const Key& key() const;
    size_t size() const;

    template<typename AddSingleMaskFormat>
    void addMultiMaskFormat(
            AddSingleMaskFormat addSingle,
            const SkZip<SkGlyphVariant, SkPoint>& drawables,
            const SkStrikeSpec& strikeSpec);

    SubRun* firstSubRun() const;

    bool forceWForDistanceFields() const;

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

    GrTextBlob(size_t allocSize,
               const SkMatrix& drawMatrix,
               SkPoint origin,
               SkColor initialLuminance);

    void insertSubRun(SubRun* subRun);

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

    // The initial view matrix. This is used for moving additional draws of this
    // same text blob. We record the initial view matrix and initial offsets(x,y), because we
    // record vertex bounds relative to these numbers.  When blobs are reused with new matrices,
    // we need to return to source space so we can update the vertex bounds appropriately.
    const SkMatrix fInitialMatrix;

    // Initial position of this blob. Used for calculating position differences when reusing this
    // blob.
    const SkPoint fInitialOrigin;

    const SkColor fInitialLuminance;

    SkMaskFilterBase::BlurRec fBlurRec;
    StrokeInfo fStrokeInfo;
    Key fKey;

    // We can reuse distance field text, but only if the new view matrix would not result in
    // a mip change.  Because there can be multiple runs in a blob, we track the overall
    // maximum minimum scale, and minimum maximum scale, we can support before we need to regen
    SkScalar fMaxMinScale{-SK_ScalarMax};
    SkScalar fMinMaxScale{SK_ScalarMax};

    uint8_t fTextType{0};

    SubRun* fFirstSubRun{nullptr};
    SubRun* fLastSubRun{nullptr};
    SkArenaAlloc fAlloc;
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
    VertexRegenerator(GrResourceProvider*, GrTextBlob::SubRun* subRun,
                      GrDeferredUploadTarget*, GrAtlasManager*);

    // Return {success, number of glyphs regenerated}
    std::tuple<bool, int> regenerate(int begin, int end);

private:
    // Return {success, number of glyphs regenerated}
    std::tuple<bool, int> updateTextureCoordinates(int begin, int end);

    GrResourceProvider* fResourceProvider;
    GrDeferredUploadTarget* fUploadTarget;
    GrAtlasManager* fFullAtlasManager;
    SubRun* fSubRun;
};

// -- GrTextBlob::SubRun ---------------------------------------------------------------------------
// Hold data to draw the different types of sub run. SubRuns are produced knowing all the
// glyphs that are included in them.
class GrTextBlob::SubRun {
    enum SubRunType {
        kDirectMask,
        kTransformedMask,
        kTransformedPath,
        kTransformedSDFT
    };

public:
    struct VertexData {
        union {
            // Initially, filled with packed id, but changed to GrGlyph* in the onPrepare stage.
            SkPackedGlyphID packedGlyphID;
            GrGlyph* grGlyph;
        } glyph;
        const SkPoint pos;
        // The rectangle of the glyphs in strike space. But, for kDirectMask this also implies a
        // device space rect.
        GrIRect16 rect;
    };

    // SubRun for masks
    SubRun(SubRunType type,
           GrTextBlob* textBlob,
           const SkStrikeSpec& strikeSpec,
           GrMaskFormat format,
           SkRect vertexBounds,
           const SkSpan<VertexData>& vertexData);

    // SubRun for paths
    SubRun(GrTextBlob* textBlob, const SkStrikeSpec& strikeSpec);

    // TODO when this object is more internal, drop the privacy
    void resetBulkUseToken();
    GrDrawOpAtlas::BulkUseTokenUpdater* bulkUseToken();

    GrMaskFormat maskFormat() const;

    size_t vertexStride() const;
    size_t quadOffset(size_t index) const;
    void fillVertexData(
            void* vertexDst, int offset, int count,
            GrColor color, const SkMatrix& drawMatrix, SkPoint drawOrigin,
            SkIRect clip) const;

    int glyphCount() const;

    bool drawAsDistanceFields() const;
    bool needsTransform() const;
    bool needsPadding() const;
    int atlasPadding() const;
    SkSpan<const VertexData> vertexData() const;


    // Acquire a GrTextStrike and convert the SkPackedGlyphIDs to GrGlyphs for this run
    void prepareGrGlyphs(GrStrikeCache*);
    // has 'prepareGrGlyphs' been called (i.e., can the GrGlyphs be accessed) ?
    SkDEBUGCODE(bool isPrepared() const { return SkToBool(fStrike); })

    // The rectangle that surrounds all the glyph bounding boxes in device space.
    SkRect deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const;

    GrGlyph* grGlyph(int i) const;

    // df properties
    bool hasUseLCDText() const;
    bool isAntiAliased() const;

    const SkStrikeSpec& strikeSpec() const;

    static SubRun* MakePaths(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                             const SkFont& runFont,
                             const SkStrikeSpec& strikeSpec,
                             GrTextBlob* blob,
                             SkArenaAlloc* alloc);
    static SubRun* MakeSDFT(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                            const SkFont& runFont,
                            const SkStrikeSpec& strikeSpec,
                            GrTextBlob* blob,
                            SkArenaAlloc* alloc);
    static SubRun* MakeDirectMask(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                  const SkStrikeSpec& strikeSpec,
                                  GrMaskFormat format,
                                  GrTextBlob* blob,
                                  SkArenaAlloc* alloc);
    static SubRun* MakeTransformedMask(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                       const SkStrikeSpec& strikeSpec,
                                       GrMaskFormat format,
                                       GrTextBlob* blob,
                                       SkArenaAlloc* alloc);

    void insertSubRunOpsIntoTarget(GrTextTarget* target,
                                   const SkSurfaceProps& props,
                                   const SkPaint& paint,
                                   const GrClip* clip,
                                   const SkMatrixProvider& deviceMatrix,
                                   SkPoint drawOrigin);

    std::unique_ptr<GrAtlasTextOp> makeOp(const SkMatrixProvider& matrixProvider,
                                          SkPoint drawOrigin,
                                          const SkIRect& clipRect,
                                          const SkPaint& paint,
                                          const SkSurfaceProps&,
                                          GrTextTarget*);

    SubRun* fNextSubRun{nullptr};
    GrTextBlob* fBlob;
    uint64_t fAtlasGeneration{GrDrawOpAtlas::kInvalidAtlasGeneration};

private:
    struct AtlasPt {
        uint16_t u;
        uint16_t v;
    };

    // Normal text mask, SDFT, or color.
    struct Mask2DVertex {
        SkPoint devicePos;
        GrColor color;
        AtlasPt atlasPos;
    };
    struct ARGB2DVertex {
        ARGB2DVertex(SkPoint d, GrColor, AtlasPt a) : devicePos{d}, atlasPos{a} {}
        SkPoint devicePos;
        AtlasPt atlasPos;
    };

    // Perspective SDFT or SDFT forced to 3D or perspective color.
    struct Mask3DVertex {
        SkPoint3 devicePos;
        GrColor color;
        AtlasPt atlasPos;
    };
    struct ARGB3DVertex {
        ARGB3DVertex(SkPoint3 d, GrColor, AtlasPt a) : devicePos{d}, atlasPos{a} {}
        SkPoint3 devicePos;
        AtlasPt atlasPos;
    };

    static SubRun* InitForAtlas(SubRunType type,
                                const SkZip<SkGlyphVariant, SkPoint>& drawables,
                                const SkStrikeSpec& strikeSpec,
                                GrMaskFormat format,
                                GrTextBlob* blob,
                                SkArenaAlloc* alloc);
    bool hasW() const;
    void setUseLCDText(bool useLCDText);
    void setAntiAliased(bool antiAliased);
    bool drawAsPaths() const;

    const SubRunType fType;
    const GrMaskFormat fMaskFormat;
    bool fUseLCDText{false};
    bool fAntiAliased{false};

    const SkStrikeSpec fStrikeSpec;
    sk_sp<GrTextStrike> fStrike;

    GrDrawOpAtlas::BulkUseTokenUpdater fBulkUseToken;
    // The vertex bounds in device space if needsTransform() is false, otherwise the bounds in
    // source space. The bounds are the joined rectangles of all the glyphs.
    const SkRect fVertexBounds;
    const SkSpan<VertexData> fVertexData;
    std::vector<PathGlyph> fPaths;
};  // SubRun

#endif  // GrTextBlob_DEFINED
