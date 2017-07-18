/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Fuzz.h"
#include "SkCanvas.h"
#include "SkCodec.h"
#include "SkCommandLineFlags.h"
#include "SkData.h"
#include "SkFlattenableSerialization.h"
#include "SkImage.h"
#include "SkImageEncoder.h"
#include "SkImageFilter.h"
#include "SkMallocPixelRef.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPicture.h"
#include "SkRegion.h"
#include "SkStream.h"
#include "SkSurface.h"

#if SK_SUPPORT_GPU
#include "SkSLCompiler.h"
#endif

#include <iostream>
#include <signal.h>
#include "sk_tool_utils.h"


DEFINE_string2(bytes, b, "", "A path to a file or a directory. If a file, the "
        "contents will be used as the fuzz bytes. If a directory, all files "
        "in the directory will be used as fuzz bytes for the fuzzer, one at a "
        "time.");
DEFINE_string2(name, n, "", "If --type is 'api', fuzz the API with this name.");

DEFINE_string2(type, t, "api", "How to interpret --bytes, either 'image_scale'"
        ", 'image_mode', 'skp', 'icc', or 'api'.");
DEFINE_string2(dump, d, "", "If not empty, dump 'image*' or 'skp' types as a "
        "PNG with this name.");

static int printUsage() {
    SkDebugf("Usage: fuzz -t <type> -b <path/to/file> [-n api-to-fuzz]\n");
    return 1;
}
static int fuzz_file(const char* path);
static uint8_t calculate_option(SkData*);

static void fuzz_api(sk_sp<SkData>);
static void fuzz_color_deserialize(sk_sp<SkData>);
static void fuzz_icc(sk_sp<SkData>);
static void fuzz_img(sk_sp<SkData>, uint8_t, uint8_t);
static void fuzz_path_deserialize(sk_sp<SkData>);
static void fuzz_region_deserialize(sk_sp<SkData>);
static void fuzz_skp(sk_sp<SkData>);
static void fuzz_filter_fuzz(sk_sp<SkData>);

#if SK_SUPPORT_GPU
static void fuzz_sksl2glsl(sk_sp<SkData>);
#endif

int main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);

    const char* path = FLAGS_bytes.isEmpty() ? argv[0] : FLAGS_bytes[0];

    if (!sk_isdir(path)) {
        return fuzz_file(path);
    }

    SkOSFile::Iter it(path);
    for (SkString file; it.next(&file); ) {
        SkString p = SkOSPath::Join(path, file.c_str());
        SkDebugf("Fuzzing %s\n", p.c_str());
        int rv = fuzz_file(p.c_str());
        if (rv != 0) {
            return rv;
        }
    }
    return 0;
}

static int fuzz_file(const char* path) {
    sk_sp<SkData> bytes(SkData::MakeFromFileName(path));
    if (!bytes) {
        SkDebugf("Could not read %s\n", path);
        return 1;
    }

    uint8_t option = calculate_option(bytes.get());

    if (!FLAGS_type.isEmpty()) {
        if (0 == strcmp("api", FLAGS_type[0])) {
            fuzz_api(bytes);
            return 0;
        }
        if (0 == strcmp("color_deserialize", FLAGS_type[0])) {
            fuzz_color_deserialize(bytes);
            return 0;
        }
        if (0 == strcmp("icc", FLAGS_type[0])) {
            fuzz_icc(bytes);
            return 0;
        }
        if (0 == strcmp("image_scale", FLAGS_type[0])) {
            fuzz_img(bytes, option, 0);
            return 0;
        }
        if (0 == strcmp("image_mode", FLAGS_type[0])) {
            fuzz_img(bytes, 0, option);
            return 0;
        }
        if (0 == strcmp("path_deserialize", FLAGS_type[0])) {
            fuzz_path_deserialize(bytes);
            return 0;
        }
        if (0 == strcmp("region_deserialize", FLAGS_type[0])) {
            fuzz_region_deserialize(bytes);
            return 0;
        }
        if (0 == strcmp("skp", FLAGS_type[0])) {
            fuzz_skp(bytes);
            return 0;
        }
        if (0 == strcmp("filter_fuzz", FLAGS_type[0])) {
            fuzz_filter_fuzz(bytes);
            return 0;
        }
#if SK_SUPPORT_GPU
        if (0 == strcmp("sksl2glsl", FLAGS_type[0])) {
            fuzz_sksl2glsl(bytes);
            return 0;
        }
#endif
    }
    return printUsage();
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

static void fuzz_api(sk_sp<SkData> bytes) {
    const char* name = FLAGS_name.isEmpty() ? "" : FLAGS_name[0];

    for (auto r = sk_tools::Registry<Fuzzable>::Head(); r; r = r->next()) {
        auto fuzzable = r->factory();
        if (0 == strcmp(name, fuzzable.name)) {
            SkDebugf("Fuzzing %s...\n", fuzzable.name);
            Fuzz fuzz(std::move(bytes));
            fuzzable.fn(&fuzz);
            SkDebugf("[terminated] Success!\n");
            return;
        }
    }

    SkDebugf("When using --type api, please choose an API to fuzz with --name/-n:\n");
    for (auto r = sk_tools::Registry<Fuzzable>::Head(); r; r = r->next()) {
        auto fuzzable = r->factory();
        SkDebugf("\t%s\n", fuzzable.name);
    }
}

static void dump_png(SkBitmap bitmap) {
    if (!FLAGS_dump.isEmpty()) {
        sk_tool_utils::EncodeImageToFile(FLAGS_dump[0], bitmap, SkEncodedImageFormat::kPNG, 100);
        SkDebugf("Dumped to %s\n", FLAGS_dump[0]);
    }
}

static void fuzz_img(sk_sp<SkData> bytes, uint8_t scale, uint8_t mode) {
    // We can scale 1x, 2x, 4x, 8x, 16x
    scale = scale % 5;
    float fscale = (float)pow(2.0f, scale);
    SkDebugf("Scaling factor: %f\n", fscale);

    // We have 5 different modes of decoding.
    mode = mode % 5;
    SkDebugf("Mode: %d\n", mode);

    // This is mostly copied from DMSrcSink's CodecSrc::draw method.
    SkDebugf("Decoding\n");
    std::unique_ptr<SkCodec> codec(SkCodec::NewFromData(bytes));
    if (nullptr == codec.get()) {
        SkDebugf("[terminated] Couldn't create codec.\n");
        return;
    }

    SkImageInfo decodeInfo = codec->getInfo();
    SkISize size = codec->getScaledDimensions(fscale);
    decodeInfo = decodeInfo.makeWH(size.width(), size.height());

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
            switch (codec->getScanlineOrder()) {
                case SkCodec::kTopDown_SkScanlineOrder:
                case SkCodec::kBottomUp_SkScanlineOrder:
                    // We do not need to check the return value.  On an incomplete
                    // image, memory will be filled with a default value.
                    codec->getScanlines(dst, height, rowBytes);
                    break;
            }
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
                const int linesToSkip = SkTMin(stripeHeight, height - i * stripeHeight);
                codec->skipScanlines(linesToSkip);

                // Read a stripe
                const int startY = (i + 1) * stripeHeight;
                const int linesToRead = SkTMin(stripeHeight, height - startY);
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
                const int linesToRead = SkTMin(stripeHeight, height - startY);
                codec->getScanlines(bitmap.getAddr(0, startY), linesToRead, bitmap.rowBytes());

                // Skip a stripe
                const int linesToSkip = SkTMin(stripeHeight, height - (i + 1) * stripeHeight);
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
            // Keep track of left and top (for drawing subsetBm into canvas). We could use
            // fscale * x and fscale * y, but we want integers such that the next subset will start
            // where the last one ended. So we'll add decodeInfo.width() and height().
            int left = 0;
            for (int x = 0; x < W; x += w) {
                int top = 0;
                for (int y = 0; y < H; y+= h) {
                    // Do not make the subset go off the edge of the image.
                    const int preScaleW = SkTMin(w, W - x);
                    const int preScaleH = SkTMin(h, H - y);
                    subset.setXYWH(x, y, preScaleW, preScaleH);
                    // And fscale
                    // FIXME: Should we have a version of getScaledDimensions that takes a subset
                    // into account?
                    decodeInfo = decodeInfo.makeWH(
                            SkTMax(1, SkScalarRoundToInt(preScaleW * fscale)),
                            SkTMax(1, SkScalarRoundToInt(preScaleH * fscale)));
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
                            // fall through to failure
                        case SkCodec::kUnimplemented:
                            if (0 == (x|y)) {
                                // First subset is okay to return unimplemented.
                                SkDebugf("[terminated] subset codec not supported\n");
                                return;
                            }
                            // If the first subset succeeded, why would a later one fail?
                            // fall through to failure
                        default:
                            SkDebugf("[terminated] subset codec failed to decode (%d, %d, %d, %d) "
                                                  "with dimensions (%d x %d)\t error %d\n",
                                                  x, y, decodeInfo.width(), decodeInfo.height(),
                                                  W, H, result);
                            return;
                    }
                    // translate by the scaled height.
                    top += decodeInfo.height();
                }
                // translate by the scaled width.
                left += decodeInfo.width();
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
                             "in frame %d with error %d\n", i, result);
                    return;
                }

                result = codec->incrementalDecode();
                if (result == SkCodec::kIncompleteInput || result == SkCodec::kErrorInInput) {
                    SkDebugf("okay\n");
                    // Frames beyond this one will not decode.
                    break;
                }
                if (result == SkCodec::kSuccess) {
                    SkDebugf("okay - decoded frame %d\n", i);
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

static void fuzz_skp(sk_sp<SkData> bytes) {
    SkMemoryStream stream(bytes);
    SkDebugf("Decoding\n");
    sk_sp<SkPicture> pic(SkPicture::MakeFromStream(&stream));
    if (!pic) {
        SkDebugf("[terminated] Couldn't decode as a picture.\n");
        return;
    }
    SkDebugf("Rendering\n");
    SkBitmap bitmap;
    if (!FLAGS_dump.isEmpty()) {
        SkIRect size = pic->cullRect().roundOut();
        bitmap.allocN32Pixels(size.width(), size.height());
    }
    SkCanvas canvas(bitmap);
    canvas.drawPicture(pic);
    SkDebugf("[terminated] Success! Decoded and rendered an SkPicture!\n");
    dump_png(bitmap);
}

static void fuzz_icc(sk_sp<SkData> bytes) {
    sk_sp<SkColorSpace> space(SkColorSpace::MakeICC(bytes->data(), bytes->size()));
    if (!space) {
        SkDebugf("[terminated] Couldn't decode ICC.\n");
        return;
    }
    SkDebugf("[terminated] Success! Decoded ICC.\n");
}

static void fuzz_color_deserialize(sk_sp<SkData> bytes) {
    sk_sp<SkColorSpace> space(SkColorSpace::Deserialize(bytes->data(), bytes->size()));
    if (!space) {
        SkDebugf("[terminated] Couldn't deserialize Colorspace.\n");
        return;
    }
    SkDebugf("[terminated] Success! deserialized Colorspace.\n");
}

static void fuzz_path_deserialize(sk_sp<SkData> bytes) {
    SkPath path;
    if (!path.readFromMemory(bytes->data(), bytes->size())) {
        SkDebugf("[terminated] Couldn't initialize SkPath.\n");
        return;
    }
    auto s = SkSurface::MakeRasterN32Premul(1024, 1024);
    s->getCanvas()->drawPath(path, SkPaint());
    SkDebugf("[terminated] Success! Initialized SkPath.\n");
}

static void fuzz_region_deserialize(sk_sp<SkData> bytes) {
    SkRegion region;
    if (!region.readFromMemory(bytes->data(), bytes->size())) {
        SkDebugf("[terminated] Couldn't initialize SkRegion.\n");
        return;
    }
    region.computeRegionComplexity();
    region.isComplex();
    SkRegion r2;
    if (region == r2) {
        region.contains(0,0);
    } else {
        region.contains(1,1);
    }
    auto s = SkSurface::MakeRasterN32Premul(1024, 1024);
    s->getCanvas()->drawRegion(region, SkPaint());
    SkDEBUGCODE(region.validate());
    SkDebugf("[terminated] Success! Initialized SkRegion.\n");
}

static void fuzz_filter_fuzz(sk_sp<SkData> bytes) {

    const int BitmapSize = 24;
    SkBitmap bitmap;
    bitmap.allocN32Pixels(BitmapSize, BitmapSize);
    SkCanvas canvas(bitmap);
    canvas.clear(0x00000000);

    sk_sp<SkImageFilter> flattenable = SkValidatingDeserializeImageFilter(
        bytes->data(), bytes->size());

    // Adding some info, but the test passed if we got here without any trouble
    if (flattenable != NULL) {
        SkDebugf("Valid stream detected.\n");
        // Let's see if using the filters can cause any trouble...
        SkPaint paint;
        paint.setImageFilter(flattenable);
        canvas.save();
        canvas.clipRect(SkRect::MakeXYWH(
            0, 0, SkIntToScalar(BitmapSize), SkIntToScalar(BitmapSize)));

        // This call shouldn't crash or cause ASAN to flag any memory issues
        // If nothing bad happens within this call, everything is fine
        canvas.drawBitmap(bitmap, 0, 0, &paint);

        SkDebugf("Filter DAG rendered successfully\n");
        canvas.restore();
    } else {
        SkDebugf("Invalid stream detected.\n");
    }

    SkDebugf("[terminated] Done\n");
}

#if SK_SUPPORT_GPU
static void fuzz_sksl2glsl(sk_sp<SkData> bytes) {
    SkSL::Compiler compiler;
    SkSL::String output;
    SkSL::Program::Settings settings;
    sk_sp<GrShaderCaps> caps = SkSL::ShaderCapsFactory::Default();
    settings.fCaps = caps.get();
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(SkSL::Program::kFragment_Kind,
                                                              SkString((const char*) bytes->data()),
                                                              settings);
    if (!program || !compiler.toGLSL(*program, &output)) {
        SkDebugf("[terminated] Couldn't compile input.\n");
        return;
    }
    SkDebugf("[terminated] Success! Compiled input.\n");
}
#endif

Fuzz::Fuzz(sk_sp<SkData> bytes) : fBytes(bytes), fNextByte(0) {}

void Fuzz::signalBug() { SkDebugf("Signal bug\n"); raise(SIGSEGV); }

size_t Fuzz::size() { return fBytes->size(); }

bool Fuzz::exhausted() {
    return fBytes->size() == fNextByte;
}
