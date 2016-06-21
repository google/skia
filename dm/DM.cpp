/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CrashHandler.h"
#include "DMJsonWriter.h"
#include "DMSrcSink.h"
#include "DMSrcSinkAndroid.h"
#include "ProcStats.h"
#include "Resources.h"
#include "SkBBHFactory.h"
#include "SkChecksum.h"
#include "SkCodec.h"
#include "SkColorPriv.h"
#include "SkColorSpace.h"
#include "SkCommonFlags.h"
#include "SkCommonFlagsConfig.h"
#include "SkData.h"
#include "SkFontMgr.h"
#include "SkGraphics.h"
#include "SkHalf.h"
#include "SkLeanWindows.h"
#include "SkMD5.h"
#include "SkMutex.h"
#include "SkOSFile.h"
#include "SkPM4fPriv.h"
#include "SkSpinlock.h"
#include "SkTHash.h"
#include "SkTaskGroup.h"
#include "SkThreadUtils.h"
#include "Test.h"
#include "Timer.h"
#include "picture_utils.h"
#include "sk_tool_utils.h"

#ifdef SK_PDF_IMAGE_STATS
extern void SkPDFImageDumpStats();
#endif

#include "png.h"

#include <stdlib.h>

#ifndef SK_BUILD_FOR_WIN32
    #include <unistd.h>
#endif

DEFINE_string(src, "tests gm skp image", "Source types to test.");
DEFINE_bool(nameByHash, false,
            "If true, write to FLAGS_writePath[0]/<hash>.png instead of "
            "to FLAGS_writePath[0]/<config>/<sourceType>/<sourceOptions>/<name>.png");
DEFINE_bool2(pathOpsExtended, x, false, "Run extended pathOps tests.");
DEFINE_string(matrix, "1 0 0 1",
              "2x2 scale+skew matrix to apply or upright when using "
              "'matrix' or 'upright' in config.");
DEFINE_bool(gpu_threading, false, "Allow GPU work to run on multiple threads?");

DEFINE_string(blacklist, "",
        "Space-separated config/src/srcOptions/name quadruples to blacklist.  '_' matches anything.  E.g. \n"
        "'--blacklist gpu skp _ _' will blacklist all SKPs drawn into the gpu config.\n"
        "'--blacklist gpu skp _ _ 8888 gm _ aarects' will also blacklist the aarects GM on 8888.");

DEFINE_string2(readPath, r, "", "If set check for equality with golden results in this directory.");

DEFINE_string(uninterestingHashesFile, "",
        "File containing a list of uninteresting hashes. If a result hashes to something in "
        "this list, no image is written for that result.");

DEFINE_int32(shards, 1, "We're splitting source data into this many shards.");
DEFINE_int32(shard,  0, "Which shard do I run?");

DEFINE_string(mskps, "", "Directory to read mskps from, or a single mskp file.");

using namespace DM;
using sk_gpu_test::GrContextFactory;
using sk_gpu_test::GLTestContext;
using sk_gpu_test::ContextInfo;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static const double kStartMs = SkTime::GetMSecs();

static FILE* gVLog;

template <typename... Args>
static void vlog(const char* fmt, Args&&... args) {
    if (gVLog) {
        fprintf(gVLog, "%s\t", HumanizeMs(SkTime::GetMSecs() - kStartMs).c_str());
        fprintf(gVLog, fmt, args...);
        fflush(gVLog);
    }
}

template <typename... Args>
static void info(const char* fmt, Args&&... args) {
    vlog(fmt, args...);
    if (!FLAGS_quiet) {
        printf(fmt, args...);
    }
}
static void info(const char* fmt) {
    if (!FLAGS_quiet) {
        printf("%s", fmt);  // Clang warns printf(fmt) is insecure.
    }
}

SK_DECLARE_STATIC_MUTEX(gFailuresMutex);
static SkTArray<SkString> gFailures;

static void fail(const SkString& err) {
    SkAutoMutexAcquire lock(gFailuresMutex);
    SkDebugf("\n\nFAILURE: %s\n\n", err.c_str());
    gFailures.push_back(err);
}


// We use a spinlock to make locking this in a signal handler _somewhat_ safe.
static SkSpinlock gMutex;
static int32_t            gPending;
static SkTArray<SkString> gRunning;

static void done(const char* config, const char* src, const char* srcOptions, const char* name) {
    SkString id = SkStringPrintf("%s %s %s %s", config, src, srcOptions, name);
    vlog("done  %s\n", id.c_str());
    int pending;
    {
        SkAutoMutexAcquire lock(gMutex);
        for (int i = 0; i < gRunning.count(); i++) {
            if (gRunning[i] == id) {
                gRunning.removeShuffle(i);
                break;
            }
        }
        pending = --gPending;
    }
    // We write our dm.json file every once in a while in case we crash.
    // Notice this also handles the final dm.json when pending == 0.
    if (pending % 500 == 0) {
        JsonWriter::DumpJson();
    }
}

static void start(const char* config, const char* src, const char* srcOptions, const char* name) {
    SkString id = SkStringPrintf("%s %s %s %s", config, src, srcOptions, name);
    vlog("start %s\n", id.c_str());
    SkAutoMutexAcquire lock(gMutex);
    gRunning.push_back(id);
}

static void print_status() {
    int curr = sk_tools::getCurrResidentSetSizeMB(),
        peak = sk_tools::getMaxResidentSetSizeMB();
    SkString elapsed = HumanizeMs(SkTime::GetMSecs() - kStartMs);

    SkAutoMutexAcquire lock(gMutex);
    info("\n%s elapsed, %d active, %d queued, %dMB RAM, %dMB peak\n",
         elapsed.c_str(), gRunning.count(), gPending - gRunning.count(), curr, peak);
    for (auto& task : gRunning) {
        info("\t%s\n", task.c_str());
    }
}

#if defined(SK_BUILD_FOR_WIN32)
    static LONG WINAPI crash_handler(EXCEPTION_POINTERS* e) {
        static const struct {
            const char* name;
            DWORD code;
        } kExceptions[] = {
        #define _(E) {#E, E}
            _(EXCEPTION_ACCESS_VIOLATION),
            _(EXCEPTION_BREAKPOINT),
            _(EXCEPTION_INT_DIVIDE_BY_ZERO),
            _(EXCEPTION_STACK_OVERFLOW),
            // TODO: more?
        #undef _
        };

        SkAutoMutexAcquire lock(gMutex);

        const DWORD code = e->ExceptionRecord->ExceptionCode;
        info("\nCaught exception %u", code);
        for (const auto& exception : kExceptions) {
            if (exception.code == code) {
                info(" %s", exception.name);
            }
        }
        info(", was running:\n");
        for (auto& task : gRunning) {
            info("\t%s\n", task.c_str());
        }
        fflush(stdout);

        // Execute default exception handler... hopefully, exit.
        return EXCEPTION_EXECUTE_HANDLER;
    }
    static void setup_crash_handler() { SetUnhandledExceptionFilter(crash_handler); }

#elif !defined(SK_BUILD_FOR_ANDROID)
    #include <execinfo.h>
    #include <signal.h>
    #include <stdlib.h>

    static void crash_handler(int sig) {
        SkAutoMutexAcquire lock(gMutex);

        info("\nCaught signal %d [%s], was running:\n", sig, strsignal(sig));
        for (auto& task : gRunning) {
            info("\t%s\n", task.c_str());
        }

        void* stack[64];
        int count = backtrace(stack, SK_ARRAY_COUNT(stack));
        char** symbols = backtrace_symbols(stack, count);
        info("\nStack trace:\n");
        for (int i = 0; i < count; i++) {
            info("    %s\n", symbols[i]);
        }
        fflush(stdout);

        _Exit(sig);
    }

    static void setup_crash_handler() {
        const int kSignals[] = { SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV };
        for (int sig : kSignals) {
            signal(sig, crash_handler);
        }
    }

#else  // Android
    static void setup_crash_handler() {}
#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

struct Gold : public SkString {
    Gold() : SkString("") {}
    Gold(const SkString& sink, const SkString& src,
         const SkString& srcOptions, const SkString& name,
         const SkString& md5)
        : SkString("") {
        this->append(sink);
        this->append(src);
        this->append(srcOptions);
        this->append(name);
        this->append(md5);
    }
    struct Hash {
        uint32_t operator()(const Gold& g) const {
            return SkGoodHash()((const SkString&)g);
        }
    };
};
static SkTHashSet<Gold, Gold::Hash> gGold;

static void add_gold(JsonWriter::BitmapResult r) {
    gGold.add(Gold(r.config, r.sourceType, r.sourceOptions, r.name, r.md5));
}

static void gather_gold() {
    if (!FLAGS_readPath.isEmpty()) {
        SkString path(FLAGS_readPath[0]);
        path.append("/dm.json");
        if (!JsonWriter::ReadJson(path.c_str(), add_gold)) {
            fail(SkStringPrintf("Couldn't read %s for golden results.", path.c_str()));
        }
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#if defined(SK_BUILD_FOR_WIN32)
    static const char* kNewline = "\r\n";
#else
    static const char* kNewline = "\n";
#endif

static SkTHashSet<SkString> gUninterestingHashes;

static void gather_uninteresting_hashes() {
    if (!FLAGS_uninterestingHashesFile.isEmpty()) {
        SkAutoTUnref<SkData> data(SkData::NewFromFileName(FLAGS_uninterestingHashesFile[0]));
        if (!data) {
            info("WARNING: unable to read uninteresting hashes from %s\n",
                 FLAGS_uninterestingHashesFile[0]);
            return;
        }
        SkTArray<SkString> hashes;
        SkStrSplit((const char*)data->data(), kNewline, &hashes);
        for (const SkString& hash : hashes) {
            gUninterestingHashes.add(hash);
        }
        info("FYI: loaded %d distinct uninteresting hashes from %d lines\n",
             gUninterestingHashes.count(), hashes.count());
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

struct TaggedSrc : public SkAutoTDelete<Src> {
    SkString tag;
    SkString options;
};

struct TaggedSink : public SkAutoTDelete<Sink> {
    SkString tag;
};

static const bool kMemcpyOK = true;

static SkTArray<TaggedSrc,  kMemcpyOK>  gSrcs;
static SkTArray<TaggedSink, kMemcpyOK> gSinks;

static bool in_shard() {
    static int N = 0;
    return N++ % FLAGS_shards == FLAGS_shard;
}

static void push_src(const char* tag, ImplicitString options, Src* s) {
    SkAutoTDelete<Src> src(s);
    if (in_shard() &&
        FLAGS_src.contains(tag) &&
        !SkCommandLineFlags::ShouldSkip(FLAGS_match, src->name().c_str())) {
        TaggedSrc& s = gSrcs.push_back();
        s.reset(src.release());
        s.tag = tag;
        s.options = options;
    }
}

static void push_codec_src(Path path, CodecSrc::Mode mode, CodecSrc::DstColorType dstColorType,
        SkAlphaType dstAlphaType, float scale) {
    if (FLAGS_simpleCodec) {
        if (mode != CodecSrc::kCodec_Mode || dstColorType != CodecSrc::kGetFromCanvas_DstColorType
                || scale != 1.0f)
            // Only decode in the simple case.
            return;
    }
    SkString folder;
    switch (mode) {
        case CodecSrc::kCodec_Mode:
            folder.append("codec");
            break;
        case CodecSrc::kCodecZeroInit_Mode:
            folder.append("codec_zero_init");
            break;
        case CodecSrc::kScanline_Mode:
            folder.append("scanline");
            break;
        case CodecSrc::kStripe_Mode:
            folder.append("stripe");
            break;
        case CodecSrc::kCroppedScanline_Mode:
            folder.append("crop");
            break;
        case CodecSrc::kSubset_Mode:
            folder.append("codec_subset");
            break;
    }

    switch (dstColorType) {
        case CodecSrc::kGrayscale_Always_DstColorType:
            folder.append("_kGray8");
            break;
        case CodecSrc::kIndex8_Always_DstColorType:
            folder.append("_kIndex8");
            break;
        case CodecSrc::kNonNative8888_Always_DstColorType:
            folder.append("_kNonNative");
            break;
        default:
            break;
    }

    switch (dstAlphaType) {
        case kPremul_SkAlphaType:
            folder.append("_premul");
            break;
        case kUnpremul_SkAlphaType:
            folder.append("_unpremul");
            break;
        default:
            break;
    }

    if (1.0f != scale) {
        folder.appendf("_%.3f", scale);
    }

    CodecSrc* src = new CodecSrc(path, mode, dstColorType, dstAlphaType, scale);
    push_src("image", folder, src);
}

static void push_android_codec_src(Path path, CodecSrc::DstColorType dstColorType,
        SkAlphaType dstAlphaType, int sampleSize) {
    SkString folder;
    folder.append("scaled_codec");

    switch (dstColorType) {
        case CodecSrc::kGrayscale_Always_DstColorType:
            folder.append("_kGray8");
            break;
        case CodecSrc::kIndex8_Always_DstColorType:
            folder.append("_kIndex8");
            break;
        case CodecSrc::kNonNative8888_Always_DstColorType:
            folder.append("_kNonNative");
            break;
        default:
            break;
    }

    switch (dstAlphaType) {
        case kPremul_SkAlphaType:
            folder.append("_premul");
            break;
        case kUnpremul_SkAlphaType:
            folder.append("_unpremul");
            break;
        default:
            break;
    }

    if (1 != sampleSize) {
        folder.appendf("_%.3f", 1.0f / (float) sampleSize);
    }

    AndroidCodecSrc* src = new AndroidCodecSrc(path, dstColorType, dstAlphaType, sampleSize);
    push_src("image", folder, src);
}

static void push_image_gen_src(Path path, ImageGenSrc::Mode mode, SkAlphaType alphaType, bool isGpu)
{
    SkString folder;
    switch (mode) {
        case ImageGenSrc::kCodec_Mode:
            folder.append("gen_codec");
            break;
        case ImageGenSrc::kPlatform_Mode:
            folder.append("gen_platform");
            break;
    }

    if (isGpu) {
        folder.append("_gpu");
    } else {
        switch (alphaType) {
            case kOpaque_SkAlphaType:
                folder.append("_opaque");
                break;
            case kPremul_SkAlphaType:
                folder.append("_premul");
                break;
            case kUnpremul_SkAlphaType:
                folder.append("_unpremul");
                break;
            default:
                break;
        }
    }

    ImageGenSrc* src = new ImageGenSrc(path, mode, alphaType, isGpu);
    push_src("image", folder, src);
}

static void push_codec_srcs(Path path) {
    SkAutoTUnref<SkData> encoded(SkData::NewFromFileName(path.c_str()));
    if (!encoded) {
        info("Couldn't read %s.", path.c_str());
        return;
    }
    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromData(encoded));
    if (nullptr == codec.get()) {
        info("Couldn't create codec for %s.", path.c_str());
        return;
    }

    // native scaling is only supported by WEBP and JPEG
    bool supportsNativeScaling = false;

    SkTArray<CodecSrc::Mode> nativeModes;
    nativeModes.push_back(CodecSrc::kCodec_Mode);
    nativeModes.push_back(CodecSrc::kCodecZeroInit_Mode);
    switch (codec->getEncodedFormat()) {
        case SkEncodedFormat::kJPEG_SkEncodedFormat:
            nativeModes.push_back(CodecSrc::kScanline_Mode);
            nativeModes.push_back(CodecSrc::kStripe_Mode);
            nativeModes.push_back(CodecSrc::kCroppedScanline_Mode);
            supportsNativeScaling = true;
            break;
        case SkEncodedFormat::kWEBP_SkEncodedFormat:
            nativeModes.push_back(CodecSrc::kSubset_Mode);
            supportsNativeScaling = true;
            break;
        case SkEncodedFormat::kDNG_SkEncodedFormat:
            break;
        default:
            nativeModes.push_back(CodecSrc::kScanline_Mode);
            break;
    }

    SkTArray<CodecSrc::DstColorType> colorTypes;
    colorTypes.push_back(CodecSrc::kGetFromCanvas_DstColorType);
    colorTypes.push_back(CodecSrc::kNonNative8888_Always_DstColorType);
    switch (codec->getInfo().colorType()) {
        case kGray_8_SkColorType:
            colorTypes.push_back(CodecSrc::kGrayscale_Always_DstColorType);
            if (kWBMP_SkEncodedFormat == codec->getEncodedFormat()) {
                colorTypes.push_back(CodecSrc::kIndex8_Always_DstColorType);
            }
            break;
        case kIndex_8_SkColorType:
            colorTypes.push_back(CodecSrc::kIndex8_Always_DstColorType);
            break;
        default:
            break;
    }

    SkTArray<SkAlphaType> alphaModes;
    alphaModes.push_back(kPremul_SkAlphaType);
    if (codec->getInfo().alphaType() != kOpaque_SkAlphaType) {
        alphaModes.push_back(kUnpremul_SkAlphaType);
    }

    for (CodecSrc::Mode mode : nativeModes) {
        for (CodecSrc::DstColorType colorType : colorTypes) {
            for (SkAlphaType alphaType : alphaModes) {
                // Only test kCroppedScanline_Mode when the alpha type is premul.  The test is
                // slow and won't be interestingly different with different alpha types.
                if (CodecSrc::kCroppedScanline_Mode == mode &&
                        kPremul_SkAlphaType != alphaType) {
                    continue;
                }

                push_codec_src(path, mode, colorType, alphaType, 1.0f);

                // Skip kNonNative on different native scales.  It won't be interestingly
                // different.
                if (supportsNativeScaling &&
                        CodecSrc::kNonNative8888_Always_DstColorType == colorType) {
                    // Native Scales
                    // SkJpegCodec natively supports scaling to the following:
                    for (auto scale : { 0.125f, 0.25f, 0.375f, 0.5f, 0.625f, 0.750f, 0.875f }) {
                        push_codec_src(path, mode, colorType, alphaType, scale);
                    }
                }
            }
        }
    }

    if (FLAGS_simpleCodec) {
        return;
    }

    const int sampleSizes[] = { 1, 2, 3, 4, 5, 6, 7, 8 };

    for (int sampleSize : sampleSizes) {
        for (CodecSrc::DstColorType colorType : colorTypes) {
            for (SkAlphaType alphaType : alphaModes) {
                // We can exercise all of the kNonNative support code in the swizzler with just a
                // few sample sizes.  Skip the rest.
                if (CodecSrc::kNonNative8888_Always_DstColorType == colorType && sampleSize > 3) {
                    continue;
                }

                push_android_codec_src(path, colorType, alphaType, sampleSize);
            }
        }
    }

    static const char* const rawExts[] = {
        "arw", "cr2", "dng", "nef", "nrw", "orf", "raf", "rw2", "pef", "srw",
        "ARW", "CR2", "DNG", "NEF", "NRW", "ORF", "RAF", "RW2", "PEF", "SRW",
    };

    // There is not currently a reason to test RAW images on image generator.
    // If we want to enable these tests, we will need to fix skbug.com/5079.
    for (const char* ext : rawExts) {
        if (path.endsWith(ext)) {
            return;
        }
    }

    // Push image generator GPU test.
    push_image_gen_src(path, ImageGenSrc::kCodec_Mode, codec->getInfo().alphaType(), true);

    // Push image generator CPU tests.
    for (SkAlphaType alphaType : alphaModes) {
        push_image_gen_src(path, ImageGenSrc::kCodec_Mode, alphaType, false);

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
        if (kWEBP_SkEncodedFormat != codec->getEncodedFormat() &&
            kWBMP_SkEncodedFormat != codec->getEncodedFormat() &&
            kUnpremul_SkAlphaType != alphaType)
        {
            push_image_gen_src(path, ImageGenSrc::kPlatform_Mode, alphaType, false);
        }
#elif defined(SK_BUILD_FOR_WIN)
        if (kWEBP_SkEncodedFormat != codec->getEncodedFormat() &&
            kWBMP_SkEncodedFormat != codec->getEncodedFormat())
        {
            push_image_gen_src(path, ImageGenSrc::kPlatform_Mode, alphaType, false);
        }
#endif
    }
}

static void push_brd_src(Path path, CodecSrc::DstColorType dstColorType, BRDSrc::Mode mode,
        uint32_t sampleSize) {
    SkString folder("brd_android_codec");
    switch (mode) {
        case BRDSrc::kFullImage_Mode:
            break;
        case BRDSrc::kDivisor_Mode:
            folder.append("_divisor");
            break;
        default:
            SkASSERT(false);
            return;
    }

    switch (dstColorType) {
        case CodecSrc::kGetFromCanvas_DstColorType:
            break;
        case CodecSrc::kIndex8_Always_DstColorType:
            folder.append("_kIndex");
            break;
        case CodecSrc::kGrayscale_Always_DstColorType:
            folder.append("_kGray");
            break;
        default:
            SkASSERT(false);
            return;
    }

    if (1 != sampleSize) {
        folder.appendf("_%.3f", 1.0f / (float) sampleSize);
    }

    BRDSrc* src = new BRDSrc(path, mode, dstColorType, sampleSize);
    push_src("image", folder, src);
}

static void push_brd_srcs(Path path) {
    // Only run Index8 and grayscale to one sampleSize and Mode. Though interesting
    // to test these color types, they should not reveal anything across various
    // sampleSizes and Modes
    for (auto type : { CodecSrc::kIndex8_Always_DstColorType,
                       CodecSrc::kGrayscale_Always_DstColorType }) {
        // Arbitrarily choose Mode and sampleSize.
        push_brd_src(path, type, BRDSrc::kFullImage_Mode, 2);
    }


    // Test on a variety of sampleSizes, making sure to include:
    // - 2, 4, and 8, which are natively supported by jpeg
    // - multiples of 2 which are not divisible by 4 (analogous for 4)
    // - larger powers of two, since BRD clients generally use powers of 2
    // We will only produce output for the larger sizes on large images.
    const uint32_t sampleSizes[] = { 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 24, 32, 64 };

    const BRDSrc::Mode modes[] = {
            BRDSrc::kFullImage_Mode,
            BRDSrc::kDivisor_Mode,
    };

    for (uint32_t sampleSize : sampleSizes) {
        for (BRDSrc::Mode mode : modes) {
            push_brd_src(path, CodecSrc::kGetFromCanvas_DstColorType, mode, sampleSize);
        }
    }
}

static bool brd_supported(const char* ext) {
    static const char* const exts[] = {
        "jpg", "jpeg", "png", "webp",
        "JPG", "JPEG", "PNG", "WEBP",
    };

    for (uint32_t i = 0; i < SK_ARRAY_COUNT(exts); i++) {
        if (0 == strcmp(exts[i], ext)) {
            return true;
        }
    }
    return false;
}

static bool gather_srcs() {
    for (const skiagm::GMRegistry* r = skiagm::GMRegistry::Head(); r; r = r->next()) {
        push_src("gm", "", new GMSrc(r->factory()));
    }
    for (int i = 0; i < FLAGS_skps.count(); i++) {
        const char* path = FLAGS_skps[i];
        if (sk_isdir(path)) {
            SkOSFile::Iter it(path, "skp");
            for (SkString file; it.next(&file); ) {
                push_src("skp", "", new SKPSrc(SkOSPath::Join(path, file.c_str())));
            }
        } else {
            push_src("skp", "", new SKPSrc(path));
        }
    }

    for (int i = 0; i < FLAGS_mskps.count(); i++) {
        const char* path = FLAGS_mskps[i];
        if (sk_isdir(path)) {
            SkOSFile::Iter it(path, "mskp");
            for (SkString file; it.next(&file);) {
                push_src("mskp", "",
                         new MSKPSrc(SkOSPath::Join(path, file.c_str())));
            }
        } else {
            push_src("mskp", "", new MSKPSrc(path));
        }
    }

    SkTArray<SkString> images;
    if (!CollectImages(FLAGS_images, &images)) {
        return false;
    }

    for (auto image : images) {
        push_codec_srcs(image);
        if (FLAGS_simpleCodec) {
            continue;
        }

        const char* ext = strrchr(image.c_str(), '.');
        if (ext && brd_supported(ext+1)) {
            push_brd_srcs(image);
        }
    }

    SkTArray<SkString> colorImages;
    if (!CollectImages(FLAGS_colorImages, &colorImages)) {
        return false;
    }

    for (auto colorImage : colorImages) {
        ColorCodecSrc* src = new ColorCodecSrc(colorImage, ColorCodecSrc::kBaseline_Mode);
        push_src("image", "color_codec_baseline", src);

        src = new ColorCodecSrc(colorImage, ColorCodecSrc::kDst_HPZR30w_Mode);
        push_src("image", "color_codec_HPZR30w", src);

        src = new ColorCodecSrc(colorImage, ColorCodecSrc::kDst_sRGB_Mode);
        push_src("image", "color_codec_sRGB", src);

#if defined(SK_TEST_QCMS)
        src = new ColorCodecSrc(colorImage, ColorCodecSrc::kQCMS_HPZR30w_Mode);
        push_src("image", "color_codec_QCMS_HPZR30w", src);
#endif
    }

    return true;
}

static void push_sink(const SkCommandLineConfig& config, Sink* s) {
    SkAutoTDelete<Sink> sink(s);

    // Try a simple Src as a canary.  If it fails, skip this sink.
    struct : public Src {
        Error draw(SkCanvas* c) const override {
            c->drawRect(SkRect::MakeWH(1,1), SkPaint());
            return "";
        }
        SkISize size() const override { return SkISize::Make(16, 16); }
        Name name() const override { return "justOneRect"; }
    } justOneRect;

    SkBitmap bitmap;
    SkDynamicMemoryWStream stream;
    SkString log;
    Error err = sink->draw(justOneRect, &bitmap, &stream, &log);
    if (err.isFatal()) {
        info("Could not run %s: %s\n", config.getTag().c_str(), err.c_str());
        exit(1);
    }

    TaggedSink& ts = gSinks.push_back();
    ts.reset(sink.release());
    ts.tag = config.getTag();
}

static bool gpu_supported() {
#if SK_SUPPORT_GPU
    return FLAGS_gpu;
#else
    return false;
#endif
}

static Sink* create_sink(const SkCommandLineConfig* config) {
#if SK_SUPPORT_GPU
    if (gpu_supported()) {
        if (const SkCommandLineConfigGpu* gpuConfig = config->asConfigGpu()) {
            GrContextFactory::ContextType contextType = gpuConfig->getContextType();
            GrContextFactory::ContextOptions contextOptions =
                    GrContextFactory::kNone_ContextOptions;
            if (gpuConfig->getUseNVPR()) {
                contextOptions = static_cast<GrContextFactory::ContextOptions>(
                    contextOptions | GrContextFactory::kEnableNVPR_ContextOptions);
            }
            if (SkColorAndColorSpaceAreGammaCorrect(gpuConfig->getColorType(),
                                                    gpuConfig->getColorSpace())) {
                contextOptions = static_cast<GrContextFactory::ContextOptions>(
                    contextOptions | GrContextFactory::kRequireSRGBSupport_ContextOptions);
            }
            GrContextFactory testFactory;
            if (!testFactory.get(contextType, contextOptions)) {
                info("WARNING: can not create GPU context for config '%s'. "
                     "GM tests will be skipped.\n", gpuConfig->getTag().c_str());
                return nullptr;
            }
            return new GPUSink(contextType, contextOptions, gpuConfig->getSamples(),
                               gpuConfig->getUseDIText(), gpuConfig->getColorType(),
                               sk_ref_sp(gpuConfig->getColorSpace()), FLAGS_gpu_threading);
        }
    }
#endif

#define SINK(t, sink, ...) if (config->getBackend().equals(t)) { return new sink(__VA_ARGS__); }

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    SINK("hwui",           HWUISink);
#endif

    if (FLAGS_cpu) {
        auto srgbColorSpace = SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named);

        SINK("565",  RasterSink, kRGB_565_SkColorType);
        SINK("8888", RasterSink, kN32_SkColorType);
        SINK("srgb", RasterSink, kN32_SkColorType, srgbColorSpace);
        SINK("f16",  RasterSink, kRGBA_F16_SkColorType);
        SINK("pdf",  PDFSink);
        SINK("skp",  SKPSink);
        SINK("svg",  SVGSink);
        SINK("null", NullSink);
        SINK("xps",  XPSSink);
        SINK("pdfa", PDFSink, true);
    }
#undef SINK
    return nullptr;
}

static Sink* create_via(const SkString& tag, Sink* wrapped) {
#define VIA(t, via, ...) if (tag.equals(t)) { return new via(__VA_ARGS__); }
    VIA("twice",     ViaTwice,             wrapped);
    VIA("serialize", ViaSerialization,     wrapped);
    VIA("pic",       ViaPicture,           wrapped);
    VIA("2ndpic",    ViaSecondPicture,     wrapped);
    VIA("sp",        ViaSingletonPictures, wrapped);
    VIA("tiles",     ViaTiles, 256, 256, nullptr,            wrapped);
    VIA("tiles_rt",  ViaTiles, 256, 256, new SkRTreeFactory, wrapped);

    if (FLAGS_matrix.count() == 4) {
        SkMatrix m;
        m.reset();
        m.setScaleX((SkScalar)atof(FLAGS_matrix[0]));
        m.setSkewX ((SkScalar)atof(FLAGS_matrix[1]));
        m.setSkewY ((SkScalar)atof(FLAGS_matrix[2]));
        m.setScaleY((SkScalar)atof(FLAGS_matrix[3]));
        VIA("matrix",  ViaMatrix,  m, wrapped);
        VIA("upright", ViaUpright, m, wrapped);
    }

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    VIA("androidsdk", ViaAndroidSDK, wrapped);
#endif

#undef VIA
    return nullptr;
}

static bool gather_sinks() {
    SkCommandLineConfigArray configs;
    ParseConfigs(FLAGS_config, &configs);
    for (int i = 0; i < configs.count(); i++) {
        const SkCommandLineConfig& config = *configs[i];
        Sink* sink = create_sink(&config);
        if (sink == nullptr) {
            info("Skipping config %s: Don't understand '%s'.\n", config.getTag().c_str(),
                 config.getTag().c_str());
            continue;
        }

        const SkTArray<SkString>& parts = config.getViaParts();
        for (int j = parts.count(); j-- > 0;) {
            const SkString& part = parts[j];
            Sink* next = create_via(part, sink);
            if (next == nullptr) {
                info("Skipping config %s: Don't understand '%s'.\n", config.getTag().c_str(),
                     part.c_str());
                delete sink;
                sink = nullptr;
                break;
            }
            sink = next;
        }
        if (sink) {
            push_sink(config, sink);
        }
    }

    // If no configs were requested (just running tests, perhaps?), then we're okay.
    // Otherwise, make sure that at least one sink was constructed correctly. This catches
    // the case of bots without a GPU being assigned GPU configs.
    return (configs.count() == 0) || (gSinks.count() > 0);
}

static bool dump_png(SkBitmap bitmap, const char* path, const char* md5) {
    const int w = bitmap.width(),
              h = bitmap.height();

    sk_sp<SkData> encodedBitmap = sk_tools::encode_bitmap_for_png(bitmap);
    if (encodedBitmap.get() == nullptr) {
        return false;
    }
    uint32_t* rgba = static_cast<uint32_t*>(encodedBitmap.get()->writable_data());

    // We don't need bitmap anymore.  Might as well drop our ref.
    bitmap.reset();

    FILE* f = fopen(path, "wb");
    if (!f) { return false; }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) {
        fclose(f);
        return false;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_write_struct(&png, &info);
        fclose(f);
        return false;
    }

    SkString description;
    description.append("Key: ");
    for (int i = 0; i < FLAGS_key.count(); i++) {
        description.appendf("%s ", FLAGS_key[i]);
    }
    description.append("Properties: ");
    for (int i = 0; i < FLAGS_properties.count(); i++) {
        description.appendf("%s ", FLAGS_properties[i]);
    }
    description.appendf("MD5: %s", md5);

    png_text text[2];
    text[0].key = (png_charp)"Author";
    text[0].text = (png_charp)"DM dump_png()";
    text[0].compression = PNG_TEXT_COMPRESSION_NONE;
    text[1].key = (png_charp)"Description";
    text[1].text = (png_charp)description.c_str();
    text[1].compression = PNG_TEXT_COMPRESSION_NONE;
    png_set_text(png, info, text, 2);

    png_init_io(png, f);
    png_set_IHDR(png, info, (png_uint_32)w, (png_uint_32)h, 8,
                 PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);
    for (int j = 0; j < h; j++) {
        png_bytep row = (png_bytep)(rgba + w*j);
        png_write_rows(png, &row, 1);
    }
    png_write_end(png, info);

    png_destroy_write_struct(&png, &info);
    fclose(f);
    return true;
}

static bool match(const char* needle, const char* haystack) {
    return 0 == strcmp("_", needle) || nullptr != strstr(haystack, needle);
}

static bool is_blacklisted(const char* sink, const char* src,
                           const char* srcOptions, const char* name) {
    for (int i = 0; i < FLAGS_blacklist.count() - 3; i += 4) {
        if (match(FLAGS_blacklist[i+0], sink) &&
            match(FLAGS_blacklist[i+1], src) &&
            match(FLAGS_blacklist[i+2], srcOptions) &&
            match(FLAGS_blacklist[i+3], name)) {
            return true;
        }
    }
    return false;
}

// Even when a Task Sink reports to be non-threadsafe (e.g. GPU), we know things like
// .png encoding are definitely thread safe.  This lets us offload that work to CPU threads.
static SkTaskGroup gDefinitelyThreadSafeWork;

// The finest-grained unit of work we can run: draw a single Src into a single Sink,
// report any errors, and perhaps write out the output: a .png of the bitmap, or a raw stream.
struct Task {
    Task(const TaggedSrc& src, const TaggedSink& sink) : src(src), sink(sink) {}
    const TaggedSrc&  src;
    const TaggedSink& sink;

    static void Run(const Task& task) {
        SkString name = task.src->name();

        SkString log;
        if (!FLAGS_dryRun) {
            SkBitmap bitmap;
            SkDynamicMemoryWStream stream;
            start(task.sink.tag.c_str(), task.src.tag.c_str(),
                  task.src.options.c_str(), name.c_str());
            Error err = task.sink->draw(*task.src, &bitmap, &stream, &log);
            if (!log.isEmpty()) {
                info("%s %s %s %s:\n%s\n", task.sink.tag.c_str()
                                         , task.src.tag.c_str()
                                         , task.src.options.c_str()
                                         , name.c_str()
                                         , log.c_str());
            }
            if (!err.isEmpty()) {
                if (err.isFatal()) {
                    fail(SkStringPrintf("%s %s %s %s: %s",
                                        task.sink.tag.c_str(),
                                        task.src.tag.c_str(),
                                        task.src.options.c_str(),
                                        name.c_str(),
                                        err.c_str()));
                } else {
                    done(task.sink.tag.c_str(), task.src.tag.c_str(),
                         task.src.options.c_str(), name.c_str());
                    return;
                }
            }

            // We're likely switching threads here, so we must capture by value, [=] or [foo,bar].
            SkStreamAsset* data = stream.detachAsStream();
            gDefinitelyThreadSafeWork.add([task,name,bitmap,data]{
                SkAutoTDelete<SkStreamAsset> ownedData(data);

                // Why doesn't the copy constructor do this when we have pre-locked pixels?
                bitmap.lockPixels();

                SkString md5;
                if (!FLAGS_writePath.isEmpty() || !FLAGS_readPath.isEmpty()) {
                    SkMD5 hash;
                    if (data->getLength()) {
                        hash.writeStream(data, data->getLength());
                        data->rewind();
                    } else {
                        // If we're BGRA (Linux, Windows), swizzle over to RGBA (Mac, Android).
                        // This helps eliminate multiple 0-pixel-diff hashes on gold.skia.org.
                        // (Android's general slow speed breaks the tie arbitrarily in RGBA's favor.)
                        // We might consider promoting 565 to RGBA too.
                        if (bitmap.colorType() == kBGRA_8888_SkColorType) {
                            SkBitmap swizzle;
                            SkAssertResult(bitmap.copyTo(&swizzle, kRGBA_8888_SkColorType));
                            hash.write(swizzle.getPixels(), swizzle.getSize());
                        } else {
                            hash.write(bitmap.getPixels(), bitmap.getSize());
                        }
                    }
                    SkMD5::Digest digest;
                    hash.finish(digest);
                    for (int i = 0; i < 16; i++) {
                        md5.appendf("%02x", digest.data[i]);
                    }
                }

                if (!FLAGS_readPath.isEmpty() &&
                    !gGold.contains(Gold(task.sink.tag, task.src.tag,
                                         task.src.options, name, md5))) {
                    fail(SkStringPrintf("%s not found for %s %s %s %s in %s",
                                        md5.c_str(),
                                        task.sink.tag.c_str(),
                                        task.src.tag.c_str(),
                                        task.src.options.c_str(),
                                        name.c_str(),
                                        FLAGS_readPath[0]));
                }

                if (!FLAGS_writePath.isEmpty()) {
                    const char* ext = task.sink->fileExtension();
                    if (data->getLength()) {
                        WriteToDisk(task, md5, ext, data, data->getLength(), nullptr);
                        SkASSERT(bitmap.drawsNothing());
                    } else if (!bitmap.drawsNothing()) {
                        WriteToDisk(task, md5, ext, nullptr, 0, &bitmap);
                    }
                }
            });
        }
        done(task.sink.tag.c_str(), task.src.tag.c_str(), task.src.options.c_str(), name.c_str());
    }

    static void WriteToDisk(const Task& task,
                            SkString md5,
                            const char* ext,
                            SkStream* data, size_t len,
                            const SkBitmap* bitmap) {
        bool gammaCorrect = false;
        if (bitmap) {
            gammaCorrect = SkImageInfoIsGammaCorrect(bitmap->info());
        }

        JsonWriter::BitmapResult result;
        result.name          = task.src->name();
        result.config        = task.sink.tag;
        result.sourceType    = task.src.tag;
        result.sourceOptions = task.src.options;
        result.ext           = ext;
        result.gammaCorrect  = gammaCorrect;
        result.md5           = md5;
        JsonWriter::AddBitmapResult(result);

        // If an MD5 is uninteresting, we want it noted in the JSON file,
        // but don't want to dump it out as a .png (or whatever ext is).
        if (gUninterestingHashes.contains(md5)) {
            return;
        }

        const char* dir = FLAGS_writePath[0];
        if (0 == strcmp(dir, "@")) {  // Needed for iOS.
            dir = FLAGS_resourcePath[0];
        }
        sk_mkdir(dir);

        SkString path;
        if (FLAGS_nameByHash) {
            path = SkOSPath::Join(dir, result.md5.c_str());
            path.append(".");
            path.append(ext);
            if (sk_exists(path.c_str())) {
                return;  // Content-addressed.  If it exists already, we're done.
            }
        } else {
            path = SkOSPath::Join(dir, task.sink.tag.c_str());
            sk_mkdir(path.c_str());
            path = SkOSPath::Join(path.c_str(), task.src.tag.c_str());
            sk_mkdir(path.c_str());
            if (strcmp(task.src.options.c_str(), "") != 0) {
              path = SkOSPath::Join(path.c_str(), task.src.options.c_str());
              sk_mkdir(path.c_str());
            }
            path = SkOSPath::Join(path.c_str(), task.src->name().c_str());
            path.append(".");
            path.append(ext);
        }

        if (bitmap) {
            if (!dump_png(*bitmap, path.c_str(), result.md5.c_str())) {
                fail(SkStringPrintf("Can't encode PNG to %s.\n", path.c_str()));
                return;
            }
        } else {
            SkFILEWStream file(path.c_str());
            if (!file.isValid()) {
                fail(SkStringPrintf("Can't open %s for writing.\n", path.c_str()));
                return;
            }
            if (!file.writeStream(data, len)) {
                fail(SkStringPrintf("Can't write to %s.\n", path.c_str()));
                return;
            }
        }
    }
};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

// Unit tests don't fit so well into the Src/Sink model, so we give them special treatment.

static SkTDArray<skiatest::Test> gParallelTests, gSerialTests;

static void gather_tests() {
    if (!FLAGS_src.contains("tests")) {
        return;
    }
    for (const skiatest::TestRegistry* r = skiatest::TestRegistry::Head(); r; r = r->next()) {
        if (!in_shard()) {
            continue;
        }
        // Despite its name, factory() is returning a reference to
        // link-time static const POD data.
        const skiatest::Test& test = r->factory();
        if (SkCommandLineFlags::ShouldSkip(FLAGS_match, test.name)) {
            continue;
        }
        if (test.needsGpu && gpu_supported()) {
            (FLAGS_gpu_threading ? gParallelTests : gSerialTests).push(test);
        } else if (!test.needsGpu && FLAGS_cpu) {
            gParallelTests.push(test);
        }
    }
}

static void run_test(skiatest::Test test) {
    struct : public skiatest::Reporter {
        void reportFailed(const skiatest::Failure& failure) override {
            fail(failure.toString());
            JsonWriter::AddTestFailure(failure);
        }
        bool allowExtendedTest() const override {
            return FLAGS_pathOpsExtended;
        }
        bool verbose() const override { return FLAGS_veryVerbose; }
    } reporter;

    if (!FLAGS_dryRun && !is_blacklisted("_", "tests", "_", test.name)) {
        start("unit", "test", "", test.name);
        GrContextFactory factory;
        test.proc(&reporter, &factory);
    }
    done("unit", "test", "", test.name);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

DEFINE_int32(status_sec, 15, "Print status this often (and if we crash).");

SkThread* start_status_thread() {
    auto thread = new SkThread([] (void*) {
        for (;;) {
            print_status();
        #if defined(SK_BUILD_FOR_WIN)
            Sleep(FLAGS_status_sec * 1000);
        #else
            sleep(FLAGS_status_sec);
        #endif
        }
    });
    thread->start();
    return thread;
}

#define PORTABLE_FONT_PREFIX "Toy Liberation "

static sk_sp<SkTypeface> create_from_name(const char familyName[], SkFontStyle style) {
    if (familyName && strlen(familyName) > sizeof(PORTABLE_FONT_PREFIX)
            && !strncmp(familyName, PORTABLE_FONT_PREFIX, sizeof(PORTABLE_FONT_PREFIX) - 1)) {
        return sk_tool_utils::create_portable_typeface(familyName, style);
    }
    return nullptr;
}

#undef PORTABLE_FONT_PREFIX

extern sk_sp<SkTypeface> (*gCreateTypefaceDelegate)(const char [], SkFontStyle );

int dm_main();
int dm_main() {
    setbuf(stdout, nullptr);
    setup_crash_handler();

    if (FLAGS_verbose) {
        gVLog = stderr;
    } else if (!FLAGS_writePath.isEmpty()) {
        sk_mkdir(FLAGS_writePath[0]);
        gVLog = freopen(SkOSPath::Join(FLAGS_writePath[0], "verbose.log").c_str(), "w", stderr);
    }

    JsonWriter::DumpJson();  // It's handy for the bots to assume this is ~never missing.
    SkAutoGraphics ag;
    SkTaskGroup::Enabler enabled(FLAGS_threads);
    gCreateTypefaceDelegate = &create_from_name;

    {
        SkString testResourcePath = GetResourcePath("color_wheel.png");
        SkFILEStream testResource(testResourcePath.c_str());
        if (!testResource.isValid()) {
            info("Some resources are missing.  Do you need to set --resourcePath?\n");
        }
    }
    gather_gold();
    gather_uninteresting_hashes();

    if (!gather_srcs()) {
        return 1;
    }
    if (!gather_sinks()) {
        return 1;
    }
    gather_tests();
    gPending = gSrcs.count() * gSinks.count() + gParallelTests.count() + gSerialTests.count();
    info("%d srcs * %d sinks + %d tests == %d tasks",
         gSrcs.count(), gSinks.count(), gParallelTests.count() + gSerialTests.count(), gPending);
    SkAutoTDelete<SkThread> statusThread(start_status_thread());

    // Kick off as much parallel work as we can, making note of any serial work we'll need to do.
    SkTaskGroup parallel;
    SkTArray<Task> serial;

    for (auto& sink : gSinks)
    for (auto&  src : gSrcs) {
        if (src->veto(sink->flags()) ||
            is_blacklisted(sink.tag.c_str(), src.tag.c_str(),
                           src.options.c_str(), src->name().c_str())) {
            SkAutoMutexAcquire lock(gMutex);
            gPending--;
            continue;
        }

        Task task(src, sink);
        if (src->serial() || sink->serial()) {
            serial.push_back(task);
        } else {
            parallel.add([task] { Task::Run(task); });
        }
    }
    for (auto test : gParallelTests) {
        parallel.add([test] { run_test(test); });
    }

    // With the parallel work running, run serial tasks and tests here on main thread.
    for (auto task : serial) { Task::Run(task); }
    for (auto test : gSerialTests) { run_test(test); }

    // Wait for any remaining parallel work to complete (including any spun off of serial tasks).
    parallel.wait();
    gDefinitelyThreadSafeWork.wait();

    // We'd better have run everything.
    SkASSERT(gPending == 0);
    // Make sure we've flushed all our results to disk.
    JsonWriter::DumpJson();

    // At this point we're back in single-threaded land.
    sk_tool_utils::release_portable_typefaces();

    if (gFailures.count() > 0) {
        info("Failures:\n");
        for (int i = 0; i < gFailures.count(); i++) {
            info("\t%s\n", gFailures[i].c_str());
        }
        info("%d failures\n", gFailures.count());
        return 1;
    }

#ifdef SK_PDF_IMAGE_STATS
    SkPDFImageDumpStats();
#endif  // SK_PDF_IMAGE_STATS

    print_status();
    info("Finished!\n");
    return 0;
}

// TODO: currently many GPU tests are declared outside SK_SUPPORT_GPU guards.
// Thus we export the empty RunWithGPUTestContexts when SK_SUPPORT_GPU=0.
namespace skiatest {

#if SK_SUPPORT_GPU
bool IsGLContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return kOpenGL_GrBackend == GrContextFactory::ContextTypeBackend(type);
}
bool IsVulkanContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return kVulkan_GrBackend == GrContextFactory::ContextTypeBackend(type);
}
bool IsRenderingGLContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return IsGLContextType(type) && GrContextFactory::IsRenderingContext(type);
}
bool IsNullGLContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return type == GrContextFactory::kNullGL_ContextType;
}
#else
bool IsGLContextType(int) { return false; }
bool IsVulkanContextType(int) { return false; }
bool IsRenderingGLContextType(int) { return false; }
bool IsNullGLContextType(int) { return false; }
#endif

void RunWithGPUTestContexts(GrContextTestFn* test, GrContextTypeFilterFn* contextTypeFilter,
                            Reporter* reporter, GrContextFactory* factory) {
#if SK_SUPPORT_GPU

    for (int typeInt = 0; typeInt < GrContextFactory::kContextTypeCnt; ++typeInt) {
        GrContextFactory::ContextType contextType = (GrContextFactory::ContextType) typeInt;
        ContextInfo ctxInfo = factory->getContextInfo(contextType);
        if (contextTypeFilter && !(*contextTypeFilter)(contextType)) {
            continue;
        }
        // Use "native" instead of explicitly trying OpenGL and OpenGL ES. Do not use GLES on,
        // desktop since tests do not account for not fixing http://skbug.com/2809
        if (contextType == GrContextFactory::kGL_ContextType ||
            contextType == GrContextFactory::kGLES_ContextType) {
            if (contextType != GrContextFactory::kNativeGL_ContextType) {
                continue;
            }
        }
        if (ctxInfo.grContext()) {
            (*test)(reporter, ctxInfo);
        }
        ctxInfo = factory->getContextInfo(contextType,
                                          GrContextFactory::kEnableNVPR_ContextOptions);
        if (ctxInfo.grContext()) {
            (*test)(reporter, ctxInfo);
        }
    }
#endif
}
} // namespace skiatest

#if !defined(SK_BUILD_FOR_IOS)
int main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
    return dm_main();
}
#endif
