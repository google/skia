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

    // Handle user edits to fMaxCount
    if (fParams->fMaxCount != fCapacity) {
        this->setCapacity(fParams->fMaxCount);
    }

    double now = timer.secs();
    float deltaTime = static_cast<float>(now - fLastTime);
    fLastTime = now;

    SkParticleUpdateParams updateParams;
    updateParams.fDeltaTime = deltaTime;
    updateParams.fRandom = &fRandom;

    // Remove particles that have reached their end of life
    for (int i = 0; i < fCount; ++i) {
        if (now > fParticles[i].fTimeOfDeath) {
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
        double t = (now - fSpawnTime) / fParams->fEffectDuration;
        updateParams.fParticleT = static_cast<float>(fLooping ? fmod(t, 1.0) : SkTPin(t, 0.0, 1.0));

        for (int i = 0; i < numToSpawn; ++i) {
            // Mutate our SkRandom so each particle definitely gets a different stable generator
            fRandom.nextU();

            fParticles[fCount].fTimeOfBirth = now;
            fParticles[fCount].fTimeOfDeath = now + fParams->fLifetime.eval(updateParams.fParticleT,
                                                                            fRandom);
            fParticles[fCount].fPose = fParams->fEmitter->emit(fRandom);
            fParticles[fCount].fVelocity.fLinear = { 0.0f, 0.0f };
            fParticles[fCount].fVelocity.fAngular = 0.0f;
            fParticles[fCount].fColor = { 1.0f, 1.0f, 1.0f, 1.0f };
            fParticles[fCount].fFrame = 0.0f;

            fParticles[fCount].fStableRandom = fStableRandoms[fCount] = fRandom;
            fCount++;
        }

        // Apply spawn affectors
        for (int i = fCount - numToSpawn; i < fCount; ++i) {
            for (auto affector : fParams->fSpawnAffectors) {
                if (affector) {
                    affector->apply(updateParams, fParticles[i]);
                }
            }
        }
    }

    // Restore all stable random generators so update affectors get consistent behavior each frame
    for (int i = 0; i < fCount; ++i) {
        fParticles[i].fStableRandom = fStableRandoms[i];
    }

    // Apply update rules
    for (int i = 0; i < fCount; ++i) {
        // Compute fraction of lifetime that's elapsed
        updateParams.fParticleT =
            static_cast<float>((now - fParticles[i].fTimeOfBirth) /
                               (fParticles[i].fTimeOfDeath - fParticles[i].fTimeOfBirth));

        for (auto affector : fParams->fUpdateAffectors) {
            if (affector) {
                affector->apply(updateParams, fParticles[i]);
            }
        }

        // Integrate position / orientation
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
