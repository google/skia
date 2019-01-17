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
#include "SkTHash.h"
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
using namespace skjson;

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

bool TryParse(const Value& v, float* f) {
    if (const NumberValue* num = v) {
        *f = static_cast<float>(**num);
        return true;
    }
    return false;
}

bool TryParse(const Value& v, int* i) {
    if (const NumberValue* num = v) {
        double dbl = **num;
        *i = static_cast<int>(dbl);
        return static_cast<double>(*i) == dbl;
    }
    return false;
}

bool TryParse(const Value& v, SkString* s) {
    if (const StringValue* str = v) {
        s->set(str->begin(), str->size());
        return true;
    }
    return false;
}

template <typename T>
T ParseOrDefault(const Value& v, const T& defaultValue) {
    T result;
    if (!TryParse(v, &result)) {
        result = defaultValue;
    }
    return result;
}

static void SkPoint_toJson(SkJSONWriter& w, const char* name, const SkPoint& p) {
    w.beginObject(name, false);
    w.appendFloat("x", p.fX);
    w.appendFloat("y", p.fY);
    w.endObject();
}

static SkPoint SkPoint_FromJson(const Value& v) {
    if (!v.is<ObjectValue>()) {
        return SkPoint();
    }
    const ObjectValue& obj = v.as<ObjectValue>();
    return SkPoint { ParseOrDefault(obj["x"], 0.0f), ParseOrDefault(obj["y"], 0.0f) };
}

static void SkColor4f_toJson(SkJSONWriter& w, const char* name, const SkColor4f& c) {
    w.beginArray(name, false);
    w.appendFloat(c.fR);
    w.appendFloat(c.fG);
    w.appendFloat(c.fB);
    w.appendFloat(c.fA);
    w.endArray();
}

static SkColor4f SkColor4f_FromJson(const Value& v) {
    if (!v.is<ArrayValue>() || v.as<ArrayValue>().size() != 4) {
        return SkColor4f();
    }

    const ArrayValue& arr = v.as<ArrayValue>();
    return SkColor4f {
        ParseOrDefault(arr[0], 0.0f),
        ParseOrDefault(arr[1], 0.0f),
        ParseOrDefault(arr[2], 0.0f),
        ParseOrDefault(arr[3], 0.0f)
    };
}

}

struct SkRangedFloat {
    float eval(SkRandom& random) { return random.nextRangeF(fMin, fMax); }
    float* vec() { return &fMin; }

    void toJson(SkJSONWriter& w) const {
        w.beginObject(nullptr, false);
        w.appendFloat("min", fMin);
        w.appendFloat("max", fMax);
        w.endObject();
    }
    static SkRangedFloat FromJson(const Value& v) {
        if (!v.is<ObjectValue>()) {
            return SkRangedFloat();
        }

        const ObjectValue& obj = v.as<ObjectValue>();
        return SkRangedFloat {
            ParseOrDefault(obj["min"], 0.0f),
            ParseOrDefault(obj["max"], 0.0f)
        };
    }

    float fMin;
    float fMax;
};

///////////////////////////////////////////////////////////////////////////////

class SkParticleEmitter : public SkRefCnt {
public:
    virtual void emit(SkRandom&, SkPoint* position, SkVector* right) const = 0;
    virtual void toJson(SkJSONWriter&) const = 0;
    static sk_sp<SkParticleEmitter> FromJson(const Value& v);
    virtual void drawUI() = 0;
};

class SkCircleEmitter : public SkParticleEmitter {
public:
    SkCircleEmitter(SkPoint center, SkScalar radius) : fCenter(center), fRadius(radius) {}

    void emit(SkRandom& random, SkPoint* position, SkVector* right) const override {
        SkVector v;
        do {
            v.fX = random.nextSScalar1();
            v.fY = random.nextSScalar1();
        } while (v.distanceToOrigin() > 1);
        *right = up_to_right(v);
        if (!right->normalize()) {
            right->set(1, 0);
        }
        *position = fCenter + (v * fRadius);
    }

    void toJson(SkJSONWriter& w) const override {
        w.beginObject();
        w.appendString("type", "Circle");
        SkPoint_toJson(w, "center", fCenter);
        w.appendFloat("radius", fRadius);
        w.endObject();
    }
    static sk_sp<SkParticleEmitter> FromJson(const Value& v) {
        const ObjectValue& obj = v.as<ObjectValue>();
        return sk_sp<SkCircleEmitter>(new SkCircleEmitter(
            SkPoint_FromJson(obj["center"]),
            ParseOrDefault(obj["radius"], 0.0f)
        ));
    }

    void drawUI() override {
        ImGui::DragFloat2("Center", &fCenter.fX);
        ImGui::DragFloat("Radius", &fRadius);
    }

private:
    SkPoint  fCenter;
    SkScalar fRadius;
};

class SkLineEmitter : public SkParticleEmitter {
public:
    SkLineEmitter(SkPoint p1, SkPoint p2) : fP1(p1), fP2(p2) {}

    void emit(SkRandom& random, SkPoint* position, SkVector* right) const override {
        *right = (fP2 - fP1);
        if (!right->normalize()) {
            right->set(1, 0);
        }
        *position = fP1 + (fP2 - fP1) * random.nextUScalar1();
    }

    void toJson(SkJSONWriter& w) const override {
        w.beginObject();
        w.appendString("type", "Line");
        SkPoint_toJson(w, "P1", fP1);
        SkPoint_toJson(w, "P2", fP2);
        w.endObject();
    }
    static sk_sp<SkParticleEmitter> FromJson(const Value& v) {
        const ObjectValue& obj = v.as<ObjectValue>();
        return sk_sp<SkParticleEmitter>(new SkLineEmitter(
            SkPoint_FromJson(obj["P1"]),
            SkPoint_FromJson(obj["P2"])
        ));
    }

    void drawUI() override {
        ImGui::DragFloat2("P1", &fP1.fX);
        ImGui::DragFloat2("P2", &fP2.fX);
    }

private:
    SkPoint fP1;
    SkPoint fP2;
};

sk_sp<SkParticleEmitter> SkParticleEmitter::FromJson(const Value& v) {
    if (const ObjectValue* obj = v) {
        const StringValue* typeString = (*obj)["type"];
        if (typeString) {
            const char* type = typeString->begin();
            if (strcmp(type, "Circle") == 0) {
                return SkCircleEmitter::FromJson(v);
            } else if (strcmp(type, "Line") == 0) {
                return SkLineEmitter::FromJson(v);
            }
        }
    }
    return nullptr;
}

#if 0
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
#endif

///////////////////////////////////////////////////////////////////////////////

class SkParticleEffectParams : public SkRefCnt {
public:
    int fMaxCount;
    int fRate;
    SkRangedFloat fLifetime;
    SkColor4f fStartColor;
    SkColor4f fEndColor;

    // Sprite image parameters
    SkString fImage;
    int fImageCols;
    int fImageRows;

    // Emitter shape & parameters
    sk_sp<SkParticleEmitter> fEmitter;

    void toJson(SkJSONWriter& w) const {
        w.beginObject();
        w.appendS32("maxCount", fMaxCount);
        w.appendS32("rate", fRate);
        w.appendName("life"); fLifetime.toJson(w);
        SkColor4f_toJson(w, "startColor", fStartColor);
        SkColor4f_toJson(w, "endColor", fEndColor);

        w.appendString("image", fImage.c_str());
        w.appendS32("imageCols", fImageCols);
        w.appendS32("imageRows", fImageRows);

        w.appendName("emitter");
        if (fEmitter) {
            fEmitter->toJson(w);
        }
        w.endObject();
    }

    static sk_sp<SkParticleEffectParams> FromJson(const Value& v) {
        const ObjectValue& obj = v.as<ObjectValue>();
        sk_sp<SkParticleEffectParams> params(new SkParticleEffectParams);
        params->fMaxCount = ParseOrDefault(obj["maxCount"], 100);
        params->fRate = ParseOrDefault(obj["rate"], 0);
        params->fLifetime = SkRangedFloat::FromJson(obj["life"]);
        params->fStartColor = SkColor4f_FromJson(obj["startColor"]);
        params->fEndColor = SkColor4f_FromJson(obj["endColor"]);

        params->fImage = ParseOrDefault(obj["image"], SkString());
        params->fImageCols = ParseOrDefault(obj["imageCols"], 1);
        params->fImageRows = ParseOrDefault(obj["imageRows"], 1);

        params->fEmitter = SkParticleEmitter::FromJson(obj["emitter"]);
        return params;
    }

    void drawUI() {
        // Note that we don't allow runtime editing of max count yet
        ImGui::DragInt("Rate", &fRate, 1.0f, 0, 4000);
        ImGui::DragFloat2("Lifetime", fLifetime.vec(), 1.0f, 0.0f, 30.0f);
        ImGui::ColorEdit4("Start Color", fStartColor.vec());
        ImGui::ColorEdit4("End Color", fEndColor.vec());

        // No editing of image params yet, either

        fEmitter->drawUI();
    }
};

class SkParticleEffect : public SkRefCnt {
public:
    typedef std::function<void(SkRandom&, SkPoint&, SkVector&, SkVector&)> InitParticleFn;
    typedef std::function<void(SkRandom&, SkPoint&, SkVector&, SkVector&, float)> UpdateParticleFn;

    SkParticleEffect(sk_sp<SkParticleEffectParams> params)
            : fParams(std::move(params))
            , fCount(0)
            , fLastTime(-1.0f)
            , fSpawnRemainder(0.0f) {
        fParticles.reset(new Particle[fParams->fMaxCount]);
        fXforms.reset(new SkRSXform[fParams->fMaxCount]);
        fSpriteRects.reset(new SkRect[fParams->fMaxCount]);
        fColors.reset(new SkColor[fParams->fMaxCount]);

        // Set all particles to not yet alive
        for (int i = 0; i < fParams->fMaxCount; ++i) {
            fParticles[i].fTimeOfDeath = -1.0f;
        }

        // Load image, determine sprite rect size
        fImage = GetResourceAsImage(fParams->fImage.c_str());
        int w = fImage->width();
        int h = fImage->height();
        SkASSERT(w % fParams->fImageCols == 0);
        SkASSERT(h % fParams->fImageRows == 0);
        fImageRect = SkRect::MakeIWH(w / fParams->fImageCols, h / fParams->fImageRows);
    }

    void update(SkRandom& random, const SkAnimTimer& timer,
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

        Sk4f startColor = Sk4f::Load(fParams->fStartColor.vec());
        Sk4f colorScale = Sk4f::Load(fParams->fEndColor.vec()) - startColor;

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
            int frame = static_cast<int>(t * this->spriteCount() + 0.5);
            frame = SkTPin(frame, 0, this->spriteCount() - 1);
            fSpriteRects[i] = this->spriteRect(frame);

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
        float desired = fParams->fRate * elapsed + fSpawnRemainder;
        int numToSpawn = sk_float_round2int(desired);
        fSpawnRemainder = desired - numToSpawn;
        numToSpawn = SkTPin(numToSpawn, 0, fParams->fMaxCount - fCount);
        for (int i = 0; i < numToSpawn; ++i) {
            fParticles[fCount].fTimeOfBirth = now;
            fParticles[fCount].fTimeOfDeath = now + fParams->fLifetime.eval(random);
            fParams->fEmitter->emit(random, &fParticles[fCount].fPosition,
                                    &fParticles[fCount].fRight);
            fParticles[fCount].fScale = 1.0f;
            fParticles[fCount].fVelocity = SkPoint{ 0, 0 };
            fSpriteRects[fCount] = this->spriteRect(0);
            initParticle(random, fParticles[fCount].fPosition, fParticles[fCount].fRight,
                         fParticles[fCount].fVelocity);
            fCount++;
        }

        // Re-generate all xforms
        SkPoint ofs = this->spriteCenter();
        for (int i = 0; i < fCount; ++i) {
            // TODO: Include scale
            fXforms[i] = MakeXform(fParticles[i].fPosition, fParticles[i].fRight, ofs);
        }
    }

    void draw(SkCanvas* canvas) {
        canvas->drawAtlas(fImage, fXforms.get(), fSpriteRects.get(), fColors.get(), fCount,
                          SkBlendMode::kModulate, nullptr, nullptr);
    }

    SkParticleEffectParams* getParams() { return fParams.get(); }

private:
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

        SkPoint  fPosition;
        SkVector fVelocity;
        SkVector fRight;  // Orientation
        SkScalar fScale;

        // Texture coord rects and colors are stored in parallel arrays for drawAtlas.
    };

    sk_sp<SkParticleEffectParams> fParams;
    sk_sp<SkImage>                fImage;
    SkRect                        fImageRect;

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
    sk_sp<SkParticleEffectParams> params(new SkParticleEffectParams);

    switch (idx) {
        case 1: {
            params->fMaxCount = 32;
            params->fRate = 2;
            params->fLifetime = SkRangedFloat{ 1.0f, 3.0f };
            params->fStartColor = SkColor4f{ 1.0f, 1.0f, 1.0f, 1.0f };
            params->fEndColor = SkColor4f{ 1.0f, 1.0f, 1.0f, 1.0f };

            params->fImage = "images/explosion_sprites.png";
            params->fImageCols = params->fImageRows = 8;

            params->fEmitter.reset(new SkCircleEmitter({ 0,0 }, 50));
            break;
        }

        case 0:
        default: {
            params->fMaxCount = 4096;
            params->fRate = 2000;
            params->fLifetime = SkRangedFloat{ 1.0f, 3.0f };
            params->fStartColor = SkColor4f{ 1.0f, 0.35f, 0.0f, 1.0f };
            params->fEndColor = SkColor4f{ 0.2f, 0.25f, 0.5f, 1.0f };

            params->fImage = "images/half-transparent-white-pixel.png";
            params->fImageCols = params->fImageRows = 1;

            params->fEmitter.reset(new SkLineEmitter({ 0,0 }, { 100,0 }));
            break;
        }
    }

    return sk_sp<SkParticleEffect>(new SkParticleEffect(std::move(params)));

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
//    fEmitter.reset(new SkCircleEmitter({ 100, 100 }, 50));
}

ParticlesSlide::~ParticlesSlide() {}

SkISize ParticlesSlide::getDimensions() const  {
    return SkISize::Make(500, 500);
}

void ParticlesSlide::draw(SkCanvas* canvas) {
    canvas->clear(0);

    if (ImGui::Begin("Particles")) {
        static char filename[64] = "particles.json";
        ImGui::InputText("Filename", filename, sizeof(filename));
        if (ImGui::Button("Load")) {
            auto fileData = SkData::MakeFromFileName(filename);
            if (fileData) {
                DOM dom(static_cast<const char*>(fileData->data()), fileData->size());
                auto newParams = SkParticleEffectParams::FromJson(dom.root());
                if (newParams) {
                    fEffect.reset(new SkParticleEffect(std::move(newParams)));
                }
            }
        }
        ImGui::SameLine();

        if (ImGui::Button("Save")) {
            SkFILEWStream fileStream(filename);
            if (fileStream.isValid()) {
                SkJSONWriter writer(&fileStream, SkJSONWriter::Mode::kPretty);
                fEffect->getParams()->toJson(writer);
                writer.flush();
                fileStream.flush();
            } else {
                SkDebugf("Failed to open file\n");
            }
        }

#if 0
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
#endif
        if (ImGui::DragInt("Preset", &preset, 1.0f, 0, 1)) {
            fEffect = makePreset(preset);
        }

        fEffect->getParams()->drawUI();

        ImGui::DragFloat("Floor", &fFloor);
        ImGui::Checkbox("Show Path", &fDrawPath);
    }
    ImGui::End();

    fEffect->draw(canvas);

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
    fEffect->update(fRandom, timer, initParticle, updateParticle);
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
