/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "sk_tool_utils.h"
#include "Resources.h"
#include "SkCanvas.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkTypeface.h"

class DFTextGM : public skiagm::GM {
public:
    DFTextGM() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    void onOnceBeforeDraw() override {
        fEmojiTypeface = sk_tool_utils::emoji_typeface();
        fEmojiText = sk_tool_utils::emoji_sample_text();
    }

    SkString onShortName() override {
        SkString name("dftext");
        name.append(sk_tool_utils::platform_os_emoji());
        return name;
    }

    SkISize onISize() override {
        return SkISize::Make(1024, 768);
    }

    virtual void onDraw(SkCanvas* inputCanvas) override {
        SkScalar textSizes[] = { 9.0f, 9.0f*2.0f, 9.0f*5.0f, 9.0f*2.0f*5.0f };
        SkScalar scales[] = { 2.0f*5.0f, 5.0f, 2.0f, 1.0f };

        // set up offscreen rendering with distance field text
#if SK_SUPPORT_GPU
        GrContext* ctx = inputCanvas->getGrContext();
        SkISize size = onISize();
        SkImageInfo info = SkImageInfo::MakeN32(size.width(), size.height(), kPremul_SkAlphaType,
                                                inputCanvas->imageInfo().refColorSpace());
        SkSurfaceProps props(SkSurfaceProps::kUseDeviceIndependentFonts_Flag,
                             SkSurfaceProps::kLegacyFontHost_InitType);
        auto surface(SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info, 0, &props));
        SkCanvas* canvas = surface ? surface->getCanvas() : inputCanvas;
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

        sk_tool_utils::set_portable_typeface(&paint, "serif");

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
            y += paint.getFontMetrics(nullptr)*scales[i];
        }

        // check rotation
        for (size_t i = 0; i < 5; ++i) {
            SkScalar rotX = SkIntToScalar(10);
            SkScalar rotY = y;

            SkAutoCanvasRestore acr(canvas, true);
            canvas->translate(SkIntToScalar(10 + i * 200), -80);
            canvas->rotate(SkIntToScalar(i * 5), rotX, rotY);
            for (int ps = 6; ps <= 32; ps += 3) {
                paint.setTextSize(SkIntToScalar(ps));
                canvas->drawText(text, textLen, rotX, rotY, paint);
                rotY += paint.getFontMetrics(nullptr);
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
            y += paint.getFontMetrics(nullptr)*scaleFactor;
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

        paint.setColor(0xFFF7F3F7);
        SkRect r = SkRect::MakeLTRB(670, 215, 820, 397);
        canvas->drawRect(r, paint);

        x = SkIntToScalar(680);
        y = SkIntToScalar(235);
        paint.setTextSize(SkIntToScalar(19));
        for (size_t i = 0; i < SK_ARRAY_COUNT(fg); ++i) {
            paint.setColor(fg[i]);

            canvas->drawText(text, textLen, x, y, paint);
            y += paint.getFontMetrics(nullptr);
        }

        paint.setColor(0xFF181C18);
        r = SkRect::MakeLTRB(820, 215, 970, 397);
        canvas->drawRect(r, paint);

        x = SkIntToScalar(830);
        y = SkIntToScalar(235);
        paint.setTextSize(SkIntToScalar(19));
        for (size_t i = 0; i < SK_ARRAY_COUNT(fg); ++i) {
            paint.setColor(fg[i]);

            canvas->drawText(text, textLen, x, y, paint);
            y += paint.getFontMetrics(nullptr);
        }

        // check skew
        {
            paint.setLCDRenderText(false);
            SkAutoCanvasRestore acr(canvas, true);
            canvas->skew(0.0f, 0.151515f);
            paint.setTextSize(SkIntToScalar(32));
            canvas->drawText(text, textLen, 745, 70, paint);
        }
        {
            paint.setLCDRenderText(true);
            SkAutoCanvasRestore acr(canvas, true);
            canvas->skew(0.5f, 0.0f);
            paint.setTextSize(SkIntToScalar(32));
            canvas->drawText(text, textLen, 580, 125, paint);
        }

        // check color emoji
        if (fEmojiTypeface) {
            paint.setTypeface(fEmojiTypeface);
            paint.setTextSize(SkIntToScalar(19));
            canvas->drawString(fEmojiText, 670, 90, paint);
        }
#if SK_SUPPORT_GPU
        // render offscreen buffer
        if (surface) {
            SkAutoCanvasRestore acr(inputCanvas, true);
            // since we prepended this matrix already, we blit using identity
            inputCanvas->resetMatrix();
            inputCanvas->drawImage(surface->makeImageSnapshot().get(), 0, 0, nullptr);
        }
#endif
    }

private:
    sk_sp<SkTypeface> fEmojiTypeface;
    const char* fEmojiText;

    typedef skiagm::GM INHERITED;
};

DEF_GM(return new DFTextGM;)
