/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkParticleData_DEFINED
#define SkParticleData_DEFINED

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

struct SkParticlePoseAndVelocity {
    SkParticlePose     fPose;
    SkParticleVelocity fVelocity;
};

struct SkParticleUpdateParams {
    SkRandom* fRandom;
    SkRandom* fStableRandom;
    float fDeltaTime;
    float fParticleT;
};

#endif // SkParticleData_DEFINED
