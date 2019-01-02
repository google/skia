/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCommandLineFlags.h"
#include "SkGraphics.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkTArray.h"
#include "gm.h"
#include <cstdio>
#include <functional>

static DEFINE_bool(cpu_detect, true, "Detect runtime CPU features and optimizations?");
static DEFINE_string2(source, s, "", "GMs or .skps to draw.");
static DEFINE_string(ct, "8888",
                     "Color type: A8, G8, 565, 4444, 8888, 888x, 1010102, 101010x, F16, F32, BGRA");
static DEFINE_bool(upm, false, "Unpremul?");

struct Task {
    const char*                    name;
    SkISize                        size;
    std::function<void(SkCanvas*)> draw;
};

int main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);

    if (FLAGS_cpu_detect) {
        SkGraphics::Init();
    }

    SkImageInfo info = []{
        SkColorType ct = kRGBA_8888_SkColorType;
        if (FLAGS_ct.contains("A8"))      { ct =      kAlpha_8_SkColorType; }
        if (FLAGS_ct.contains("G8"))      { ct =       kGray_8_SkColorType; }
        if (FLAGS_ct.contains("565"))     { ct =      kRGB_565_SkColorType; }
        if (FLAGS_ct.contains("4444"))    { ct =    kARGB_4444_SkColorType; }
        if (FLAGS_ct.contains("8888"))    { ct =    kRGBA_8888_SkColorType; }
        if (FLAGS_ct.contains("888x"))    { ct =     kRGB_888x_SkColorType; }
        if (FLAGS_ct.contains("1010102")) { ct = kRGBA_1010102_SkColorType; }
        if (FLAGS_ct.contains("101010x")) { ct =  kRGB_101010x_SkColorType; }
        if (FLAGS_ct.contains("F16"))     { ct =     kRGBA_F16_SkColorType; }
        if (FLAGS_ct.contains("F32"))     { ct =     kRGBA_F32_SkColorType; }
        if (FLAGS_ct.contains("BGRA"))    { ct =    kBGRA_8888_SkColorType; }

        SkAlphaType at = FLAGS_upm ? kUnpremul_SkAlphaType
                                   : kPremul_SkAlphaType;

        sk_sp<SkColorSpace> cs;  // TODO(mtklein)

        return SkImageInfo::Make(0,0, ct,at,cs);
    }();

    SkTArray<Task> tasks;

    for (skiagm::GMFactory f : skiagm::GMRegistry::Range()) {
        std::shared_ptr<skiagm::GM> gm{f(nullptr)};

        if (FLAGS_source.contains(gm->getName())) {
            tasks.push_back(Task{
                gm->getName(),
                gm->getISize(),
                [=](SkCanvas* c) { gm->draw(c); }});
        }
    }
    for (const SkString& source : FLAGS_source) {
        SkFILEStream stream(source.c_str());
        if (!stream.isValid()) {
            continue;
        }
        if (sk_sp<SkPicture> picture = SkPicture::MakeFromStream(&stream)) {
            tasks.push_back(Task{
                source.c_str(),
                picture->cullRect().roundOut().size(),
                [=](SkCanvas* c) { c->drawPicture(picture); }});
        }
    }

    if (tasks.empty()) {
        for (skiagm::GMFactory f : skiagm::GMRegistry::Range()) {
            std::unique_ptr<skiagm::GM> gm{f(nullptr)};
            printf("%s\n", gm->getName());
        }
    }

    for (auto&& task : tasks) {
        sk_sp<SkSurface> surface = SkSurface::MakeRaster(info.makeWH(task.size.width(),
                                                                     task.size.height()));
        fprintf(stderr, "%s", task.name);
        task.draw(surface->getCanvas());
        fprintf(stderr, "\tok\n");
    }

    return 0;
}
