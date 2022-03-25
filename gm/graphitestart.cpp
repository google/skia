/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GrRecordingContext.h"
#include "tools/Resources.h"

namespace {

class MetaContext {
public:
    MetaContext(SkCanvas* canvas) {
#ifdef SK_GRAPHITE_ENABLED
        fRecorder = canvas->recorder();
#endif
        fContext = canvas->recordingContext() ? canvas->recordingContext()->asDirectContext()
                                              : nullptr;
    }

    sk_sp<SkImage> makeTextureImage(sk_sp<SkImage> orig) const {
#ifdef SK_GRAPHITE_ENABLED
        if (fRecorder) {
            return orig->makeTextureImage(fRecorder);
        }
#endif

        if (fContext) {
            return orig->makeTextureImage(fContext);
        }

        return orig;
    }

private:
#ifdef SK_GRAPHITE_ENABLED
    skgpu::Recorder* fRecorder;
#endif

    GrDirectContext* fContext;
};

sk_sp<SkShader> create_gradient_shader(SkRect r) {
    // TODO: it seems like only the x-component of sk_FragCoord is making it to the shader!
    SkPoint pts[2] = { {r.fLeft, r.fTop}, {r.fRight, r.fTop} };
    SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
    float offsets[] = { 0.0f, 0.75f, 1.0f };

    return SkGradientShader::MakeLinear(pts, colors, offsets, SK_ARRAY_COUNT(colors),
                                        SkTileMode::kClamp);
}

sk_sp<SkShader> create_image_shader(const MetaContext& context) {
    SkImageInfo ii = SkImageInfo::Make(128, 128, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkBitmap bitmap;

    bitmap.allocPixels(ii);
    bitmap.eraseColor(SK_ColorWHITE);

    SkCanvas canvas(bitmap);

    SkColor colors[3][3] = {
            { SK_ColorRED,    SK_ColorDKGRAY, SK_ColorBLUE },
            { SK_ColorLTGRAY, SK_ColorCYAN,   SK_ColorYELLOW },
            { SK_ColorGREEN,  SK_ColorWHITE,  SK_ColorMAGENTA }
    };

    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            SkPaint paint;
            paint.setColor(colors[y][x]);
            canvas.drawRect(SkRect::MakeXYWH(x*42, y*42, 43, 43), paint);
        }
    }

    bitmap.setAlphaType(kOpaque_SkAlphaType);
    bitmap.setImmutable();

    sk_sp<SkImage> img = SkImage::MakeFromBitmap(bitmap);
    img = context.makeTextureImage(std::move(img));

    return img->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, SkSamplingOptions());
}

sk_sp<SkShader> create_blend_shader(const MetaContext& context, SkBlendMode bm) {
    constexpr SkColor4f kTransYellow = {1.0f, 1.0f, 0.0f, 0.5f};

    sk_sp<SkShader> dst = SkShaders::Color(kTransYellow, nullptr);
    return SkShaders::Blend(bm, std::move(dst), create_image_shader(context));
}

void draw_blend_mode_swatches(SkCanvas* canvas, SkRect clipRect) {
    static const int kTileHeight = 16;
    static const int kTileWidth = 16;
    static const SkColor4f kOpaqueWhite { 1.0f, 1.0f, 1.0f, 1.0f };
    static const SkColor4f kTransBluish { 0.0f, 0.5f, 1.0f, 0.5f };
    static const SkColor4f kTransWhite { 1.0f, 1.0f, 1.0f, 0.75f };

    SkPaint dstPaint;
    dstPaint.setColor(kOpaqueWhite);
    dstPaint.setBlendMode(SkBlendMode::kSrc);
    dstPaint.setAntiAlias(false);

    SkPaint srcPaint;
    srcPaint.setColor(kTransBluish);
    srcPaint.setAntiAlias(false);

    SkRect r = SkRect::MakeXYWH(clipRect.fLeft, clipRect.fTop, kTileWidth, kTileHeight);

    // For the first pass we draw: transparent bluish on top of opaque white
    // For the second pass we draw: transparent white on top of transparent bluish
    for (int passes = 0; passes < 2; ++passes) {
        for (int i = 0; i <= (int)SkBlendMode::kLastCoeffMode; ++i) {
            if (r.fLeft+kTileWidth > clipRect.fRight) {
                r.offsetTo(clipRect.fLeft, r.fTop+kTileHeight);
            }

            canvas->drawRect(r.makeInset(1.0f, 1.0f), dstPaint);
            srcPaint.setBlendMode(static_cast<SkBlendMode>(i));
            canvas->drawRect(r.makeInset(2.0f, 2.0f), srcPaint);

            r.offset(kTileWidth, 0.0f);
        }

        r.offsetTo(clipRect.fLeft, r.fTop+kTileHeight);
        srcPaint.setColor(kTransWhite);
        dstPaint.setColor(kTransBluish);
    }
}

} // anonymous namespace

namespace skiagm {

// This is just for bootstrapping Graphite.
class GraphiteStartGM : public GM {
public:
    GraphiteStartGM() {
        this->setBGColor(SK_ColorBLACK);
        GetResourceAsBitmap("images/color_wheel.gif", &fBitmap);
    }

protected:
    static const int kWidth = 256;
    static const int kHeight = 384;

    SkString onShortName() override {
        return SkString("graphitestart");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onDraw(SkCanvas* canvas) override {

        MetaContext context(canvas);

        // UL corner
        {
            SkPaint p;
            p.setShader(create_image_shader(context));

            SkPath path;
            path.moveTo(1,   1);
            path.lineTo(64,  127);
            path.lineTo(127, 1);
            path.lineTo(63,  63);
            path.close();

            canvas->drawPath(path, p);
        }

        // UR corner
        {
            SkRect r{129, 2, 255, 127};
            SkPaint p;
            p.setShader(create_gradient_shader(r));
            canvas->drawRect(r, p);
        }

        // LL corner
        {
            SkPaint p;
            p.setColor(SK_ColorRED);
            canvas->drawRect({2, 129, 127, 255}, p);
        }

        // LR corner
        {
            SkPaint p;
            p.setShader(create_blend_shader(context, SkBlendMode::kModulate));
            canvas->drawRect({129, 129, 255, 255}, p);
        }

#ifdef SK_GRAPHITE_ENABLED
        // TODO: failing serialize test on Linux, not sure what's going on
        canvas->writePixels(fBitmap, 0, 256);
#endif

        draw_blend_mode_swatches(canvas, SkRect::MakeXYWH(128, 256, 128, 128));
    }

private:
    SkBitmap fBitmap;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new GraphiteStartGM;)

}  // namespace skiagm
