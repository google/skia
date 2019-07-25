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

protected:
    SkString fName;
};

class SkParticleEffectParams : public SkRefCnt {
public:
    SkParticleEffectParams();

    int       fMaxCount;
    float     fEffectDuration;
    float     fRate;

    // Drawable (image, sprite sheet, etc.)
    sk_sp<SkParticleDrawable> fDrawable;

    // Code to configure particles at spawn time
    SkString fSpawnCode;

    // Code to update existing particles over their lifetime
    SkString fUpdateCode;

    SkTArray<sk_sp<SkParticleBinding>> fBindings;

    void visitFields(SkFieldVisitor* v);

private:
    friend class SkParticleEffect;

    // Cached
    struct Program {
        std::unique_ptr<SkSL::ByteCode> fByteCode;
        SkTArray<std::unique_ptr<SkParticleExternalValue>> fExternalValues;
    };
    Program fSpawnProgram;
    Program fUpdateProgram;

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
