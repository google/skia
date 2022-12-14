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
#include "modules/particles/include/SkParticleData.h"

#include <memory>
#include <vector>

class SkCanvas;
class SkFieldVisitor;
class SkParticleBinding;
class SkParticleDrawable;
struct SkParticleProgram;

namespace skresources {
    class ResourceProvider;
}  // namespace skresources

namespace SkSL {
    class ExternalFunction;
    struct UniformInfo;
}  // namespace SkSL

class SkParticleEffectParams : public SkRefCnt {
public:
    SkParticleEffectParams();

    // Maximum number of particles per instance of the effect
    int   fMaxCount;

    // What is drawn for each particle? (Image, shape, sprite sheet, etc.)
    // See SkParticleDrawable::Make*
    sk_sp<SkParticleDrawable> fDrawable;

    // Particle behavior is driven by SkSL code. Effect functions get a mutable Effect struct:
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
    //   float  seed  = 0;               // Random value, used with rand() (see below)
    // };
    //
    // Particle functions get a mutable Particle struct, as well as a uniform copy of the current
    // Effect, named 'effect'.
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
    //   float  seed;
    // };
    //
    // All functions have access to a global function named 'rand'. It takes a float seed value,
    // which it uses and updates (using a PRNG). It returns a random floating point value in [0, 1].
    // Typical usage is to pass the particle or effect's seed value to rand.
    // For particle functions, the seed is rewound after each update, so calls to 'rand(p.seed)'
    // will return consistent values from one update to the next.
    //
    // Finally, there is one global uniform values available, 'dt'. This is a floating point
    // number of seconds that have elapsed since the last update.
    //
    // There are four functions that can be defined in fCode:
    //
    // 'void effectSpawn(inout Effect e)' is called when an instance of the effect is first
    // created, and again at every loop point (if the effect is played with the looping flag).
    //
    // 'void effectUpdate(inout Effect e)' is called once per update to adjust properties of the
    // effect (ie emitter).
    //
    // 'void spawn(inout Particle p)' is called once for each particle when it is first created,
    // to set initial values. At a minimum, this should set 'lifetime' to the number of seconds
    // that the particle will exist. Other parameters will get default values from the effect.
    //
    // 'void update(inout Particle p)' is called for each particle on every call to the running
    // SkParticleEffect's update() method. It can animate any of the particle's values. Note that
    // the 'lifetime' field has a different meaning in 'update', and should not be used or changed.

    SkString fCode;

    // External objects accessible by the effect's SkSL code. Each binding is a name and particular
    // kind of object. See SkParticleBinding::Make* for details.
    SkTArray<sk_sp<SkParticleBinding>> fBindings;

    void visitFields(SkFieldVisitor* v);

    // Load/compute cached resources
    void prepare(const skresources::ResourceProvider*);

private:
    friend class SkParticleEffect;

    std::unique_ptr<SkParticleProgram> fProgram;
};

class SkParticleEffect : public SkRefCnt {
public:
    SkParticleEffect(sk_sp<SkParticleEffectParams> params);

    // Start playing this effect, specifying initial values for the emitter's properties
    void start(double now, bool looping, SkPoint position, SkVector heading, float scale,
               SkVector velocity, float spin, SkColor4f color, float frame, float seed);

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
                    0.0f);                       // seed
    }

    void update(double now);
    void draw(SkCanvas* canvas);

    bool isAlive() const { return (fState.fAge >= 0 && fState.fAge <= 1); }
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

    void setRate    (float     r) { fState.fRate     = r; }
    void setBurst   (int       b) { fState.fBurst    = b; }
    void setPosition(SkPoint   p) { fState.fPosition = p; }
    void setHeading (SkVector  h) { fState.fHeading  = h; }
    void setScale   (float     s) { fState.fScale    = s; }
    void setVelocity(SkVector  v) { fState.fVelocity = v; }
    void setSpin    (float     s) { fState.fSpin     = s; }
    void setColor   (SkColor4f c) { fState.fColor    = c; }
    void setFrame   (float     f) { fState.fFrame    = f; }

    const SkSL::UniformInfo* uniformInfo() const;
    float* uniformData() { return fUniforms.data(); }

    // Sets named uniform to the data in 'val'. 'count' must be equal to the total number of floats
    // in the uniform (eg, the number of elements in a vector). Returns false if the uniform isn't
    // found, or if count is incorrect. Returns true if the value is changed successfully.
    bool setUniform(const char* name, const float* val, int count);

    static void RegisterParticleTypes();

private:
    void setCapacity(int capacity);
    void updateStorage();

    // Helpers to break down update
    void advanceTime(double now);

    enum class EntryPoint {
        kSpawn,
        kUpdate,
    };

    void runEffectScript(EntryPoint entryPoint);
    void runParticleScript(EntryPoint entryPoint, int start, int count);

    sk_sp<SkParticleEffectParams> fParams;

    bool   fLooping;
    int    fCount;
    double fLastTime;
    float  fSpawnRemainder;

    // C++ version of the SkSL Effect struct. This is the inout parameter to per-effect scripts,
    // and provided as a uniform (named 'effect') to all scripts.
    struct EffectState {
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
        float     fRandom;
    };
    EffectState fState;

    SkParticles          fParticles;
    SkAutoTMalloc<float> fStableRandoms;

    // Cached
    int fCapacity = 0;
    SkTArray<float, true> fUniforms;

    friend struct SkParticleProgram;
};

#endif // SkParticleEffect_DEFINED
