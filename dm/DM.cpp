/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DMJsonWriter.h"
#include "DMSrcSink.h"
#include "ProcStats.h"
#include "Resources.h"
#include "SkBBHFactory.h"
#include "SkChecksum.h"
#include "SkChromeTracingTracer.h"
#include "SkCodec.h"
#include "SkColorPriv.h"
#include "SkColorSpace.h"
#include "SkColorSpacePriv.h"
#include "SkCommonFlags.h"
#include "SkCommonFlagsConfig.h"
#include "SkCommonFlagsGpu.h"
#include "SkData.h"
#include "SkDebugfTracer.h"
#include "SkDocument.h"
#include "SkEventTracingPriv.h"
#include "SkFontMgr.h"
#include "SkFontMgrPriv.h"
#include "SkGraphics.h"
#include "SkHalf.h"
#include "SkICC.h"
#include "SkLeanWindows.h"
#include "SkMD5.h"
#include "SkMutex.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkScan.h"
#include "SkSpinlock.h"
#include "SkTestFontMgr.h"
#include "SkTHash.h"
#include "SkTaskGroup.h"
#include "SkTypeface_win.h"
#include "Test.h"
#include "ios_utils.h"
#include "sk_tool_utils.h"

#include <vector>

#include "png.h"

#include <stdlib.h>

#ifndef SK_BUILD_FOR_WIN
    #include <unistd.h>
#endif

#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK) && defined(SK_HAS_HEIF_LIBRARY)
#include <binder/IPCThreadState.h>
#endif

extern bool gSkForceRasterPipelineBlitter;

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
        "Space-separated config/src/srcOptions/name quadruples to blacklist. "
        "'_' matches anything. '~' negates the match. E.g. \n"
        "'--blacklist gpu skp _ _' will blacklist all SKPs drawn into the gpu config.\n"
        "'--blacklist gpu skp _ _ 8888 gm _ aarects' will also blacklist the aarects GM on 8888.\n"
        "'--blacklist ~8888 svg _ svgparse_' blocks non-8888 SVGs that contain \"svgparse_\" in "
                                            "the name.");

DEFINE_string2(readPath, r, "", "If set check for equality with golden results in this directory.");

DEFINE_string(uninterestingHashesFile, "",
        "File containing a list of uninteresting hashes. If a result hashes to something in "
        "this list, no image is written for that result.");

DEFINE_int32(shards, 1, "We're splitting source data into this many shards.");
DEFINE_int32(shard,  0, "Which shard do I run?");

DEFINE_string(mskps, "", "Directory to read mskps from, or a single mskp file.");
DEFINE_bool(forceRasterPipeline, false, "sets gSkForceRasterPipelineBlitter");

DEFINE_string(bisect, "",
        "Pair of: SKP file to bisect, followed by an l/r bisect trail string (e.g., 'lrll'). The "
        "l/r trail specifies which half to keep at each step of a binary search through the SKP's "
        "paths. An empty string performs no bisect. Only the SkPaths are bisected; all other draws "
        "are thrown out. This is useful for finding a reduced repo case for path drawing bugs.");

DEFINE_bool(ignoreSigInt, false, "ignore SIGINT signals during test execution");

DEFINE_string(dont_write, "", "File extensions to skip writing to --writePath.");  // See skia:6821

DEFINE_bool(gdi, false, "On Windows, use GDI instead of DirectWrite for font rendering.");

DEFINE_bool(checkF16, false, "Ensure that F16Norm pixels are clamped.");

using namespace DM;
using sk_gpu_test::GrContextFactory;
using sk_gpu_test::GLTestContext;
using sk_gpu_test::ContextInfo;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static constexpr skcms_TransferFunction k2020_TF =
    {2.22222f, 0.909672f, 0.0903276f, 0.222222f, 0.0812429f, 0, 0};

static sk_sp<SkColorSpace> rec2020() {
    return SkColorSpace::MakeRGB(k2020_TF, SkNamedGamut::kRec2020);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static FILE* gVLog;

template <typename... Args>
static void vlog(const char* fmt, Args&&... args) {
    if (gVLog) {
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

struct Running {
    SkString   id;
    SkThreadID thread;

    void dump() const {
        info("\t%s\n", id.c_str());
    }
};

// We use a spinlock to make locking this in a signal handler _somewhat_ safe.
static SkSpinlock        gMutex;
static int               gPending;
static SkTArray<Running> gRunning;

static void done(const char* config, const char* src, const char* srcOptions, const char* name) {
    SkString id = SkStringPrintf("%s %s %s %s", config, src, srcOptions, name);
    vlog("done  %s\n", id.c_str());
    int pending;
    {
        SkAutoMutexAcquire lock(gMutex);
        for (int i = 0; i < gRunning.count(); i++) {
            if (gRunning[i].id == id) {
                gRunning.removeShuffle(i);
                break;
            }
        }
        pending = --gPending;
    }

    // We write out dm.json file and print out a progress update every once in a while.
    // Notice this also handles the final dm.json and progress update when pending == 0.
    if (pending % 500 == 0) {
        JsonWriter::DumpJson();

        int curr = sk_tools::getCurrResidentSetSizeMB(),
            peak = sk_tools::getMaxResidentSetSizeMB();

        SkAutoMutexAcquire lock(gMutex);
        info("\n%dMB RAM, %dMB peak, %d queued, %d active:\n",
             curr, peak, gPending - gRunning.count(), gRunning.count());
        for (auto& task : gRunning) {
            task.dump();
        }
    }
}

static void start(const char* config, const char* src, const char* srcOptions, const char* name) {
    SkString id = SkStringPrintf("%s %s %s %s", config, src, srcOptions, name);
    vlog("start %s\n", id.c_str());
    SkAutoMutexAcquire lock(gMutex);
    gRunning.push_back({id,SkGetThreadID()});
}

static void find_culprit() {
    // Assumes gMutex is locked.
    SkThreadID thisThread = SkGetThreadID();
    for (auto& task : gRunning) {
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
        SkAutoMutexAcquire lock(gMutex);

        info("\nCaught signal %d [%s] (%dMB RAM, peak %dMB), was running:\n",
             sig, strsignal(sig),
             sk_tools::getCurrResidentSetSizeMB(), sk_tools::getMaxResidentSetSizeMB());

        for (auto& task : gRunning) {
            task.dump();
        }
        find_culprit();

    #if !defined(SK_BUILD_FOR_ANDROID)
        void* stack[64];
        int count = backtrace(stack, SK_ARRAY_COUNT(stack));
        char** symbols = backtrace_symbols(stack, count);
        info("\nStack trace:\n");
        for (int i = 0; i < count; i++) {
            info("    %s\n", symbols[i]);
        }
    #endif
        fflush(stdout);

        signal(sig, previous_handler[sig]);
        raise(sig);
    }

    static void setup_crash_handler() {
        const int kSignals[] = { SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGINT, SIGSEGV, SIGTERM };
        for (int sig : kSignals) {
            previous_handler[sig] = signal(sig, crash_handler);
        }

        if (FLAGS_ignoreSigInt) {
            signal(SIGINT, SIG_IGN);
        }
    }
#endif

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

class HashAndEncode {
public:
    explicit HashAndEncode(const SkBitmap&);

    void write(SkWStream*) const;

    bool writePngTo(const char* path, const char* md5) const;

private:
    const SkISize               fSize;
    std::unique_ptr<uint64_t[]> fPixels;
};

HashAndEncode::HashAndEncode(const SkBitmap& bitmap) : fSize(bitmap.info().dimensions()) {
    skcms_AlphaFormat srcAlpha;
    switch (bitmap.alphaType()) {
        case kUnknown_SkAlphaType: SkASSERT(false); return;

        case kOpaque_SkAlphaType:
        case kUnpremul_SkAlphaType: srcAlpha = skcms_AlphaFormat_Unpremul;        break;
        case kPremul_SkAlphaType:   srcAlpha = skcms_AlphaFormat_PremulAsEncoded; break;
    }

    skcms_PixelFormat srcFmt;
    switch (bitmap.colorType()) {
        case kUnknown_SkColorType: SkASSERT(false); return;

        case kAlpha_8_SkColorType:      srcFmt = skcms_PixelFormat_A_8;          break;
        case kRGB_565_SkColorType:      srcFmt = skcms_PixelFormat_BGR_565;      break;
        case kARGB_4444_SkColorType:    srcFmt = skcms_PixelFormat_ABGR_4444;    break;
        case kRGBA_8888_SkColorType:    srcFmt = skcms_PixelFormat_RGBA_8888;    break;
        case kBGRA_8888_SkColorType:    srcFmt = skcms_PixelFormat_BGRA_8888;    break;
        case kRGBA_1010102_SkColorType: srcFmt = skcms_PixelFormat_RGBA_1010102; break;
        case kGray_8_SkColorType:       srcFmt = skcms_PixelFormat_G_8;          break;
        case kRGBA_F16Norm_SkColorType: srcFmt = skcms_PixelFormat_RGBA_hhhh;    break;
        case kRGBA_F16_SkColorType:     srcFmt = skcms_PixelFormat_RGBA_hhhh;    break;
        case kRGBA_F32_SkColorType:     srcFmt = skcms_PixelFormat_RGBA_ffff;    break;

        case kRGB_888x_SkColorType:     srcFmt = skcms_PixelFormat_RGBA_8888;
                                        srcAlpha = skcms_AlphaFormat_Opaque;       break;
        case kRGB_101010x_SkColorType:  srcFmt = skcms_PixelFormat_RGBA_1010102;
                                        srcAlpha = skcms_AlphaFormat_Opaque;       break;
    }

    skcms_ICCProfile srcProfile = *skcms_sRGB_profile();
    if (auto cs = bitmap.colorSpace()) {
        cs->toProfile(&srcProfile);
    }

    // Our common format that can represent anything we draw and encode as a PNG:
    //   - 16-bit big-endian RGBA
    //   - unpremul
    //   - Rec. 2020 gamut and transfer function
    skcms_PixelFormat dstFmt   = skcms_PixelFormat_RGBA_16161616BE;
    skcms_AlphaFormat dstAlpha = skcms_AlphaFormat_Unpremul;
    skcms_ICCProfile dstProfile;
    rec2020()->toProfile(&dstProfile);

    int N = fSize.width() * fSize.height();
    fPixels.reset(new uint64_t[N]);

    if (!skcms_Transform(bitmap.getPixels(), srcFmt, srcAlpha, &srcProfile,
                         fPixels.get(),      dstFmt, dstAlpha, &dstProfile, N)) {
        SkASSERT(false);
        fPixels.reset(nullptr);
    }
}

void HashAndEncode::write(SkWStream* st) const {
    st->write(&fSize, sizeof(fSize));
    if (const uint64_t* px = fPixels.get()) {
        st->write(px, sizeof(*px) * fSize.width() * fSize.height());
    }

    // N.B. changing salt will change the hash of all images produced by DM,
    // and will cause tens of thousands of new images to be uploaded to Gold.
    int salt = 1;
    st->write(&salt, sizeof(salt));
}

bool HashAndEncode::writePngTo(const char* path, const char* md5) const {
    if (!fPixels) {
        return false;
    }

    FILE* f = fopen(path, "wb");
    if (!f) {
        return false;
    }

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
    text[0].key  = (png_charp)"Author";
    text[0].text = (png_charp)"DM unified Rec.2020";
    text[0].compression = PNG_TEXT_COMPRESSION_NONE;
    text[1].key  = (png_charp)"Description";
    text[1].text = (png_charp)description.c_str();
    text[1].compression = PNG_TEXT_COMPRESSION_NONE;
    png_set_text(png, info, text, SK_ARRAY_COUNT(text));

    png_init_io(png, f);
    png_set_IHDR(png, info, (png_uint_32)fSize.width()
                          , (png_uint_32)fSize.height()
                          , 16/*bits per channel*/
                          , PNG_COLOR_TYPE_RGB_ALPHA
                          , PNG_INTERLACE_NONE
                          , PNG_COMPRESSION_TYPE_DEFAULT
                          , PNG_FILTER_TYPE_DEFAULT);

    // Fastest encoding and decoding, at slight file size cost is no filtering, compression 1.
    png_set_filter(png, PNG_FILTER_TYPE_BASE, PNG_FILTER_NONE);
    // TODO(mtklein): set back to 1 after all the bots have cycled through new images / hashes?
    png_set_compression_level(png, 9);

    static const sk_sp<SkData> profile = SkWriteICCProfile(k2020_TF, SkNamedGamut::kRec2020);
    png_set_iCCP(png, info,
                 "Rec.2020",
                 0/*compression type... no idea what options are available here*/,
                 (png_const_bytep)profile->data(),
                 (png_uint_32)    profile->size());

    png_write_info(png, info);
    for (int y = 0; y < fSize.height(); y++) {
        png_write_row(png, (png_bytep)(fPixels.get() + y*fSize.width()));
    }
    png_write_end(png, info);

    png_destroy_write_struct(&png, &info);
    fclose(f);
    return true;
}

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

#if defined(SK_BUILD_FOR_WIN)
    static const char* kNewline = "\r\n";
#else
    static const char* kNewline = "\n";
#endif

static SkTHashSet<SkString> gUninterestingHashes;

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

        SkTArray<SkString> hashes;
        SkStrSplit(contents.c_str(), kNewline, &hashes);
        for (const SkString& hash : hashes) {
            gUninterestingHashes.add(hash);
        }
        info("FYI: loaded %d distinct uninteresting hashes from %d lines\n",
             gUninterestingHashes.count(), hashes.count());
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

static const bool kMemcpyOK = true;

static SkTArray<TaggedSrc,  kMemcpyOK>  gSrcs;
static SkTArray<TaggedSink, kMemcpyOK> gSinks;

static bool in_shard() {
    static int N = 0;
    return N++ % FLAGS_shards == FLAGS_shard;
}

static void push_src(const char* tag, ImplicitString options, Src* s) {
    std::unique_ptr<Src> src(s);
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

static void push_codec_srcs(Path path) {
    sk_sp<SkData> encoded(SkData::MakeFromFileName(path.c_str()));
    if (!encoded) {
        info("Couldn't read %s.", path.c_str());
        return;
    }
    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(encoded);
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

    SkTArray<CodecSrc::DstColorType> colorTypes;
    colorTypes.push_back(CodecSrc::kGetFromCanvas_DstColorType);
    colorTypes.push_back(CodecSrc::kNonNative8888_Always_DstColorType);
    switch (codec->getInfo().colorType()) {
        case kGray_8_SkColorType:
            colorTypes.push_back(CodecSrc::kGrayscale_Always_DstColorType);
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

    {
        std::vector<SkCodec::FrameInfo> frameInfos = codec->getFrameInfo();
        if (frameInfos.size() > 1) {
            for (auto dstCT : { CodecSrc::kNonNative8888_Always_DstColorType,
                    CodecSrc::kGetFromCanvas_DstColorType }) {
                for (auto at : { kUnpremul_SkAlphaType, kPremul_SkAlphaType }) {
                    push_codec_src(path, CodecSrc::kAnimated_Mode, dstCT, at, 1.0f);
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
#endif
    }
}

template <typename T>
void gather_file_srcs(const SkCommandLineFlags::StringArray& flags, const char* ext,
                      const char* src_name = nullptr) {
    if (!src_name) {
        // With the exception of Lottie files, the source name is the extension.
        src_name = ext;
    }

    for (int i = 0; i < flags.count(); i++) {
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
    for (skiagm::GMFactory f : skiagm::GMRegistry::Range()) {
        push_src("gm", "", new GMSrc(f));
    }

    gather_file_srcs<SKPSrc>(FLAGS_skps, "skp");
    gather_file_srcs<MSKPSrc>(FLAGS_mskps, "mskp");
#if defined(SK_ENABLE_SKOTTIE)
    gather_file_srcs<SkottieSrc>(FLAGS_lotties, "json", "lottie");
#endif
#if defined(SK_XML)
    gather_file_srcs<SVGSrc>(FLAGS_svgs, "svg");
#endif
    if (!FLAGS_bisect.isEmpty()) {
        // An empty l/r trail string will draw all the paths.
        push_src("bisect", "",
                 new BisectSrc(FLAGS_bisect[0], FLAGS_bisect.count() > 1 ? FLAGS_bisect[1] : ""));
    }

    SkTArray<SkString> images;
    if (!CollectImages(FLAGS_images, &images)) {
        return false;
    }

    for (auto image : images) {
        push_codec_srcs(image);
    }

    SkTArray<SkString> colorImages;
    if (!CollectImages(FLAGS_colorImages, &colorImages)) {
        return false;
    }

    for (auto colorImage : colorImages) {
        push_src("colorImage", "decode_native", new ColorCodecSrc(colorImage, false));
        push_src("colorImage", "decode_to_dst", new ColorCodecSrc(colorImage,  true));
    }

    return true;
}

static void push_sink(const SkCommandLineConfig& config, Sink* s) {
    std::unique_ptr<Sink> sink(s);

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

static sk_sp<SkColorSpace> rgb_to_gbr() {
    return SkColorSpace::MakeSRGB()->makeColorSpin();
}

static Sink* create_sink(const GrContextOptions& grCtxOptions, const SkCommandLineConfig* config) {
    if (FLAGS_gpu) {
        if (const SkCommandLineConfigGpu* gpuConfig = config->asConfigGpu()) {
            GrContextFactory::ContextType contextType = gpuConfig->getContextType();
            GrContextFactory::ContextOverrides contextOverrides = gpuConfig->getContextOverrides();
            GrContextFactory testFactory(grCtxOptions);
            if (!testFactory.get(contextType, contextOverrides)) {
                info("WARNING: can not create GPU context for config '%s'. "
                     "GM tests will be skipped.\n", gpuConfig->getTag().c_str());
                return nullptr;
            }
            if (gpuConfig->getTestThreading()) {
                SkASSERT(!gpuConfig->getTestPersistentCache());
                return new GPUThreadTestingSink(
                        contextType, contextOverrides, gpuConfig->getSurfType(),
                        gpuConfig->getSamples(), gpuConfig->getUseDIText(),
                        gpuConfig->getColorType(), gpuConfig->getAlphaType(),
                        sk_ref_sp(gpuConfig->getColorSpace()), FLAGS_gpu_threading, grCtxOptions);
            } else if (gpuConfig->getTestPersistentCache()) {
                return new GPUPersistentCacheTestingSink(
                        contextType, contextOverrides, gpuConfig->getSurfType(),
                        gpuConfig->getSamples(), gpuConfig->getUseDIText(),
                        gpuConfig->getColorType(), gpuConfig->getAlphaType(),
                        sk_ref_sp(gpuConfig->getColorSpace()), FLAGS_gpu_threading, grCtxOptions);
            } else {
                return new GPUSink(contextType, contextOverrides, gpuConfig->getSurfType(),
                                   gpuConfig->getSamples(), gpuConfig->getUseDIText(),
                                   gpuConfig->getColorType(), gpuConfig->getAlphaType(),
                                   sk_ref_sp(gpuConfig->getColorSpace()), FLAGS_gpu_threading,
                                   grCtxOptions);
            }
        }
    }
    if (const SkCommandLineConfigSvg* svgConfig = config->asConfigSvg()) {
        int pageIndex = svgConfig->getPageIndex();
        return new SVGSink(pageIndex);
    }

#define SINK(t, sink, ...) if (config->getBackend().equals(t)) return new sink(__VA_ARGS__)

    if (FLAGS_cpu) {
        SINK("g8",      RasterSink, kGray_8_SkColorType);
        SINK("565",     RasterSink, kRGB_565_SkColorType);
        SINK("4444",    RasterSink, kARGB_4444_SkColorType);
        SINK("8888",    RasterSink, kN32_SkColorType);
        SINK("rgba",    RasterSink, kRGBA_8888_SkColorType);
        SINK("bgra",    RasterSink, kBGRA_8888_SkColorType);
        SINK("rgbx",    RasterSink, kRGB_888x_SkColorType);
        SINK("1010102", RasterSink, kRGBA_1010102_SkColorType);
        SINK("101010x", RasterSink, kRGB_101010x_SkColorType);
        SINK("pdf",     PDFSink, false, SK_ScalarDefaultRasterDPI);
        SINK("skp",     SKPSink);
        SINK("svg",     SVGSink);
        SINK("null",    NullSink);
        SINK("xps",     XPSSink);
        SINK("pdfa",    PDFSink, true,  SK_ScalarDefaultRasterDPI);
        SINK("pdf300",  PDFSink, false, 300);
        SINK("jsdebug", DebugSink);

        // Configs relevant to color management testing (and 8888 for reference).

        // 'narrow' has a gamut narrower than sRGB, and different transfer function.
        auto narrow = SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2, gNarrow_toXYZD50),
               srgb = SkColorSpace::MakeSRGB(),
         srgbLinear = SkColorSpace::MakeSRGBLinear(),
                 p3 = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDCIP3);

        SINK(     "f16",  RasterSink,  kRGBA_F16_SkColorType, srgbLinear);
        SINK(    "srgb",  RasterSink, kRGBA_8888_SkColorType, srgb      );
        SINK(   "esrgb",  RasterSink,  kRGBA_F16_SkColorType, srgb      );
        SINK(   "esgbr",  RasterSink,  kRGBA_F16_SkColorType, rgb_to_gbr());
        SINK(  "narrow",  RasterSink, kRGBA_8888_SkColorType, narrow    );
        SINK( "enarrow",  RasterSink,  kRGBA_F16_SkColorType, narrow    );
        SINK(      "p3",  RasterSink, kRGBA_8888_SkColorType, p3        );
        SINK(     "ep3",  RasterSink,  kRGBA_F16_SkColorType, p3        );
        SINK( "rec2020",  RasterSink, kRGBA_8888_SkColorType, rec2020() );
        SINK("erec2020",  RasterSink,  kRGBA_F16_SkColorType, rec2020() );

        SINK("f16norm",  RasterSink,  kRGBA_F16Norm_SkColorType, srgb);

        SINK(    "f32",  RasterSink,  kRGBA_F32_SkColorType, srgbLinear);
    }
#undef SINK
    return nullptr;
}

static Sink* create_via(const SkString& tag, Sink* wrapped) {
#define VIA(t, via, ...) if (tag.equals(t)) return new via(__VA_ARGS__)
    VIA("gbr",       ViaCSXform,           wrapped, rgb_to_gbr(), true);
    VIA("p3",        ViaCSXform,           wrapped,
                     SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDCIP3), false);
    VIA("lite",      ViaLite,              wrapped);
#ifdef TEST_VIA_SVG
    VIA("svg",       ViaSVG,               wrapped);
#endif
    VIA("serialize", ViaSerialization,     wrapped);
    VIA("pic",       ViaPicture,           wrapped);
    VIA("tiles",     ViaTiles, 256, 256, nullptr,            wrapped);
    VIA("tiles_rt",  ViaTiles, 256, 256, new SkRTreeFactory, wrapped);

    VIA("ddl",       ViaDDL, 1, 3,         wrapped);
    VIA("ddl2",      ViaDDL, 2, 3,         wrapped);

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

#undef VIA
    return nullptr;
}

static bool gather_sinks(const GrContextOptions& grCtxOptions, bool defaultConfigs) {
    SkCommandLineConfigArray configs;
    ParseConfigs(FLAGS_config, &configs);
    for (int i = 0; i < configs.count(); i++) {
        const SkCommandLineConfig& config = *configs[i];
        Sink* sink = create_sink(grCtxOptions, &config);
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
    if (configs.count() == 0 ||
        // If we're using the default configs, we're okay.
        defaultConfigs ||
        // Otherwise, make sure that all specified configs have become sinks.
        configs.count() == gSinks.count()) {
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
            SkStreamAsset* data = stream.detachAsStream().release();
            gDefinitelyThreadSafeWork.add([task,name,bitmap,data]{
                std::unique_ptr<SkStreamAsset> ownedData(data);

                std::unique_ptr<HashAndEncode> hashAndEncode;

                SkString md5;
                if (!FLAGS_writePath.isEmpty() || !FLAGS_readPath.isEmpty()) {
                    SkMD5 hash;
                    if (data->getLength()) {
                        hash.writeStream(data, data->getLength());
                        data->rewind();
                    } else {
                        hashAndEncode.reset(new HashAndEncode(bitmap));
                        hashAndEncode->write(&hash);
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
                    if (ext && !FLAGS_dont_write.contains(ext)) {
                        if (data->getLength()) {
                            WriteToDisk(task, md5, ext, data, data->getLength(), nullptr, nullptr);
                            SkASSERT(bitmap.drawsNothing());
                        } else if (!bitmap.drawsNothing()) {
                            WriteToDisk(task, md5, ext, nullptr, 0, &bitmap, hashAndEncode.get());
                        }
                    }
                }

                SkPixmap pm;
                if (FLAGS_checkF16 && bitmap.colorType() == kRGBA_F16Norm_SkColorType &&
                        bitmap.peekPixels(&pm)) {
                    bool unclamped = false;
                    for (int y = 0; y < pm.height() && !unclamped; ++y)
                    for (int x = 0; x < pm.width() && !unclamped; ++x) {
                        Sk4f rgba = SkHalfToFloat_finite_ftz(*pm.addr64(x, y));
                        float a = rgba[3];
                        if (a > 1.0f || (rgba < 0.0f).anyTrue() || (rgba > a).anyTrue()) {
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

            if (eq(gamut, SkNamedGamut::kSRGB    )) { return SkString("sRGB"); }
            if (eq(gamut, SkNamedGamut::kAdobeRGB)) { return SkString("Adobe"); }
            if (eq(gamut, SkNamedGamut::kDCIP3   )) { return SkString("P3"); }
            if (eq(gamut, SkNamedGamut::kRec2020 )) { return SkString("2020"); }
            if (eq(gamut, SkNamedGamut::kXYZ     )) { return SkString("XYZ"); }
            if (eq(gamut,     gNarrow_toXYZD50   )) { return SkString("narrow"); }
            return SkString("other");
        }
        return SkString("non-XYZ");
    }

    static SkString identify_transfer_fn(SkColorSpace* cs) {
        if (!cs) {
            return SkString("untagged");
        }

        skcms_TransferFunction tf;
        if (cs->isNumericalTransferFn(&tf)) {
            auto eq = [](skcms_TransferFunction x, skcms_TransferFunction y) {
                return x.g == y.g
                    && x.a == y.a
                    && x.b == y.b
                    && x.c == y.c
                    && x.d == y.d
                    && x.e == y.e
                    && x.f == y.f;
            };

            if (tf.a == 1 && tf.b == 0 && tf.c == 0 && tf.d == 0 && tf.e == 0 && tf.f == 0) {
                return SkStringPrintf("gamma %.3g", tf.g);
            }
            if (eq(tf, SkNamedTransferFn::kSRGB)) { return SkString("sRGB"); }
            if (eq(tf, k2020_TF                )) { return SkString("2020"); }
            return SkStringPrintf("%.3g %.3g %.3g %.3g %.3g %.3g %.3g",
                                  tf.g, tf.a, tf.b, tf.c, tf.d, tf.e, tf.f);
        }
        return SkString("non-numeric");
    }

    // Equivalence class to slice color type by in Gold.
    // Basically the same as color type ignoring channel order.
    static const char* color_depth(SkColorType ct) {
        switch (ct) {
            case kUnknown_SkColorType: break;

            case kAlpha_8_SkColorType:      return "A8";
            case kRGB_565_SkColorType:      return "565";
            case kARGB_4444_SkColorType:    return "4444";
            case kRGBA_8888_SkColorType:    return "8888";
            case kRGB_888x_SkColorType:     return "888";
            case kBGRA_8888_SkColorType:    return "8888";
            case kRGBA_1010102_SkColorType: return "1010102";
            case kRGB_101010x_SkColorType:  return "101010";
            case kGray_8_SkColorType:       return "G8";
            case kRGBA_F16Norm_SkColorType: return "F16Norm";  // TODO: "F16"?
            case kRGBA_F16_SkColorType:     return "F16";
            case kRGBA_F32_SkColorType:     return "F32";
        }
        return "Unknown";
    }

    static void WriteToDisk(const Task& task,
                            SkString md5,
                            const char* ext,
                            SkStream* data, size_t len,
                            const SkBitmap* bitmap,
                            const HashAndEncode* hashAndEncode) {

        JsonWriter::BitmapResult result;
        result.name          = task.src->name();
        result.config        = task.sink.tag;
        result.sourceType    = task.src.tag;
        result.sourceOptions = task.src.options;
        result.ext           = ext;
        result.md5           = md5;
        if (bitmap) {
            result.gamut         = identify_gamut               (bitmap->colorSpace());
            result.transferFn    = identify_transfer_fn         (bitmap->colorSpace());
            result.colorType     = sk_tool_utils::colortype_name(bitmap->colorType ());
            result.alphaType     = sk_tool_utils::alphatype_name(bitmap->alphaType ());
            result.colorDepth    = color_depth                  (bitmap->colorType());
        }
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
            SkASSERT(hashAndEncode);
            if (!hashAndEncode->writePngTo(path.c_str(), result.md5.c_str())) {
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
    for (const skiatest::Test& test : skiatest::TestRegistry::Range()) {
        if (!in_shard()) {
            continue;
        }
        if (SkCommandLineFlags::ShouldSkip(FLAGS_match, test.name)) {
            continue;
        }
        if (test.needsGpu && FLAGS_gpu) {
            (FLAGS_gpu_threading ? gParallelTests : gSerialTests).push_back(test);
        } else if (!test.needsGpu && FLAGS_cpu) {
            gParallelTests.push_back(test);
        }
    }
}

static void run_test(skiatest::Test test, const GrContextOptions& grCtxOptions) {
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
        GrContextOptions options = grCtxOptions;
        test.modifyGrContextOptions(&options);

        skiatest::ReporterContext ctx(&reporter, SkString(test.name));
        start("unit", "test", "", test.name);
        test.run(&reporter, options);
    }
    done("unit", "test", "", test.name);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

int main(int argc, char** argv) {
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK) && defined(SK_HAS_HEIF_LIBRARY)
    android::ProcessState::self()->startThreadPool();
#endif
    SkCommandLineFlags::Parse(argc, argv);

    if (!FLAGS_nativeFonts) {
        gSkFontMgr_DefaultFactory = &sk_tool_utils::MakePortableFontMgr;
    }

#if defined(SK_BUILD_FOR_WIN)
    if (FLAGS_gdi) {
        gSkFontMgr_DefaultFactory = &SkFontMgr_New_GDI;
    }
#endif

    initializeEventTracingForTools();

#if !defined(SK_BUILD_FOR_GOOGLE3) && defined(SK_BUILD_FOR_IOS)
    cd_Documents();
#endif
    setbuf(stdout, nullptr);
    setup_crash_handler();

    gSkUseAnalyticAA = FLAGS_analyticAA;
    gSkUseDeltaAA = FLAGS_deltaAA;

    if (FLAGS_forceAnalyticAA) {
        gSkForceAnalyticAA = true;
    }
    if (FLAGS_forceDeltaAA) {
        gSkForceDeltaAA = true;
    }
    if (FLAGS_forceRasterPipeline) {
        gSkForceRasterPipelineBlitter = true;
    }

    // The bots like having a verbose.log to upload, so always touch the file even if --verbose.
    if (!FLAGS_writePath.isEmpty()) {
        sk_mkdir(FLAGS_writePath[0]);
        gVLog = fopen(SkOSPath::Join(FLAGS_writePath[0], "verbose.log").c_str(), "w");
    }
    if (FLAGS_verbose) {
        gVLog = stderr;
    }

    GrContextOptions grCtxOptions;
    SetCtxOptionsFromCommonFlags(&grCtxOptions);

    JsonWriter::DumpJson();  // It's handy for the bots to assume this is ~never missing.
    SkAutoGraphics ag;
    SkTaskGroup::Enabler enabled(FLAGS_threads);

    if (nullptr == GetResourceAsData("images/color_wheel.png")) {
        info("Some resources are missing.  Do you need to set --resourcePath?\n");
    }
    gather_gold();
    gather_uninteresting_hashes();

    if (!gather_srcs()) {
        return 1;
    }
    // TODO(dogben): This is a bit ugly. Find a cleaner way to do this.
    bool defaultConfigs = true;
    for (int i = 0; i < argc; i++) {
        static const char* kConfigArg = "--config";
        if (strcmp(argv[i], kConfigArg) == 0) {
            defaultConfigs = false;
            break;
        }
    }
    if (!gather_sinks(grCtxOptions, defaultConfigs)) {
        return 1;
    }
    gather_tests();
    gPending = gSrcs.count() * gSinks.count() + gParallelTests.count() + gSerialTests.count();
    info("%d srcs * %d sinks + %d tests == %d tasks\n",
         gSrcs.count(), gSinks.count(), gParallelTests.count() + gSerialTests.count(), gPending);

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
        parallel.add([test, grCtxOptions] { run_test(test, grCtxOptions); });
    }

    // With the parallel work running, run serial tasks and tests here on main thread.
    for (auto task : serial) { Task::Run(task); }
    for (auto test : gSerialTests) { run_test(test, grCtxOptions); }

    // Wait for any remaining parallel work to complete (including any spun off of serial tasks).
    parallel.wait();
    gDefinitelyThreadSafeWork.wait();

    // At this point we're back in single-threaded land.

    // We'd better have run everything.
    SkASSERT(gPending == 0);
    // Make sure we've flushed all our results to disk.
    JsonWriter::DumpJson();

    if (gFailures.count() > 0) {
        info("Failures:\n");
        for (int i = 0; i < gFailures.count(); i++) {
            info("\t%s\n", gFailures[i].c_str());
        }
        info("%d failures\n", gFailures.count());
        return 1;
    }

    SkGraphics::PurgeAllCaches();
    info("Finished!\n");

    return 0;
}
