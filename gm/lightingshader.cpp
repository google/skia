/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkLightingShader.h"
#include "SkPoint3.h"
#include "SkShader.h"

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

    sk_tool_utils::create_hemi_normal_map(&hemi, SkIRect::MakeWH(texSize, texSize));
    return hemi;
}

// Create a truncated pyramid normal map
static SkBitmap make_frustum_normalmap(int texSize) {
    SkBitmap frustum;
    frustum.allocN32Pixels(texSize, texSize);

    sk_tool_utils::create_frustum_normal_map(&frustum, SkIRect::MakeWH(texSize, texSize));
    return frustum;
}

// Create a tetrahedral normal map
static SkBitmap make_tetra_normalmap(int texSize) {
    SkBitmap tetra;
    tetra.allocN32Pixels(texSize, texSize);

    sk_tool_utils::create_tetra_normal_map(&tetra, SkIRect::MakeWH(texSize, texSize));
    return tetra;
}

namespace skiagm {

// This GM exercises lighting shaders.
class LightingShaderGM : public GM {
public:
    LightingShaderGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));

        fLight.fColor = SkColor3f::Make(1.0f, 1.0f, 1.0f);
        fLight.fDirection = SkVector3::Make(0.0f, 0.0f, 1.0f);

        fAmbient = SkColor3f::Make(0.1f, 0.1f, 0.1f);
    }

protected:
    enum NormalMap {
        kHemi_NormalMap,
        kFrustum_NormalMap,
        kTetra_NormalMap,

        kLast_NormalMap = kTetra_NormalMap
    };

    static const int kNormalMapCount = kLast_NormalMap+1;

    SkString onShortName() override {
        return SkString("lightingshader");
    }

    SkISize onISize() override {
        return SkISize::Make(kGMSize, kGMSize);
    }

    void onOnceBeforeDraw() override {
        fDiffuse = make_checkerboard(kTexSize);

        fNormalMaps[kHemi_NormalMap]    = make_hemi_normalmap(kTexSize);
        fNormalMaps[kFrustum_NormalMap] = make_frustum_normalmap(kTexSize);
        fNormalMaps[kTetra_NormalMap]   = make_tetra_normalmap(kTexSize);
    }

    void drawRect(SkCanvas* canvas, const SkRect& r, NormalMap mapType) {

        SkRect bitmapBounds = SkRect::MakeIWH(fDiffuse.width(), fDiffuse.height());

        SkMatrix matrix;
        matrix.setRectToRect(bitmapBounds, r, SkMatrix::kFill_ScaleToFit);
    
        SkAutoTUnref<SkShader> fShader(SkLightingShader::Create(
                                                        fDiffuse,
                                                        fNormalMaps[mapType],
                                                        fLight, fAmbient,
                                                        &matrix));

        SkPaint paint;
        paint.setShader(fShader);

        canvas->drawRect(r, paint);
    }

    void onDraw(SkCanvas* canvas) override {
        SkRect r = SkRect::MakeWH(SkIntToScalar(kTexSize), SkIntToScalar(kTexSize));
        this->drawRect(canvas, r, kHemi_NormalMap);

        r.offset(kGMSize - kTexSize, 0);
        this->drawRect(canvas, r, kFrustum_NormalMap);

        r.offset(0, kGMSize - kTexSize);
        this->drawRect(canvas, r, kTetra_NormalMap);

        r.offset(kTexSize - kGMSize, 0);
        this->drawRect(canvas, r, kHemi_NormalMap);
    }

private:
    static const int kTexSize = 128;
    static const int kGMSize  = 512;

    SkBitmap                fDiffuse;
    SkBitmap                fNormalMaps[kNormalMapCount];

    SkLightingShader::Light fLight;
    SkColor3f               fAmbient;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(LightingShaderGM); )

}
