/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <string.h>

using namespace skia_private;

class DFTextGM : public skiagm::GM {
public:
    DFTextGM() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    void onOnceBeforeDraw() override { fEmojiSample = ToolUtils::EmojiSample(); }

    SkString getName() const override { return SkString("dftext"); }

    SkISize getISize() override { return SkISize::Make(1024, 768); }

    void onDraw(SkCanvas* inputCanvas) override {
        SkScalar textSizes[] = { 9.0f, 9.0f*2.0f, 9.0f*5.0f, 9.0f*2.0f*5.0f };
        SkScalar scales[] = { 2.0f*5.0f, 5.0f, 2.0f, 1.0f };

        // set up offscreen rendering with distance field text
        auto ctx = inputCanvas->recordingContext();
        SkISize size = getISize();
        SkImageInfo info = SkImageInfo::MakeN32(size.width(), size.height(), kPremul_SkAlphaType,
                                                inputCanvas->imageInfo().refColorSpace());
        SkSurfaceProps inputProps;
        inputCanvas->getProps(&inputProps);
        SkSurfaceProps props(SkSurfaceProps::kUseDeviceIndependentFonts_Flag | inputProps.flags(),
                             inputProps.pixelGeometry());
        auto surface(SkSurfaces::RenderTarget(ctx, skgpu::Budgeted::kNo, info, 0, &props));
        SkCanvas* canvas = surface ? surface->getCanvas() : inputCanvas;
        // init our new canvas with the old canvas's matrix
        canvas->setMatrix(inputCanvas->getLocalToDeviceAs3x3());
        // apply global scale to test glyph positioning
        canvas->scale(1.05f, 1.05f);
        canvas->clear(0xffffffff);

        SkPaint paint;
        paint.setAntiAlias(true);

        SkFont font(ToolUtils::CreatePortableTypeface("serif", SkFontStyle()));
        font.setSubpixel(true);

        const char* text = "Hamburgefons";
        const size_t textLen = strlen(text);

        // check scaling up
        SkScalar x = SkIntToScalar(0);
        SkScalar y = SkIntToScalar(78);
        for (size_t i = 0; i < std::size(textSizes); ++i) {
            SkAutoCanvasRestore acr(canvas, true);
            canvas->translate(x, y);
            canvas->scale(scales[i], scales[i]);
            font.setSize(textSizes[i]);
            canvas->drawSimpleText(text, textLen, SkTextEncoding::kUTF8, 0, 0, font, paint);
            y += font.getMetrics(nullptr)*scales[i];
        }

        // check rotation
        for (size_t i = 0; i < 5; ++i) {
            SkScalar rotX = SkIntToScalar(10);
            SkScalar rotY = y;

            SkAutoCanvasRestore acr(canvas, true);
            canvas->translate(SkIntToScalar(10 + i * 200), -80);
            canvas->rotate(SkIntToScalar(i * 5), rotX, rotY);
            for (int ps = 6; ps <= 32; ps += 3) {
                font.setSize(SkIntToScalar(ps));
                canvas->drawSimpleText(text, textLen, SkTextEncoding::kUTF8, rotX, rotY, font, paint);
                rotY += font.getMetrics(nullptr);
            }
        }

        // check scaling down
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        x = SkIntToScalar(680);
        y = SkIntToScalar(20);
        size_t arraySize = std::size(textSizes);
        for (size_t i = 0; i < arraySize; ++i) {
            SkAutoCanvasRestore acr(canvas, true);
            canvas->translate(x, y);
            SkScalar scaleFactor = SkScalarInvert(scales[arraySize - i - 1]);
            canvas->scale(scaleFactor, scaleFactor);
            font.setSize(textSizes[i]);
            canvas->drawSimpleText(text, textLen, SkTextEncoding::kUTF8, 0, 0, font, paint);
            y += font.getMetrics(nullptr)*scaleFactor;
        }

        // check pos text
        {
            SkAutoCanvasRestore acr(canvas, true);

            canvas->scale(2.0f, 2.0f);

            AutoTArray<SkGlyphID> glyphs(SkToInt(textLen));
            int count = font.textToGlyphs(text, textLen, SkTextEncoding::kUTF8, glyphs.get(), textLen);
            AutoTArray<SkPoint>  pos(count);
            font.setSize(textSizes[0]);
            font.getPos(glyphs.get(), count, pos.get(), {340, 75});

            auto blob = SkTextBlob::MakeFromPosText(glyphs.get(), count * sizeof(SkGlyphID),
                                                    pos.get(), font, SkTextEncoding::kGlyphID);
            canvas->drawTextBlob(blob, 0, 0, paint);
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
        font.setSize(SkIntToScalar(19));
        for (size_t i = 0; i < std::size(fg); ++i) {
            paint.setColor(fg[i]);

            canvas->drawSimpleText(text, textLen, SkTextEncoding::kUTF8, x, y, font, paint);
            y += font.getMetrics(nullptr);
        }

        paint.setColor(0xFF181C18);
        r = SkRect::MakeLTRB(820, 215, 970, 397);
        canvas->drawRect(r, paint);

        x = SkIntToScalar(830);
        y = SkIntToScalar(235);
        font.setSize(SkIntToScalar(19));
        for (size_t i = 0; i < std::size(fg); ++i) {
            paint.setColor(fg[i]);

            canvas->drawSimpleText(text, textLen, SkTextEncoding::kUTF8, x, y, font, paint);
            y += font.getMetrics(nullptr);
        }

        // check skew
        {
            font.setEdging(SkFont::Edging::kAntiAlias);
            SkAutoCanvasRestore acr(canvas, true);
            canvas->skew(0.0f, 0.151515f);
            font.setSize(SkIntToScalar(32));
            canvas->drawSimpleText(text, textLen, SkTextEncoding::kUTF8, 745, 70, font, paint);
        }
        {
            font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
            SkAutoCanvasRestore acr(canvas, true);
            canvas->skew(0.5f, 0.0f);
            font.setSize(SkIntToScalar(32));
            canvas->drawSimpleText(text, textLen, SkTextEncoding::kUTF8, 580, 125, font, paint);
        }

        // check perspective
        {
            font.setEdging(SkFont::Edging::kAntiAlias);
            SkAutoCanvasRestore acr(canvas, true);
            SkMatrix persp;
            persp.setAll(0.9839f, 0, 0,
                         0.2246f, 0.6829f, 0,
                         0.0002352f, -0.0003844f, 1);
            canvas->concat(persp);
            canvas->translate(1100, -295);
            font.setSize(37.5f);
            canvas->drawSimpleText(text, textLen, SkTextEncoding::kUTF8, 0, 0, font, paint);
        }
        {
            font.setSubpixel(false);
            font.setEdging(SkFont::Edging::kAlias);
            SkAutoCanvasRestore acr(canvas, true);
            SkMatrix persp;
            persp.setAll(0.9839f, 0, 0,
                         0.2246f, 0.6829f, 0,
                         0.0002352f, -0.0003844f, 1);
            canvas->concat(persp);
            canvas->translate(1075, -245);
            canvas->scale(375, 375);
            font.setSize(0.1f);
            canvas->drawSimpleText(text, textLen, SkTextEncoding::kUTF8, 0, 0, font, paint);
        }

        // check color emoji
        if (fEmojiSample.typeface) {
            SkFont emojiFont;
            emojiFont.setSubpixel(true);
            emojiFont.setTypeface(fEmojiSample.typeface);
            emojiFont.setSize(SkIntToScalar(19));
            canvas->drawSimpleText(fEmojiSample.sampleText,
                                   strlen(fEmojiSample.sampleText),
                                   SkTextEncoding::kUTF8,
                                   670,
                                   90,
                                   emojiFont,
                                   paint);
        }

        // render offscreen buffer
        if (surface) {
            SkAutoCanvasRestore acr(inputCanvas, true);
            // since we prepended this matrix already, we blit using identity
            inputCanvas->resetMatrix();
            inputCanvas->drawImage(surface->makeImageSnapshot().get(), 0, 0);
        }
    }

private:
    ToolUtils::EmojiTestSample fEmojiSample;

    using INHERITED = skiagm::GM;
};

DEF_GM(return new DFTextGM;)
