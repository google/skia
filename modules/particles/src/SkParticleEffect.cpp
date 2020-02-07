/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "modules/particles/include/SkParticleEffect.h"

#include "include/core/SkPaint.h"
#include "modules/particles/include/SkParticleBinding.h"
#include "modules/particles/include/SkParticleDrawable.h"
#include "modules/particles/include/SkReflected.h"
#include "modules/skresources/include/SkResources.h"
#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLCompiler.h"

static inline float bits_to_float(uint32_t u) {
    float f;
    memcpy(&f, &u, sizeof(uint32_t));
    return f;
}

static inline uint32_t float_to_bits(float f) {
    uint32_t u;
    memcpy(&u, &f, sizeof(uint32_t));
    return u;
}

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
  uint   flags;
};

uniform float dt;
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
  uint   flags;
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

    auto buildProgram = [this](const SkSL::String& code) ->
                                     std::pair<std::unique_ptr<SkSL::ByteCode>,
                                               SkTArray<std::unique_ptr<SkParticleExternalValue>>> {
        SkSL::Compiler compiler;
        SkSL::Program::Settings settings;

        SkTArray<std::unique_ptr<SkParticleExternalValue>> externalValues;

        auto rand = std::make_unique<SkRandomExternalValue>("rand", compiler);
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
            return std::make_pair(nullptr, std::move(externalValues));
        }

        auto byteCode = compiler.toByteCode(*program);
        if (!byteCode) {
            SkDebugf("%s\n", compiler.errorText().c_str());
            return std::make_pair(nullptr, std::move(externalValues));
        }
        return std::make_pair(std::move(byteCode), std::move(externalValues));
    };

    SkSL::String effectCode(kCommonHeader);
    effectCode.append(fEffectCode.c_str());

    SkSL::String particleCode(kCommonHeader);
    particleCode.append(kParticleHeader);
    particleCode.append(fParticleCode.c_str());

    auto effectProgram = buildProgram(effectCode);
    if (effectProgram.first) {
        fEffectProgram.fInterpreter.reset(
                new SkSL::Interpreter<INTERPRETER_WIDTH>(std::move(effectProgram.first)));
        fEffectProgram.fExternalValues.swap(effectProgram.second);
    }

    auto particleProgram = buildProgram(particleCode);
    if (particleProgram.first) {
        fParticleProgram.fInterpreter.reset(
                new SkSL::Interpreter<INTERPRETER_WIDTH>(std::move(particleProgram.first)));
        fParticleProgram.fExternalValues.swap(particleProgram.second);
    }
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

void SkParticleEffect::start(double now, bool looping, SkPoint position, SkVector heading,
                             float scale, SkVector velocity, float spin, SkColor4f color,
                             float frame, uint32_t flags) {
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
    fState.fFlags    = flags;

    // Defer running effectSpawn until the first update (to reuse the code when looping)
}

// Spawns new effects that were requested by any *effect* script (copies default values from
// the current effect state).
void SkParticleEffect::processEffectSpawnRequests(double now) {
    for (const auto& spawnReq : fSpawnRequests) {
        sk_sp<SkParticleEffect> newEffect(new SkParticleEffect(std::move(spawnReq.fParams),
                                                               fRandom));
        fRandom.nextU();

        newEffect->start(now, spawnReq.fLoop, fState.fPosition, fState.fHeading, fState.fScale,
                         fState.fVelocity, fState.fSpin, fState.fColor, fState.fFrame,
                         fState.fFlags);
        fSubEffects.push_back(std::move(newEffect));
    }
    fSpawnRequests.reset();
}

void SkParticleEffect::runEffectScript(double now, const char* entry) {
    SkSL::Interpreter<INTERPRETER_WIDTH>* interpreter = fParams->fEffectProgram.fInterpreter.get();
    if (interpreter) {
        const auto& byteCode = interpreter->getCode();
        if (auto fun = byteCode.getFunction(entry)) {
            for (const auto& value : fParams->fEffectProgram.fExternalValues) {
                value->setRandom(&fRandom);
                value->setEffect(this);
            }
            interpreter->setUniforms(fEffectUniforms.data());
            static constexpr int numChannels = sizeof(EffectState) / sizeof(float);
            SkASSERT(numChannels == fun->getParameterSlotCount());
            float* args[numChannels];
            for (int i = 0; i < numChannels; ++i) {
                args[i] = &fState.fAge + i;
            }
            SkAssertResult(interpreter->runStriped(fun, 1, args));
            this->processEffectSpawnRequests(now);
        }
    }
}

void SkParticleEffect::processParticleSpawnRequests(double now, int start) {
    const auto& data = fParticles.fData;
    for (const auto& spawnReq : fSpawnRequests) {
        int idx = start + spawnReq.fIndex;
        sk_sp<SkParticleEffect> newEffect(new SkParticleEffect(std::move(spawnReq.fParams),
                                                               fParticles.fRandom[idx]));
        newEffect->start(now, spawnReq.fLoop,
                         { data[SkParticles::kPositionX      ][idx],
                           data[SkParticles::kPositionY      ][idx] },
                         { data[SkParticles::kHeadingX       ][idx],
                           data[SkParticles::kHeadingY       ][idx] },
                           data[SkParticles::kScale          ][idx],
                         { data[SkParticles::kVelocityX      ][idx],
                           data[SkParticles::kVelocityY      ][idx] },
                           data[SkParticles::kVelocityAngular][idx],
                         { data[SkParticles::kColorR         ][idx],
                           data[SkParticles::kColorG         ][idx],
                           data[SkParticles::kColorB         ][idx],
                           data[SkParticles::kColorA         ][idx] },
                           data[SkParticles::kSpriteFrame    ][idx],
             float_to_bits(data[SkParticles::kFlags          ][idx]));
        fSubEffects.push_back(std::move(newEffect));
    }
    fSpawnRequests.reset();
}

void SkParticleEffect::runParticleScript(double now, const char* entry, int start, int count) {
    SkSL::Interpreter<INTERPRETER_WIDTH>* interpreter =
                                                       fParams->fParticleProgram.fInterpreter.get();
    if (interpreter) {
        const auto& byteCode = interpreter->getCode();
        if (auto fun = byteCode.getFunction(entry)) {
            float* args[SkParticles::kNumChannels];
            for (int i = 0; i < SkParticles::kNumChannels; ++i) {
                args[i] = fParticles.fData[i].get() + start;
            }
            SkRandom* randomBase = fParticles.fRandom.get() + start;
            for (const auto& value : fParams->fParticleProgram.fExternalValues) {
                value->setRandom(randomBase);
                value->setEffect(this);
            }
            memcpy(&fParticleUniforms[1], &fState.fAge, sizeof(EffectState));
            interpreter->setUniforms(fParticleUniforms.data());
            SkAssertResult(interpreter->runStriped(fun, count, (float**) args));
            this->processParticleSpawnRequests(now, start);
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
    fEffectUniforms[0] = deltaTime;
    fParticleUniforms[0] = deltaTime;

    // Is this the first update after calling start()?
    // Run 'effectSpawn' to set initial emitter properties.
    if (fState.fAge == 0.0f && fState.fLoopCount == 0) {
        this->runEffectScript(now, "effectSpawn");
    }

    fState.fAge += deltaTime / fState.fLifetime;
    if (fState.fAge > 1) {
        // We always run effectDeath when age crosses 1, whether we're looping or actually dying
        this->runEffectScript(now, "effectDeath");

        if (fLooping) {
            // If we looped, then run effectSpawn again (with the updated loop count)
            fState.fLoopCount += sk_float_floor2int(fState.fAge);
            fState.fAge = fmodf(fState.fAge, 1.0f);
            this->runEffectScript(now, "effectSpawn");
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
    this->runParticleScript(now, "death", fCount, numDyingParticles);

    // Run 'effectUpdate' to adjust emitter properties
    this->runEffectScript(now, "effectUpdate");

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
            fParticles.fData[SkParticles::kFlags          ][fCount] = bits_to_float(fState.fFlags);
            fParticles.fRandom[fCount] = fRandom;
            fCount++;
        }

        // Run the spawn script
        this->runParticleScript(now, "spawn", spawnBase, numToSpawn);

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
    this->runParticleScript(now, "update", 0, fCount);

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
    if (this->isAlive(false)) {
        this->advanceTime(now);
    }

    // Now update all of our sub-effects, removing any that have died
    for (int i = 0; i < fSubEffects.count(); ++i) {
        fSubEffects[i]->update(now);
        if (!fSubEffects[i]->isAlive()) {
            fSubEffects[i] = fSubEffects.back();
            fSubEffects.pop_back();
            --i;
        }
    }
}

void SkParticleEffect::draw(SkCanvas* canvas) {
    if (this->isAlive(false) && fParams->fDrawable) {
        SkPaint paint;
        paint.setFilterQuality(SkFilterQuality::kMedium_SkFilterQuality);
        fParams->fDrawable->draw(canvas, fParticles, fCount, paint);
    }

    for (const auto& subEffect : fSubEffects) {
        subEffect->draw(canvas);
    }
}

void SkParticleEffect::setCapacity(int capacity) {
    for (int i = 0; i < SkParticles::kNumChannels; ++i) {
        fParticles.fData[i].realloc(capacity);
    }
    fParticles.fRandom.realloc(capacity);
    fStableRandoms.realloc(capacity);

    fCapacity = capacity;
    fCount = std::min(fCount, fCapacity);
}

void SkParticleEffect::RegisterParticleTypes() {
    REGISTER_REFLECTED(SkReflected);
    SkParticleBinding::RegisterBindingTypes();
    SkParticleDrawable::RegisterDrawableTypes();
}
