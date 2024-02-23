/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkKnownRuntimeEffects.h"

#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkRuntimeEffectPriv.h"

namespace SkKnownRuntimeEffects {

SkRuntimeEffect* GetKnownRuntimeEffect(StableKey stableKey) {

    SkRuntimeEffect::Options options;
    SkRuntimeEffectPriv::SetStableKey(&options, static_cast<uint32_t>(stableKey));

    switch (stableKey) {
        case StableKey::kInvalid:
            return nullptr;
        case StableKey::kBlend: {
            static SkRuntimeEffect* sBlendEffect =
                    SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                                        "uniform shader s, d;"
                                        "uniform blender b;"
                                        "half4 main(float2 xy) {"
                                            "return b.eval(s.eval(xy), d.eval(xy));"
                                        "}",
                                        options);
            return sBlendEffect;
        }
    }

    SkUNREACHABLE;
}

} // namespace SkKnownRuntimeEffects
