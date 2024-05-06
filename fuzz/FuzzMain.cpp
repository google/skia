/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkImage.h"
#include "include/core/SkMallocPixelRef.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/encode/SkPngEncoder.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkReadBuffer.h"
#include "src/utils/SkOSPath.h"
#include "tools/ToolUtils.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/fonts/FontToolUtils.h"

#include <iostream>
#include <map>
#include <regex>
#include <signal.h>

static DEFINE_string2(bytes, b, "", "A path to a file or a directory. If a file, the "
                      "contents will be used as the fuzz bytes. If a directory, all files "
                      "in the directory will be used as fuzz bytes for the fuzzer, one at a "
                      "time.");
static DEFINE_string2(name, n, "", "If --type is 'api', fuzz the API with this name.");
static DEFINE_string2(dump, d, "", "If not empty, dump 'image*' or 'skp' types as a "
                                   "PNG with this name.");
static DEFINE_int(loops, 1, "Run the fuzzer on each input this many times.");
DEFINE_bool2(verbose, v, false, "Print more information while fuzzing.");

// This cannot be inlined in DEFINE_string2 due to interleaved ifdefs
static constexpr char g_type_message[] = "How to interpret --bytes, one of:\n"
                                         "android_codec\n"
                                         "animated_image_decode\n"
                                         "api\n"
                                         "color_deserialize\n"
                                         "colrv1\n"
                                         "filter_fuzz (equivalent to Chrome's filter_fuzz_stub)\n"
                                         "image_decode\n"
                                         "image_decode_incremental\n"
                                         "image_mode\n"
                                         "image_scale\n"
                                         "json\n"
                                         "path_deserialize\n"
                                         "region_deserialize\n"
                                         "region_set_path\n"
                                         "skdescriptor_deserialize\n"
                                         "skmeshspecialization\n"
#if defined(SK_ENABLE_SKOTTIE)
                                         "skottie_json\n"
#endif
                                         "skp\n"
                                         "skruntimeblender\n"
                                         "skruntimecolorfilter\n"
                                         "skruntimeeffect\n"
                                         "sksl2glsl\n"
                                         "sksl2metal\n"
                                         "sksl2pipeline\n"
                                         "sksl2spirv\n"
                                         "sksl2wgsl\n"
                                         "svg_dom\n"
                                         "textblob";

static DEFINE_string2(type, t, "", g_type_message);

static int fuzz_file(const SkString& path, SkString type);
static uint8_t calculate_option(SkData*);
static SkString try_auto_detect(const SkString& path, SkString* name);

static void fuzz_android_codec(const sk_sp<SkData>&);
static void fuzz_animated_img(const sk_sp<SkData>&);
static void fuzz_api(const sk_sp<SkData>&, const SkString& name);
static void fuzz_color_deserialize(const sk_sp<SkData>&);
static void fuzz_colrv1(const sk_sp<SkData>&);
static void fuzz_filter_fuzz(const sk_sp<SkData>&);
static void fuzz_image_decode(const sk_sp<SkData>&);
static void fuzz_image_decode_incremental(const sk_sp<SkData>&);
static void fuzz_img(const sk_sp<SkData>&, uint8_t, uint8_t);
static void fuzz_json(const sk_sp<SkData>&);
static void fuzz_parse_path(const sk_sp<SkData>&);
static void fuzz_path_deserialize(const sk_sp<SkData>&);
static void fuzz_region_deserialize(const sk_sp<SkData>&);
static void fuzz_region_set_path(const sk_sp<SkData>&);
static void fuzz_skdescriptor_deserialize(const sk_sp<SkData>&);
static void fuzz_skmeshspecification(const sk_sp<SkData>&);
static void fuzz_skp(const sk_sp<SkData>&);
static void fuzz_skruntimeblender(const sk_sp<SkData>&);
static void fuzz_skruntimecolorfilter(const sk_sp<SkData>&);
static void fuzz_skruntimeeffect(const sk_sp<SkData>&);
static void fuzz_sksl2glsl(const sk_sp<SkData>&);
static void fuzz_sksl2metal(const sk_sp<SkData>&);
static void fuzz_sksl2pipeline(const sk_sp<SkData>&);
static void fuzz_sksl2spirv(const sk_sp<SkData>&);
static void fuzz_sksl2wgsl(const sk_sp<SkData>&);
static void fuzz_textblob_deserialize(const sk_sp<SkData>&);

static void print_api_names();

#if defined(SK_ENABLE_SVG)
static void fuzz_svg_dom(const sk_sp<SkData>&);
#endif

#if defined(SK_ENABLE_SKOTTIE)
static void fuzz_skottie_json(const sk_sp<SkData>&);
#endif

int main(int argc, char** argv) {
    CommandLineFlags::SetUsage(
            "Usage: fuzz -t <type> -b <path/to/file> [-n api-to-fuzz]\n"
            "       fuzz -b <path/to/file>\n"
            "--help lists the valid types. If type is not specified,\n"
            "fuzz will make a guess based on the name of the file.\n");
    CommandLineFlags::Parse(argc, argv);
    ToolUtils::UsePortableFontMgr();

    SkString path = SkString(FLAGS_bytes.isEmpty() ? argv[0] : FLAGS_bytes[0]);
    SkString type = SkString(FLAGS_type.isEmpty() ? "" : FLAGS_type[0]);

    int loopCount = std::max(FLAGS_loops, 1);

    if (!sk_isdir(path.c_str())) {
        for (int i = 0; i < loopCount; ++i) {
            int rv = fuzz_file(path, type);
            if (rv != 0) {
                return rv;
            }
        }
        return 0;
    }

    SkOSFile::Iter it(path.c_str());
    for (SkString file; it.next(&file); ) {
        SkString p = SkOSPath::Join(path.c_str(), file.c_str());
        SkDebugf("Fuzzing %s\n", p.c_str());
        for (int i = 0; i < loopCount; ++i) {
            int rv = fuzz_file(p, type);
            if (rv != 0) {
                return rv;
            }
        }
    }
    return 0;
}

static int fuzz_file(const SkString& path, SkString type) {
    sk_sp<SkData> bytes(SkData::MakeFromFileName(path.c_str()));
    if (!bytes) {
        SkDebugf("Could not read %s\n", path.c_str());
        return 1;
    }

    SkString name = SkString(FLAGS_name.isEmpty() ? "" : FLAGS_name[0]);

    if (type.isEmpty()) {
        type = try_auto_detect(path, &name);
    }

    if (type.isEmpty()) {
        SkDebugf("Could not autodetect type of %s\n", path.c_str());
        return 1;
    }
    if (type.equals("android_codec")) {
        fuzz_android_codec(std::move(bytes));
        return 0;
    }
    if (type.equals("animated_image_decode")) {
        fuzz_animated_img(std::move(bytes));
        return 0;
    }
    if (type.equals("api")) {
        fuzz_api(bytes, name);
        return 0;
    }
    if (type.equals("color_deserialize")) {
        fuzz_color_deserialize(std::move(bytes));
        return 0;
    }
    if (type.equals("colrv1")) {
        fuzz_colrv1(std::move(bytes));
        return 0;
    }
    if (type.equals("filter_fuzz")) {
        fuzz_filter_fuzz(std::move(bytes));
        return 0;
    }
    if (type.equals("image_decode")) {
        fuzz_image_decode(std::move(bytes));
        return 0;
    }
    if (type.equals("image_decode_incremental")) {
        fuzz_image_decode_incremental(std::move(bytes));
        return 0;
    }
    if (type.equals("image_scale")) {
        uint8_t option = calculate_option(bytes.get());
        fuzz_img(std::move(bytes), option, 0);
        return 0;
    }
    if (type.equals("image_mode")) {
        uint8_t option = calculate_option(bytes.get());
        fuzz_img(std::move(bytes), 0, option);
        return 0;
    }
    if (type.equals("json")) {
        fuzz_json(std::move(bytes));
        return 0;
    }
    if (type.equals("parse_path")) {
        fuzz_parse_path(std::move(bytes));
        return 0;
    }
    if (type.equals("path_deserialize")) {
        fuzz_path_deserialize(std::move(bytes));
        return 0;
    }
    if (type.equals("region_deserialize")) {
        fuzz_region_deserialize(std::move(bytes));
        return 0;
    }
    if (type.equals("region_set_path")) {
        fuzz_region_set_path(std::move(bytes));
        return 0;
    }
    if (type.equals("pipe")) {
        SkDebugf("I would prefer not to.\n");
        return 0;
    }
    if (type.equals("skdescriptor_deserialize")) {
        fuzz_skdescriptor_deserialize(std::move(bytes));
        return 0;
    }
#if defined(SK_ENABLE_SKOTTIE)
    if (type.equals("skottie_json")) {
        fuzz_skottie_json(std::move(bytes));
        return 0;
    }
#endif
    if (type.equals("skmeshspecification")) {
        fuzz_skmeshspecification(std::move(bytes));
        return 0;
    }
    if (type.equals("skp")) {
        fuzz_skp(std::move(bytes));
        return 0;
    }
    if (type.equals("skruntimeblender")) {
        fuzz_skruntimeblender(std::move(bytes));
        return 0;
    }
    if (type.equals("skruntimecolorfilter")) {
        fuzz_skruntimecolorfilter(std::move(bytes));
        return 0;
    }
    if (type.equals("skruntimeeffect")) {
        fuzz_skruntimeeffect(std::move(bytes));
        return 0;
    }
    if (type.equals("sksl2glsl")) {
        fuzz_sksl2glsl(std::move(bytes));
        return 0;
    }
    if (type.equals("sksl2metal")) {
        fuzz_sksl2metal(std::move(bytes));
        return 0;
    }
    if (type.equals("sksl2pipeline")) {
        fuzz_sksl2pipeline(std::move(bytes));
        return 0;
    }
    if (type.equals("sksl2spirv")) {
        fuzz_sksl2spirv(std::move(bytes));
        return 0;
    }
    if (type.equals("sksl2wgsl")) {
        fuzz_sksl2wgsl(std::move(bytes));
        return 0;
    }
#if defined(SK_ENABLE_SVG)
    if (type.equals("svg_dom")) {
        fuzz_svg_dom(std::move(bytes));
        return 0;
    }
#endif
    if (type.equals("textblob")) {
        fuzz_textblob_deserialize(std::move(bytes));
        return 0;
    }
    SkDebugf("Unknown type %s\n", type.c_str());
    CommandLineFlags::PrintUsage();
    return 1;
}

static std::map<std::string, std::string> cf_api_map = {
    {"api_create_ddl", "CreateDDL"},
    {"api_draw_functions", "DrawFunctions"},
    {"api_ddl_threading", "DDLThreadingGL"},
    {"api_gradients", "Gradients"},
    {"api_image_filter", "ImageFilter"},
    {"api_mock_gpu_canvas", "MockGPUCanvas"},
    {"api_null_canvas", "NullCanvas"},
    {"api_path_measure", "PathMeasure"},
    {"api_pathop", "Pathop"},
    {"api_polyutils", "PolyUtils"},
#if defined(SK_GRAPHITE) && defined(SK_ENABLE_PRECOMPILE)
    {"api_precompile", "Precompile"},
#endif
    {"api_raster_n32_canvas", "RasterN32Canvas"},
    {"api_skparagraph", "SkParagraph"},
    {"api_svg_canvas", "SVGCanvas"},
    {"cubic_quad_roots", "CubicQuadRoots"},
    {"jpeg_encoder", "JPEGEncoder"},
    {"png_encoder", "PNGEncoder"},
    {"skia_pathop_fuzzer", "LegacyChromiumPathop"},
    {"webp_encoder", "WEBPEncoder"}
};

// maps clusterfuzz/oss-fuzz -> Skia's name
static std::map<std::string, std::string> cf_map = {
    {"android_codec", "android_codec"},
    {"animated_image_decode", "animated_image_decode"},
    {"colrv1", "colrv1"},
    {"image_decode", "image_decode"},
    {"image_decode_incremental", "image_decode_incremental"},
    {"image_filter_deserialize", "filter_fuzz"},
    {"image_filter_deserialize_width", "filter_fuzz"},
    {"path_deserialize", "path_deserialize"},
    {"region_deserialize", "region_deserialize"},
    {"region_set_path", "region_set_path"},
    {"skdescriptor_deserialize", "skdescriptor_deserialize"},
    {"skjson", "json"},
    {"skmeshspecification", "skmeshspecification"},
    {"skp", "skp"},
    {"skruntimeeffect", "skruntimeeffect"},
    {"sksl2glsl", "sksl2glsl"},
    {"sksl2metal", "sksl2metal"},
    {"sksl2spirv", "sksl2spirv"},
    {"sksl2pipeline", "sksl2pipeline"},
#if defined(SK_ENABLE_SKOTTIE)
    {"skottie_json", "skottie_json"},
#endif
#if defined(SK_ENABLE_SVG)
    {"svg_dom", "svg_dom"},
#endif
    {"textblob_deserialize", "textblob"}
};

static SkString try_auto_detect(const SkString& path, SkString* name) {
    std::cmatch m;
    std::regex clusterfuzz("clusterfuzz-testcase(-minimized)?-([a-z0-9_]+)-[\\d]+");
    std::regex skiafuzzer("(api-)?(\\w+)-[a-f0-9]+");

    if (std::regex_search(path.c_str(), m, clusterfuzz)) {
        std::string type = m.str(2);

        if (cf_api_map.find(type) != cf_api_map.end()) {
            *name = SkString(cf_api_map[type].c_str());
            return SkString("api");
        } else {
            if (cf_map.find(type) != cf_map.end()) {
                return SkString(cf_map[type].c_str());
            }
        }
    } else if (std::regex_search(path.c_str(), m, skiafuzzer)) {
        std::string a1 = m.str(1);
        std::string typeOrName = m.str(2);
        if (a1.length() > 0) {
            // it's an api fuzzer
            *name = SkString(typeOrName.c_str());
            return SkString("api");
        } else {
            return SkString(typeOrName.c_str());
        }
    }

    return SkString();
}

void FuzzJSON(const uint8_t *data, size_t size);

static void fuzz_json(const sk_sp<SkData>& data){
    FuzzJSON(data->bytes(), data->size());
    SkDebugf("[terminated] Done parsing!\n");
}

#if defined(SK_ENABLE_SKOTTIE)
void FuzzSkottieJSON(const uint8_t *data, size_t size);

static void fuzz_skottie_json(const sk_sp<SkData>& data){
    FuzzSkottieJSON(data->bytes(), data->size());
    SkDebugf("[terminated] Done animating!\n");
}
#endif

#if defined(SK_ENABLE_SVG)
void FuzzSVG(const uint8_t *data, size_t size);

static void fuzz_svg_dom(const sk_sp<SkData>& data){
    FuzzSVG(data->bytes(), data->size());
    SkDebugf("[terminated] Done DOM!\n");
}
#endif

void FuzzCOLRv1(const uint8_t* data, size_t size);

static void fuzz_colrv1(const sk_sp<SkData>& data) {
    FuzzCOLRv1(data->bytes(), data->size());
    SkDebugf("[terminated] Done COLRv1!\n");
}

// This adds up the first 1024 bytes and returns it as an 8 bit integer.  This allows afl-fuzz to
// deterministically excercise different paths, or *options* (such as different scaling sizes or
// different image modes) without needing to introduce a parameter.  This way we don't need a
// image_scale1, image_scale2, image_scale4, etc fuzzer, we can just have a image_scale fuzzer.
// Clients are expected to transform this number into a different range, e.g. with modulo (%).
static uint8_t calculate_option(SkData* bytes) {
    uint8_t total = 0;
    const uint8_t* data = bytes->bytes();
    for (size_t i = 0; i < 1024 && i < bytes->size(); i++) {
        total += data[i];
    }
    return total;
}

static void print_api_names(){
    SkDebugf("When using --type api, please choose an API to fuzz with --name/-n:\n");
    for (const Fuzzable& fuzzable : sk_tools::Registry<Fuzzable>::Range()) {
        SkDebugf("\t%s\n", fuzzable.name);
    }
}

static void fuzz_api(const sk_sp<SkData>& data, const SkString& name) {
    for (const Fuzzable& fuzzable : sk_tools::Registry<Fuzzable>::Range()) {
        if (name.equals(fuzzable.name)) {
            SkDebugf("Fuzzing %s...\n", fuzzable.name);
            Fuzz fuzz(data->bytes(), data->size());
            fuzzable.fn(&fuzz);
            SkDebugf("[terminated] Success!\n");
            return;
        }
    }

    print_api_names();
}

static void dump_png(const SkBitmap& bitmap) {
    if (!FLAGS_dump.isEmpty()) {
        SkFILEWStream file(FLAGS_dump[0]);
        SkPngEncoder::Encode(&file, bitmap.pixmap(), {});
        SkDebugf("Dumped to %s\n", FLAGS_dump[0]);
    }
}

bool FuzzAnimatedImage(const uint8_t *data, size_t size);

static void fuzz_animated_img(const sk_sp<SkData>& data) {
    if (FuzzAnimatedImage(data->bytes(), data->size())) {
        SkDebugf("[terminated] Success from decoding/drawing animated image!\n");
        return;
    }
    SkDebugf("[terminated] Could not decode or draw animated image.\n");
}

bool FuzzImageDecode(const uint8_t *data, size_t size);

static void fuzz_image_decode(const sk_sp<SkData>& data) {
    if (FuzzImageDecode(data->bytes(), data->size())) {
         SkDebugf("[terminated] Success from decoding/drawing image!\n");
         return;
    }
    SkDebugf("[terminated] Could not decode or draw image.\n");
}

bool FuzzIncrementalImageDecode(const uint8_t *data, size_t size);

static void fuzz_image_decode_incremental(const sk_sp<SkData>& data) {
    if (FuzzIncrementalImageDecode(data->bytes(), data->size())) {
        SkDebugf("[terminated] Success using incremental decode!\n");
        return;
    }
    SkDebugf("[terminated] Could not incrementally decode and image.\n");
}

bool FuzzAndroidCodec(const uint8_t *fuzzData, size_t fuzzSize, uint8_t sampleSize);

static void fuzz_android_codec(const sk_sp<SkData>& data) {
    Fuzz fuzz(data->bytes(), data->size());
    uint8_t sampleSize;
    fuzz.nextRange(&sampleSize, 1, 64);
    if (FuzzAndroidCodec(fuzz.remainingData(), fuzz.remainingSize(), sampleSize)) {
        SkDebugf("[terminated] Success on Android Codec sampleSize=%u!\n", sampleSize);
        return;
    }
    SkDebugf("[terminated] Could not use Android Codec sampleSize=%u!\n", sampleSize);
}

// This is a "legacy" fuzzer that likely does too much. It was based off of how
// DM reads in images. image_decode, image_decode_incremental and android_codec
// are more targeted fuzzers that do a subset of what this one does.
static void fuzz_img(const sk_sp<SkData>& bytes, uint8_t scale, uint8_t mode) {
    // We can scale 1x, 2x, 4x, 8x, 16x
    scale = scale % 5;
    float fscale = (float)pow(2.0f, scale);
    SkDebugf("Scaling factor: %f\n", fscale);

    // We have 5 different modes of decoding.
    mode = mode % 5;
    SkDebugf("Mode: %d\n", mode);

    // This is mostly copied from DMSrcSink's CodecSrc::draw method.
    SkDebugf("Decoding\n");
    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(bytes));
    if (nullptr == codec) {
        SkDebugf("[terminated] Couldn't create codec.\n");
        return;
    }

    SkImageInfo decodeInfo = codec->getInfo();
    SkISize size = codec->getScaledDimensions(fscale);
    decodeInfo = decodeInfo.makeDimensions(size);

    SkBitmap bitmap;
    SkCodec::Options options;
    options.fZeroInitialized = SkCodec::kYes_ZeroInitialized;

    if (!bitmap.tryAllocPixelsFlags(decodeInfo, SkBitmap::kZeroPixels_AllocFlag)) {
        SkDebugf("[terminated] Could not allocate memory.  Image might be too large (%d x %d)",
                 decodeInfo.width(), decodeInfo.height());
        return;
    }

    switch (mode) {
        case 0: {//kCodecZeroInit_Mode, kCodec_Mode
            switch (codec->getPixels(decodeInfo, bitmap.getPixels(), bitmap.rowBytes(), &options)) {
                case SkCodec::kSuccess:
                    SkDebugf("[terminated] Success!\n");
                    break;
                case SkCodec::kIncompleteInput:
                    SkDebugf("[terminated] Partial Success\n");
                    break;
                case SkCodec::kErrorInInput:
                    SkDebugf("[terminated] Partial Success with error\n");
                    break;
                case SkCodec::kInvalidConversion:
                    SkDebugf("Incompatible colortype conversion\n");
                    // Crash to allow afl-fuzz to know this was a bug.
                    raise(SIGSEGV);
                    break;
                default:
                    SkDebugf("[terminated] Couldn't getPixels.\n");
                    return;
            }
            break;
        }
        case 1: {//kScanline_Mode
            if (SkCodec::kSuccess != codec->startScanlineDecode(decodeInfo)) {
                SkDebugf("[terminated] Could not start scanline decoder\n");
                return;
            }

            void* dst = bitmap.getAddr(0, 0);
            size_t rowBytes = bitmap.rowBytes();
            uint32_t height = decodeInfo.height();
            // We do not need to check the return value.  On an incomplete
            // image, memory will be filled with a default value.
            codec->getScanlines(dst, height, rowBytes);
            SkDebugf("[terminated] Success!\n");
            break;
        }
        case 2: { //kStripe_Mode
            const int height = decodeInfo.height();
            // This value is chosen arbitrarily.  We exercise more cases by choosing a value that
            // does not align with image blocks.
            const int stripeHeight = 37;
            const int numStripes = (height + stripeHeight - 1) / stripeHeight;

            // Decode odd stripes
            if (SkCodec::kSuccess != codec->startScanlineDecode(decodeInfo)
                    || SkCodec::kTopDown_SkScanlineOrder != codec->getScanlineOrder()) {
                // This mode was designed to test the new skip scanlines API in libjpeg-turbo.
                // Jpegs have kTopDown_SkScanlineOrder, and at this time, it is not interesting
                // to run this test for image types that do not have this scanline ordering.
                SkDebugf("[terminated] Could not start top-down scanline decoder\n");
                return;
            }

            for (int i = 0; i < numStripes; i += 2) {
                // Skip a stripe
                const int linesToSkip = std::min(stripeHeight, height - i * stripeHeight);
                codec->skipScanlines(linesToSkip);

                // Read a stripe
                const int startY = (i + 1) * stripeHeight;
                const int linesToRead = std::min(stripeHeight, height - startY);
                if (linesToRead > 0) {
                    codec->getScanlines(bitmap.getAddr(0, startY), linesToRead, bitmap.rowBytes());
                }
            }

            // Decode even stripes
            const SkCodec::Result startResult = codec->startScanlineDecode(decodeInfo);
            if (SkCodec::kSuccess != startResult) {
                SkDebugf("[terminated] Failed to restart scanline decoder with same parameters.\n");
                return;
            }
            for (int i = 0; i < numStripes; i += 2) {
                // Read a stripe
                const int startY = i * stripeHeight;
                const int linesToRead = std::min(stripeHeight, height - startY);
                codec->getScanlines(bitmap.getAddr(0, startY), linesToRead, bitmap.rowBytes());

                // Skip a stripe
                const int linesToSkip = std::min(stripeHeight, height - (i + 1) * stripeHeight);
                if (linesToSkip > 0) {
                    codec->skipScanlines(linesToSkip);
                }
            }
            SkDebugf("[terminated] Success!\n");
            break;
        }
        case 3: { //kSubset_Mode
            // Arbitrarily choose a divisor.
            int divisor = 2;
            // Total width/height of the image.
            const int W = codec->getInfo().width();
            const int H = codec->getInfo().height();
            if (divisor > W || divisor > H) {
                SkDebugf("[terminated] Cannot codec subset: divisor %d is too big "
                         "with dimensions (%d x %d)\n", divisor, W, H);
                return;
            }
            // subset dimensions
            // SkWebpCodec, the only one that supports subsets, requires even top/left boundaries.
            const int w = SkAlign2(W / divisor);
            const int h = SkAlign2(H / divisor);
            SkIRect subset;
            SkCodec::Options opts;
            opts.fSubset = &subset;
            SkBitmap subsetBm;
            // We will reuse pixel memory from bitmap.
            void* pixels = bitmap.getPixels();
            for (int x = 0; x < W; x += w) {
                for (int y = 0; y < H; y+= h) {
                    // Do not make the subset go off the edge of the image.
                    const int preScaleW = std::min(w, W - x);
                    const int preScaleH = std::min(h, H - y);
                    subset.setXYWH(x, y, preScaleW, preScaleH);
                    // And fscale
                    // FIXME: Should we have a version of getScaledDimensions that takes a subset
                    // into account?
                    decodeInfo = decodeInfo.makeWH(
                            std::max(1, SkScalarRoundToInt(preScaleW * fscale)),
                            std::max(1, SkScalarRoundToInt(preScaleH * fscale)));
                    size_t rowBytes = decodeInfo.minRowBytes();
                    if (!subsetBm.installPixels(decodeInfo, pixels, rowBytes)) {
                        SkDebugf("[terminated] Could not install pixels.\n");
                        return;
                    }
                    const SkCodec::Result result = codec->getPixels(decodeInfo, pixels, rowBytes,
                            &opts);
                    switch (result) {
                        case SkCodec::kSuccess:
                        case SkCodec::kIncompleteInput:
                        case SkCodec::kErrorInInput:
                            SkDebugf("okay\n");
                            break;
                        case SkCodec::kInvalidConversion:
                            if (0 == (x|y)) {
                                // First subset is okay to return unimplemented.
                                SkDebugf("[terminated] Incompatible colortype conversion\n");
                                return;
                            }
                            // If the first subset succeeded, a later one should not fail.
                            [[fallthrough]];
                        case SkCodec::kUnimplemented:
                            if (0 == (x|y)) {
                                // First subset is okay to return unimplemented.
                                SkDebugf("[terminated] subset codec not supported\n");
                                return;
                            }
                            // If the first subset succeeded, why would a later one fail?
                            [[fallthrough]];
                        default:
                            SkDebugf("[terminated] subset codec failed to decode (%d, %d, %d, %d) "
                                                  "with dimensions (%d x %d)\t error %d\n",
                                                  x, y, decodeInfo.width(), decodeInfo.height(),
                                                  W, H, result);
                            return;
                    }
                }
            }
            SkDebugf("[terminated] Success!\n");
            break;
        }
        case 4: { //kAnimated_Mode
            std::vector<SkCodec::FrameInfo> frameInfos = codec->getFrameInfo();
            if (frameInfos.size() == 0) {
                SkDebugf("[terminated] Not an animated image\n");
                break;
            }

            for (size_t i = 0; i < frameInfos.size(); i++) {
                options.fFrameIndex = i;
                auto result = codec->startIncrementalDecode(decodeInfo, bitmap.getPixels(),
                        bitmap.rowBytes(), &options);
                if (SkCodec::kSuccess != result) {
                    SkDebugf("[terminated] failed to start incremental decode "
                             "in frame %zu with error %d\n", i, result);
                    return;
                }

                result = codec->incrementalDecode();
                if (result == SkCodec::kIncompleteInput || result == SkCodec::kErrorInInput) {
                    SkDebugf("okay\n");
                    // Frames beyond this one will not decode.
                    break;
                }
                if (result == SkCodec::kSuccess) {
                    SkDebugf("okay - decoded frame %zu\n", i);
                } else {
                    SkDebugf("[terminated] incremental decode failed with "
                             "error %d\n", result);
                    return;
                }
            }
            SkDebugf("[terminated] Success!\n");
            break;
        }
        default:
            SkDebugf("[terminated] Mode not implemented yet\n");
    }

    dump_png(bitmap);
}

void FuzzSKP(const uint8_t *data, size_t size);

static void fuzz_skp(const sk_sp<SkData>& data) {
    FuzzSKP(data->bytes(), data->size());
    SkDebugf("[terminated] Finished SKP\n");
}

void FuzzColorspace(const uint8_t *data, size_t size);

static void fuzz_color_deserialize(const sk_sp<SkData>& data) {
    FuzzColorspace(data->bytes(), data->size());
    SkDebugf("[terminated] Finished SkColorspace\n");
}

void FuzzParsePath(const uint8_t *data, size_t size);

static void fuzz_parse_path(const sk_sp<SkData>& data) {
    FuzzParsePath(data->bytes(), data->size());
    SkDebugf("[terminated] parse_path didn't crash!\n");
}

void FuzzPathDeserialize(const uint8_t *data, size_t size);

static void fuzz_path_deserialize(const sk_sp<SkData>& data) {
    FuzzPathDeserialize(data->bytes(), data->size());
    SkDebugf("[terminated] path_deserialize didn't crash!\n");
}

bool FuzzRegionDeserialize(const uint8_t *data, size_t size);

static void fuzz_region_deserialize(const sk_sp<SkData>& data) {
    if (!FuzzRegionDeserialize(data->bytes(), data->size())) {
        SkDebugf("[terminated] Couldn't initialize SkRegion.\n");
        return;
    }
    SkDebugf("[terminated] Success! Initialized SkRegion.\n");
}

void FuzzTextBlobDeserialize(const uint8_t *data, size_t size);

static void fuzz_textblob_deserialize(const sk_sp<SkData>& data) {
    FuzzTextBlobDeserialize(data->bytes(), data->size());
    SkDebugf("[terminated] textblob didn't crash!\n");
}

void FuzzRegionSetPath(Fuzz* fuzz);

static void fuzz_region_set_path(const sk_sp<SkData>& data) {
    Fuzz fuzz(data->bytes(), data->size());
    FuzzRegionSetPath(&fuzz);
    SkDebugf("[terminated] region_set_path didn't crash!\n");
}

void FuzzImageFilterDeserialize(const uint8_t *data, size_t size);

static void fuzz_filter_fuzz(const sk_sp<SkData>& data) {
    FuzzImageFilterDeserialize(data->bytes(), data->size());
    SkDebugf("[terminated] filter_fuzz didn't crash!\n");
}

void FuzzSkMeshSpecification(const uint8_t *fuzzData, size_t fuzzSize);

static void fuzz_skmeshspecification(const sk_sp<SkData>& data) {
    FuzzSkMeshSpecification(data->bytes(), data->size());
    SkDebugf("[terminated] SkMeshSpecification::Make didn't crash!\n");
}

bool FuzzSkRuntimeBlender(const uint8_t *data, size_t size);

static void fuzz_skruntimeblender(const sk_sp<SkData>& data) {
    if (FuzzSkRuntimeBlender(data->bytes(), data->size())) {
        SkDebugf("[terminated] Success! Compiled and executed SkSL blender.\n");
    } else {
        SkDebugf("[terminated] Could not compile or execute SkSL blender.\n");
    }
}

bool FuzzSkRuntimeColorFilter(const uint8_t *data, size_t size);

static void fuzz_skruntimecolorfilter(const sk_sp<SkData>& data) {
    if (FuzzSkRuntimeColorFilter(data->bytes(), data->size())) {
        SkDebugf("[terminated] Success! Compiled and executed SkSL color filter.\n");
    } else {
        SkDebugf("[terminated] Could not compile or execute SkSL color filter.\n");
    }
}

bool FuzzSkRuntimeEffect(const uint8_t *data, size_t size);

static void fuzz_skruntimeeffect(const sk_sp<SkData>& data) {
    if (FuzzSkRuntimeEffect(data->bytes(), data->size())) {
        SkDebugf("[terminated] Success! Compiled and executed SkSL shader.\n");
    } else {
        SkDebugf("[terminated] Could not compile or execute SkSL shader.\n");
    }
}

bool FuzzSKSL2GLSL(const uint8_t *data, size_t size);

static void fuzz_sksl2glsl(const sk_sp<SkData>& data) {
    if (FuzzSKSL2GLSL(data->bytes(), data->size())) {
        SkDebugf("[terminated] Success! Compiled input to GLSL.\n");
    } else {
        SkDebugf("[terminated] Could not compile input to GLSL.\n");
    }
}

bool FuzzSKSL2Metal(const uint8_t *data, size_t size);

static void fuzz_sksl2metal(const sk_sp<SkData>& data) {
    if (FuzzSKSL2Metal(data->bytes(), data->size())) {
        SkDebugf("[terminated] Success! Compiled input to Metal.\n");
    } else {
        SkDebugf("[terminated] Could not compile input to Metal.\n");
    }
}

bool FuzzSKSL2Pipeline(const uint8_t *data, size_t size);

static void fuzz_sksl2pipeline(const sk_sp<SkData>& data) {
    if (FuzzSKSL2Pipeline(data->bytes(), data->size())) {
        SkDebugf("[terminated] Success! Compiled input to pipeline stage.\n");
    } else {
        SkDebugf("[terminated] Could not compile input to pipeline stage.\n");
    }
}

bool FuzzSKSL2SPIRV(const uint8_t *data, size_t size);

static void fuzz_sksl2spirv(const sk_sp<SkData>& data) {
    if (FuzzSKSL2SPIRV(data->bytes(), data->size())) {
        SkDebugf("[terminated] Success! Compiled input to SPIR-V.\n");
    } else {
        SkDebugf("[terminated] Could not compile input to SPIR-V.\n");
    }
}

bool FuzzSKSL2WGSL(const uint8_t *data, size_t size);

static void fuzz_sksl2wgsl(const sk_sp<SkData>& data) {
    if (FuzzSKSL2WGSL(data->bytes(), data->size())) {
        SkDebugf("[terminated] Success! Compiled input to WGSL.\n");
    } else {
        SkDebugf("[terminated] Could not compile input to WGSL.\n");
    }
}

void FuzzSkDescriptorDeserialize(const uint8_t *data, size_t size);

static void fuzz_skdescriptor_deserialize(const sk_sp<SkData>& data) {
    FuzzSkDescriptorDeserialize(data->bytes(), data->size());
    SkDebugf("[terminated] Did not crash while deserializing an SkDescriptor.\n");
}
