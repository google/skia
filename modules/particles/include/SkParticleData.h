/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkParticleData_DEFINED
#define SkParticleData_DEFINED

#include "SkColor.h"
#include "SkPoint.h"
#include "SkRandom.h"
#include "SkRSXform.h"

/*
 *  Various structs used to communicate particle information among emitters, affectors, etc.
 */

struct SkParticlePose {
    SkPoint  fPosition;
    SkVector fHeading;
    SkScalar fScale;

    SkRSXform asRSXform(SkPoint ofs) const {
        const float s =  fHeading.fX * fScale;
        const float c = -fHeading.fY * fScale;
        return SkRSXform::Make(c, s,
                               fPosition.fX + -c * ofs.fX +  s * ofs.fY,
                               fPosition.fY + -s * ofs.fX + -c * ofs.fY);
    }
};

struct SkParticleVelocity {
    SkVector fLinear;
    SkScalar fAngular;
};

struct SkParticleState {
    float              fAge;          // Normalized age [0, 1]
    float              fInvLifetime;  // 1 / Lifetime
    SkParticlePose     fPose;
    SkParticleVelocity fVelocity;
    SkColor4f          fColor;
    SkScalar           fFrame;        // Parameter to drawable for animated sprites, etc.
    SkRandom           fRandom;
};

struct SkParticleUpdateParams {
    float fDeltaTime;
};

#endif // SkParticleData_DEFINED
