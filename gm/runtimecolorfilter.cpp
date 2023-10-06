/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkVertices.h"
#include "include/effects/SkRuntimeEffect.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"

#include <stddef.h>
#include <utility>

const char* gNoop = R"(
    half4 main(half4 color) {
        return color;
    }
)";

const char* gLumaSrc = R"(
    half4 main(half4 color) {
        return dot(color.rgb, half3(0.3, 0.6, 0.1)).000r;
    }
)";

// Build up the same effect with increasingly complex control flow syntax.
// All of these are semantically equivalent and can be reduced in principle to one basic block.

// Simplest to run; hardest to write?
const char* gTernary = R"(
    half4 main(half4 color) {
        half luma = dot(color.rgb, half3(0.3, 0.6, 0.1));

        half scale = luma < 0.33333 ? 0.5
                   : luma < 0.66666 ? (0.166666 + 2.0 * (luma - 0.33333)) / luma
                   :   /* else */     (0.833333 + 0.5 * (luma - 0.66666)) / luma;
        return half4(color.rgb * scale, color.a);
    }
)";

// Uses conditional if statements but no early return.
const char* gIfs = R"(
    half4 main(half4 color) {
        half luma = dot(color.rgb, half3(0.3, 0.6, 0.1));

        half scale = 0;
        if (luma < 0.33333) {
            scale = 0.5;
        } else if (luma < 0.66666) {
            scale = (0.166666 + 2.0 * (luma - 0.33333)) / luma;
        } else {
            scale = (0.833333 + 0.5 * (luma - 0.66666)) / luma;
        }
        return half4(color.rgb * scale, color.a);
    }
)";

// Distilled from AOSP tone mapping shaders, more like what people tend to write.
const char* gEarlyReturn = R"(
    half4 main(half4 color) {
        half luma = dot(color.rgb, half3(0.3, 0.6, 0.1));

        half scale = 0;
        if (luma < 0.33333) {
            return half4(color.rgb * 0.5, color.a);
        } else if (luma < 0.66666) {
            scale = 0.166666 + 2.0 * (luma - 0.33333);
        } else {
            scale = 0.833333 + 0.5 * (luma - 0.66666);
        }
        return half4(color.rgb * (scale/luma), color.a);
    }
)";

class RuntimeColorFilterGM : public skiagm::GM {
public:
    RuntimeColorFilterGM() = default;

protected:
    SkString getName() const override { return SkString("runtimecolorfilter"); }

    SkISize getISize() override { return SkISize::Make(256 * 3, 256 * 2); }

    void onOnceBeforeDraw() override {
        fImg = ToolUtils::GetResourceAsImage("images/mandrill_256.png");
    }

    void onDraw(SkCanvas* canvas) override {
        auto draw_filter = [&](const char* src) {
            auto [effect, err] = SkRuntimeEffect::MakeForColorFilter(SkString(src));
            if (!effect) {
                SkDebugf("%s\n%s\n", src, err.c_str());
            }
            SkASSERT(effect);
            SkPaint p;
            p.setColorFilter(effect->makeColorFilter(nullptr));
            canvas->drawImage(fImg, 0, 0, SkSamplingOptions(), &p);
            canvas->translate(256, 0);
        };

        for (const char* src : {gNoop, gLumaSrc}) {
            draw_filter(src);
        }
        canvas->translate(-256*2, 256);
        for (const char* src : {gTernary, gIfs, gEarlyReturn}) {
            draw_filter(src);
        }
    }

    sk_sp<SkImage> fImg;
};
DEF_GM(return new RuntimeColorFilterGM;)

DEF_SIMPLE_GM(runtimecolorfilter_vertices_atlas_and_patch, canvas, 404, 404) {
    const SkRect r = SkRect::MakeWH(128, 128);

    // Make a vertices that draws the same as SkRect 'r'.
    SkPoint pos[4];
    r.toQuad(pos);
    constexpr SkColor kColors[] = {SK_ColorBLUE, SK_ColorGREEN, SK_ColorCYAN, SK_ColorYELLOW};
    auto verts = SkVertices::MakeCopy(SkVertices::kTriangleFan_VertexMode, 4, pos, pos, kColors);

    // Make an image from the vertices to do equivalent drawAtlas, drawPatch using an image shader.
    auto info = SkImageInfo::Make({128, 128},
                                  kRGBA_8888_SkColorType,
                                  kPremul_SkAlphaType,
                                  canvas->imageInfo().refColorSpace());
    auto surf = SkSurfaces::Raster(info);
    surf->getCanvas()->drawVertices(verts, SkBlendMode::kDst, SkPaint());
    auto atlas = surf->makeImageSnapshot();
    auto xform = SkRSXform::Make(1, 0, 0, 0);

    // Make a patch that draws the same as the SkRect 'r'
    SkVector vx = pos[1] - pos[0];
    SkVector vy = pos[3] - pos[0];
    vx.setLength(vx.length()/3.f);
    vy.setLength(vy.length()/3.f);
    const SkPoint cubics[12] = {
            pos[0], pos[0] + vx, pos[1] - vx,
            pos[1], pos[1] + vy, pos[2] - vy,
            pos[2], pos[2] - vx, pos[3] + vx,
            pos[3], pos[3] - vy, pos[0] + vy
    };

    auto [effect, err] = SkRuntimeEffect::MakeForColorFilter(SkString(gLumaSrc));
    if (!effect) {
        SkDebugf("%s\n%s\n", gLumaSrc, err.c_str());
    }
    SkASSERT(effect);
    sk_sp<SkColorFilter> colorfilter = effect->makeColorFilter(nullptr);

    auto makePaint = [&](bool useCF, bool useShader) {
        SkPaint paint;
        paint.setColorFilter(useCF ? colorfilter : nullptr);
        paint.setShader(useShader ? atlas->makeShader(SkFilterMode::kNearest) : nullptr);
        return paint;
    };

    auto drawVertices = [&](float x, bool useCF, bool useShader) {
        SkAutoCanvasRestore acr(canvas, /*doSave=*/true);
        canvas->translate(x, 0);
        // Use just the shader or just the vertex colors.
        auto mode = useShader ? SkBlendMode::kSrc : SkBlendMode::kDst;
        canvas->drawVertices(verts, mode, makePaint(useCF, useShader));
    };

    auto drawAtlas = [&](float x, bool useCF) {
        SkAutoCanvasRestore acr(canvas, /*doSave=*/true);
        canvas->translate(x, 0);
        SkPaint paint = makePaint(useCF, /*useShader=*/false);
        constexpr SkColor kColor = SK_ColorWHITE;
        canvas->drawAtlas(atlas.get(),
                          &xform,
                          &r,
                          &kColor,
                          1,
                          SkBlendMode::kModulate,
                          SkFilterMode::kNearest,
                          nullptr,
                          &paint);
    };

    auto drawPatch = [&](float x, bool useCF) {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->translate(x, 0);
        SkPaint paint = makePaint(useCF, /*useShader=*/true);
        canvas->drawPatch(cubics, nullptr, pos, SkBlendMode::kModulate, paint);
    };

    drawVertices(                0,  /*useCF=*/false, /*useShader=*/false);
    drawVertices(   r.width() + 10,  /*useCF=*/ true, /*useShader=*/false);
    drawVertices(2*(r.width() + 10), /*useCF=*/ true, /*useShader=*/ true);

    canvas->translate(0, r.height() + 10);
    drawAtlas(             0, /*useCF=*/false);
    drawAtlas(r.width() + 10, /*useCF=*/ true);

    canvas->translate(0, r.height() + 10);
    drawPatch(             0, /*useCF=*/false);
    drawPatch(r.width() + 10, /*useCF=*/ true);
}
