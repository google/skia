/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/SkottiePriv.h"

#include "include/utils/SkParse.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/sksg/include/SkSGDraw.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "modules/sksg/include/SkSGRect.h"
#include "modules/sksg/include/SkSGRenderNode.h"

namespace skottie {
namespace internal {

sk_sp<sksg::RenderNode> AnimationBuilder::attachSolidLayer(const skjson::ObjectValue& jlayer,
                                                           LayerInfo* layer_info) const {
    layer_info->fSize = SkSize::Make(ParseDefault<float>(jlayer["sw"], 0.0f),
                                     ParseDefault<float>(jlayer["sh"], 0.0f));
    const skjson::StringValue* hex_str = jlayer["sc"];
    uint32_t c;
    if (layer_info->fSize.isEmpty() ||
        !hex_str ||
        *hex_str->begin() != '#' ||
        !SkParse::FindHex(hex_str->begin() + 1, &c)) {
        this->log(Logger::Level::kError, &jlayer, "Could not parse solid layer.");
        return nullptr;
    }

    const SkColor color = 0xff000000 | c;

    auto solid_paint = sksg::Color::Make(color);
    solid_paint->setAntiAlias(true);

    return sksg::Draw::Make(sksg::Rect::Make(SkRect::MakeSize(layer_info->fSize)),
                            std::move(solid_paint));
}

} // namespace internal
} // namespace skottie
