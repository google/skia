/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlender.h"
#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/sksg/include/SkSGRenderEffect.h"
#include "modules/sksg/include/SkSGRenderNode.h"
#include "src/utils/SkJSON.h"

#include <array>
#include <cstddef>
#include <utility>

namespace skottie::internal {

namespace {

enum CustomBlenders {
    HARDMIX = 17,
};

static sk_sp<SkBlender> hardMix() {
    static SkRuntimeEffect* hardMixEffect = []{
        const char hardMix[] =
            "half4 main(half4 src, half4 dst) {"
                "src.rgb = unpremul(src).rgb + unpremul(dst).rgb;"
                "src.rgb = min(floor(src.rgb), 1) * src.a;"

                "return src + (1 - src.a)*dst;"
            "}"
        ;
        auto result = SkRuntimeEffect::MakeForBlender(SkString(hardMix));
        return result.effect.release();
    }();
    return hardMixEffect->makeBlender(nullptr);
}

static sk_sp<SkBlender> get_blender(const skjson::ObjectValue& jobject,
                                    const AnimationBuilder* abuilder) {
    static constexpr SkBlendMode kBlendModeMap[] = {
        SkBlendMode::kSrcOver,    // 0:'normal'
        SkBlendMode::kMultiply,   // 1:'multiply'
        SkBlendMode::kScreen,     // 2:'screen'
        SkBlendMode::kOverlay,    // 3:'overlay
        SkBlendMode::kDarken,     // 4:'darken'
        SkBlendMode::kLighten,    // 5:'lighten'
        SkBlendMode::kColorDodge, // 6:'color-dodge'
        SkBlendMode::kColorBurn,  // 7:'color-burn'
        SkBlendMode::kHardLight,  // 8:'hard-light'
        SkBlendMode::kSoftLight,  // 9:'soft-light'
        SkBlendMode::kDifference, // 10:'difference'
        SkBlendMode::kExclusion,  // 11:'exclusion'
        SkBlendMode::kHue,        // 12:'hue'
        SkBlendMode::kSaturation, // 13:'saturation'
        SkBlendMode::kColor,      // 14:'color'
        SkBlendMode::kLuminosity, // 15:'luminosity'
        SkBlendMode::kPlus,       // 16:'add'
    };

    const size_t mode = ParseDefault<size_t>(jobject["bm"], 0);

    // Special handling of src-over, so we can detect the trivial/no-fancy-blending case
    // (a null blender is equivalent to src-over).
    if (!mode) {
        return nullptr;
    }

    // Modes that are expressible as SkBlendMode.
    if (mode < std::size(kBlendModeMap)) {
        return SkBlender::Mode(kBlendModeMap[mode]);
    }

    // Modes that require custom blenders.
    switch (mode)
    {
    case HARDMIX:
        return hardMix();
    default:
        break;
    }

    abuilder->log(Logger::Level::kWarning, &jobject, "Unsupported blend mode %zu\n", mode);
    return nullptr;
}

}  // namespace

sk_sp<sksg::RenderNode> AnimationBuilder::attachBlendMode(const skjson::ObjectValue& jobject,
                                                          sk_sp<sksg::RenderNode> child) const {
    if (auto blender = get_blender(jobject, this)) {
        fHasNontrivialBlending = true;
        child = sksg::BlenderEffect::Make(std::move(child), std::move(blender));
    }

    return child;
}

}  // namespace skottie::internal
