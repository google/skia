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
#include "include/core/SkVertices.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"
#include "tools/Resources.h"

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
            return {axis * (1.0f / length), acos(u.dot(v))};
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

class Sample3DView : public Sample {
protected:
    float   fNear = 0.05f;
    float   fFar = 4;
    float   fAngle = SK_ScalarPI / 12;

    SkV3    fEye { 0, 0, 1.0f/tan(fAngle/2) - 1 };
    SkV3    fCOA { 0, 0, 0 };
    SkV3    fUp  { 0, 1, 0 };

    const char* kLocalToWorld = "local_to_world";

public:
    void concatCamera(SkCanvas* canvas, const SkRect& area, SkScalar zscale) {
        SkM44 camera = Sk3LookAt(fEye, fCOA, fUp),
              perspective = Sk3Perspective(fNear, fFar, fAngle),
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

static bool front(const SkM44& m) {
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

#include "include/core/SkTime.h"

class RotateAnimator {
    SkV3        fAxis = {0, 0, 0};
    SkScalar    fAngle = 0,
                fPrevAngle = 1234567;
    double      fNow = 0,
                fPrevNow = 0;

    SkScalar    fAngleSpeed = 0,
                fAngleSign = 1;

    static constexpr double kSlowDown = 4;
    static constexpr SkScalar kMaxSpeed = 16;

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

class SampleCubeBase : public Sample3DView {
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
    SampleCubeBase(Flags flags)
        : fSphere({200 + DX, 200 + DY}, 400)
        , fFlags(flags)
    {}

    bool onChar(SkUnichar uni) override {
        switch (uni) {
            case 'Z': fLight.fDistance += 10; return true;
            case 'z': fLight.fDistance -= 10; return true;
        }
        return this->Sample3DView::onChar(uni);
    }

    virtual void drawContent(SkCanvas* canvas, SkColor, int index, bool drawFront) = 0;

    void onDrawContent(SkCanvas* canvas) override {
        if (!canvas->recordingContext() && !(fFlags & kCanRunOnCPU)) {
            return;
        }

        canvas->save();
        canvas->translate(DX, DY);

        this->concatCamera(canvas, {0, 0, 400, 400}, 200);

        for (bool drawFront : {false, true}) {
            int index = 0;
            for (auto f : faces) {
                SkAutoCanvasRestore acr(canvas, true);

                SkM44 trans = SkM44::Translate(200, 200, 0);   // center of the rotation
                SkM44 m = fRotateAnimator.rotation() * fRotation * f.asM44(200);

                canvas->concat(trans);

                // "World" space - content is centered at the origin, in device scale (+-200)
                canvas->markCTM(kLocalToWorld);

                canvas->concat(m * inv(trans));
                this->drawContent(canvas, f.fColor, index++, drawFront);
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

    bool onAnimate(double nanos) override {
        return fRotateAnimator.isAnimating();
    }

private:
    using INHERITED = Sample3DView;
};

class SampleBump3D : public SampleCubeBase {
    sk_sp<SkShader>        fBmpShader, fImgShader;
    sk_sp<SkRuntimeEffect> fEffect;
    SkRRect                fRR;

public:
    SampleBump3D() : SampleCubeBase(Flags(kCanRunOnCPU | kShowLightDome)) {}

    SkString name() override { return SkString("bump3d"); }

    void onOnceBeforeDraw() override {
        fRR = SkRRect::MakeRectXY({20, 20, 380, 380}, 50, 50);
        auto img = GetResourceAsImage("images/brickwork-texture.jpg");
        fImgShader = img->makeShader(SkSamplingOptions(), SkMatrix::Scale(2, 2));
        img = GetResourceAsImage("images/brickwork_normal-map.jpg");
        fBmpShader = img->makeShader(SkSamplingOptions(), SkMatrix::Scale(2, 2));

        const char code[] = R"(
            uniform shader color_map;
            uniform shader normal_map;

            layout (marker=local_to_world)          uniform float4x4 localToWorld;
            layout (marker=normals(local_to_world)) uniform float4x4 localToWorldAdjInv;
            uniform float3   lightPos;

            float3 convert_normal_sample(half4 c) {
                float3 n = 2 * c.rgb - 1;
                n.y = -n.y;
                return n;
            }

            half4 main(float2 p) {
                float3 norm = convert_normal_sample(sample(normal_map, p));
                float3 plane_norm = normalize(localToWorldAdjInv * norm.xyz0).xyz;

                float3 plane_pos = (localToWorld * p.xy01).xyz;
                float3 light_dir = normalize(lightPos - plane_pos);

                float ambient = 0.2;
                float dp = dot(plane_norm, light_dir);
                float scale = min(ambient + max(dp, 0), 1);

                return sample(color_map, p) * scale.xxx1;
            }
        )";
        auto [effect, error] = SkRuntimeEffect::Make(SkString(code));
        if (!effect) {
            SkDebugf("runtime error %s\n", error.c_str());
        }
        fEffect = effect;
    }

    void drawContent(SkCanvas* canvas, SkColor color, int index, bool drawFront) override {
        if (!drawFront || !front(canvas->getLocalToDevice())) {
            return;
        }

        SkRuntimeShaderBuilder builder(fEffect);
        builder.uniform("lightPos") = fLight.computeWorldPos(fSphere);
        // localToWorld matrices are automatically populated, via layout(marker)

        builder.child("color_map")  = fImgShader;
        builder.child("normal_map") = fBmpShader;

        SkPaint paint;
        paint.setColor(color);
        paint.setShader(builder.makeShader(nullptr, true));

        canvas->drawRRect(fRR, paint);
    }
};
DEF_SAMPLE( return new SampleBump3D; )

class SampleVerts3D : public SampleCubeBase {
    sk_sp<SkRuntimeEffect> fEffect;
    sk_sp<SkVertices>      fVertices;

public:
    SampleVerts3D() : SampleCubeBase(kShowLightDome) {}

    SkString name() override { return SkString("verts3d"); }

    void onOnceBeforeDraw() override {
        using Attr = SkVertices::Attribute;
        Attr attrs[] = {
            Attr(Attr::Type::kFloat3, Attr::Usage::kNormalVector),
        };

        SkVertices::Builder builder(SkVertices::kTriangleFan_VertexMode, 66, 0, attrs, 1);

        SkPoint* pos = builder.positions();
        SkV3* nrm = (SkV3*)builder.customData();

        SkPoint center = { 200, 200 };
        SkScalar radius = 200;

        pos[0] = center;
        nrm[0] = { 0, 0, 1 };

        for (int i = 0; i < 65; ++i) {
            SkScalar t = (i / 64.0f) * 2 * SK_ScalarPI;
            SkScalar s = SkScalarSin(t),
                     c = SkScalarCos(t);
            pos[i + 1] = center + SkPoint { c * radius, s * radius };
            nrm[i + 1] = { c, s, 0 };
        }

        fVertices = builder.detach();

        const char code[] = R"(
            varying float3 vtx_normal;

            layout (marker=local_to_world)          uniform float4x4 localToWorld;
            layout (marker=normals(local_to_world)) uniform float4x4 localToWorldAdjInv;
            uniform float3   lightPos;

            half4 main(float2 p) {
                float3 norm = normalize(vtx_normal);
                float3 plane_norm = normalize(localToWorldAdjInv * norm.xyz0).xyz;

                float3 plane_pos = (localToWorld * p.xy01).xyz;
                float3 light_dir = normalize(lightPos - plane_pos);

                float ambient = 0.2;
                float dp = dot(plane_norm, light_dir);
                float scale = min(ambient + max(dp, 0), 1);

                return half4(0.7, 0.9, 0.3, 1) * scale.xxx1;
            }
        )";
        auto [effect, error] = SkRuntimeEffect::Make(SkString(code));
        if (!effect) {
            SkDebugf("runtime error %s\n", error.c_str());
        }
        fEffect = effect;
    }

    void drawContent(SkCanvas* canvas, SkColor color, int index, bool drawFront) override {
        if (!drawFront || !front(canvas->getLocalToDevice())) {
            return;
        }

        SkRuntimeShaderBuilder builder(fEffect);
        builder.uniform("lightPos") = fLight.computeWorldPos(fSphere);

        SkPaint paint;
        paint.setColor(color);
        paint.setShader(builder.makeShader(nullptr, true));

        canvas->drawVertices(fVertices, paint);
    }
};
DEF_SAMPLE( return new SampleVerts3D; )

#include "modules/skottie/include/Skottie.h"

class SampleSkottieCube : public SampleCubeBase {
    sk_sp<skottie::Animation> fAnim[6];

public:
    SampleSkottieCube() : SampleCubeBase(kCanRunOnCPU) {}

    SkString name() override { return SkString("skottie3d"); }

    void onOnceBeforeDraw() override {
        const char* files[] = {
            "skottie/skottie-chained-mattes.json",
            "skottie/skottie-gradient-ramp.json",
            "skottie/skottie_sample_2.json",
            "skottie/skottie-3d-3planes.json",
            "skottie/skottie-text-animator-4.json",
            "skottie/skottie-motiontile-effect-phase.json",

        };
        for (unsigned i = 0; i < SK_ARRAY_COUNT(files); ++i) {
            if (auto stream = GetResourceAsStream(files[i])) {
                fAnim[i] = skottie::Animation::Make(stream.get());
            }
        }
    }

    void drawContent(SkCanvas* canvas, SkColor color, int index, bool drawFront) override {
        if (!drawFront || !front(canvas->getLocalToDevice())) {
            return;
        }

        SkPaint paint;
        paint.setColor(color);
        SkRect r = {0, 0, 400, 400};
        canvas->drawRect(r, paint);
        fAnim[index]->render(canvas, &r);
    }

    bool onAnimate(double nanos) override {
        for (auto& anim : fAnim) {
            SkScalar dur = anim->duration();
            SkScalar t = fmod(1e-9 * nanos, dur) / dur;
            anim->seek(t);
        }
        return true;
    }
};
DEF_SAMPLE( return new SampleSkottieCube; )
