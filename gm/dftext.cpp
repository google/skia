/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "Resources.h"
#include "SkCanvas.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkTypeface.h"

class DFTextGM : public skiagm::GM {
public:
    DFTextGM() {
        this->setBGColor(0xFFFFFFFF);
        fTypeface = NULL;
    }

    virtual ~DFTextGM() {
        SkSafeUnref(fTypeface);
    }

protected:
    void onOnceBeforeDraw() SK_OVERRIDE {
        SkString filename = GetResourcePath("/Funkster.ttf");
        SkAutoTDelete<SkFILEStream> stream(new SkFILEStream(filename.c_str()));
        if (!stream->isValid()) {
            SkDebugf("Could not find Funkster.ttf, please set --resourcePath correctly.\n");
            return;
        }

        fTypeface = SkTypeface::CreateFromStream(stream.detach());
    }

    SkString onShortName() SK_OVERRIDE {
        return SkString("dftext");
    }

    SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(1024, 768);
    }

    static void rotate_about(SkCanvas* canvas,
        SkScalar degrees,
        SkScalar px, SkScalar py) {
        canvas->translate(px, py);
        canvas->rotate(degrees);
        canvas->translate(-px, -py);
    }

    virtual void onDraw(SkCanvas* inputCanvas) SK_OVERRIDE {
#ifdef SK_BUILD_FOR_ANDROID
        SkScalar textSizes[] = { 9.0f, 9.0f*2.0f, 9.0f*5.0f, 9.0f*2.0f*5.0f };
#else
        SkScalar textSizes[] = { 11.0f, 11.0f*2.0f, 11.0f*5.0f, 11.0f*2.0f*5.0f };
#endif
        SkScalar scales[] = { 2.0f*5.0f, 5.0f, 2.0f, 1.0f };

        // set up offscreen rendering with distance field text
#if SK_SUPPORT_GPU
        GrContext* ctx = inputCanvas->getGrContext();
        SkImageInfo info = SkImageInfo::MakeN32Premul(onISize());
        SkSurfaceProps props(SkSurfaceProps::kUseDistanceFieldFonts_Flag,
                             SkSurfaceProps::kLegacyFontHost_InitType);
        SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTarget(ctx, SkSurface::kNo_Budgeted,
                                                                   info, 0, &props));
        SkCanvas* canvas = surface.get() ? surface->getCanvas() : inputCanvas;
        // init our new canvas with the old canvas's matrix
        canvas->setMatrix(inputCanvas->getTotalMatrix());
#else
        SkCanvas* canvas = inputCanvas;
#endif
        // apply global scale to test glyph positioning
        canvas->scale(1.05f, 1.05f);
        canvas->clear(0xffffffff);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setSubpixelText(true);
#if !SK_SUPPORT_GPU
        paint.setDistanceFieldTextTEMP(true);
#endif
        sk_tool_utils::set_portable_typeface(&paint, "Times New Roman", SkTypeface::kNormal);

        const char* text = "Hamburgefons";
        const size_t textLen = strlen(text);

        // check scaling up
        SkScalar x = SkIntToScalar(0);
        SkScalar y = SkIntToScalar(78);
        for (size_t i = 0; i < SK_ARRAY_COUNT(textSizes); ++i) {
            SkAutoCanvasRestore acr(canvas, true);
            canvas->translate(x, y);
            canvas->scale(scales[i], scales[i]);
            paint.setTextSize(textSizes[i]);
            canvas->drawText(text, textLen, 0, 0, paint);
            y += paint.getFontMetrics(NULL)*scales[i];
        }

        // check rotation
        for (size_t i = 0; i < 5; ++i) {
            SkScalar rotX = SkIntToScalar(10);
            SkScalar rotY = y;

            SkAutoCanvasRestore acr(canvas, true);
            canvas->translate(SkIntToScalar(10 + i * 200), -80);
            rotate_about(canvas, SkIntToScalar(i * 5), rotX, rotY);
            for (int ps = 6; ps <= 32; ps += 3) {
                paint.setTextSize(SkIntToScalar(ps));
                canvas->drawText(text, textLen, rotX, rotY, paint);
                rotY += paint.getFontMetrics(NULL);
            }
        }

        // check scaling down
        paint.setLCDRenderText(true);
        x = SkIntToScalar(680);
        y = SkIntToScalar(20);
        size_t arraySize = SK_ARRAY_COUNT(textSizes);
        for (size_t i = 0; i < arraySize; ++i) {
            SkAutoCanvasRestore acr(canvas, true);
            canvas->translate(x, y);
            SkScalar scaleFactor = SkScalarInvert(scales[arraySize - i - 1]);
            canvas->scale(scaleFactor, scaleFactor);
            paint.setTextSize(textSizes[i]);
            canvas->drawText(text, textLen, 0, 0, paint);
            y += paint.getFontMetrics(NULL)*scaleFactor;
        }

        // check pos text
        {
            SkAutoCanvasRestore acr(canvas, true);

            canvas->scale(2.0f, 2.0f);

            SkAutoTArray<SkPoint>  pos(SkToInt(textLen));
            SkAutoTArray<SkScalar> widths(SkToInt(textLen));
            paint.setTextSize(textSizes[0]);

            paint.getTextWidths(text, textLen, &widths[0]);

            SkScalar x = SkIntToScalar(340);
            SkScalar y = SkIntToScalar(75);
            for (unsigned int i = 0; i < textLen; ++i) {
                pos[i].set(x, y);
                x += widths[i];
            }

            canvas->drawPosText(text, textLen, &pos[0], paint);
        }


        // check gamma-corrected blending
        const SkColor fg[] = {
            0xFFFFFFFF,
            0xFFFFFF00, 0xFFFF00FF, 0xFF00FFFF,
            0xFFFF0000, 0xFF00FF00, 0xFF0000FF,
            0xFF000000,
        };

        paint.setColor(0xFFF1F1F1);
        SkRect r = SkRect::MakeLTRB(670, 250, 820, 460);
        canvas->drawRect(r, paint);

        x = SkIntToScalar(680);
        y = SkIntToScalar(270);
#ifdef SK_BUILD_FOR_ANDROID
        paint.setTextSize(SkIntToScalar(19));
#else
        paint.setTextSize(SkIntToScalar(22));
#endif
        for (size_t i = 0; i < SK_ARRAY_COUNT(fg); ++i) {
            paint.setColor(fg[i]);

            canvas->drawText(text, textLen, x, y, paint);
            y += paint.getFontMetrics(NULL);
        }

        paint.setColor(0xFF1F1F1F);
        r = SkRect::MakeLTRB(820, 250, 970, 460);
        canvas->drawRect(r, paint);

        x = SkIntToScalar(830);
        y = SkIntToScalar(270);
#ifdef SK_BUILD_FOR_ANDROID
        paint.setTextSize(SkIntToScalar(19));
#else
        paint.setTextSize(SkIntToScalar(22));
#endif
        for (size_t i = 0; i < SK_ARRAY_COUNT(fg); ++i) {
            paint.setColor(fg[i]);

            canvas->drawText(text, textLen, x, y, paint);
            y += paint.getFontMetrics(NULL);
        }

        // check color emoji
        paint.setTypeface(fTypeface);
        canvas->drawText(text, textLen, 670, 100, paint);

#if SK_SUPPORT_GPU
        // render offscreen buffer
        if (surface) {
            SkAutoCanvasRestore acr(inputCanvas, true);
            // since we prepended this matrix already, we blit using identity
            inputCanvas->resetMatrix();
            SkImage* image = surface->newImageSnapshot();
            inputCanvas->drawImage(image, 0, 0, NULL);
            image->unref();
        }
#endif
    }

private:
    SkTypeface* fTypeface;

    typedef skiagm::GM INHERITED;
};

DEF_GM( return SkNEW(DFTextGM); )
