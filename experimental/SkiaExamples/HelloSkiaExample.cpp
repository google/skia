/*
 * Copyright 2013 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "SkExample.h"

#include "SkApplication.h"
#include "SkDraw.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"

class HelloSkia : public SkExample {
public:
    HelloSkia(SkExampleWindow* window) : SkExample(window) {
        fName = "HelloSkia";
        fBGColor = SK_ColorWHITE;
        fRotationAngle = SkIntToScalar(0);

        fWindow->setupBackend(SkExampleWindow::kGPU_DeviceType);
        // Another option is software rendering:
        // fWindow->setupBackend(SkExampleWindow::kRaster_DeviceType);
    }

protected:
    void draw(SkCanvas* canvas) {
        // Clear background
        canvas->drawColor(fBGColor);

        SkPaint paint;
        paint.setColor(SK_ColorRED);

        // Draw a rectangle with blue paint
        SkRect rect = {
                SkIntToScalar(10), SkIntToScalar(10),
                SkIntToScalar(128), SkIntToScalar(128)
        };
        canvas->drawRect(rect, paint);

        // Set up a linear gradient and draw a circle
        {
            SkPoint linearPoints[] = {
                    {SkIntToScalar(0), SkIntToScalar(0)},
                    {SkIntToScalar(300), SkIntToScalar(300)}
            };
            SkColor linearColors[] = {SK_ColorGREEN, SK_ColorBLACK};

            SkShader* shader = SkGradientShader::CreateLinear(
                    linearPoints, linearColors, NULL, 2,
                    SkShader::kMirror_TileMode);
            SkAutoUnref shader_deleter(shader);

            paint.setShader(shader);
            paint.setFlags(SkPaint::kAntiAlias_Flag);

            canvas->drawCircle(SkIntToScalar(200), SkIntToScalar(200),
                    SkIntToScalar(64), paint);

            // Detach shader
            paint.setShader(NULL);
        }

        // Draw a message with a nice black paint.
        paint.setFlags(
                SkPaint::kAntiAlias_Flag |
                SkPaint::kSubpixelText_Flag |  // ... avoid waggly text when rotating.
                SkPaint::kUnderlineText_Flag);
        paint.setColor(SK_ColorBLACK);
        paint.setTextSize(SkIntToScalar(20));

        canvas->save();

        static const char message[] = "Hello Skia!!!";

        // Translate and rotate
        canvas->translate(SkIntToScalar(300), SkIntToScalar(300));
        fRotationAngle += SkDoubleToScalar(0.2);
        if (fRotationAngle > SkDoubleToScalar(360.0)) {
            fRotationAngle -= SkDoubleToScalar(360.0);
        }
        canvas->rotate(fRotationAngle);

        // Draw the text:
        canvas->drawText(message, strlen(message), SkIntToScalar(0), SkIntToScalar(0), paint);

        canvas->restore();

        // Invalidate the window to force a redraw. Poor man's animation mechanism.
        this->fWindow->inval(NULL);
    }

private:
    SkScalar fRotationAngle;
    SkColor fBGColor;
};

static SkExample* MyFactory(SkExampleWindow* window) {
    return new HelloSkia(window);
}

// Register this class as a Skia Example.
SkExample::Registry registry(MyFactory);
