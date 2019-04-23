/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkShader.h"
#include "src/core/SkNormalSource.h"
#include "src/shaders/SkLightingShader.h"
#include "tools/ToolUtils.h"

// Create a hemispherical normal map
static SkBitmap make_hemi_normalmap(int texSize) {
    SkBitmap hemi;
    hemi.allocN32Pixels(texSize, texSize);

    ToolUtils::create_hemi_normal_map(&hemi, SkIRect::MakeWH(texSize, texSize));
    return hemi;
}

// Create a truncated pyramid normal map
static SkBitmap make_frustum_normalmap(int texSize) {
    SkBitmap frustum;
    frustum.allocN32Pixels(texSize, texSize);

    ToolUtils::create_frustum_normal_map(&frustum, SkIRect::MakeWH(texSize, texSize));
    return frustum;
}

// Create a tetrahedral normal map
static SkBitmap make_tetra_normalmap(int texSize) {
    SkBitmap tetra;
    tetra.allocN32Pixels(texSize, texSize);

    ToolUtils::create_tetra_normal_map(&tetra, SkIRect::MakeWH(texSize, texSize));
    return tetra;
}

namespace skiagm {

// This GM exercises lighting shaders by drawing rotated and non-rotated normal mapped rects with
// a directional light off to the viewers right.
class LightingShaderGM : public GM {
public:
    LightingShaderGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    enum NormalMap {
        kHemi_NormalMap,
        kFrustum_NormalMap,
        kTetra_NormalMap,

        kLast_NormalMap = kTetra_NormalMap
    };

    static constexpr int kNormalMapCount = kLast_NormalMap+1;

    SkString onShortName() override { return SkString("lightingshader"); }

    SkISize onISize() override { return SkISize::Make(kGMSize, kGMSize); }

    void onOnceBeforeDraw() override {
        {
            SkLights::Builder builder;

            // The direction vector is towards the light w/ +Z coming out of the screen
            builder.add(SkLights::Light::MakeDirectional(SkColor3f::Make(1.0f, 1.0f, 1.0f),
                                                         SkVector3::Make(SK_ScalarRoot2Over2,
                                                                         0.0f,
                                                                         SK_ScalarRoot2Over2)));
            builder.setAmbientLightColor(SkColor3f::Make(0.2f, 0.2f, 0.2f));

            fLights = builder.finish();
        }

        fDiffuse = ToolUtils::create_checkerboard_bitmap(
                kTexSize, kTexSize, 0x00000000, ToolUtils::color_to_565(0xFF804020), 8);

        fNormalMaps[kHemi_NormalMap]    = make_hemi_normalmap(kTexSize);
        fNormalMaps[kFrustum_NormalMap] = make_frustum_normalmap(kTexSize);
        fNormalMaps[kTetra_NormalMap]   = make_tetra_normalmap(kTexSize);
    }

    void drawRect(SkCanvas* canvas, const SkRect& r, NormalMap mapType) {

        SkRect bitmapBounds = SkRect::MakeIWH(fDiffuse.width(), fDiffuse.height());

        SkMatrix matrix;
        matrix.setRectToRect(bitmapBounds, r, SkMatrix::kFill_ScaleToFit);

        const SkMatrix& ctm = canvas->getTotalMatrix();

        SkPaint paint;
        sk_sp<SkShader> diffuseShader = fDiffuse.makeShader(&matrix);
        sk_sp<SkShader> normalMap = fNormalMaps[mapType].makeShader(&matrix);
        sk_sp<SkNormalSource> normalSource = SkNormalSource::MakeFromNormalMap(std::move(normalMap),
                                                                               ctm);
        paint.setShader(SkLightingShader::Make(std::move(diffuseShader), std::move(normalSource),
                                               fLights));

        canvas->drawRect(r, paint);
    }

    // Draw an axis-aligned and rotated version of the normal mapped rect
    void drawPair(SkCanvas* canvas, const SkRect& r, NormalMap mapType, const SkVector& v) {
        SkMatrix m;
        m.setRotate(45.0f, r.centerX(), r.centerY());
        m.postTranslate(kScale * v.fX, kScale * v.fY);

        this->drawRect(canvas, r, mapType);

        canvas->save();
            canvas->setMatrix(m);
            this->drawRect(canvas, r, mapType);
        canvas->restore();
    }

    void onDraw(SkCanvas* canvas) override {
        SkRect r;

        r = SkRect::MakeWH(SkIntToScalar(kTexSize), SkIntToScalar(kTexSize));
        this->drawPair(canvas, r, kHemi_NormalMap, SkVector::Make(1.0f, 0.0f));

        r.offset(kGMSize - kTexSize, 0);
        this->drawPair(canvas, r, kFrustum_NormalMap, SkVector::Make(0.0f, 1.0f));

        r.offset(0, kGMSize - kTexSize);
        this->drawPair(canvas, r, kTetra_NormalMap, SkVector::Make(-1.0, 0.0f));

        r.offset(kTexSize - kGMSize, 0);
        this->drawPair(canvas, r, kHemi_NormalMap, SkVector::Make(0.0f, -1));
    }

private:
    static constexpr int kTexSize = 128;
    static constexpr int kGMSize  = 512;
    static constexpr SkScalar kScale = kGMSize/2.0f - kTexSize/2.0f;

    SkBitmap        fDiffuse;
    SkBitmap        fNormalMaps[kNormalMapCount];

    sk_sp<SkLights> fLights;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new LightingShaderGM;)
}
