/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkParticleData_DEFINED
#define SkParticleData_DEFINED

#include "include/private/SkTemplates.h"
#include "include/utils/SkRandom.h"

/*
 *  Various structs used to communicate particle information among emitters, affectors, etc.
 */

struct SkParticles {
    enum Channels {
        kAge,
        kLifetime,  // During spawn, this is actual lifetime. Later, it's inverse lifetime.
        kPositionX,
        kPositionY,
        kHeadingX,
        kHeadingY,
        kScale,
        kVelocityX,
        kVelocityY,
        kVelocityAngular,
        kColorR,
        kColorG,
        kColorB,
        kColorA,
        kSpriteFrame,
        kFlags,       // This is the only non-float data (it's actually uint32_t)

        kNumChannels,
    };

    SkAutoTMalloc<float>    fData[kNumChannels];
    SkAutoTMalloc<SkRandom> fRandom;
};

#endif // SkParticleData_DEFINED
