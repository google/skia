/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/RuntimeEffectDictionary.h"

#include "src/core/SkRuntimeEffectPriv.h"

namespace skgpu::graphite {

void RuntimeEffectDictionary::set(int codeSnippetID, sk_sp<const SkRuntimeEffect> effect) {
    // The same code-snippet ID should never refer to two different effects.
    SkASSERT(!fDict.find(codeSnippetID) || (SkRuntimeEffectPriv::Hash(*fDict[codeSnippetID]) ==
                                            SkRuntimeEffectPriv::Hash(*effect)));
    fDict.set(codeSnippetID, std::move(effect));
}

} // namespace skgpu::graphite
