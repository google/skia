/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkM44.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkStream.h"
#include "include/core/SkVertices.h"
#include "src/base/SkRandom.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
#include "tools/viewer/ClickHandlerSlide.h"

struct VSphere {
    SkV2     fCenter;
    SkScalar fRadius;

    VSphere(SkV2 center, SkScalar radius) : fCenter(center), fRadius(radius) {}

    bool contains(SkV2 v) const {
        return (v - fCenter).length() <= fRadius;
    }

    SkV2 pinLoc(SkV2 p) const {
        auto v = p - fCenter;
        if (v.length() > fRadius) {
            v *= (fRadius / v.length());
        }
        return fCenter + v;
    }

    SkV3 computeUnitV3(SkV2 v) const {
        v = (v - fCenter) * (1 / fRadius);
        SkScalar len2 = v.lengthSquared();
        if (len2 > 1) {
            v = v.normalize();
            len2 = 1;
        }
        SkScalar z = SkScalarSqrt(1 - len2);
        return {v.x, v.y, z};
    }

    struct RotateInfo {
        SkV3    fAxis;
        SkScalar fAngle;
    };

    RotateInfo computeRotationInfo(SkV2 a, SkV2 b) const {
        SkV3 u = this->computeUnitV3(a);
        SkV3 v = this->computeUnitV3(b);
        SkV3 axis = u.cross(v);
        SkScalar length = axis.length();

        if (!SkScalarNearlyZero(length)) {
            return {axis * (1.0f / length), std::acos(u.dot(v))};
        }
        return {{0, 0, 0}, 0};
    }

    SkM44 computeRotation(SkV2 a, SkV2 b) const {
        auto [axis, angle] = this->computeRotationInfo(a, b);
        return SkM44::Rotate(axis, angle);
    }
};

static SkM44 inv(const SkM44& m) {
    SkM44 inverse;
    SkAssertResult(m.invert(&inverse));
    return inverse;
}

// Compute the inverse transpose (of the upper-left 3x3) of a matrix, used to transform vectors
static SkM44 normals(SkM44 m) {
    m.setRow(3, {0, 0, 0, 1});
    m.setCol(3, {0, 0, 0, 1});
    SkAssertResult(m.invert(&m));
    return m.transpose();
}

class ThreeDSlide : public ClickHandlerSlide {
protected:
    float   fNear = 0.05f;
    float   fFar = 4;
    float   fAngle = SK_ScalarPI / 12;

    SkV3    fEye { 0, 0, 1.0f/std::tan(fAngle/2) - 1 };
    SkV3    fCOA { 0, 0, 0 };
    SkV3    fUp  { 0, 1, 0 };

public:
    void concatCamera(SkCanvas* canvas, const SkRect& area, SkScalar zscale) {
        SkM44 camera = SkM44::LookAt(fEye, fCOA, fUp),
              perspective = SkM44::Perspective(fNear, fFar, fAngle),
              viewport = SkM44::Translate(area.centerX(), area.centerY(), 0) *
                         SkM44::Scale(area.width()*0.5f, area.height()*0.5f, zscale);

        canvas->concat(viewport * perspective * camera * inv(viewport));
    }
};

struct Face {
    SkScalar fRx, fRy;
    SkColor  fColor;

    static SkM44 T(SkScalar x, SkScalar y, SkScalar z) {
        return SkM44::Translate(x, y, z);
    }

    static SkM44 R(SkV3 axis, SkScalar rad) {
        return SkM44::Rotate(axis, rad);
    }

    SkM44 asM44(SkScalar scale) const {
        return R({0,1,0}, fRy) * R({1,0,0}, fRx) * T(0, 0, scale);
    }
};

static bool isFrontFacing(const SkM44& m) {
    SkM44 m2(SkM44::kUninitialized_Constructor);
    if (!m.invert(&m2)) {
        m2.setIdentity();
    }
    /*
     *  Classically we want to dot the transpose(inverse(ctm)) with our surface normal.
     *  In this case, the normal is known to be {0, 0, 1}, so we only actually need to look
     *  at the z-scale of the inverse (the transpose doesn't change the main diagonal, so
     *  no need to actually transpose).
     */
    return m2.rc(2,2) > 0;
}

const Face faces[] = {
    {             0,             0,  SK_ColorRED }, // front
    {             0,   SK_ScalarPI,  SK_ColorGREEN }, // back

    { SK_ScalarPI/2,             0,  SK_ColorBLUE }, // top
    {-SK_ScalarPI/2,             0,  SK_ColorCYAN }, // bottom

    {             0, SK_ScalarPI/2,  SK_ColorMAGENTA }, // left
    {             0,-SK_ScalarPI/2,  SK_ColorYELLOW }, // right
};

#include "include/effects/SkRuntimeEffect.h"

struct LightOnSphere {
    SkV2     fLoc;
    SkScalar fDistance;
    SkScalar fRadius;

    SkV3 computeWorldPos(const VSphere& s) const {
        return s.computeUnitV3(fLoc) * fDistance;
    }

    void draw(SkCanvas* canvas) const {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorWHITE);
        canvas->drawCircle(fLoc.x, fLoc.y, fRadius + 2, paint);
        paint.setColor(SK_ColorBLACK);
        canvas->drawCircle(fLoc.x, fLoc.y, fRadius, paint);
    }
};

#include "src/base/SkTime.h"

class RotateAnimator {
    SkV3        fAxis = {0, 0, 0};
    SkScalar    fAngle = 0,
                fPrevAngle = 1234567;
    double      fNow = 0,
                fPrevNow = 0;

    SkScalar    fAngleSpeed = 0,
                fAngleSign = 1;

    inline static constexpr double kSlowDown = 4;
    inline static constexpr SkScalar kMaxSpeed = 16;

public:
    void update(SkV3 axis, SkScalar angle) {
        if (angle != fPrevAngle) {
            fPrevAngle = fAngle;
            fAngle = angle;

            fPrevNow = fNow;
            fNow = SkTime::GetSecs();

            fAxis = axis;
        }
    }

    SkM44 rotation() {
        if (fAngleSpeed > 0) {
            double now = SkTime::GetSecs();
            double dtime = now - fPrevNow;
            fPrevNow = now;
            double delta = fAngleSign * fAngleSpeed * dtime;
            fAngle += delta;
            fAngleSpeed -= kSlowDown * dtime;
            if (fAngleSpeed < 0) {
                fAngleSpeed = 0;
            }
        }
        return SkM44::Rotate(fAxis, fAngle);

    }

    void start() {
        if (fPrevNow != fNow) {
            fAngleSpeed = (fAngle - fPrevAngle) / (fNow - fPrevNow);
            fAngleSign = fAngleSpeed < 0 ? -1 : 1;
            fAngleSpeed = std::min(kMaxSpeed, std::abs(fAngleSpeed));
        } else {
            fAngleSpeed = 0;
        }
        fPrevNow = SkTime::GetSecs();
        fAngle = 0;
    }

    void reset() {
        fAngleSpeed = 0;
        fAngle = 0;
        fPrevAngle = 1234567;
    }

    bool isAnimating() const { return fAngleSpeed != 0; }
};

class CubeBaseSlide : public ThreeDSlide {
    enum {
        DX = 400,
        DY = 300
    };

    SkM44 fRotation;        // part of model

    RotateAnimator fRotateAnimator;

protected:
    enum Flags {
        kCanRunOnCPU    = 1 << 0,
        kShowLightDome  = 1 << 1,
    };

    LightOnSphere fLight = {{200 + DX, 200 + DY}, 800, 12};

    VSphere fSphere;
    Flags   fFlags;

public:
    CubeBaseSlide(Flags flags)
        : fSphere({200 + DX, 200 + DY}, 400)
        , fFlags(flags)
    {}

    bool onChar(SkUnichar uni) override {
        switch (uni) {
            case 'Z': fLight.fDistance += 10; return true;
            case 'z': fLight.fDistance -= 10; return true;
        }
        return this->ThreeDSlide::onChar(uni);
    }

    virtual void drawFace(SkCanvas*, SkColor, int face, bool front, const SkM44& localToWorld) = 0;

    void draw(SkCanvas* canvas) override {
        if (!canvas->recordingContext() && !(fFlags & kCanRunOnCPU)) {
            return;
        }

        canvas->save();
        canvas->translate(DX, DY);

        this->concatCamera(canvas, {0, 0, 400, 400}, 200);

        SkM44 m = fRotateAnimator.rotation() * fRotation;
        for (bool front : {false, true}) {
            int index = 0;
            for (auto f : faces) {
                SkAutoCanvasRestore acr(canvas, true);

                SkM44 trans = SkM44::Translate(200, 200, 0);   // center of the rotation

                canvas->concat(trans);

                // "World" space - content is centered at the origin, in device scale (+-200)
                SkM44 localToWorld = m * f.asM44(200) * inv(trans);

                canvas->concat(localToWorld);
                this->drawFace(canvas, f.fColor, index++, front, localToWorld);
            }
        }

        canvas->restore();  // camera & center the content in the window

        if (fFlags & kShowLightDome){
            fLight.draw(canvas);

            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setColor(0x40FF0000);
            canvas->drawCircle(fSphere.fCenter.x, fSphere.fCenter.y, fSphere.fRadius, paint);
            canvas->drawLine(fSphere.fCenter.x, fSphere.fCenter.y - fSphere.fRadius,
                             fSphere.fCenter.x, fSphere.fCenter.y + fSphere.fRadius, paint);
            canvas->drawLine(fSphere.fCenter.x - fSphere.fRadius, fSphere.fCenter.y,
                             fSphere.fCenter.x + fSphere.fRadius, fSphere.fCenter.y, paint);
        }
    }

    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) override {
        SkV2 p = fLight.fLoc - SkV2{x, y};
        if (p.length() <= fLight.fRadius) {
            Click* c = new Click();
            c->fMeta.setS32("type", 0);
            return c;
        }
        if (fSphere.contains({x, y})) {
            Click* c = new Click();
            c->fMeta.setS32("type", 1);

            fRotation = fRotateAnimator.rotation() * fRotation;
            fRotateAnimator.reset();
            return c;
        }
        return nullptr;
    }
    bool onClick(Click* click) override {
        if (click->fMeta.hasS32("type", 0)) {
            fLight.fLoc = fSphere.pinLoc({click->fCurr.fX, click->fCurr.fY});
            return true;
        }
        if (click->fMeta.hasS32("type", 1)) {
            if (click->fState == skui::InputState::kUp) {
                fRotation = fRotateAnimator.rotation() * fRotation;
                fRotateAnimator.start();
            } else {
                auto [axis, angle] = fSphere.computeRotationInfo(
                                                {click->fOrig.fX, click->fOrig.fY},
                                                {click->fCurr.fX, click->fCurr.fY});
                fRotateAnimator.update(axis, angle);
            }
            return true;
        }
        return true;
    }

    bool animate(double nanos) override {
        return fRotateAnimator.isAnimating();
    }
};

class Bump3DSlide : public CubeBaseSlide {
    sk_sp<SkShader>        fBmpShader, fImgShader;
    sk_sp<SkRuntimeEffect> fEffect;
    SkRRect                fRR;

public:
    Bump3DSlide() : CubeBaseSlide(Flags(kCanRunOnCPU | kShowLightDome)) { fName = "bump3d"; }

    void load(SkScalar w, SkScalar h) override {
        fRR = SkRRect::MakeRectXY({20, 20, 380, 380}, 50, 50);
        auto img = ToolUtils::GetResourceAsImage("images/brickwork-texture.jpg");
        fImgShader = img->makeShader(SkSamplingOptions(), SkMatrix::Scale(2, 2));
        img = ToolUtils::GetResourceAsImage("images/brickwork_normal-map.jpg");
        fBmpShader = img->makeShader(SkSamplingOptions(), SkMatrix::Scale(2, 2));

        const char code[] = R"(
            uniform shader color_map;
            uniform shader normal_map;

            uniform float4x4 localToWorld;
            uniform float4x4 localToWorldAdjInv;
            uniform float3   lightPos;

            float3 convert_normal_sample(half4 c) {
                float3 n = 2 * c.rgb - 1;
                n.y = -n.y;
                return n;
            }

            half4 main(float2 p) {
                float3 norm = convert_normal_sample(normal_map.eval(p));
                float3 plane_norm = normalize(localToWorldAdjInv * norm.xyz0).xyz;

                float3 plane_pos = (localToWorld * p.xy01).xyz;
                float3 light_dir = normalize(lightPos - plane_pos);

                float ambient = 0.2;
                float dp = dot(plane_norm, light_dir);
                float scale = min(ambient + max(dp, 0), 1);

                return color_map.eval(p) * scale.xxx1;
            }
        )";
        auto [effect, error] = SkRuntimeEffect::MakeForShader(SkString(code));
        if (!effect) {
            SkDebugf("runtime error %s\n", error.c_str());
        }
        fEffect = effect;
    }

    void drawFace(SkCanvas* canvas, SkColor color, int face, bool front,
                  const SkM44& localToWorld) override {
        if (!front || !isFrontFacing(canvas->getLocalToDevice())) {
            return;
        }

        SkRuntimeShaderBuilder builder(fEffect);
        builder.uniform("lightPos") = fLight.computeWorldPos(fSphere);
        builder.uniform("localToWorld") = localToWorld;
        builder.uniform("localToWorldAdjInv") = normals(localToWorld);

        builder.child("color_map")  = fImgShader;
        builder.child("normal_map") = fBmpShader;

        SkPaint paint;
        paint.setColor(color);
        paint.setShader(builder.makeShader());

        canvas->drawRRect(fRR, paint);
    }
};
DEF_SLIDE( return new Bump3DSlide; )

#include "modules/skottie/include/Skottie.h"

class SkottieCubeSlide : public CubeBaseSlide {
    sk_sp<skottie::Animation> fAnim[6];

public:
    SkottieCubeSlide() : CubeBaseSlide(kCanRunOnCPU) { fName = "skottie3d"; }

    void load(SkScalar w, SkScalar h) override {
        const char* files[] = {
            "skottie/skottie-chained-mattes.json",
            "skottie/skottie-gradient-ramp.json",
            "skottie/skottie_sample_2.json",
            "skottie/skottie-3d-3planes.json",
            "skottie/skottie-text-animator-4.json",
            "skottie/skottie-motiontile-effect-phase.json",

        };
        for (unsigned i = 0; i < std::size(files); ++i) {
            if (auto stream = GetResourceAsStream(files[i])) {
                fAnim[i] = skottie::Animation::Make(stream.get());
            }
        }
    }

    void drawFace(SkCanvas* canvas, SkColor color, int face, bool front, const SkM44&) override {
        if (!front || !isFrontFacing(canvas->getLocalToDevice())) {
            return;
        }

        SkPaint paint;
        paint.setColor(color);
        SkRect r = {0, 0, 400, 400};
        canvas->drawRect(r, paint);
        fAnim[face]->render(canvas, &r);
    }

    bool animate(double nanos) override {
        for (auto& anim : fAnim) {
            SkScalar dur = anim->duration();
            SkScalar t = fmod(1e-9 * nanos, dur) / dur;
            anim->seek(t);
        }
        return true;
    }
};
DEF_SLIDE( return new SkottieCubeSlide; )
