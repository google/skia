/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkGradient.h"
#include "include/private/base/SkAssert.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkRasterPipelineVizualizer.h"
#include "tools/viewer/ClickHandlerSlide.h"
#include "tools/viewer/Slide.h"

#include <vector>

static constexpr float kPanelSize = 100.f;
static constexpr float kPadding = 10.f;
static constexpr float kBorder = 2.f;
static constexpr float kHandleRadius = 3.f;

static SkBitmap make_panel() {
    SkBitmap panel;
    panel.setInfo(SkImageInfo::Make(kPanelSize, kPanelSize,
                                    kRGBA_8888_SkColorType, kOpaque_SkAlphaType));
    panel.allocPixels();
    panel.eraseColor(SK_ColorBLACK);
    return panel;
}

static SkColor4f rgb(uint8_t r, uint8_t g, uint8_t b) {
    constexpr float scale = 1.0f/255;
    return {r*scale, g*scale, b*scale, 1};
}

class RPVizSlide : public ClickHandlerSlide {
public:
    RPVizSlide() { fName = "RasterPipelineViz"; }

    void draw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorWHITE);

        SkPaint paint;
        paint.setColor(SK_ColorRED);
        // paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 1.0));
        const SkColor4f colors[] = {
                rgb(0xE4, 0x03, 0x03),  // Red
                rgb(0xFF, 0x8C, 0x00),  // Orange
                rgb(0xFF, 0xED, 0x00),  // Yellow
                rgb(0x00, 0x80, 0x26),  // Green
                rgb(0x00, 0x4D, 0xFF),  // Blue
                rgb(0x75, 0x07, 0x87)   // Violet
        };
        SkPoint points[] = {
                {fStartDX, fStartDY},
                {fEndX, fEndY},
        };
        auto shader = SkShaders::LinearGradient(
                points, {{colors, {}, SkTileMode::kClamp}, {}});
        paint.setShader(shader);
        paint.setBlendMode(SkBlendMode::kSrc);

        SkBitmap dst;
        dst.setInfo(SkImageInfo::Make(
                kPanelSize, kPanelSize, kRGBA_8888_SkColorType, kOpaque_SkAlphaType));
        dst.allocPixels();
        dst.eraseColor(SK_ColorBLACK);

        // Suggested flow for customizing this (or making your own)
        // 1) Create a shader of interest and put it into a paint.
        // 2) Use SkRasterPipelineVisualizer::DebugStageBuilder to create an *empty* list of stages.
        // 3) Call SkRasterPipelineVisualizer::CreateBlitter and check stdout for a dump
        //    of the stages (and it will assert).
        // 4) With SkRasterPipeline_opts.h as a reference, add one or more entries for each stage
        //    to pick out the "interesting" lanes.
        // 5) Re-compile and run. Modify stages to taste.
        SkRasterPipelineVisualizer::DebugStageBuilder stageBuilder;
        stageBuilder
                .add(make_panel(), SkRasterPipelineOp::debug_x,
                     make_panel(), SkRasterPipelineOp::debug_y)
                .add(make_panel(), SkRasterPipelineOp::debug_x)
                .add(make_panel(), SkRasterPipelineOp::debug_x)
                .add(make_panel(), SkRasterPipelineOp::debug_r_255,
                     make_panel(), SkRasterPipelineOp::debug_g_255,
                     make_panel(), SkRasterPipelineOp::debug_b_255,
                     make_panel(), SkRasterPipelineOp::debug_a_255);

        std::vector<SkRasterPipelineVisualizer::DebugStage> stages = stageBuilder.build();

        static constexpr size_t kStackMemory = 1024;  // arbitrary
        SkSTArenaAlloc<kStackMemory> alloc;
        auto blitter = SkRasterPipelineVisualizer::CreateBlitter(
                dst.pixmap(), stages, paint, SkMatrix::I(), &alloc, nullptr, {});

        blitter->blitRect(0, 0, kPanelSize, kPanelSize);

        SkPaint panelBackdrop;
        panelBackdrop.setColor(SK_ColorGRAY);
        panelBackdrop.setStyle(SkPaint::Style::kFill_Style);

        for (size_t row = 0; row < stages.size(); row++) {
            for (size_t col = 0; col < stages[row].panels.size(); col++) {
                float x = (kPanelSize + kPadding) * col + kPadding;
                float y = (kPanelSize + kPadding) * row + kPadding;
                canvas->drawRect(SkRect::MakeXYWH(x - kBorder,
                                                  y - kBorder,
                                                  kPanelSize + 2 * kBorder,
                                                  kPanelSize + 2 * kBorder),
                                 panelBackdrop);
                auto panelImg = SkImages::RasterFromBitmap(stages[row].panels[col]);
                canvas->drawImage(panelImg, x, y);
            }
        }

        // Draw the output
        auto dstImg = SkImages::RasterFromBitmap(dst);
        fOutputCornerX = kPadding;
        fOutputCornerY = (kPanelSize + kPadding) * stages.size() + kPadding;
        canvas->drawImage(dstImg, fOutputCornerX, fOutputCornerY);

        // Draw the handles
        SkPaint handlePaint;
        handlePaint.setAntiAlias(true);
        handlePaint.setStrokeWidth(2);
        handlePaint.setStyle(SkPaint::Style::kStroke_Style);

        canvas->drawCircle(
                fOutputCornerX + fStartDX, fOutputCornerY + fStartDY, kHandleRadius, handlePaint);
        canvas->drawCircle(
                fOutputCornerX + fEndX, fOutputCornerY + fEndY, kHandleRadius, handlePaint);
    }

private:
    class Click : public ClickHandlerSlide::Click {
    public:
        Click(float* x, float* y) : fX(x), fY(y) {}

        void doClick(RPVizSlide* that) {
            float newX = SkTPin(fCurr.fX - that->fOutputCornerX, 0.f, kPanelSize);
            float newY = SkTPin(fCurr.fY - that->fOutputCornerY, 0.f, kPanelSize);
            *fX = newX;
            *fY = newY;
        }

    private:
        float* fX;
        float* fY;
    };

    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey) override {
        SkPoint start{fOutputCornerX + fStartDX, fOutputCornerY + fStartDY};
        SkPoint end{fOutputCornerX + fEndX, fOutputCornerY + fEndY};

        SkPoint click{x, y};

        if (SkPoint::Distance(start, click) < SkPoint::Distance(end, click)) {
            return new Click(&fStartDX, &fStartDY);
        }
        return new Click(&fEndX, &fEndY);
    }

    bool onClick(ClickHandlerSlide::Click* click) override {
        Click* myClick = (Click*)click;
        myClick->doClick(this);
        return true;
    }

    float fOutputCornerX = 0.f;
    float fOutputCornerY = 0.f;

    float fStartDX = 5.f;
    float fStartDY = 5.f;
    float fEndX = 95.f;
    float fEndY = 95.f;
};

DEF_SLIDE(return new RPVizSlide();)
