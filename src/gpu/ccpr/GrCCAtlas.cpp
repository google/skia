/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrCCAtlas.h"

#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/ccpr/GrCCPathCache.h"

static SkISize choose_initial_atlas_size(const GrCCAtlas::Specs& specs) {
    // Begin with the first pow2 dimensions whose area is theoretically large enough to contain the
    // pending paths, favoring height over width if necessary.
    int log2area = SkNextLog2(std::max(specs.fApproxNumPixels, 1));
    int height = 1 << ((log2area + 1) / 2);
    int width = 1 << (log2area / 2);

    width = SkTPin(width, specs.fMinTextureSize, specs.fMaxPreferredTextureSize);
    height = SkTPin(height, specs.fMinTextureSize, specs.fMaxPreferredTextureSize);

    return SkISize::Make(width, height);
}

static int choose_max_atlas_size(const GrCCAtlas::Specs& specs, const GrCaps& caps) {
    return (std::max(specs.fMinHeight, specs.fMinWidth) <= specs.fMaxPreferredTextureSize) ?
            specs.fMaxPreferredTextureSize : caps.maxRenderTargetSize();
}

GrCCAtlas::GrCCAtlas(CoverageType coverageType, const Specs& specs, const GrCaps& caps)
        : GrDynamicAtlas(CoverageTypeToColorType(coverageType),
                         CoverageTypeHasInternalMultisample(coverageType),
                         choose_initial_atlas_size(specs), choose_max_atlas_size(specs, caps), caps)
        , fCoverageType(coverageType) {
    SkASSERT(specs.fMaxPreferredTextureSize > 0);
}

GrCCAtlas::~GrCCAtlas() {
}

void GrCCAtlas::setFillBatchID(int id) {
    // This can't be called anymore once makeRenderTargetContext() has been called.
    SkASSERT(!this->isInstantiated());
    fFillBatchID = id;
}

void GrCCAtlas::setStrokeBatchID(int id) {
    // This can't be called anymore once makeRenderTargetContext() has been called.
    SkASSERT(!this->isInstantiated());
    fStrokeBatchID = id;
}

void GrCCAtlas::setEndStencilResolveInstance(int idx) {
    // This can't be called anymore once makeRenderTargetContext() has been called.
    SkASSERT(!this->isInstantiated());
    fEndStencilResolveInstance = idx;
}

static uint32_t next_atlas_unique_id() {
    static std::atomic<uint32_t> nextID;
    return nextID++;
}

sk_sp<GrCCCachedAtlas> GrCCAtlas::refOrMakeCachedAtlas(GrOnFlushResourceProvider* onFlushRP) {
    if (!fCachedAtlas) {
        static const GrUniqueKey::Domain kAtlasDomain = GrUniqueKey::GenerateDomain();

        GrUniqueKey atlasUniqueKey;
        GrUniqueKey::Builder builder(&atlasUniqueKey, kAtlasDomain, 1, "CCPR Atlas");
        builder[0] = next_atlas_unique_id();
        builder.finish();

        onFlushRP->assignUniqueKeyToProxy(atlasUniqueKey, this->textureProxy());

        fCachedAtlas = sk_make_sp<GrCCCachedAtlas>(fCoverageType, atlasUniqueKey,
                                                   sk_ref_sp(this->textureProxy()));
    }

    SkASSERT(fCachedAtlas->coverageType() == fCoverageType);
    SkASSERT(fCachedAtlas->getOnFlushProxy() == this->textureProxy());
    return fCachedAtlas;
}

GrCCAtlas* GrCCAtlasStack::addRect(const SkIRect& devIBounds, SkIVector* devToAtlasOffset) {
    GrCCAtlas* retiredAtlas = nullptr;
    if (fAtlases.empty() || !fAtlases.back().addRect(devIBounds, devToAtlasOffset)) {
        // The retired atlas is out of room and can't grow any bigger.
        retiredAtlas = !fAtlases.empty() ? &fAtlases.back() : nullptr;
        fAtlases.emplace_back(fCoverageType, fSpecs, *fCaps);
        SkASSERT(devIBounds.width() <= fSpecs.fMinWidth);
        SkASSERT(devIBounds.height() <= fSpecs.fMinHeight);
        SkAssertResult(fAtlases.back().addRect(devIBounds, devToAtlasOffset));
    }
    return retiredAtlas;
}
