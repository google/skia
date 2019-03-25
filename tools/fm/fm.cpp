// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "CommandLineFlags.h"
#include "CommonFlags.h"
#include "EventTracingPriv.h"
#include "GrContextOptions.h"
#include "SkCodec.h"
#include "SkGraphics.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkPicture.h"
#include "ToolUtils.h"
#include "gm.h"
#include <functional>
#include <stdio.h>

static DEFINE_string2(sources, s, "help", "Which GMs, .skps, or images to draw.");
static DEFINE_string2(backend, b, "help", "Backend used to create a canvas to draw into.");

static DEFINE_string(ct,   "8888", "The canvas color type for raster backends.");
static DEFINE_string(at, "premul", "The canvas alpha type for raster backends.");
static DEFINE_string(cs,   "srgb", "The canvas color space for raster backends.");

static DEFINE_string(api, "", "GPU API to use for GPU backends.");
static DEFINE_int(samples, 0, "Number of samples per pixel for GPU backends.");

static DEFINE_bool(decode_to_dst, false,
                   "Decode images to destination format rather than suggested natural format.");

static DEFINE_bool(cpu_detect, true, "Detect CPU features for runtime optimizations?");

static DEFINE_bool2(verbose, v, false, "Print progress to stdout.");


struct Source {
    SkString                           name;
    SkISize                            size;
    std::function<SkString(SkCanvas*)> draw;
};

static Source gm_source(std::shared_ptr<skiagm::GM> gm) {
    return {
        SkString{gm->getName()},
        gm->getISize(),
        [gm](SkCanvas* canvas) {
            SkString err;
            gm->draw(canvas, &err);
            return err;
        },
    };
}

static Source picture_source(SkString name, sk_sp<SkPicture> pic) {
    return {
        name,
        pic->cullRect().roundOut().size(),
        [pic](SkCanvas* canvas) {
            canvas->drawPicture(pic);
            return SkString();
        },
    };
}

static Source codec_source(SkString name, std::shared_ptr<SkCodec> codec) {
    return {
        name,
        codec->dimensions(),
        [codec](SkCanvas* canvas) {
            SkImageInfo info = codec->getInfo();
            if (FLAGS_decode_to_dst) {
                info = canvas->imageInfo().makeWH(info.width(),
                                                  info.height());
            }

            SkBitmap bm;
            bm.allocPixels(info);

            switch (auto result = codec->getPixels(info, bm.getPixels(), bm.rowBytes())) {
                case SkCodec::kSuccess:
                case SkCodec::kErrorInInput:
                case SkCodec::kIncompleteInput:
                    canvas->drawBitmap(bm, 0,0);
                    return SkString();
                default:
                    return SkStringPrintf("SkCodec::getPixels failed: %d.", result);
            }

        },
    };
}

int main(int argc, char** argv) {
    CommandLineFlags::Parse(argc, argv);

    if (FLAGS_cpu_detect) {
        SkGraphics::Init();
    }
    initializeEventTracingForTools();
    ToolUtils::SetDefaultFontMgr();
    SetAnalyticAAFromCommonFlags();
    GrContextOptions grCtxOptions;
    SetCtxOptionsFromCommonFlags(&grCtxOptions);

    auto needs_help = [](const CommandLineFlags::StringArray& flag) {
        return flag.isEmpty()
            || flag.contains("list")
            || flag.contains("help");
    };

    SkTArray<Source> sources;
    for (skiagm::GMFactory factory : skiagm::GMRegistry::Range()) {
        std::shared_ptr<skiagm::GM> gm{factory(nullptr)};

        if (needs_help(FLAGS_sources)) {
            fprintf(stdout, "%s\n", gm->getName());
        } else if (FLAGS_sources.contains(gm->getName())) {
            sources.push_back(gm_source(gm));
        }
    }
    for (const SkString& source : FLAGS_sources) {
        if (sk_sp<SkData> blob = SkData::MakeFromFileName(source.c_str())) {
            const SkString name = SkOSPath::Basename(source.c_str());

            if (sk_sp<SkPicture> pic = SkPicture::MakeFromData(blob.get())) {
                sources.push_back(picture_source(name, pic));
            }
            if (std::shared_ptr<SkCodec> codec = SkCodec::MakeFromData(blob)) {
                sources.push_back(codec_source(name, codec));
            }
        }
    }

    return 0;
}
