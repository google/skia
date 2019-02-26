/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkParticleEffect.h"

#include "Resources.h"
#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "SkColorData.h"
#include "SkPaint.h"
#include "SkParticleAffector.h"
#include "SkParticleDrawable.h"
#include "SkParticleEmitter.h"
#include "SkReflected.h"
#include "SkRSXform.h"

void SkParticleEffectParams::visitFields(SkFieldVisitor* v) {
    v->visit("MaxCount", fMaxCount);
    v->visit("Duration", fEffectDuration);
    v->visit("Rate", fRate);
    v->visit("Life", fLifetime);

    v->visit("Drawable", fDrawable);
    v->visit("Emitter", fEmitter);

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

void SkParticleEffect::start(const SkAnimTimer& timer, bool looping) {
    fCount = 0;
    fLastTime = fSpawnTime = timer.secs();
    fSpawnRemainder = 0.0f;
    fLooping = looping;
}

void SkParticleEffect::update(const SkAnimTimer& timer) {
    if (!timer.isRunning() || !this->isAlive() || !fParams->fDrawable) {
        return;
    }

    double now = timer.secs();
    float deltaTime = static_cast<float>(now - fLastTime);
    if (deltaTime < 0.0f) {
        return;
    }
    fLastTime = now;

    // Handle user edits to fMaxCount
    if (fParams->fMaxCount != fCapacity) {
        this->setCapacity(fParams->fMaxCount);
    }

    SkParticleUpdateParams updateParams;
    updateParams.fDeltaTime = deltaTime;

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
    if (fParams->fEmitter) {
        // This isn't "particle" t, it's effect t.
        float t = static_cast<float>((now - fSpawnTime) / fParams->fEffectDuration);
        t = fLooping ? fmodf(t, 1.0f) : SkTPin(t, 0.0f, 1.0f);

        for (int i = 0; i < numToSpawn; ++i) {
            // Mutate our SkRandom so each particle definitely gets a different stable generator
            fRandom.nextU();

            // Temporarily set our age to the *effect* age, so spawn affectors are driven by that
            fParticles[fCount].fAge = t;
            fParticles[fCount].fInvLifetime =
                    sk_ieee_float_divide(1.0f, fParams->fLifetime.eval(t, fRandom));
            fParticles[fCount].fPose = fParams->fEmitter->emit(fRandom);
            fParticles[fCount].fVelocity.fLinear = { 0.0f, 0.0f };
            fParticles[fCount].fVelocity.fAngular = 0.0f;
            fParticles[fCount].fColor = { 1.0f, 1.0f, 1.0f, 1.0f };
            fParticles[fCount].fFrame = 0.0f;

            fParticles[fCount].fRandom = fStableRandoms[fCount] = fRandom;
            fCount++;
        }

        // Apply spawn affectors, then reset our age to 0 (the *particle* age)
        for (auto affector : fParams->fSpawnAffectors) {
            if (affector) {
                affector->apply(updateParams, fParticles + (fCount - numToSpawn), numToSpawn);
            }
        }
        for (int i = fCount - numToSpawn; i < fCount; ++i) {
            fParticles[i].fAge = 0.0f;
        }
    }

    // Restore all stable random generators so update affectors get consistent behavior each frame
    for (int i = 0; i < fCount; ++i) {
        fParticles[i].fRandom = fStableRandoms[i];
    }

    // Apply update rules
    for (auto affector : fParams->fUpdateAffectors) {
        if (affector) {
            affector->apply(updateParams, fParticles, fCount);
        }
    }

    // Do fixed-function update work (integration of position and orientation)
    for (int i = 0; i < fCount; ++i) {
        fParticles[i].fPose.fPosition += fParticles[i].fVelocity.fLinear * deltaTime;

        SkScalar c, s = SkScalarSinCos(fParticles[i].fVelocity.fAngular * deltaTime, &c);
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
