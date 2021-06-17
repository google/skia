/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlender.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/utils/SkTextUtils.h"
#include "tools/ToolUtils.h"

#include <initializer_list>
#include <unordered_map>

namespace skiagm {

// This GM recreates the blend mode images from the Android documentation.
class AndroidBlendModesGM : public GM {
public:
    AndroidBlendModesGM() {
        this->setBGColor(SK_ColorBLACK);
    }

protected:
    virtual void onSetBlend(SkPaint* paint, SkBlendMode blend) = 0;

    SkISize onISize() override {
        return SkISize::Make(kNumCols * kBitmapSize, kNumRows * kBitmapSize);
    }

    void onOnceBeforeDraw() override {
        SkImageInfo ii = SkImageInfo::MakeN32Premul(kBitmapSize, kBitmapSize);
        {
            fCompositeSrc.allocPixels(ii);
            SkCanvas tmp(fCompositeSrc);
            tmp.clear(SK_ColorTRANSPARENT);
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(ToolUtils::color_to_565(kBlue));
            tmp.drawRect(SkRect::MakeLTRB(16, 96, 160, 240), p);
        }

        {
            fCompositeDst.allocPixels(ii);
            SkCanvas tmp(fCompositeDst);
            tmp.clear(SK_ColorTRANSPARENT);
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(ToolUtils::color_to_565(kRed));
            tmp.drawCircle(160, 95, 80, p);
        }
    }

    void drawTile(SkCanvas* canvas, int xOffset, int yOffset, SkBlendMode mode) {
        canvas->translate(xOffset, yOffset);

        canvas->clipRect(SkRect::MakeXYWH(0, 0, 256, 256));

        canvas->saveLayer(nullptr, nullptr);

        SkPaint p;
        canvas->drawImage(fCompositeDst.asImage(), 0, 0, SkSamplingOptions(), &p);

        this->onSetBlend(&p, mode);

        canvas->drawImage(fCompositeSrc.asImage(), 0, 0, SkSamplingOptions(), &p);
    }

    void onDraw(SkCanvas* canvas) override {
        SkFont font(ToolUtils::create_portable_typeface());

        ToolUtils::draw_checkerboard(canvas, kWhite, kGrey, 32);

        int xOffset = 0, yOffset = 0;

        // Android doesn't expose all the blend modes
        // Note that the Android documentation calls:
        //    Skia's kPlus,     add
        //    Skia's kModulate, multiply
        for (SkBlendMode mode : { SkBlendMode::kPlus /* add */, SkBlendMode::kClear,
                                  SkBlendMode::kDarken, SkBlendMode::kDst,
                                  SkBlendMode::kDstATop, SkBlendMode::kDstIn,
                                  SkBlendMode::kDstOut, SkBlendMode::kDstOver,
                                  SkBlendMode::kLighten, SkBlendMode::kModulate /* multiply */,
                                  SkBlendMode::kOverlay, SkBlendMode::kScreen,
                                  SkBlendMode::kSrc, SkBlendMode::kSrcATop,
                                  SkBlendMode::kSrcIn, SkBlendMode::kSrcOut,
                                  SkBlendMode::kSrcOver, SkBlendMode::kXor } ) {

            int saveCount = canvas->save();
            this->drawTile(canvas, xOffset, yOffset, mode);
            canvas->restoreToCount(saveCount);

            SkTextUtils::DrawString(canvas, SkBlendMode_Name(mode),
                               xOffset + kBitmapSize/2.0f,
                               yOffset + kBitmapSize,
                               font, SkPaint(), SkTextUtils::kCenter_Align);

            xOffset += 256;
            if (xOffset >= 1024) {
                xOffset = 0;
                yOffset += 256;
            }
        }
    }

private:
    static const int kBitmapSize = 256;
    static const int kNumRows = 5;
    static const int kNumCols = 4;

    static const SkColor  kBlue  = SkColorSetARGB(255, 22, 150, 243);
    static const SkColor  kRed   = SkColorSetARGB(255, 233, 30, 99);
    static const SkColor  kWhite = SkColorSetARGB(255, 243, 243, 243);
    static const SkColor  kGrey  = SkColorSetARGB(255, 222, 222, 222);

    SkBitmap fCompositeSrc;
    SkBitmap fCompositeDst;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

// This GM recreates the blend mode images from the Android documentation using SkBlendMode.
class NativeAndroidBlendModesGM : public AndroidBlendModesGM {
public:
    SkString onShortName() override {
        return SkString("androidblendmodes");
    }

    void onSetBlend(SkPaint* paint, SkBlendMode blend) override {
        paint->setBlendMode(blend);
    }
};
//////////////////////////////////////////////////////////////////////////////

// This GM recreates the blend mode images from the Android documentation using Runtime Blending.
class RuntimeAndroidBlendModesGM : public AndroidBlendModesGM {
public:
    SkString onShortName() override {
        return SkString("runtime_androidblendmodes");
    }

    sk_sp<SkBlender> getRuntimeBlend(SkBlendMode mode) {
        if (sk_sp<SkBlender> effect = fBlendMap[mode]) {
            return effect;
        }

        const char* func;
        switch (mode) {
            case SkBlendMode::kPlus:     func = "blend_plus"; break;
            case SkBlendMode::kClear:    func = "blend_clear"; break;
            case SkBlendMode::kDarken:   func = "blend_darken"; break;
            case SkBlendMode::kDst:      func = "blend_dst"; break;
            case SkBlendMode::kDstATop:  func = "blend_dst_atop"; break;
            case SkBlendMode::kDstIn:    func = "blend_dst_in"; break;
            case SkBlendMode::kDstOut:   func = "blend_dst_out"; break;
            case SkBlendMode::kDstOver:  func = "blend_dst_over"; break;
            case SkBlendMode::kLighten:  func = "blend_lighten"; break;
            case SkBlendMode::kModulate: func = "blend_modulate"; break;
            case SkBlendMode::kOverlay:  func = "blend_overlay"; break;
            case SkBlendMode::kScreen:   func = "blend_screen"; break;
            case SkBlendMode::kSrc:      func = "blend_src"; break;
            case SkBlendMode::kSrcATop:  func = "blend_src_atop"; break;
            case SkBlendMode::kSrcIn:    func = "blend_src_in"; break;
            case SkBlendMode::kSrcOut:   func = "blend_src_out"; break;
            case SkBlendMode::kSrcOver:  func = "blend_src_over"; break;
            case SkBlendMode::kXor:      func = "blend_xor"; break;
            default:                     func = "invalid"; break;
        }

        constexpr char kShaderText[] = R"(
half4 blend_clear(half4 src, half4 dst) { return half4(0); }

half4 blend_src(half4 src, half4 dst) { return src; }

half4 blend_dst(half4 src, half4 dst) { return dst; }

half4 blend_src_over(half4 src, half4 dst) { return src + (1 - src.a)*dst; }

half4 blend_dst_over(half4 src, half4 dst) { return (1 - dst.a)*src + dst; }

half4 blend_src_in(half4 src, half4 dst) { return (src == half4(0) ? half4(0) : src*dst.a); }

half4 blend_dst_in(half4 src, half4 dst) { return blend_src_in(dst, src); }

half4 blend_src_out(half4 src, half4 dst) { return (1 - dst.a)*src; }

half4 blend_dst_out(half4 src, half4 dst) { return (1 - src.a)*dst; }

half4 blend_src_atop(half4 src, half4 dst) { return dst.a*src + (1 - src.a)*dst; }

half4 blend_dst_atop(half4 src, half4 dst)  { return  (1 - dst.a) * src + src.a*dst; }

half4 blend_xor(half4 src, half4 dst) { return (1 - dst.a)*src + (1 - src.a)*dst; }

half4 blend_plus(half4 src, half4 dst) { return min(src + dst, 1); }

half4 blend_modulate(half4 src, half4 dst) { return src*dst; }

half4 blend_screen(half4 src, half4 dst) { return src + (1 - src)*dst; }

half _blend_overlay_component(half2 s, half2 d) {
    return (2*d.x <= d.y)
            ? 2*s.x*d.x
            : s.y*d.y - 2*(d.y - d.x)*(s.y - s.x);
}

half4 blend_overlay(half4 src, half4 dst) {
    half4 result = half4(_blend_overlay_component(src.ra, dst.ra),
                         _blend_overlay_component(src.ga, dst.ga),
                         _blend_overlay_component(src.ba, dst.ba),
                         src.a + (1 - src.a)*dst.a);
    result.rgb += dst.rgb*(1 - src.a) + src.rgb*(1 - dst.a);
    return result;
}

half4 blend_darken(half4 src, half4 dst) {
    half4 result = blend_src_over(src, dst);
    result.rgb = min(result.rgb, (1 - dst.a)*src.rgb + dst.rgb);
    return result;
}

half4 blend_lighten(half4 src, half4 dst) {
    half4 result = blend_src_over(src, dst);
    result.rgb = max(result.rgb, (1 - dst.a)*src.rgb + dst.rgb);
    return result;
}

half4 main(half4 src, half4 dst) {
    return %s(src, dst);
}
)";
        SkString code = SkStringPrintf(kShaderText, func);
        SkRuntimeEffect::Result result = SkRuntimeEffect::MakeForBlender(std::move(code));
        SkASSERT(result.effect);
        return (fBlendMap[mode] = result.effect->makeBlender(/*uniforms=*/nullptr));
    }

    void onSetBlend(SkPaint* paint, SkBlendMode blend) override {
        paint->experimental_setBlender(this->getRuntimeBlend(blend));
    }

private:
    std::unordered_map<SkBlendMode, sk_sp<SkBlender>> fBlendMap;
};

DEF_GM(return new NativeAndroidBlendModesGM;)
DEF_GM(return new RuntimeAndroidBlendModesGM;)
}  // namespace skiagm
