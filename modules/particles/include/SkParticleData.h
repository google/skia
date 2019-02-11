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
    SkVector fRight;
    SkScalar fScale;

    SkRSXform asRSXform(SkPoint ofs) const {
        const float s = fRight.fY * fScale;
        const float c = fRight.fX * fScale;
        return SkRSXform::Make(c, s,
                               fPosition.fX + -c * ofs.fX + s * ofs.fY,
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

// Ranged constant. Keeping this here temporarily. Phase this out in favor of SkCurve everywhere.
struct SkRangedFloat {
    float eval(SkRandom& random) { return random.nextRangeF(fMin, fMax); }
    float* vec() { return &fMin; }

    float fMin = 0.0f;
    float fMax = 0.0f;

    void visitFields(SkFieldVisitor* v) {
        v->visit("min", fMin);
        v->visit("max", fMax);
    }
};

#endif // SkParticleData_DEFINED
