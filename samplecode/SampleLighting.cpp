
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "Resources.h"

#include "SkCanvas.h"
#include "SkImageDecoder.h"
#include "SkLightingShader.h"

////////////////////////////////////////////////////////////////////////////

class LightingView : public SampleView {
public:
    SkAutoTUnref<SkShader> fShader;
    SkBitmap               fDiffuseBitmap;
    SkBitmap               fNormalBitmap;
    SkScalar               fLightAngle;
    int                    fColorFactor;
    SkColor                fAmbientColor;

    LightingView() {
        SkString diffusePath = GetResourcePath("brickwork-texture.jpg");
        SkImageDecoder::DecodeFile(diffusePath.c_str(), &fDiffuseBitmap);
        SkString normalPath = GetResourcePath("brickwork_normal-map.jpg");
        SkImageDecoder::DecodeFile(normalPath.c_str(), &fNormalBitmap);

        fLightAngle = 0.0f;
        fColorFactor = 0;

        SkLightingShader::Light light;
        light.fColor = SkColorSetRGB(0xff, 0xff, 0xff);
        light.fDirection.fX = SkScalarSin(fLightAngle)*SkScalarSin(SK_ScalarPI*0.25f);
        light.fDirection.fY = SkScalarCos(fLightAngle)*SkScalarSin(SK_ScalarPI*0.25f);
        light.fDirection.fZ = SkScalarCos(SK_ScalarPI*0.25f);

        fAmbientColor = SkColorSetRGB(0x1f, 0x1f, 0x1f);

        fShader.reset(SkLightingShader::Create(fDiffuseBitmap, fNormalBitmap,
                                               light, fAmbientColor, nullptr));
    }

    virtual ~LightingView() {}

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
        fColorFactor++;

        SkLightingShader::Light light;
        light.fColor = SkColorSetRGB(0xff, 0xff, (fColorFactor >> 1) & 0xff);
        light.fDirection.fX = SkScalarSin(fLightAngle)*SkScalarSin(SK_ScalarPI*0.25f);
        light.fDirection.fY = SkScalarCos(fLightAngle)*SkScalarSin(SK_ScalarPI*0.25f);
        light.fDirection.fZ = SkScalarCos(SK_ScalarPI*0.25f);

        fShader.reset(SkLightingShader::Create(fDiffuseBitmap, fNormalBitmap,
                                               light, fAmbientColor, nullptr));

        SkPaint paint;
        paint.setShader(fShader);
        paint.setColor(SK_ColorBLACK);

        SkRect r = SkRect::MakeWH((SkScalar)fDiffuseBitmap.width(), 
                                  (SkScalar)fDiffuseBitmap.height());
        canvas->drawRect(r, paint);

        // so we're constantly updating
        this->inval(NULL);
    }

    SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        this->inval(NULL);
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new LightingView; }
static SkViewRegister reg(MyFactory);
