/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "Resources.h"
#include "SkCanvas.h"
#include "SkSurface.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"
#include "sk_tool_utils.h"

/**
 * This GM tests reusing the same text blobs with distance fields rendering using various
 * combinations of perspective and non-perspetive matrices, scissor clips, and different x,y params
 * passed to the draw.
 */
class DFTextBlobPerspGM : public skiagm::GM {
public:
    DFTextBlobPerspGM() { this->setBGColor(0xFFFFFFFF); }

protected:
    SkString onShortName() override {
        SkString name("dftext_blob_persp");
        name.append(sk_tool_utils::platform_font_manager());
        return name;
    }

    SkISize onISize() override { return SkISize::Make(900, 350); }

    void onOnceBeforeDraw() override {
        for (int i = 0; i < 3; ++i) {
            SkPaint paint;
            paint.setTextSize(32);
            paint.setAntiAlias(i > 0);
            paint.setLCDRenderText(i > 1);
            paint.setSubpixelText(true);
            SkTextBlobBuilder builder;
            sk_tool_utils::add_to_text_blob(&builder, "SkiaText", paint, 0, 0);
            fBlobs.emplace_back(builder.make());
        }
    }

    virtual void onDraw(SkCanvas* inputCanvas) override {
    // set up offscreen rendering with distance field text
#if SK_SUPPORT_GPU
        GrContext* ctx = inputCanvas->getGrContext();
        SkISize size = this->onISize();
        if (!inputCanvas->getBaseLayerSize().isEmpty()) {
            size = inputCanvas->getBaseLayerSize();
        }
        SkImageInfo info = SkImageInfo::MakeN32(size.width(), size.height(), kPremul_SkAlphaType,
                                                inputCanvas->imageInfo().refColorSpace());
        SkSurfaceProps props(SkSurfaceProps::kUseDeviceIndependentFonts_Flag,
                             SkSurfaceProps::kLegacyFontHost_InitType);
        auto surface = SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info, 0, &props);
        SkCanvas* canvas = surface ? surface->getCanvas() : inputCanvas;
        // init our new canvas with the old canvas's matrix
        canvas->setMatrix(inputCanvas->getTotalMatrix());
#else
        SkCanvas* canvas = inputCanvas;
#endif
        SkScalar x = 0, y = 0;
        SkScalar maxH = 0;
        for (auto twm : {TranslateWithMatrix::kNo, TranslateWithMatrix::kYes}) {
            for (auto pm : {PerspMode::kNone, PerspMode::kX, PerspMode::kY, PerspMode::kXY}) {
                for (auto& blob : fBlobs) {
                    for (bool clip : {false, true}) {
                        canvas->save();
                        SkScalar w = blob->bounds().width();
                        SkScalar h = blob->bounds().height();
                        if (clip) {
                            auto rect =
                                    SkRect::MakeXYWH(x + 5, y + 5, w * 3.f / 4.f, h * 3.f / 4.f);
                            canvas->clipRect(rect, false);
                        }
                        this->drawBlob(canvas, blob.get(), SK_ColorBLACK, x, y + h, pm, twm);
                        x += w + 20.f;
                        maxH = SkTMax(h, maxH);
                        canvas->restore();
                    }
                }
                x = 0;
                y += maxH + 20.f;
                maxH = 0;
            }
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
    enum class PerspMode { kNone, kX, kY, kXY };

    enum class TranslateWithMatrix : bool { kNo, kYes };

    void drawBlob(SkCanvas* canvas, SkTextBlob* blob, SkColor color, SkScalar x, SkScalar y,
                  PerspMode perspMode, TranslateWithMatrix translateWithMatrix) {
        canvas->save();
        SkMatrix persp = SkMatrix::I();
        switch (perspMode) {
            case PerspMode::kNone:
                break;
            case PerspMode::kX:
                persp.setPerspX(0.005f);
                break;
            case PerspMode::kY:
                persp.setPerspY(00.005f);
                break;
            case PerspMode::kXY:
                persp.setPerspX(-0.001f);
                persp.setPerspY(-0.0015f);
                break;
        }
        persp = SkMatrix::Concat(persp, SkMatrix::MakeTrans(-x, -y));
        persp = SkMatrix::Concat(SkMatrix::MakeTrans(x, y), persp);
        canvas->concat(persp);
        if (TranslateWithMatrix::kYes == translateWithMatrix) {
            canvas->translate(x, y);
            x = 0;
            y = 0;
        }
        SkPaint paint;
        paint.setColor(color);
        canvas->drawTextBlob(blob, x, y, paint);
        canvas->restore();
    }

    SkTArray<sk_sp<SkTextBlob>, true> fBlobs;
    typedef skiagm::GM INHERITED;
};

DEF_GM(return new DFTextBlobPerspGM;)
