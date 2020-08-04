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
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/text/GrStrikeCache.h"

class GrAtlasManager;
class GrAtlasTextOp;
class GrDeferredUploadTarget;
class GrDrawOp;
class GrGlyph;
class GrStrikeCache;
class GrSubRun;

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
//
class GrTextBlob final : public SkNVRefCnt<GrTextBlob>, public SkGlyphRunPainterInterface {
public:
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

    const Key& key() const;
    size_t size() const;

    template<typename AddSingleMaskFormat>
    void addMultiMaskFormat(
            AddSingleMaskFormat addSingle,
            const SkZip<SkGlyphVariant, SkPoint>& drawables,
            const SkStrikeSpec& strikeSpec);

    const SkTInternalLList<GrSubRun>& subRunList() const { return fSubRunList; }

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

    void insertSubRun(GrSubRun* subRun);

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

    SkTInternalLList<GrSubRun> fSubRunList;
    SkArenaAlloc fAlloc;
};

// -- GrSubRun -------------------------------------------------------------------------------------
class GrSubRun {
public:
    virtual ~GrSubRun() = default;
    virtual void draw(const GrClip* clip,
                      const SkMatrixProvider& viewMatrix,
                      const SkGlyphRunList& glyphRunList,
                      GrRenderTargetContext* rtc) const = 0;

private:
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(GrSubRun);
};

// -- GrPathSubRun ---------------------------------------------------------------------------------
class GrPathSubRun final : public GrSubRun {
    struct PathGlyph;

public:
    GrPathSubRun(bool isAntiAliased, const SkStrikeSpec& strikeSpec, SkSpan<PathGlyph> paths);

    void draw(const GrClip* clip,
              const SkMatrixProvider& viewMatrix,
              const SkGlyphRunList& glyphRunList,
              GrRenderTargetContext* rtc) const override;

    static GrSubRun* Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                          bool isAntiAliased,
                          const SkStrikeSpec& strikeSpec,
                          SkArenaAlloc* alloc);

private:
    struct PathGlyph {
        PathGlyph(const SkPath& path, SkPoint origin);
        SkPath fPath;
        SkPoint fOrigin;
    };

    const bool fIsAntiAliased;
    const SkStrikeSpec fStrikeSpec;
    const SkSpan<const PathGlyph> fPaths;
};

// -- GrAtlasSubRun --------------------------------------------------------------------------------
class GrAtlasSubRun : public GrSubRun {
public:
    static constexpr int kVerticesPerGlyph = 4;
    virtual size_t vertexStride() const = 0;
    virtual int glyphCount() const = 0;

    virtual std::tuple<const GrClip*, std::unique_ptr<GrDrawOp>>
    makeAtlasTextOp(const GrClip* clip,
                    const SkMatrixProvider& viewMatrix,
                    const SkGlyphRunList& glyphRunList,
                    GrRenderTargetContext* rtc) const = 0;
    virtual void fillVertexData(
            void* vertexDst, int offset, int count,
            GrColor color, const SkMatrix& drawMatrix, SkPoint drawOrigin,
            SkIRect clip) const = 0;

    // This call is not thread safe. It should only be called from GrDrawOp::onPrepare which
    // is single threaded.
    virtual std::tuple<bool, int> regenerateAtlas(
            int begin, int end, GrMeshDrawOp::Target* target) const = 0;

protected:
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
};

// -- GrGlyphVector --------------------------------------------------------------------------------
class GrGlyphVector {
    union Variant {
        // Initially, filled with packed id, but changed to GrGlyph* in the onPrepare stage.
        SkPackedGlyphID packedGlyphID;
        GrGlyph* grGlyph;
    };

public:
    static GrGlyphVector Make(
            const SkStrikeSpec& spec, SkSpan<SkGlyphVariant> glyphs, SkArenaAlloc* alloc);
    SkSpan<const GrGlyph*> glyphs() const;

    SkScalar strikeToSourceRatio() const { return fStrikeSpec.strikeToSourceRatio(); }
    std::tuple<bool, int> regenerateAtlas(
            int begin, int end,
            GrMaskFormat maskFormat,
            int srcPadding,
            GrMeshDrawOp::Target *target,
            bool bilerpPadding = false);

    static size_t GlyphVectorSize(size_t count) {
        return sizeof(Variant) * count;
    }

private:
    GrGlyphVector(const SkStrikeSpec& spec, SkSpan<Variant> glyphs);

    const SkStrikeSpec fStrikeSpec;
    SkSpan<Variant> fGlyphs;
    sk_sp<GrTextStrike> fStrike{nullptr};
    uint64_t fAtlasGeneration{GrDrawOpAtlas::kInvalidAtlasGeneration};
    GrDrawOpAtlas::BulkUseTokenUpdater fBulkUseToken;
};

// -- GrDirectMaskSubRun ---------------------------------------------------------------------------
class GrDirectMaskSubRun final : public GrAtlasSubRun {
public:
    using VertexData =  SkPoint;  // The left top corner of the glyph bounding box.

    GrDirectMaskSubRun(GrMaskFormat format,
                       GrTextBlob* blob,
                       const SkRect& bounds,
                       SkSpan<const VertexData> vertexData,
                       GrGlyphVector glyphs);

    static GrSubRun* Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                          const SkStrikeSpec& strikeSpec,
                          GrMaskFormat format,
                          GrTextBlob* blob,
                          SkArenaAlloc* alloc);

    void draw(const GrClip* clip,
              const SkMatrixProvider& viewMatrix,
              const SkGlyphRunList& glyphRunList,
              GrRenderTargetContext* rtc) const override;

    size_t vertexStride() const override;

    int glyphCount() const override;

    std::tuple<const GrClip*, std::unique_ptr<GrDrawOp>>
    makeAtlasTextOp(const GrClip* clip,
                    const SkMatrixProvider& viewMatrix,
                    const SkGlyphRunList& glyphRunList,
                    GrRenderTargetContext* rtc) const override;

    std::tuple<bool, int>
    regenerateAtlas(int begin, int end, GrMeshDrawOp::Target* target) const override;

    void fillVertexData(void* vertexDst, int offset, int count, GrColor color,
                        const SkMatrix& drawMatrix, SkPoint drawOrigin,
                        SkIRect clip) const override;
private:
    // The rectangle that surrounds all the glyph bounding boxes in device space.
    SkRect deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const;

    const GrMaskFormat fMaskFormat;
    GrTextBlob* const fBlob;
    // The vertex bounds in device space. The bounds are the joined rectangles of all the glyphs.
    const SkRect fVertexBounds;
    const SkSpan<const VertexData> fVertexData;

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GrGlyphVector fGlyphs;
};

// -- GrTransformedMaskSubRun ----------------------------------------------------------------------
class GrTransformedMaskSubRun final : public GrAtlasSubRun {
public:
    struct VertexData {
        const SkPoint pos;
        // The rectangle of the glyphs in strike space. But, for kDirectMask this also implies a
        // device space rect.
        GrIRect16 rect;
    };

    // SubRun for masks
    GrTransformedMaskSubRun(GrMaskFormat format,
                            GrTextBlob* blob,
                            const SkRect& bounds,
                            SkSpan<const VertexData> vertexData,
                            GrGlyphVector glyphs);

    static GrSubRun* Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                          const SkStrikeSpec& strikeSpec,
                          GrMaskFormat format,
                          GrTextBlob* blob,
                          SkArenaAlloc* alloc);

    void draw(const GrClip* clip,
              const SkMatrixProvider& viewMatrix,
              const SkGlyphRunList& glyphRunList,
              GrRenderTargetContext* rtc) const override;

    std::tuple<const GrClip*, std::unique_ptr<GrDrawOp>>
    makeAtlasTextOp(const GrClip* clip,
                    const SkMatrixProvider& viewMatrix,
                    const SkGlyphRunList& glyphRunList,
                    GrRenderTargetContext* rtc) const override;

    std::tuple<bool, int> regenerateAtlas(
            int begin, int end, GrMeshDrawOp::Target* target) const override;

    void fillVertexData(
            void* vertexDst, int offset, int count,
            GrColor color, const SkMatrix& drawMatrix, SkPoint drawOrigin,
            SkIRect clip) const override;

    size_t vertexStride() const override;
    int glyphCount() const override;

private:
    bool hasW() const;
    // The rectangle that surrounds all the glyph bounding boxes in device space.
    SkRect deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const;

    const GrMaskFormat fMaskFormat;
    GrTextBlob* fBlob;

    // The bounds in source space. The bounds are the joined rectangles of all the glyphs.
    const SkRect fVertexBounds;
    const SkSpan<const VertexData> fVertexData;

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GrGlyphVector fGlyphs;
};

// -- GrSDFTSubRun ---------------------------------------------------------------------------------
// Hold data to draw Scaled Distance Field Text sub runs.
class GrSDFTSubRun final : public GrAtlasSubRun {
public:
    struct VertexData {
        const SkPoint pos;
        // The rectangle of the glyphs in strike space.
        GrIRect16 rect;
    };

    GrSDFTSubRun(GrMaskFormat format,
                 GrTextBlob* blob,
                 SkRect vertexBounds,
                 SkSpan<const VertexData> vertexData,
                 GrGlyphVector glyphs,
                 bool useLCDText,
                 bool antiAliased);

    static GrSubRun* Make(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                          const SkFont& runFont,
                          const SkStrikeSpec& strikeSpec,
                          GrTextBlob* blob,
                          SkArenaAlloc* alloc);

    void draw(const GrClip* clip,
              const SkMatrixProvider& viewMatrix,
              const SkGlyphRunList& glyphRunList,
              GrRenderTargetContext* rtc) const override;

    std::tuple<const GrClip*, std::unique_ptr<GrDrawOp>>
    makeAtlasTextOp(const GrClip* clip,
                    const SkMatrixProvider& viewMatrix,
                    const SkGlyphRunList& glyphRunList,
                    GrRenderTargetContext* rtc) const override;

    std::tuple<bool, int> regenerateAtlas(
            int begin, int end, GrMeshDrawOp::Target* target) const override;

    void fillVertexData(
            void* vertexDst, int offset, int count,
            GrColor color, const SkMatrix& drawMatrix, SkPoint drawOrigin,
            SkIRect clip) const override;

    size_t vertexStride() const override;
    int glyphCount() const override;

private:
    bool hasW() const;

    // The rectangle that surrounds all the glyph bounding boxes in device space.
    SkRect deviceRect(const SkMatrix& drawMatrix, SkPoint drawOrigin) const;

    const GrMaskFormat fMaskFormat;
    GrTextBlob* fBlob;

    // The bounds in source space. The bounds are the joined rectangles of all the glyphs.
    const SkRect fVertexBounds;
    const SkSpan<const VertexData> fVertexData;

    // The regenerateAtlas method mutates fGlyphs. It should be called from onPrepare which must
    // be single threaded.
    mutable GrGlyphVector fGlyphs;

    const bool fUseLCDText;
    const bool fAntiAliased;
};

#endif  // GrTextBlob_DEFINED
