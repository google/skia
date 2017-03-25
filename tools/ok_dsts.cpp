/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ok.h"
#include "SkSurface.h"

struct SWDst : Dst {
    sk_sp<SkSurface> surface;

    static std::unique_ptr<Dst> Create(Options options, SkISize size) {
        SkImageInfo info = SkImageInfo::MakeN32Premul(size.width(), size.height());
        if (options("ct") == "565") { info = info.makeColorType(kRGB_565_SkColorType); }
        if (options("ct") == "f16") { info = info.makeColorType(kRGBA_F16_SkColorType); }
        SWDst dst;
        dst.surface = SkSurface::MakeRaster(info);
        return move_unique(dst);
    }

    SkCanvas* canvas() override {
        return surface->getCanvas();
    }

    void write(std::string path_prefix) override {
        auto image = surface->makeImageSnapshot();
        sk_sp<SkData> png{image->encode()};
        SkFILEWStream{(path_prefix + ".png").c_str()}.write(png->data(), png->size());
    }
};
static Register sw{"sw", SWDst::Create};
