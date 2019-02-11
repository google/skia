/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkParticleEmitter_DEFINED
#define SkParticleEmitter_DEFINED

#include "SkReflected.h"

struct SkParticlePose;
class SkRandom;

class SkParticleEmitter : public SkReflected {
public:
    REFLECTED_ABSTRACT(SkParticleEmitter, SkReflected)

    virtual void emit(SkRandom&, SkParticlePose& pose) const = 0;

    static void RegisterEmitterTypes();
};

#endif // SkParticleEmitter_DEFINED
