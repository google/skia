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
                                     2);
    return bitmap;
}

// Create a hemispherical normal map
static SkBitmap make_hemi_normalmap(int texSize) {
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
            locZ = sqrtf(1.0f - locZ);
            unsigned char r = static_cast<unsigned char>((0.5f * locX + 0.5f) * 255);
            unsigned char g = static_cast<unsigned char>((-0.5f * locY + 0.5f) * 255);
            unsigned char b = static_cast<unsigned char>((0.5f * locZ + 0.5f) * 255);
            *hemi.getAddr32(x, y) = SkPackARGB32(0xFF, r, g, b);
        }
    }

    return hemi;
}

// Create a truncated pyramid normal map
static SkBitmap make_frustum_normalmap(int texSize) {
    SkBitmap frustum;
    frustum.allocN32Pixels(texSize, texSize);

    SkIRect inner = SkIRect::MakeWH(texSize, texSize);
    inner.inset(texSize/4, texSize/4);

    SkPoint3 norm;
    const SkPoint3 left =  SkPoint3::Make(-SK_ScalarRoot2Over2, 0.0f, SK_ScalarRoot2Over2);
    const SkPoint3 up =    SkPoint3::Make(0.0f, -SK_ScalarRoot2Over2, SK_ScalarRoot2Over2);
    const SkPoint3 right = SkPoint3::Make(SK_ScalarRoot2Over2,  0.0f, SK_ScalarRoot2Over2);
    const SkPoint3 down =  SkPoint3::Make(0.0f,  SK_ScalarRoot2Over2, SK_ScalarRoot2Over2);

    for (int y = 0; y < texSize; ++y) {
        for (int x = 0; x < texSize; ++x) {
            if (inner.contains(x, y)) {
                norm.set(0.0f, 0.0f, 1.0f);
            } else {
                SkScalar locX = x + 0.5f - texSize/2.0f;
                SkScalar locY = y + 0.5f - texSize/2.0f;

                if (locX >= 0.0f) {
                    if (locY > 0.0f) {
                        norm = locX >= locY ? right : down;   // LR corner
                    } else {
                        norm = locX > -locY ? right : up;     // UR corner
                    }    
                } else {
                    if (locY > 0.0f) {
                        norm = -locX > locY ? left : down;    // LL corner
                    } else {
                        norm = locX > locY ? up : left;       // UL corner
                    }    
                }
            }

            SkASSERT(SkScalarNearlyEqual(norm.length(), 1.0f));
            unsigned char r = static_cast<unsigned char>((0.5f *  norm.fX + 0.5f) * 255);
            unsigned char g = static_cast<unsigned char>((-0.5f * norm.fY + 0.5f) * 255);
            unsigned char b = static_cast<unsigned char>((0.5f *  norm.fZ + 0.5f) * 255);
            *frustum.getAddr32(x, y) = SkPackARGB32(0xFF, r, g, b);
        }
    }

    return frustum;
}

namespace skiagm {

// This GM exercises lighting shaders.
class LightingShaderGM : public GM {
public:
    LightingShaderGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));

        fLight.fColor = SkColor3f::Make(1.0f, 1.0f, 1.0f);
        fLight.fDirection.fX = 0.0f;
        fLight.fDirection.fY = 0.0f;
        fLight.fDirection.fZ = 1.0f;

        fAmbient = SkColor3f::Make(0.1f, 0.1f, 0.1f);
    }

protected:

    SkString onShortName() override {
        return SkString("lightingshader");
    }

    SkISize onISize() override {
        return SkISize::Make(kGMSize, kGMSize);
    }

    void onOnceBeforeDraw() override {
        fDiffuse = make_checkerboard(kTexSize);
        fHemiNormalMap = make_hemi_normalmap(kTexSize);
        fFrustumNormalMap = make_frustum_normalmap(kTexSize);
    }

    void drawRect(SkCanvas* canvas, const SkRect& r, bool hemi) {

        SkRect bitmapBounds = SkRect::MakeIWH(fDiffuse.width(), fDiffuse.height());

        SkMatrix matrix;
        matrix.setRectToRect(bitmapBounds, r, SkMatrix::kFill_ScaleToFit);
    
        SkAutoTUnref<SkShader> fShader(SkLightingShader::Create(
                                                        fDiffuse,
                                                        hemi ? fHemiNormalMap : fFrustumNormalMap, 
                                                        fLight, fAmbient,
                                                        &matrix));

        SkPaint paint;
        paint.setShader(fShader);

        canvas->drawRect(r, paint);
    }

    void onDraw(SkCanvas* canvas) override {
        SkRect r = SkRect::MakeWH(SkIntToScalar(kTexSize), SkIntToScalar(kTexSize));
        this->drawRect(canvas, r, true);

        r.offset(kGMSize - kTexSize, 0);
        this->drawRect(canvas, r, false);

        r.offset(0, kGMSize - kTexSize);
        this->drawRect(canvas, r, true);

        r.offset(kTexSize - kGMSize, 0);
        this->drawRect(canvas, r, false);
    }

private:
    static const int kTexSize = 128;
    static const int kGMSize  = 512;

    SkBitmap                fDiffuse;
    SkBitmap                fHemiNormalMap;
    SkBitmap                fFrustumNormalMap;

    SkLightingShader::Light fLight;
    SkColor3f               fAmbient;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(LightingShaderGM); )

}
