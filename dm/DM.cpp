/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "dm/DMJsonWriter.h"
#include "dm/DMSrcSink.h"
#include "include/codec/SkBmpDecoder.h"
#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "include/codec/SkGifDecoder.h"
#include "include/codec/SkIcoDecoder.h"
#include "include/codec/SkJpegDecoder.h"
#include "include/codec/SkPngDecoder.h"
#include "include/codec/SkWbmpDecoder.h"
#include "include/codec/SkWebpDecoder.h"
#include "include/core/SkBBHFactory.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkDocument.h"
#include "include/core/SkGraphics.h"
#include "src/base/SkHalf.h"
#include "src/base/SkLeanWindows.h"
#include "src/base/SkNoDestructor.h"
#include "src/base/SkSpinlock.h"
#include "src/base/SkTime.h"
#include "src/base/SkVx.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkMD5.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkStringUtils.h"
#include "src/core/SkTHash.h"
#include "src/core/SkTaskGroup.h"
#include "src/utils/SkOSPath.h"
#include "tests/Test.h"
#include "tests/TestHarness.h"
#include "tools/AutoreleasePool.h"
#include "tools/HashAndEncode.h"
#include "tools/ProcStats.h"
#include "tools/Resources.h"
#include "tools/TestFontDataProvider.h"
#include "tools/ToolUtils.h"
#include "tools/flags/CommonFlags.h"
#include "tools/flags/CommonFlagsConfig.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/ios_utils.h"
#include "tools/trace/ChromeTracingTracer.h"
#include "tools/trace/EventTracingPriv.h"
#include "tools/trace/SkDebugfTracer.h"

#include <memory>
#include <vector>

#include <stdlib.h>

#ifndef SK_BUILD_FOR_WIN
    #include <unistd.h>
#endif

#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK) && defined(SK_HAS_HEIF_LIBRARY)
    #include <binder/IPCThreadState.h>
#endif

#if defined(SK_BUILD_FOR_MAC)
    #include "include/utils/mac/SkCGUtils.h"
    #include "src/utils/mac/SkUniqueCFRef.h"
#endif

#if defined(SK_ENABLE_SVG)
    #include "modules/svg/include/SkSVGOpenTypeSVGDecoder.h"
#endif

#ifdef SK_CODEC_DECODES_AVIF
#include "include/codec/SkAvifDecoder.h"
#endif

#ifdef SK_HAS_HEIF_LIBRARY
#include "include/android/SkHeifDecoder.h"
#endif

#ifdef SK_CODEC_DECODES_JPEGXL
#include "include/codec/SkJpegxlDecoder.h"
#endif

#ifdef SK_CODEC_DECODES_RAW
#include "include/codec/SkRawDecoder.h"
#endif

using namespace skia_private;

extern bool gSkForceRasterPipelineBlitter;
extern bool gForceHighPrecisionRasterPipeline;
extern bool gCreateProtectedContext;

static DEFINE_string(src, "tests gm skp mskp lottie rive svg image colorImage",
                     "Source types to test.");
static DEFINE_bool(nameByHash, false,
                   "If true, write to FLAGS_writePath[0]/<hash>.png instead of "
                   "to FLAGS_writePath[0]/<config>/<sourceType>/<sourceOptions>/<name>.png");
static DEFINE_bool2(pathOpsExtended, x, false, "Run extended pathOps tests.");
static DEFINE_string(matrix, "1 0 0 1",
                    "2x2 scale+skew matrix to apply or upright when using "
                    "'matrix' or 'upright' in config.");

static DEFINE_string(skip, "",
        "Space-separated config/src/srcOptions/name quadruples to skip. "
        "'_' matches anything. '~' negates the match. E.g. \n"
        "'--skip gpu skp _ _' will skip all SKPs drawn into the gpu config.\n"
        "'--skip gpu skp _ _ 8888 gm _ aarects' will also skip the aarects GM on 8888.\n"
        "'--skip ~8888 svg _ svgparse_' blocks non-8888 SVGs that contain \"svgparse_\" in "
                                            "the name.");

static DEFINE_string2(readPath, r, "",
                      "If set check for equality with golden results in this directory.");
DEFINE_string2(writePath, w, "", "If set, write bitmaps here as .pngs.");


static DEFINE_string(uninterestingHashesFile, "",
        "File containing a list of uninteresting hashes. If a result hashes to something in "
        "this list, no image is written for that result.");

static DEFINE_int(shards, 1, "We're splitting source data into this many shards.");
static DEFINE_int(shard,  0, "Which shard do I run?");

static DEFINE_string(mskps, "", "Directory to read mskps from, or a single mskp file.");
static DEFINE_bool(forceRasterPipeline, false, "sets gSkForceRasterPipelineBlitter");
static DEFINE_bool(forceRasterPipelineHP, false, "sets gSkForceRasterPipelineBlitter and gForceHighPrecisionRasterPipeline");
static DEFINE_bool(createProtected, false, "attempts to create a protected backend context");

static DEFINE_string(bisect, "",
        "Pair of: SKP file to bisect, followed by an l/r bisect trail string (e.g., 'lrll'). The "
        "l/r trail specifies which half to keep at each step of a binary search through the SKP's "
        "paths. An empty string performs no bisect. Only the SkPaths are bisected; all other draws "
        "are thrown out. This is useful for finding a reduced repo case for path drawing bugs.");

static DEFINE_bool(ignoreSigInt, false, "ignore SIGINT signals during test execution");

static DEFINE_bool(checkF16, false, "Ensure that F16Norm pixels are clamped.");

static DEFINE_string(colorImages, "",
              "List of images and/or directories to decode with color correction. "
              "A directory with no images is treated as a fatal error.");

static DEFINE_bool2(veryVerbose, V, false, "tell individual tests to be verbose.");

static DEFINE_bool(cpu, true, "Run CPU-bound work?");
static DEFINE_bool(gpu, true, "Run GPU-bound work?");
static DEFINE_bool(graphite, true, "Run Graphite work?");
static DEFINE_bool(neverYieldToWebGPU, false, "Run Graphite with never-yield context option.");

static DEFINE_bool(dryRun, false,
                   "just print the tests that would be run, without actually running them.");

static DEFINE_string(images, "",
                     "List of images and/or directories to decode. A directory with no images"
                     " is treated as a fatal error.");

static DEFINE_bool(simpleCodec, false,
                   "Runs of a subset of the codec tests, "
                   "with no scaling or subsetting, always using the canvas color type.");

static DEFINE_string2(match, m, nullptr,
               "[~][^]substring[$] [...] of name to run.\n"
               "Multiple matches may be separated by spaces.\n"
               "~ causes a matching name to always be skipped\n"
               "^ requires the start of the name to match\n"
               "$ requires the end of the name to match\n"
               "^ and $ requires an exact match\n"
               "If a name does not match any list entry,\n"
               "it is skipped unless some list entry starts with ~");

static DEFINE_bool2(quiet, q, false, "if true, don't print status updates.");
static DEFINE_bool2(verbose, v, false, "enable verbose output from the test driver.");

static DEFINE_string(skps, "skps", "Directory to read skps from.");
static DEFINE_string(lotties, "lotties", "Directory to read (Bodymovin) jsons from.");
static DEFINE_string(svgs, "", "Directory to read SVGs from, or a single SVG file.");

static DEFINE_int_2(threads, j, -1,
               "Run threadsafe tests on a threadpool with this many extra threads, "
               "defaulting to one extra thread per core.");

static DEFINE_string(key, "",
                     "Space-separated key/value pairs to add to JSON identifying this builder.");
static DEFINE_string(properties, "",
                     "Space-separated key/value pairs to add to JSON identifying this run.");

static DEFINE_bool(rasterize_pdf, false, "Rasterize PDFs when possible.");

#if defined(__MSVC_RUNTIME_CHECKS)
#include <rtcapi.h>
int RuntimeCheckErrorFunc(int errorType, const char* filename, int linenumber,
                          const char* moduleName, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    SkDebugf("Line #%d\nFile: %s\nModule: %s\n",
             linenumber, filename ? filename : "Unknown", moduleName ? moduleName : "Unknwon");
    return 1;
}
#endif

using namespace DM;
using sk_gpu_test::GrContextFactory;
using sk_gpu_test::ContextInfo;
using skiatest::TestType;
#ifdef SK_GL
using sk_gpu_test::GLTestContext;
#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static FILE* gVLog;

static void vlog(const char* fmt, ...) SK_PRINTF_LIKE(1, 2);

static void vlog(const char* fmt, ...) {
    if (gVLog) {
        va_list args;
        va_start(args, fmt);
        vfprintf(gVLog, fmt, args);
        fflush(gVLog);
        va_end(args);
    }
}

static void info(const char* fmt, ...) SK_PRINTF_LIKE(1, 2);

static void info(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    if (gVLog) {
        va_list vlogArgs;
        va_copy(vlogArgs, args);
        vfprintf(gVLog, fmt, vlogArgs);
        fflush(gVLog);
        va_end(vlogArgs);
    }

    if (!FLAGS_quiet) {
        vprintf(fmt, args);
    }

    va_end(args);
}

static TArray<SkString>* gFailures = new TArray<SkString>;

static void fail(const SkString& err) {
    static SkSpinlock mutex;
    SkAutoSpinlock lock(mutex);
    SkDebugf("\n\nFAILURE: %s\n\n", err.c_str());
    gFailures->push_back(err);
}

struct Running {
    SkString   id;
    SkThreadID thread;

    void dump() const {
        info("\t%s\n", id.c_str());
    }
};

static void dump_json() {
    if (!FLAGS_writePath.isEmpty()) {
        JsonWriter::DumpJson(FLAGS_writePath[0], FLAGS_key, FLAGS_properties);
    }
}

static void register_codecs() {
    SkCodecs::Register(SkPngDecoder::Decoder());
    SkCodecs::Register(SkJpegDecoder::Decoder());
    SkCodecs::Register(SkWebpDecoder::Decoder());
    SkCodecs::Register(SkGifDecoder::Decoder());
    SkCodecs::Register(SkBmpDecoder::Decoder());
    SkCodecs::Register(SkWbmpDecoder::Decoder());
    SkCodecs::Register(SkIcoDecoder::Decoder());

#ifdef SK_CODEC_DECODES_AVIF
    SkCodecs::Register(SkAvifDecoder::Decoder());
#endif
#ifdef SK_HAS_HEIF_LIBRARY
    SkCodecs::Register(SkHeifDecoder::Decoder());
#endif
#ifdef SK_CODEC_DECODES_JPEGXL
    SkCodecs::Register(SkJpegxlDecoder::Decoder());
#endif
#ifdef SK_CODEC_DECODES_RAW
    SkCodecs::Register(SkRawDecoder::Decoder());
#endif
}

// We use a spinlock to make locking this in a signal handler _somewhat_ safe.
static SkSpinlock                      gMutex;
static int                             gPending;
static int                             gTotalCounts;
static double                          gLastUpdate;
static SkNoDestructor<TArray<Running>> gRunning;

static void done(const char* config, const char* src, const char* srcOptions, const char* name) {
    SkString id = SkStringPrintf("%s %s %s %s", config, src, srcOptions, name);
    vlog("[%d/%d] %s done\n", gTotalCounts - gPending, gTotalCounts, id.c_str());
    int pending;
    {
        SkAutoSpinlock lock(gMutex);
        for (int i = 0; i < gRunning->size(); i++) {
            if (gRunning->at(i).id == id) {
                gRunning->removeShuffle(i);
                break;
            }
        }
        pending = --gPending;
    }

    // We write out dm.json file and print out a progress update every once in a while.
    // Notice this also handles the final dm.json and progress update when pending == 0.
    double lastUpdate = gLastUpdate;
    double now = SkTime::GetNSecs();
    if (pending % 500 == 0 || now - lastUpdate > 4e9) {
        dump_json();

        int curr = sk_tools::getCurrResidentSetSizeMB(),
            peak = sk_tools::getMaxResidentSetSizeMB();

        SkAutoSpinlock lock(gMutex);

        // Since multiple threads can call `done`, it's possible that another thread has raced with
        // this one and printed an update since we did our progress check above. We detect this case
        // by checking to see if `gLastUpdate` has changed; if so, we don't need to print anything.
        if (lastUpdate == gLastUpdate) {
            info("\n[%d/%d] %dMB RAM, %dMB peak, %d queued, %d threads:\n\t%s  done\n",
                 gTotalCounts - gPending, gTotalCounts,
                 curr, peak, gPending - gRunning->size(), gRunning->size() + 1, id.c_str());
            for (auto& task : *gRunning) {
                task.dump();
            }
            gLastUpdate = now;
        }
    }
}

static void start(const char* config, const char* src, const char* srcOptions, const char* name) {
    SkString id = SkStringPrintf("%s %s %s %s", config, src, srcOptions, name);
    vlog("\tstart %s\n", id.c_str());
    SkAutoSpinlock lock(gMutex);
    gRunning->push_back({id,SkGetThreadID()});
}

static void find_culprit() {
    // Assumes gMutex is locked.
    SkThreadID thisThread = SkGetThreadID();
    for (auto& task : *gRunning) {
        if (task.thread == thisThread) {
            info("Likely culprit:\n");
            task.dump();
        }
    }
}

#if defined(SK_BUILD_FOR_WIN)
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

        SkAutoSpinlock lock(gMutex);

        const DWORD code = e->ExceptionRecord->ExceptionCode;
        info("\nCaught exception %lu", code);
        for (const auto& exception : kExceptions) {
            if (exception.code == code) {
                info(" %s", exception.name);
            }
        }
        info(", was running:\n");
        for (auto& task : *gRunning) {
            task.dump();
        }
        find_culprit();
        fflush(stdout);

        // Execute default exception handler... hopefully, exit.
        return EXCEPTION_EXECUTE_HANDLER;
    }

    static void setup_crash_handler() {
        SetUnhandledExceptionFilter(crash_handler);
    }
#else
    #include <signal.h>
    #if !defined(SK_BUILD_FOR_ANDROID)
        #include <execinfo.h>

#endif

    static constexpr int max_of() { return 0; }
    template <typename... Rest>
    static constexpr int max_of(int x, Rest... rest) {
        return x > max_of(rest...) ? x : max_of(rest...);
    }

    static void (*previous_handler[max_of(SIGABRT,SIGBUS,SIGFPE,SIGILL,SIGSEGV,SIGTERM)+1])(int);

    static void crash_handler(int sig) {
        SkAutoSpinlock lock(gMutex);

        info("\nCaught signal %d [%s] (%dMB RAM, peak %dMB), was running:\n",
             sig, strsignal(sig),
             sk_tools::getCurrResidentSetSizeMB(), sk_tools::getMaxResidentSetSizeMB());

        for (auto& task : *gRunning) {
            task.dump();
        }
        find_culprit();

    #if !defined(SK_BUILD_FOR_ANDROID)
        void* stack[128];
        int count = backtrace(stack, std::size(stack));
        char** symbols = backtrace_symbols(stack, count);
        info("\nStack trace:\n");
        for (int i = 0; i < count; i++) {
            info("    %s\n", symbols[i]);
        }
    #endif
        fflush(stdout);

        if (sig == SIGINT && FLAGS_ignoreSigInt) {
            info("Ignoring signal %d because of --ignoreSigInt.\n"
                 "This is probably a sign the bot is overloaded with work.\n", sig);
        } else {
            signal(sig, previous_handler[sig]);
            raise(sig);
        }
    }

    static void setup_crash_handler() {
        const int kSignals[] = { SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGINT, SIGSEGV, SIGTERM };
        for (int sig : kSignals) {
            previous_handler[sig] = signal(sig, crash_handler);
        }
    }
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
static THashSet<Gold, Gold::Hash>* gGold = new THashSet<Gold, Gold::Hash>;

static void add_gold(JsonWriter::BitmapResult r) {
    gGold->add(Gold(r.config, r.sourceType, r.sourceOptions, r.name, r.md5));
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

#if defined(SK_BUILD_FOR_WIN)
    static constexpr char kNewline[] = "\r\n";
#else
    static constexpr char kNewline[] = "\n";
#endif

static THashSet<SkString>* gUninterestingHashes = new THashSet<SkString>;

static void gather_uninteresting_hashes() {
    if (!FLAGS_uninterestingHashesFile.isEmpty()) {
        sk_sp<SkData> data(SkData::MakeFromFileName(FLAGS_uninterestingHashesFile[0]));
        if (!data) {
            info("WARNING: unable to read uninteresting hashes from %s\n",
                 FLAGS_uninterestingHashesFile[0]);
            return;
        }

        // Copy to a string to make sure SkStrSplit has a terminating \0 to find.
        SkString contents((const char*)data->data(), data->size());

        TArray<SkString> hashes;
        SkStrSplit(contents.c_str(), kNewline, &hashes);
        for (const SkString& hash : hashes) {
            gUninterestingHashes->add(hash);
        }
        info("FYI: loaded %d distinct uninteresting hashes from %d lines\n",
             gUninterestingHashes->count(), hashes.size());
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

struct TaggedSrc : public std::unique_ptr<Src> {
    SkString tag;
    SkString options;
};

struct TaggedSink : public std::unique_ptr<Sink> {
    SkString tag;
};

static constexpr bool kMemcpyOK = true;

static TArray<TaggedSrc,  kMemcpyOK>* gSrcs  = new TArray<TaggedSrc,  kMemcpyOK>;
static TArray<TaggedSink, kMemcpyOK>* gSinks = new TArray<TaggedSink, kMemcpyOK>;

static bool in_shard() {
    static int N = 0;
    return N++ % FLAGS_shards == FLAGS_shard;
}

static void push_src(const char* tag, ImplicitString options, Src* inSrc) {
    std::unique_ptr<Src> src(inSrc);
    if (in_shard() && FLAGS_src.contains(tag) &&
        !CommandLineFlags::ShouldSkip(FLAGS_match, src->name().c_str())) {
        TaggedSrc& s = gSrcs->push_back();
        s.reset(src.release());  // NOLINT(misc-uniqueptr-reset-release)
        s.tag = tag;
        s.options = options;
    }
}

static void push_codec_src(Path path, CodecSrc::Mode mode, CodecSrc::DstColorType dstColorType,
        SkAlphaType dstAlphaType, float scale) {
    if (FLAGS_simpleCodec) {
        const bool simple = CodecSrc::kCodec_Mode == mode || CodecSrc::kAnimated_Mode == mode;
        if (!simple || dstColorType != CodecSrc::kGetFromCanvas_DstColorType || scale != 1.0f) {
            // Only decode in the simple case.
            return;
        }
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
        case CodecSrc::kAnimated_Mode:
            folder.append("codec_animated");
            break;
    }

    switch (dstColorType) {
        case CodecSrc::kGrayscale_Always_DstColorType:
            folder.append("_kGray8");
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

#ifdef SK_ENABLE_ANDROID_UTILS
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

static void push_brd_srcs(Path path, bool gray) {
    if (gray) {
        // Only run grayscale to one sampleSize and Mode. Though interesting
        // to test grayscale, it should not reveal anything across various
        // sampleSizes and Modes
        // Arbitrarily choose Mode and sampleSize.
        push_brd_src(path, CodecSrc::kGrayscale_Always_DstColorType,
                     BRDSrc::kFullImage_Mode, 2);
    }

    // Test on a variety of sampleSizes, making sure to include:
    // - 2, 4, and 8, which are natively supported by jpeg
    // - multiples of 2 which are not divisible by 4 (analogous for 4)
    // - larger powers of two, since BRD clients generally use powers of 2
    // We will only produce output for the larger sizes on large images.
    const uint32_t sampleSizes[] = { 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 24, 32, 64 };

    const BRDSrc::Mode modes[] = { BRDSrc::kFullImage_Mode, BRDSrc::kDivisor_Mode, };

    for (uint32_t sampleSize : sampleSizes) {
        for (BRDSrc::Mode mode : modes) {
            push_brd_src(path, CodecSrc::kGetFromCanvas_DstColorType, mode, sampleSize);
        }
    }
}
#endif // SK_ENABLE_ANDROID_UTILS

static void push_codec_srcs(Path path) {
    sk_sp<SkData> encoded(SkData::MakeFromFileName(path.c_str()));
    if (!encoded) {
        info("Couldn't read %s.", path.c_str());
        return;
    }
    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(encoded);
    if (nullptr == codec) {
        info("Couldn't create codec for %s.", path.c_str());
        return;
    }

    // native scaling is only supported by WEBP and JPEG
    bool supportsNativeScaling = false;

    TArray<CodecSrc::Mode> nativeModes;
    nativeModes.push_back(CodecSrc::kCodec_Mode);
    nativeModes.push_back(CodecSrc::kCodecZeroInit_Mode);
    switch (codec->getEncodedFormat()) {
        case SkEncodedImageFormat::kJPEG:
            nativeModes.push_back(CodecSrc::kScanline_Mode);
            nativeModes.push_back(CodecSrc::kStripe_Mode);
            nativeModes.push_back(CodecSrc::kCroppedScanline_Mode);
            supportsNativeScaling = true;
            break;
        case SkEncodedImageFormat::kWEBP:
            nativeModes.push_back(CodecSrc::kSubset_Mode);
            supportsNativeScaling = true;
            break;
        case SkEncodedImageFormat::kDNG:
            break;
        default:
            nativeModes.push_back(CodecSrc::kScanline_Mode);
            break;
    }

    TArray<CodecSrc::DstColorType> colorTypes;
    colorTypes.push_back(CodecSrc::kGetFromCanvas_DstColorType);
    colorTypes.push_back(CodecSrc::kNonNative8888_Always_DstColorType);
    switch (codec->getInfo().colorType()) {
        case kGray_8_SkColorType:
            colorTypes.push_back(CodecSrc::kGrayscale_Always_DstColorType);
            break;
        default:
            break;
    }

    TArray<SkAlphaType> alphaModes;
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

    {
        std::vector<SkCodec::FrameInfo> frameInfos = codec->getFrameInfo();
        if (frameInfos.size() > 1) {
            for (auto dstCT : { CodecSrc::kNonNative8888_Always_DstColorType,
                    CodecSrc::kGetFromCanvas_DstColorType }) {
                for (auto at : { kUnpremul_SkAlphaType, kPremul_SkAlphaType }) {
                    push_codec_src(path, CodecSrc::kAnimated_Mode, dstCT, at, 1.0f);
                }
            }
            for (float scale : { .5f, .33f }) {
                push_codec_src(path, CodecSrc::kAnimated_Mode, CodecSrc::kGetFromCanvas_DstColorType,
                               kPremul_SkAlphaType, scale);
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

    const char* ext = strrchr(path.c_str(), '.');
    if (ext) {
        ext++;

        static const char* const rawExts[] = {
            "arw", "cr2", "dng", "nef", "nrw", "orf", "raf", "rw2", "pef", "srw",
            "ARW", "CR2", "DNG", "NEF", "NRW", "ORF", "RAF", "RW2", "PEF", "SRW",
        };
        for (const char* rawExt : rawExts) {
            if (0 == strcmp(rawExt, ext)) {
                // RAW is not supported by image generator (skbug.com/5079) or BRD.
                return;
            }
        }

#ifdef SK_ENABLE_ANDROID_UTILS
        static const char* const brdExts[] = {
            "jpg", "jpeg", "png", "webp",
            "JPG", "JPEG", "PNG", "WEBP",
        };
        for (const char* brdExt : brdExts) {
            if (0 == strcmp(brdExt, ext)) {
                bool gray = codec->getInfo().colorType() == kGray_8_SkColorType;
                push_brd_srcs(path, gray);
                break;
            }
        }
#endif
    }

    // Push image generator GPU test.
    push_image_gen_src(path, ImageGenSrc::kCodec_Mode, codec->getInfo().alphaType(), true);

    // Push image generator CPU tests.
    for (SkAlphaType alphaType : alphaModes) {
        push_image_gen_src(path, ImageGenSrc::kCodec_Mode, alphaType, false);

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
        if (SkEncodedImageFormat::kWEBP != codec->getEncodedFormat() &&
            SkEncodedImageFormat::kWBMP != codec->getEncodedFormat() &&
            kUnpremul_SkAlphaType != alphaType)
        {
            push_image_gen_src(path, ImageGenSrc::kPlatform_Mode, alphaType, false);
        }
#elif defined(SK_BUILD_FOR_WIN)
        if (SkEncodedImageFormat::kWEBP != codec->getEncodedFormat() &&
            SkEncodedImageFormat::kWBMP != codec->getEncodedFormat())
        {
            push_image_gen_src(path, ImageGenSrc::kPlatform_Mode, alphaType, false);
        }
#elif defined(SK_ENABLE_NDK_IMAGES)
        push_image_gen_src(path, ImageGenSrc::kPlatform_Mode, alphaType, false);
#endif
    }
}

template <typename T>
void gather_file_srcs(const CommandLineFlags::StringArray& flags,
                      const char*                          ext,
                      const char*                          src_name = nullptr) {
    if (!src_name) {
        // With the exception of Lottie files, the source name is the extension.
        src_name = ext;
    }

    for (int i = 0; i < flags.size(); i++) {
        const char* path = flags[i];
        if (sk_isdir(path)) {
            SkOSFile::Iter it(path, ext);
            for (SkString file; it.next(&file); ) {
                push_src(src_name, "", new T(SkOSPath::Join(path, file.c_str())));
            }
        } else {
            push_src(src_name, "", new T(path));
        }
    }
}

static bool gather_srcs() {
    for (const skiagm::GMFactory& f : skiagm::GMRegistry::Range()) {
        push_src("gm", "", new GMSrc(f));
    }

    gather_file_srcs<SKPSrc>(FLAGS_skps, "skp");
    gather_file_srcs<MSKPSrc>(FLAGS_mskps, "mskp");
#if defined(SK_ENABLE_SKOTTIE)
    gather_file_srcs<SkottieSrc>(FLAGS_lotties, "json", "lottie");
#endif
#if defined(SK_ENABLE_SVG)
    gather_file_srcs<SVGSrc>(FLAGS_svgs, "svg");
#endif
    if (!FLAGS_bisect.isEmpty()) {
        // An empty l/r trail string will draw all the paths.
        push_src("bisect", "",
                 new BisectSrc(FLAGS_bisect[0], FLAGS_bisect.size() > 1 ? FLAGS_bisect[1] : ""));
    }

    TArray<SkString> images;
    if (!CommonFlags::CollectImages(FLAGS_images, &images)) {
        return false;
    }

    for (const SkString& image : images) {
        push_codec_srcs(image);
    }

    TArray<SkString> colorImages;
    if (!CommonFlags::CollectImages(FLAGS_colorImages, &colorImages)) {
        return false;
    }

    for (const SkString& colorImage : colorImages) {
        push_src("colorImage", "decode_native", new ColorCodecSrc(colorImage, false));
        push_src("colorImage", "decode_to_dst", new ColorCodecSrc(colorImage,  true));
    }

    return true;
}

static void push_sink(const SkCommandLineConfig& config, Sink* s) {
    std::unique_ptr<Sink> sink(s);

    // Try a simple Src as a canary.  If it fails, skip this sink.
    struct : public Src {
        Result draw(SkCanvas* c, skiatest::graphite::GraphiteTestContext*) const override {
            c->drawRect(SkRect::MakeWH(1,1), SkPaint());
            return Result::Ok();
        }
        SkISize size() const override { return SkISize::Make(16, 16); }
        Name name() const override { return "justOneRect"; }
    } justOneRect;

    SkBitmap bitmap;
    SkDynamicMemoryWStream stream;
    SkString log;
    Result result = sink->draw(justOneRect, &bitmap, &stream, &log);
    if (result.isFatal()) {
        info("Could not run %s: %s\n", config.getTag().c_str(), result.c_str());
        exit(1);
    }

    TaggedSink& ts = gSinks->push_back();
    ts.reset(sink.release());  // NOLINT(misc-uniqueptr-reset-release)
    ts.tag = config.getTag();
}

static Sink* create_sink(const GrContextOptions& grCtxOptions, const SkCommandLineConfig* config) {
    if (FLAGS_gpu) {
        if (const SkCommandLineConfigGpu* gpuConfig = config->asConfigGpu()) {
            GrContextFactory testFactory(grCtxOptions);
            if (!testFactory.get(gpuConfig->getContextType(), gpuConfig->getContextOverrides())) {
                info("WARNING: can not create GPU context for config '%s'. "
                     "GM tests will be skipped.\n", gpuConfig->getTag().c_str());
                return nullptr;
            }
            if (gpuConfig->getTestPersistentCache()) {
                return new GPUPersistentCacheTestingSink(gpuConfig, grCtxOptions);
            } else if (gpuConfig->getTestPrecompile()) {
                return new GPUPrecompileTestingSink(gpuConfig, grCtxOptions);
            } else if (gpuConfig->getUseDDLSink()) {
                return new GPUDDLSink(gpuConfig, grCtxOptions);
            } else if (gpuConfig->getSlug()) {
                return new GPUSlugSink(gpuConfig, grCtxOptions);
            } else if (gpuConfig->getSerializedSlug()) {
                return new GPUSerializeSlugSink(gpuConfig, grCtxOptions);
            } else if (gpuConfig->getRemoteSlug()) {
                return new GPURemoteSlugSink(gpuConfig, grCtxOptions);
            } else {
                return new GPUSink(gpuConfig, grCtxOptions);
            }
        }
    }
#if defined(SK_GRAPHITE)
    if (FLAGS_graphite) {
        if (const SkCommandLineConfigGraphite *graphiteConfig = config->asConfigGraphite()) {
            return new GraphiteSink(graphiteConfig);
        }
    }
#endif
    if (const SkCommandLineConfigSvg* svgConfig = config->asConfigSvg()) {
        int pageIndex = svgConfig->getPageIndex();
        return new SVGSink(pageIndex);
    }

#define SINK(t, sink, ...) if (config->getBackend().equals(t)) return new sink(__VA_ARGS__)

    if (FLAGS_cpu) {
        SINK("r8",          RasterSink, kR8_unorm_SkColorType);
        SINK("565",         RasterSink, kRGB_565_SkColorType);
        SINK("4444",        RasterSink, kARGB_4444_SkColorType);
        SINK("8888",        RasterSink, kN32_SkColorType);
        SINK("rgba",        RasterSink, kRGBA_8888_SkColorType);
        SINK("bgra",        RasterSink, kBGRA_8888_SkColorType);
        SINK("rgbx",        RasterSink, kRGB_888x_SkColorType);
        SINK("1010102",     RasterSink, kRGBA_1010102_SkColorType);
        SINK("101010x",     RasterSink, kRGB_101010x_SkColorType);
        SINK("bgra1010102", RasterSink, kBGRA_1010102_SkColorType);
        SINK("bgr101010x",  RasterSink, kBGR_101010x_SkColorType);
        SINK("f16",         RasterSink, kRGBA_F16_SkColorType);
        SINK("f16norm",     RasterSink, kRGBA_F16Norm_SkColorType);
        SINK("f32",         RasterSink, kRGBA_F32_SkColorType);
        SINK("srgba",       RasterSink, kSRGBA_8888_SkColorType);

        SINK("pdf",         PDFSink, false, SK_ScalarDefaultRasterDPI);
        SINK("skp",         SKPSink);
        SINK("svg",         SVGSink);
        SINK("null",        NullSink);
        SINK("xps",         XPSSink);
        SINK("pdfa",        PDFSink, true,  SK_ScalarDefaultRasterDPI);
        SINK("pdf300",      PDFSink, false, 300);
        SINK("jsdebug",     DebugSink);
    }
#undef SINK
    return nullptr;
}

static Sink* create_via(const SkString& tag, Sink* wrapped) {
#define VIA(t, via, ...) if (tag.equals(t)) return new via(__VA_ARGS__)
#ifdef TEST_VIA_SVG
    VIA("svg",       ViaSVG,               wrapped);
#endif
    VIA("serialize", ViaSerialization,     wrapped);
    VIA("pic",       ViaPicture,           wrapped);
    VIA("rtblend",   ViaRuntimeBlend,      wrapped);

    if (FLAGS_matrix.size() == 4) {
        SkMatrix m;
        m.reset();
        m.setScaleX((SkScalar)atof(FLAGS_matrix[0]));
        m.setSkewX ((SkScalar)atof(FLAGS_matrix[1]));
        m.setSkewY ((SkScalar)atof(FLAGS_matrix[2]));
        m.setScaleY((SkScalar)atof(FLAGS_matrix[3]));
        VIA("matrix",  ViaMatrix,  m, wrapped);
        VIA("upright", ViaUpright, m, wrapped);
    }

#undef VIA

    return nullptr;
}

static bool gather_sinks(const GrContextOptions& grCtxOptions, bool defaultConfigs) {
    if (FLAGS_src.size() == 1 && FLAGS_src.contains("tests")) {
        // If we're just running tests skip trying to accumulate sinks. The 'justOneRect' test
        // can fail for protected contexts.
        return true;
    }

    SkCommandLineConfigArray configs;
    ParseConfigs(FLAGS_config, &configs);
    AutoreleasePool pool;
    for (int i = 0; i < configs.size(); i++) {
        const SkCommandLineConfig& config = *configs[i];
        Sink* sink = create_sink(grCtxOptions, &config);
        if (sink == nullptr) {
            info("Skipping config %s: Don't understand '%s'.\n", config.getTag().c_str(),
                 config.getTag().c_str());
            continue;
        }

        // The command line config already parsed out the via-style color space. Apply it here.
        sink->setColorSpace(config.refColorSpace());

        const TArray<SkString>& parts = config.getViaParts();
        for (int j = parts.size(); j-- > 0;) {
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
    if (configs.size() == 0 ||
        // If we're using the default configs, we're okay.
        defaultConfigs ||
        // Otherwise, make sure that all specified configs have become sinks.
        configs.size() == gSinks->size()) {
        return true;
    }
    return false;
}

static bool match(const char* needle, const char* haystack) {
    if ('~' == needle[0]) {
        return !match(needle + 1, haystack);
    }
    if (0 == strcmp("_", needle)) {
        return true;
    }
    return nullptr != strstr(haystack, needle);
}

static bool should_skip(const char* sink, const char* src,
                        const char* srcOptions, const char* name) {
    for (int i = 0; i < FLAGS_skip.size() - 3; i += 4) {
        if (match(FLAGS_skip[i+0], sink) &&
            match(FLAGS_skip[i+1], src) &&
            match(FLAGS_skip[i+2], srcOptions) &&
            match(FLAGS_skip[i+3], name)) {
            return true;
        }
    }
    return false;
}

// Even when a Task Sink reports to be non-threadsafe (e.g. GPU), we know things like
// .png encoding are definitely thread safe.  This lets us offload that work to CPU threads.
static SkTaskGroup* gDefinitelyThreadSafeWork = new SkTaskGroup;

// The finest-grained unit of work we can run: draw a single Src into a single Sink,
// report any errors, and perhaps write out the output: a .png of the bitmap, or a raw stream.
struct Task {
    Task(const TaggedSrc& src, const TaggedSink& sink) : src(src), sink(sink) {}
    const TaggedSrc&  src;
    const TaggedSink& sink;

    static void Run(const Task& task) {
        AutoreleasePool pool;
        SkString name = task.src->name();

        SkString log;
        if (!FLAGS_dryRun) {
            SkBitmap bitmap;
            SkDynamicMemoryWStream stream;
            start(task.sink.tag.c_str(), task.src.tag.c_str(),
                  task.src.options.c_str(), name.c_str());
            Result result = task.sink->draw(*task.src, &bitmap, &stream, &log);
            if (!log.isEmpty()) {
                info("%s %s %s %s:\n%s\n", task.sink.tag.c_str()
                                         , task.src.tag.c_str()
                                         , task.src.options.c_str()
                                         , name.c_str()
                                         , log.c_str());
            }
            if (result.isSkip()) {
                done(task.sink.tag.c_str(), task.src.tag.c_str(),
                     task.src.options.c_str(), name.c_str());
                return;
            }
            if (result.isFatal()) {
                fail(SkStringPrintf("%s %s %s %s: %s",
                                    task.sink.tag.c_str(),
                                    task.src.tag.c_str(),
                                    task.src.options.c_str(),
                                    name.c_str(),
                                    result.c_str()));
            }

            // We're likely switching threads here, so we must capture by value, [=] or [foo,bar].
            SkStreamAsset* data = stream.detachAsStream().release();
            gDefinitelyThreadSafeWork->add([task,name,bitmap,data]{
                std::unique_ptr<SkStreamAsset> ownedData(data);

                std::unique_ptr<HashAndEncode> hashAndEncode;

                SkString md5;
                if (!FLAGS_writePath.isEmpty() || !FLAGS_readPath.isEmpty()) {
                    SkMD5 hash;
                    if (data->getLength()) {
                        hash.writeStream(data, data->getLength());
                        data->rewind();
                    } else {
                        hashAndEncode = std::make_unique<HashAndEncode>(bitmap);
                        hashAndEncode->feedHash(&hash);
                    }
                    md5 = hash.finish().toLowercaseHexString();
                }

                if (!FLAGS_readPath.isEmpty() &&
                    !gGold->contains(Gold(task.sink.tag, task.src.tag,
                                          task.src.options, name, md5))) {
                    fail(SkStringPrintf("%s not found for %s %s %s %s in %s",
                                        md5.c_str(),
                                        task.sink.tag.c_str(),
                                        task.src.tag.c_str(),
                                        task.src.options.c_str(),
                                        name.c_str(),
                                        FLAGS_readPath[0]));
                }

                // Tests sometimes use a nullptr ext to indicate no image should be uploaded.
                const char* ext = task.sink->fileExtension();
                if (ext && !FLAGS_writePath.isEmpty()) {
                #if defined(SK_BUILD_FOR_MAC)
                    if (FLAGS_rasterize_pdf && SkString("pdf").equals(ext)) {
                        SkASSERT(data->getLength() > 0);

                        sk_sp<SkData> blob = SkData::MakeFromStream(data, data->getLength());

                        SkUniqueCFRef<CGDataProviderRef> provider{
                            CGDataProviderCreateWithData(nullptr,
                                                         blob->data(),
                                                         blob->size(),
                                                         nullptr)};

                        SkUniqueCFRef<CGPDFDocumentRef> pdf{
                            CGPDFDocumentCreateWithProvider(provider.get())};

                        CGPDFPageRef page = CGPDFDocumentGetPage(pdf.get(), 1);

                        CGRect bounds = CGPDFPageGetBoxRect(page, kCGPDFMediaBox);
                        const int w = (int)CGRectGetWidth (bounds),
                                  h = (int)CGRectGetHeight(bounds);

                        SkBitmap rasterized;
                        rasterized.allocPixels(SkImageInfo::Make(
                            w, h, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
                        rasterized.eraseColor(SK_ColorWHITE);

                        SkUniqueCFRef<CGColorSpaceRef> cs{CGColorSpaceCreateDeviceRGB()};
                        CGBitmapInfo info = kCGBitmapByteOrder32Big |
                                            (CGBitmapInfo)kCGImageAlphaPremultipliedLast;

                        SkUniqueCFRef<CGContextRef> ctx{CGBitmapContextCreate(
                            rasterized.getPixels(), w,h,8, rasterized.rowBytes(), cs.get(), info)};
                        CGContextDrawPDFPage(ctx.get(), page);

                        // Skip calling hashAndEncode->feedHash(SkMD5*)... we want the .pdf's hash.
                        hashAndEncode = std::make_unique<HashAndEncode>(rasterized);
                        WriteToDisk(task, md5, "png", nullptr,0, &rasterized, hashAndEncode.get());
                    } else
                #endif
                    if (data->getLength()) {
                        WriteToDisk(task, md5, ext, data, data->getLength(), nullptr, nullptr);
                        SkASSERT(bitmap.drawsNothing());
                    } else if (!bitmap.drawsNothing()) {
                        WriteToDisk(task, md5, ext, nullptr, 0, &bitmap, hashAndEncode.get());
                    }
                }

                SkPixmap pm;
                if (FLAGS_checkF16 && bitmap.colorType() == kRGBA_F16Norm_SkColorType &&
                        bitmap.peekPixels(&pm)) {
                    bool unclamped = false;
                    for (int y = 0; y < pm.height() && !unclamped; ++y)
                    for (int x = 0; x < pm.width() && !unclamped; ++x) {
                        skvx::float4 rgba = from_half(skvx::half4::Load(pm.addr64(x, y)));
                        float a = rgba[3];
                        if (a > 1.0f || any(rgba < 0.0f) || any(rgba > a)) {
                            SkDebugf("[%s] F16Norm pixel [%d, %d] unclamped: (%g, %g, %g, %g)\n",
                                     name.c_str(), x, y, rgba[0], rgba[1], rgba[2], rgba[3]);
                            unclamped = true;
                        }
                    }
                }
            });
        }
        done(task.sink.tag.c_str(), task.src.tag.c_str(), task.src.options.c_str(), name.c_str());
    }

    static SkString identify_gamut(SkColorSpace* cs) {
        if (!cs) {
            return SkString("untagged");
        }

        skcms_Matrix3x3 gamut;
        if (cs->toXYZD50(&gamut)) {
            auto eq = [](skcms_Matrix3x3 x, skcms_Matrix3x3 y) {
                for (int i = 0; i < 3; i++)
                for (int j = 0; j < 3; j++) {
                    if (x.vals[i][j] != y.vals[i][j]) { return false; }
                }
                return true;
            };

            if (eq(gamut, SkNamedGamut::kSRGB     )) { return SkString("sRGB"); }
            if (eq(gamut, SkNamedGamut::kAdobeRGB )) { return SkString("Adobe"); }
            if (eq(gamut, SkNamedGamut::kDisplayP3)) { return SkString("P3"); }
            if (eq(gamut, SkNamedGamut::kRec2020  )) { return SkString("2020"); }
            if (eq(gamut, SkNamedGamut::kXYZ      )) { return SkString("XYZ"); }
            if (eq(gamut,     gNarrow_toXYZD50    )) { return SkString("narrow"); }
            return SkString("other");
        }
        return SkString("non-XYZ");
    }

    static SkString identify_transfer_fn(SkColorSpace* cs) {
        if (!cs) {
            return SkString("untagged");
        }

        auto eq = [](skcms_TransferFunction x, skcms_TransferFunction y) {
            return x.g == y.g
                && x.a == y.a
                && x.b == y.b
                && x.c == y.c
                && x.d == y.d
                && x.e == y.e
                && x.f == y.f;
        };

        skcms_TransferFunction tf;
        cs->transferFn(&tf);
        switch (skcms_TransferFunction_getType(&tf)) {
            case skcms_TFType_sRGBish:
                if (tf.a == 1 && tf.b == 0 && tf.c == 0 && tf.d == 0 && tf.e == 0 && tf.f == 0) {
                    return SkStringPrintf("gamma %.3g", tf.g);
                }
                if (eq(tf, SkNamedTransferFn::kSRGB)) { return SkString("sRGB"); }
                if (eq(tf, SkNamedTransferFn::kRec2020)) { return SkString("2020"); }
                return SkStringPrintf("%.3g %.3g %.3g %.3g %.3g %.3g %.3g",
                                        tf.g, tf.a, tf.b, tf.c, tf.d, tf.e, tf.f);

            case skcms_TFType_PQish:
                if (eq(tf, SkNamedTransferFn::kPQ)) { return SkString("PQ"); }
                return SkStringPrintf("PQish %.3g %.3g %.3g %.3g %.3g %.3g",
                                      tf.a, tf.b, tf.c, tf.d, tf.e, tf.f);

            case skcms_TFType_HLGish:
                if (eq(tf, SkNamedTransferFn::kHLG)) { return SkString("HLG"); }
                return SkStringPrintf("HLGish %.3g %.3g %.3g %.3g %.3g (%.3g)",
                                      tf.a, tf.b, tf.c, tf.d, tf.e, tf.f+1);

            case skcms_TFType_HLGinvish: break;
            case skcms_TFType_Invalid: break;
        }
        return SkString("non-numeric");
    }

    static void WriteToDisk(const Task& task,
                            SkString md5,
                            const char* ext,
                            SkStream* data, size_t len,
                            const SkBitmap* bitmap,
                            const HashAndEncode* hashAndEncode) {

        // Determine whether or not the OldestSupportedSkpVersion extra_config is provided.
        bool isOldestSupportedSkp = false;
        for (int i = 1; i < FLAGS_key.size(); i += 2) {
            if (0 == strcmp(FLAGS_key[i-1], "extra_config") &&
                0 == strcmp(FLAGS_key[i], "OldestSupportedSkpVersion")) {
                isOldestSupportedSkp = true;
                break;
            }
        }

        JsonWriter::BitmapResult result;
        result.name          = task.src->name();
        result.config        = task.sink.tag;
        result.sourceType    = task.src.tag;
        // If the OldestSupportedSkpVersion extra_config is provided, override the "skp"
        // source_type with "old-skp". This has the effect of grouping the oldest supported SKPs in
        // a separate Gold corpus for easier triaging.
        if (isOldestSupportedSkp && 0 == strcmp(result.sourceType.c_str(), "skp")) {
            result.sourceType = "old-skp";
        }
        result.sourceOptions = task.src.options;
        result.ext           = ext;
        result.md5           = md5;
        if (bitmap) {
            result.gamut         = identify_gamut            (bitmap->colorSpace());
            result.transferFn    = identify_transfer_fn      (bitmap->colorSpace());
            result.colorType     = ToolUtils::colortype_name (bitmap->colorType());
            result.alphaType     = ToolUtils::alphatype_name (bitmap->alphaType());
            result.colorDepth    = ToolUtils::colortype_depth(bitmap->colorType());
        }
        JsonWriter::AddBitmapResult(result);

        // If an MD5 is uninteresting, we want it noted in the JSON file,
        // but don't want to dump it out as a .png (or whatever ext is).
        if (gUninterestingHashes->contains(md5)) {
            return;
        }

        const char* dir = FLAGS_writePath[0];
        SkString resources = GetResourcePath();
        if (0 == strcmp(dir, "@")) {  // Needed for iOS.
            dir = resources.c_str();
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
            if (0 != strcmp(task.src.options.c_str(), "")) {
              path = SkOSPath::Join(path.c_str(), task.src.options.c_str());
              sk_mkdir(path.c_str());
            }
            path = SkOSPath::Join(path.c_str(), task.src->name().c_str());
            path.append(".");
            path.append(ext);
        }

        SkFILEWStream file(path.c_str());
        if (!file.isValid()) {
            fail(SkStringPrintf("Can't open %s for writing.\n", path.c_str()));
            return;
        }
        if (bitmap) {
            SkASSERT(hashAndEncode);
            if (!hashAndEncode->encodePNG(&file,
                                          result.md5.c_str(),
                                          FLAGS_key,
                                          FLAGS_properties)) {
                fail(SkStringPrintf("Can't encode PNG to %s.\n", path.c_str()));
                return;
            }
        } else {
            if (!file.writeStream(data, len)) {
                fail(SkStringPrintf("Can't write to %s.\n", path.c_str()));
                return;
            }
        }
    }
};

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

// Unit tests don't fit so well into the Src/Sink model, so we give them special treatment.

static SkTDArray<skiatest::Test>* gCPUTests = new SkTDArray<skiatest::Test>;
static SkTDArray<skiatest::Test>* gGaneshTests = new SkTDArray<skiatest::Test>;
static SkTDArray<skiatest::Test>* gGraphiteTests = new SkTDArray<skiatest::Test>;

static void gather_tests() {
    if (!FLAGS_src.contains("tests")) {
        return;
    }
    for (const skiatest::Test& test : skiatest::TestRegistry::Range()) {
        if (!in_shard()) {
            continue;
        }
        if (CommandLineFlags::ShouldSkip(FLAGS_match, test.fName)) {
            continue;
        }
        if (test.fTestType == TestType::kGanesh && FLAGS_gpu) {
            gGaneshTests->push_back(test);
        } else if (test.fTestType == TestType::kGraphite && FLAGS_graphite) {
            gGraphiteTests->push_back(test);
        } else if (test.fTestType == TestType::kCPU && FLAGS_cpu) {
            gCPUTests->push_back(test);
        }
    }
}

struct DMReporter : public skiatest::Reporter {
    void reportFailed(const skiatest::Failure& failure) override {
        fail(failure.toString());
    }
    bool allowExtendedTest() const override {
        return FLAGS_pathOpsExtended;
    }
    bool verbose() const override { return FLAGS_veryVerbose; }
};

static void run_cpu_test(skiatest::Test test) {
    DMReporter reporter;
    if (!FLAGS_dryRun && !should_skip("_", "tests", "_", test.fName)) {
        skiatest::ReporterContext ctx(&reporter, SkString(test.fName));
        start("unit", "test", "", test.fName);
        test.cpu(&reporter);
    }
    done("unit", "test", "", test.fName);
}

static void run_ganesh_test(skiatest::Test test, const GrContextOptions& grCtxOptions) {
    DMReporter reporter;
    if (!FLAGS_dryRun && !should_skip("_", "tests", "_", test.fName)) {
        AutoreleasePool pool;
        GrContextOptions options = grCtxOptions;
        test.modifyGrContextOptions(&options);

        skiatest::ReporterContext ctx(&reporter, SkString(test.fName));
        start("unit", "test", "", test.fName);
        test.ganesh(&reporter, options);
    }
    done("unit", "test", "", test.fName);
}

static void run_graphite_test(skiatest::Test test, skiatest::graphite::TestOptions& options) {
    DMReporter reporter;
    if (!FLAGS_dryRun && !should_skip("_", "tests", "_", test.fName)) {
        AutoreleasePool pool;
        test.modifyGraphiteContextOptions(&options.fContextOptions);

        skiatest::ReporterContext ctx(&reporter, SkString(test.fName));
        start("unit", "test", "", test.fName);
        test.graphite(&reporter, options);
    }
    done("unit", "test", "", test.fName);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

TestHarness CurrentTestHarness() {
    return TestHarness::kDM;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

int main(int argc, char** argv) {
#if defined(__MSVC_RUNTIME_CHECKS)
    _RTC_SetErrorFunc(RuntimeCheckErrorFunc);
#endif
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK) && defined(SK_HAS_HEIF_LIBRARY)
    android::ProcessState::self()->startThreadPool();
#endif
    CommandLineFlags::Parse(argc, argv);

    initializeEventTracingForTools();

#if !defined(SK_BUILD_FOR_GOOGLE3) && defined(SK_BUILD_FOR_IOS)
    cd_Documents();
#endif
    setbuf(stdout, nullptr);
    setup_crash_handler();

    skiatest::SetFontTestDataDirectory();

    gSkForceRasterPipelineBlitter     = FLAGS_forceRasterPipelineHP || FLAGS_forceRasterPipeline;
    gForceHighPrecisionRasterPipeline = FLAGS_forceRasterPipelineHP;
    gCreateProtectedContext           = FLAGS_createProtected;

    // The bots like having a verbose.log to upload, so always touch the file even if --verbose.
    if (!FLAGS_writePath.isEmpty()) {
        sk_mkdir(FLAGS_writePath[0]);
        gVLog = fopen(SkOSPath::Join(FLAGS_writePath[0], "verbose.log").c_str(), "w");
    }
    if (FLAGS_verbose) {
        gVLog = stderr;
    }

    skiatest::graphite::TestOptions graphiteOptions;
    if (FLAGS_neverYieldToWebGPU) {
        graphiteOptions.fNeverYieldToWebGPU = true;
    }

    GrContextOptions grCtxOptions;
    CommonFlags::SetCtxOptions(&grCtxOptions);

    dump_json();  // It's handy for the bots to assume this is ~never missing.

    SkGraphics::Init();
#if defined(SK_ENABLE_SVG)
    SkGraphics::SetOpenTypeSVGDecoderFactory(SkSVGOpenTypeSVGDecoder::Make);
#endif
    SkTaskGroup::Enabler enabled(FLAGS_threads);
    register_codecs();

    if (nullptr == GetResourceAsData("images/color_wheel.png")) {
        info("Some resources are missing.  Do you need to set --resourcePath?\n");
    }
    gather_gold();
    gather_uninteresting_hashes();

    if (!gather_srcs()) {
        return 1;
    }
    bool defaultConfigs = true;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--config") == 0) {
            defaultConfigs = false;
            break;
        }
    }
    if (!gather_sinks(grCtxOptions, defaultConfigs)) {
        return 1;
    }
    gather_tests();
    int testCount = gCPUTests->size() + gGaneshTests->size() + gGraphiteTests->size();
    gPending = gSrcs->size() * gSinks->size() + testCount;
    gTotalCounts = gPending;
    gLastUpdate = SkTime::GetNSecs();
    info("%d srcs * %d sinks + %d tests == %d tasks\n",
         gSrcs->size(), gSinks->size(), testCount,
         gPending);

    // Kick off as much parallel work as we can, making note of any serial work we'll need to do.
    SkTaskGroup parallel;
    TArray<Task> serial;

    for (TaggedSink& sink : *gSinks) {
        for (TaggedSrc& src : *gSrcs) {
            if (src->veto(sink->flags()) ||
                should_skip(sink.tag.c_str(), src.tag.c_str(),
                            src.options.c_str(), src->name().c_str())) {
                SkAutoSpinlock lock(gMutex);
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
    }
    for (skiatest::Test& test : *gCPUTests) {
        parallel.add([test] { run_cpu_test(test); });
    }

    // With the parallel work running, run serial tasks and tests here on main thread.
    for (Task& task : serial) { Task::Run(task); }
    for (skiatest::Test& test : *gGaneshTests) { run_ganesh_test(test, grCtxOptions); }
    for (skiatest::Test& test : *gGraphiteTests) { run_graphite_test(test, graphiteOptions); }

    // Wait for any remaining parallel work to complete (including any spun off of serial tasks).
    parallel.wait();
    gDefinitelyThreadSafeWork->wait();

    // At this point we're back in single-threaded land.

    // We'd better have run everything.
    SkASSERT(gPending == 0);
    // Make sure we've flushed all our results to disk.
    dump_json();

    if (!gFailures->empty()) {
        info("Failures:\n");
        for (const SkString& fail : *gFailures) {
            info("\t%s\n", fail.c_str());
        }
        info("%d failures\n", gFailures->size());
        return 1;
    }

    SkGraphics::PurgeAllCaches();
    info("Finished!\n");

    return 0;
}
