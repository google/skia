/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkParticleEffect_DEFINED
#define SkParticleEffect_DEFINED

#include "SkAutoMalloc.h"
#include "SkColor.h"
#include "SkCurve.h"
#include "SkParticleData.h"
#include "SkRandom.h"
#include "SkRect.h"
#include "SkRefCnt.h"
#include "SkString.h"
#include "SkTArray.h"

class SkAnimTimer;
class SkCanvas;
class SkFieldVisitor;
class SkImage;
class SkParticleAffector;
class SkParticleEmitter;
struct SkRSXform;

// TODO: Phase this out, once all properties are driven by the two-lists-of-affectors
struct InitialVelocityParams {
    float fAngle = 0.0f;
    float fAngleSpread = 0.0f;
    SkRangedFloat fStrength = { 0.0f, 0.0f };
    bool fBidirectional = false;

    SkRangedFloat fSpin = { 0.0f, 0.0f };
    bool fBidirectionalSpin = false;

    SkParticleVelocity eval(SkRandom& random) const;

    void visitFields(SkFieldVisitor* v);
};

class SkParticleEffectParams : public SkRefCnt {
public:
    int           fMaxCount = 128;
    float         fRate = 8.0f;
    SkRangedFloat fLifetime = { 1.0f, 1.0f };
    SkColor4f     fStartColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    SkColor4f     fEndColor = { 1.0f, 1.0f, 1.0f, 1.0f };

    SkCurve       fSize = 1.0f;

    // TODO: Add local vs. world copies of these
    // Initial velocity controls
    InitialVelocityParams fVelocity;

    // Sprite image parameters
    // TODO: Move sprite stuff in here, out of effect
    SkString fImage;
    int      fImageCols = 1;
    int      fImageRows = 1;

    // Emitter shape & parameters
    sk_sp<SkParticleEmitter> fEmitter;

    // Update rules
    SkTArray<sk_sp<SkParticleAffector>> fAffectors;

    void visitFields(SkFieldVisitor* v);
};

class SkParticleEffect : public SkRefCnt {
public:
    SkParticleEffect(sk_sp<SkParticleEffectParams> params);

    void update(SkRandom& random, const SkAnimTimer& timer);
    void draw(SkCanvas* canvas);

    SkParticleEffectParams* getParams() { return fParams.get(); }

private:
    void setCapacity(int capacity);

    int spriteCount() const { return fParams->fImageCols * fParams->fImageRows; }
    SkRect spriteRect(int i) const {
        SkASSERT(i >= 0 && i < this->spriteCount());
        int row = i / fParams->fImageCols;
        int col = i % fParams->fImageCols;
        return fImageRect.makeOffset(col * fImageRect.width(), row * fImageRect.height());
    }
    SkPoint spriteCenter() const {
        return { fImageRect.width() * 0.5f, fImageRect.height() * 0.5f };
    }

    struct Particle {
        double fTimeOfBirth;
        double fTimeOfDeath;
        SkRandom fStableRandom;

        // Texture coord rects and colors are stored in parallel arrays for drawAtlas.
        SkParticlePoseAndVelocity fPV;
    };

    sk_sp<SkParticleEffectParams> fParams;
    sk_sp<SkImage>                fImage;
    SkRect                        fImageRect;

    int    fCount;
    double fLastTime;
    float  fSpawnRemainder;

    SkAutoTMalloc<Particle>  fParticles;
    SkAutoTMalloc<SkRSXform> fXforms;
    SkAutoTMalloc<SkRect>    fSpriteRects;
    SkAutoTMalloc<SkColor>   fColors;

    // Cached
    int fCapacity;
};

#endif // SkParticleEffect_DEFINED
