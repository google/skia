/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/text/gpu/SlugImpl.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSerialProcs.h"
#include "include/private/SkAssert.h"
#include "include/private/chromium/Slug.h"
#include "src/core/SkDevice.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/text/GlyphRun.h"
#include "src/text/gpu/SubRunAllocator.h"
#include "src/text/gpu/SubRunContainer.h"
#include "src/text/gpu/VertexFiller.h"

#include <memory>
#include <utility>

class SkStrikeClient;

namespace sktext::gpu {

SlugImpl::SlugImpl(SubRunAllocator&& alloc,
                   gpu::SubRunContainerOwner subRuns,
                   SkRect sourceBounds,
                   SkPoint origin)
        : fAlloc {std::move(alloc)}
        , fSubRuns(std::move(subRuns))
        , fSourceBounds{sourceBounds}
        , fOrigin{origin} {}

namespace {
SkMatrix position_matrix(const SkMatrix& drawMatrix, SkPoint drawOrigin) {
    SkMatrix position_matrix = drawMatrix;
    return position_matrix.preTranslate(drawOrigin.x(), drawOrigin.y());
}

// Every Slug has finite, non-empty source bounds and a finite origin. Slugs are only created with
// values that pass this check, and deserialization rejects any that don't.
bool valid_bounds_and_origin(SkRect sourceBounds, SkPoint origin) {
    return sourceBounds.isFinite() && !sourceBounds.isEmpty() && origin.isFinite();
}
}  // namespace

void SlugImpl::doFlatten(SkWriteBuffer& buffer) const {
    buffer.writeRect(fSourceBounds);
    buffer.writePoint(fOrigin);
    fSubRuns->flattenAllocSizeHint(buffer);
    fSubRuns->flattenRuns(buffer);
}

sk_sp<Slug> SlugImpl::MakeFromBuffer(SkReadBuffer& buffer, const SkStrikeClient* client) {
    SkRect sourceBounds = buffer.readRect();
    SkPoint origin = buffer.readPoint();
    if (!buffer.validate(valid_bounds_and_origin(sourceBounds, origin))) {
        return nullptr;
    }
    int allocSizeHint = gpu::SubRunContainer::AllocSizeHintFromBuffer(buffer);

    auto [initializer, _, alloc] =
            SubRunAllocator::AllocateClassMemoryAndArena<SlugImpl>(allocSizeHint);

    gpu::SubRunContainerOwner container =
            gpu::SubRunContainer::MakeFromBufferInAlloc(buffer, client, &alloc);

    // Something went wrong while reading.
    if (!buffer.isValid()) {
        return nullptr;
    }

    return sk_sp<SlugImpl>(
            initializer.initialize(std::move(alloc), std::move(container), sourceBounds, origin));
}

bool SlugImpl::canDraw(const SkMatrix& drawMatrix, bool supportBilerpAtlas) const {
    // Only direct-mask glyphs are packed into the atlas without padding, and only when the atlas
    // doesn't support bilerp. Every other SubRun type is either padded in the atlas (atlas
    // entries are keyed by padding, so they are never shared with direct masks) or doesn't use
    // the atlas at all, so it can be drawn with any matrix.
    if (supportBilerpAtlas || !fSubRuns->hasDirectMaskSubRuns()) {
        return true;
    }
    // Direct masks may only be drawn with a bounded integer translation of their creation
    // matrix. Use the same check, on the same single-precision positionMatrix, that the backends
    // use when drawing the direct-mask SubRuns.
    auto [canUseDirect, _] = VertexFiller::CanUseDirect(this->initialPositionMatrix(),
                                                       position_matrix(drawMatrix, fOrigin));
    return canUseDirect;
}

sk_sp<SlugImpl> SlugImpl::Make(const SkMatrix& viewMatrix,
                               const sktext::GlyphRunList& glyphRunList,
                               const SkPaint& paint,
                               SkStrikeDeviceInfo strikeDeviceInfo,
                               sktext::StrikeForGPUCacheInterface* strikeCache) {
    // Only make Slugs that can be deserialized: SlugImpl::MakeFromBuffer requires valid bounds and
    // origin, and SubRunContainer::MakeFromBufferInAlloc requires a finite positionMatrix (its
    // translation can overflow even when the viewMatrix and origin are finite).
    const SkMatrix positionMatrix = position_matrix(viewMatrix, glyphRunList.origin());
    if (!positionMatrix.isFinite() ||
        !valid_bounds_and_origin(glyphRunList.sourceBounds(), glyphRunList.origin())) {
        return nullptr;
    }

    size_t subRunSizeHint = gpu::SubRunContainer::EstimateAllocSize(glyphRunList);
    auto [initializer, _, alloc] =
            SubRunAllocator::AllocateClassMemoryAndArena<SlugImpl>(subRunSizeHint);

    auto subRuns = gpu::SubRunContainer::MakeInAlloc(glyphRunList,
                                                     positionMatrix,
                                                     paint,
                                                     strikeDeviceInfo,
                                                     strikeCache,
                                                     &alloc,
                                                     gpu::SubRunContainer::kAddSubRuns,
                                                     "Make Slug");

    sk_sp<SlugImpl> slug = sk_sp<SlugImpl>(initializer.initialize(std::move(alloc),
                                                                  std::move(subRuns),
                                                                  glyphRunList.sourceBounds(),
                                                                  glyphRunList.origin()));

    // There is nothing to draw here. This is particularly a problem with RSX form blobs where a
    // single space becomes a run with no glyphs.
    if (slug->fSubRuns->isEmpty()) { return nullptr; }

    return slug;
}

void Slug::AddDeserialProcs(SkDeserialProcs* procs, const SkStrikeClient* client) {
    SkASSERT(procs);
    procs->fSlugCtx = const_cast<SkStrikeClient*>(client);
    procs->fSlugProc = [](SkReadBuffer& buffer, void* ctx) -> sk_sp<Slug> {
        auto client = static_cast<const SkStrikeClient*>(ctx);
        return SlugImpl::MakeFromBuffer(buffer, client);
    };
}

}  // namespace sktext::gpu
