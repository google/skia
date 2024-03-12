/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef sktext_SlugImpl_DEFINED
#define sktext_SlugImpl_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAssert.h"
#include "include/private/chromium/Slug.h"
#include "src/text/gpu/SubRunAllocator.h"
#include "src/text/gpu/SubRunContainer.h"

#include <cstddef>

class SkMatrix;
class SkPaint;
class SkReadBuffer;
class SkStrikeClient;
class SkWriteBuffer;
struct SkStrikeDeviceInfo;

namespace sktext {
class GlyphRunList;
class StrikeForGPUCacheInterface;
}

namespace sktext::gpu {

class SlugImpl final : public Slug {
public:
    SlugImpl(SubRunAllocator&& alloc,
             gpu::SubRunContainerOwner subRuns,
             SkRect sourceBounds,
             SkPoint origin);
    ~SlugImpl() override = default;

    static sk_sp<SlugImpl> Make(const SkMatrix& viewMatrix,
                                const sktext::GlyphRunList& glyphRunList,
                                const SkPaint& paint,
                                SkStrikeDeviceInfo strikeDeviceInfo,
                                sktext::StrikeForGPUCacheInterface* strikeCache);
    static sk_sp<Slug> MakeFromBuffer(SkReadBuffer& buffer,
                                      const SkStrikeClient* client);
    void doFlatten(SkWriteBuffer& buffer) const override;

    SkRect sourceBounds() const override { return fSourceBounds; }
    SkRect sourceBoundsWithOrigin() const override { return fSourceBounds.makeOffset(fOrigin); }

    const SkMatrix& initialPositionMatrix() const { return fSubRuns->initialPosition(); }
    SkPoint origin() const { return fOrigin; }

    const gpu::SubRunContainerOwner& subRuns() const { return fSubRuns; }

    // Change memory management to handle the data after Slug, but in the same allocation
    // of memory. Only allow placement new.
    void operator delete(void* p) { ::operator delete(p); }
    void* operator new(size_t) { SK_ABORT("All slugs are created by placement new."); }
    void* operator new(size_t, void* p) { return p; }

private:
    // The allocator must come first because it needs to be destroyed last. Other fields of this
    // structure may have pointers into it.
    SubRunAllocator fAlloc;
    gpu::SubRunContainerOwner fSubRuns;
    const SkRect fSourceBounds;
    const SkPoint fOrigin;
};

}  // namespace sktext::gpu

#endif
