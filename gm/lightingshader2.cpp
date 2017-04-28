/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkLightingShader.h"
#include "SkNormalSource.h"
#include "SkPoint3.h"
#include "SkShader.h"

// Create a truncated pyramid normal map
static SkBitmap make_frustum_normalmap(int texSize) {
    SkBitmap frustum;
    frustum.allocN32Pixels(texSize, texSize);

    sk_tool_utils::create_frustum_normal_map(&frustum, SkIRect::MakeWH(texSize, texSize));
    return frustum;
}

namespace skiagm {

// This GM exercises lighting shaders. Specifically, nullptr arguments, scaling when using
// normal maps, paint transparency, zero directional lights, multiple directional lights.
class LightingShader2GM : public GM {
public:
    LightingShader2GM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

protected:
    SkString onShortName() override {
        return SkString("lightingshader2");
    }

    SkISize onISize() override {
        return SkISize::Make(600, 740);
    }

    void onOnceBeforeDraw() override {
        const SkVector3 kLightFromUpperRight = SkVector3::Make(0.788f, 0.394f, 0.473f);
        const SkVector3 kLightFromUpperLeft = SkVector3::Make(-0.788f, 0.394f, 0.473f);

        // Standard set of lights
        SkLights::Builder builder;
        builder.add(SkLights::Light::MakeDirectional(SkColor3f::Make(1.0f, 1.0f, 1.0f),
                                                     kLightFromUpperRight));
        builder.setAmbientLightColor(SkColor3f::Make(0.2f, 0.2f, 0.2f));
        fLights = builder.finish();

        // No directional lights
        SkLights::Builder builderNoDir;
        builderNoDir.setAmbientLightColor(SkColor3f::Make(0.2f, 0.2f, 0.2f));
        fLightsNoDir = builderNoDir.finish();

        // Two directional lights
        SkLights::Builder builderTwoDir;
        builderTwoDir.add(SkLights::Light::MakeDirectional(SkColor3f::Make(1.0f, 0.0f, 1.0f),
                                                           kLightFromUpperRight));
        builderTwoDir.add(SkLights::Light::MakeDirectional(SkColor3f::Make(0.0f, 1.0f, 1.0f),
                                                           kLightFromUpperLeft));
        builderTwoDir.setAmbientLightColor(SkColor3f::Make(0.2f, 0.2f, 0.2f));
        fLightsTwoDir = builderTwoDir.finish();

        fRect = SkRect::MakeIWH(kTexSize, kTexSize);
        SkMatrix matrix;
        SkRect bitmapBounds = SkRect::MakeIWH(kTexSize, kTexSize);
        matrix.setRectToRect(bitmapBounds, fRect, SkMatrix::kFill_ScaleToFit);

        SkBitmap opaqueDiffuseMap = sk_tool_utils::create_checkerboard_bitmap(
                kTexSize, kTexSize,
                sk_tool_utils::color_to_565(0x0),
                sk_tool_utils::color_to_565(0xFF804020),
                8);
        fOpaqueDiffuse = SkShader::MakeBitmapShader(opaqueDiffuseMap, SkShader::kClamp_TileMode,
                                                    SkShader::kClamp_TileMode, &matrix);

        SkBitmap translucentDiffuseMap = sk_tool_utils::create_checkerboard_bitmap(
                kTexSize, kTexSize,
                SkColorSetARGB(0x55, 0x00, 0x00, 0x00),
                SkColorSetARGB(0x55, 0x80, 0x40, 0x20),
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

    static constexpr int NUM_BOOLEAN_PARAMS = 4;
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
            paint.setColor(0xFF00FF00);
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

        constexpr SkScalar LABEL_SIZE = 10.0f;
        SkPaint labelPaint;
        labelPaint.setTypeface(sk_tool_utils::create_portable_typeface("sans-serif",
                                                                       SkFontStyle()));
        labelPaint.setAntiAlias(true);
        labelPaint.setTextSize(LABEL_SIZE);

        constexpr int GRID_COLUMN_NUM = 4;
        constexpr SkScalar GRID_CELL_WIDTH = kTexSize + 20.0f + NUM_BOOLEAN_PARAMS * LABEL_SIZE;

        int gridNum = 0;

        // Running through all possible bool parameter combinations
        for (bool useNormalSource : {true, false}) {
            for (bool useDiffuseShader : {true, false}) {
                for (bool useTranslucentPaint : {true, false}) {
                    for (bool useTranslucentShader : {true, false}) {

                        // Determining position
                        SkScalar xPos = (gridNum % GRID_COLUMN_NUM) * GRID_CELL_WIDTH;
                        SkScalar yPos = (gridNum / GRID_COLUMN_NUM) * GRID_CELL_WIDTH;

                        canvas->save();

                        canvas->translate(xPos, yPos);
                        this->drawRect(canvas, 1.0f, 1.0f, 0.f, useNormalSource, useDiffuseShader,
                                       useTranslucentPaint, useTranslucentShader, fLights);
                        // Drawing labels
                        canvas->translate(0.0f, SkIntToScalar(kTexSize));
                        {
                            canvas->translate(0.0f, LABEL_SIZE);
                            SkString label;
                            label.appendf("useNormalSource: %d", useNormalSource);
                            canvas->drawString(label, 0.0f, 0.0f, labelPaint);
                        }
                        {
                            canvas->translate(0.0f, LABEL_SIZE);
                            SkString label;
                            label.appendf("useDiffuseShader: %d", useDiffuseShader);
                            canvas->drawString(label, 0.0f, 0.0f, labelPaint);
                        }
                        {
                            canvas->translate(0.0f, LABEL_SIZE);
                            SkString label;
                            label.appendf("useTranslucentPaint: %d", useTranslucentPaint);
                            canvas->drawString(label, 0.0f, 0.0f, labelPaint);
                        }
                        {
                            canvas->translate(0.0f, LABEL_SIZE);
                            SkString label;
                            label.appendf("useTranslucentShader: %d", useTranslucentShader);
                            canvas->drawString(label, 0.0f, 0.0f, labelPaint);
                        }

                        canvas->restore();

                        gridNum++;
                    }
                }
            }
        }


        // Rotation/scale test
        {
            SkScalar xPos = (gridNum % GRID_COLUMN_NUM) * GRID_CELL_WIDTH;
            SkScalar yPos = (gridNum / GRID_COLUMN_NUM) * GRID_CELL_WIDTH;

            canvas->save();
            canvas->translate(xPos, yPos);
            this->drawRect(canvas, 0.6f, 0.6f, 45.0f, true, true, true, true, fLights);
            canvas->restore();

            gridNum++;
        }

        // Anisotropic scale test
        {
            SkScalar xPos = (gridNum % GRID_COLUMN_NUM) * GRID_CELL_WIDTH;
            SkScalar yPos = (gridNum / GRID_COLUMN_NUM) * GRID_CELL_WIDTH;

            canvas->save();
            canvas->translate(xPos, yPos);
            this->drawRect(canvas, 0.6f, 0.4f, 30.0f, true, true, true, true, fLights);
            canvas->restore();

            gridNum++;
        }

        // No directional lights test
        {
            SkScalar xPos = (gridNum % GRID_COLUMN_NUM) * GRID_CELL_WIDTH;
            SkScalar yPos = (gridNum / GRID_COLUMN_NUM) * GRID_CELL_WIDTH;

            canvas->save();
            canvas->translate(xPos, yPos);
            this->drawRect(canvas, 1.0f, 1.0f, 0.0f, true, true, false, false, fLightsNoDir);
            canvas->restore();

            gridNum++;
        }

        // Two directional lights test
        {
            SkScalar xPos = (gridNum % GRID_COLUMN_NUM) * GRID_CELL_WIDTH;
            SkScalar yPos = (gridNum / GRID_COLUMN_NUM) * GRID_CELL_WIDTH;

            canvas->save();
            canvas->translate(xPos, yPos);
            this->drawRect(canvas, 1.0f, 1.0f, 0.0f, true, true, false, false, fLightsTwoDir);
            canvas->restore();

            gridNum++;
        }
    }

private:
    static constexpr int kTexSize = 96;

    sk_sp<SkShader> fOpaqueDiffuse;
    sk_sp<SkShader> fTranslucentDiffuse;
    sk_sp<SkShader> fNormalMapShader;

    SkRect fRect;
    sk_sp<SkLights> fLights;
    sk_sp<SkLights> fLightsNoDir;
    sk_sp<SkLights> fLightsTwoDir;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new LightingShader2GM;)
}
