/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathEffectPriv_DEFINED
#define SkPathEffectPriv_DEFINED

#include "include/core/SkPathEffect.h"

// Provides access to internal SkPathEffect APIs
class SkPathEffectPriv {
public:

    static bool ComputeFastBounds(const SkPathEffect* pe, SkRect* bounds) {
        return pe->computeFastBounds(bounds);
    }

private:
    SkPathEffectPriv();
};

#endif // SkPathEffectPriv_DEFINED
