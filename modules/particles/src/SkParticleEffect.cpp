/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "modules/particles/include/SkParticleEffect.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRSXform.h"
#include "include/private/SkColorData.h"
#include "modules/particles/include/SkParticleDrawable.h"
#include "modules/particles/include/SkReflected.h"
#include "src/core/SkMakeUnique.h"
#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLExternalValue.h"

// Exposes a particle's random generator as an external, readable value. read returns a float [0, 1)
class SkRandomExternalValue : public SkSL::ExternalValue {
public:
    SkRandomExternalValue(const char* name, SkSL::Compiler& compiler)
        : INHERITED(name, *compiler.context().fFloat_Type)
        , fRandom(nullptr) {
    }

    void setRandom(SkRandom* random) { fRandom = random; }
    bool canRead() const override { return true; }
    void read(int index, float* target) override {
        *target = fRandom[index].nextF();
    }

private:
    SkRandom* fRandom;
    typedef SkSL::ExternalValue INHERITED;
};

static const char* kDefaultCode =
R"(
// float rand; Every read returns a random float [0 .. 1)
layout(ctype=float) in uniform float dt;
layout(ctype=float) in uniform float effectAge;

struct Particle {
  float  age;
  float  lifetime;
  float2 pos;
  float2 dir;
  float  scale;
  float2 vel;
  float  spin;
  float4 color;
  float  frame;
}

void main(inout Particle p) {
}
)";

SkParticleEffectParams::SkParticleEffectParams()
        : fMaxCount(128)
        , fEffectDuration(1.0f)
        , fRate(8.0f)
        , fDrawable(nullptr)
        , fSpawnCode(kDefaultCode)
        , fUpdateCode(kDefaultCode) {
    this->rebuild();
}

void SkParticleEffectParams::visitFields(SkFieldVisitor* v) {
    SkString oldSpawnCode = fSpawnCode;
    SkString oldUpdateCode = fUpdateCode;

    v->visit("MaxCount", fMaxCount);
    v->visit("Duration", fEffectDuration);
    v->visit("Rate", fRate);

    v->visit("Drawable", fDrawable);

    v->visit("Spawn", fSpawnCode);
    v->visit("Update", fUpdateCode);

    if (fSpawnCode != oldSpawnCode || fUpdateCode != oldUpdateCode) {
        this->rebuild();
    }
}

void SkParticleEffectParams::rebuild() {
    auto buildProgram = [this](Program* p, const SkString& code) {
        SkSL::Compiler compiler;
        SkSL::Program::Settings settings;

        auto rand = skstd::make_unique<SkRandomExternalValue>("rand", compiler);
        compiler.registerExternalValue(rand.get());
        auto program = compiler.convertProgram(SkSL::Program::kGeneric_Kind,
                                               SkSL::String(fSpawnCode.c_str()), settings);
        if (!program) {
            SkDebugf("%s\n", compiler.errorText().c_str());
            return;
        }

        auto byteCode = compiler.toByteCode(*program);
        if (!byteCode) {
            SkDebugf("%s\n", compiler.errorText().c_str());
            return;
        }

        p->fByteCode = std::move(byteCode);
        p->fRandomValue = std::move(rand);
    };

    buildProgram(&fSpawnProgram, fSpawnCode);
    buildProgram(&fUpdateProgram, fUpdateCode);
}

SkParticleEffect::SkParticleEffect(sk_sp<SkParticleEffectParams> params, const SkRandom& random)
        : fParams(std::move(params))
        , fRandom(random)
        , fLooping(false)
        , fSpawnTime(-1.0)
        , fCount(0)
        , fLastTime(-1.0)
        , fSpawnRemainder(0.0f) {
    this->setCapacity(fParams->fMaxCount);
}

void SkParticleEffect::start(double now, bool looping) {
    fCount = 0;
    fLastTime = fSpawnTime = now;
    fSpawnRemainder = 0.0f;
    fLooping = looping;
}

void SkParticleEffect::update(double now) {
    if (!this->isAlive() || !fParams->fDrawable) {
        return;
    }

    float deltaTime = static_cast<float>(now - fLastTime);
    if (deltaTime <= 0.0f) {
        return;
    }
    fLastTime = now;

    // Handle user edits to fMaxCount
    if (fParams->fMaxCount != fCapacity) {
        this->setCapacity(fParams->fMaxCount);
    }

    float effectAge = static_cast<float>((now - fSpawnTime) / fParams->fEffectDuration);
    effectAge = fLooping ? fmodf(effectAge, 1.0f) : SkTPin(effectAge, 0.0f, 1.0f);

    float updateParams[2] = { deltaTime, effectAge };

    // Advance age for existing particles, and remove any that have reached their end of life
    for (int i = 0; i < fCount; ++i) {
        fParticles.fData[SkParticles::kAge][i] +=
                fParticles.fData[SkParticles::kLifetime][i] * deltaTime;
        if (fParticles.fData[SkParticles::kAge][i] > 1.0f) {
            // NOTE: This is fast, but doesn't preserve drawing order. Could be a problem...
            for (int j = 0; j < SkParticles::kNumChannels; ++j) {
                fParticles.fData[j][i] = fParticles.fData[j][fCount - 1];
            }
            fStableRandoms[i] = fStableRandoms[fCount - 1];
            --i;
            --fCount;
        }
    }

    // Spawn new particles
    float desired = fParams->fRate * deltaTime + fSpawnRemainder;
    int numToSpawn = sk_float_round2int(desired);
    fSpawnRemainder = desired - numToSpawn;
    numToSpawn = SkTPin(numToSpawn, 0, fParams->fMaxCount - fCount);
    if (numToSpawn) {
        const int spawnBase = fCount;

        for (int i = 0; i < numToSpawn; ++i) {
            // Mutate our SkRandom so each particle definitely gets a different generator
            fRandom.nextU();
            fParticles.fData[SkParticles::kAge            ][fCount] = 0.0f;
            fParticles.fData[SkParticles::kLifetime       ][fCount] = 0.0f;
            fParticles.fData[SkParticles::kPositionX      ][fCount] = 0.0f;
            fParticles.fData[SkParticles::kPositionY      ][fCount] = 0.0f;
            fParticles.fData[SkParticles::kHeadingX       ][fCount] = 0.0f;
            fParticles.fData[SkParticles::kHeadingY       ][fCount] = -1.0f;
            fParticles.fData[SkParticles::kScale          ][fCount] = 1.0f;
            fParticles.fData[SkParticles::kVelocityX      ][fCount] = 0.0f;
            fParticles.fData[SkParticles::kVelocityY      ][fCount] = 0.0f;
            fParticles.fData[SkParticles::kVelocityAngular][fCount] = 0.0f;
            fParticles.fData[SkParticles::kColorR         ][fCount] = 1.0f;
            fParticles.fData[SkParticles::kColorG         ][fCount] = 1.0f;
            fParticles.fData[SkParticles::kColorB         ][fCount] = 1.0f;
            fParticles.fData[SkParticles::kColorA         ][fCount] = 1.0f;
            fParticles.fData[SkParticles::kSpriteFrame    ][fCount] = 0.0f;
            fParticles.fRandom[fCount] = fRandom;
            fCount++;
        }

        // Run the spawn script
        if (const auto& byteCode = fParams->fSpawnProgram.fByteCode) {
            float* args[SkParticles::kNumChannels];
            for (int i = 0; i < SkParticles::kNumChannels; ++i) {
                args[i] = fParticles.fData[i].get() + spawnBase;
            }
            fParams->fSpawnProgram.fRandomValue->setRandom(fParticles.fRandom.get() + spawnBase);
            byteCode->runStriped(byteCode->getFunction("main"), args, SkParticles::kNumChannels,
                                 numToSpawn, updateParams, 2, nullptr, 0);
        }

        // Now stash copies of the random generators and compute inverse particle lifetimes
        // (so that subsequent updates are faster)
        for (int i = spawnBase; i < fCount; ++i) {
            fParticles.fData[SkParticles::kLifetime][i] =
                    sk_ieee_float_divide(1.0f, fParticles.fData[SkParticles::kLifetime][i]);
            fStableRandoms[i] = fParticles.fRandom[i];
        }
    }

    // Restore all stable random generators so update affectors get consistent behavior each frame
    for (int i = 0; i < fCount; ++i) {
        fParticles.fRandom[i] = fStableRandoms[i];
    }

    // Run the update script
    if (const auto& byteCode = fParams->fUpdateProgram.fByteCode) {
        float* args[SkParticles::kNumChannels];
        for (int i = 0; i < SkParticles::kNumChannels; ++i) {
            args[i] = fParticles.fData[i].get();
        }
        fParams->fUpdateProgram.fRandomValue->setRandom(fParticles.fRandom.get());
        byteCode->runStriped(byteCode->getFunction("main"), args, SkParticles::kNumChannels,
                             fCount, updateParams, 2, nullptr, 0);
    }

    // Do fixed-function update work (integration of position and orientation)
    for (int i = 0; i < fCount; ++i) {
        fParticles.fData[SkParticles::kPositionX][i] +=
                fParticles.fData[SkParticles::kVelocityX][i] * deltaTime;
        fParticles.fData[SkParticles::kPositionY][i] +=
                fParticles.fData[SkParticles::kVelocityY][i] * deltaTime;

        SkScalar s = SkScalarSin(fParticles.fData[SkParticles::kVelocityAngular][i] * deltaTime),
                 c = SkScalarCos(fParticles.fData[SkParticles::kVelocityAngular][i] * deltaTime);
        float oldHeadingX = fParticles.fData[SkParticles::kHeadingX][i],
              oldHeadingY = fParticles.fData[SkParticles::kHeadingY][i];
        fParticles.fData[SkParticles::kHeadingX][i] = oldHeadingX * c - oldHeadingY * s;
        fParticles.fData[SkParticles::kHeadingY][i] = oldHeadingX * s + oldHeadingY * c;
    }

    // Mark effect as dead if we've reached the end (and are not looping)
    if (!fLooping && (now - fSpawnTime) > fParams->fEffectDuration) {
        fSpawnTime = -1.0;
    }
}

void SkParticleEffect::draw(SkCanvas* canvas) {
    if (this->isAlive() && fParams->fDrawable) {
        SkPaint paint;
        paint.setFilterQuality(SkFilterQuality::kMedium_SkFilterQuality);
        fParams->fDrawable->draw(canvas, fParticles, fCount, &paint);
    }
}

void SkParticleEffect::setCapacity(int capacity) {
    for (int i = 0; i < SkParticles::kNumChannels; ++i) {
        fParticles.fData[i].realloc(capacity);
    }
    fParticles.fRandom.realloc(capacity);
    fStableRandoms.realloc(capacity);

    fCapacity = capacity;
    fCount = SkTMin(fCount, fCapacity);
}
