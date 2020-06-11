/*
 * Copyright 2020 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/svg/model/SkSVGDOM.h"
#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"

void FuzzSVG(sk_sp<SkData> bytes) {
    const uint8_t* data = bytes->bytes();
    const size_t size = bytes->size();
    if (size < 2) {
        return;
    }
    uint8_t w;
    uint8_t h;
    std::memcpy(&w, data, sizeof(w));
    std::memcpy(&h, data, sizeof(w));
    w %= 128;
    h %= 128;

    SkMemoryStream stream(bytes);
    sk_sp<SkSVGDOM> dom = SkSVGDOM::MakeFromStream(stream);
    if (!dom) {
        return;
    }

    auto s = SkSurface::MakeRasterN32Premul(128, 128);
    if (!s) {
        return;
    }
    
    SkSize winSize = SkSize::Make(w, h);
    dom->setContainerSize(winSize);
    dom->render(s->getCanvas());

}

#if defined(IS_FUZZING_WITH_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    auto bytes = SkData::MakeWithoutCopy(data, size);
    FuzzSVG(bytes);
    return 0;
}
#endif