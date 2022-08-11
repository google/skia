/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Renderer.h"

#include "src/gpu/AtlasTypes.h"
#include "src/gpu/graphite/render/BitmapTextRenderStep.h"
#include "src/gpu/graphite/render/SDFTextRenderStep.h"

namespace skgpu::graphite {

const Renderer& Renderer::TextDirect(bool isA8) {
    static const BitmapTextRenderStep kDirectA8{true};
    static const BitmapTextRenderStep kDirectColor{false};

    static const Renderer kTextDirectA8Renderer{"TextDirectA8Renderer",
                                                &kDirectA8};
    static const Renderer kTextDirectColorRenderer{"TextDirectColorRenderer",
                                                   &kDirectColor};
    if (isA8) {
        return kTextDirectA8Renderer;
    } else {
        return kTextDirectColorRenderer;
    }
}

const Renderer& Renderer::TextSDF(bool useLCDText) {
    static const SDFTextRenderStep kA8{false};
    static const SDFTextRenderStep kLCD{true};

    static const Renderer kTextSDFA8Renderer{"TextSDFA8Renderer",
                                             &kA8};
    static const Renderer kTextSDFLCDRenderer{"TextSDFLCDRenderer",
                                              &kLCD};

    if (useLCDText) {
        return kTextSDFLCDRenderer;
    } else {
        return kTextSDFA8Renderer;
    }
    SkUNREACHABLE;
}

} // namespace skgpu
