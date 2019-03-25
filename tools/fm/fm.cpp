// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "CommandLineFlags.h"
#include "CommonFlags.h"
#include "EventTracingPriv.h"
#include "GrContextOptions.h"
#include "HashAndEncode.h"
#include "SkCodec.h"
#include "SkColorSpace.h"
#include "SkColorSpacePriv.h"
#include "SkGraphics.h"
#include "SkMD5.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkPicture.h"
#include "ToolUtils.h"
#include "gm.h"
#include <chrono>
#include <functional>
#include <stdio.h>
#include <stdlib.h>

static DEFINE_string2(sources, s, "", "Which GMs, .skps, or images to draw.");
static DEFINE_string2(backend, b, "", "Backend used to create a canvas to draw into.");

static DEFINE_string(ct,    "8888", "The color type for raster backends.");
static DEFINE_string(at,  "premul", "The alpha type for raster backends.");
static DEFINE_string(gamut, "srgb", "The color gamut for raster backends.");
static DEFINE_string(tf,    "srgb", "The transfer function for raster backends.");

static DEFINE_int(msaa, 0, "Number of samples per pixel for GPU backends.");

static DEFINE_bool(decode_to_dst, false,
                   "Decode images to destination format rather than suggested natural format.");

static DEFINE_bool(cpu_detect, true, "Detect CPU features for runtime optimizations?");

static DEFINE_bool2(verbose, v, false, "Print progress to stdout.");

static DEFINE_string(key,        "", "Metadata passed through to .png encoder and .json output.");
static DEFINE_string(parameters, "", "Metadata passed through to .png encoder and .json output.");

static DEFINE_string2(writePath, w, "", "Write .pngs to this directory if set.");

static void exit_with_failure() {
    // TODO: dump stack trace, debug trap, print currently running job, etc?
    exit(1);
}

struct Source {
    SkString                       name;
    SkISize                        size;
    std::function<void(SkCanvas*)> draw;
};

static Source gm_source(std::shared_ptr<skiagm::GM> gm) {
    return {
        SkString{gm->getName()},
        gm->getISize(),
        [gm](SkCanvas* canvas) {
            SkString err;
            if (skiagm::DrawResult::kFail == gm->draw(canvas, &err)) {
                fprintf(stderr, "Drawing GM %s failed: %s\n",
                        gm->getName(), err.c_str());
                exit_with_failure();
            }
        },
    };
}

static Source picture_source(SkString name, sk_sp<SkPicture> pic) {
    return {
        name,
        pic->cullRect().roundOut().size(),
        [pic](SkCanvas* canvas) {
            canvas->drawPicture(pic);
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
                    break;
                default:
                    fprintf(stderr, "SkCodec::getPixels failed: %d.", result);
                    exit_with_failure();
            }
        },
    };
}

template <typename T>
struct Labeled {
    const char* label;
    T           value;
};

template <typename T, int N>
static bool parse_flag(const CommandLineFlags::StringArray& flag,
                       const char* flag_name,
                       const Labeled<T> (&array)[N],
                       T* value) {
    for (auto entry : array) {
        if (flag.contains(entry.label)) {
            *value = entry.value;
            return true;
        }
    }
    fprintf(stderr, "Known values for --%s:\n", flag_name);
    for (auto entry : array) {
        fprintf(stderr, "    --%s %s\n", flag_name, entry.label);
    }
    return false;
}

static sk_sp<SkImage> draw_with_cpu(Source source, SkImageInfo info) {
    sk_sp<SkSurface> surface = SkSurface::MakeRaster(info);
    source.draw(surface->getCanvas());
    return surface->makeImageSnapshot();
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


    SkTArray<Source> sources;
    for (skiagm::GMFactory factory : skiagm::GMRegistry::Range()) {
        std::shared_ptr<skiagm::GM> gm{factory(nullptr)};

        if (FLAGS_sources.isEmpty()) {
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
    if (sources.empty()) {
        return 0;
    }

    const Labeled<SkColorType> kColorTypes[] = {
        { "a8",           kAlpha_8_SkColorType },
        { "g8",            kGray_8_SkColorType },
        { "565",          kRGB_565_SkColorType },
        { "4444",       kARGB_4444_SkColorType },
        { "8888",             kN32_SkColorType },
        { "888x",        kRGB_888x_SkColorType },
        { "1010102", kRGBA_1010102_SkColorType },
        { "101010x",  kRGB_101010x_SkColorType },
        { "f16norm", kRGBA_F16Norm_SkColorType },
        { "f16",         kRGBA_F16_SkColorType },
        { "f32",         kRGBA_F32_SkColorType },
        { "rgba",       kRGBA_8888_SkColorType },
        { "bgra",       kBGRA_8888_SkColorType },
    };
    const Labeled<SkAlphaType> kAlphaTypes[] = {
        {   "premul",   kPremul_SkAlphaType },
        { "unpremul", kUnpremul_SkAlphaType },
    };
    const Labeled<skcms_Matrix3x3> kGamuts[] = {
        { "srgb",    SkNamedGamut::kSRGB },
        { "p3",      SkNamedGamut::kDCIP3 },
        { "rec2020", SkNamedGamut::kRec2020 },
        { "adobe",   SkNamedGamut::kAdobeRGB },
        { "narrow",  gNarrow_toXYZD50},
    };
    const Labeled<skcms_TransferFunction> kTransferFunctions[] = {
        { "srgb"   , SkNamedTransferFn::kSRGB },
        { "rec2020", {2.22222f, 0.909672f, 0.0903276f, 0.222222f, 0.0812429f, 0, 0} },
        { "2.2"    , SkNamedTransferFn::k2Dot2 },
        { "linear" , SkNamedTransferFn::kLinear },
    };
    const Labeled<sk_sp<SkImage>(*)(Source, SkImageInfo)> kBackends[] = {
        { "cpu", draw_with_cpu },
    };

    SkColorType            ct;
    SkAlphaType            at;
    skcms_Matrix3x3        gamut;
    skcms_TransferFunction tf;
    sk_sp<SkImage>       (*backend)(Source, SkImageInfo);

    if (!parse_flag(FLAGS_ct     , "ct"     , kColorTypes       , &ct)    ||
        !parse_flag(FLAGS_at     , "at"     , kAlphaTypes       , &at)    ||
        !parse_flag(FLAGS_gamut  , "gamut"  , kGamuts           , &gamut) ||
        !parse_flag(FLAGS_tf     , "tf"     , kTransferFunctions, &tf)    ||
        !parse_flag(FLAGS_backend, "backend", kBackends         , &backend)) {
        return 1;
    }

    const SkImageInfo info = SkImageInfo::Make(0,0, ct,at, SkColorSpace::MakeRGB(tf,gamut));

    for (auto source : sources) {
        if (FLAGS_verbose) {
            fprintf(stdout, "%50s", source.name.c_str());
        }

        const auto start = std::chrono::steady_clock::now();

        sk_sp<SkImage> image = backend(source, info.makeWH(source.size.width(),
                                                           source.size.height()));

        SkBitmap bitmap;
        if (!image->asLegacyBitmap(&bitmap)) {
            fprintf(stderr, "SkImage::asLegacyBitmap() failed.\n");
            exit_with_failure();
        }

        HashAndEncode hashAndEncode{bitmap};
        SkString md5;
        {
            SkMD5 hash;
            hashAndEncode.write(&hash);

            SkMD5::Digest digest;
            hash.finish(digest);
            for (int i = 0; i < 16; i++) {
                md5.appendf("%02x", digest.data[i]);
            }
        }

        if (!FLAGS_writePath.isEmpty()) {
            sk_mkdir(FLAGS_writePath[0]);
            SkString path = SkStringPrintf("%s/%s.png", FLAGS_writePath[0], source.name.c_str());

            if (!hashAndEncode.writePngTo(path.c_str(), md5.c_str(), FLAGS_key, FLAGS_parameters)) {
                fprintf(stderr, "Could not write a .png to %s.\n", path.c_str());
                exit_with_failure();
            }

        }

        const auto elapsed = std::chrono::steady_clock::now() - start;

        if (FLAGS_verbose) {
            fprintf(stdout, "\t%s\t%7dms\n",
                    md5.c_str(),
                    (int)std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
        }
    }

    return 0;
}
