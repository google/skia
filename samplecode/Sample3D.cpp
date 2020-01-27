/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/private/SkM44.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"
#include "tools/Resources.h"

static SkV3 normalize(SkV3 v) { return v * (1.0f / v.length()); }

struct SkVec2 {
    SkScalar x, y;

    bool operator==(const SkVec2 v) const { return x == v.x && y == v.y; }
    bool operator!=(const SkVec2 v) const { return !(*this == v); }

    static SkScalar   Dot(SkVec2 a, SkVec2 b) { return a.x * b.x + a.y * b.y; }
    static SkScalar Cross(SkVec2 a, SkVec2 b) { return a.x * b.y - a.y * b.x; }

    SkVec2 operator-() const { return {-x, -y}; }
    SkVec2 operator+(SkVec2 v) const { return {x+v.x, y+v.y}; }
    SkVec2 operator-(SkVec2 v) const { return {x-v.x, y-v.y}; }

    SkVec2 operator*(SkVec2 v) const { return {x*v.x, y*v.y}; }
    friend SkVec2 operator*(SkVec2 v, SkScalar s) { return {v.x*s, v.y*s}; }
    friend SkVec2 operator*(SkScalar s, SkVec2 v) { return {v.x*s, v.y*s}; }

    void operator+=(SkVec2 v) { *this = *this + v; }
    void operator-=(SkVec2 v) { *this = *this - v; }
    void operator*=(SkVec2 v) { *this = *this * v; }
    void operator*=(SkScalar s) { *this = *this * s; }

    SkScalar lengthSquared() const { return Dot(*this, *this); }
    SkScalar length() const { return SkScalarSqrt(this->lengthSquared()); }

    SkScalar   dot(SkVec2 v) const { return Dot(*this, v); }
    SkScalar cross(SkVec2 v) const { return Cross(*this, v); }
};

static SkVec2 normalize(SkVec2 v) {
    SkScalar len = v.length();
    SkASSERT(len > 0);
    return v * (1.0f / len);
}

struct VSphere {
    SkVec2   fCenter;
    SkScalar fRadius;

    VSphere(SkVec2 center, SkScalar radius) : fCenter(center), fRadius(radius) {}

    bool contains(SkVec2 v) const {
        return (v - fCenter).length() <= fRadius;
    }

    SkVec2 pinLoc(SkVec2 p) const {
        auto v = p - fCenter;
        if (v.length() > fRadius) {
            v *= (fRadius / v.length());
        }
        return fCenter + v;
    }

    SkV3 computeUnitV3(SkVec2 v) const {
        v = (v - fCenter) * (1 / fRadius);
        SkScalar len2 = v.lengthSquared();
        if (len2 > 1) {
            v = normalize(v);
            len2 = 1;
        }
        SkScalar z = SkScalarSqrt(1 - len2);
        return {v.x, v.y, z};
    }

    SkM44 computeRotation(SkVec2 a, SkVec2 b) {
        SkV3 u = this->computeUnitV3(a);
        SkV3 v = this->computeUnitV3(b);
        SkV3 axis = u.cross(v);
        SkScalar sinValue = axis.length();
        SkScalar cosValue = u.dot(v);

        SkM44 m;
        if (!SkScalarNearlyZero(sinValue)) {
            m.setRotateUnitSinCos(axis * (1.0f / sinValue), sinValue, cosValue);
        }
        return m;
    }
};

static SkM44 inv(const SkM44& m) {
    SkM44 inverse;
    SkAssertResult(m.invert(&inverse));
    return inverse;
}

static SkPoint project(const SkM44& m, SkV4 p) {
    auto v = m * p;
    return {v.x / v.w, v.y / v.w};
}

class Sample3DView : public Sample {
protected:
    float   fNear = 0.05f;
    float   fFar = 4;
    float   fAngle = SK_ScalarPI / 12;

    SkV3    fEye { 0, 0, 1.0f/tan(fAngle/2) - 1 };
    SkV3    fCOA { 0, 0, 0 };
    SkV3    fUp  { 0, 1, 0 };

    SkM44   fRot;
    SkV3    fTrans;

    void rotate(float x, float y, float z) {
        SkM44 r;
        if (x) {
            r.setRotateUnit({1, 0, 0}, x);
        } else if (y) {
            r.setRotateUnit({0, 1, 0}, y);
        } else {
            r.setRotateUnit({0, 0, 1}, z);
        }
        fRot = r * fRot;
    }

public:
    void saveCamera(SkCanvas* canvas, const SkRect& area, SkScalar zscale) {
        SkM44 camera = Sk3LookAt(fEye, fCOA, fUp),
              perspective = Sk3Perspective(fNear, fFar, fAngle),
              viewport = SkM44::Translate(area.centerX(), area.centerY(), 0) *
                         SkM44::Scale(area.width()*0.5f, area.height()*0.5f, zscale);

        // want "world" to be in our big coordinates (e.g. area), so apply this inverse
        // as part of our "camera".
        canvas->experimental_saveCamera(viewport * perspective, camera * inv(viewport));
    }

    bool onChar(SkUnichar uni) override {
        float delta = SK_ScalarPI / 30;
        switch (uni) {
            case '8': this->rotate( delta, 0, 0); return true;
            case '2': this->rotate(-delta, 0, 0); return true;
            case '4': this->rotate(0,  delta, 0); return true;
            case '6': this->rotate(0, -delta, 0); return true;
            case '-': this->rotate(0, 0,  delta); return true;
            case '+': this->rotate(0, 0, -delta); return true;

            case 'i': fTrans.z += 0.1f; SkDebugf("z %g\n", fTrans.z); return true;
            case 'k': fTrans.z -= 0.1f; SkDebugf("z %g\n", fTrans.z); return true;

            case 'n': fNear += 0.1f; SkDebugf("near %g\n", fNear); return true;
            case 'N': fNear -= 0.1f; SkDebugf("near %g\n", fNear); return true;
            case 'f': fFar  += 0.1f; SkDebugf("far  %g\n", fFar); return true;
            case 'F': fFar  -= 0.1f; SkDebugf("far  %g\n", fFar); return true;
            default: break;
        }
        return false;
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
    SkM44 m2;
    m.invert(&m2);
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

#include "include/core/SkColorFilter.h"
#include "include/effects/SkColorMatrix.h"

static SkColorMatrix comput_planar_lighting(SkCanvas* canvas, SkV3 lightDir) {
    SkM44 l2w = canvas->experimental_getLocalToWorld();
    auto normal = normalize(l2w * SkV3{0, 0, 1});
    float dot = -normal.dot(lightDir);

    SkColorMatrix cm;
    if (dot < 0) {
        dot = 0;
    }

    float ambient = 0.5f;
    float scale = ambient + dot;
    cm.setScale(scale, scale, scale, 1);
    return cm;
}

struct Light {
    SkPoint fCenter;
    SkPoint fEndPt;
    SkScalar fRadius;
    SkScalar fHeight;

    bool hitTest(SkScalar x, SkScalar y) const {
        auto xx = x - fCenter.fX;
        auto yy = y - fCenter.fY;
        return xx*xx + yy*yy <= fRadius*fRadius;
    }

    void update(SkScalar x, SkScalar y) {
        auto xx = x - fCenter.fX;
        auto yy = y - fCenter.fY;
        auto len = SkScalarSqrt(xx*xx + yy*yy);
        if (len > fRadius) {
            xx *= fRadius / len;
            yy *= fRadius / len;
        }
        fEndPt = {fCenter.fX + xx, fCenter.fY + yy};
    }

    SkV3 getDir() const {
        auto pt = fEndPt - fCenter;
        return normalize({pt.fX, pt.fY, -fHeight});
    }

    void draw(SkCanvas* canvas) {
        SkPaint paint;
        paint.setAntiAlias(true);
        canvas->drawCircle(fCenter.fX, fCenter.fY, 5, paint);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawCircle(fCenter.fX, fCenter.fY, fRadius, paint);
        paint.setColor(SK_ColorRED);
        canvas->drawLine(fCenter.fX, fCenter.fY, fEndPt.fX, fEndPt.fY, paint);
    }
};

class SampleRR3D : public Sample3DView {
    SkRRect fRR;
    Light   fLight = {
        {60, 60}, {60, 60}, 50, 10
    };
    sk_sp<SkShader> fShader;

    SkString name() override { return SkString("rrect3d"); }

    void onOnceBeforeDraw() override {
        fRR = SkRRect::MakeRectXY({20, 20, 380, 380}, 50, 50);
        fShader = GetResourceAsImage("images/mandrill_128.png")
                        ->makeShader(SkMatrix::MakeScale(3, 3));
    }

    bool onChar(SkUnichar uni) override {
        return this->Sample3DView::onChar(uni);
    }

    void drawContent(SkCanvas* canvas, const SkM44& m) {
        SkM44 trans = SkM44::Translate(200, 200, 0);   // center of the rotation

        canvas->experimental_concat44(trans * fRot * m * inv(trans));

        if (!front(canvas->experimental_getLocalToDevice())) {
            return;
        }

        SkPaint paint;
        paint.setAlphaf(front(canvas->experimental_getLocalToDevice()) ? 1 : 0.25f);
        paint.setShader(fShader);

        SkColorMatrix cm = comput_planar_lighting(canvas, fLight.getDir());
        paint.setColorFilter(SkColorFilters::Matrix(cm));

        canvas->drawRRect(fRR, paint);
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->save();
        canvas->translate(400, 300);

        this->saveCamera(canvas, {0, 0, 400, 400}, 200);

        for (auto f : faces) {
            SkAutoCanvasRestore acr(canvas, true);
            this->drawContent(canvas, f.asM44(200));
        }

        canvas->restore();
        canvas->restore();

        fLight.draw(canvas);
    }

    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) override {
        if (fLight.hitTest(x, y)) {
            return new Click();
        }
        return nullptr;
    }
    bool onClick(Click* click) override {
        fLight.update(click->fCurr.fX, click->fCurr.fY);
        return true;
    }
};
DEF_SAMPLE( return new SampleRR3D(); )

#include "include/effects/SkRuntimeEffect.h"

struct LightPos {
    SkV4     fPos;
    SkScalar fUIRadius;

    bool hitTest(SkScalar x, SkScalar y) const {
        auto xx = x - fPos.x;
        auto yy = y - fPos.y;
        return xx*xx + yy*yy <= fUIRadius*fUIRadius;
    }

    void update(SkScalar x, SkScalar y) {
        fPos.x = x;
        fPos.y = y;
    }

    void draw(SkCanvas* canvas) {
        SkPaint paint;
        paint.setAntiAlias(true);

        SkAutoCanvasRestore acr(canvas, true);
        canvas->experimental_concat44(SkM44::Translate(0, 0, fPos.z));
        canvas->drawCircle(fPos.x, fPos.y, fUIRadius, paint);
    }
};

class SamplePointLight3D : public Sample3DView {
    SkRRect fRR;
    LightPos fLight = {{200, 200, 800, 1}, 8};

    sk_sp<SkShader> fShader;
    sk_sp<SkRuntimeEffect> fEffect;

    SkM44 fWorldToClick,
          fClickToWorld;

    SkString name() override { return SkString("pointlight3d"); }

    void onOnceBeforeDraw() override {
        fRR = SkRRect::MakeRectXY({20, 20, 380, 380}, 50, 50);
        fShader = GetResourceAsImage("images/mandrill_128.png")
                        ->makeShader(SkMatrix::MakeScale(3, 3));

        const char code[] = R"(
        //    in fragmentProcessor texture;
        //       color = sample(texture) * half(scale);

            uniform float4x4 localToWorld;
            uniform float3   lightPos;

            // TODO: Remove these helpers once all intrinsics work on the raster backend
            float3 normalize_(float3 v) {
                return v / sqrt(dot(v, v));
            }

            float max_(float a, float b) {
                return a > b ? a : b;
            }

            void main(float x, float y, inout half4 color) {
                float3 plane_pos = (localToWorld * float4(x, y, 0, 1)).xyz;
                float3 plane_norm = normalize_((localToWorld * float4(0, 0, 1, 0)).xyz);
                float3 light_dir = normalize_(lightPos - plane_pos);
                float ambient = 0.5;
                float dp = dot(plane_norm, light_dir);
                float scale = ambient + max_(dp, 0);

                color = color * half4(float4(scale, scale, scale, 1));
            }
        )";
        auto [effect, error] = SkRuntimeEffect::Make(SkString(code));
        if (!effect) {
            SkDebugf("runtime error %s\n", error.c_str());
        }
        fEffect = effect;
    }

    bool onChar(SkUnichar uni) override {
        switch (uni) {
            case 'Z': fLight.fPos.z += 10; return true;
            case 'z': fLight.fPos.z -= 10; return true;
        }
        return this->Sample3DView::onChar(uni);
    }

    void drawContent(SkCanvas* canvas, const SkM44& m, SkColor color) {
        SkM44 trans = SkM44::Translate(200, 200, 0);   // center of the rotation

        canvas->experimental_concat44(trans * fRot * m * inv(trans));

        // wonder if the runtimeeffect can do this reject? (in a setup function)
        if (!front(canvas->experimental_getLocalToDevice())) {
            return;
        }

        struct Uniforms {
            SkM44  fLocalToWorld;
            SkV3   fLightPos;
        } uni;
        uni.fLocalToWorld = canvas->experimental_getLocalToWorld();
        uni.fLightPos     = {fLight.fPos.x, fLight.fPos.y, fLight.fPos.z};
        sk_sp<SkData> data = SkData::MakeWithCopy(&uni, sizeof(uni));

        SkPaint paint;
        paint.setColor(color);
        paint.setShader(fEffect->makeShader(data, &fShader, 0, nullptr, true));

        canvas->drawRRect(fRR, paint);
    }

    void setClickToWorld(SkCanvas* canvas, const SkM44& clickM) {
        auto l2d = canvas->experimental_getLocalToDevice();
        fWorldToClick = inv(clickM) * l2d;
        fClickToWorld = inv(fWorldToClick);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkM44 clickM = canvas->experimental_getLocalToDevice();

        canvas->save();
        canvas->translate(400, 300);

        this->saveCamera(canvas, {0, 0, 400, 400}, 200);

        this->setClickToWorld(canvas, clickM);

        for (auto f : faces) {
            SkAutoCanvasRestore acr(canvas, true);
            this->drawContent(canvas, f.asM44(200), f.fColor);
        }

        fLight.draw(canvas);
        canvas->restore();
        canvas->restore();
    }

    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) override {
        auto L = fWorldToClick * fLight.fPos;
        SkPoint c = project(fClickToWorld, {x, y, L.z/L.w, 1});
        if (fLight.hitTest(c.fX, c.fY)) {
            return new Click();
        }
        return nullptr;
    }
    bool onClick(Click* click) override {
        auto L = fWorldToClick * fLight.fPos;
        SkPoint c = project(fClickToWorld, {click->fCurr.fX, click->fCurr.fY, L.z/L.w, 1});
        fLight.update(c.fX, c.fY);
        return true;
    }
};
DEF_SAMPLE( return new SamplePointLight3D(); )

#include "include/core/SkColorPriv.h"
#include "include/core/SkSurface.h"

struct LightOnSphere {
    SkVec2   fLoc;
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

class SampleBump3D : public Sample3DView {
    enum {
        DX = 400,
        DY = 300
    };

    SkRRect fRR;
    LightOnSphere fLight = {{200 + DX, 200 + DY}, 800, 12};

    VSphere fSphere;

    sk_sp<SkShader> fBmpShader, fImgShader;
    sk_sp<SkRuntimeEffect> fEffect;

    SkM44 fWorldToClick,
          fClickToWorld;

    SkM44 fRotation,        // part of model
          fClickRotation;  // temp during a click/drag

public:
    SampleBump3D() : fSphere({200 + DX, 200 + DY}, 400) {}

    SkString name() override { return SkString("bump3d"); }

    void onOnceBeforeDraw() override {
        fRR = SkRRect::MakeRectXY({20, 20, 380, 380}, 50, 50);
        auto img = GetResourceAsImage("images/brickwork-texture.jpg");
        fImgShader = img->makeShader(SkMatrix::MakeScale(2, 2));
        img = GetResourceAsImage("images/brickwork_normal-map.jpg");
        fBmpShader = img->makeShader(SkMatrix::MakeScale(2, 2));

        const char code[] = R"(
            in fragmentProcessor color_map;
            in fragmentProcessor normal_map;

            uniform float4x4 localToWorld;
            uniform float4x4 localToWorldAdjInv;
            uniform float3   lightPos;

            float3 convert_normal_sample(half4 c) {
                float3 n = 2 * c.rgb - 1;
                n.y = -n.y;
                return n;
            }

            void main(float x, float y, inout half4 color) {
                float3 norm = convert_normal_sample(sample(normal_map));
                float3 plane_norm = normalize(localToWorld * float4(norm, 0)).xyz;

                float3 plane_pos = (localToWorld * float4(x, y, 0, 1)).xyz;
                float3 light_dir = normalize(lightPos - plane_pos);

                float ambient = 0.2;
                float dp = dot(plane_norm, light_dir);
                float scale = min(ambient + max(dp, 0), 1);

                color = sample(color_map) * half4(float4(scale, scale, scale, 1));
            }
        )";
        auto [effect, error] = SkRuntimeEffect::Make(SkString(code));
        if (!effect) {
            SkDebugf("runtime error %s\n", error.c_str());
        }
        fEffect = effect;
    }

    bool onChar(SkUnichar uni) override {
        switch (uni) {
            case 'Z': fLight.fDistance += 10; return true;
            case 'z': fLight.fDistance -= 10; return true;
        }
        return this->Sample3DView::onChar(uni);
    }

    void drawContent(SkCanvas* canvas, const SkM44& m, SkColor color) {
        SkM44 trans = SkM44::Translate(200, 200, 0);   // center of the rotation

        canvas->experimental_concat44(trans * fRot * m * inv(trans));

        // wonder if the runtimeeffect can do this reject? (in a setup function)
        if (!front(canvas->experimental_getLocalToDevice())) {
            return;
        }

        auto adj_inv = [](const SkM44& m) {
            SkM44 inv;
            SkAssertResult(m.invert(&inv));
            return inv.transpose();
        };

        struct Uniforms {
            SkM44  fLocalToWorld;
            SkM44  fLocalToWorldAdjInv;
            SkV3   fLightPos;
        } uni;
        uni.fLocalToWorld = canvas->experimental_getLocalToWorld();
        uni.fLocalToWorldAdjInv = adj_inv(uni.fLocalToWorld);
        uni.fLightPos = fLight.computeWorldPos(fSphere);

        sk_sp<SkData> data = SkData::MakeWithCopy(&uni, sizeof(uni));
        sk_sp<SkShader> children[] = { fImgShader, fBmpShader };

        SkPaint paint;
        paint.setColor(color);
        paint.setShader(fEffect->makeShader(data, children, 2, nullptr, true));

        canvas->drawRRect(fRR, paint);
    }

    void setClickToWorld(SkCanvas* canvas, const SkM44& clickM) {
        auto l2d = canvas->experimental_getLocalToDevice();
        fWorldToClick = inv(clickM) * l2d;
        fClickToWorld = inv(fWorldToClick);
    }

    void onDrawContent(SkCanvas* canvas) override {
        if (canvas->getGrContext() == nullptr) {
            return;
        }
        SkM44 clickM = canvas->experimental_getLocalToDevice();

        canvas->save();
        canvas->translate(DX, DY);

        this->saveCamera(canvas, {0, 0, 400, 400}, 200);

        this->setClickToWorld(canvas, clickM);

        for (auto f : faces) {
            SkAutoCanvasRestore acr(canvas, true);
            this->drawContent(canvas, fClickRotation * fRotation * f.asM44(200), f.fColor);
        }

        canvas->restore();  // camera
        canvas->restore();  // center the content in the window

        fLight.draw(canvas);
        {
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
        SkVec2 p = fLight.fLoc - SkVec2{x, y};
        if (p.length() <= fLight.fRadius) {
            Click* c = new Click();
            c->fMeta.setS32("type", 0);
            return c;
        }
        if (fSphere.contains({x, y})) {
            Click* c = new Click();
            c->fMeta.setS32("type", 1);
            return c;
        }
        return nullptr;
    }
    bool onClick(Click* click) override {
#if 0
        auto L = fWorldToClick * fLight.fPos;
        SkPoint c = project(fClickToWorld, {click->fCurr.fX, click->fCurr.fY, L.z/L.w, 1});
        fLight.update(c.fX, c.fY);
#endif
        if (click->fMeta.hasS32("type", 0)) {
            fLight.fLoc = fSphere.pinLoc({click->fCurr.fX, click->fCurr.fY});
            return true;
        }
        if (click->fMeta.hasS32("type", 1)) {
            if (click->fState == skui::InputState::kUp) {
                fRotation = fClickRotation * fRotation;
                fClickRotation.setIdentity();
            } else {
                fClickRotation = fSphere.computeRotation({click->fOrig.fX, click->fOrig.fY},
                                                          {click->fCurr.fX, click->fCurr.fY});
            }
            return true;
        }
        return true;
    }
};
DEF_SAMPLE( return new SampleBump3D; )
