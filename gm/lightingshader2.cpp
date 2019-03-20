/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLightingShader.h"
#include "SkNormalSource.h"
#include "SkPoint3.h"
#include "SkShader.h"
#include "SkTypeface.h"
#include "ToolUtils.h"
#include "gm.h"

// Create a truncated pyramid normal map
static SkBitmap make_frustum_normalmap(int texSize) {
    SkBitmap frustum;
    frustum.allocN32Pixels(texSize, texSize);

    ToolUtils::create_frustum_normal_map(&frustum, SkIRect::MakeWH(texSize, texSize));
    return frustum;
}

namespace skiagm {

// This GM exercises lighting shaders. Specifically, nullptr arguments, scaling when using
// normal maps, paint transparency, zero directional lights, multiple directional lights.
class LightingShader2GM : public GM {
public:
    LightingShader2GM() : fRect(SkRect::MakeIWH(kTexSize, kTexSize)) {
        this->setBGColor(ToolUtils::color_to_565(0xFF0000CC));
    }

protected:
    SkString onShortName() override {
        return SkString("lightingshader2");
    }

    SkISize onISize() override {
        return SkISize::Make(600, 740);
    }

    void onOnceBeforeDraw() override {
        // The light direction is towards the light with +Z coming out of the screen
        const SkVector3 kLightFromUpperRight = SkVector3::Make(0.788f, 0.394f, 0.473f);
        const SkVector3 kLightFromUpperLeft = SkVector3::Make(-0.788f, 0.394f, 0.473f);

        // Standard set of lights
        {
            SkLights::Builder builder;
            builder.add(SkLights::Light::MakeDirectional(SkColor3f::Make(1.0f, 1.0f, 1.0f),
                                                         kLightFromUpperRight));
            builder.setAmbientLightColor(SkColor3f::Make(0.2f, 0.2f, 0.2f));
            fLights = builder.finish();
        }

        // No directional lights
        {
            SkLights::Builder builder;
            builder.setAmbientLightColor(SkColor3f::Make(0.2f, 0.2f, 0.2f));
            fLightsNoDir = builder.finish();
        }

        // Two directional lights
        {
            SkLights::Builder builder;
            builder.add(SkLights::Light::MakeDirectional(SkColor3f::Make(1.0f, 0.0f, 0.0f),
                                                         kLightFromUpperRight));
            builder.add(SkLights::Light::MakeDirectional(SkColor3f::Make(0.0f, 1.0f, 0.0f),
                                                         kLightFromUpperLeft));
            builder.setAmbientLightColor(SkColor3f::Make(0.2f, 0.2f, 0.2f));
            fLightsTwoDir = builder.finish();
        }

        SkMatrix matrix;
        SkRect bitmapBounds = SkRect::MakeIWH(kTexSize, kTexSize);
        matrix.setRectToRect(bitmapBounds, fRect, SkMatrix::kFill_ScaleToFit);

        SkBitmap opaqueDiffuseMap = ToolUtils::create_checkerboard_bitmap(
                kTexSize, kTexSize, SK_ColorBLACK, 0xFF808080, 8);
        fOpaqueDiffuse = SkShader::MakeBitmapShader(opaqueDiffuseMap, SkShader::kClamp_TileMode,
                                                    SkShader::kClamp_TileMode, &matrix);

        SkBitmap translucentDiffuseMap =
                ToolUtils::create_checkerboard_bitmap(kTexSize,
                                                      kTexSize,
                                                      SkColorSetARGB(0x55, 0x00, 0x00, 0x00),
                                                      SkColorSetARGB(0x55, 0x80, 0x80, 0x80),
                                                      8);
        fTranslucentDiffuse = SkShader::MakeBitmapShader(translucentDiffuseMap,
                                                         SkShader::kClamp_TileMode,
                                                         SkShader::kClamp_TileMode, &matrix);

        SkBitmap normalMap = make_frustum_normalmap(kTexSize);
        fNormalMapShader = SkShader::MakeBitmapShader(normalMap, SkShader::kClamp_TileMode,
                                                      SkShader::kClamp_TileMode, &matrix);

    }

    // Scales shape around origin, rotates shape around origin, then translates shape to origin
    void positionCTM(SkCanvas *canvas, SkScalar scaleX, SkScalar scaleY, SkScalar rotate) const {
        canvas->translate(kTexSize/2.0f, kTexSize/2.0f);
        canvas->scale(scaleX, scaleY);
        canvas->rotate(rotate);
        canvas->translate(-kTexSize/2.0f, -kTexSize/2.0f);
    }

    void drawRect(SkCanvas* canvas, SkScalar scaleX, SkScalar scaleY,
                  SkScalar rotate, bool useNormalSource, bool useDiffuseShader,
                  bool useTranslucentPaint, bool useTranslucentShader, sk_sp<SkLights> lights) {
        canvas->save();

        this->positionCTM(canvas, scaleX, scaleY, rotate);

        const SkMatrix& ctm = canvas->getTotalMatrix();

        SkPaint paint;
        sk_sp<SkNormalSource> normalSource = nullptr;
        sk_sp<SkShader> diffuseShader = nullptr;

        if (useNormalSource) {
            normalSource = SkNormalSource::MakeFromNormalMap(fNormalMapShader, ctm);
        }

        if (useDiffuseShader) {
            diffuseShader = (useTranslucentShader) ? fTranslucentDiffuse : fOpaqueDiffuse;
        } else {
            paint.setColor(SK_ColorGREEN);
        }

        if (useTranslucentPaint) {
            paint.setAlpha(0x99);
        }

        paint.setShader(SkLightingShader::Make(std::move(diffuseShader), std::move(normalSource),
                                               std::move(lights)));
        canvas->drawRect(fRect, paint);

        canvas->restore();
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint labelPaint;
        SkFont  font(ToolUtils::create_portable_typeface("sans-serif", SkFontStyle()), kLabelSize);

        int gridNum = 0;

        // Running through all possible bool parameter combinations
        for (bool useNormalSource : {true, false}) {
            for (bool useDiffuseShader : {true, false}) {
                for (bool useTranslucentPaint : {true, false}) {
                    for (bool useTranslucentShader : {true, false}) {

                        // Determining position
                        SkScalar xPos = (gridNum % kGridColumnNum) * kGridCellWidth;
                        SkScalar yPos = (gridNum / kGridColumnNum) * kGridCellWidth;

                        canvas->save();

                        canvas->translate(xPos, yPos);
                        this->drawRect(canvas, 1.0f, 1.0f, 0.f, useNormalSource, useDiffuseShader,
                                       useTranslucentPaint, useTranslucentShader, fLights);
                        // Drawing labels
                        canvas->translate(0.0f, SkIntToScalar(kTexSize));
                        {
                            canvas->translate(0.0f, kLabelSize);
                            SkString label;
                            label.appendf("useNormalSource: %d", useNormalSource);
                            canvas->drawString(label, 0.0f, 0.0f, font, labelPaint);
                        }
                        {
                            canvas->translate(0.0f, kLabelSize);
                            SkString label;
                            label.appendf("useDiffuseShader: %d", useDiffuseShader);
                            canvas->drawString(label, 0.0f, 0.0f, font, labelPaint);
                        }
                        {
                            canvas->translate(0.0f, kLabelSize);
                            SkString label;
                            label.appendf("useTranslucentPaint: %d", useTranslucentPaint);
                            canvas->drawString(label, 0.0f, 0.0f, font, labelPaint);
                        }
                        {
                            canvas->translate(0.0f, kLabelSize);
                            SkString label;
                            label.appendf("useTranslucentShader: %d", useTranslucentShader);
                            canvas->drawString(label, 0.0f, 0.0f, font, labelPaint);
                        }

                        canvas->restore();

                        gridNum++;
                    }
                }
            }
        }


        // Rotation/scale test
        {
            SkScalar xPos = (gridNum % kGridColumnNum) * kGridCellWidth;
            SkScalar yPos = (gridNum / kGridColumnNum) * kGridCellWidth;

            canvas->save();
            canvas->translate(xPos, yPos);
            this->drawRect(canvas, 0.6f, 0.6f, 45.0f, true, true, true, true, fLights);
            canvas->restore();

            gridNum++;
        }

        // Anisotropic scale test
        {
            SkScalar xPos = (gridNum % kGridColumnNum) * kGridCellWidth;
            SkScalar yPos = (gridNum / kGridColumnNum) * kGridCellWidth;

            canvas->save();
            canvas->translate(xPos, yPos);
            this->drawRect(canvas, 0.6f, 0.4f, 30.0f, true, true, true, true, fLights);
            canvas->restore();

            gridNum++;
        }

        // No directional lights test
        {
            SkScalar xPos = (gridNum % kGridColumnNum) * kGridCellWidth;
            SkScalar yPos = (gridNum / kGridColumnNum) * kGridCellWidth;

            canvas->save();
            canvas->translate(xPos, yPos);
            this->drawRect(canvas, 1.0f, 1.0f, 0.0f, true, true, false, false, fLightsNoDir);
            canvas->restore();

            gridNum++;
        }

        // Two directional lights test
        {
            SkScalar xPos = (gridNum % kGridColumnNum) * kGridCellWidth;
            SkScalar yPos = (gridNum / kGridColumnNum) * kGridCellWidth;

            canvas->save();
            canvas->translate(xPos, yPos);
            this->drawRect(canvas, 1.0f, 1.0f, 0.0f, true, true, false, false, fLightsTwoDir);
            canvas->restore();

            gridNum++;
        }
    }

private:
    static constexpr int kTexSize = 96;
    static constexpr int kNumBooleanParams = 4;
    static constexpr SkScalar kLabelSize = 10.0f;
    static constexpr int kGridColumnNum = 4;
    static constexpr SkScalar kGridCellWidth = kTexSize + 20.0f + kNumBooleanParams * kLabelSize;

    sk_sp<SkShader> fOpaqueDiffuse;
    sk_sp<SkShader> fTranslucentDiffuse;
    sk_sp<SkShader> fNormalMapShader;

    const SkRect    fRect;
    sk_sp<SkLights> fLights;
    sk_sp<SkLights> fLightsNoDir;
    sk_sp<SkLights> fLightsTwoDir;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new LightingShader2GM;)
}
