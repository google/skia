/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkColorPriv.h"
#include "SkLightingShader.h"

static SkBitmap make_checkerboard(int texSize) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(texSize, texSize);

    SkCanvas canvas(bitmap);
    sk_tool_utils::draw_checkerboard(&canvas,
                                     sk_tool_utils::color_to_565(0x0),
                                     sk_tool_utils::color_to_565(0xFF804020),
                                     16);
    return bitmap;
}

// Create a hemispherical normal map
static SkBitmap make_normalmap(int texSize) {
    SkBitmap hemi;
    hemi.allocN32Pixels(texSize, texSize);

    for (int y = 0; y < texSize; ++y) {
        for (int x = 0; x < texSize; ++x) {
            SkScalar locX = (x + 0.5f - texSize/2.0f) / (texSize/2.0f);
            SkScalar locY = (y + 0.5f - texSize/2.0f) / (texSize/2.0f);
            
            SkScalar locZ = locX * locX + locY * locY;
            if (locZ >= 1.0f) {
                locX = 0.0f;
                locY = 0.0f;
                locZ = 0.0f;
            }
            locZ = sqrt(1.0f - locZ);
            unsigned char r = static_cast<unsigned char>((0.5f * locX + 0.5f) * 255);
            unsigned char g = static_cast<unsigned char>((-0.5f * locY + 0.5f) * 255);
            unsigned char b = static_cast<unsigned char>((0.5f * locZ + 0.5f) * 255);
            *hemi.getAddr32(x, y) = SkPackARGB32(0xFF, r, g, b);
        }
    }

    return hemi;
}


namespace skiagm {

// This GM exercises lighting shaders.
class LightingShaderGM : public GM {
public:
    LightingShaderGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

protected:

    SkString onShortName() override {
        return SkString("lightingshader");
    }

    SkISize onISize() override {
        return SkISize::Make(kTexSize, kTexSize);
    }

    void onOnceBeforeDraw() override {
        fDiffuse = make_checkerboard(kTexSize);
        fNormalMap = make_normalmap(kTexSize);
    }

    void onDraw(SkCanvas* canvas) override {

        SkColor ambient = SkColorSetRGB(0x1f, 0x1f, 0x1f);

        SkLightingShader::Light light;
        light.fColor = SkColorSetRGB(0xff, 0xff, 0xff);
        light.fDirection.fX = 0.0f;
        light.fDirection.fY = 0.0f;
        light.fDirection.fZ = 1.0f;

        SkAutoTUnref<SkShader> fShader(SkLightingShader::Create(fDiffuse, fNormalMap, 
                                                                light, ambient));

        SkPaint paint;
        paint.setShader(fShader);

        SkRect r = SkRect::MakeWH(SkIntToScalar(kTexSize), SkIntToScalar(kTexSize));

        canvas->drawRect(r, paint);
    }

private:
    static const int kTexSize = 128;

    SkBitmap fDiffuse;
    SkBitmap fNormalMap;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(LightingShaderGM); )

}
