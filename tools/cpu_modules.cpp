/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/particles/include/SkParticleDrawable.h"
#include "modules/particles/include/SkParticleEffect.h"
#include "modules/particles/include/SkParticleSerialization.h"
#include "modules/particles/include/SkReflected.h"

// Doesn't do anything important; just exists to show we can use modules/particles without the GPU
// backend being available.
int main(int argc, char** argv) {
    // Register types for serialization
    REGISTER_REFLECTED(SkReflected);
    SkParticleBinding::RegisterBindingTypes();
    SkParticleDrawable::RegisterDrawableTypes();
    return 0;
}
