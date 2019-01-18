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
#include "SkImage.h"
#include "SkJSON.h"
#include "SkJSONWriter.h"
#include "SkNx.h"
#include "SkRect.h"
#include "SkRSXform.h"
#include "SkTArray.h"

#include "imgui.h"

using namespace sk_app;
using namespace skjson;

class SkFieldVisitor;

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

bool TryParse(const Value& v, bool* b) {
    switch (v.getType()) {
    case Value::Type::kNumber:
        *b = SkToBool(*v.as<NumberValue>());
        return true;
    case Value::Type::kBool:
        *b = *v.as<BoolValue>();
        return true;
    default:
        break;
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

    float fMin = 0.0f;
    float fMax = 0.0f;
};

///////////////////////////////////////////////////////////////////////////////

class SkFieldVisitor {
public:
    // Add meta-field struct/flags for defaults, ranges, type hints (angle), etc.

    virtual void visit(const char*, float&) = 0;
    virtual void visit(const char*, int&) = 0;
    virtual void visit(const char*, bool&) = 0;

    virtual void visit(const char*, SkPoint&) = 0;
    virtual void visit(const char*, SkColor4f&) = 0;
    virtual void visit(const char*, SkRangedFloat&) = 0;
};

class SkToJsonVisitor : public SkFieldVisitor {
public:
    SkToJsonVisitor(SkJSONWriter& writer) : fWriter(writer) {}

    // Primitives
    void visit(const char* name, float& f) override { fWriter.appendFloat(name, f); }
    void visit(const char* name, int& i)   override { fWriter.appendS32(name, i); }
    void visit(const char* name, bool& b)  override { fWriter.appendBool(name, b); }

    // Compound types
    void visit(const char* name, SkPoint& p) override {
        fWriter.beginObject(name, false);
        fWriter.appendFloat("x", p.fX);
        fWriter.appendFloat("y", p.fY);
        fWriter.endObject();
    }

    void visit(const char* name, SkColor4f& c) override {
        fWriter.beginArray(name, false);
        fWriter.appendFloat(c.fR);
        fWriter.appendFloat(c.fG);
        fWriter.appendFloat(c.fB);
        fWriter.appendFloat(c.fA);
        fWriter.endArray();
    }

    void visit(const char* name, SkRangedFloat& rf) override {
        fWriter.beginObject(nullptr, false);
        fWriter.appendFloat("min", rf.fMin);
        fWriter.appendFloat("max", rf.fMax);
        fWriter.endObject();
    }

private:
    SkJSONWriter& fWriter;
};

class SkFromJsonVisitor : public SkFieldVisitor {
public:
    SkFromJsonVisitor(const Value& v) : fValue(v) {}

    void visit(const char* name, float& f) override { f = ParseOrDefault(get(name), 0.0f); }
    void visit(const char* name, int& i)   override { i = ParseOrDefault(get(name), 0); }
    void visit(const char* name, bool& b)  override { b = ParseOrDefault(get(name), false); }

    void visit(const char* name, SkPoint& p) override {
        if (const ObjectValue* obj = get(name)) {
            p.fX = ParseOrDefault((*obj)["x"], 0.0f);
            p.fY = ParseOrDefault((*obj)["y"], 0.0f);
        }
    }

    void visit(const char* name, SkColor4f& c) override {
        const ArrayValue* arr = get(name);
        if (arr && arr->size() == 4) {
            c.fR = ParseOrDefault((*arr)[0], 0.0f);
            c.fG = ParseOrDefault((*arr)[1], 0.0f);
            c.fB = ParseOrDefault((*arr)[2], 0.0f);
            c.fA = ParseOrDefault((*arr)[3], 0.0f);
        }
    }

    void visit(const char* name, SkRangedFloat& rf) override {
        if (const ObjectValue* obj = get(name)) {
            rf.fMin = ParseOrDefault((*obj)["min"], 0.0f);
            rf.fMax = ParseOrDefault((*obj)["max"], 0.0f);
        }
    }

private:
    const Value& get(const char* name) const {
        if (!name) { return fValue; }
        if (fValue.is<ObjectValue>()) {
            return fValue.as<ObjectValue>()[name];
        } else {
            static NullValue gNull;
            return gNull;
        }
    }

    const Value& fValue;
};

class SkGuiVisitor : public SkFieldVisitor {
public:
    void visit(const char* name, float& f) override { ImGui::DragFloat(name, &f); }
    void visit(const char* name, int& i)   override { ImGui::DragInt(name, &i); }
    void visit(const char* name, bool& b)  override { ImGui::Checkbox(name, &b); }

    void visit(const char* name, SkPoint& p) override { ImGui::DragFloat2(name, &p.fX); }
    void visit(const char* name, SkColor4f& c) override { ImGui::ColorEdit4(name, c.vec()); }
    void visit(const char* name, SkRangedFloat& rf) override { ImGui::DragFloat2(name, &rf.fMin); }
};

///////////////////////////////////////////////////////////////////////////////

class SkParticleEmitter : public SkRefCnt {
public:
    virtual void emit(SkRandom&, SkPoint* position, SkVector* right) const = 0;
    virtual void toJson(SkJSONWriter&) const = 0;
    static sk_sp<SkParticleEmitter> FromJson(const Value& v);
    virtual void drawUI() = 0;

    virtual void visitFields(SkFieldVisitor*) = 0;
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

    void visitFields(SkFieldVisitor* v) override {
        // Visit "Type" tag? Handle this in the sk_sp visitor for the parent object?
        v->visit("center", fCenter);
        v->visit("radius", fRadius);
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

    void visitFields(SkFieldVisitor* v) override {
        v->visit("P1", fP1);
        v->visit("P2", fP2);
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

struct InitialVelocityParams {
    float fAngle = 0.0f;
    float fAngleSpread = 0.0f;
    SkRangedFloat fStrength;
    bool fBidirectional = false;

    SkVector eval(SkRandom& random) {
        float angle = fAngle + fAngleSpread * (random.nextF() - 0.5f);
        SkScalar c, s = SkScalarSinCos(angle, &c);
        float strength = fStrength.eval(random);
        if (fBidirectional && random.nextBool()) {
            strength = -strength;
        }
        return SkVector{ c * strength, s * strength };
    }

    void toJson(SkJSONWriter& w) const {
        w.beginObject();
        w.appendFloat("angle", fAngle);
        w.appendFloat("spread", fAngleSpread);
        w.appendName("strength"); fStrength.toJson(w);
        w.appendBool("bidi", fBidirectional);
        w.endObject();
    }

    static InitialVelocityParams FromJson(const Value& v) {
        InitialVelocityParams ivp;
        if (const ObjectValue* obj = v) {
            ivp.fAngle = ParseOrDefault((*obj)["angle"], 0.0f);
            ivp.fAngleSpread = ParseOrDefault((*obj)["spread"], 0.0f);
            ivp.fStrength = SkRangedFloat::FromJson((*obj)["strength"]);
            ivp.fBidirectional = ParseOrDefault((*obj)["bidi"], false);
        }
        return ivp;
    }

    void drawUI() {
        ImGui::SliderAngle("Angle", &fAngle, 0.0f);
        ImGui::SliderAngle("Spread", &fAngleSpread, 0.0f);
        ImGui::DragFloat2("Strength", fStrength.vec());
        ImGui::Checkbox("Bidirectional", &fBidirectional);
    }

    void visitFields(SkFieldVisitor* v) {
        v->visit("Angle", fAngle);
        v->visit("Spread", fAngleSpread);
        v->visit("Strength", fStrength);
        v->visit("Bidirectional", fBidirectional);
    }
};

// TODO: Make things variable over time of effect (rather than over time of particle?)
// TODO: Make more things ranged at start (eg, colors). Need to remember what values we picked.

class SkParticleEffectParams : public SkRefCnt {
public:
    int fMaxCount;
    int fRate;
    SkRangedFloat fLifetime;
    SkColor4f fStartColor;
    SkColor4f fEndColor;

    // TODO: Add local vs. world copies of these
    // Initial velocity controls
    InitialVelocityParams fVelocity;

    // Sprite image parameters
    // TODO: Move sprite stuff in here, out of effect
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

        w.appendName("velocity"); fVelocity.toJson(w);

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

        params->fVelocity = InitialVelocityParams::FromJson(obj["velocity"]);

        params->fImage = ParseOrDefault(obj["image"], SkString());
        params->fImageCols = ParseOrDefault(obj["imageCols"], 1);
        params->fImageRows = ParseOrDefault(obj["imageRows"], 1);

        params->fEmitter = SkParticleEmitter::FromJson(obj["emitter"]);
        return params;
    }

    void visitFields(SkFieldVisitor* v) {
        v->visit("MaxCount", fMaxCount);
        v->visit("Rate", fRate);
        v->visit("Life", fLifetime);
        v->visit("StartColor", fStartColor);
        v->visit("EndColor", fEndColor);

        // Push/pop object? Could be hint to GUI. Or base-class template for unknown types that
        // handles that and calls visitFields?
        // XXX
        fVelocity.visitFields(v);
    }

    void drawUI() {
        // Note that we don't allow runtime editing of max count yet
        ImGui::DragInt("Rate", &fRate, 1.0f, 0, 4000);
        ImGui::DragFloat2("Lifetime", fLifetime.vec(), 1.0f, 0.0f, 30.0f);
        ImGui::ColorEdit4("Start Color", fStartColor.vec());
        ImGui::ColorEdit4("End Color", fEndColor.vec());

        fVelocity.drawUI();

        // No editing of image params yet, either

        fEmitter->drawUI();
    }
};

class SkParticleEffect : public SkRefCnt {
public:
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

    void update(SkRandom& random, const SkAnimTimer& timer, UpdateParticleFn&& updateParticle) {
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
            fParticles[fCount].fVelocity = fParams->fVelocity.eval(random);
            fSpriteRects[fCount] = this->spriteRect(0);
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
        SkPaint paint;
        paint.setFilterQuality(SkFilterQuality::kMedium_SkFilterQuality);
        canvas->drawAtlas(fImage, fXforms.get(), fSpriteRects.get(), fColors.get(), fCount,
                          SkBlendMode::kModulate, nullptr, &paint);
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

static sk_sp<SkParticleEffectParams> LoadEffectParams(const char* filename) {
    auto fileData = SkData::MakeFromFileName(filename);
    if (fileData) {
        DOM dom(static_cast<const char*>(fileData->data()), fileData->size());
        return SkParticleEffectParams::FromJson(dom.root());
    }
    return nullptr;
}

ParticlesSlide::ParticlesSlide() {
    fName = "Particles";
    fEffect.reset(new SkParticleEffect(LoadEffectParams("resources/particles/default.json")));
}

ParticlesSlide::~ParticlesSlide() {}

SkISize ParticlesSlide::getDimensions() const  {
    return SkISize::Make(500, 500);
}

void ParticlesSlide::draw(SkCanvas* canvas) {
    canvas->clear(0);

    if (ImGui::Begin("Particles")) {
        static char filename[64] = "resources/particles/default.json";
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

        fEffect->getParams()->drawUI();
    }
    ImGui::End();

    fEffect->draw(canvas);
}

bool ParticlesSlide::animate(const SkAnimTimer& timer) {
    auto updateParticle = [=](SkRandom& random, SkPoint& position, SkVector& right,
                              SkVector& velocity, float elapsed) {
//        velocity.fY += elapsed * 50.0f;

        position.fX += velocity.fX * elapsed;
        position.fY += velocity.fY * elapsed;
    };
    fEffect->update(fRandom, timer, updateParticle);
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
