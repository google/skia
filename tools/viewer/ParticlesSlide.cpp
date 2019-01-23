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
#include "SkPathMeasure.h"
#include "SkReflected.h"
#include "SkRect.h"
#include "SkRSXform.h"
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

}

struct SkRangedFloat {
    float eval(SkRandom& random) { return random.nextRangeF(fMin, fMax); }
    float* vec() { return &fMin; }

    float fMin = 0.0f;
    float fMax = 0.0f;

    void visitFields(SkFieldVisitor* v) {
        v->visit("min", fMin);
        v->visit("max", fMax);
    }
};

///////////////////////////////////////////////////////////////////////////////

class SkParticleEmitter : public SkReflected {
public:
    REFLECTED_ABSTRACT(SkParticleEmitter, SkReflected)

    virtual void emit(SkRandom&, SkPoint* position, SkVector* right) const = 0;
};

class SkCircleEmitter : public SkParticleEmitter {
public:
    SkCircleEmitter(SkPoint center = { 0.0f, 0.0f }, SkScalar radius = 0.0f)
        : fCenter(center), fRadius(radius) {}

    REFLECTED(SkCircleEmitter, SkParticleEmitter)

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

    void visitFields(SkFieldVisitor* v) override {
        v->visit("Center", fCenter);
        v->visit("Radius", fRadius);
    }

private:
    SkPoint  fCenter;
    SkScalar fRadius;
};

class SkLineEmitter : public SkParticleEmitter {
public:
    SkLineEmitter(SkPoint p1 = { 0.0f, 0.0f }, SkPoint p2 = { 0.0f, 0.0f }) : fP1(p1), fP2(p2) {}

    REFLECTED(SkLineEmitter, SkParticleEmitter)

    void emit(SkRandom& random, SkPoint* position, SkVector* right) const override {
        *right = (fP2 - fP1);
        if (!right->normalize()) {
            right->set(1, 0);
        }
        *position = fP1 + (fP2 - fP1) * random.nextUScalar1();
    }

    void visitFields(SkFieldVisitor* v) override {
        v->visit("P1", fP1);
        v->visit("P2", fP2);
    }

private:
    SkPoint fP1;
    SkPoint fP2;
};

class SkTextEmitter : public SkParticleEmitter {
public:
    SkTextEmitter(const char* text = "", SkScalar fontSize = 96)
            : fText(text), fFontSize(fontSize) {
        this->rebuild();
    }

    REFLECTED(SkTextEmitter, SkParticleEmitter)

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

    void visitFields(SkFieldVisitor* v) override {
        SkString oldText = fText;
        SkScalar oldSize = fFontSize;

        v->visit("Text", fText);
        v->visit("FontSize", fFontSize);

        if (fText != oldText || fFontSize != oldSize) {
            this->rebuild();
        }
    }

private:
    SkString fText;
    SkScalar fFontSize;

    void rebuild() {
        fTotalLength = 0;
        fContours.reset();
        fLengths.reset();

        if (fText.isEmpty()) {
            return;
        }

        SkFont font(sk_tool_utils::create_portable_typeface());
        font.setSize(fFontSize);
        SkPath path;
        SkTextUtils::GetPath(fText.c_str(), fText.size(), kUTF8_SkTextEncoding, 0, 0, font, &path);
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

    // Cached
    SkScalar fTotalLength;
    SkTArray<SkPath> fContours;
    SkTArray<SkScalar> fLengths;
};

///////////////////////////////////////////////////////////////////////////////

class SkParticleAffector : public SkReflected {
public:
    REFLECTED_ABSTRACT(SkParticleAffector, SkReflected)

    virtual void apply(SkRandom& random, SkPoint& position, SkVector& right, SkVector& velocity,
                       float elapsed) = 0;
};

class SkDirectionalForceAffector : public SkParticleAffector {
public:
    SkDirectionalForceAffector(SkVector force = { 0.0f, 0.0f }) : fForce(force) {}

    REFLECTED(SkDirectionalForceAffector, SkParticleAffector)

    void apply(SkRandom&, SkPoint&, SkVector&, SkVector& velocity, float elapsed) override {
        velocity += fForce * elapsed;
    }

    void visitFields(SkFieldVisitor* v) override {
        v->visit("Force", fForce);
    }

private:
    SkVector fForce;
};

class SkPointForceAffector : public SkParticleAffector {
public:
    SkPointForceAffector(SkPoint point = { 0.0f, 0.0f }, SkScalar constant = 0.0f,
                         SkScalar invSquare = 0.0f)
            : fPoint(point), fConstant(constant), fInvSquare(invSquare) {}

    REFLECTED(SkPointForceAffector, SkParticleAffector)

    void apply(SkRandom&, SkPoint& position, SkVector&, SkVector& velocity, float elapsed) {
        SkVector toPoint = fPoint - position;
        SkScalar lenSquare = toPoint.dot(toPoint);
        toPoint.normalize();
        velocity += toPoint * (fConstant + (fInvSquare / lenSquare)) * elapsed;
    }

    void visitFields(SkFieldVisitor* v) override {
        v->visit("Point", fPoint);
        v->visit("Constant", fConstant);
        v->visit("InvSquare", fInvSquare);
    }

private:
    SkPoint  fPoint;
    SkScalar fConstant;
    SkScalar fInvSquare;
};

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

    void visitFields(SkFieldVisitor* v) {
        v->visit("Angle", fAngle, kAngle_Field);
        v->visit("Spread", fAngleSpread, kAngle_Field);
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

    // Update rules
    SkTArray<sk_sp<SkParticleAffector>> fAffectors;

    void visitFields(SkFieldVisitor* v) {
        v->visit("MaxCount", fMaxCount);
        v->visit("Rate", fRate);
        v->visit("Life", fLifetime);
        v->visit("StartColor", fStartColor);
        v->visit("EndColor", fEndColor);

        v->visit("Velocity", fVelocity);

        v->visit("Image", fImage);
        v->visit("ImageCols", fImageCols);
        v->visit("ImageRows", fImageRows);

        v->visit("Emitter", fEmitter);

        v->visit("Affectors", fAffectors);
    }
};

class SkParticleEffect : public SkRefCnt {
public:
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

    void update(SkRandom& random, const SkAnimTimer& timer) {
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

            for (auto affector : fParams->fAffectors) {
                if (affector) {
                    affector->apply(random, fParticles[i].fPosition, fParticles[i].fRight,
                                    fParticles[i].fVelocity, elapsed);
                }
            }

//            SkVector up = right_to_up(fXforms[i].fSCos, fXforms[i].fSSin);
//            fXforms[i].fTx += up.fX * elapsed * 50;
//            fXforms[i].fTy += up.fY * elapsed * 50;
//            fXforms[i].fTx += random.nextRangeScalar(-15.0f, 15.0f) * elapsed;
//            fXforms[i].fTy -= random.nextRangeScalar(15.0f, 40.0f) * elapsed;

            // Integrate position
            fParticles[i].fPosition += fParticles[i].fVelocity * elapsed;
        }

        // Spawn new particles
        float desired = fParams->fRate * elapsed + fSpawnRemainder;
        int numToSpawn = sk_float_round2int(desired);
        fSpawnRemainder = desired - numToSpawn;
        numToSpawn = SkTPin(numToSpawn, 0, fParams->fMaxCount - fCount);
        if (fParams->fEmitter) {
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

///////////////////////////////////////////////////////////////////////////////

class SkToJsonVisitor : public SkFieldVisitor {
public:
    SkToJsonVisitor(SkJSONWriter& writer) : fWriter(writer) {}

    // Primitives
    void visit(const char* name, float& f, SkField<float>) override {
        fWriter.appendFloat(name, f);
    }
    void visit(const char* name, int& i, SkField<int>) override {
        fWriter.appendS32(name, i);
    }
    void visit(const char* name, bool& b, SkField<bool>) override {
        fWriter.appendBool(name, b);
    }
    void visit(const char* name, SkString& s, SkField<SkString>) override {
        fWriter.appendString(name, s.c_str());
    }

    // Compound types
    void visit(const char* name, SkPoint& p, SkField<SkPoint>) override {
        fWriter.beginObject(name, false);
        fWriter.appendFloat("x", p.fX);
        fWriter.appendFloat("y", p.fY);
        fWriter.endObject();
    }

    void visit(const char* name, SkColor4f& c, SkField<SkColor4f>) override {
        fWriter.beginArray(name, false);
        fWriter.appendFloat(c.fR);
        fWriter.appendFloat(c.fG);
        fWriter.appendFloat(c.fB);
        fWriter.appendFloat(c.fA);
        fWriter.endArray();
    }

    void visit(sk_sp<SkReflected>& e, const SkReflected::Type* baseType) override {
        fWriter.appendString("Type", e ? e->getType()->fName : "Null");
    }

    void enterObject(const char* name) override { fWriter.beginObject(name); }
    void exitObject()                  override { fWriter.endObject(); }

    void visit(const char* name, SkTArray<sk_sp<SkReflected>>& arr,
               const SkReflected::Type* baseType) {
        fWriter.beginArray(name);
        for (auto ptr : arr) {
            SkFieldVisitor::visit(nullptr, ptr);
        }
        fWriter.endArray();
    }

private:
    SkJSONWriter& fWriter;
};

class SkFromJsonVisitor : public SkFieldVisitor {
public:
    SkFromJsonVisitor(const Value& v) : fRoot(v) {
        fStack.push_back(&fRoot);
    }

    void visit(const char* name, float& f, SkField<float> field) override {
        f = ParseOrDefault(get(name), field.fDefaultValue);
    }
    void visit(const char* name, int& i, SkField<int> field) override {
        i = ParseOrDefault(get(name), field.fDefaultValue);
    }
    void visit(const char* name, bool& b, SkField<bool> field) override {
        b = ParseOrDefault(get(name), field.fDefaultValue);
    }
    void visit(const char* name, SkString& s, SkField<SkString> field) override {
        s = ParseOrDefault(get(name), field.fDefaultValue);
    }

    void visit(const char* name, SkPoint& p, SkField<SkPoint> field) override {
        if (const ObjectValue* obj = get(name)) {
            p.fX = ParseOrDefault((*obj)["x"], field.fDefaultValue.fX);
            p.fY = ParseOrDefault((*obj)["y"], field.fDefaultValue.fY);
        } else {
            p = field.fDefaultValue;
        }
    }

    void visit(const char* name, SkColor4f& c, SkField<SkColor4f> field) override {
        const ArrayValue* arr = get(name);
        if (arr && arr->size() == 4) {
            c.fR = ParseOrDefault((*arr)[0], field.fDefaultValue.fR);
            c.fG = ParseOrDefault((*arr)[1], field.fDefaultValue.fG);
            c.fB = ParseOrDefault((*arr)[2], field.fDefaultValue.fB);
            c.fA = ParseOrDefault((*arr)[3], field.fDefaultValue.fA);
        } else {
            c = field.fDefaultValue;
        }
    }

    void visit(sk_sp<SkReflected>& e, const SkReflected::Type* baseType) override {
        const StringValue* typeString = get("Type");
        const char* type = typeString ? typeString->begin() : "Null";
        e = SkReflected::CreateInstance(type);
    }

    void enterObject(const char* name) override {
        fStack.push_back((const ObjectValue*)get(name));
    }
    void exitObject() override {
        fStack.pop_back();
    }

    void visit(const char* name, SkTArray<sk_sp<SkReflected>>& arr,
               const SkReflected::Type* baseType) {
        arr.reset();

        if (const ArrayValue* arrVal = get(name)) {
            arr.reserve(arrVal->size());

            for (const ObjectValue* obj : *arrVal) {
                sk_sp<SkReflected> ptr = nullptr;
                if (obj) {
                    fStack.push_back(obj);
                    this->visit(ptr, baseType);
                    if (ptr) {
                        ptr->visitFields(this);
                    }
                    fStack.pop_back();
                }
                if (ptr && ptr->isOfType(baseType)) {
                    arr.push_back(ptr);
                }
            }
        }
    }

private:
    const Value& get(const char* name) const {
        if (const Value* cur = fStack.back()) {
            if (!name) {
                return *cur;
            } else if (cur->is<ObjectValue>()) {
                return cur->as<ObjectValue>()[name];
            }
        }
        static NullValue gNull;
        return gNull;
    }

    const Value& fRoot;
    SkSTArray<16, const Value*, true> fStack;
};

static int InputTextCallback(ImGuiInputTextCallbackData* data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        SkString* s = (SkString*)data->UserData;
        SkASSERT(data->Buf == s->writable_str());
        SkString tmp(data->Buf, data->BufTextLen);
        s->swap(tmp);
        data->Buf = s->writable_str();
    }
    return 0;
}

class SkGuiVisitor : public SkFieldVisitor {
public:
    SkGuiVisitor() {
        fTreeStack.push_back(true);
    }

#define IF_OPEN(WIDGET) if (fTreeStack.back()) { WIDGET; }

    void visit(const char* name, float& f, SkField<float> field) override {
        if (fTreeStack.back()) {
            if (field.fFlags & kAngle_Field) {
                ImGui::SliderAngle(name, &f, 0.0f);
            } else {
                ImGui::DragFloat(name, &f);
            }
        }
    }
    void visit(const char* name, int& i, SkField<int>) override {
        IF_OPEN(ImGui::DragInt(name, &i))
    }
    void visit(const char* name, bool& b, SkField<bool>) override {
        IF_OPEN(ImGui::Checkbox(name, &b))
    }
    void visit(const char* name, SkString& s, SkField<SkString>) override {
        if (fTreeStack.back()) {
            ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackResize;
            ImGui::InputText(name, s.writable_str(), s.size() + 1, flags, InputTextCallback, &s);
        }
    }

    void visit(const char* name, SkPoint& p, SkField<SkPoint>) override {
        IF_OPEN(ImGui::DragFloat2(name, &p.fX))
    }
    void visit(const char* name, SkColor4f& c, SkField<SkColor4f>) override {
        IF_OPEN(ImGui::ColorEdit4(name, c.vec()))
    }

    void visit(sk_sp<SkReflected>& e, const SkReflected::Type* baseType) override {
        if (fTreeStack.back()) {
            const SkReflected::Type* curType = e ? e->getType() : nullptr;
            if (ImGui::BeginCombo("Type", curType ? curType->fName : "Null")) {
                auto visitType = [curType,&e](const SkReflected::Type* t) {
                    if (ImGui::Selectable(t->fName, curType == t)) {
                        e = t->fFactory();
                    }
                };
                SkReflected::VisitTypes(visitType, baseType);
                ImGui::EndCombo();
            }
        }
    }

    void enterObject(const char* name) override {
        if (fTreeStack.back()) {
            fTreeStack.push_back(ImGui::TreeNode(name));
        } else {
            fTreeStack.push_back(false);
        }
    }
    void exitObject() override {
        if (fTreeStack.back()) {
            ImGui::TreePop();
        }
        fTreeStack.pop_back();
    }

#undef IF_OPEN

    void visit(const char* name, SkTArray<sk_sp<SkReflected>>& arr,
               const SkReflected::Type* baseType) {
        this->enterObject(name);
        if (fTreeStack.back()) {
            for (int i = 0; i < arr.count(); ++i) {
                ImGui::PushID(i);
                this->enterObject("Item");

                this->visit(arr[i], baseType);
                if (arr[i]) {
                    arr[i]->visitFields(this);
                }

                this->exitObject();
                ImGui::PopID();
            }
            if (ImGui::Button("+")) {
                arr.push_back(nullptr);
            }
        }
        this->exitObject();
    }

private:
    SkSTArray<16, bool, true> fTreeStack;
};

static sk_sp<SkParticleEffectParams> LoadEffectParams(const char* filename) {
    sk_sp<SkParticleEffectParams> params(new SkParticleEffectParams());
    if (auto fileData = SkData::MakeFromFileName(filename)) {
        DOM dom(static_cast<const char*>(fileData->data()), fileData->size());
        SkFromJsonVisitor fromJson(dom.root());
        params->visitFields(&fromJson);
    }
    return params;
}

ParticlesSlide::ParticlesSlide() {
    // Register types for serialization
    REGISTER_REFLECTED(SkReflected);
    REGISTER_REFLECTED(SkParticleEmitter);
    REGISTER_REFLECTED(SkCircleEmitter);
    REGISTER_REFLECTED(SkLineEmitter);
    REGISTER_REFLECTED(SkTextEmitter);
    REGISTER_REFLECTED(SkParticleAffector);
    REGISTER_REFLECTED(SkDirectionalForceAffector);
    REGISTER_REFLECTED(SkPointForceAffector);

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
            if (auto newParams = LoadEffectParams(filename)) {
                fEffect.reset(new SkParticleEffect(std::move(newParams)));
            }
        }
        ImGui::SameLine();

        if (ImGui::Button("Save")) {
            SkFILEWStream fileStream(filename);
            if (fileStream.isValid()) {
                SkJSONWriter writer(&fileStream, SkJSONWriter::Mode::kPretty);
                SkToJsonVisitor toJson(writer);
                writer.beginObject();
                fEffect->getParams()->visitFields(&toJson);
                writer.endObject();
                writer.flush();
                fileStream.flush();
            } else {
                SkDebugf("Failed to open file\n");
            }
        }

        SkGuiVisitor gui;
        fEffect->getParams()->visitFields(&gui);
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
