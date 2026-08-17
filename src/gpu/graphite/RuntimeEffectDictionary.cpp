/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/RuntimeEffectDictionary.h"

#include "src/core/SkMeshPriv.h"
#include "src/core/SkRuntimeEffectPriv.h"

namespace skgpu::graphite {

void RuntimeEffectDictionary::set(int codeSnippetID, sk_sp<const SkRuntimeEffect> effect) {
    SkAutoSpinlock lock{fSpinLock};

    // The same code-snippet ID should never refer to two different effects.
    SkASSERT(!fDict.find(codeSnippetID) || (SkRuntimeEffectPriv::Hash(*fDict[codeSnippetID]) ==
                                            SkRuntimeEffectPriv::Hash(*effect)));
    fDict.set(codeSnippetID, std::move(effect));
}

void RuntimeEffectDictionary::set(int codeSnippetID, sk_sp<const SkMeshSpecification> spec) {
    SkAutoSpinlock lock{fSpinLock};

    SkASSERT(!fMeshSpecDict.find(codeSnippetID) ||
             SkMeshSpecificationPriv::Hash(**fMeshSpecDict.find(codeSnippetID))
                == SkMeshSpecificationPriv::Hash(*spec));
    fMeshSpecDict.set(codeSnippetID, std::move(spec));
}

} // namespace skgpu::graphite
