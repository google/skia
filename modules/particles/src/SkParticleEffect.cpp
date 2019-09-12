/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "modules/particles/include/SkParticleEffect.h"

#include "include/core/SkPaint.h"
#include "modules/particles/include/SkCurve.h"
#include "modules/particles/include/SkParticleBinding.h"
#include "modules/particles/include/SkParticleDrawable.h"
#include "modules/particles/include/SkReflected.h"
#include "src/core/SkMakeUnique.h"
#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLCompiler.h"

// Exposes a particle's random generator as an external, readable value. read returns a float [0, 1)
class SkRandomExternalValue : public SkParticleExternalValue {
public:
    SkRandomExternalValue(const char* name, SkSL::Compiler& compiler)
        : SkParticleExternalValue(name, compiler, *compiler.context().fFloat_Type) {}

    bool canRead() const override { return true; }
    void read(int index, float* target) override {
        *target = fRandom[index].nextF();
    }
};

static const char* kCommonHeader =
R"(
struct Effect {
  float  age;
  float  lifetime;
  int    loop;
  float  rate;
  int    burst;

  float2 pos;
  float2 dir;
  float  scale;
  float2 vel;
  float  spin;
  float4 color;
  float  frame;
};

in uniform float dt;
)";

static const char* kParticleHeader =
R"(
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
};

in uniform Effect effect;
)";

static const char* kDefaultEffectCode =
R"(void effectSpawn(inout Effect effect) {
}

void effectUpdate(inout Effect effect) {
}
)";

static const char* kDefaultParticleCode =
R"(void spawn(inout Particle p) {
}

void update(inout Particle p) {
}
)";

SkParticleEffectParams::SkParticleEffectParams()
        : fMaxCount(128)
        , fDrawable(nullptr)
        , fEffectCode(kDefaultEffectCode)
        , fParticleCode(kDefaultParticleCode) {
    this->rebuild();
}

void SkParticleEffectParams::visitFields(SkFieldVisitor* v) {
    SkString oldEffectCode = fEffectCode;
    SkString oldParticleCode = fParticleCode;

    v->visit("MaxCount", fMaxCount);

    v->visit("Drawable", fDrawable);

    v->visit("EffectCode", fEffectCode);
    v->visit("Code", fParticleCode);

    v->visit("Bindings", fBindings);

    // TODO: Or, if any change to binding metadata?
    if (fParticleCode != oldParticleCode || fEffectCode != oldEffectCode) {
        this->rebuild();
    }
}

void SkParticleEffectParams::rebuild() {
    auto buildProgram = [this](const SkSL::String& code, Program* p) {
        SkSL::Compiler compiler;
        SkSL::Program::Settings settings;

        SkTArray<std::unique_ptr<SkParticleExternalValue>> externalValues;

        auto rand = skstd::make_unique<SkRandomExternalValue>("rand", compiler);
        compiler.registerExternalValue(rand.get());
        externalValues.push_back(std::move(rand));

        for (const auto& binding : fBindings) {
            if (binding) {
                auto value = binding->toValue(compiler);
                compiler.registerExternalValue(value.get());
                externalValues.push_back(std::move(value));
            }
        }

        auto program = compiler.convertProgram(SkSL::Program::kGeneric_Kind, code, settings);
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
        p->fExternalValues.swap(externalValues);
    };

    SkSL::String effectCode(kCommonHeader);
    effectCode.append(fEffectCode.c_str());

    SkSL::String particleCode(kCommonHeader);
    particleCode.append(kParticleHeader);
    particleCode.append(fParticleCode.c_str());

    buildProgram(effectCode, &fEffectProgram);
    buildProgram(particleCode, &fParticleProgram);
}

SkParticleEffect::SkParticleEffect(sk_sp<SkParticleEffectParams> params, const SkRandom& random)
        : fParams(std::move(params))
        , fRandom(random)
        , fLooping(false)
        , fCount(0)
        , fLastTime(-1.0)
        , fSpawnRemainder(0.0f) {
    fState.fAge = -1.0f;
    this->setCapacity(fParams->fMaxCount);
}

void SkParticleEffect::start(double now, bool looping) {
    fCount = 0;
    fLastTime = now;
    fSpawnRemainder = 0.0f;
    fLooping = looping;

    fState.fDeltaTime = 0.0f;
    fState.fAge = 0.0f;

    // A default lifetime makes sense - many effects are simple loops that don't really care.
    // Every effect should define its own rate of emission, or only use bursts, so leave that as
    // zero initially.
    fState.fLifetime = 1.0f;
    fState.fLoopCount = 0;
    fState.fRate = 0.0f;
    fState.fBurst = 0;

    fState.fPosition = { 0.0f, 0.0f };
    fState.fHeading  = { 0.0f, -1.0f };
    fState.fScale    = 1.0f;
    fState.fVelocity = { 0.0f, 0.0f };
    fState.fSpin     = 0.0f;
    fState.fColor    = { 1.0f, 1.0f, 1.0f, 1.0f };
    fState.fFrame    = 0.0f;

    // Defer running effectSpawn until the first update (to reuse the code when looping)
}

void SkParticleEffect::update(double now) {
    if (!this->isAlive() || !fParams->fDrawable) {
        return;
    }

    // TODO: Sub-frame spawning. Tricky with script driven position. Supply variable effect.age?
    // Could be done if effect.age were an external value that offset by particle lane, perhaps.
    fState.fDeltaTime = static_cast<float>(now - fLastTime);
    if (fState.fDeltaTime <= 0.0f) {
        return;
    }
    fLastTime = now;

    // Handle user edits to fMaxCount
    if (fParams->fMaxCount != fCapacity) {
        this->setCapacity(fParams->fMaxCount);
    }

    bool runEffectSpawn = (fState.fAge == 0.0f) && (fState.fLoopCount == 0);

    fState.fAge += fState.fDeltaTime / fState.fLifetime;
    if (fState.fAge > 1) {
        if (fLooping) {
            fState.fLoopCount += sk_float_floor2int(fState.fAge);
            fState.fAge = fmodf(fState.fAge, 1.0f);
            runEffectSpawn = true;
        } else {
            // Effect is dead if we've reached the end (and are not looping)
            return;
        }
    }

    // Run optional effectSpawn to set initial spawn rate and other emitter properties.
    // This also runs on each loop point, for looped effects.
    if (runEffectSpawn) {
        if (const auto& byteCode = fParams->fEffectProgram.fByteCode) {
            if (auto fun = byteCode->getFunction("effectSpawn")) {
                for (const auto& value : fParams->fEffectProgram.fExternalValues) {
                    value->setRandom(&fRandom);
                }
                SkAssertResult(byteCode->run(fun, &fState.fAge, nullptr, 1,
                                             &fState.fDeltaTime, 1));
            }
        }
    }

    // Advance age for existing particles, and remove any that have reached their end of life
    // TODO: Add an (optional) death script for particles?
    for (int i = 0; i < fCount; ++i) {
        fParticles.fData[SkParticles::kAge][i] +=
                fParticles.fData[SkParticles::kLifetime][i] * fState.fDeltaTime;
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

    // On first frame, we may have a pending burst from effectSpawn
    int burstCount = fState.fBurst;
    fState.fBurst = 0;

    // Run optional effectUpdate to adjust spawn rate and other emitter properties
    if (const auto& byteCode = fParams->fEffectProgram.fByteCode) {
        if (auto fun = byteCode->getFunction("effectUpdate")) {
            for (const auto& value : fParams->fEffectProgram.fExternalValues) {
                value->setRandom(&fRandom);
            }
            SkAssertResult(byteCode->run(fun, &fState.fAge, nullptr, 1,
                                         &fState.fDeltaTime, 1));
            burstCount += fState.fBurst;
        }
    }

    // Do integration of effect position and orientation
    {
        fState.fPosition += fState.fVelocity * fState.fDeltaTime;
        float s = sk_float_sin(fState.fSpin * fState.fDeltaTime),
              c = sk_float_cos(fState.fSpin * fState.fDeltaTime);
        // Using setNormalize to prevent scale drift
        fState.fHeading.setNormalize(fState.fHeading.fX * c - fState.fHeading.fY * s,
                                     fState.fHeading.fX * s + fState.fHeading.fY * c);
    }

    auto runProgram = [this](const SkParticleEffectParams* params, const char* entry,
                             SkParticles& particles, int start, int count) {
        if (const auto& byteCode = params->fParticleProgram.fByteCode) {
            float* args[SkParticles::kNumChannels];
            for (int i = 0; i < SkParticles::kNumChannels; ++i) {
                args[i] = particles.fData[i].get() + start;
            }
            SkRandom* randomBase = particles.fRandom.get() + start;
            for (const auto& value : params->fParticleProgram.fExternalValues) {
                value->setRandom(randomBase);
            }
            SkAssertResult(byteCode->runStriped(byteCode->getFunction(entry),
                                                args, SkParticles::kNumChannels, count,
                                                &fState.fDeltaTime, sizeof(EffectState) / 4,
                                                nullptr, 0));
        }
    };

    // Spawn new particles
    float desired = fState.fRate * fState.fDeltaTime + fSpawnRemainder;
    int numToSpawn = sk_float_round2int(desired);
    fSpawnRemainder = desired - numToSpawn;
    numToSpawn = SkTPin(numToSpawn + burstCount, 0, fParams->fMaxCount - fCount);
    if (numToSpawn) {
        const int spawnBase = fCount;

        for (int i = 0; i < numToSpawn; ++i) {
            // Mutate our SkRandom so each particle definitely gets a different generator
            fRandom.nextU();
            fParticles.fData[SkParticles::kAge            ][fCount] = 0.0f;
            fParticles.fData[SkParticles::kLifetime       ][fCount] = 0.0f;
            fParticles.fData[SkParticles::kPositionX      ][fCount] = fState.fPosition.fX;
            fParticles.fData[SkParticles::kPositionY      ][fCount] = fState.fPosition.fY;
            fParticles.fData[SkParticles::kHeadingX       ][fCount] = fState.fHeading.fX;
            fParticles.fData[SkParticles::kHeadingY       ][fCount] = fState.fHeading.fY;
            fParticles.fData[SkParticles::kScale          ][fCount] = fState.fScale;
            fParticles.fData[SkParticles::kVelocityX      ][fCount] = fState.fVelocity.fX;
            fParticles.fData[SkParticles::kVelocityY      ][fCount] = fState.fVelocity.fY;
            fParticles.fData[SkParticles::kVelocityAngular][fCount] = fState.fSpin;
            fParticles.fData[SkParticles::kColorR         ][fCount] = fState.fColor.fR;
            fParticles.fData[SkParticles::kColorG         ][fCount] = fState.fColor.fG;
            fParticles.fData[SkParticles::kColorB         ][fCount] = fState.fColor.fB;
            fParticles.fData[SkParticles::kColorA         ][fCount] = fState.fColor.fA;
            fParticles.fData[SkParticles::kSpriteFrame    ][fCount] = fState.fFrame;
            fParticles.fRandom[fCount] = fRandom;
            fCount++;
        }

        // Run the spawn script
        runProgram(fParams.get(), "spawn", fParticles, spawnBase, numToSpawn);

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
    runProgram(fParams.get(), "update", fParticles, 0, fCount);

    // Do fixed-function update work (integration of position and orientation)
    for (int i = 0; i < fCount; ++i) {
        fParticles.fData[SkParticles::kPositionX][i] +=
                fParticles.fData[SkParticles::kVelocityX][i] * fState.fDeltaTime;
        fParticles.fData[SkParticles::kPositionY][i] +=
                fParticles.fData[SkParticles::kVelocityY][i] * fState.fDeltaTime;

        float spin = fParticles.fData[SkParticles::kVelocityAngular][i];
        float s = sk_float_sin(spin * fState.fDeltaTime),
              c = sk_float_cos(spin * fState.fDeltaTime);
        float oldHeadingX = fParticles.fData[SkParticles::kHeadingX][i],
              oldHeadingY = fParticles.fData[SkParticles::kHeadingY][i];
        fParticles.fData[SkParticles::kHeadingX][i] = oldHeadingX * c - oldHeadingY * s;
        fParticles.fData[SkParticles::kHeadingY][i] = oldHeadingX * s + oldHeadingY * c;
    }
}

void SkParticleEffect::draw(SkCanvas* canvas) {
    if (this->isAlive() && fParams->fDrawable) {
        SkPaint paint;
        paint.setFilterQuality(SkFilterQuality::kMedium_SkFilterQuality);
        fParams->fDrawable->draw(canvas, fParticles, fCount, paint);
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

void SkParticleEffect::RegisterParticleTypes() {
    REGISTER_REFLECTED(SkReflected);
    SkParticleBinding::RegisterBindingTypes();
    SkParticleDrawable::RegisterDrawableTypes();
}
