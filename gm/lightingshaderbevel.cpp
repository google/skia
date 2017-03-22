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
#include "SkPath.h"
#include "SkPoint3.h"
#include "SkShader.h"


namespace skiagm {

// This GM exercises lighting shaders when used with bevel SkNormalSource objects.
class LightingShaderBevelGM : public GM {
public:
    LightingShaderBevelGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

protected:
    SkString onShortName() override {
        return SkString("lightingshaderbevel");
    }

    SkISize onISize() override {
        return SkISize::Make(SkScalarCeilToInt(GRID_NUM_COLUMNS * GRID_CELL_WIDTH),
                             SkScalarCeilToInt(GRID_NUM_ROWS    * GRID_CELL_WIDTH));
    }

    void onOnceBeforeDraw() override {
        SkLights::Builder builder;
        const SkVector3 kLightFromUpperRight = SkVector3::Make(0.788f, 0.394f, 0.473f);

        builder.add(SkLights::Light::MakeDirectional(SkColor3f::Make(1.0f, 1.0f, 1.0f),
                                                     kLightFromUpperRight));
        builder.setAmbientLightColor(SkColor3f::Make(0.2f, 0.2f, 0.2f));
        fLights = builder.finish();

        // fRect is assumed to be square throughout this file
        fRect = SkRect::MakeIWH(kTexSize, kTexSize);
        SkMatrix matrix;
        SkRect bitmapBounds = SkRect::MakeIWH(kTexSize, kTexSize);
        matrix.setRectToRect(bitmapBounds, fRect, SkMatrix::kFill_ScaleToFit);

        SkBitmap diffuseMap = sk_tool_utils::create_checkerboard_bitmap(
                kTexSize, kTexSize,
                sk_tool_utils::color_to_565(0x0),
                sk_tool_utils::color_to_565(0xFF804020),
                8);
        fDiffuse = SkShader::MakeBitmapShader(diffuseMap, SkShader::kClamp_TileMode,
                                              SkShader::kClamp_TileMode, &matrix);

        fConvexPath.moveTo(fRect.width() / 2.0f, 0.0f);
        fConvexPath.lineTo(0.0f, fRect.height());
        fConvexPath.lineTo(fRect.width(), fRect.height());
        fConvexPath.close();

        // Creating concave path
        {
            SkScalar x = 0.0f;
            SkScalar y = fRect.height() / 2.0f;

            const int NUM_SPIKES = 8;

            const SkScalar x0 = x;
            const SkScalar dx = fRect.width() / (NUM_SPIKES * 2);
            const SkScalar dy = SK_Scalar1 * 10;


            fConcavePath.moveTo(x, y + dy);
            for (int i = 0; i < NUM_SPIKES; i++) {
                x += dx;
                fConcavePath.lineTo(x, y - dy);
                x += dx;
                fConcavePath.lineTo(x, y + dy);
            }
            fConcavePath.lineTo(x, y + (2 * dy));
            fConcavePath.lineTo(x0, y + (2 * dy));
            fConcavePath.close();
        }
    }

    // Scales shape around origin, rotates shape around origin, then translates shape to origin
    void positionCTM(SkCanvas *canvas, SkScalar scaleX, SkScalar scaleY, SkScalar rotate) const {
        canvas->translate(kTexSize/2.0f, kTexSize/2.0f);
        canvas->scale(scaleX, scaleY);
        canvas->rotate(rotate);
        canvas->translate(-kTexSize/2.0f, -kTexSize/2.0f);
    }

    enum Shape {
        kCircle_Shape,
        kRect_Shape,
        kRRect_Shape,
        kConvexPath_Shape,
        kConcavePath_Shape,

        kLast_Shape = kConcavePath_Shape
    };
    void drawShape(enum Shape shape, SkCanvas* canvas, SkScalar scaleX, SkScalar scaleY,
                   SkScalar rotate, SkNormalSource::BevelType bevelType, SkScalar bevelHeight) {
        canvas->save();

        this->positionCTM(canvas, scaleX, scaleY, rotate);

        SkPaint paint;

        SkScalar bevelWidth = 10.0f;
        sk_sp<SkNormalSource> normalSource = SkNormalSource::MakeBevel(bevelType, bevelWidth,
                                                                       bevelHeight);

        paint.setShader(SkLightingShader::Make(fDiffuse, std::move(normalSource), fLights));
        paint.setAntiAlias(true);
        switch(shape) {
            case kCircle_Shape:
                canvas->drawCircle(fRect.centerX(), fRect.centerY(), fRect.width()/2.0f, paint);
                break;
            case kRect_Shape:
                canvas->drawRect(fRect, paint);
                break;
            case kRRect_Shape:
                canvas->drawRoundRect(fRect, 5.0f, 5.0f, paint);
                break;
            case kConvexPath_Shape:
                canvas->drawPath(fConvexPath, paint);
                break;
            case kConcavePath_Shape:
                canvas->drawPath(fConcavePath, paint);
                break;
            default:
                SkDEBUGFAIL("Invalid shape enum for drawShape");
        }

        canvas->restore();
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint labelPaint;
        labelPaint.setTypeface(sk_tool_utils::create_portable_typeface("sans-serif",
                                                                       SkFontStyle()));
        labelPaint.setAntiAlias(true);
        labelPaint.setTextSize(LABEL_SIZE);

        int gridNum = 0;

        // Running through all possible parameter combinations
        for (auto bevelType : {SkNormalSource::BevelType::kLinear,
                               SkNormalSource::BevelType::kRoundedIn,
                               SkNormalSource::BevelType::kRoundedOut}) {
            for (SkScalar bevelHeight: {-7.0f, 7.0f}) {
                for (int shapeInt = 0; shapeInt < NUM_SHAPES; shapeInt++) {
                    Shape shape = (Shape)shapeInt;

                    // Determining position
                    SkScalar xPos = (gridNum / GRID_NUM_ROWS) * GRID_CELL_WIDTH;
                    SkScalar yPos = (gridNum % GRID_NUM_ROWS) * GRID_CELL_WIDTH;

                    canvas->save();

                    canvas->translate(xPos, yPos);
                    this->drawShape(shape, canvas, 1.0f, 1.0f, 0.f, bevelType, bevelHeight);
                    // Drawing labels
                    canvas->translate(0.0f, SkIntToScalar(kTexSize));
                    {
                        canvas->translate(0.0f, LABEL_SIZE);
                        SkString label;
                        label.append("bevelType: ");
                        switch (bevelType) {
                            case SkNormalSource::BevelType::kLinear:
                                label.append("linear");
                                break;
                            case SkNormalSource::BevelType::kRoundedIn:
                                label.append("roundedIn");
                                break;
                            case SkNormalSource::BevelType::kRoundedOut:
                                label.append("roundedOut");
                                break;
                        }
                        canvas->drawText(label.c_str(), label.size(), 0.0f, 0.0f, labelPaint);
                    }
                    {
                        canvas->translate(0.0f, LABEL_SIZE);
                        SkString label;
                        label.appendf("bevelHeight: %.1f", bevelHeight);
                        canvas->drawText(label.c_str(), label.size(), 0.0f, 0.0f, labelPaint);
                    }

                    canvas->restore();

                    gridNum++;
                }
            }
        }

        // Testing rotation
        for (int shapeInt = 0; shapeInt < NUM_SHAPES; shapeInt++) {
            Shape shape = (Shape)shapeInt;

            // Determining position
            SkScalar xPos = (gridNum / GRID_NUM_ROWS) * GRID_CELL_WIDTH;
            SkScalar yPos = (gridNum % GRID_NUM_ROWS) * GRID_CELL_WIDTH;

            canvas->save();

            canvas->translate(xPos, yPos);
            this->drawShape(shape, canvas, SK_ScalarRoot2Over2, SK_ScalarRoot2Over2, 45.0f,
                            SkNormalSource::BevelType::kLinear, 7.0f);

            // Drawing labels
            canvas->translate(0.0f, SkIntToScalar(kTexSize));
            {
                canvas->translate(0.0f, LABEL_SIZE);
                SkString label;
                label.appendf("bevelType: linear");
                canvas->drawText(label.c_str(), label.size(), 0.0f, 0.0f, labelPaint);
            }
            {
                canvas->translate(0.0f, LABEL_SIZE);
                SkString label;
                label.appendf("bevelHeight: %.1f", 7.0f);
                canvas->drawText(label.c_str(), label.size(), 0.0f, 0.0f, labelPaint);
            }
            {
                canvas->translate(0.0f, LABEL_SIZE);
                SkString label;
                label.appendf("rotated");
                canvas->drawText(label.c_str(), label.size(), 0.0f, 0.0f, labelPaint);
            }

            canvas->restore();

            gridNum++;
        }

        // Making sure NUM_COMBINATIONS_PER_SHAPE is set correctly
        SkASSERT(gridNum == (NUM_COMBINATIONS_PER_SHAPE*NUM_SHAPES));
    }

private:
    static constexpr int kTexSize = 96;
    static constexpr int NUM_SHAPES = kLast_Shape + 1;
    static constexpr int NUM_COMBINATIONS_PER_SHAPE = 7;
    static constexpr int GRID_NUM_ROWS = NUM_SHAPES;
    static constexpr int GRID_NUM_COLUMNS = NUM_COMBINATIONS_PER_SHAPE;
    static constexpr SkScalar LABEL_SIZE = 10.0f;
    static constexpr int NUM_LABELS_PER_CELL = 3;
    static constexpr SkScalar GRID_CELL_WIDTH = kTexSize + 10.0f + NUM_LABELS_PER_CELL * LABEL_SIZE;

    sk_sp<SkShader> fDiffuse;

    SkRect fRect;
    SkPath fConvexPath;
    SkPath fConcavePath;
    sk_sp<SkLights> fLights;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new LightingShaderBevelGM;)
}
