/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Texture.h"

#include "include/core/SkTraceMemoryDump.h"
#include "include/gpu/MutableTextureState.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/TextureUtils.h"

namespace skgpu::graphite {

Texture::Texture(const SharedContext* sharedContext,
                 SkISize dimensions,
                 const TextureInfo& info,
                 sk_sp<MutableTextureState> mutableState,
                 Ownership ownership,
                 skgpu::Budgeted budgeted)
        : Resource(sharedContext, ownership, budgeted, ComputeSize(dimensions, info))
        , fDimensions(dimensions)
        , fInfo(info)
        , fMutableState(std::move(mutableState)) {}

Texture::~Texture() {}

void Texture::setReleaseCallback(sk_sp<RefCntedCallback> releaseCallback) {
    fReleaseCallback = std::move(releaseCallback);
}

void Texture::invokeReleaseProc() {
    if (fReleaseCallback) {
        // Depending on the ref count of fReleaseCallback this may or may not actually trigger
        // the ReleaseProc to be called.
        fReleaseCallback.reset();
    }
}

MutableTextureState* Texture::mutableState() const { return fMutableState.get(); }

void Texture::onDumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump,
                                     const char* dumpName) const {
    SkString dimensionsStr;
    dimensionsStr.printf("(%dx%d)", fDimensions.width(), fDimensions.height());
    traceMemoryDump->dumpStringValue(dumpName, "dimensions", dimensionsStr.c_str());
    traceMemoryDump->dumpStringValue(dumpName, "textureInfo", fInfo.toString().c_str());
}

} // namespace skgpu::graphite
