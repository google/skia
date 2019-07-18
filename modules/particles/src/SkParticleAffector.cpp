/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "modules/particles/include/SkParticleAffector.h"

#include "include/core/SkContourMeasure.h"
#include "include/core/SkPath.h"
#include "include/utils/SkParsePath.h"
#include "include/utils/SkRandom.h"
#include "include/utils/SkTextUtils.h"
#include "modules/particles/include/SkCurve.h"
#include "modules/particles/include/SkParticleData.h"
#include "src/core/SkMakeUnique.h"
#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLExternalValue.h"

void SkParticleAffector::apply(const SkParticleUpdateParams& params,
                               SkParticleState ps[], int count) {
    if (fEnabled) {
        this->onApply(params, ps, count);
    }
}

void SkParticleAffector::visitFields(SkFieldVisitor* v) {
    v->visit("Enabled", fEnabled);
}

class SkLinearVelocityAffector : public SkParticleAffector {
public:
    SkLinearVelocityAffector(const SkCurve& angle = 0.0f,
                             const SkCurve& strength = 0.0f,
                             bool force = true,
                             SkParticleFrame frame = kWorld_ParticleFrame)
        : fAngle(angle)
        , fStrength(strength)
        , fForce(force)
        , fFrame(frame) {}

    REFLECTED(SkLinearVelocityAffector, SkParticleAffector)

    void onApply(const SkParticleUpdateParams& params, SkParticleState ps[], int count) override {
        for (int i = 0; i < count; ++i) {
            float angle = fAngle.eval(params, ps[i]);
            SkScalar rad = SkDegreesToRadians(angle);
            SkScalar s_local = SkScalarSin(rad),
                     c_local = SkScalarCos(rad);
            SkVector heading = ps[i].getFrameHeading(static_cast<SkParticleFrame>(fFrame));
            SkScalar c = heading.fX * c_local - heading.fY * s_local;
            SkScalar s = heading.fX * s_local + heading.fY * c_local;
            float strength = fStrength.eval(params, ps[i]);
            SkVector force = { c * strength, s * strength };
            if (fForce) {
                ps[i].fVelocity.fLinear += force * params.fDeltaTime;
            } else {
                ps[i].fVelocity.fLinear = force;
            }
        }
    }

    void visitFields(SkFieldVisitor* v) override {
        SkParticleAffector::visitFields(v);
        v->visit("Force", fForce);
        v->visit("Frame", fFrame, gParticleFrameMapping, SK_ARRAY_COUNT(gParticleFrameMapping));
        v->visit("Angle", fAngle);
        v->visit("Strength", fStrength);
    }

private:
    SkCurve fAngle;
    SkCurve fStrength;
    bool fForce;
    int  fFrame;
};

class SkAngularVelocityAffector : public SkParticleAffector {
public:
    SkAngularVelocityAffector(const SkCurve& strength = 0.0f, bool force = true)
        : fStrength(strength)
        , fForce(force) {}

    REFLECTED(SkAngularVelocityAffector, SkParticleAffector)

    void onApply(const SkParticleUpdateParams& params, SkParticleState ps[], int count) override {
        for (int i = 0; i < count; ++i) {
            float strength = fStrength.eval(params, ps[i]);
            if (fForce) {
                ps[i].fVelocity.fAngular += strength * params.fDeltaTime;
            } else {
                ps[i].fVelocity.fAngular = strength;
            }
        }
    }

    void visitFields(SkFieldVisitor* v) override {
        SkParticleAffector::visitFields(v);
        v->visit("Force", fForce);
        v->visit("Strength", fStrength);
    }

private:
    SkCurve fStrength;
    bool    fForce;
};

class SkPointForceAffector : public SkParticleAffector {
public:
    SkPointForceAffector(SkPoint point = { 0.0f, 0.0f }, SkScalar constant = 0.0f,
                         SkScalar invSquare = 0.0f)
            : fPoint(point), fConstant(constant), fInvSquare(invSquare) {}

    REFLECTED(SkPointForceAffector, SkParticleAffector)

    void onApply(const SkParticleUpdateParams& params, SkParticleState ps[], int count) override {
        for (int i = 0; i < count; ++i) {
            SkVector toPoint = fPoint - ps[i].fPose.fPosition;
            SkScalar lenSquare = toPoint.dot(toPoint);
            toPoint.normalize();
            ps[i].fVelocity.fLinear +=
                    toPoint * (fConstant + (fInvSquare / lenSquare)) * params.fDeltaTime;
        }
    }

    void visitFields(SkFieldVisitor* v) override {
        SkParticleAffector::visitFields(v);
        v->visit("Point", fPoint);
        v->visit("Constant", fConstant);
        v->visit("InvSquare", fInvSquare);
    }

private:
    SkPoint  fPoint;
    SkScalar fConstant;
    SkScalar fInvSquare;
};

class SkOrientationAffector : public SkParticleAffector {
public:
    SkOrientationAffector(const SkCurve& angle = 0.0f,
                          SkParticleFrame frame = kLocal_ParticleFrame)
        : fAngle(angle)
        , fFrame(frame) {}

    REFLECTED(SkOrientationAffector, SkParticleAffector)

    void onApply(const SkParticleUpdateParams& params, SkParticleState ps[], int count) override {
        for (int i = 0; i < count; ++i) {
            float angle = fAngle.eval(params, ps[i]);
            SkScalar rad = SkDegreesToRadians(angle);
            SkScalar s_local = SkScalarSin(rad),
                     c_local = SkScalarCos(rad);
            SkVector heading = ps[i].getFrameHeading(static_cast<SkParticleFrame>(fFrame));
            ps[i].fPose.fHeading.set(heading.fX * c_local - heading.fY * s_local,
                                     heading.fX * s_local + heading.fY * c_local);
        }
    }

    void visitFields(SkFieldVisitor *v) override {
        SkParticleAffector::visitFields(v);
        v->visit("Frame", fFrame, gParticleFrameMapping, SK_ARRAY_COUNT(gParticleFrameMapping));
        v->visit("Angle", fAngle);
    }

private:
    SkCurve fAngle;
    int     fFrame;
};

class SkPositionInCircleAffector : public SkParticleAffector {
public:
    SkPositionInCircleAffector(const SkCurve& x = 0.0f, const SkCurve& y = 0.0f,
                               const SkCurve& radius = 0.0f, bool setHeading = true)
        : fX(x)
        , fY(y)
        , fRadius(radius)
        , fSetHeading(setHeading) {}

    REFLECTED(SkPositionInCircleAffector, SkParticleAffector)

    void onApply(const SkParticleUpdateParams& params, SkParticleState ps[], int count) override {
        for (int i = 0; i < count; ++i) {
            SkVector v;
            do {
                v.fX = ps[i].fRandom.nextSScalar1();
                v.fY = ps[i].fRandom.nextSScalar1();
            } while (v.dot(v) > 1);

            SkPoint center = { fX.eval(params, ps[i]), fY.eval(params, ps[i]) };
            SkScalar radius = fRadius.eval(params, ps[i]);
            ps[i].fPose.fPosition = center + (v * radius);
            if (fSetHeading) {
                if (!v.normalize()) {
                    v.set(0, -1);
                }
                ps[i].fPose.fHeading = v;
            }
        }
    }

    void visitFields(SkFieldVisitor* v) override {
        SkParticleAffector::visitFields(v);
        v->visit("SetHeading", fSetHeading);
        v->visit("X", fX);
        v->visit("Y", fY);
        v->visit("Radius", fRadius);
    }

private:
    SkCurve fX;
    SkCurve fY;
    SkCurve fRadius;
    bool    fSetHeading;
};

class SkPositionOnPathAffector : public SkParticleAffector {
public:
    SkPositionOnPathAffector(const char* path = "", bool setHeading = true,
                             SkParticleValue input = SkParticleValue())
            : fPath(path)
            , fInput(input)
            , fSetHeading(setHeading) {
        this->rebuild();
    }

    REFLECTED(SkPositionOnPathAffector, SkParticleAffector)

    void onApply(const SkParticleUpdateParams& params, SkParticleState ps[], int count) override {
        if (fContours.empty()) {
            return;
        }

        for (int i = 0; i < count; ++i) {
            float t = fInput.eval(params, ps[i]);
            SkScalar len = fTotalLength * t;
            int idx = 0;
            while (idx < fContours.count() && len > fContours[idx]->length()) {
                len -= fContours[idx++]->length();
            }
            SkVector localXAxis;
            if (!fContours[idx]->getPosTan(len, &ps[i].fPose.fPosition, &localXAxis)) {
                ps[i].fPose.fPosition = { 0, 0 };
                localXAxis = { 1, 0 };
            }
            if (fSetHeading) {
                ps[i].fPose.fHeading.set(localXAxis.fY, -localXAxis.fX);
            }
        }
    }

    void visitFields(SkFieldVisitor* v) override {
        SkString oldPath = fPath;

        SkParticleAffector::visitFields(v);
        v->visit("Input", fInput);
        v->visit("SetHeading", fSetHeading);
        v->visit("Path", fPath);

        if (fPath != oldPath) {
            this->rebuild();
        }
    }

private:
    SkString        fPath;
    SkParticleValue fInput;
    bool            fSetHeading;

    void rebuild() {
        SkPath path;
        if (!SkParsePath::FromSVGString(fPath.c_str(), &path)) {
            return;
        }

        fTotalLength = 0;
        fContours.reset();

        SkContourMeasureIter iter(path, false);
        while (auto contour = iter.next()) {
            fContours.push_back(contour);
            fTotalLength += contour->length();
        }
    }

    // Cached
    SkScalar                          fTotalLength;
    SkTArray<sk_sp<SkContourMeasure>> fContours;
};

class SkPositionOnTextAffector : public SkParticleAffector {
public:
    SkPositionOnTextAffector(const char* text = "", SkScalar fontSize = 96, bool setHeading = true,
                             SkParticleValue input = SkParticleValue())
            : fText(text)
            , fFontSize(fontSize)
            , fInput(input)
            , fSetHeading(setHeading) {
        this->rebuild();
    }

    REFLECTED(SkPositionOnTextAffector, SkParticleAffector)

    void onApply(const SkParticleUpdateParams& params, SkParticleState ps[], int count) override {
        if (fContours.empty()) {
            return;
        }

        // TODO: Refactor to share code with PositionOnPathAffector
        for (int i = 0; i < count; ++i) {
            float t = fInput.eval(params, ps[i]);
            SkScalar len = fTotalLength * t;
            int idx = 0;
            while (idx < fContours.count() && len > fContours[idx]->length()) {
                len -= fContours[idx++]->length();
            }
            SkVector localXAxis;
            if (!fContours[idx]->getPosTan(len, &ps[i].fPose.fPosition, &localXAxis)) {
                ps[i].fPose.fPosition = { 0, 0 };
                localXAxis = { 1, 0 };
            }
            if (fSetHeading) {
                ps[i].fPose.fHeading.set(localXAxis.fY, -localXAxis.fX);
            }
        }
    }

    void visitFields(SkFieldVisitor* v) override {
        SkString oldText = fText;
        SkScalar oldSize = fFontSize;

        SkParticleAffector::visitFields(v);
        v->visit("Input", fInput);
        v->visit("SetHeading", fSetHeading);
        v->visit("Text", fText);
        v->visit("FontSize", fFontSize);

        if (fText != oldText || fFontSize != oldSize) {
            this->rebuild();
        }
    }

private:
    SkString        fText;
    SkScalar        fFontSize;
    SkParticleValue fInput;
    bool            fSetHeading;

    void rebuild() {
        fTotalLength = 0;
        fContours.reset();

        if (fText.isEmpty()) {
            return;
        }

        // Use the font manager's default font
        SkFont font(nullptr, fFontSize);
        SkPath path;
        SkTextUtils::GetPath(fText.c_str(), fText.size(), SkTextEncoding::kUTF8, 0, 0, font, &path);
        SkContourMeasureIter iter(path, false);
        while (auto contour = iter.next()) {
            fContours.push_back(contour);
            fTotalLength += contour->length();
        }
    }

    // Cached
    SkScalar                          fTotalLength;
    SkTArray<sk_sp<SkContourMeasure>> fContours;
};

class SkSizeAffector : public SkParticleAffector {
public:
    SkSizeAffector(const SkCurve& curve = 1.0f) : fCurve(curve) {}

    REFLECTED(SkSizeAffector, SkParticleAffector)

    void onApply(const SkParticleUpdateParams& params, SkParticleState ps[], int count) override {
        for (int i = 0; i < count; ++i) {
            ps[i].fPose.fScale = fCurve.eval(params, ps[i]);
        }
    }

    void visitFields(SkFieldVisitor* v) override {
        SkParticleAffector::visitFields(v);
        v->visit("Curve", fCurve);
    }

private:
    SkCurve fCurve;
};

class SkFrameAffector : public SkParticleAffector {
public:
    SkFrameAffector(const SkCurve& curve = 1.0f) : fCurve(curve) {}

    REFLECTED(SkFrameAffector, SkParticleAffector)

    void onApply(const SkParticleUpdateParams& params, SkParticleState ps[], int count) override {
        for (int i = 0; i < count; ++i) {
            ps[i].fFrame = fCurve.eval(params, ps[i]);
        }
    }

    void visitFields(SkFieldVisitor* v) override {
        SkParticleAffector::visitFields(v);
        v->visit("Curve", fCurve);
    }

private:
    SkCurve fCurve;
};

class SkColorAffector : public SkParticleAffector {
public:
    SkColorAffector(const SkColorCurve& curve = SkColor4f{ 1.0f, 1.0f, 1.0f, 1.0f })
        : fCurve(curve) {}

    REFLECTED(SkColorAffector, SkParticleAffector)

    void onApply(const SkParticleUpdateParams& params, SkParticleState ps[], int count) override {
        for (int i = 0; i < count; ++i) {
            ps[i].fColor = fCurve.eval(params, ps[i]);
        }
    }

    void visitFields(SkFieldVisitor* v) override {
        SkParticleAffector::visitFields(v);
        v->visit("Curve", fCurve);
    }

private:
    SkColorCurve fCurve;
};

static const char* kDefaultCode =
    "// float rand; Every read returns a random float [0 .. 1)\n"
    "layout(ctype=float) in uniform float dt;\n"
    "layout(ctype=float) in uniform float effectAge;\n"
    "\n"
    "void main(in    float age,\n"
    "          in    float invLifetime,\n"
    "          inout float2 pos,\n"
    "          inout float2 dir,\n"
    "          inout float  scale,\n"
    "          inout float2 vel,\n"
    "          inout float  spin,\n"
    "          inout float4 color) {\n"
    "}\n";

class SkRandomExternalValue : public SkSL::ExternalValue {
public:
    SkRandomExternalValue(const char* name, SkSL::Compiler& compiler)
        : INHERITED(name, *compiler.context().fFloat_Type)
        , fRandom(nullptr) { }

    void setRandom(SkRandom* random) { fRandom = random; }
    bool canRead() const override { return true; }
    void read(int /*unusedIndex*/, float* target) override { *target = fRandom->nextF(); }

private:
    SkRandom* fRandom;
    typedef SkSL::ExternalValue INHERITED;
};

class SkInterpreterAffector : public SkParticleAffector {
public:
    SkInterpreterAffector() : fCode(kDefaultCode) {
        this->rebuild();
    }

    REFLECTED(SkInterpreterAffector, SkParticleAffector)

    void onApply(const SkParticleUpdateParams& params, SkParticleState ps[], int count) override {
        for (int i = 0; i < count; ++i) {
            fRandomValue->setRandom(&ps[i].fRandom);
            SkAssertResult(fByteCode->run(fMain, &ps[i].fAge, nullptr, 1, &params.fDeltaTime, 2));
        }
    }

    void visitFields(SkFieldVisitor* v) override {
        SkString oldCode = fCode;

        SkParticleAffector::visitFields(v);
        v->visit("Code", fCode);

        if (fCode != oldCode) {
            this->rebuild();
        }
    }

private:
    SkString fCode;

    // Cached
    std::unique_ptr<SkSL::ByteCode> fByteCode;
    std::unique_ptr<SkRandomExternalValue> fRandomValue;
    SkSL::ByteCodeFunction* fMain;

    void rebuild() {
        SkSL::Compiler compiler;
        SkSL::Program::Settings settings;
        auto rand = skstd::make_unique<SkRandomExternalValue>("rand", compiler);
        compiler.registerExternalValue(rand.get());
        auto program = compiler.convertProgram(SkSL::Program::kGeneric_Kind,
                                               SkSL::String(fCode.c_str()), settings);
        if (!program) {
            SkDebugf("%s\n", compiler.errorText().c_str());
            return;
        }

        auto byteCode = compiler.toByteCode(*program);
        if (compiler.errorCount()) {
            SkDebugf("%s\n", compiler.errorText().c_str());
            return;
        }

        fMain = byteCode->fFunctions[0].get();
        fByteCode = std::move(byteCode);
        fRandomValue = std::move(rand);
    }
};

void SkParticleAffector::RegisterAffectorTypes() {
    REGISTER_REFLECTED(SkParticleAffector);
    REGISTER_REFLECTED(SkLinearVelocityAffector);
    REGISTER_REFLECTED(SkAngularVelocityAffector);
    REGISTER_REFLECTED(SkPointForceAffector);
    REGISTER_REFLECTED(SkOrientationAffector);
    REGISTER_REFLECTED(SkPositionInCircleAffector);
    REGISTER_REFLECTED(SkPositionOnPathAffector);
    REGISTER_REFLECTED(SkPositionOnTextAffector);
    REGISTER_REFLECTED(SkSizeAffector);
    REGISTER_REFLECTED(SkFrameAffector);
    REGISTER_REFLECTED(SkColorAffector);
    REGISTER_REFLECTED(SkInterpreterAffector);
}

sk_sp<SkParticleAffector> SkParticleAffector::MakeLinearVelocity(const SkCurve& angle,
                                                                 const SkCurve& strength,
                                                                 bool force,
                                                                 SkParticleFrame frame) {
    return sk_sp<SkParticleAffector>(new SkLinearVelocityAffector(angle, strength, force, frame));
}

sk_sp<SkParticleAffector> SkParticleAffector::MakeAngularVelocity(const SkCurve& strength,
                                                                  bool force) {
    return sk_sp<SkParticleAffector>(new SkAngularVelocityAffector(strength, force));
}

sk_sp<SkParticleAffector> SkParticleAffector::MakePointForce(SkPoint point, SkScalar constant,
                                                             SkScalar invSquare) {
    return sk_sp<SkParticleAffector>(new SkPointForceAffector(point, constant, invSquare));
}

sk_sp<SkParticleAffector> SkParticleAffector::MakeOrientation(const SkCurve& angle,
                                                              SkParticleFrame frame) {
    return sk_sp<SkParticleAffector>(new SkOrientationAffector(angle, frame));
}

sk_sp<SkParticleAffector> SkParticleAffector::MakeSize(const SkCurve& curve) {
    return sk_sp<SkParticleAffector>(new SkSizeAffector(curve));
}

sk_sp<SkParticleAffector> SkParticleAffector::MakeFrame(const SkCurve& curve) {
    return sk_sp<SkParticleAffector>(new SkFrameAffector(curve));
}

sk_sp<SkParticleAffector> SkParticleAffector::MakeColor(const SkColorCurve& curve) {
    return sk_sp<SkParticleAffector>(new SkColorAffector(curve));
}
