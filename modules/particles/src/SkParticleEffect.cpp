/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkParticleEffect.h"

#include "SkCanvas.h"
#include "SkColorData.h"
#include "SkPaint.h"
#include "SkParticleAffector.h"
#include "SkParticleDrawable.h"
#include "SkReflected.h"
#include "SkRSXform.h"

void SkParticleEffectParams::visitFields(SkFieldVisitor* v) {
    v->visit("MaxCount", fMaxCount);
    v->visit("Duration", fEffectDuration);
    v->visit("Rate", fRate);
    v->visit("Life", fLifetime);

    v->visit("Drawable", fDrawable);

    v->visit("Spawn", fSpawnAffectors);
    v->visit("Update", fUpdateAffectors);
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

    SkParticleUpdateParams updateParams;
    updateParams.fDeltaTime = deltaTime;
    updateParams.fEffectAge = effectAge;

    // During spawn, values that refer to kAge_Source get the *effect* age
    updateParams.fAgeSource = SkParticleValue::kEffectAge_Source;

    // Advance age for existing particles, and remove any that have reached their end of life
    for (int i = 0; i < fCount; ++i) {
        fParticles[i].fAge += fParticles[i].fInvLifetime * deltaTime;
        if (fParticles[i].fAge > 1.0f) {
            // NOTE: This is fast, but doesn't preserve drawing order. Could be a problem...
            fParticles[i]     = fParticles[fCount - 1];
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
            fParticles[fCount].fAge = 0.0f;
            fParticles[fCount].fPose.fPosition = { 0.0f, 0.0f };
            fParticles[fCount].fPose.fHeading = { 0.0f, -1.0f };
            fParticles[fCount].fPose.fScale = 1.0f;
            fParticles[fCount].fVelocity.fLinear = { 0.0f, 0.0f };
            fParticles[fCount].fVelocity.fAngular = 0.0f;
            fParticles[fCount].fColor = { 1.0f, 1.0f, 1.0f, 1.0f };
            fParticles[fCount].fFrame = 0.0f;
            fParticles[fCount].fRandom = fRandom;
            fCount++;
        }

        // Apply spawn affectors
        for (auto affector : fParams->fSpawnAffectors) {
            if (affector) {
                affector->apply(updateParams, fParticles + spawnBase, numToSpawn);
            }
        }

        // Now stash copies of the random generators and compute particle lifetimes
        // (so the curve can refer to spawn-computed source values)
        for (int i = spawnBase; i < fCount; ++i) {
            fParticles[i].fInvLifetime =
                sk_ieee_float_divide(1.0f, fParams->fLifetime.eval(updateParams, fParticles[i]));
            fStableRandoms[i] = fParticles[i].fRandom;
        }
    }

    // Restore all stable random generators so update affectors get consistent behavior each frame
    for (int i = 0; i < fCount; ++i) {
        fParticles[i].fRandom = fStableRandoms[i];
    }

    // During update, values that refer to kAge_Source get the *particle* age
    updateParams.fAgeSource = SkParticleValue::kParticleAge_Source;

    // Apply update rules
    for (auto affector : fParams->fUpdateAffectors) {
        if (affector) {
            affector->apply(updateParams, fParticles, fCount);
        }
    }

    // Do fixed-function update work (integration of position and orientation)
    for (int i = 0; i < fCount; ++i) {
        fParticles[i].fPose.fPosition += fParticles[i].fVelocity.fLinear * deltaTime;

        SkScalar s = SkScalarSin(fParticles[i].fVelocity.fAngular * deltaTime),
                 c = SkScalarCos(fParticles[i].fVelocity.fAngular * deltaTime);
        SkVector oldHeading = fParticles[i].fPose.fHeading;
        fParticles[i].fPose.fHeading = { oldHeading.fX * c - oldHeading.fY * s,
                                         oldHeading.fX * s + oldHeading.fY * c };
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
        fParams->fDrawable->draw(canvas, fParticles.get(), fCount, &paint);
    }
}

void SkParticleEffect::setCapacity(int capacity) {
    fParticles.realloc(capacity);
    fStableRandoms.realloc(capacity);

    fCapacity = capacity;
    fCount = SkTMin(fCount, fCapacity);
}

constexpr SkFieldVisitor::EnumStringMapping gValueSourceMapping[] = {
    { SkParticleValue::kAge_Source,          "Age" },
    { SkParticleValue::kRandom_Source,       "Random" },
    { SkParticleValue::kParticleAge_Source,  "ParticleAge" },
    { SkParticleValue::kEffectAge_Source,    "EffectAge" },
    { SkParticleValue::kPositionX_Source,    "PositionX" },
    { SkParticleValue::kPositionY_Source,    "PositionY" },
    { SkParticleValue::kHeadingX_Source,     "HeadingX" },
    { SkParticleValue::kHeadingY_Source,     "HeadingY" },
    { SkParticleValue::kScale_Source,        "Scale" },
    { SkParticleValue::kVelocityX_Source,    "VelocityX" },
    { SkParticleValue::kVelocityY_Source,    "VelocityY" },
    { SkParticleValue::kRotation_Source,     "Rotation" },
    { SkParticleValue::kColorR_Source,       "ColorR" },
    { SkParticleValue::kColorG_Source,       "ColorG" },
    { SkParticleValue::kColorB_Source,       "ColorB" },
    { SkParticleValue::kColorA_Source,       "ColorA" },
    { SkParticleValue::kSpriteFrame_Source,  "SpriteFrame" },
};

constexpr SkFieldVisitor::EnumStringMapping gValueTileModeMapping[] = {
    { SkParticleValue::kClamp_TileMode,  "Clamp" },
    { SkParticleValue::kRepeat_TileMode, "Repeat" },
    { SkParticleValue::kMirror_TileMode, "Mirror" },
};

static bool source_needs_frame(int source) {
    switch (source) {
        case SkParticleValue::kHeadingX_Source:
        case SkParticleValue::kHeadingY_Source:
        case SkParticleValue::kVelocityX_Source:
        case SkParticleValue::kVelocityY_Source:
            return true;
        default:
            return false;
    }
}

void SkParticleValue::visitFields(SkFieldVisitor* v) {
    v->visit("Source", fSource, gValueSourceMapping, SK_ARRAY_COUNT(gValueSourceMapping));
    if (source_needs_frame(fSource)) {
        v->visit("Frame", fFrame, gParticleFrameMapping, SK_ARRAY_COUNT(gParticleFrameMapping));
    }
    v->visit("TileMode", fTileMode, gValueTileModeMapping, SK_ARRAY_COUNT(gValueTileModeMapping));
    v->visit("Left", fLeft);
    v->visit("Right", fRight);

    // Re-compute cached evaluation parameters
    fScale = sk_float_isfinite(1.0f / (fRight - fLeft)) ? 1.0f / (fRight - fLeft) : 0;
    fBias = -fLeft * fScale;
}

float SkParticleValue::getSourceValue(const SkParticleUpdateParams& params,
                                      SkParticleState& ps) const {
    switch ((kAge_Source == fSource) ? params.fAgeSource : fSource) {
        // Do all the simple (non-frame-dependent) sources first:
        case kRandom_Source:      return ps.fRandom.nextF();
        case kParticleAge_Source: return ps.fAge;
        case kEffectAge_Source:   return params.fEffectAge;

        case kPositionX_Source:   return ps.fPose.fPosition.fX;
        case kPositionY_Source:   return ps.fPose.fPosition.fY;
        case kScale_Source:       return ps.fPose.fScale;
        case kRotation_Source:    return ps.fVelocity.fAngular;

        case kColorR_Source:      return ps.fColor.fR;
        case kColorG_Source:      return ps.fColor.fG;
        case kColorB_Source:      return ps.fColor.fB;
        case kColorA_Source:      return ps.fColor.fA;
        case kSpriteFrame_Source: return ps.fFrame;
    }

    SkASSERT(source_needs_frame(fSource));
    SkVector frameUp = ps.getFrameHeading(static_cast<SkParticleFrame>(fFrame));
    SkVector frameRight = { -frameUp.fY, frameUp.fX };

    switch (fSource) {
        case kHeadingX_Source:  return ps.fPose.fHeading.dot(frameRight);
        case kHeadingY_Source:  return ps.fPose.fHeading.dot(frameUp);
        case kVelocityX_Source: return ps.fVelocity.fLinear.dot(frameRight);
        case kVelocityY_Source: return ps.fVelocity.fLinear.dot(frameUp);
    }

    SkDEBUGFAIL("Unreachable");
    return 0.0f;
}

float SkParticleValue::eval(const SkParticleUpdateParams& params, SkParticleState& ps) const {
    float v = this->getSourceValue(params, ps);
    v = (v * fScale) + fBias;

    switch (fTileMode) {
        case kClamp_TileMode:
            v = SkTPin(v, 0.0f, 1.0f);
            break;
        case kRepeat_TileMode:
            v = sk_float_mod(v, 1.0f);
            if (v < 0) {
                v += 1.0f;
            }
            break;
        case kMirror_TileMode:
            v = sk_float_mod(v, 2.0f);
            if (v < 0) {
                v += 2.0f;
            }
            v = 1.0f - sk_float_abs(v - 1.0f);
            break;
    }

    return v;
}
