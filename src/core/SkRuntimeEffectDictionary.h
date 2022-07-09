/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRuntimeEffectDictionary_DEFINED
#define SkRuntimeEffectDictionary_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkTHash.h"
#include "src/core/SkRuntimeEffectPriv.h"

class SkRuntimeEffect;

// We keep track of all SkRuntimeEffects that are used by a recording, along with their code
// snippet ID. This ensures that we have a live reference to every effect that we're going to
// paint, and gives us a way to retrieve their shader text when we see an their code-snippet ID.
class SkRuntimeEffectDictionary {
public:
    const SkRuntimeEffect* find(int codeSnippetID) const {
        sk_sp<const SkRuntimeEffect>* effect = fDict.find(codeSnippetID);
        return effect ? effect->get() : nullptr;
    }

    void set(int codeSnippetID, sk_sp<const SkRuntimeEffect> effect) {
         // The same code-snippet ID should never refer to two different effects.
         SkASSERT(!fDict.find(codeSnippetID) || (SkRuntimeEffectPriv::Hash(*fDict[codeSnippetID]) ==
                                                 SkRuntimeEffectPriv::Hash(*effect)));
         fDict.set(codeSnippetID, std::move(effect));
    }

    void reset() { fDict.reset(); }

private:
    SkTHashMap<int, sk_sp<const SkRuntimeEffect>> fDict;
};

#endif // SkRuntimeEffectDictionary_DEFINED
