/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#ifndef SK_BUILD_FOR_GOOGLE3

#include "experimental/xform/SkShape.h"
#include "experimental/xform/SkXform.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "tools/timer/TimeUtils.h"

class XformGM : public skiagm::GM {
    sk_sp<MatrixXF> fRoot, fRA, fRB, fA, fB;
    sk_sp<Shape> fShape;

public:
    XformGM() {
        fRoot = MatrixXF::Make();

        fRA = MatrixXF::Make(fRoot);
        fRB = MatrixXF::Make(fRoot);

        fA = MatrixXF::Make(fRA);
        fB = MatrixXF::Make(fRB);

        fRA->setRotate(30);
        fA->setTranslate(100, 0);

        fRB->setTranslate(100, 0);
        fB->setRotate(30);

        sk_sp<GroupShape> g = GroupShape::Make();
        g->append(GeoShape::Make(fA,  {0, 0, 100, 60}, SK_ColorRED));
        g->append(GeoShape::Make(fB,  {0, 0, 100, 60}, SK_ColorGREEN));
        g->append(GeoShape::Make(fRA, {0, 0, 100, 60}, SK_ColorBLUE));
        g->append(GeoShape::Make(fRB, {0, 0, 100, 60}, SK_ColorGRAY));
        g->append(GeoShape::Make(fRoot, {0, 0, 100, 60}, 0xFFCC8844));

        sk_sp<MatrixXF> sub = MatrixXF::Make();
        SkMatrix m;
        m.setScale(0.5, 0.5);
        m.postTranslate(50, 50);
        sub->setLocalMatrix(m);

        sk_sp<GroupShape> parent = GroupShape::Make();
        parent->append(g);
        parent->append(GroupShape::Make(sub, g));
        fShape = parent;
    }

protected:
    SkString onShortName() override { return SkString("exp_xform"); }

    SkISize onISize() override { return SkISize::Make(520, 520); }

    void onDraw(SkCanvas* canvas) override {
        auto ctx = XContext::Make(canvas);

        if (0) {
            canvas->translate(2, 2);

            SkRect rect{0, 0, 100, 60};
            SkPaint paint; paint.setStyle(SkPaint::kStroke_Style);
            canvas->drawRect(rect, paint);
            canvas->save(); canvas->translate(10, 10);
            paint.setColor(SK_ColorRED); canvas->drawRect(rect, paint); canvas->restore();
            canvas->save(); canvas->scale(2, 2);
            paint.setColor(SK_ColorBLUE); canvas->drawRect(rect, paint); canvas->restore();
            canvas->save(); canvas->scale(2, 2); canvas->translate(10, 10);
            paint.setColor(SK_ColorBLACK); canvas->drawRect(rect, paint); canvas->restore();
            canvas->save(); canvas->translate(10, 10); canvas->scale(2, 2);
            paint.setColor(SK_ColorBLACK); canvas->drawRect(rect, paint); canvas->restore();

            auto x0 = MatrixXF::Make();
            auto x1 = MatrixXF::Make(x0);
            auto x2 = MatrixXF::Make(x1);
            x1->setScale(2, 2);
            x2->setTranslate(10, 10);

            auto sh = GeoShape::Make(x2,  {0, 0, 100, 60}, 0x8800FF00);
            sh->draw(ctx.get());
            return;
        }
        fShape->draw(ctx.get());
    }

    bool onAnimate(double nanos) override {
        float scale = 3 + sinf(TimeUtils::Scaled(1e-9 * nanos, 1, 0)) * 2;
        fRoot->setScale(scale, scale);
        fRA->setRotate(TimeUtils::Scaled(1e-9 * nanos, 40, 0));
        fB->setRotate(TimeUtils::Scaled(1e-9 * nanos, 40*sqrtf(2), 0));
        return true;
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new XformGM; )

#endif
