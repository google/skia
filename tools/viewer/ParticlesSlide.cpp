/*
* Copyright 2019 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "ParticlesSlide.h"

#include "Resources.h"
#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "SkColorData.h"
#include "SkFont.h"
#include "SkImage.h"
#include "SkJSON.h"
#include "SkJSONWriter.h"
#include "SkMakeUnique.h"
#include "SkNx.h"
#include "SkPath.h"
#include "SkPathMeasure.h"
#include "SkRect.h"
#include "SkRSXform.h"
#include "SkSurface.h"
#include "SkTArray.h"
#include "SkTextUtils.h"
#include "imgui.h"
#include "sk_tool_utils.h"

using namespace sk_app;

namespace {

inline SkRSXform MakeXform(SkPoint pos, SkVector right, SkPoint ofs) {
    const float s = right.fY;
    const float c = right.fX;
    return SkRSXform::Make(c, s, pos.fX + -c * ofs.fX + s * ofs.fY, pos.fY + -s * ofs.fX + -c * ofs.fY);
}

inline SkVector right_to_up(SkVector right) {
    return { right.fY, -right.fX };
}

inline SkVector up_to_right(SkVector up) {
    return { -up.fY, up.fX };
}

}

struct SkRangedFloat {
    float eval(SkRandom& random) { return random.nextRangeF(fMin, fMax); }
    float* vec() { return &fMin; }

    template <typename Visitor>
    void reflect(Visitor visit) {
        visit(fMin, "Min");
        visit(fMax, "Max");
    }

    float fMin;
    float fMax;
};

class SkSpriteSheet {
public:
    SkSpriteSheet() : fImage(nullptr), fColumns(0), fRows(0) {}

    SkSpriteSheet(sk_sp<SkImage> image, int numCols, int numRows)
            : fImage(std::move(image))
            , fColumns(numCols)
            , fRows(numRows) {
        int w = fImage->width();
        int h = fImage->height();
        SkASSERT(w % numCols == 0);
        SkASSERT(h % numRows == 0);
        fRect = SkRect::MakeIWH(w / numCols, h / numRows);
    }

    SkPoint center() const {
        return { fRect.width() * 0.5f, fRect.height() * 0.5f };
    }

    int count() const { return fColumns * fRows; }

    SkRect rect(int i) const {
        SkASSERT(i >= 0 && i < this->count());
        int row = i / fColumns;
        int col = i % fColumns;
        return fRect.makeOffset(col * fRect.width(), row * fRect.height());
    }

    const SkImage* image() const { return fImage.get(); }

private:
    sk_sp<SkImage> fImage;
    int fColumns;
    int fRows;
    SkRect fRect;
};

///////////////////////////////////////////////////////////////////////////////

class SkParticleEmitter : public SkRefCnt {
public:
    virtual void emit(SkRandom&, SkPoint* position, SkVector* right) const = 0;
};

class SkCircleEmitter : public SkParticleEmitter {
public:
    SkCircleEmitter(SkPoint center, SkScalar radius) : fCenter(center), fRadius(radius) {}

    void emit(SkRandom& random, SkPoint* position, SkVector* right) const override {
        SkVector v;
        do {
            v.fX = random.nextRangeScalar(-1, 1);
            v.fY = random.nextRangeScalar(-1, 1);
        } while (v.distanceToOrigin() > 1);
        *right = up_to_right(v);
        if (!right->normalize()) {
            right->set(1, 0);
        }
        *position = fCenter + (v * fRadius);
    }

    template <typename Visitor>
    void reflect(Visitor visit) {
        INHERITED::reflect(visit);
        visit(fCenter, "Center");
        visit(fRadius, "Radius");
    }

private:
    SkPoint  fCenter;
    SkScalar fRadius;

    typedef SkParticleEmitter INHERITED;
};

class SkPathEmitter : public SkParticleEmitter {
public:
    SkPathEmitter(const SkPath& path) {
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

    void emit(SkRandom& random, SkPoint* position, SkVector* right) const override {
        if (fContours.count() == 0) {
            *position = SkPoint{ 0, 0 };
            *right = { 1, 0 };
            return;
        }

        SkScalar len = random.nextRangeScalar(0, fTotalLength);
        int idx = 0;
        while (idx < fContours.count() && len > fLengths[idx]) {
            len -= fLengths[idx++];
        }
        SkPathMeasure cm(fContours[idx], false);
        cm.getPosTan(len, position, right);
    }

private:
    SkScalar fTotalLength;
    SkTArray<SkPath> fContours;
    SkTArray<SkScalar> fLengths;
};

///////////////////////////////////////////////////////////////////////////////

class SkParticleEffect : public SkRefCnt {
public:
    struct Params {
        int fMaxCount;
        int fRate;
        SkRangedFloat fLifetime;
        SkColor4f fStartColor;
        SkColor4f fEndColor;
    };

    typedef std::function<void(SkRandom&, SkPoint&, SkVector&, SkVector&)> InitParticleFn;
    typedef std::function<void(SkRandom&, SkPoint&, SkVector&, SkVector&, float)> UpdateParticleFn;

    SkParticleEffect(const SkSpriteSheet& sprite, const Params& params)
            : fSprite(std::move(sprite))
            , fParams(params)
            , fCount(0)
            , fLastTime(-1.0f)
            , fSpawnRemainder(0.0f) {
        fParticles.reset(new Particle[fParams.fMaxCount]);
        fXforms.reset(new SkRSXform[fParams.fMaxCount]);
        fSpriteRects.reset(new SkRect[fParams.fMaxCount]);
        fColors.reset(new SkColor[fParams.fMaxCount]);

        // Set all particles to not yet alive
        for (int i = 0; i < fParams.fMaxCount; ++i) {
            fParticles[i].fTimeOfDeath = -1.0f;
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

        float elapsed = static_cast<float>(now - fLastTime);
        fLastTime = now;

        Sk4f startColor = Sk4f::Load(fParams.fStartColor.vec());
        Sk4f colorScale = Sk4f::Load(fParams.fEndColor.vec()) - startColor;

        // Age/update old particles
        for (int i = 0; i < fCount; ++i) {
            if (now > fParticles[i].fTimeOfDeath) {
                // NOTE: This is fast, but doesn't preserve drawing order. Could be a problem...
                fParticles[i]   = fParticles[fCount - 1];
                fSpriteRects[i] = fSpriteRects[fCount - 1];
                fColors[i]      = fColors[fCount - 1];
                --i;
                --fCount;
                continue;
            }

            // Compute fraction of lifetime that's elapsed
            float t = static_cast<float>((now - fParticles[i].fTimeOfBirth) /
                      (fParticles[i].fTimeOfDeath - fParticles[i].fTimeOfBirth));

            // Set sprite rect by lifetime
            int frame = static_cast<int>(t * fSprite.count() + 0.5);
            frame = SkTPin(frame, 0, fSprite.count() - 1);
            fSpriteRects[i] = fSprite.rect(frame);

            // Set color by lifetime
            fColors[i] = Sk4f_toL32(swizzle_rb(startColor + (colorScale * t)));

            updateParticle(random, fParticles[i].fPosition, fParticles[i].fRight,
                           fParticles[i].fVelocity, elapsed);

//            SkVector up = right_to_up(fXforms[i].fSCos, fXforms[i].fSSin);
//            fXforms[i].fTx += up.fX * elapsed * 50;
//            fXforms[i].fTy += up.fY * elapsed * 50;
//            fXforms[i].fTx += random.nextRangeScalar(-15.0f, 15.0f) * elapsed;
//            fXforms[i].fTy -= random.nextRangeScalar(15.0f, 40.0f) * elapsed;
        }

        // Spawn new particles
        float desired = fParams.fRate * elapsed + fSpawnRemainder;
        int numToSpawn = sk_float_round2int(desired);
        fSpawnRemainder = desired - numToSpawn;
        numToSpawn = SkTPin(numToSpawn, 0, fParams.fMaxCount - fCount);
        for (int i = 0; i < numToSpawn; ++i) {
            fParticles[fCount].fTimeOfBirth = now;
            fParticles[fCount].fTimeOfDeath = now + fParams.fLifetime.eval(random);
            emitter->emit(random, &fParticles[fCount].fPosition, &fParticles[fCount].fRight);
            fParticles[fCount].fScale = 1.0f;
            fParticles[fCount].fVelocity = SkPoint{ 0, 0 };
            fSpriteRects[fCount] = fSprite.rect(0);
            initParticle(random, fParticles[fCount].fPosition, fParticles[fCount].fRight,
                         fParticles[fCount].fVelocity);
            fCount++;
        }

        // Re-generate all xforms
        SkPoint ofs = fSprite.center();
        for (int i = 0; i < fCount; ++i) {
            // TODO: Include scale
            fXforms[i] = MakeXform(fParticles[i].fPosition, fParticles[i].fRight, ofs);
        }
    }

    void draw(SkCanvas* canvas) {
        canvas->drawAtlas(fSprite.image(), fXforms.get(), fSpriteRects.get(), fColors.get(), fCount,
                          SkBlendMode::kModulate, nullptr, nullptr);
    }

    int getRate() const { return fParams.fRate; }
    void setRate(int rate) { fParams.fRate = rate; }

    SkRangedFloat getLifetime() const { return fParams.fLifetime; }
    void setLifetime(SkRangedFloat lifetime) { fParams.fLifetime = lifetime; }

    SkColor4f getStartColor() const { return fParams.fStartColor; }
    SkColor4f getEndColor() const { return fParams.fEndColor; }
    void setStartColor(SkColor4f color) { fParams.fStartColor = color; }
    void setEndColor(SkColor4f color) { fParams.fEndColor = color; }

private:
    struct Particle {
        double fTimeOfBirth;
        double fTimeOfDeath;

        SkPoint  fPosition;
        SkVector fVelocity;
        SkVector fRight;  // Orientation
        SkScalar fScale;

        // Texture coord rects and colors are stored in parallel arrays for drawAtlas.
    };

    SkSpriteSheet fSprite;
    Params fParams;
    int fCount;
    double fLastTime;
    float fSpawnRemainder;

    std::unique_ptr<Particle[]>  fParticles;
    std::unique_ptr<SkRSXform[]> fXforms;
    std::unique_ptr<SkRect[]>    fSpriteRects;
    std::unique_ptr<SkColor[]>   fColors;
};

static int preset = 0;

static sk_sp<SkParticleEffect> makePreset(int idx) {
    switch (idx) {
        case 1: {
            auto image = GetResourceAsImage("images/explosion_sprites.png");
            SkSpriteSheet sprite(std::move(image), 8, 8);

            SkParticleEffect::Params params = {
                32,
                2,
                SkRangedFloat { 1.0f, 3.0f },
                SkColor4f { 1.0f, 1.0f, 1.0f, 1.0f },
                SkColor4f { 1.0f, 1.0f, 1.0f, 1.0f },
            };
            return sk_sp<SkParticleEffect>(new SkParticleEffect(sprite, params));
        }

        case 0:
        default: {
            auto surface = SkSurface::MakeRasterN32Premul(1, 1);
            surface->getCanvas()->clear(SkColorSetARGB(128, 255, 255, 255));
            SkSpriteSheet sprite(surface->makeImageSnapshot(), 1, 1);

            SkParticleEffect::Params params = {
                4096,
                2000,
                SkRangedFloat { 1.0f, 3.0f },
                SkColor4f { 1.0f, 0.35f, 0.0f, 1.0f },
                SkColor4f { 0.2f, 0.25f, 0.5f, 1.0f },
            };
            return sk_sp<SkParticleEffect>(new SkParticleEffect(sprite, params));
        }
    }
}

ParticlesSlide::ParticlesSlide() {
    fName = "Particles";
    fDrawPath = false;

    fEffect = makePreset(preset);

    strcpy(fEmitterText, ".");
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
    fEmitter.reset(new SkPathEmitter(fPath));
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

        if (ImGui::DragInt("Preset", &preset, 1.0f, 0, 1)) {
            fEffect = makePreset(preset);
        }

        int rate = fEffect->getRate();
        if (ImGui::DragInt("Rate", &rate, 1.0f, 0, 4000)) {
            fEffect->setRate(rate);
        }

        SkRangedFloat lifetime = fEffect->getLifetime();
        if (ImGui::DragFloat2("Lifetime", lifetime.vec(), 1.0f, 0.0f, 30.0f)) {
            fEffect->setLifetime(lifetime);
        }

        SkColor4f startColor = fEffect->getStartColor();
        if (ImGui::ColorEdit4("Start Color", startColor.vec())) {
            fEffect->setStartColor(startColor);
        }
        SkColor4f endColor = fEffect->getEndColor();
        if (ImGui::ColorEdit4("End Color", endColor.vec())) {
            fEffect->setEndColor(endColor);
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
    auto initParticle = [](SkRandom& random, SkPoint& position, SkVector& right,
                           SkVector& velocity) {
        velocity.fX = random.nextRangeF(-25.0f, 25.0f);
    };
    auto updateParticle = [=](SkRandom& random, SkPoint& position, SkVector& right,
                              SkVector& velocity, float elapsed) {
        velocity.fY += elapsed * 50.0f;

        position.fX += velocity.fX * elapsed;
        position.fY += velocity.fY * elapsed;

        if (position.fY > fFloor) {
            position.fY = fFloor;
            velocity.fY = -velocity.fY * 0.7f;
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
