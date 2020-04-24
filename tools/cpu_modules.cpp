/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/particles/include/SkParticleEffect.h"

// Doesn't do anything important; just exists to show we can use modules/particles without the GPU
// backend being available.
int main(int argc, char** argv) {
    // Register types for serialization
    SkParticleEffect::RegisterParticleTypes();
    return 0;
}
