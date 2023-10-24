/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "tools/DecodeUtils.h"
#include "tools/GpuToolUtils.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#endif

namespace skiagm {

class GraphiteReplayGM : public GM {
public:
    GraphiteReplayGM() = default;

protected:
    void onOnceBeforeDraw() override {
        this->setBGColor(SK_ColorBLACK);
        fImage = ToolUtils::GetResourceAsImage("images/mandrill_128.png");
    }

    SkString getName() const override { return SkString("graphite-replay"); }

    SkISize getISize() override { return SkISize::Make(kTileWidth * 3, kTileHeight * 2); }

    bool onAnimate(double nanos) override {
        fStartX = kTileWidth * (1.0f + sinf(nanos * 1e-9)) * 0.5f;
        return true;
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
#if defined(SK_GRAPHITE)
        skgpu::graphite::Recorder* recorder = canvas->recorder();
        if (recorder) {
            this->drawGraphite(canvas, recorder);
            return DrawResult::kOk;
        }
#endif
        return this->drawNonGraphite(canvas, errorMsg);
    }

private:
    static constexpr int kImageSize = 128;
    static constexpr int kPadding = 2;
    static constexpr int kPaddedImageSize = kImageSize + kPadding * 2;
    static constexpr int kTileWidth = kPaddedImageSize * 2;
    static constexpr int kTileHeight = kPaddedImageSize * 2;

    float fStartX = 0.0f;

    sk_sp<SkImage> fImage;

    void drawContent(SkCanvas* canvas, int y) {
        SkPaint gradientPaint;
        constexpr SkPoint points[2] = {{0.0f, 0.0f}, {kImageSize, kImageSize}};
        constexpr SkColor colors[4] = {SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorRED};
        gradientPaint.setShader(SkGradientShader::MakeLinear(
                points, colors, nullptr, std::size(colors), SkTileMode::kClamp));

        // Draw image.
        canvas->drawImage(fImage, kPadding, kPadding + y);

        // Draw gradient.
        canvas->save();
        canvas->translate(kPaddedImageSize + kPadding, kPadding + y);
        canvas->drawRect(SkRect::MakeXYWH(0, 0, kImageSize, kImageSize), gradientPaint);
        canvas->restore();
    }

    void drawTile(SkCanvas* canvas) {
        // Clip off the right 1/4 of the tile, after clearing.
        canvas->clear(SkColors::kRed);
        canvas->clipIRect(SkIRect::MakeWH(3 * kTileWidth / 4, kTileHeight));

        // Draw content directly.
        drawContent(canvas, 0);

        // Draw content to a saved layer.
        SkPaint pAlpha;
        pAlpha.setAlphaf(0.5f);
        canvas->saveLayer(nullptr, &pAlpha);
        drawContent(canvas, kPaddedImageSize);
        canvas->restore();
    }

#if defined(SK_GRAPHITE)
    void drawGraphite(SkCanvas* canvas, skgpu::graphite::Recorder* canvasRecorder) {
        SkImageInfo tileImageInfo =
                canvas->imageInfo().makeDimensions(SkISize::Make(kTileWidth, kTileHeight));
        skgpu::graphite::TextureInfo textureInfo =
                static_cast<skgpu::graphite::Surface*>(canvas->getSurface())
                        ->backingTextureProxy()
                        ->textureInfo();

        skgpu::graphite::Context* context = canvasRecorder->priv().context();
        std::unique_ptr<skgpu::graphite::Recorder> recorder =
                context->makeRecorder(ToolUtils::CreateTestingRecorderOptions());
        SkCanvas* recordingCanvas = recorder->makeDeferredCanvas(tileImageInfo, textureInfo);
        this->drawTile(recordingCanvas);
        std::unique_ptr<skgpu::graphite::Recording> recording = recorder->snap();

        // Flush the initial clear added by MakeGraphite.
        std::unique_ptr<skgpu::graphite::Recording> canvasRecording = canvasRecorder->snap();
        context->insertRecording({canvasRecording.get()});

        for (int y = 0; y < 2; ++y) {
            for (int x = 0; x < 2; ++x) {
                context->insertRecording(
                        {recording.get(),
                         canvas->getSurface(),
                         {x * kTileWidth + SkScalarRoundToInt(fStartX), y * kTileHeight}});
            }
        }
    }
#endif

    DrawResult drawNonGraphite(SkCanvas* canvas, SkString* errorMsg) {
        SkImageInfo tileImageInfo =
                canvas->imageInfo().makeDimensions(SkISize::Make(kTileWidth, kTileHeight));

        sk_sp<SkSurface> imageSurface = canvas->makeSurface(tileImageInfo);
        if (!imageSurface) {
            *errorMsg = "Cannot create new SkSurface.";
            return DrawResult::kSkip;
        }

        SkCanvas* imageCanvas = imageSurface->getCanvas();
        this->drawTile(imageCanvas);
        sk_sp<SkImage> image = imageSurface->makeImageSnapshot();

        for (int y = 0; y < 2; ++y) {
            for (int x = 0; x < 2; ++x) {
                canvas->drawImage(image, x * kTileWidth + fStartX, y * kTileHeight);
            }
        }
        return DrawResult::kOk;
    }
};

DEF_GM(return new GraphiteReplayGM;)

}  // namespace skiagm
