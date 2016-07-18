/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "DecodeFile.h"
#include "SampleCode.h"
#include "Resources.h"

#include "SkCanvas.h"
#include "SkLightingShader.h"
#include "SkPoint3.h"

static sk_sp<SkLights> create_lights(SkScalar angle, SkScalar blue) {

    const SkVector3 dir = SkVector3::Make(SkScalarSin(angle)*SkScalarSin(SK_ScalarPI*0.25f),
                                          SkScalarCos(angle)*SkScalarSin(SK_ScalarPI*0.25f),
                                          SkScalarCos(SK_ScalarPI*0.25f));

    SkLights::Builder builder;

    builder.add(SkLights::Light(SkColor3f::Make(1.0f, 1.0f, blue), dir));
    builder.add(SkLights::Light(SkColor3f::Make(0.1f, 0.1f, 0.1f)));

    return builder.finish();
}

////////////////////////////////////////////////////////////////////////////

class LightingView : public SampleView {
public:
    SkBitmap        fDiffuseBitmap;
    SkBitmap        fNormalBitmap;
    SkScalar        fLightAngle;
    SkScalar        fColorFactor;

    LightingView() {
        SkString diffusePath = GetResourcePath("brickwork-texture.jpg");
        decode_file(diffusePath.c_str(), &fDiffuseBitmap);
        SkString normalPath = GetResourcePath("brickwork_normal-map.jpg");
        decode_file(normalPath.c_str(), &fNormalBitmap);

        fLightAngle = 0.0f;
        fColorFactor = 0.0f;
    }

protected:
    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Lighting");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        fLightAngle += 0.015f;
        fColorFactor += 0.01f;
        if (fColorFactor > 1.0f) {
            fColorFactor = 0.0f;
        }

        sk_sp<SkLights> lights(create_lights(fLightAngle, fColorFactor));
        SkPaint paint;
        paint.setShader(SkLightingShader::Make(fDiffuseBitmap, fNormalBitmap,
                                               std::move(lights), SkVector::Make(1.0f, 0.0f),
                                               nullptr, nullptr));
        paint.setColor(SK_ColorBLACK);

        SkRect r = SkRect::MakeWH((SkScalar)fDiffuseBitmap.width(),
                                  (SkScalar)fDiffuseBitmap.height());
        canvas->drawRect(r, paint);

        // so we're constantly updating
        this->inval(nullptr);
    }

    SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        this->inval(nullptr);
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new LightingView; }
static SkViewRegister reg(MyFactory);
