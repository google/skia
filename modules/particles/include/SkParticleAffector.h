/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkParticleAffector_DEFINED
#define SkParticleAffector_DEFINED

#include "SkReflected.h"

struct SkParticlePoseAndVelocity;
struct SkParticleUpdateParams;

class SkParticleAffector : public SkReflected {
public:
    REFLECTED_ABSTRACT(SkParticleAffector, SkReflected)

    virtual void apply(SkParticleUpdateParams& params, SkParticlePoseAndVelocity& pv) = 0;

    static void RegisterAffectorTypes();
};

#endif // SkParticleAffector_DEFINED
