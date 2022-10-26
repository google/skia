/*
* Copyright 2022 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef sktext_gpu_SubRunContainer_DEFINED
#define sktext_gpu_SubRunContainer_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "src/core/SkDevice.h"
#include "src/gpu/AtlasTypes.h"
#include "src/text/gpu/SubRunAllocator.h"

class SkMatrix;
class SkMatrixProvider;
class SkPaint;
class SkReadBuffer;
class SkStrikeClient;
class SkWriteBuffer;

namespace sktext {
class GlyphRunList;
class StrikeForGPUCacheInterface;
    namespace gpu {
    class Glyph;
    class StrikeCache;
    }
}

#if SK_SUPPORT_GPU  // Ganesh support
#include "src/gpu/ganesh/GrColor.h"
#include "src/gpu/ganesh/ops/GrOp.h"

class GrAtlasManager;
class GrDeferredUploadTarget;
class GrMeshDrawTarget;
class GrClip;
namespace skgpu::v1 { class SurfaceDrawContext; }
#endif

#if defined(SK_GRAPHITE_ENABLED)
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/SubRunData.h"
#include "src/gpu/graphite/geom/Transform_graphite.h"

namespace skgpu {
enum class MaskFormat : int;
}

namespace skgpu::graphite {
class DrawWriter;
class Recorder;
class Renderer;
class RendererProvider;
}
#endif

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
    virtual skgpu::MaskFormat maskFormat() const = 0;

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

#if defined(SK_GRAPHITE_ENABLED)
    virtual std::tuple<bool, int> regenerateAtlas(
            int begin, int end, skgpu::graphite::Recorder*) const = 0;

    // returns bounds of the stored data and matrix to transform it to device space
    virtual std::tuple<skgpu::graphite::Rect, skgpu::graphite::Transform> boundsAndDeviceMatrix(
            const skgpu::graphite::Transform& localToDevice, SkPoint drawOrigin) const = 0;

    virtual const skgpu::graphite::Renderer* renderer(
            const skgpu::graphite::RendererProvider*) const = 0;

    virtual void fillInstanceData(
            skgpu::graphite::DrawWriter*,
            int offset, int count,
            int ssboIndex,
            SkScalar depth) const = 0;
#endif

    virtual void testingOnly_packedGlyphIDToGlyph(StrikeCache* cache) const = 0;

protected:
#if defined(SK_GRAPHITE_ENABLED)
    void draw(skgpu::graphite::Device*,
              SkPoint drawOrigin,
              const SkPaint&,
              sk_sp<SkRefCnt> subRunStorage) const;
#endif
};

// -- SubRun -------------------------------------------------------------------------------------
// SubRun defines the most basic functionality of a SubRun; the ability to draw, and the
// ability to be in a list.
class SubRun;
using SubRunOwner = std::unique_ptr<SubRun, SubRunAllocator::Destroyer>;
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
#if defined(SK_GRAPHITE_ENABLED)
    // Produce uploads and draws for this subRun
    virtual void draw(SkCanvas*,
                      SkPoint drawOrigin,
                      const SkPaint&,
                      sk_sp<SkRefCnt> subRunStorage,
                      skgpu::graphite::Device*) const = 0;
#endif

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

// -- SubRunContainer ------------------------------------------------------------------------------
class SubRunContainer;
using SubRunContainerOwner = std::unique_ptr<SubRunContainer, SubRunAllocator::Destroyer>;
class SubRunContainer {
public:
    explicit SubRunContainer(const SkMatrix& initialPositionMatrix);
    SubRunContainer() = delete;
    SubRunContainer(const SubRunContainer&) = delete;
    SubRunContainer& operator=(const SubRunContainer&) = delete;

    // Delete the move operations because the SubRuns contain pointers to fInitialPositionMatrix.
    SubRunContainer(SubRunContainer&&) = delete;
    SubRunContainer& operator=(SubRunContainer&&) = delete;

    void flattenAllocSizeHint(SkWriteBuffer& buffer) const;
    static int AllocSizeHintFromBuffer(SkReadBuffer& buffer);

    void flattenRuns(SkWriteBuffer& buffer) const;
    static SubRunContainerOwner MakeFromBufferInAlloc(SkReadBuffer& buffer,
                                                      const SkStrikeClient* client,
                                                      SubRunAllocator* alloc);

    enum SubRunCreationBehavior {kAddSubRuns, kStrikeCalculationsOnly};
    // The returned SubRunContainerOwner will never be null. If subRunCreation ==
    // kStrikeCalculationsOnly, then the returned container will be empty.
    static SK_WARN_UNUSED_RESULT SubRunContainerOwner MakeInAlloc(
            const GlyphRunList& glyphRunList,
            const SkMatrix& positionMatrix,
            const SkPaint& runPaint,
            SkStrikeDeviceInfo strikeDeviceInfo,
            StrikeForGPUCacheInterface* strikeCache,
            sktext::gpu::SubRunAllocator* alloc,
            SubRunCreationBehavior creationBehavior,
            const char* tag);

    static size_t EstimateAllocSize(const GlyphRunList& glyphRunList);

#if SK_SUPPORT_GPU
    void draw(SkCanvas* canvas,
              const GrClip* clip,
              const SkMatrixProvider& viewMatrix,
              SkPoint drawOrigin,
              const SkPaint& paint,
              const SkRefCnt* subRunStorage,
              skgpu::v1::SurfaceDrawContext* sdc) const;
#endif
#ifdef SK_GRAPHITE_ENABLED
    void draw(SkCanvas*,
              SkPoint drawOrigin,
              const SkPaint&,
              const SkRefCnt* subRunStorage,
              skgpu::graphite::Device*) const;
#endif

    const SkMatrix& initialPosition() const { return fInitialPositionMatrix; }
    bool isEmpty() const { return fSubRuns.isEmpty(); }
    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const;

private:
    friend struct SubRunContainerPeer;
    const SkMatrix fInitialPositionMatrix;
    SubRunList fSubRuns;
};
}  // namespace sktext::gpu

#endif  // sktext_gpu_SubRunContainer_DEFINED
