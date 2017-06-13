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

        if (options("cs") == "srgb") {
            auto cs = info.colorType() == kRGBA_F16_SkColorType ? SkColorSpace::MakeSRGBLinear()
                                                                : SkColorSpace::MakeSRGB();
            info = info.makeColorSpace(std::move(cs));
        }

        SWDst dst;
        dst.info = info;
        return move_unique(dst);
    }

    Status draw(Src* src) override {
        auto size = src->size();
        surface = SkSurface::MakeRaster(info.makeWH(size.width(), size.height()));
        return src->draw(surface->getCanvas());
    }

    sk_sp<SkImage> image() override {
        return surface->makeImageSnapshot();
    }
};
static Register sw{"sw", "draw with the software backend", SWDst::Create};
static Register _8888{"8888", "alias for sw", SWDst::Create};

static Register _565{"565", "alias for sw:ct=565", [](Options options) {
    options["ct"] = "565";
    return SWDst::Create(options);
}};

static Register srgb{"srgb", "alias for sw:cs=srgb", [](Options options) {
    options["cs"] = "srgb";
    return SWDst::Create(options);
}};

static Register f16{"f16", "alias for sw:ct=f16,cs=srgb", [](Options options) {
    options["ct"] = "f16";
    options["cs"] = "srgb";
    return SWDst::Create(options);
}};

extern bool gSkForceRasterPipelineBlitter;
static Register rp{"rp", "draw forcing SkRasterPipelineBlitter", [](Options options) {
    gSkForceRasterPipelineBlitter = true;
    return SWDst::Create(options);
}};
