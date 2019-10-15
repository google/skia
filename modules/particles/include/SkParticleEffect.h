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
    class ByteCode;
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
    //   float age;       // Normalized age of the effect
    //   float lifetime;  // Effect's duration, in seconds - script should set this in effectSpawn
    //   int   loop;      // Number of loops that have elapsed (0 on initial spawn)
    //   float rate;      // Rate to generate new particles (particles / second)
    //   int   burst;     // Number of particles to emit in a single update
    //                    // Set during spawn to emit that many at once on each loop
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
    // that the particle will exist. Other parameters will get default values from the effect.
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

    // Start playing this effect, specifying initial values for the emitter's properties
    void start(double now, bool looping, SkPoint position, SkVector heading, float scale,
               SkVector velocity, float spin, SkColor4f color, float frame, uint32_t flags);

    // Start playing this effect, with default values for the emitter's properties
    void start(double now, bool looping) {
        this->start(now, looping,
                    { 0.0f, 0.0f },              // position
                    { 0.0f, -1.0f },             // heading
                    1.0f,                        // scale
                    { 0.0f, 0.0f },              // velocity
                    0.0f,                        // spin
                    { 1.0f, 1.0f, 1.0f, 1.0f },  // color
                    0.0f,                        // sprite frame
                    0);                          // flags
    }

    void update(double now);
    void draw(SkCanvas* canvas);

    bool isAlive(bool includeSubEffects = true) const {
        return (fState.fAge >= 0 && fState.fAge <= 1)
            || (includeSubEffects && !fSubEffects.empty());
    }
    int getCount() const { return fCount; }

    float     getRate()     const { return fState.fRate;     }
    int       getBurst()    const { return fState.fBurst;    }
    SkPoint   getPosition() const { return fState.fPosition; }
    SkVector  getHeading()  const { return fState.fHeading;  }
    float     getScale()    const { return fState.fScale;    }
    SkVector  getVelocity() const { return fState.fVelocity; }
    float     getSpin()     const { return fState.fSpin;     }
    SkColor4f getColor()    const { return fState.fColor;    }
    float     getFrame()    const { return fState.fFrame;    }
    uint32_t  getFlags()    const { return fState.fFlags;    }

    void setRate    (float     r) { fState.fRate     = r; }
    void setBurst   (int       b) { fState.fBurst    = b; }
    void setPosition(SkPoint   p) { fState.fPosition = p; }
    void setHeading (SkVector  h) { fState.fHeading  = h; }
    void setScale   (float     s) { fState.fScale    = s; }
    void setVelocity(SkVector  v) { fState.fVelocity = v; }
    void setSpin    (float     s) { fState.fSpin     = s; }
    void setColor   (SkColor4f c) { fState.fColor    = c; }
    void setFrame   (float     f) { fState.fFrame    = f; }
    void setFlags   (uint32_t  f) { fState.fFlags    = f; }

    static void RegisterParticleTypes();

private:
    void setCapacity(int capacity);

    // Helpers to break down update
    void advanceTime(double now);

    void processEffectSpawnRequests(double now);
    void runEffectScript(double now, const char* entry);

    void processParticleSpawnRequests(double now, int start);
    void runParticleScript(double now, const char* entry, int start, int count);

    sk_sp<SkParticleEffectParams> fParams;

    SkRandom fRandom;

    bool   fLooping;
    int    fCount;
    double fLastTime;
    float  fSpawnRemainder;

    // Effect-associated values exposed to script. They are some mix of uniform and inout,
    // depending on whether we're executing per-effect or per-particle scripts.
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
        uint32_t  fFlags;
    };
    EffectState fState;

    SkParticles             fParticles;
    SkAutoTMalloc<SkRandom> fStableRandoms;

    // Cached
    int fCapacity;

    // Private interface used by SkEffectBinding and SkEffectExternalValue to spawn sub effects
    friend class SkEffectExternalValue;
    struct SpawnRequest {
        SpawnRequest(int index, bool loop, sk_sp<SkParticleEffectParams> params)
            : fIndex(index)
            , fLoop(loop)
            , fParams(std::move(params)) {}

        int fIndex;
        bool fLoop;
        sk_sp<SkParticleEffectParams> fParams;
    };
    void addSpawnRequest(int index, bool loop, sk_sp<SkParticleEffectParams> params) {
        fSpawnRequests.emplace_back(index, loop, std::move(params));
    }
    SkTArray<SpawnRequest> fSpawnRequests;

    SkTArray<sk_sp<SkParticleEffect>> fSubEffects;
};

#endif // SkParticleEffect_DEFINED
