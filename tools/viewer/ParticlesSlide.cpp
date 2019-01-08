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

static inline SkRSXform MakeXform(SkPoint pos, SkVector right, SkPoint ofs) {
    const float s = right.fY;
    const float c = right.fX;
    return SkRSXform::Make(c, s, pos.fX + -c*ofs.fX + s*ofs.fY, pos.fY + -s*ofs.fX + -c*ofs.fY);
}

static inline SkVector right_to_up(SkScalar rightX, SkScalar rightY) {
    return { rightY, -rightX };
}

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

    SkRSXform emit(SkRandom& random, SkPoint ofs) const {
        if (fContours.count() == 0) {
            return SkRSXform::Make(1, 0, 0, 0);
        }

        SkScalar len = random.nextRangeScalar(0, fTotalLength);
        int idx = 0;
        while (idx < fContours.count() && len > fLengths[idx]) {
            len -= fLengths[idx++];
        }
        SkPathMeasure cm(fContours[idx], false);
        SkPoint pos = { 0, 0 };
        SkVector tan = { 0, 1 };
        cm.getPosTan(len, &pos, &tan);
        return MakeXform(pos, tan, ofs);
    }

private:
    SkScalar fTotalLength;
    SkTArray<SkPath> fContours;
    SkTArray<SkScalar> fLengths;
};

class SkParticleEffect {
public:
    typedef std::function<void(SkRandom&, SkRSXform&, SkVector&)> InitParticleFn;
    typedef std::function<void(SkRandom&, SkRSXform&, SkVector&, double)> UpdateParticleFn;

    SkParticleEffect(sk_sp<SkImage> image, int maxCount, int rate, SkRangedFloat lifetime)
            : fImage(std::move(image))
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

    void update(const SkParticleEmitter* emitter, SkRandom& random, const SkAnimTimer& timer,
                InitParticleFn&& initParticle, UpdateParticleFn&& updateParticle) {
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

            updateParticle(random, fXforms[i], fParams[i].fVelocity, elapsed);

//            SkVector up = right_to_up(fXforms[i].fSCos, fXforms[i].fSSin);
//            fXforms[i].fTx += up.fX * elapsed * 50;
//            fXforms[i].fTy += up.fY * elapsed * 50;
//            fXforms[i].fTx += random.nextRangeScalar(-15.0f, 15.0f) * elapsed;
//            fXforms[i].fTy -= random.nextRangeScalar(15.0f, 40.0f) * elapsed;
        }

        // Spawn new particles
        int numToSpawn = SkTMin(fRate, fMaxCount - fCount);
        SkPoint ofs = { fImage->width() * 0.5f, fImage->height() * 0.5f };
        for (int i = 0; i < numToSpawn; ++i) {
            fParams[fCount].fTimeOfBirth = now;
            fParams[fCount].fTimeOfDeath = now + random.nextRangeF(fLifetime.fMin, fLifetime.fMax);
            fXforms[fCount] = emitter->emit(random, ofs);
            fParams[fCount].fVelocity = SkPoint{ 0, 0 };
            initParticle(random, fXforms[fCount], fParams[fCount].fVelocity);
            fCount++;
        }
    }

    void draw(SkCanvas* canvas) {
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
        SkVector fVelocity;
    };
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
    fDrawPath = false;

    sk_sp<SkImage> image;
    {
        auto surface = SkSurface::MakeRasterN32Premul(1, 1);
        if (true) {
            surface->getCanvas()->clear(SkColorSetARGB(96, 255, 255, 255));
        } else {
            surface->getCanvas()->clear(0);
            SkPaint paint;
            paint.setColor(SK_ColorWHITE);
            paint.setAntiAlias(true);
            paint.setAlpha(96);
            surface->getCanvas()->drawCircle(16, 16, 1, paint);
        }
        image = surface->makeImageSnapshot();
    }

    fEffect = skstd::make_unique<SkParticleEffect>(std::move(image), 4096, 300,
                                                   SkRangedFloat { 1.0f, 3.0f });

    strcpy(fEmitterText, "SKIA");
    fFontSize = 96;
    this->buildEmitter();
}

void ParticlesSlide::buildEmitter() {
    if (strlen(fEmitterText) == 0) {
        return;
    }

    SkFont font(sk_tool_utils::create_portable_typeface());
    font.setSize(fFontSize);
    SkTextUtils::GetPath(fEmitterText, strlen(fEmitterText), kUTF8_SkTextEncoding, 0, 0, font,
                         &fPath);
    fEmitter = skstd::make_unique<SkParticleEmitter>(fPath);
}

ParticlesSlide::~ParticlesSlide() {}

SkISize ParticlesSlide::getDimensions() const  {
    return SkISize::Make(500, 500);
}

void ParticlesSlide::draw(SkCanvas* canvas) {
    canvas->clear(0);

    fEffect->draw(canvas);

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

        ImGui::DragFloat("Floor", &fFloor);
        ImGui::Checkbox("Show Path", &fDrawPath);
    }
    ImGui::End();

    if (fDrawPath) {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorRED);
        canvas->drawPath(fPath, paint);
    }
}

bool ParticlesSlide::animate(const SkAnimTimer& timer) {
    auto initParticle = [](SkRandom& random, SkRSXform& xform, SkVector& velocity) {
        velocity.fX = random.nextRangeF(-25.0f, 25.0f);
    };
    auto updateParticle = [=](SkRandom& random, SkRSXform& xform, SkVector& velocity,
                             double elapsed) {
        velocity.fY += elapsed * 50.0f;

        xform.fTx += velocity.fX * elapsed;
        xform.fTy += velocity.fY * elapsed;

        if (xform.fTy > fFloor) {
            xform.fTy = fFloor;
            velocity.fY = -velocity.fY;
        }
    };
    fEffect->update(fEmitter.get(), fRandom, timer, initParticle, updateParticle);
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
