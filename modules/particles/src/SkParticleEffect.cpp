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
#include "SkNx.h"
#include "SkPaint.h"
#include "SkParticleAffector.h"
#include "SkParticleEmitter.h"
#include "SkReflected.h"
#include "SkRSXform.h"

SkParticleVelocity InitialVelocityParams::eval(SkRandom& random) const {
    float angle = fAngle + fAngleSpread * (random.nextF() - 0.5f);
    SkScalar c, s = SkScalarSinCos(angle, &c);
    float strength = fStrength.eval(random);
    if (fBidirectional && random.nextBool()) {
        strength = -strength;
    }
    float spin = SkDegreesToRadians(fSpin.eval(random));
    if (fBidirectionalSpin && random.nextBool()) {
        spin = -spin;
    }
    return SkParticleVelocity{ SkVector{ c * strength, s * strength }, spin };
}

void InitialVelocityParams::visitFields(SkFieldVisitor* v) {
    v->visit("Angle", fAngle, SkField::kAngle_Field);
    v->visit("Spread", fAngleSpread, SkField::kAngle_Field);
    v->visit("Strength", fStrength);
    v->visit("Bidirectional", fBidirectional);

    v->visit("Spin", fSpin);
    v->visit("BidirectionalSpin", fBidirectionalSpin);
}

void SkParticleEffectParams::visitFields(SkFieldVisitor* v) {
    v->visit("MaxCount", fMaxCount);
    v->visit("Rate", fRate);
    v->visit("Life", fLifetime);
    v->visit("StartColor", fStartColor);
    v->visit("EndColor", fEndColor);

    v->visit("Size", fSize);
    v->visit("Velocity", fVelocity);

    v->visit("Image", fImage);
    v->visit("ImageCols", fImageCols);
    v->visit("ImageRows", fImageRows);

    v->visit("Emitter", fEmitter);

    v->visit("Affectors", fAffectors);
}

SkParticleEffect::SkParticleEffect(sk_sp<SkParticleEffectParams> params)
        : fParams(std::move(params))
        , fCount(0)
        , fLastTime(-1.0f)
        , fSpawnRemainder(0.0f) {
    this->setCapacity(fParams->fMaxCount);

    // Load image, determine sprite rect size
    fImage = GetResourceAsImage(fParams->fImage.c_str());
    if (!fImage) {
        uint32_t whitePixel = ~0;
        SkPixmap pmap(SkImageInfo::MakeN32Premul(1, 1), &whitePixel, sizeof(uint32_t));
        fImage = SkImage::MakeRasterCopy(pmap);
    }
    int w = fImage->width();
    int h = fImage->height();
    SkASSERT(w % fParams->fImageCols == 0);
    SkASSERT(h % fParams->fImageRows == 0);
    fImageRect = SkRect::MakeIWH(w / fParams->fImageCols, h / fParams->fImageRows);
}

void SkParticleEffect::update(SkRandom& random, const SkAnimTimer& timer) {
    if (!timer.isRunning()) {
        return;
    }

    // Handle user edits to fMaxCount
    if (fParams->fMaxCount != fCapacity) {
        this->setCapacity(fParams->fMaxCount);
    }

    double now = timer.secs();

    if (fLastTime < 0) {
        // Hack: kick us off with 1/30th of a second on first update
        fLastTime = now - (1.0 / 30);
    }

    float deltaTime = static_cast<float>(now - fLastTime);
    fLastTime = now;

    Sk4f startColor = Sk4f::Load(fParams->fStartColor.vec());
    Sk4f colorScale = Sk4f::Load(fParams->fEndColor.vec()) - startColor;

    SkParticleUpdateParams updateParams;
    updateParams.fDeltaTime = deltaTime;
    updateParams.fRandom = &random;

    // Age/update old particles
    for (int i = 0; i < fCount; ++i) {
        if (now > fParticles[i].fTimeOfDeath) {
            // NOTE: This is fast, but doesn't preserve drawing order. Could be a problem...
            fParticles[i] = fParticles[fCount - 1];
            fSpriteRects[i] = fSpriteRects[fCount - 1];
            fColors[i] = fColors[fCount - 1];
            --i;
            --fCount;
            continue;
        }

        // Compute fraction of lifetime that's elapsed
        float t = static_cast<float>((now - fParticles[i].fTimeOfBirth) /
            (fParticles[i].fTimeOfDeath - fParticles[i].fTimeOfBirth));

        SkRandom stableRandom = fParticles[i].fStableRandom;
        updateParams.fStableRandom = &stableRandom;
        updateParams.fParticleT = t;

        // Set sprite rect by lifetime
        int frame = static_cast<int>(t * this->spriteCount() + 0.5);
        frame = SkTPin(frame, 0, this->spriteCount() - 1);
        fSpriteRects[i] = this->spriteRect(frame);

        // Set color by lifetime
        fColors[i] = Sk4f_toL32(swizzle_rb(startColor + (colorScale * t)));
        for (auto affector : fParams->fAffectors) {
            if (affector) {
                affector->apply(updateParams, fParticles[i].fPV);
            }
        }

        // Set size by lifetime
        fParticles[i].fPV.fPose.fScale = fParams->fSize.eval(t, stableRandom);

        // Integrate position / orientation
        fParticles[i].fPV.fPose.fPosition += fParticles[i].fPV.fVelocity.fLinear * deltaTime;

        SkScalar c, s = SkScalarSinCos(fParticles[i].fPV.fVelocity.fAngular * deltaTime, &c);
        SkVector oldHeading = fParticles[i].fPV.fPose.fHeading;
        fParticles[i].fPV.fPose.fHeading = { oldHeading.fX * c - oldHeading.fY * s,
                                             oldHeading.fX * s + oldHeading.fY * c };
    }

    // Spawn new particles
    float desired = fParams->fRate * deltaTime + fSpawnRemainder;
    int numToSpawn = sk_float_round2int(desired);
    fSpawnRemainder = desired - numToSpawn;
    numToSpawn = SkTPin(numToSpawn, 0, fParams->fMaxCount - fCount);
    if (fParams->fEmitter) {
        for (int i = 0; i < numToSpawn; ++i) {
            fParticles[fCount].fTimeOfBirth = now;
            fParticles[fCount].fTimeOfDeath = now + fParams->fLifetime.eval(random);
            fParticles[fCount].fPV.fPose = fParams->fEmitter->emit(random);
            fParticles[fCount].fPV.fVelocity = fParams->fVelocity.eval(random);
            fParticles[fCount].fStableRandom = random;
            fSpriteRects[fCount] = this->spriteRect(0);
            fCount++;
        }
    }

    // Re-generate all xforms
    SkPoint ofs = this->spriteCenter();
    for (int i = 0; i < fCount; ++i) {
        fXforms[i] = fParticles[i].fPV.fPose.asRSXform(ofs);
    }
}

void SkParticleEffect::draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setFilterQuality(SkFilterQuality::kMedium_SkFilterQuality);
    canvas->drawAtlas(fImage, fXforms.get(), fSpriteRects.get(), fColors.get(), fCount,
                        SkBlendMode::kModulate, nullptr, &paint);
}

void SkParticleEffect::setCapacity(int capacity) {
    fParticles.realloc(capacity);
    fXforms.realloc(capacity);
    fSpriteRects.realloc(capacity);
    fColors.realloc(capacity);

    fCapacity = capacity;
    fCount = SkTMin(fCount, fCapacity);
}
