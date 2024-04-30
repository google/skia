/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFont.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/base/SkTArray.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <initializer_list>

using namespace skia_private;

/**
 * This GM tests reusing the same text blobs with distance fields rendering using various
 * combinations of perspective and non-perspetive matrices, scissor clips, and different x,y params
 * passed to the draw.
 */
class DFTextBlobPerspGM : public skiagm::GM {
public:
    DFTextBlobPerspGM() { this->setBGColor(0xFFFFFFFF); }

protected:
    SkString getName() const override { return SkString("dftext_blob_persp"); }

    SkISize getISize() override { return SkISize::Make(900, 350); }

    void onOnceBeforeDraw() override {
        for (int i = 0; i < 3; ++i) {
            SkFont font = ToolUtils::DefaultPortableFont();
            font.setSize(32);
            font.setEdging(i == 0 ? SkFont::Edging::kAlias :
                           (i == 1 ? SkFont::Edging::kAntiAlias :
                            SkFont::Edging::kSubpixelAntiAlias));
            font.setSubpixel(true);
            SkTextBlobBuilder builder;
            ToolUtils::add_to_text_blob(&builder, "SkiaText", font, 0, 0);
            fBlobs.emplace_back(builder.make());
        }
    }

    void onDraw(SkCanvas* inputCanvas) override {
        // set up offscreen rendering with distance field text
        auto ctx = inputCanvas->recordingContext();
        SkISize size = this->getISize();
        if (!inputCanvas->getBaseLayerSize().isEmpty()) {
            size = inputCanvas->getBaseLayerSize();
        }
        SkImageInfo info = SkImageInfo::MakeN32(size.width(), size.height(), kPremul_SkAlphaType,
                                                inputCanvas->imageInfo().refColorSpace());
        SkSurfaceProps inputProps;
        inputCanvas->getProps(&inputProps);
        SkSurfaceProps props(SkSurfaceProps::kUseDeviceIndependentFonts_Flag | inputProps.flags(),
                             inputProps.pixelGeometry());
        auto surface = SkSurfaces::RenderTarget(ctx, skgpu::Budgeted::kNo, info, 0, &props);
        SkCanvas* canvas = surface ? surface->getCanvas() : inputCanvas;
        // init our new canvas with the old canvas's matrix
        canvas->setMatrix(inputCanvas->getLocalToDeviceAs3x3());
        SkScalar x = 0, y = 0;
        SkScalar maxH = 0;
        for (auto twm : {TranslateWithMatrix::kNo, TranslateWithMatrix::kYes}) {
            for (auto pm : {PerspMode::kNone, PerspMode::kX, PerspMode::kY, PerspMode::kXY}) {
                for (auto& blob : fBlobs) {
                    for (bool clip : {false, true}) {
                        SkAutoCanvasRestore acr(canvas, true);
                        SkScalar w = blob->bounds().width();
                        SkScalar h = blob->bounds().height();
                        if (clip) {
                            auto rect =
                                    SkRect::MakeXYWH(x + 5, y + 5, w * 3.f / 4.f, h * 3.f / 4.f);
                            canvas->clipRect(rect, false);
                        }
                        this->drawBlob(canvas, blob.get(), SK_ColorBLACK, x, y + h, pm, twm);
                        x += w + 20.f;
                        maxH = std::max(h, maxH);
                    }
                }
                x = 0;
                y += maxH + 20.f;
                maxH = 0;
            }
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
        persp = SkMatrix::Concat(persp, SkMatrix::Translate(-x, -y));
        persp = SkMatrix::Concat(SkMatrix::Translate(x, y), persp);
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

    TArray<sk_sp<SkTextBlob>> fBlobs;
    using INHERITED = skiagm::GM;
};

DEF_GM(return new DFTextBlobPerspGM;)
