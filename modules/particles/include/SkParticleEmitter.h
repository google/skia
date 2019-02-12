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
struct SkPoint;
class SkRandom;

class SkParticleEmitter : public SkReflected {
public:
    REFLECTED_ABSTRACT(SkParticleEmitter, SkReflected)

    virtual SkParticlePose emit(SkRandom&) const = 0;

    static void RegisterEmitterTypes();

    static sk_sp<SkParticleEmitter> MakeCircle(SkPoint center, SkScalar radius);
    static sk_sp<SkParticleEmitter> MakeLine(SkPoint p1, SkPoint p2);
    static sk_sp<SkParticleEmitter> MakeText(const char* text, SkScalar fontSize);
};

#endif // SkParticleEmitter_DEFINED
