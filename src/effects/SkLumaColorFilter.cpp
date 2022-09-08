/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "include/effects/SkLumaColorFilter.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkRuntimeEffectPriv.h"

sk_sp<SkColorFilter> SkLumaColorFilter::Make() {
#ifdef SK_ENABLE_SKSL
    static const SkRuntimeEffect* effect = SkMakeCachedRuntimeEffect(
        SkRuntimeEffect::MakeForColorFilter,
        "half4 main(half4 inColor) {"
            "return saturate(dot(half3(0.2126, 0.7152, 0.0722), inColor.rgb)).000r;"
        "}"
    ).release();
    SkASSERT(effect);

    return effect->makeColorFilter(SkData::MakeEmpty());
#else
    // TODO(skia:12197)
    return nullptr;
#endif
}
