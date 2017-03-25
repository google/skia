/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ok.h"
#include "SkSurface.h"

struct SWDst : Dst {
    SkImageInfo      info;
    sk_sp<SkSurface> surface;

    static std::unique_ptr<Dst> Create(Options options) {
        SkImageInfo info = SkImageInfo::MakeN32Premul(0,0);
        if (options("ct") == "565") { info = info.makeColorType(kRGB_565_SkColorType); }
        if (options("ct") == "f16") { info = info.makeColorType(kRGBA_F16_SkColorType); }

        SWDst dst;
        dst.info = info;
        return move_unique(dst);
    }

    bool draw(Src* src) override {
        auto size = src->size();
        surface = SkSurface::MakeRaster(info.makeWH(size.width(), size.height()));
        return src->draw(surface->getCanvas());
    }

    sk_sp<SkImage> image() override {
        return surface->makeImageSnapshot();
    }
};
static Register sw{"sw", SWDst::Create};
