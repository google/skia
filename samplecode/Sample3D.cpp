/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix44.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/utils/Sk3D.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"
#include "tools/Resources.h"

static SkMatrix44 inv(const SkMatrix44& m) {
    SkMatrix44 inverse;
    SkAssertResult(m.invert(&inverse));
    return inverse;
}

class Sample3DView : public Sample {
protected:
    float   fNear = 0.05f;
    float   fFar = 4;
    float   fAngle = SK_ScalarPI / 12;

    SkPoint3    fEye { 0, 0, 1.0f/tan(fAngle/2) - 1 };
//    SkPoint3    fCOA { 0, 0, 0 };
    SkPoint3    fUp  { 0, 1, 0 };

    SkMatrix44  fRot;
    SkPoint3    fTrans;

    void rotate(float x, float y, float z) {
        SkMatrix44 r;
        if (x) {
            r.setRotateAboutUnit(1, 0, 0, x);
        } else if (y) {
            r.setRotateAboutUnit(0, 1, 0, y);
        } else {
            r.setRotateAboutUnit(0, 0, 1, z);
        }
        fRot.postConcat(r);
    }

public:
    SkMatrix44 proj() const { return Sk3Perspective(fNear, fFar, fAngle); }
    SkMatrix44 camera() const {
        SkPoint3 coa = { fEye.fX, fEye.fY, -10000 };
        return Sk3LookAt(fEye, coa, fUp);
    }

    SkMatrix44 get44(const SkRect& r, const SkMatrix44& model) const {
        SkScalar w = r.width();
        SkScalar h = r.height();

        SkMatrix44 viewport;
        viewport.setScale(w*0.5f, h*0.5f, 1).postTranslate(r.centerX(), r.centerY(), 0);

        return viewport * this->proj() * this->camera() * fRot * model * inv(viewport);
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

            case 'X': fEye.fX += 0.1f; return true;
            case 'x': fEye.fX -= 0.1f; return true;
            case 'Y': fEye.fY += 0.1f; return true;
            case 'y': fEye.fY -= 0.1f; return true;
            case 'Z': fEye.fZ += 0.1f; return true;
            case 'z': fEye.fZ -= 0.1f; return true;

            case 'n': fNear += 0.1f; SkDebugf("near %g\n", fNear); return true;
            case 'N': fNear -= 0.1f; SkDebugf("near %g\n", fNear); return true;
            case 'f': fFar  += 0.1f; SkDebugf("far  %g\n", fFar); return true;
            case 'F': fFar  -= 0.1f; SkDebugf("far  %g\n", fFar); return true;
            default: break;
        }
        return false;
    }
};

struct SkV3 {
    float x, y, z;

    static SkScalar Dot(const SkV3& a, const SkV3& b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
    static SkV3   Cross(const SkV3& a, const SkV3& b) {
        return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
    }

    SkV3 operator+(const SkV3& v) const { return { x + v.x, y + v.y, z + v.z }; }
    SkV3 operator-(const SkV3& v) const { return { x - v.x, y - v.y, z - v.z }; }

    friend SkV3 operator*(const SkV3& v, SkScalar s) {
        return { v.x*s, v.y*s, v.z*s };
    }
    friend SkV3 operator*(SkScalar s, const SkV3& v) { return v*s; }

    SkScalar operator*(const SkV3& v) const { return   Dot(*this, v); }
    SkV3     operator%(const SkV3& v) const { return Cross(*this, v); }

    SkScalar lengthSquared() const { return Dot(*this, *this); }
    SkScalar length() const { return SkScalarSqrt(Dot(*this, *this)); }
};

typedef SkV3 SkP3;

static SkMatrix44 RX(SkScalar rad) {
    SkScalar c = SkScalarCos(rad), s = SkScalarSin(rad);
    SkMatrix44 m;
    m.set3x3(1, 0, 0,
             0, c, s,
             0,-s, c);
    return m;
}

static SkMatrix44 RY(SkScalar rad) {
    SkScalar c = SkScalarCos(rad), s = SkScalarSin(rad);
    SkMatrix44 m;
    m.set3x3( c, 0,-s,
              0, 1, 0,
              s, 0, c);
    return m;
}

struct Face {
    SkScalar fRx, fRy;

    static SkMatrix44 T(SkScalar x, SkScalar y, SkScalar z) {
        SkMatrix44 m;
        m.setTranslate(x, y, z);
        return m;
    }

    static SkMatrix44 R(SkScalar x, SkScalar y, SkScalar z, SkScalar rad) {
        SkMatrix44 m;
        m.setRotateAboutUnit(x, y, z, rad);
        return m;
    }

    SkMatrix44 asM44(SkScalar scale) const {
        return RY(fRy) * RX(fRx) * T(0, 0, scale);
    }
};

static void transpose(SkMatrix44* m) {
    SkScalar array[16];
    m->asColMajorf(array);
    m->setRowMajor(array);
}

static bool front(const SkMatrix44& m) {
    SkMatrix44 m2;
    m.invert(&m2);
    transpose(&m2);

    auto v = m2 * SkVector4{0, 0, 1, 0};
    return v.fData[2] > 0;
}

const Face faces[] = {
    {             0,             0 }, // front
    {             0,   SK_ScalarPI }, // back

    { SK_ScalarPI/2,             0 }, // top
    {-SK_ScalarPI/2,             0 }, // bottom

    {             0, SK_ScalarPI/2 }, // left
    {             0,-SK_ScalarPI/2 }, // right
};

class SampleCube3D : public Sample3DView {
    SkRRect fRR;
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

    void drawContent(SkCanvas* canvas, const SkMatrix44& m) {
        auto mx = this->get44({0, 0, 400, 400}, m);
        canvas->concat(mx);

        SkPaint paint;
        paint.setAlphaf(front(mx) ? 1 : 0.25f);
        paint.setShader(fShader);
        canvas->drawRRect(fRR, paint);
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->translate(400, 300);

        for (auto f : faces) {
            SkAutoCanvasRestore acr(canvas, true);
            this->drawContent(canvas, f.asM44(1));
        }
    }
};
DEF_SAMPLE( return new SampleCube3D(); )

///////////

static SkMatrix44 ctm(SkCanvas* canvas) {
    SkMatrix44 m;
    SkScalar array[16];
    canvas->getColMajor44(array);
    m.setColMajor(array);
    return m;
}

class SamplePlane3D : public Sample3DView {
    SkRRect fRR;
    sk_sp<SkShader> fShader;

    SkString name() override { return SkString("plane3d"); }

    void onOnceBeforeDraw() override {
        fRR = SkRRect::MakeRectXY({20, 20, 380, 380}, 50, 50);
        fShader = GetResourceAsImage("images/mandrill_128.png")
                        ->makeShader(SkMatrix::MakeScale(3, 3));
    }

    bool onChar(SkUnichar uni) override {
        return this->Sample3DView::onChar(uni);
    }

    void drawContent(SkCanvas* canvas, sk_sp<SkShader> shader) {
        SkPaint paint;
        paint.setColor(0xFFCCCCCC);
        paint.setAlphaf(front(ctm(canvas)) ? 1 : 0.25f);
        paint.setShader(shader);
        canvas->drawRRect(fRR, paint);
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->translate(400, 300);

        this->drawContent(canvas, nullptr);

        auto mx = this->get44({0, 0, 400, 400}, SkMatrix44());
        canvas->concat(mx);

        this->drawContent(canvas, fShader);
    }
};
DEF_SAMPLE( return new SamplePlane3D(); )
