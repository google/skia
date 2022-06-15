/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Renderer.h"

#include "src/gpu/AtlasTypes.h"
#include "src/gpu/graphite/render/TextDirectRenderStep.h"
#include "src/gpu/graphite/render/TextSDFRenderStep.h"

namespace skgpu::graphite {

const Renderer& Renderer::TextDirect(MaskFormat maskFormat) {
    static const TextDirectRenderStep kMask{false};
    static const TextDirectRenderStep kBitmap{true};

    static const Renderer kTextMaskRenderer{"TextMaskRenderer",
                                            &kMask};
    static const Renderer kTextBitmapRenderer{"TextBitmapRenderer",
                                              &kBitmap};

    switch(maskFormat) {
        case MaskFormat::kA8: return kTextMaskRenderer;
        case MaskFormat::kA565: return kTextBitmapRenderer;
        case MaskFormat::kARGB: return kTextBitmapRenderer;
    }
    SkUNREACHABLE;
}

const Renderer& Renderer::TextSDF(MaskFormat maskFormat) {
    static const TextSDFRenderStep kA8{false};
    static const TextSDFRenderStep k565{true};

    static const Renderer kTextSDFA8Renderer{"TextSDFA8Renderer",
                                             &kA8};
    static const Renderer kTextSDF565Renderer{"TextSDF565Renderer",
                                              &k565};

    switch(maskFormat) {
        case MaskFormat::kA8: return kTextSDFA8Renderer;
        case MaskFormat::kA565: return kTextSDF565Renderer;
            // ARGB is not valid
        case MaskFormat::kARGB: SkUNREACHABLE;
    }
    SkUNREACHABLE;
}

} // namespace skgpu
