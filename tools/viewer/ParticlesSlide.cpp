/*
* Copyright 2019 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "ParticlesSlide.h"

#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "SkFont.h"
#include "SkImage.h"
#include "SkMakeUnique.h"
#include "SkPath.h"
#include "SkPathMeasure.h"
#include "SkRSXform.h"
#include "SkSurface.h"
#include "SkTArray.h"
#include "SkTextUtils.h"

#include "imgui.h"
#include "sk_tool_utils.h"

using namespace sk_app;

struct SkRangedFloat { float fMin; float fMax; };

class SkParticleEmitter {
public:
    SkParticleEmitter(const SkPath& path) {
        fTotalLength = 0;
        SkPathMeasure pm(path, false);
        do {
            SkPath contour;
            if (pm.getSegment(0, SK_ScalarInfinity, &contour, true)) {
                fContours.push_back(contour);
                fTotalLength += pm.getLength();
                fLengths.push_back(pm.getLength());
            }
        } while (pm.nextContour());
    }

    SkRSXform emit(SkRandom& random) const {
        if (fContours.count() == 0) {
            return SkRSXform::Make(1, 0, 0, 0);
        }

        SkScalar len = random.nextRangeScalar(0, fTotalLength);
        int c = 0;
        while (c < fContours.count() && len > fLengths[c]) {
            len -= fLengths[c++];
        }
        SkPathMeasure cm(fContours[c], false);
        SkPoint pos = { 0, 0 };
        SkVector tan = { 0, 1 };
        cm.getPosTan(len, &pos, &tan);
        return SkRSXform::Make(tan.fY, -tan.fX, pos.fX, pos.fY);
    }

private:
    SkScalar fTotalLength;
    SkTArray<SkPath> fContours;
    SkTArray<SkScalar> fLengths;
};

class SkParticleEffect {
public:
    SkParticleEffect(std::unique_ptr<SkParticleEmitter> emitter, sk_sp<SkImage> image,
                     int maxCount, int rate, SkRangedFloat lifetime)
            : fEmitter(std::move(emitter))
            , fImage(std::move(image))
            , fCount(0)
            , fMaxCount(maxCount)
            , fRate(rate)
            , fLifetime(lifetime)
            , fLastTime(-1.0f) {
        fParams.reset(new ParticleParams[fMaxCount]);
        fXforms.reset(new SkRSXform[fMaxCount]);
        fTexs.reset(new SkRect[fMaxCount]);

        // Set all sprite rects to the entire image (for now) and not yet alive
        for (int i = 0; i < fMaxCount; ++i) {
            fParams[i].fTimeOfDeath = -1.0f;
            fTexs[i] = SkRect::MakeWH(fImage->width(), fImage->height());
        }
    }

    void setEmitter(std::unique_ptr<SkParticleEmitter> emitter) {
        fEmitter = std::move(emitter);
    }

    void update(SkRandom& random, const SkAnimTimer& timer) {
        if (!timer.isRunning()) {
            return;
        }

        double now = timer.secs();

        if (fLastTime < 0) {
            // Hack: kick us off with 1/30th of a second on first update
            fLastTime = now - (1.0 / 30);
        }

        double elapsed = now - fLastTime;
        fLastTime = now;

        // Age/update old particles
        for (int i = 0; i < fCount; ++i) {
            if (now > fParams[i].fTimeOfDeath) {
                fParams[i] = fParams[fCount - 1];
                fXforms[i] = fXforms[fCount - 1];
                fTexs[i] = fTexs[fCount - 1];
                --i;
                --fCount;
                continue;
            }

//            fXforms[i].fTx += fXforms[i].fSCos * elapsed * 50;
//            fXforms[i].fTy += fXforms[i].fSSin * elapsed * 50;
            fXforms[i].fTx += random.nextRangeScalar(-15.0f, 15.0f) * elapsed;
            fXforms[i].fTy -= random.nextRangeScalar(15.0f, 40.0f) * elapsed;
        }

        // Spawn new particles
        int numToSpawn = SkTMin(fRate, fMaxCount - fCount);
        for (int i = 0; i < numToSpawn; ++i) {
            fParams[fCount].fTimeOfBirth = now;
            fParams[fCount].fTimeOfDeath = now + random.nextRangeF(fLifetime.fMin, fLifetime.fMax);
            fXforms[fCount] = fEmitter->emit(random);
            fCount++;
        }
    }

    void draw(SkCanvas* canvas) {
        canvas->clear(0);
        canvas->drawAtlas(fImage, fXforms.get(), fTexs.get(), fCount, nullptr, nullptr);
    }

    int getRate() const { return fRate; }
    void setRate(int rate) { fRate = SkTMax(0, rate); }

    SkRangedFloat getLifetime() const { return fLifetime; }
    void setLifetime(SkRangedFloat lifetime) {
        fLifetime = { SkTMax(0.0f, lifetime.fMin), SkTMax(0.0f, lifetime.fMax) };
    }

private:
    struct ParticleParams {
        double fTimeOfBirth;
        double fTimeOfDeath;
    };
    std::unique_ptr<SkParticleEmitter> fEmitter;
    sk_sp<SkImage> fImage;

    int fCount;
    int fMaxCount;
    int fRate;
    SkRangedFloat fLifetime;
    double fLastTime;
    std::unique_ptr<ParticleParams[]> fParams;
    std::unique_ptr<SkRSXform[]> fXforms;
    std::unique_ptr<SkRect[]> fTexs;
};

ParticlesSlide::ParticlesSlide() {
    fName = "Particles";

    sk_sp<SkImage> image;
    {
        auto surface = SkSurface::MakeRasterN32Premul(8, 8);
        surface->getCanvas()->clear(0);
        SkPaint paint;
        paint.setColor(SK_ColorWHITE);
        paint.setAntiAlias(true);
        paint.setAlpha(96);
        surface->getCanvas()->drawCircle(4, 4, 1, paint);
        image = surface->makeImageSnapshot();
    }

    fEffect = skstd::make_unique<SkParticleEffect>(nullptr, std::move(image), 4096, 300,
                                                   SkRangedFloat { 0.1f, 0.3f });

    strcpy(fEmitterText, "Particles");
    fFontSize = 96;
    this->buildEmitter();
}

void ParticlesSlide::buildEmitter() {
    if (strlen(fEmitterText) == 0) {
        return;
    }

    SkPath path;
    SkFont font(sk_tool_utils::create_portable_typeface());
    font.setSize(fFontSize);
    SkTextUtils::GetPath(fEmitterText, strlen(fEmitterText), kUTF8_SkTextEncoding, 0, 0, font,
                         &path);
    auto emitter = skstd::make_unique<SkParticleEmitter>(path);
    fEffect->setEmitter(std::move(emitter));
}

ParticlesSlide::~ParticlesSlide() {}

SkISize ParticlesSlide::getDimensions() const  {
    return SkISize::Make(500, 500);
}

void ParticlesSlide::draw(SkCanvas* canvas) {
    if (ImGui::Begin("Particles")) {
        bool rebuildEmitter = false;
        if (ImGui::InputText("Emitter", fEmitterText, sizeof(fEmitterText))) {
            rebuildEmitter = true;
        }
        if (ImGui::DragInt("Font Size", &fFontSize, 1.0f, 12, 200)) {
            rebuildEmitter = true;
        }
        if (rebuildEmitter) {
            this->buildEmitter();
        }

        int rate = fEffect->getRate();
        if (ImGui::DragInt("Rate", &rate)) {
            fEffect->setRate(rate);
        }

        SkRangedFloat lifetime = fEffect->getLifetime();
        bool setLifetime = false;
        if (ImGui::DragFloat("Min Life", &lifetime.fMin)) {
            setLifetime = true;
        }
        if (ImGui::DragFloat("Max Life", &lifetime.fMax)) {
            setLifetime = true;
        }
        if (setLifetime) {
            fEffect->setLifetime(lifetime);
        }
    }
    ImGui::End();

    fEffect->draw(canvas);
}

bool ParticlesSlide::animate(const SkAnimTimer& timer) {
    fEffect->update(fRandom, timer);
    return true;
}

void ParticlesSlide::load(SkScalar winWidth, SkScalar winHeight) {}

void ParticlesSlide::unload() {}

bool ParticlesSlide::onChar(SkUnichar c) {
    return false;
}

bool ParticlesSlide::onMouse(SkScalar x, SkScalar y, Window::InputState state, uint32_t modifiers) {
    return false;
}

// Notes:

// Some kind of value over time? Quadratic? Could use SkPath (and interpolate for ranged)?
// SkPath implies 2D coords, which is odd for single-valued (or more than 2).
