/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextBlob_DEFINED
#define GrTextBlob_DEFINED

#include <algorithm>
#include <limits>

#include "include/core/SkPoint3.h"
#include "include/core/SkRefCnt.h"
#include "src/core/SkGlyphRunPainter.h"
#include "src/core/SkIPoint16.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkOpts.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTInternalLList.h"
#include "src/core/SkTLazy.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrSubRunAllocator.h"
#include "src/gpu/ops/GrOp.h"

class GrAtlasManager;
class GrAtlasTextOp;
class GrDeferredUploadTarget;
class GrGlyph;
class GrMeshDrawTarget;
class GrStrikeCache;
class GrSubRun;

class SkMatrixProvider;
class SkSurfaceProps;
class SkTextBlob;
class SkTextBlobRunIterator;

namespace skgpu { namespace v1 { class SurfaceDrawContext; }}

// -- GrAtlasSubRun --------------------------------------------------------------------------------
// GrAtlasSubRun is the API that GrAtlasTextOp uses to generate vertex data for drawing.
//     There are three different ways GrAtlasSubRun is specialized.
//      * DirectMaskSubRun - this is by far the most common type of SubRun. The mask pixels are
//        in 1:1 correspondence with the pixels on the device. The destination rectangles in this
//        SubRun are in device space. This SubRun handles color glyphs.
//      * TransformedMaskSubRun - handles glyph where the image in the atlas needs to be
//        transformed to the screen. It is usually used for large color glyph which can't be
//        drawn with paths or scaled distance fields. The destination rectangles are in source
//        space.
//      * SDFTSubRun - scaled distance field text handles largish single color glyphs that still
//        can fit in the atlas; the sizes between direct SubRun, and path SubRun. The destination

class GrAtlasSubRun;
using GrAtlasSubRunOwner = std::unique_ptr<GrAtlasSubRun, GrSubRunAllocator::Destroyer>;
class GrAtlasSubRun  {
public:
    static constexpr int kVerticesPerGlyph = 4;

    virtual ~GrAtlasSubRun() = default;

    virtual size_t vertexStride(const SkMatrix& drawMatrix) const = 0;
    virtual int glyphCount() const = 0;

#if SK_GPU_V1
    virtual std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(
            const GrClip*,
            const SkMatrixProvider& viewMatrix,
            const SkGlyphRunList&,
            const SkPaint&,
            skgpu::v1::SurfaceDrawContext*,
            GrAtlasSubRunOwner subRun) const = 0;
#endif

    virtual void fillVertexData(
            void* vertexDst, int offset, int count,
            GrColor color, const SkMatrix& positionMatrix,
            SkIRect clip) const = 0;

    virtual void testingOnly_packedGlyphIDToGrGlyph(GrStrikeCache* cache) = 0;

    // This call is not thread safe. It should only be called from GrDrawOp::onPrepare which
    // is single threaded.
    virtual std::tuple<bool, int> regenerateAtlas(
            int begin, int end, GrMeshDrawTarget* target) const = 0;
};

// -- GrSubRun -------------------------------------------------------------------------------------
// GrSubRun is the API the GrTextBlob uses for the SubRun.
// There are several types of SubRun, which can be broken into five classes:
//   * PathSubRun - handle very large single color glyphs using paths to render the glyph.
//   * DirectMaskSubRun - handle the majority of the glyphs where the cache entry's pixels are in
//     1:1 correspondence to the device pixels.
//   * TransformedMaskSubRun - handle large bitmap/argb glyphs that need to be scaled to the screen.
//   * SDFTSubRun - use signed distance fields to draw largish glyphs to the screen.
//   * GrAtlasSubRun - this is an abstract class used for atlas drawing.
class GrSubRun;
using GrSubRunOwner = std::unique_ptr<GrSubRun, GrSubRunAllocator::Destroyer>;
class GrSubRun {
public:
    virtual ~GrSubRun() = default;

#if SK_GPU_V1
    // Produce GPU ops for this subRun.
    virtual void draw(const GrClip*,
                      const SkMatrixProvider& viewMatrix,
                      const SkGlyphRunList&,
                      const SkPaint&,
                      skgpu::v1::SurfaceDrawContext*) const = 0;
#endif

    // Given an already cached subRun, can this subRun handle this combination paint, matrix, and
    // position.
    virtual bool canReuse(const SkPaint& paint, const SkMatrix& drawMatrix) const = 0;

    // Return the underlying atlas SubRun if it exists. Otherwise, return nullptr.
    // * Don't use this API. It is only to support testing.
    virtual GrAtlasSubRun* testingOnly_atlasSubRun() = 0;

    GrSubRunOwner fNext;
};

struct GrSubRunList {
    class Iterator {
    public:
        using value_type = GrSubRun;
        using difference_type = ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::input_iterator_tag;
        Iterator(GrSubRun* subRun) : fPtr{subRun} { }
        Iterator& operator++() { fPtr = fPtr->fNext.get(); return *this; }
        Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; }
        bool operator==(const Iterator& rhs) const { return fPtr == rhs.fPtr; }
        bool operator!=(const Iterator& rhs) const { return fPtr != rhs.fPtr; }
        reference operator*() { return *fPtr; }

    private:
        GrSubRun* fPtr;
    };

    void append(GrSubRunOwner subRun) {
        GrSubRunOwner* newTail = &subRun->fNext;
        *fTail = std::move(subRun);
        fTail = newTail;
    }
    bool isEmpty() const { return fHead == nullptr; }
    Iterator begin() { return Iterator{ fHead.get()}; }
    Iterator end() { return Iterator{nullptr}; }
    Iterator begin() const { return Iterator{ fHead.get()}; }
    Iterator end() const { return Iterator{nullptr}; }
    GrSubRun& front() const {return *fHead; }

    GrSubRunOwner fHead{nullptr};
    GrSubRunOwner* fTail{&fHead};
};

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

    // Key is not used as part of a hash map, so the hash is never taken. It's only used in a
    // list search using operator =().
    struct Key {
        static std::tuple<bool, Key> Make(const SkGlyphRunList& glyphRunList,
                                          const SkPaint& paint,
                                          const SkSurfaceProps& surfaceProps,
                                          const GrColorInfo& colorInfo,
                                          const SkMatrix& drawMatrix,
                                          const GrSDFTControl& control);
        uint32_t fUniqueID;
        // Color may affect the gamma of the mask we generate, but in a fairly limited way.
        // Each color is assigned to on of a fixed number of buckets based on its
        // luminance. For each luminance bucket there is a "canonical color" that
        // represents the bucket.  This functionality is currently only supported for A8
        SkColor fCanonicalColor;
        SkScalar fFrameWidth;
        SkScalar fMiterLimit;
        SkPixelGeometry fPixelGeometry;
        SkMaskFilterBase::BlurRec fBlurRec;
        uint32_t fScalerContextFlags;
        SkMatrix fDrawMatrix;
        // Below here fields are of size 1 byte.
        uint8_t fSetOfDrawingTypes;
        bool fHasBlur;
        SkPaint::Style fStyle;
        SkPaint::Join fJoin;

        bool operator==(const Key& other) const;
    };

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(GrTextBlob);

    // Make a GrTextBlob and its sub runs.
    static sk_sp<GrTextBlob> Make(const SkGlyphRunList& glyphRunList,
                                  const SkPaint& paint,
                                  const SkMatrix& drawMatrix,
                                  const GrSDFTControl& control,
                                  SkGlyphRunListPainter* painter);

    ~GrTextBlob() override;

    // Change memory management to handle the data after GrTextBlob, but in the same allocation
    // of memory. Only allow placement new.
    void operator delete(void* p);
    void* operator new(size_t);
    void* operator new(size_t, void* p);

    const Key& key() { return fKey; }

    void addKey(const Key& key);
    bool hasPerspective() const;
    const SkMatrix& initialMatrix() const { return fInitialMatrix; }

    std::tuple<SkScalar, SkScalar> scaleBounds() const {
        return {fMaxMinScale, fMinMaxScale};
    }

    bool canReuse(const SkPaint& paint, const SkMatrix& drawMatrix) const;

    const Key& key() const;
    size_t size() const;

    const GrSubRunList& subRunList() const {
        return fSubRunList;
    }

private:
    GrTextBlob(int allocSize, const SkMatrix& drawMatrix, SkColor initialLuminance);

    template<typename AddSingleMaskFormat>
    void addMultiMaskFormat(
            AddSingleMaskFormat addSingle,
            const SkZip<SkGlyphVariant, SkPoint>& drawables,
            const SkStrikeSpec& strikeSpec);

#if SK_GPU_V1
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
#endif // SK_GPU_V1

    // The allocator must come first because it needs to be destroyed last. Other fields of this
    // structure may have pointers into it.
    GrSubRunAllocator fAlloc;

    // Owner and list of the SubRun.
    GrSubRunList fSubRunList;

    // Overall size of this struct plus vertices and glyphs at the end.
    const int fSize;

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
};

#if SK_GPU_V1
class GrSubRunNoCachePainter : public SkGlyphRunPainterInterface {
public:
    GrSubRunNoCachePainter(skgpu::v1::SurfaceDrawContext*,
                           GrSubRunAllocator*,
                           const GrClip*,
                           const SkMatrixProvider& viewMatrix,
                           const SkGlyphRunList&,
                           const SkPaint&);
    void processDeviceMasks(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                            const SkStrikeSpec& strikeSpec) override;
    void processSourceMasks(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                            const SkStrikeSpec& strikeSpec) override;
    void processSourcePaths(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                            const SkFont& runFont, const SkStrikeSpec& strikeSpec) override;
    void processSourceSDFT(const SkZip<SkGlyphVariant, SkPoint>& drawables,
                           const SkStrikeSpec& strikeSpec, const SkFont& runFont,
                           SkScalar minScale, SkScalar maxScale) override;

private:

    // Draw passes ownership of the sub run to the op.
    void draw(GrAtlasSubRunOwner subRun);

    skgpu::v1::SurfaceDrawContext* const fSDC;
    GrSubRunAllocator* const fAlloc;
    const GrClip* const fClip;
    const SkMatrixProvider& fViewMatrix;
    const SkGlyphRunList& fGlyphRunList;
    const SkPaint& fPaint;
};
#endif // SK_GPU_V1

#endif  // GrTextBlob_DEFINED
