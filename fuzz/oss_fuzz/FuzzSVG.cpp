/*
 * Copyright 2020 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "modules/svg/include/SkSVGDOM.h"
#include "modules/svg/include/SkSVGNode.h"
#include "tools/fonts/TestFontMgr.h"

#if defined(SK_ENABLE_SVG)

void FuzzSVG(const uint8_t *data, size_t size) {
    uint8_t w = 100;
    uint8_t h = 200;

    SkMemoryStream stream(data, size);
    sk_sp<SkSVGDOM> dom =
            SkSVGDOM::Builder().setFontManager(ToolUtils::MakePortableFontMgr()).make(stream);
    if (!dom) {
        return;
    }

    auto s = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(128, 128));
    if (!s) {
        return;
    }
    SkSize winSize = SkSize::Make(w, h);
    dom->setContainerSize(winSize);
    dom->containerSize();
    dom->render(s->getCanvas());
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 30000) {
        return 0;
    }
    FuzzSVG(data, size);
    return 0;
}
#endif

#endif // SK_ENABLE_SVG
