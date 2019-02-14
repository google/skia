/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkParticleAffector_DEFINED
#define SkParticleAffector_DEFINED

#include "SkReflected.h"

#include "SkPoint.h"

struct SkCurve;
struct SkParticlePoseAndVelocity;
struct SkParticleUpdateParams;

class SkParticleAffector : public SkReflected {
public:
    REFLECTED_ABSTRACT(SkParticleAffector, SkReflected)

    virtual void apply(SkParticleUpdateParams& params, SkParticlePoseAndVelocity& pv) = 0;

    static void RegisterAffectorTypes();

    static sk_sp<SkParticleAffector> MakeLinearVelocity(const SkCurve& angle,
                                                        const SkCurve& strength,
                                                        bool force);
    static sk_sp<SkParticleAffector> MakePointForce(SkPoint point, SkScalar constant,
                                                    SkScalar invSquare);
    static sk_sp<SkParticleAffector> MakeOrientAlongVelocity();

    static sk_sp<SkParticleAffector> MakeSizeAffector(const SkCurve& curve);
};

#endif // SkParticleAffector_DEFINED
