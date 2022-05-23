/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sktext_gpu_TextBlob_DEFINED
#define sktext_gpu_TextBlob_DEFINED

#include <algorithm>
#include <limits>

#include "include/core/SkPoint3.h"
#include "include/core/SkRefCnt.h"
#include "include/private/chromium/Slug.h"
#include "src/core/SkDevice.h"
#include "src/core/SkGlyphRunPainter.h"
#include "src/core/SkIPoint16.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkOpts.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTInternalLList.h"
#include "src/core/SkTLazy.h"
#if SK_SUPPORT_GPU
#include "src/gpu/ganesh/GrColor.h"
#include "src/gpu/ganesh/ops/GrOp.h"
#endif
#include "src/text/gpu/SubRunAllocator.h"

#if SK_SUPPORT_GPU
class GrAtlasManager;
class GrDeferredUploadTarget;
class GrMeshDrawTarget;
#endif

class SkMatrixProvider;
class SkStrikeClient;
class SkSurfaceProps;
class SkTextBlob;
class SkTextBlobRunIterator;

namespace sktext::gpu {
class Glyph;
class StrikeCache;
}

namespace skgpu::v1 { class SurfaceDrawContext; }

namespace sktext::gpu {

// -- AtlasSubRun --------------------------------------------------------------------------------
// AtlasSubRun is the API that AtlasTextOp uses to generate vertex data for drawing.
//     There are three different ways AtlasSubRun is specialized.
//      * DirectMaskSubRun* - this is by far the most common type of SubRun. The mask pixels are
//        in 1:1 correspondence with the pixels on the device. The destination rectangles in this
//        SubRun are in device space. This SubRun handles color glyphs.
//      * TransformedMaskSubRun* - handles glyph where the image in the atlas needs to be
//        transformed to the screen. It is usually used for large color glyph which can't be
//        drawn with paths or scaled distance fields, but will be used to draw bitmap glyphs to
//        the screen, if the matrix does not map 1:1 to the screen. The destination rectangles
//        are in source space.
//      * SDFTSubRun* - scaled distance field text handles largish single color glyphs that still
//        can fit in the atlas; the sizes between direct SubRun, and path SubRun. The destination
//        rectangles are in source space.
class AtlasSubRun  {
public:
    virtual ~AtlasSubRun() = default;

    virtual int glyphCount() const = 0;

#if SK_SUPPORT_GPU
    virtual size_t vertexStride(const SkMatrix& drawMatrix) const = 0;

    virtual std::tuple<const GrClip*, GrOp::Owner>
    makeAtlasTextOp(
            const GrClip*,
            const SkMatrixProvider& viewMatrix,
            SkPoint drawOrigin,
            const SkPaint&,
            sk_sp<SkRefCnt>&& subRunStorage,
            skgpu::v1::SurfaceDrawContext*) const = 0;

    virtual void fillVertexData(
            void* vertexDst, int offset, int count,
            GrColor color,
            const SkMatrix& drawMatrix,
            SkPoint drawOrigin,
            SkIRect clip) const = 0;

    // This call is not thread safe. It should only be called from GrDrawOp::onPrepare which
    // is single threaded.
    virtual std::tuple<bool, int> regenerateAtlas(
            int begin, int end, GrMeshDrawTarget* target) const = 0;
#endif

    virtual void testingOnly_packedGlyphIDToGlyph(StrikeCache* cache) const = 0;
};

// -- SubRun -------------------------------------------------------------------------------------
// SubRun defines the most basic functionality of a SubRun; the ability to draw, and the
// ability to be in a list.
class SubRun;
using SubRunOwner = std::unique_ptr<SubRun, SubRunAllocator::Destroyer>;
class BlobSubRun;
class SubRun {
public:
    virtual ~SubRun();
#if SK_SUPPORT_GPU
    // Produce GPU ops for this subRun or just draw them.
    virtual void draw(SkCanvas*,
                      const GrClip*,
                      const SkMatrixProvider& viewMatrix,
                      SkPoint drawOrigin,
                      const SkPaint&,
                      sk_sp<SkRefCnt> subRunStorage,
                      skgpu::v1::SurfaceDrawContext*) const = 0;
#endif

    virtual const BlobSubRun* blobCast() const;
    void flatten(SkWriteBuffer& buffer) const;
    static SubRunOwner MakeFromBuffer(const SkMatrix& initialPositionMatrix,
                                      SkReadBuffer& buffer,
                                      sktext::gpu::SubRunAllocator* alloc,
                                      const SkStrikeClient* client);

    // Size hint for unflattening this run. If this is accurate, it will help with the allocation
    // of the slug. If it's off then there may be more allocations needed to unflatten.
    virtual int unflattenSize() const = 0;

    // Given an already cached subRun, can this subRun handle this combination paint, matrix, and
    // position.
    virtual bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const = 0;

    // Return the underlying atlas SubRun if it exists. Otherwise, return nullptr.
    // * Don't use this API. It is only to support testing.
    virtual const AtlasSubRun* testingOnly_atlasSubRun() const = 0;

protected:
    enum SubRunType : int;
    virtual SubRunType subRunType() const = 0;
    virtual void doFlatten(SkWriteBuffer& buffer) const = 0;

private:
    friend class SubRunList;
    SubRunOwner fNext;
};

// -- SubRunList ---------------------------------------------------------------------------------
class SubRunList {
public:
    class Iterator {
    public:
        using value_type = SubRun;
        using difference_type = ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::input_iterator_tag;
        Iterator(SubRun* subRun) : fPtr{subRun} { }
        Iterator& operator++() { fPtr = fPtr->fNext.get(); return *this; }
        Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; }
        bool operator==(const Iterator& rhs) const { return fPtr == rhs.fPtr; }
        bool operator!=(const Iterator& rhs) const { return fPtr != rhs.fPtr; }
        reference operator*() { return *fPtr; }

    private:
        SubRun* fPtr;
    };

    void append(SubRunOwner subRun) {
        SubRunOwner* newTail = &subRun->fNext;
        *fTail = std::move(subRun);
        fTail = newTail;
    }
    bool isEmpty() const { return fHead == nullptr; }
    Iterator begin() { return Iterator{ fHead.get()}; }
    Iterator end() { return Iterator{nullptr}; }
    Iterator begin() const { return Iterator{ fHead.get()}; }
    Iterator end() const { return Iterator{nullptr}; }
    SubRun& front() const {return *fHead; }

private:
    SubRunOwner fHead{nullptr};
    SubRunOwner* fTail{&fHead};
};

// -- TextBlob -----------------------------------------------------------------------------------
// A TextBlob contains a fully processed SkTextBlob, suitable for nearly immediate drawing
// on the GPU.  These are initially created with valid positions and colors, but with invalid
// texture coordinates.
//
// A TextBlob contains a number of SubRuns that are created in the blob's arena. Each SubRun
// tracks its own glyph and position data.
//
// In these classes, I'm trying to follow the convention about matrices and origins.
// * drawMatrix and drawOrigin - describes transformations for the current draw command.
// * positionMatrix - is equal to drawMatrix * [drawOrigin-as-translation-matrix]
// * initial Matrix - describes the combined initial matrix and origin the TextBlob was created
//                    with.
//
//
class TextBlob final : public SkRefCnt,
                       public SkGlyphRunPainterInterface {
public:
    // Key is not used as part of a hash map, so the hash is never taken. It's only used in a
    // list search using operator =().
    struct Key {
        static std::tuple<bool, Key> Make(const SkGlyphRunList& glyphRunList,
                                          const SkPaint& paint,
                                          const SkMatrix& drawMatrix,
                                          const SkStrikeDeviceInfo& strikeDevice);
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
        SkMatrix fPositionMatrix;
        // Below here fields are of size 1 byte.
        bool fHasSomeDirectSubRuns;
        bool fHasBlur;
        SkPaint::Style fStyle;
        SkPaint::Join fJoin;

        bool operator==(const Key& other) const;
    };

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(TextBlob);

    // Make a TextBlob and its sub runs.
    static sk_sp<TextBlob> Make(const SkGlyphRunList& glyphRunList,
                                const SkPaint& paint,
                                const SkMatrix& positionMatrix,
                                SkStrikeDeviceInfo strikeDeviceInfo,
                                SkStrikeForGPUCacheInterface* strikeCache);

    TextBlob(SubRunAllocator&& alloc,
             int totalMemorySize,
             const SkMatrix& positionMatrix,
             SkColor initialLuminance);

    ~TextBlob() override;

    // Change memory management to handle the data after TextBlob, but in the same allocation
    // of memory. Only allow placement new.
    void operator delete(void* p);
    void* operator new(size_t);
    void* operator new(size_t, void* p);

    const Key& key() { return fKey; }

    void addKey(const Key& key);
    bool hasPerspective() const;

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const;

    const Key& key() const;
    size_t size() const { return SkTo<size_t>(fSize); }

#if SK_SUPPORT_GPU
    void draw(SkCanvas*,
              const GrClip* clip,
              const SkMatrixProvider& viewMatrix,
              SkPoint drawOrigin,
              const SkPaint& paint,
              skgpu::v1::SurfaceDrawContext* sdc);
#endif
    const AtlasSubRun* testingOnlyFirstSubRun() const;

private:
    // Methods to satisfy SkGlyphRunPainterInterface
    bool processDeviceMasks(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                            sk_sp<SkStrike>&& strike) override;
    bool processSourcePaths(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                            const SkFont& runFont,
                            const SkDescriptor& descriptor,
                            SkScalar strikeToSourceScale) override;
    bool processSourceDrawables(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                                sk_sp<SkStrike>&& strike,
                                const SkDescriptor& descriptor,
                                SkScalar strikeToSourceScale) override;
    bool processSourceSDFT(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                           sk_sp<SkStrike>&& strike,
                           SkScalar strikeToSourceScale,
                           const SkFont& runFont,
                           const sktext::gpu::SDFTMatrixRange& matrixRange) override;
    bool processSourceMasks(const SkZip<SkGlyphVariant, SkPoint>& accepted,
                            sk_sp<SkStrike>&& strike,
                            SkScalar strikeToSourceScale) override;

    // The allocator must come first because it needs to be destroyed last. Other fields of this
    // structure may have pointers into it.
    SubRunAllocator fAlloc;

    // Owner and list of the SubRun.
    SubRunList fSubRunList;

    // Overall size of this struct plus vertices and glyphs at the end.
    const int fSize;

    // The initial view matrix combined with the initial origin. Used to determine if a cached
    // subRun can be used in this draw situation.
    const SkMatrix fInitialPositionMatrix;

    const SkColor fInitialLuminance;

    Key fKey;

    bool fSomeGlyphsExcluded{false};
};

}  // namespace sktext::gpu

// TODO: why is this only in v1?
namespace skgpu::v1 {
sk_sp<sktext::gpu::Slug> MakeSlug(const SkMatrixProvider& drawMatrix,
                                  const SkGlyphRunList& glyphRunList,
                                  const SkPaint& initialPaint,
                                  const SkPaint& drawingPaint,
                                  SkStrikeDeviceInfo strikeDeviceInfo,
                                  SkStrikeForGPUCacheInterface* strikeCache);
}  // namespace skgpu::v1

#endif  // sktext_gpu_TextBlob_DEFINED
