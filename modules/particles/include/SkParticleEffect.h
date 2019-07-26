/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkParticleEffect_DEFINED
#define SkParticleEffect_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTemplates.h"
#include "include/utils/SkRandom.h"
#include "modules/particles/include/SkParticleData.h"
#include "modules/particles/include/SkReflected.h"

#include <memory>

class SkCanvas;
struct SkCurve;
struct SkColorCurve;
class SkParticleDrawable;
class SkParticleExternalValue;

namespace SkSL {
    struct ByteCode;
    class Compiler;
}

class SkParticleBinding : public SkReflected {
public:
    SkParticleBinding(const char* name = "name") : fName(name) {}

    REFLECTED_ABSTRACT(SkParticleBinding, SkReflected)

    void visitFields(SkFieldVisitor* v) override;
    virtual std::unique_ptr<SkParticleExternalValue> toValue(SkSL::Compiler&) = 0;

    static void RegisterBindingTypes();

    /*
     * All SkParticleBinding objects expose a particular native object to an effect's SkSL code.
     * In all cases, the 'name' is the symbol that will be used to access the object from the SkSL.
     * Each binding is a callable object, so the SkSL name behaves like a function. The behavior of
     * each kind of binding is described below.
     */

    // Binds an SkCurve to an effect's SkSL. The curve is a one-dimensional function, described
    // in SkCurve.h. It is called in the SkSL as 'name(t)', and returns a single float value.
    static sk_sp<SkParticleBinding> MakeCurve(const char* name, const SkCurve& curve);

    // Binds an SkColorCurve to an effect's SkSL. The curve is a one-dimensional, function,
    // described in SkCurve.h. It is called in the SkSL as 'name(t)', and returns a float4 value.
    static sk_sp<SkParticleBinding> MakeColorCurve(const char* name, const SkColorCurve& curve);

    // Binds an SkPath to an effect's SkSL. The path is specified using SVG syntax. It is called
    // in the SkSL as 'name(t)'. 't' is a normalized distance along the path. This returns a float4
    // value, containing the position in .xy, and the normal in .zw.
    static sk_sp<SkParticleBinding> MakePathBinding(const char* name, const char* path);

protected:
    SkString fName;
};

class SkParticleEffectParams : public SkRefCnt {
public:
    SkParticleEffectParams();

    int   fMaxCount;        // Maximum number of particles per instance of the effect
    float fEffectDuration;  // How long does the effect last after being played, in seconds?
    float fRate;            // How many particles are emitted per second?

    // What is drawn for each particle? (Image, shape, sprite sheet, etc.)
    // See SkParticleDrawable::Make*
    sk_sp<SkParticleDrawable> fDrawable;

    // Particle behavior is driven by two SkSL functions defined in the fCode string.
    // Both functions get a mutable Particle struct:
    //
    // struct Particle {
    //   float  age;
    //   float  lifetime;
    //   float2 pos   = { 0, 0 };        // Local position, relative to the effect.
    //   float2 dir   = { 0, -1 };       // Heading. Should be a normalized vector.
    //   float  scale = 1;               // Size, normalized relative to the drawable's native size
    //   float2 vel   = { 0, 0 };        // Linear velocity, in (units / second)
    //   float  spin  = 0;               // Angular velocity, in (radians / second)
    //   float4 color = { 1, 1, 1, 1 };  // RGBA color
    //   float  frame = 0;               // Normalized sprite index for multi-frame drawables
    // };
    //
    // In addition, both functions have access to a global variable named 'rand'. Every read of
    // 'rand' returns a random floating point value in [0, 1). The random generator is stored
    // per-particle, and the state is rewound after each update, so calls to 'rand' will return
    // consistent values from one update to the next.
    //
    // Finally, there are two global uniform values available. The first is 'dt', a floating point
    // number of seconds that have elapsed since the last update. The second is 'effectAge', which
    // is the normalized age of the effect (not particle). For looping effects, this will wrap
    // back to zero when the effect's age exceeds its duration.
    //
    // 'void spawn(inout Particle p)' is called once for each particle when it is first created,
    // to set initial values. At a minimum, this should set 'lifetime' to the number of seconds
    // that the particle will exist. Other parameters have defaults shown above.
    //
    // 'void update(inout Particle p)' is called for each particle on every call to the running
    // SkParticleEffect's update() method. It can animate any of the particle's values. Note that
    // the 'lifetime' field has a different meaning in 'update', and should not be used or changed.
    SkString fCode;

    // External objects accessible by the effect's SkSL code. Each binding is a name and particular
    // kind of object. See SkParticleBinding::Make* for details.
    SkTArray<sk_sp<SkParticleBinding>> fBindings;

    void visitFields(SkFieldVisitor* v);

private:
    friend class SkParticleEffect;

    // Cached
    std::unique_ptr<SkSL::ByteCode> fByteCode;
    SkTArray<std::unique_ptr<SkParticleExternalValue>> fExternalValues;

    void rebuild();
};

class SkParticleEffect : public SkRefCnt {
public:
    SkParticleEffect(sk_sp<SkParticleEffectParams> params, const SkRandom& random);

    void start(double now, bool looping = false);
    void update(double now);
    void draw(SkCanvas* canvas);

    bool isAlive() const { return fSpawnTime >= 0; }
    int getCount() const { return fCount; }

private:
    void setCapacity(int capacity);

    sk_sp<SkParticleEffectParams> fParams;

    SkRandom fRandom;

    bool   fLooping;
    double fSpawnTime;

    int    fCount;
    double fLastTime;
    float  fSpawnRemainder;

    SkParticles             fParticles;
    SkAutoTMalloc<SkRandom> fStableRandoms;

    // Cached
    int fCapacity;
};

#endif // SkParticleEffect_DEFINED
