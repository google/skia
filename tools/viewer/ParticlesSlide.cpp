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

static SkScalar kDragSize = 8.0f;
static SkTArray<SkPoint*> gDragPoints;
int gDragIndex = -1;

struct SkParticlePose {
    SkPoint  fPosition;
    SkVector fRight;
    SkScalar fScale;

    SkRSXform asRSXform(SkPoint ofs) const {
        const float s = fRight.fY * fScale;
        const float c = fRight.fX * fScale;
        return SkRSXform::Make(c, s,
                               fPosition.fX + -c * ofs.fX +  s * ofs.fY,
                               fPosition.fY + -s * ofs.fX + -c * ofs.fY);
    }
};

struct SkParticleVelocity {
    SkVector fLinear;
    SkScalar fAngular;
};

struct SkParticlePoseAndVelocity {
    SkParticlePose     fPose;
    SkParticleVelocity fVelocity;
};

#if 0
inline SkVector right_to_up(SkVector right) {
    return { right.fY, -right.fX };
}
#endif

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

// Cubic function:
// f(x) = A(1-x)^3 + Bx(1-x)^2 + Cx^2(1-x) + Dx^3
// For each segment, evaluate as if segment is [0,1]
#if 0
class SkCurveSegment : public SkReflected {
public:
    SkCurveSegment(SkScalar c = 0.0f) {
        fRanged = false;
        fX = 1.0f;
        fMin[0] = fMin[1] = fMin[2] = fMin[3] = c;
        fMax[0] = fMax[1] = fMax[2] = fMax[3] = c;
    }

    REFLECTED(SkCurveSegment, SkReflected)

    SkScalar eval(float x, float t) const {
        float ix = (1 - x);
        float y0 = fMin[0] * ix*ix*ix + fMin[1] * ix*ix*x + fMin[2] * ix*x*x + fMin[3] * x*x*x;
        float y1 = fMax[0] * ix*ix*ix + fMax[1] * ix*ix*x + fMax[2] * ix*x*x + fMax[3] * x*x*x;
        return fRanged ? y0 + (y1 - y0) * t : y0;
    }

    void visitFields(SkFieldVisitor* v) override {
        v->visit("Ranged", fRanged);
        v->visit("X", fX);
        v->visit("A0", fMin[0]);
        v->visit("B0", fMin[1]);
        v->visit("C0", fMin[2]);
        v->visit("D0", fMin[3]);
        v->visit("A1", fMax[0]);
        v->visit("B1", fMax[1]);
        v->visit("C1", fMax[2]);
        v->visit("D1", fMax[3]);
    }

    bool fRanged;
    SkScalar fX;      // X at "right" end of segment
    SkScalar fMin[4];
    SkScalar fMax[4]; // used if ranged
};

struct SkCurve {
    SkCurve(SkScalar c = 0.0f) {
        fSegments.push_back(sk_sp<SkCurveSegment>(new SkCurveSegment(c)));
    }

    float eval(SkRandom& random, float x) const {
        int i = 0;
        for (; i < fSegments.count(); ++i) {
            if (x <= fSegments[i]->fX) {
                break;
            }
        }
        float rangeMin = (i == 0) ? 0.0f : fSegments[i - 1]->fX;
        float rangeMax = fSegments[i]->fX;
        float segmentX = (x - rangeMin) / (rangeMax - rangeMin);
        SkASSERT(0.0f <= segmentX && segmentX <= 1.0f);
        return fSegments[i]->eval(segmentX, random.nextF());
    }

    void visitFields(SkFieldVisitor* v) {
        v->visit("Segments", fSegments);
    }

    SkTArray<sk_sp<SkCurveSegment>> fSegments;
};
#endif

SkScalar SkCurve::eval(float x, SkRandom& random) const {
    float ix = (1 - x);
    float y0 = fMin[0] * ix*ix*ix + fMin[1] * 3*ix*ix*x + fMin[2] * 3*ix*x*x + fMin[3] * x*x*x;
    float y1 = fMax[0] * ix*ix*ix + fMax[1] * 3*ix*ix*x + fMax[2] * 3*ix*x*x + fMax[3] * x*x*x;
    return fRanged ? y0 + (y1 - y0) * random.nextF() : y0;
}

void SkCurve::visitFields(SkFieldVisitor* v) {
    v->visit("Ranged", fRanged);
    v->visit("A0", fMin[0]);
    v->visit("B0", fMin[1]);
    v->visit("C0", fMin[2]);
    v->visit("D0", fMin[3]);
    v->visit("A1", fMax[0]);
    v->visit("B1", fMax[1]);
    v->visit("C1", fMax[2]);
    v->visit("D1", fMax[3]);
}

///////////////////////////////////////////////////////////////////////////////

class SkParticleEmitter : public SkReflected {
public:
    REFLECTED_ABSTRACT(SkParticleEmitter, SkReflected)

    virtual void emit(SkRandom&, SkParticlePose& pose) const = 0;
};

class SkCircleEmitter : public SkParticleEmitter {
public:
    SkCircleEmitter(SkPoint center = { 0.0f, 0.0f }, SkScalar radius = 0.0f)
        : fCenter(center), fRadius(radius) {}

    REFLECTED(SkCircleEmitter, SkParticleEmitter)

    void emit(SkRandom& random, SkParticlePose& pose) const override {
        SkVector v;
        do {
            v.fX = random.nextSScalar1();
            v.fY = random.nextSScalar1();
        } while (v.distanceToOrigin() > 1);
        pose.fRight = up_to_right(v);
        if (!pose.fRight.normalize()) {
            pose.fRight.set(1, 0);
        }
        pose.fPosition = fCenter + (v * fRadius);
        pose.fScale = 1.0f;
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

    void emit(SkRandom& random, SkParticlePose& pose) const override {
        pose.fRight = (fP2 - fP1);
        if (!pose.fRight.normalize()) {
            pose.fRight.set(1, 0);
        }
        pose.fPosition = fP1 + (fP2 - fP1) * random.nextUScalar1();
        pose.fScale = 1.0f;
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

    void emit(SkRandom& random, SkParticlePose& pose) const override {
        if (fContours.count() == 0) {
            pose.fPosition = SkPoint{ 0, 0 };
            pose.fRight = { 1, 0 };
            pose.fScale = 0.0f;
            return;
        }

        SkScalar len = random.nextRangeScalar(0, fTotalLength);
        int idx = 0;
        while (idx < fContours.count() && len > fLengths[idx]) {
            len -= fLengths[idx++];
        }
        SkPathMeasure cm(fContours[idx], false);
        cm.getPosTan(len, &pose.fPosition, &pose.fRight);
        pose.fScale = 1.0f;
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

struct UpdateParams {
    SkRandom* fRandom;
    SkRandom* fStableRandom;
    float fElapsed;
    float fParticleT;
};

class SkParticleAffector : public SkReflected {
public:
    REFLECTED_ABSTRACT(SkParticleAffector, SkReflected)

    virtual void apply(UpdateParams& params, SkParticlePoseAndVelocity& pv) = 0;
};

class SkDirectionalForceAffector : public SkParticleAffector {
public:
    SkDirectionalForceAffector(SkVector force = { 0.0f, 0.0f }) : fForce(force) {}

    REFLECTED(SkDirectionalForceAffector, SkParticleAffector)

    void apply(UpdateParams& params, SkParticlePoseAndVelocity& pv) override {
        pv.fVelocity.fLinear += fForce * params.fElapsed;
    }

    void visitFields(SkFieldVisitor* v) override {
        v->visit("Force", fForce);
    }

private:
    SkVector fForce;
};

class SkRangedForceAffector : public SkParticleAffector {
public:
    SkRangedForceAffector()
        : fAngle(0.0f)
        , fStrength(0.0f)
        , fBidirectional(false) {}

    REFLECTED(SkRangedForceAffector, SkParticleAffector)

    void apply(UpdateParams& params, SkParticlePoseAndVelocity& pv) override {
        float angle = fAngle.eval(params.fParticleT, *params.fStableRandom);
        SkScalar c, s = SkScalarSinCos(angle, &c);
        float strength = fStrength.eval(params.fParticleT, *params.fStableRandom);
        if (fBidirectional && params.fStableRandom->nextBool()) {
            strength = -strength;
        }
        SkVector force = { c * strength, s * strength };
        pv.fVelocity.fLinear += force * params.fElapsed;
    }

    void visitFields(SkFieldVisitor* v) override {
        v->visit("Angle", fAngle);
        v->visit("Strength", fStrength);
        v->visit("Bidirectional", fBidirectional);
    }

private:
    SkCurve fAngle;
    SkCurve fStrength;
    bool fBidirectional;
};

class SkPointForceAffector : public SkParticleAffector {
public:
    SkPointForceAffector(SkPoint point = { 0.0f, 0.0f }, SkScalar constant = 0.0f,
                         SkScalar invSquare = 0.0f)
            : fPoint(point), fConstant(constant), fInvSquare(invSquare) {}

    REFLECTED(SkPointForceAffector, SkParticleAffector)

    void apply(UpdateParams& params, SkParticlePoseAndVelocity& pv) override {
        SkVector toPoint = fPoint - pv.fPose.fPosition;
        SkScalar lenSquare = toPoint.dot(toPoint);
        toPoint.normalize();
        pv.fVelocity.fLinear += toPoint * (fConstant + (fInvSquare / lenSquare)) * params.fElapsed;
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

class SkOrientAlongVelocityAffector : public SkParticleAffector {
public:
    SkOrientAlongVelocityAffector() {}

    REFLECTED(SkOrientAlongVelocityAffector, SkParticleAffector)

    void apply(UpdateParams& params, SkParticlePoseAndVelocity& pv) override {
        SkVector up = pv.fVelocity.fLinear;
        if (!up.normalize()) {
            up.set(0, -1);
        }
        pv.fPose.fRight = up_to_right(up);
    }

    void visitFields(SkFieldVisitor*) override {}
};

class SkJitterAffector : public SkParticleAffector {
public:
    SkJitterAffector() {}

    REFLECTED(SkJitterAffector, SkParticleAffector)

    void apply(UpdateParams& params, SkParticlePoseAndVelocity& pv) override {
        pv.fVelocity.fLinear.fX += fX.eval(*params.fRandom);
    }

    void visitFields(SkFieldVisitor* v) override {
        v->visit("X", fX);
    }

private:
    SkRangedFloat fX;
};

///////////////////////////////////////////////////////////////////////////////

struct InitialVelocityParams {
    float fAngle = 0.0f;
    float fAngleSpread = 0.0f;
    SkRangedFloat fStrength;
    bool fBidirectional = false;

    SkRangedFloat fSpin;
    bool fBidirectionalSpin = false;

    SkParticleVelocity eval(SkRandom& random) {
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

    void visitFields(SkFieldVisitor* v) {
        v->visit("Angle", fAngle, kAngle_Field);
        v->visit("Spread", fAngleSpread, kAngle_Field);
        v->visit("Strength", fStrength);
        v->visit("Bidirectional", fBidirectional);

        v->visit("Spin", fSpin);
        v->visit("BidirectionalSpin", fBidirectionalSpin);
    }
};

// TODO: Make things variable over time of effect (rather than over time of particle?)
// TODO: Make more things ranged at start (eg, colors). Need to remember what values we picked.

class SkParticleEffectParams : public SkRefCnt {
public:
    int fMaxCount;
    float fRate;
    SkRangedFloat fLifetime;
    SkColor4f fStartColor;
    SkColor4f fEndColor;

    SkCurve fSize;

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

        v->visit("Size", fSize, SkField<SkCurve>(0, 1.0f));
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

        UpdateParams updateParams;
        updateParams.fElapsed = elapsed;
        updateParams.fRandom = &random;

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

            SkRandom stableRandom = fParticles[i].fRandom;
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

//            SkVector up = right_to_up(fXforms[i].fSCos, fXforms[i].fSSin);
//            fXforms[i].fTx += up.fX * elapsed * 50;
//            fXforms[i].fTy += up.fY * elapsed * 50;
//            fXforms[i].fTx += random.nextRangeScalar(-15.0f, 15.0f) * elapsed;
//            fXforms[i].fTy -= random.nextRangeScalar(15.0f, 40.0f) * elapsed;

            // Integrate position / orientation
            fParticles[i].fPV.fPose.fPosition += fParticles[i].fPV.fVelocity.fLinear * elapsed;

            SkScalar c, s = SkScalarSinCos(fParticles[i].fPV.fVelocity.fAngular * elapsed, &c);
            SkVector old = fParticles[i].fPV.fPose.fRight;
            fParticles[i].fPV.fPose.fRight = { old.fX * c - old.fY * s, old.fX * s + old.fY * c };
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
                fParams->fEmitter->emit(random, fParticles[fCount].fPV.fPose);
                fParticles[fCount].fPV.fVelocity = fParams->fVelocity.eval(random);
                fParticles[fCount].fRandom = random;
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
        SkRandom fRandom;

        // Texture coord rects and colors are stored in parallel arrays for drawAtlas.
        SkParticlePoseAndVelocity fPV;
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
               const SkReflected::Type* baseType) override {
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

    void visit(const char* name, SkCurve& c, SkField<SkCurve> field) override {
        if (const ObjectValue* obj = get(name)) {
            SkFieldVisitor::visit(name, c, field);
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
               const SkReflected::Type* baseType) override {
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

static ImVec2 map_point(float x, float y, ImVec2 pos, ImVec2 size, float yMin, float yMax) {
    // Turn y into 0 - 1 value
    float yNorm = 1.0f - ((y - yMin) / (yMax - yMin));
    return ImVec2(pos.x + size.x * x, pos.y + size.y * yNorm);
}

static void ImGui_DrawCurve(SkScalar* pts) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Fit our image/canvas to the available width, and scale the height to maintain aspect ratio.
    float canvasWidth = SkTMax(ImGui::GetContentRegionAvailWidth(), 50.0f);
    ImVec2 size = ImVec2(canvasWidth, canvasWidth);
    ImVec2 pos = ImGui::GetCursorScreenPos();

    // Background rectangle
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(0, 0, 0, 128));

    // Determine min/max extents
    float yMin = pts[0], yMax = pts[0];
    for (int i = 1; i < 4; ++i) {
        yMin = SkTMin(yMin, pts[i]);
        yMax = SkTMax(yMax, pts[i]);
    }

    // Grow the extents by 10%, at least 1.0f
    float grow = SkTMax((yMax - yMin) * 0.1f, 1.0f);

    yMin -= grow;
    yMax += grow;

    ImVec2 a = map_point(0.0f    , pts[0], pos, size, yMin, yMax),
           b = map_point(1 / 3.0f, pts[1], pos, size, yMin, yMax),
           c = map_point(2 / 3.0f, pts[2], pos, size, yMin, yMax),
           d = map_point(1.0f    , pts[3], pos, size, yMin, yMax);

    drawList->AddBezierCurve(a, b, c, d, IM_COL32(255, 255, 255, 255), 1.0f);

    // Draw markers
    drawList->AddCircle(a, 5.0f, 0xFFFFFFFF);
    drawList->AddCircle(b, 5.0f, 0xFFFFFFFF);
    drawList->AddCircle(c, 5.0f, 0xFFFFFFFF);
    drawList->AddCircle(d, 5.0f, 0xFFFFFFFF);

    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + size.y));
    ImGui::Spacing();
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
        if (fTreeStack.back()) {
            ImGui::DragFloat2(name, &p.fX);
            gDragPoints.push_back(&p);
        }
    }
    void visit(const char* name, SkColor4f& c, SkField<SkColor4f>) override {
        IF_OPEN(ImGui::ColorEdit4(name, c.vec()))
    }

    void visit(const char* name, SkCurve& c, SkField<SkCurve>) override {
        this->enterObject(name);
        if (fTreeStack.back()) {
            ImGui::Checkbox("Ranged", &c.fRanged);
            ImGui::DragFloat4("Min", c.fMin);
            ImGui_DrawCurve(c.fMin);
            if (c.fRanged) {
                ImGui::DragFloat4("Max", c.fMax);
                ImGui_DrawCurve(c.fMax);
            }

        }
        this->exitObject();
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
               const SkReflected::Type* baseType) override {
        this->enterObject(name);
        if (fTreeStack.back()) {
            for (int i = 0; i < arr.count(); ++i) {
                ImGui::PushID(i);

                if (ImGui::Button("X")) {
                    for (int j = i; j < arr.count() - 1; ++j) {
                        arr[j] = arr[j + 1];
                    }
                    arr.pop_back();
                    ImGui::PopID();
                    continue;
                }

                ImGui::SameLine();
                if (ImGui::Button("^") && i > 0) {
                    std::swap(arr[i], arr[i - 1]);
                }
                ImGui::SameLine();
                if (ImGui::Button("v") && i < arr.count() - 1) {
                    std::swap(arr[i], arr[i + 1]);
                }

                const char* typeName = arr[i] ? arr[i]->getType()->fName : "Null";
                ImGui::SameLine(); this->enterObject(typeName);

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
    REGISTER_REFLECTED(SkRangedForceAffector);
    REGISTER_REFLECTED(SkPointForceAffector);
    REGISTER_REFLECTED(SkOrientAlongVelocityAffector);
    REGISTER_REFLECTED(SkJitterAffector);

    fName = "Particles";
    fEffect.reset(new SkParticleEffect(LoadEffectParams("resources/particles/default.json")));
}

ParticlesSlide::~ParticlesSlide() {}

SkISize ParticlesSlide::getDimensions() const  {
    return SkISize::Make(500, 500);
}

void ParticlesSlide::draw(SkCanvas* canvas) {
    canvas->clear(0);

    gDragPoints.reset();
    if (ImGui::Begin("Particles")) {
        static char filename[64] = "resources/particles/default.json";

        const char* presets[] = {
            "resources/particles/default.json",
            "resources/particles/default2.json",
            "resources/particles/explosion.json",
            "resources/particles/penguin_cannon.json",
            "resources/particles/snowfall.json",
            "resources/particles/warp.json",
        };
        const char* labels[] = { "1", "2", "3", "4", "5", "6" };
        for (int i = 0; i < 6; ++i) {
            if (i > 0) {
                ImGui::SameLine();
            }
            if (ImGui::Button(labels[i])) {
                strcpy(filename, presets[i]);
            }
        }

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

    SkPaint dragPaint;
    dragPaint.setColor(SK_ColorLTGRAY);
    dragPaint.setAntiAlias(true);
    SkPaint dragHighlight;
    dragHighlight.setStyle(SkPaint::kStroke_Style);
    dragHighlight.setColor(SK_ColorGREEN);
    dragHighlight.setStrokeWidth(2);
    dragHighlight.setAntiAlias(true);
    for (int i = 0; i < gDragPoints.count(); ++i) {
        canvas->drawCircle(*gDragPoints[i], kDragSize, dragPaint);
        if (gDragIndex == i) {
            canvas->drawCircle(*gDragPoints[i], kDragSize, dragHighlight);
        }
    }
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
    if (gDragIndex == -1) {
        if (state == Window::kDown_InputState) {
            float bestDistance = kDragSize;
            SkPoint mousePt = { x, y };
            for (int i = 0; i < gDragPoints.count(); ++i) {
                float distance = SkPoint::Distance(*gDragPoints[i], mousePt);
                if (distance < bestDistance) {
                    gDragIndex = i;
                    bestDistance = distance;
                }
            }
            return gDragIndex != -1;
        }
    } else {
        // Currently dragging
        SkASSERT(gDragIndex < gDragPoints.count());
        gDragPoints[gDragIndex]->set(x, y);
        if (state == Window::kUp_InputState) {
            gDragIndex = -1;
        }
        return true;
    }
    return false;
}
