/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ScratchResourceManager.h"

#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

ScratchResourceManager::ScratchResourceManager(ResourceProvider* resourceProvider,
                                               std::unique_ptr<ProxyReadCountMap> proxyCounts)
        : fResourceProvider(resourceProvider)
        , fProxyReadCounts(std::move(proxyCounts)) {
    SkASSERT(resourceProvider);
    SkASSERT(fProxyReadCounts);
}

ScratchResourceManager::~ScratchResourceManager() = default;

sk_sp<Texture> ScratchResourceManager::getScratchTexture(SkISize dimensions,
                                                         const TextureInfo& info,
                                                         std::string_view label) {
    for (ScratchTexture& st : fScratchTextures) {
        if (st.fAvailable &&
            st.fTexture->dimensions() == dimensions &&
            st.fTexture->textureInfo() == info) {
            // An exact match, reuse it.
            st.fAvailable = false;
            return st.fTexture;
        }
    }

    // No texture was available so go out to the resource provider, which will hopefully find a
    // cached resource that was freed up from a previous recording (or create a new one, if not).
    // TODO(b/339496039): Always start with a fixed label like "ScratchTexture" and then concatenate
    // the proxy label that's passed in onto the texture's label, including when reusing a texture.
    sk_sp<Texture> newScratchTexture = fResourceProvider->findOrCreateScratchTexture(
            dimensions, info, std::move(label), Budgeted::kYes);
    if (newScratchTexture) {
        fScratchTextures.push_back({newScratchTexture, /*fAvailable=*/false});
    }
    return newScratchTexture;
}

void ScratchResourceManager::returnTexture(sk_sp<Texture> texture) {
    for (ScratchTexture& st : fScratchTextures) {
        if (st.fTexture.get() == texture.get()) {
            SkASSERT(!st.fAvailable);
            st.fAvailable = true;
            return;
        }
    }
    // Trying to return a resource that didn't come from getScratchTexture().
    SkASSERT(false);
}

void ScratchResourceManager::pushScope() {
    // Push a null pointer to mark the beginning of the list of listeners in the next depth
    fListenerStack.push_back(nullptr);
}

void ScratchResourceManager::popScope() {
    // Must have at least the null element to start the scope being popped
    SkASSERT(!fListenerStack.empty());

    // TODO: Assert that the current sublist is empty (i.e. the back element is a null pointer) but
    // for now skip over them and leave them un-invoked to keep the unconsumed scratch resources
    // out of the pool so they remain valid in later recordings.
    int n = 0;
    while (fListenerStack.fromBack(n)) {
        n++;
    }
    SkASSERT(n < fListenerStack.size() && fListenerStack.fromBack(n) == nullptr);
    // Remove all non-null listeners after the most recent null entry AND the null entry
    fListenerStack.pop_back_n(n + 1);
}

void ScratchResourceManager::notifyResourcesConsumed() {
    // Should only be called inside a scope
    SkASSERT(!fListenerStack.empty());

    int n = 0;
    while (PendingUseListener* listener = fListenerStack.fromBack(n)) {
        listener->onUseCompleted(this);
        n++;
    }
    SkASSERT(n < fListenerStack.size() && fListenerStack.fromBack(n) == nullptr);
    // Remove all non-null listeners that were just invoked, but do not remove the null entry that
    // marks the start of this scope boundary.
    if (n > 0) {
        fListenerStack.pop_back_n(n);
    }
}

void ScratchResourceManager::markResourceInUse(PendingUseListener* listener) {
    // Should only be called inside a scope
    SkASSERT(!fListenerStack.empty());
    fListenerStack.push_back(listener);
}

}  // namespace skgpu::graphite
