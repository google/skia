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

        SkLights::Builder builder;

        builder.add(SkLights::Light(SkColor3f::Make(1.0f, 1.0f, 1.0f),
                                    SkVector3::Make(1.0f, 0.0f, 0.0f)));
        builder.add(SkLights::Light(SkColor3f::Make(0.2f, 0.2f, 0.2f)));

        fLights = builder.finish();
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
        fDiffuse = sk_tool_utils::create_checkerboard_bitmap(
                                                        kTexSize, kTexSize,
                                                        sk_tool_utils::color_to_565(0x0),
                                                        sk_tool_utils::color_to_565(0xFF804020),
                                                        8);

        fNormalMaps[kHemi_NormalMap]    = make_hemi_normalmap(kTexSize);
        fNormalMaps[kFrustum_NormalMap] = make_frustum_normalmap(kTexSize);
        fNormalMaps[kTetra_NormalMap]   = make_tetra_normalmap(kTexSize);
    }

    void drawRect(SkCanvas* canvas, const SkRect& r, NormalMap mapType) {

        SkRect bitmapBounds = SkRect::MakeIWH(fDiffuse.width(), fDiffuse.height());

        SkMatrix matrix;
        matrix.setRectToRect(bitmapBounds, r, SkMatrix::kFill_ScaleToFit);

        const SkMatrix& ctm = canvas->getTotalMatrix();

        // TODO: correctly pull out the pure rotation
        SkVector invNormRotation = { ctm[SkMatrix::kMScaleX], ctm[SkMatrix::kMSkewY] };

        SkPaint paint;
        paint.setShader(SkLightingShader::Make(fDiffuse, fNormalMaps[mapType], fLights,
                                               invNormRotation, &matrix, &matrix));

        canvas->drawRect(r, paint);
    }

    void onDraw(SkCanvas* canvas) override {
        SkMatrix m;
        SkRect r;

        {
            r = SkRect::MakeWH(SkIntToScalar(kTexSize), SkIntToScalar(kTexSize));
            this->drawRect(canvas, r, kHemi_NormalMap);

            canvas->save();
            m.setRotate(45.0f, r.centerX(), r.centerY());
            m.postTranslate(kGMSize/2.0f - kTexSize/2.0f, 0.0f);
            canvas->setMatrix(m);
            this->drawRect(canvas, r, kHemi_NormalMap);
            canvas->restore();
        }

        {
            r.offset(kGMSize - kTexSize, 0);
            this->drawRect(canvas, r, kFrustum_NormalMap);

            canvas->save();
            m.setRotate(45.0f, r.centerX(), r.centerY());
            m.postTranslate(0.0f, kGMSize/2.0f - kTexSize/2.0f);
            canvas->setMatrix(m);
            this->drawRect(canvas, r, kFrustum_NormalMap);
            canvas->restore();
        }

        {
            r.offset(0, kGMSize - kTexSize);
            this->drawRect(canvas, r, kTetra_NormalMap);

            canvas->save();
            m.setRotate(45.0f, r.centerX(), r.centerY());
            m.postTranslate(-kGMSize/2.0f + kTexSize/2.0f, 0.0f);
            canvas->setMatrix(m);
            this->drawRect(canvas, r, kTetra_NormalMap);
            canvas->restore();
        }

        {
            r.offset(kTexSize - kGMSize, 0);
            this->drawRect(canvas, r, kHemi_NormalMap);

            canvas->save();
            m.setRotate(45.0f, r.centerX(), r.centerY());
            m.postTranslate(0.0f, -kGMSize/2.0f + kTexSize/2.0f);
            canvas->setMatrix(m);
            this->drawRect(canvas, r, kHemi_NormalMap);
            canvas->restore();
        }
    }

private:
    static const int kTexSize = 128;
    static const int kGMSize  = 512;

    SkBitmap        fDiffuse;
    SkBitmap        fNormalMaps[kNormalMapCount];

    sk_sp<SkLights> fLights;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new LightingShaderGM;)
}
