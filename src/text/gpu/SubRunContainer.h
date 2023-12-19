/*
* Copyright 2022 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef sktext_gpu_SubRunContainer_DEFINED
#define sktext_gpu_SubRunContainer_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "src/text/gpu/SubRunAllocator.h"

#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <tuple>
#include <utility>

class SkCanvas;
class SkPaint;
class SkReadBuffer;
class SkStrikeClient;
class SkWriteBuffer;
struct SkIRect;
struct SkPoint;
struct SkStrikeDeviceInfo;

namespace sktext {
class GlyphRunList;
class StrikeForGPUCacheInterface;
}

namespace skgpu {
enum class MaskFormat : int;
}

#if defined(SK_GANESH) || defined(SK_USE_LEGACY_GANESH_TEXT_APIS)
#include "src/gpu/ganesh/GrColor.h"
#include "src/gpu/ganesh/ops/GrOp.h"

class GrClip;
namespace skgpu::ganesh {
class SurfaceDrawContext;
}
#endif

namespace sktext::gpu {
class GlyphVector;
class Glyph;
class StrikeCache;
class VertexFiller;

using RegenerateAtlasDelegate = std::function<std::tuple<bool, int>(GlyphVector*,
                                                                    int begin,
                                                                    int end,
                                                                    skgpu::MaskFormat,
                                                                    int padding)>;

struct RendererData {
    bool isSDF = false;
    bool isLCD = false;
};

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
class AtlasSubRun {
public:
    virtual ~AtlasSubRun() = default;

    virtual SkSpan<const Glyph*> glyphs() const = 0;
    virtual int glyphCount() const = 0;
    virtual skgpu::MaskFormat maskFormat() const = 0;
    virtual int glyphSrcPadding() const = 0;
    virtual unsigned short instanceFlags() const = 0;

#if defined(SK_GANESH) || defined(SK_USE_LEGACY_GANESH_TEXT_APIS)
    virtual size_t vertexStride(const SkMatrix& drawMatrix) const = 0;

    virtual std::tuple<const GrClip*, GrOp::Owner> makeAtlasTextOp(
            const GrClip*,
            const SkMatrix& viewMatrix,
            SkPoint drawOrigin,
            const SkPaint&,
            sk_sp<SkRefCnt>&& subRunStorage,
            skgpu::ganesh::SurfaceDrawContext*) const = 0;

    virtual void fillVertexData(
            void* vertexDst, int offset, int count,
            GrColor color,
            const SkMatrix& drawMatrix,
            SkPoint drawOrigin,
            SkIRect clip) const = 0;
#endif
    // This call is not thread safe. It should only be called from a known single-threaded env.
    virtual std::tuple<bool, int> regenerateAtlas(
            int begin, int end, RegenerateAtlasDelegate) const = 0;

    virtual const VertexFiller& vertexFiller() const = 0;

    virtual void testingOnly_packedGlyphIDToGlyph(StrikeCache* cache) const = 0;
};

using AtlasDrawDelegate = std::function<void(const sktext::gpu::AtlasSubRun* subRun,
                                             SkPoint drawOrigin,
                                             const SkPaint& paint,
                                             sk_sp<SkRefCnt> subRunStorage,
                                             sktext::gpu::RendererData)>;

// -- SubRun -------------------------------------------------------------------------------------
// SubRun defines the most basic functionality of a SubRun; the ability to draw, and the
// ability to be in a list.
class SubRun;
using SubRunOwner = std::unique_ptr<SubRun, SubRunAllocator::Destroyer>;
class SubRun {
public:
    virtual ~SubRun();

    virtual void draw(SkCanvas*, SkPoint drawOrigin, const SkPaint&, sk_sp<SkRefCnt> subRunStorage,
                      const AtlasDrawDelegate&) const = 0;

    void flatten(SkWriteBuffer& buffer) const;
    static SubRunOwner MakeFromBuffer(SkReadBuffer& buffer,
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
    enum SubRunStreamTag : int;
    virtual SubRunStreamTag subRunStreamTag() const = 0;
    virtual void doFlatten(SkWriteBuffer& buffer) const = 0;

private:
    friend class SubRunList;
    SubRunOwner fNext;
};

// -- SubRunList -----------------------------------------------------------------------------------
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
    [[nodiscard]] static SubRunContainerOwner MakeInAlloc(const GlyphRunList& glyphRunList,
                                                          const SkMatrix& positionMatrix,
                                                          const SkPaint& runPaint,
                                                          SkStrikeDeviceInfo strikeDeviceInfo,
                                                          StrikeForGPUCacheInterface* strikeCache,
                                                          sktext::gpu::SubRunAllocator* alloc,
                                                          SubRunCreationBehavior creationBehavior,
                                                          const char* tag);

    static size_t EstimateAllocSize(const GlyphRunList& glyphRunList);

    void draw(SkCanvas*, SkPoint drawOrigin, const SkPaint&, const SkRefCnt* subRunStorage,
              const AtlasDrawDelegate&) const;

    const SkMatrix& initialPosition() const { return fInitialPositionMatrix; }
    bool isEmpty() const { return fSubRuns.isEmpty(); }
    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const;

private:
    friend struct SubRunContainerPeer;
    const SkMatrix fInitialPositionMatrix;
    SubRunList fSubRuns;
};

// Returns the empty span if there is a problem reading the positions.
SkSpan<SkPoint> MakePointsFromBuffer(SkReadBuffer&, SubRunAllocator*);

}  // namespace sktext::gpu

#endif  // sktext_gpu_SubRunContainer_DEFINED
