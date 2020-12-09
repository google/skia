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
// * initial Matrix - describes the combined initial matrix and origin the GrTextBlob was created
//   with.
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
        SkScalar fFrameWidth;
        SkScalar fMiterLimit;
        SkPaint::Join fJoin;
        SkPixelGeometry fPixelGeometry;
        bool fHasBlur;
        SkMaskFilterBase::BlurRec fBlurRec;
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

    static const Key& GetKey(const GrTextBlob& blob);
    static uint32_t Hash(const Key& key);

    void addKey(const Key& key);
    bool hasPerspective() const;
    const SkMatrix& initialMatrix() const { return fInitialMatrix; }

    void setMinAndMaxScale(SkScalar scaledMin, SkScalar scaledMax);
    std::tuple<SkScalar, SkScalar> scaleBounds() const {
        return {fMaxMinScale, fMinMaxScale};
    }

    bool canReuse(const SkPaint& paint, const SkMatrix& drawMatrix);

    const Key& key() const;
    size_t size() const;

    template<typename AddSingleMaskFormat>
    void addMultiMaskFormat(
            AddSingleMaskFormat addSingle,
            const SkZip<SkGlyphVariant, SkPoint>& drawables,
            const SkStrikeSpec& strikeSpec);

    const SkTInternalLList<GrSubRun>& subRunList() const { return fSubRunList; }

private:
    GrTextBlob(size_t allocSize, const SkMatrix& drawMatrix, SkColor initialLuminance);

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

    // The initial view matrix combined with the initial origin. Used to determine if a cached
    // subRun can be used in this draw situation.
    const SkMatrix fInitialMatrix;

    const SkColor fInitialLuminance;

    Key fKey;

    // We can reuse distance field text, but only if the new view matrix would not result in
    // a mip change.  Because there can be multiple runs in a blob, we track the overall
    // maximum minimum scale, and minimum maximum scale, we can support before we need to regen
    SkScalar fMaxMinScale{-SK_ScalarMax};
    SkScalar fMinMaxScale{SK_ScalarMax};

    bool fSomeGlyphsExcluded{false};
    SkTInternalLList<GrSubRun> fSubRunList;
    SkArenaAlloc fAlloc;
};

// -- GrAtlasSubRun --------------------------------------------------------------------------------
// GrAtlasSubRun is the API that GrAtlasTextOp uses to generate vertex data for drawing.
//     There are three different ways GrAtlasSubRun is specialized.
//      * DirectMaskSubRun - this is by far the most common type of subrun. The mask pixels are
//        in 1:1 correspondence with the pixels on the device. The destination rectangles in this
//        subrun are in device space. This subrun handles color glyphs.
//      * TransformedMaskSubRun - handles glyph where the image in the atlas needs to be
//        transformed to the screen. It is usually used for large color glyph which can't be
//        drawn with paths or scaled distance fields. The destination rectangles are in source
//        space.
//      * SDFTSubRun - scaled distance field text handles largish single color glyphs that still
//        can fit in the atlas; the sizes between direct subruns, and path subruns. The destination
class GrAtlasSubRun  {
public:
    static constexpr int kVerticesPerGlyph = 4;

    virtual ~GrAtlasSubRun() = default;

    virtual size_t vertexStride(const SkMatrix& drawMatrix) const = 0;
    virtual int glyphCount() const = 0;

    virtual std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(const GrClip* clip,
                    const SkMatrixProvider& viewMatrix,
                    const SkGlyphRunList& glyphRunList,
                    GrSurfaceDrawContext* rtc) const = 0;
    virtual void fillVertexData(
            void* vertexDst, int offset, int count,
            GrColor color, const SkMatrix& positionMatrix,
            SkIRect clip) const = 0;

    virtual void testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache* cache) = 0;

    // This call is not thread safe. It should only be called from GrDrawOp::onPrepare which
    // is single threaded.
    virtual std::tuple<bool, int> regenerateAtlas(
            int begin, int end, GrMeshDrawOp::Target* target) const = 0;
};

// -- GrSubRun -------------------------------------------------------------------------------------
// GrSubRun is the API the GrTextBlob uses for the subruns.
// There are several types of subrun, which can be broken into five classes:
//   * PathSubRun - handle very large single color glyphs using paths to render the glyph.
//   * DirectMaskSubRun - handle the majority of the glyphs where the cache entry's pixels are in
//     1:1 correspondence to the device pixels.
//   * TransformedMaskSubRun - handle large bitmap/argb glyphs that need to be scaled to the screen.
//   * SDFTSubRun - use signed distance fields to draw largish glyphs to the screen.
//   * GrAtlasSubRun - this is an abstract class used for atlas drawing.
class GrSubRun {
public:
    virtual ~GrSubRun() = default;

    // Produce GPU ops for this subRun.
    virtual void draw(const GrClip* clip,
                      const SkMatrixProvider& viewMatrix,
                      const SkGlyphRunList& glyphRunList,
                      GrSurfaceDrawContext* rtc) const = 0;

    // Given an already cached subRun, can this subRun handle this combination paint, matrix, and
    // position.
    virtual bool canReuse(const SkPaint& paint, const SkMatrix& drawMatrix) = 0;

    // Return the underlying atlas subrun if it exists. Otherwise, return nullptr.
    // * Don't use this API. It is only to support testing.
    virtual GrAtlasSubRun* testingOnly_atlasSubRun() = 0;

private:
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(GrSubRun);
};
#endif  // GrTextBlob_DEFINED
