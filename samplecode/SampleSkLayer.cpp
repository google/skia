
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkView.h"
#include "SkLayer.h"

#include "SkMatrix44.h"
static void test_inv(const char label[], const SkMatrix44& mat) {
    SkDebugf("%s\n", label);
    mat.dump();

    SkMatrix44 inv;
    if (mat.invert(&inv)) {
        inv.dump();
    } else {
        SkDebugf("--- invert failed\n");
    }

    SkMatrix44 a, b;
    a.setConcat(mat, inv);
    b.setConcat(inv, mat);
    SkDebugf("concat mat with inverse pre=%d post=%d\n", a.isIdentity(), b.isIdentity());
    if (!a.isIdentity()) {
        a.dump();
    }
    if (!b.isIdentity()) {
        b.dump();
    }
    SkDebugf("\n");
}

static void test_map(SkScalar x0, SkScalar y0, SkScalar z0,
                     const SkMatrix44& mat,
                     SkScalar x1, SkScalar y1, SkScalar z1) {
    SkVector4 src, dst;
    src.set(x0, y0, z0);
    dst = mat * src;
    SkDebugf("map: src: %g %g %g dst: %g %g %g (%g) expected: %g %g %g match: %d\n",
             x0, y0, z0,
             dst.fData[0], dst.fData[1], dst.fData[2], dst.fData[3],
             x1, y1, z1,
             dst.fData[0] == x1 && dst.fData[1] == y1 && dst.fData[2] == z1);
}

static void test_33(const SkMatrix44& mat,
                    SkScalar x0, SkScalar x1, SkScalar x2,
                    SkScalar y0, SkScalar y1, SkScalar y2) {
    SkMatrix dst = mat;
    if (dst[0] != x0 || dst[1] != x1 || dst[2] != x2 ||
        dst[3] != y0 || dst[4] != y1 || dst[5] != y2) {
        SkString str;
        dst.toString(&str);
        SkDebugf("3x3: expected 3x3 [%g %g %g] [%g %g %g] bug got %s\n",
                 x0, x1, x2, y0, y1, y2, str.c_str());
    }
}

static void test44() {
    SkMatrix44 m0, m1, m2;

    test_inv("identity", m0);
    m0.setTranslate(2,3,4);
    test_inv("translate", m0);
    m0.setScale(2,3,4);
    test_inv("scale", m0);
    m0.postTranslate(5, 6, 7);
    test_inv("postTranslate", m0);
    m0.setScale(2,3,4);
    m1.setTranslate(5, 6, 7);
    m0.setConcat(m0, m1);
    test_inv("postTranslate2", m0);
    m0.setScale(2,3,4);
    m0.preTranslate(5, 6, 7);
    test_inv("preTranslate", m0);

    m0.setScale(2, 4, 6);
    m0.postScale(SkDoubleToMScalar(0.5));
    test_inv("scale/postscale to 1,2,3", m0);

    m0.reset();
    test_map(1, 0, 0, m0, 1, 0, 0);
    test_map(0, 1, 0, m0, 0, 1, 0);
    test_map(0, 0, 1, m0, 0, 0, 1);
    m0.setScale(2, 3, 4);
    test_map(1, 0, 0, m0, 2, 0, 0);
    test_map(0, 1, 0, m0, 0, 3, 0);
    test_map(0, 0, 1, m0, 0, 0, 4);
    m0.setTranslate(2, 3, 4);
    test_map(0, 0, 0, m0, 2, 3, 4);
    m0.preScale(5, 6, 7);
    test_map(1, 0, 0, m0, 7, 3, 4);
    test_map(0, 1, 0, m0, 2, 9, 4);
    test_map(0, 0, 1, m0, 2, 3, 11);

    SkMScalar deg = 45;
    m0.setRotateDegreesAbout(0, 0, 1, deg);
    test_map(1, 0, 0, m0, 0.707106769, -0.707106769, 0);

    m0.reset();
    test_33(m0, 1, 0, 0, 0, 1, 0);
    m0.setTranslate(3, 4, 5);
    test_33(m0, 1, 0, 3, 0, 1, 4);
}

///////////////////////////////////////////////////////////////////////////////

static void dump_layers(const SkLayer* layer, int tab = 0) {
    SkMatrix matrix;
    SkString matrixStr;

    layer->getLocalTransform(&matrix);
    matrix.toString(&matrixStr);

    for (int j = 0; j < tab; j++) {
        SkDebugf(" ");
    }
    SkDebugf("layer=%p parent=%p size=[%g %g] transform=%s\n",
             layer, layer->getParent(), layer->getWidth(), layer->getHeight(),
             matrixStr.c_str());
    for (int i = 0; i < layer->countChildren(); i++) {
        dump_layers(layer->getChild(i), tab + 4);
    }
}

class TestLayer : public SkLayer {
public:
    TestLayer(SkColor c) : fColor(c) {}

protected:
    virtual void onDraw(SkCanvas* canvas, SkScalar opacity) {
        SkRect r;
        r.set(0, 0, this->getWidth(), this->getHeight());

        SkPaint paint;
        paint.setColor(fColor);
        paint.setAlpha(SkScalarRoundToInt(opacity * 255));

        canvas->drawRect(r, paint);
    }

private:
    SkColor fColor;
};

class SkLayerView : public SkView {
private:
    SkLayer* fRootLayer;
    SkLayer* fLastChild;
public:
    SkLayerView() {
        test44();
        static const int W = 600;
        static const int H = 440;
        static const struct {
            int fWidth;
            int fHeight;
            SkColor fColor;
            int fPosX;
            int fPosY;
        } gData[] = {
            { 120, 80, SK_ColorRED, 0, 0 },
            { 120, 80, SK_ColorGREEN, W - 120, 0 },
            { 120, 80, SK_ColorBLUE, 0, H - 80 },
            { 120, 80, SK_ColorMAGENTA, W - 120, H - 80 },
        };

        fRootLayer = new TestLayer(0xFFDDDDDD);
        fRootLayer->setSize(W, H);
        for (size_t i = 0; i < SK_ARRAY_COUNT(gData); i++) {
            SkLayer* child = new TestLayer(gData[i].fColor);
            child->setSize(gData[i].fWidth, gData[i].fHeight);
            child->setPosition(gData[i].fPosX, gData[i].fPosY);
            fRootLayer->addChild(child)->unref();
        }

        SkLayer* child = new TestLayer(0xFFDD8844);
        child->setSize(120, 80);
        child->setPosition(fRootLayer->getWidth()/2 - child->getWidth()/2,
                           fRootLayer->getHeight()/2 - child->getHeight()/2);
        child->setAnchorPoint(SK_ScalarHalf, SK_ScalarHalf);
        {
            SkMatrix m;
            m.setRotate(SkIntToScalar(30));
            child->setMatrix(m);
        }
        fLastChild = child;
        fRootLayer->addChild(child)->unref();

        if (false) {
            SkMatrix matrix;
            matrix.setScale(0.5, 0.5);
            fRootLayer->setMatrix(matrix);
        }

//        dump_layers(fRootLayer);
    }

    virtual ~SkLayerView() {
        SkSafeUnref(fRootLayer);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "SkLayer");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDraw(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);

        canvas->translate(20, 20);
        fRootLayer->draw(canvas);

        // visual test of getLocalTransform
        if (true) {
            SkMatrix matrix;
            fLastChild->localToGlobal(&matrix);
            SkPaint paint;
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(5);
            paint.setColor(0x88FF0000);
            canvas->concat(matrix);
            canvas->drawRect(SkRect::MakeSize(fLastChild->getSize()), paint);
        }
    }

private:
    typedef SkView INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new SkLayerView; }
static SkViewRegister reg(MyFactory);
