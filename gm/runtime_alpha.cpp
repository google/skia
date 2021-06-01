#include "gm/gm.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkShader.h"
#include "include/effects/SkRuntimeEffect.h"

static void draw_with_runtime(SkBitmap& bm, SkCanvas* canvas) {
    bm.setAlphaType(kPremul_SkAlphaType);
    auto shader = bm.asImage()->makeShader(SkTileMode::kClamp, SkTileMode::kClamp,
            SkSamplingOptions({SkFilterMode::kLinear, SkMipmapMode::kNone}));
    SkString sksl(R"(
        uniform shader child;
        uniform half alpha;
        half4 main(float2 pos) {
            half4 c = sample(child, pos);
            c.a = alpha;
            return c;
        }
    )");
    auto [effect, error] = SkRuntimeEffect::Make(sksl);
    if (!effect) {
        SkDebugf("Runtime shader error '%s'\n", error.c_str());
        return;
    }

    SkRuntimeShaderBuilder builder(effect);
    builder.child("child") = shader;
    builder.uniform("alpha") = 1.0f;
    shader = builder.makeShader(nullptr, true);
    SkPaint paint;
    paint.setShader(shader);
    canvas->drawRect(SkRect::MakeXYWH(0, 0, bm.width(), bm.height()), paint);
}

DEF_SIMPLE_GM(unpremul2, canvas, 1000, 1000) {
    ToolUtils::draw_checkerboard(canvas);

    const char* filename = "images/actionRPG.png";
    auto data = GetResourceAsData(filename);
    if (!data || data->isEmpty()) {
        SkDebugf("failed to get %s\n", filename);
        return;
    }

    auto codec = SkCodec::MakeFromData(data);
    if (!codec) {
        SkDebugf("failed to make codec for %s\n", filename);
        return;
    }

    auto info = codec->getInfo().makeAlphaType(kUnpremul_SkAlphaType);
    SkBitmap bm;
    bm.allocPixels(info);

    if (auto result = codec->getPixels(bm.pixmap()); result != SkCodec::kSuccess) {
        SkDebugf("Failed to decode %s with error %s\n", filename, SkCodec::ResultToString(result));
        return;
    }
    draw_with_runtime(bm, canvas);
}
