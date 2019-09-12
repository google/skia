/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkParticleEffect_DEFINED
#define SkParticleEffect_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTemplates.h"
#include "include/utils/SkRandom.h"
#include "modules/particles/include/SkParticleData.h"

#include <memory>

class SkCanvas;
class SkFieldVisitor;
class SkParticleBinding;
class SkParticleDrawable;
class SkParticleExternalValue;

namespace SkSL {
    struct ByteCode;
}

class SkParticleEffectParams : public SkRefCnt {
public:
    SkParticleEffectParams();

    // Maximum number of particles per instance of the effect
    int   fMaxCount;

    // What is drawn for each particle? (Image, shape, sprite sheet, etc.)
    // See SkParticleDrawable::Make*
    sk_sp<SkParticleDrawable> fDrawable;

    // Particle behavior is driven by two chunks of SkSL code. Effect functions are defined in
    // fEffectCode, and get a mutable Effect struct:
    //
    // struct Effect {
    //   float age;
    //   float lifetime;
    //   int   loop;
    //   float rate;
    //   int   burst;                    // Set to trigger a burst of particles.
    //
    //   // Everything below this line controls the state of the effect, which is also the
    //   // default values for new particles.
    //   float2 pos   = { 0, 0 };        // Local position
    //   float2 dir   = { 0, -1 };       // Heading. Should be a normalized vector.
    //   float  scale = 1;               // Size, normalized relative to the drawable's native size
    //   float2 vel   = { 0, 0 };        // Linear velocity, in (units / second)
    //   float  spin  = 0;               // Angular velocity, in (radians / second)
    //   float4 color = { 1, 1, 1, 1 };  // RGBA color
    //   float  frame = 0;               // Normalized sprite index for multi-frame drawables
    // };
    //
    // Particle functions are defined in fParticleCode, and get a mutable Particle struct, as well
    // as a uniform copy of the current Effect, named 'effect'.
    //
    // struct Particle {
    //   float  age;
    //   float  lifetime;
    //   float2 pos;
    //   float2 dir;
    //   float  scale;
    //   float2 vel;
    //   float  spin;
    //   float4 color;
    //   float  frame;
    // };
    //
    // All functions have access to a global variable named 'rand'. Every read of 'rand' returns a
    // random floating point value in [0, 1). For particle functions, the state is rewound after
    // each update, so calls to 'rand' will return consistent values from one update to the next.
    //
    // Finally, there is one global uniform values available, 'dt'. This is a floating point
    // number of seconds that have elapsed since the last update.
    //
    // Effect code should define two functions:
    //
    // 'void effectSpawn(inout Effect e)' is called when an instance of the effect is first
    // created, and again at every loop point (if the effect is played with the looping flag).
    //
    // 'void effectUpdate(inout Effect e)' is called once per update to adjust properties of the
    // effect (ie emitter).
    //
    // Particle code should also define two functions:
    //
    // 'void spawn(inout Particle p)' is called once for each particle when it is first created,
    // to set initial values. At a minimum, this should set 'lifetime' to the number of seconds
    // that the particle will exist. Other parameters will will get default values from the effect.
    //
    // 'void update(inout Particle p)' is called for each particle on every call to the running
    // SkParticleEffect's update() method. It can animate any of the particle's values. Note that
    // the 'lifetime' field has a different meaning in 'update', and should not be used or changed.

    SkString fEffectCode;
    SkString fParticleCode;

    // External objects accessible by the effect's SkSL code. Each binding is a name and particular
    // kind of object. See SkParticleBinding::Make* for details.
    SkTArray<sk_sp<SkParticleBinding>> fBindings;

    void visitFields(SkFieldVisitor* v);

private:
    friend class SkParticleEffect;

    // Cached
    struct Program {
        std::unique_ptr<SkSL::ByteCode> fByteCode;
        SkTArray<std::unique_ptr<SkParticleExternalValue>> fExternalValues;
    };

    Program fEffectProgram;
    Program fParticleProgram;

    void rebuild();
};

class SkParticleEffect : public SkRefCnt {
public:
    SkParticleEffect(sk_sp<SkParticleEffectParams> params, const SkRandom& random);

    void start(double now, bool looping = false);
    void update(double now);
    void draw(SkCanvas* canvas);

    bool isAlive() const { return fState.fAge >= 0 && fState.fAge <= 1; }
    int getCount() const { return fCount; }

    static void RegisterParticleTypes();

private:
    void setCapacity(int capacity);

    sk_sp<SkParticleEffectParams> fParams;

    SkRandom fRandom;

    bool   fLooping;
    int    fCount;
    double fLastTime;
    float  fSpawnRemainder;

    // Effect-associated values exposed to script. They are some mix of uniform and inout,
    // depending on whether we're executing per-feffect or per-particle scripts.
    struct EffectState {
        float fDeltaTime;

        // Above this line is always uniform. Below is uniform for particles, inout for effect.

        float fAge;
        float fLifetime;
        int   fLoopCount;
        float fRate;
        int   fBurst;

        // Properties that determine default values for new particles
        SkPoint   fPosition;
        SkVector  fHeading;
        float     fScale;
        SkVector  fVelocity;
        float     fSpin;
        SkColor4f fColor;
        float     fFrame;
    };
    EffectState fState;

    SkParticles             fParticles;
    SkAutoTMalloc<SkRandom> fStableRandoms;

    // Cached
    int fCapacity;
};

#endif // SkParticleEffect_DEFINED
