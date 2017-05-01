/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"

static const uint32_t SkCanvas_kDontClipToLayer_PrivateSaveLayerFlag = 1U << 31;

// This GM tests out the deprecated Android-specific unclipped saveLayer "feature".
// In particular, it attempts to compare the performance of unclipped saveLayers with alternatives.

static void save_layer_unclipped(SkCanvas* canvas,
                                 SkScalar l, SkScalar t, SkScalar r, SkScalar b) {
    SkRect rect = SkRect::MakeLTRB(l, t, r, b);
    canvas->saveLayer({ &rect, nullptr, nullptr, nullptr, nullptr,
                        SkCanvas_kDontClipToLayer_PrivateSaveLayerFlag });
}

static void do_draw(SkCanvas* canvas) {
    SkPaint paint;
    SkRandom rand;

    for (int i = 0; i < 20; ++i) {
        paint.setColor(sk_tool_utils::color_to_565(rand.nextU() | (0xFF << 24)));
        canvas->drawRect({ 15, 15, 290, 40 }, paint);
        canvas->translate(0, 30);
    }
}

class UnclippedSaveLayerGM : public skiagm::GM {
public:
    enum class Mode {
        kClipped,
        kUnclipped
    };

    UnclippedSaveLayerGM(Mode mode) : fMode(mode) { this->setBGColor(SK_ColorWHITE); }

protected:
    bool runAsBench() const override { return true; }

    SkString onShortName() override {
        if (Mode::kClipped == fMode) {
            return SkString("savelayer_unclipped");
        } else {
            SkASSERT(Mode::kUnclipped == fMode);
            return SkString("savelayer_clipped");
        }
    }

    SkISize onISize() override { return SkISize::Make(320, 640); }

    void onDraw(SkCanvas* canvas) override {
        const SkScalar L = 10;
        const SkScalar T = 10;
        const SkScalar R = 310;
        const SkScalar B = 630;

        canvas->clipRect({ L, T, R, B });

        for (int i = 0; i < 100; ++i) {
            SkAutoCanvasRestore acr(canvas, true);
            if (Mode::kClipped == fMode) {
                save_layer_unclipped(canvas, L, T, R, T + 20);
                save_layer_unclipped(canvas, L, B - 20, R, B);
            } else {
                SkASSERT(Mode::kUnclipped == fMode);
                canvas->saveLayer({ L, T, R, B }, nullptr);
            }

            do_draw(canvas);
        }
    }

private:
    Mode fMode;

    typedef skiagm::GM INHERITED;
};
DEF_GM(return new UnclippedSaveLayerGM(UnclippedSaveLayerGM::Mode::kClipped);)
DEF_GM(return new UnclippedSaveLayerGM(UnclippedSaveLayerGM::Mode::kUnclipped);)

DEF_SIMPLE_GM(picture_savelayer, canvas, 320, 640) {
    SkPaint paint1, paint2, paint3;
    paint1.setAlpha(0x7f);
    paint2.setAlpha(0x3f);
    paint3.setColor(0xFFFF0000);
    SkRect rect1{40, 5, 80, 70}, rect2{5, 40, 70, 80}, rect3{10, 10, 70, 70};
    // In the future, we might also test the clipped case by allowing i = 0
    for(int i = 1; i < 2; ++i) {
        canvas->translate(100 * i, 0);
        auto flag = i ? SkCanvas_kDontClipToLayer_PrivateSaveLayerFlag : 0;
        canvas->saveLayer({ &rect1, &paint1, nullptr, nullptr, nullptr, flag});
        canvas->saveLayer({ &rect2, &paint2, nullptr, nullptr, nullptr, flag});
        canvas->drawRect(rect3, paint3);
        canvas->restore();
        canvas->restore();
    }
};

#include "Resources.h"

// Test kInitWithPrevious_SaveLayerFlag by drawing an image, save a layer with the flag, which
// should seed the layer with the image (from below). Then we punch a hole in the layer and
// restore with kPlus mode, which should show the mandrill super-bright on the outside, but
// normal where we punched the hole.
DEF_SIMPLE_GM(savelayer_initfromprev, canvas, 256, 256) {
    canvas->drawImage(GetResourceAsImage("mandrill_256.png"), 0, 0, nullptr);

    SkCanvas::SaveLayerRec rec;
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kPlus);
    rec.fSaveLayerFlags = SkCanvas::kInitWithPrevious_SaveLayerFlag;
    rec.fPaint = &paint;
    canvas->saveLayer(rec);
    paint.setBlendMode(SkBlendMode::kClear);
    canvas->drawCircle(128, 128, 96, paint);
    canvas->restore();
};

#include "SkBlurImageFilter.h"
#include "SkGradientShader.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkSurface.h"

static void draw_mask(SkCanvas* canvas, int size) {
    const SkScalar cx = size * SK_ScalarHalf,
                   cy = cx;
    const SkColor colors[] = { 0x00000000, 0xffff0000, 0x00000000, 0xffff0000, 0x00000000,
                               0xffff0000, 0x00000000, 0xffff0000, 0x00000000 };

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setShader(SkGradientShader::MakeSweep(cx, cy, colors, nullptr, SK_ARRAY_COUNT(colors)));
    canvas->drawPaint(paint);

    paint.setShader(SkGradientShader::MakeRadial({cx, cy}, size / 4, colors, nullptr, 2,
                                                 SkShader::kClamp_TileMode));
    canvas->drawCircle(cx, cy, size / 4, paint);
}

DEF_SIMPLE_GM(savelayer_clipmask, canvas, 1200, 1200) {
    static constexpr int kSize = 100;
    static constexpr SkRect kLayerBounds = { kSize * 0.25f, kSize * 0.25f,
                                             kSize * 0.75f, kSize * 0.75f };
    static constexpr struct {
        const SkRect* bounds;
        const SkScalar matrix[9];
    } kConfigs[] = {
        { nullptr, { 1     ,  0     ,   0,   0     , 1     ,   0,   0, 0, 1 } },
        { nullptr, { 2     ,  0     ,   0,   0     , 2     ,   0,   0, 0, 1 } },
        { nullptr, { 2     ,  0     , -50,   0     , 2     , -50,   0, 0, 1 } },
        { nullptr, { 0.707f, -0.707f,  50,   0.707f, 0.707f, -20,   0, 0, 1 } },
        { nullptr, { 0.5f  ,  0     ,  25,   0     , 0.5f  ,  25,   0, 0, 1 } },

        { &kLayerBounds, { 1     ,  0     ,   0,   0     , 1     ,   0,   0, 0, 1 } },
        { &kLayerBounds, { 2     ,  0     ,   0,   0     , 2     ,   0,   0, 0, 1 } },
        { &kLayerBounds, { 2     ,  0     , -50,   0     , 2     , -50,   0, 0, 1 } },
        { &kLayerBounds, { 0.707f, -0.707f,  50,   0.707f, 0.707f, -20,   0, 0, 1 } },
        { &kLayerBounds, { 0.5f  ,  0     ,  25,   0     , 0.5f  ,  25,   0, 0, 1 } },
    };

    using MaskMakerFunc = sk_sp<SkImage> (*)(int size);
    static const MaskMakerFunc kMaskMakers[] = {
        [](int size) -> sk_sp<SkImage> {
            auto surf = SkSurface::MakeRaster(SkImageInfo::MakeA8(size, size));
            draw_mask(surf->getCanvas(), size);
            return surf->makeImageSnapshot();
        },

        [](int size) -> sk_sp<SkImage> {
            auto surf = SkSurface::MakeRasterN32Premul(size, size);
            draw_mask(surf->getCanvas(), size);
            return surf->makeImageSnapshot();
        },

        [](int size) -> sk_sp<SkImage> {
            SkPictureRecorder recorder;
            draw_mask(recorder.beginRecording(size, size), size);
            return SkImage::MakeFromPicture(recorder.finishRecordingAsPicture(),
                                            SkISize::Make(size, size),
                                            nullptr, nullptr,
                                            SkImage::BitDepth::kU8,
                                            SkColorSpace::MakeSRGB());
        }
    };

    using PaintMakerFunc = SkPaint (*)();
    static const PaintMakerFunc kPaintMakers[] = {
        []() -> SkPaint { return SkPaint(); },
        []() -> SkPaint {
            SkPaint p; p.setImageFilter(SkBlurImageFilter::Make(2, 2, nullptr)); return p;
        },
        []() -> SkPaint { SkPaint p; p.setBlendMode(SkBlendMode::kSrcOut); return p; },
    };

    canvas->drawColor(0xffcccccc);

    SkMatrix clipMatrix;
    SkCanvas::SaveLayerRec rec;
    rec.fClipMatrix = &clipMatrix;

    for (const auto& paintMaker : kPaintMakers) {
        auto layerPaint = paintMaker();
        rec.fPaint = &layerPaint;

        for (const auto& maskMaker : kMaskMakers) {
            sk_sp<SkImage> mask = maskMaker(kSize);
            rec.fClipMask = mask.get();

            canvas->save();
            for (const auto cfg : kConfigs) {
                rec.fBounds = cfg.bounds;
                clipMatrix.set9(cfg.matrix);
                canvas->saveLayer(rec);

                SkPaint paint;
                paint.setColor(0xff0000ff);
                canvas->drawRect(SkRect::MakeWH(50, 50), paint);
                paint.setColor(0xffff0000);
                canvas->drawRect(SkRect::MakeXYWH(50, 0, 50, 50), paint);
                paint.setColor(0xff00ff00);
                canvas->drawRect(SkRect::MakeXYWH(50, 50, 50, 50), paint);
                paint.setColor(0xffffff00);
                canvas->drawRect(SkRect::MakeXYWH(0, 50, 50, 50), paint);

                canvas->restore();
                canvas->translate(120, 0);
            }
            canvas->restore();
            canvas->translate(0, 120);
        }
    }
}
