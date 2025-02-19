/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_RuntimeEffectDictionary_DEFINED
#define skgpu_graphite_RuntimeEffectDictionary_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkTHash.h"

class SkRuntimeEffect;

namespace skgpu::graphite {

// We keep track of all SkRuntimeEffects that are used by a recording, along with their code
// snippet ID. This ensures that we have a live reference to every effect that we're going to
// paint, and gives us a way to retrieve their shader text when we see their code-snippet ID.
class RuntimeEffectDictionary {
public:
    const SkRuntimeEffect* find(int codeSnippetID) const {
        sk_sp<const SkRuntimeEffect>* effect = fDict.find(codeSnippetID);
        return effect ? effect->get() : nullptr;
    }

    void set(int codeSnippetID, sk_sp<const SkRuntimeEffect> effect);

    void reset() { fDict.reset(); }

    bool empty() const { return fDict.empty(); }

private:
    skia_private::THashMap<int, sk_sp<const SkRuntimeEffect>> fDict;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_RuntimeEffectDictionary_DEFINED
