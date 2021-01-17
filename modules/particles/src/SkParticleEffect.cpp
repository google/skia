/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "modules/particles/include/SkParticleEffect.h"

#include "include/core/SkPaint.h"
#include "include/private/SkTPin.h"
#include "modules/particles/include/SkParticleBinding.h"
#include "modules/particles/include/SkParticleDrawable.h"
#include "modules/particles/include/SkReflected.h"
#include "modules/skresources/include/SkResources.h"
#include "src/core/SkPaintPriv.h"
#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLUtil.h"

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
  float  seed;
};

uniform float dt;

// We use a not-very-random pure-float PRNG. It does have nice properties for our situation:
// It's fast-ish. Importantly, it only uses types and operations that exist in public SkSL's
// minimum spec (no bitwise operations on integers).
float rand(inout float seed) {
  seed = sin(31*seed) + sin(19*seed + 1);
  return fract(abs(10*seed));
}
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
  float  seed;
};

uniform Effect effect;
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
        , fParticleCode(kDefaultParticleCode) {}

void SkParticleEffectParams::visitFields(SkFieldVisitor* v) {
    v->visit("MaxCount", fMaxCount);
    v->visit("Drawable", fDrawable);
    v->visit("EffectCode", fEffectCode);
    v->visit("Code", fParticleCode);
    v->visit("Bindings", fBindings);
}

void SkParticleEffectParams::prepare(const skresources::ResourceProvider* resourceProvider) {
    for (auto& binding : fBindings) {
        if (binding) {
            binding->prepare(resourceProvider);
        }
    }
    if (fDrawable) {
        fDrawable->prepare(resourceProvider);
    }

    auto buildProgram = [this](const SkSL::String& code, Program* p) {
        SkSL::ShaderCapsPointer caps = SkSL::ShaderCapsFactory::Standalone();
        SkSL::Compiler compiler(caps.get());
        SkSL::Program::Settings settings;
        settings.fRemoveDeadFunctions = false;

        std::vector<std::unique_ptr<SkSL::ExternalFunction>> externalValues;
        externalValues.reserve(fBindings.size());

        for (const auto& binding : fBindings) {
            if (binding) {
                externalValues.push_back(binding->toValue(compiler));
            }
        }

        auto program = compiler.convertProgram(SkSL::Program::kGeneric_Kind, code, settings,
                                               &externalValues);
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

SkParticleEffect::SkParticleEffect(sk_sp<SkParticleEffectParams> params)
        : fParams(std::move(params))
        , fLooping(false)
        , fCount(0)
        , fLastTime(-1.0)
        , fSpawnRemainder(0.0f) {
    fState.fAge = -1.0f;
    this->setCapacity(fParams->fMaxCount);
}

void SkParticleEffect::start(double now, bool looping, SkPoint position, SkVector heading,
                             float scale, SkVector velocity, float spin, SkColor4f color,
                             float frame, float seed) {
    fCount = 0;
    fLastTime = now;
    fSpawnRemainder = 0.0f;
    fLooping = looping;

    fState.fAge = 0.0f;

    // A default lifetime makes sense - many effects are simple loops that don't really care.
    // Every effect should define its own rate of emission, or only use bursts, so leave that as
    // zero initially.
    fState.fLifetime = 1.0f;
    fState.fLoopCount = 0;
    fState.fRate = 0.0f;
    fState.fBurst = 0;

    fState.fPosition = position;
    fState.fHeading  = heading;
    fState.fScale    = scale;
    fState.fVelocity = velocity;
    fState.fSpin     = spin;
    fState.fColor    = color;
    fState.fFrame    = frame;
    fState.fRandom   = seed;

    // Defer running effectSpawn until the first update (to reuse the code when looping)
}

// Just the update step from our "rand" function
static float advance_seed(float x) {
    return sinf(31*x) + sinf(19*x + 1);
}

void SkParticleEffect::runEffectScript(const char* entry) {
    if (const auto& byteCode = fParams->fEffectProgram.fByteCode) {
        if (auto fun = byteCode->getFunction(entry)) {
            SkAssertResult(byteCode->run(fun, &fState.fAge, sizeof(EffectState) / sizeof(float),
                                         nullptr, 0,
                                         fEffectUniforms.data(), fEffectUniforms.count()));
        }
    }
}

void SkParticleEffect::runParticleScript(const char* entry, int start, int count) {
    if (const auto& byteCode = fParams->fParticleProgram.fByteCode) {
        if (auto fun = byteCode->getFunction(entry)) {
            float* args[SkParticles::kNumChannels];
            for (int i = 0; i < SkParticles::kNumChannels; ++i) {
                args[i] = fParticles.fData[i].get() + start;
            }
            memcpy(&fParticleUniforms[1], &fState.fAge, sizeof(EffectState));
            SkAssertResult(byteCode->runStriped(fun, count, args, SkParticles::kNumChannels,
                                                nullptr, 0,
                                                fParticleUniforms.data(),
                                                fParticleUniforms.count()));
        }
    }
}

void SkParticleEffect::advanceTime(double now) {
    // TODO: Sub-frame spawning. Tricky with script driven position. Supply variable effect.age?
    // Could be done if effect.age were an external value that offset by particle lane, perhaps.
    float deltaTime = static_cast<float>(now - fLastTime);
    if (deltaTime <= 0.0f) {
        return;
    }
    fLastTime = now;

    // Handle user edits to fMaxCount
    if (fParams->fMaxCount != fCapacity) {
        this->setCapacity(fParams->fMaxCount);
    }

    // Ensure our storage block for uniforms are large enough
    auto resizeWithZero = [](SkTArray<float, true>* uniforms, const SkSL::ByteCode* byteCode) {
        if (byteCode) {
            int newCount = byteCode->getUniformSlotCount();
            if (newCount > uniforms->count()) {
                uniforms->push_back_n(newCount - uniforms->count(), 0.0f);
            } else {
                uniforms->resize(newCount);
            }
        }
    };
    resizeWithZero(&fEffectUniforms, this->effectCode());
    resizeWithZero(&fParticleUniforms, this->particleCode());

    // Copy known values into the uniform blocks
    SkASSERT(!this->effectCode() || this->effectCode()->getUniformLocation("dt") == 0);
    SkASSERT(!this->particleCode() || this->particleCode()->getUniformLocation("dt") == 0);
    SkASSERT(!this->particleCode() || this->particleCode()->getUniformLocation("effect.age") == 1);
    if (this->effectCode()) {
        fEffectUniforms[0] = deltaTime;
    }
    if (this->particleCode()) {
        fParticleUniforms[0] = deltaTime;
    }

    // Is this the first update after calling start()?
    // Run 'effectSpawn' to set initial emitter properties.
    if (fState.fAge == 0.0f && fState.fLoopCount == 0) {
        this->runEffectScript("effectSpawn");
    }

    fState.fAge += deltaTime / fState.fLifetime;
    if (fState.fAge > 1) {
        // We always run effectDeath when age crosses 1, whether we're looping or actually dying
        this->runEffectScript("effectDeath");

        if (fLooping) {
            // If we looped, then run effectSpawn again (with the updated loop count)
            fState.fLoopCount += sk_float_floor2int(fState.fAge);
            fState.fAge = fmodf(fState.fAge, 1.0f);
            this->runEffectScript("effectSpawn");
        } else {
            // Effect is dead if we've reached the end (and are not looping)
            return;
        }
    }

    // Advance age for existing particles, shuffle all dying particles to the end of the arrays
    int numDyingParticles = 0;
    for (int i = 0; i < fCount; ++i) {
        fParticles.fData[SkParticles::kAge][i] +=
                fParticles.fData[SkParticles::kLifetime][i] * deltaTime;
        if (fParticles.fData[SkParticles::kAge][i] > 1.0f) {
            // NOTE: This is fast, but doesn't preserve drawing order. Could be a problem...
            for (int j = 0; j < SkParticles::kNumChannels; ++j) {
                std::swap(fParticles.fData[j][i], fParticles.fData[j][fCount - 1]);
            }
            std::swap(fStableRandoms[i], fStableRandoms[fCount - 1]);
            --i;
            --fCount;
            ++numDyingParticles;
        }
    }

    // Run the death script for all particles that just died
    this->runParticleScript("death", fCount, numDyingParticles);

    // Run 'effectUpdate' to adjust emitter properties
    this->runEffectScript("effectUpdate");

    // Do integration of effect position and orientation
    {
        fState.fPosition += fState.fVelocity * deltaTime;
        float s = sk_float_sin(fState.fSpin * deltaTime),
              c = sk_float_cos(fState.fSpin * deltaTime);
        // Using setNormalize to prevent scale drift
        fState.fHeading.setNormalize(fState.fHeading.fX * c - fState.fHeading.fY * s,
                                     fState.fHeading.fX * s + fState.fHeading.fY * c);
    }

    // Spawn new particles
    float desired = fState.fRate * deltaTime + fSpawnRemainder + fState.fBurst;
    fState.fBurst = 0;
    int numToSpawn = sk_float_round2int(desired);
    fSpawnRemainder = desired - numToSpawn;
    numToSpawn = SkTPin(numToSpawn, 0, fParams->fMaxCount - fCount);
    if (numToSpawn) {
        const int spawnBase = fCount;

        for (int i = 0; i < numToSpawn; ++i) {
            // Mutate our random seed so each particle definitely gets a different generator
            fState.fRandom = advance_seed(fState.fRandom);
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
            fParticles.fData[SkParticles::kRandom         ][fCount] = fState.fRandom;
            fCount++;
        }

        // Run the spawn script
        this->runParticleScript("spawn", spawnBase, numToSpawn);

        // Now stash copies of the random seeds and compute inverse particle lifetimes
        // (so that subsequent updates are faster)
        for (int i = spawnBase; i < fCount; ++i) {
            fParticles.fData[SkParticles::kLifetime][i] =
                    sk_ieee_float_divide(1.0f, fParticles.fData[SkParticles::kLifetime][i]);
            fStableRandoms[i] = fParticles.fData[SkParticles::kRandom][i];
        }
    }

    // Restore all stable random seeds so update scripts get consistent behavior each frame
    for (int i = 0; i < fCount; ++i) {
        fParticles.fData[SkParticles::kRandom][i] = fStableRandoms[i];
    }

    // Run the update script
    this->runParticleScript("update", 0, fCount);

    // Do fixed-function update work (integration of position and orientation)
    for (int i = 0; i < fCount; ++i) {
        fParticles.fData[SkParticles::kPositionX][i] +=
                fParticles.fData[SkParticles::kVelocityX][i] * deltaTime;
        fParticles.fData[SkParticles::kPositionY][i] +=
                fParticles.fData[SkParticles::kVelocityY][i] * deltaTime;

        float spin = fParticles.fData[SkParticles::kVelocityAngular][i];
        float s = sk_float_sin(spin * deltaTime),
              c = sk_float_cos(spin * deltaTime);
        float oldHeadingX = fParticles.fData[SkParticles::kHeadingX][i],
              oldHeadingY = fParticles.fData[SkParticles::kHeadingY][i];
        fParticles.fData[SkParticles::kHeadingX][i] = oldHeadingX * c - oldHeadingY * s;
        fParticles.fData[SkParticles::kHeadingY][i] = oldHeadingX * s + oldHeadingY * c;
    }
}

void SkParticleEffect::update(double now) {
    if (this->isAlive()) {
        this->advanceTime(now);
    }
}

void SkParticleEffect::draw(SkCanvas* canvas) {
    if (this->isAlive() && fParams->fDrawable) {
        SkPaint paint;
        SkPaintPriv::SetFQ(&paint, SkFilterQuality::kMedium_SkFilterQuality);
        fParams->fDrawable->draw(canvas, fParticles, fCount, paint);
    }
}

void SkParticleEffect::setCapacity(int capacity) {
    for (int i = 0; i < SkParticles::kNumChannels; ++i) {
        fParticles.fData[i].realloc(capacity);
    }
    fStableRandoms.realloc(capacity);

    fCapacity = capacity;
    fCount = std::min(fCount, fCapacity);
}

void SkParticleEffect::RegisterParticleTypes() {
    REGISTER_REFLECTED(SkReflected);
    SkParticleBinding::RegisterBindingTypes();
    SkParticleDrawable::RegisterDrawableTypes();
}
