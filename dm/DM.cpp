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
#include "OverwriteLine.h"
#include "ProcStats.h"
#include "SkBBHFactory.h"
#include "SkChecksum.h"
#include "SkCommonFlags.h"
#include "SkFontMgr.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkMD5.h"
#include "SkMutex.h"
#include "SkOSFile.h"
#include "SkTHash.h"
#include "SkTaskGroup.h"
#include "SkThreadUtils.h"
#include "Test.h"
#include "Timer.h"
#include "sk_tool_utils.h"

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
DEFINE_bool2(pre_log, p, false, "Log before running each test. May be incomprehensible when threading");

__SK_FORCE_IMAGE_DECODER_LINKING;
using namespace DM;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

SK_DECLARE_STATIC_MUTEX(gFailuresMutex);
static SkTArray<SkString> gFailures;

static void fail(ImplicitString err) {
    SkAutoMutexAcquire lock(gFailuresMutex);
    SkDebugf("\n\nFAILURE: %s\n\n", err.c_str());
    gFailures.push_back(err);
}

static int32_t gPending = 0;  // Atomic.  Total number of running and queued tasks.

SK_DECLARE_STATIC_MUTEX(gRunningMutex);
static SkTArray<SkString> gRunning;

static void done(double ms,
                 ImplicitString config, ImplicitString src, ImplicitString srcOptions,
                 ImplicitString name, ImplicitString note, ImplicitString log) {
    SkString id = SkStringPrintf("%s %s %s %s", config.c_str(), src.c_str(),
                                                srcOptions.c_str(), name.c_str());
    {
        SkAutoMutexAcquire lock(gRunningMutex);
        for (int i = 0; i < gRunning.count(); i++) {
            if (gRunning[i] == id) {
                gRunning.removeShuffle(i);
                break;
            }
        }
    }
    if (!FLAGS_verbose) {
        note = "";
    }
    if (!log.isEmpty()) {
        log.prepend("\n");
    }
    auto pending = sk_atomic_dec(&gPending)-1;
    if (!FLAGS_quiet) {
        SkDebugf("%s(%4d/%-4dMB %6d) %s\t%s%s%s", FLAGS_verbose ? "\n" : kSkOverwriteLine
                                           , sk_tools::getCurrResidentSetSizeMB()
                                           , sk_tools::getMaxResidentSetSizeMB()
                                           , pending
                                           , HumanizeMs(ms).c_str()
                                           , id.c_str()
                                           , note.c_str()
                                           , log.c_str());
    }
    // We write our dm.json file every once in a while in case we crash.
    // Notice this also handles the final dm.json when pending == 0.
    if (pending % 500 == 0) {
        JsonWriter::DumpJson();
    }
}

static void start(ImplicitString config, ImplicitString src,
                  ImplicitString srcOptions, ImplicitString name) {
    SkString id = SkStringPrintf("%s %s %s %s", config.c_str(), src.c_str(),
                                                srcOptions.c_str(), name.c_str());
    SkAutoMutexAcquire lock(gRunningMutex);
    gRunning.push_back(id);
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

struct Gold : public SkString {
    Gold() : SkString("") {}
    Gold(ImplicitString sink, ImplicitString src, ImplicitString srcOptions,
         ImplicitString name, ImplicitString md5)
        : SkString("") {
        this->append(sink);
        this->append(src);
        this->append(srcOptions);
        this->append(name);
        this->append(md5);
    }
    static uint32_t Hash(const Gold& g) { return SkGoodHash((const SkString&)g); }
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

static SkTHashSet<SkString> gUninterestingHashes;

static void gather_uninteresting_hashes() {
    if (!FLAGS_uninterestingHashesFile.isEmpty()) {
        SkAutoTUnref<SkData> data(SkData::NewFromFileName(FLAGS_uninterestingHashesFile[0]));
        SkTArray<SkString> hashes;
        SkStrSplit((const char*)data->data(), "\n", &hashes);
        for (const SkString& hash : hashes) {
            gUninterestingHashes.add(hash);
        }
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

struct TaggedSrc : public SkAutoTDelete<Src> {
    const char* tag;
    const char* options;
};

struct TaggedSink : public SkAutoTDelete<Sink> {
    const char* tag;
};

static const bool kMemcpyOK = true;

static SkTArray<TaggedSrc,  kMemcpyOK>  gSrcs;
static SkTArray<TaggedSink, kMemcpyOK> gSinks;

static bool in_shard() {
    static int N = 0;
    return N++ % FLAGS_shards == FLAGS_shard;
}

static void push_src(const char* tag, const char* options, Src* s) {
    SkAutoTDelete<Src> src(s);
    if (in_shard() &&
        FLAGS_src.contains(tag) &&
        !SkCommandLineFlags::ShouldSkip(FLAGS_match, src->name().c_str())) {
        TaggedSrc& s = gSrcs.push_back();
        s.reset(src.detach());
        s.tag = tag;
        s.options = options;
    }
}

static void push_codec_srcs(Path path) {
    SkAutoTUnref<SkData> encoded(SkData::NewFromFileName(path.c_str()));
    if (!encoded) {
        SkDebugf("Couldn't read %s.", path.c_str());
        return;
    }
    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromData(encoded));
    if (NULL == codec.get()) {
        SkDebugf("Couldn't create codec for %s.", path.c_str());
        return;
    }

    // Choose scales for scaling tests.
    // TODO (msarett): Add more scaling tests as we implement more flexible scaling.
    // TODO (msarett): Implement scaling tests for SkImageDecoder in order to compare with these
    //                 tests.  SkImageDecoder supports downscales by integer factors.
    const float scales[] = { 0.125f, 0.25f, 0.375f, 0.5f, 0.625f, 0.750f, 0.875f, 1.0f };

    for (float scale : scales) {
        if (scale != 1.0f && (path.endsWith(".webp") || path.endsWith(".WEBP"))) {
            // FIXME: skbug.com/4038 Scaling webp seems to leave some pixels uninitialized/
            // compute their colors based on uninitialized values.
            continue;
        }
        // Build additional test cases for images that decode natively to non-canvas types
        switch(codec->getInfo().colorType()) {
            case kGray_8_SkColorType:
                push_src("image", "codec_kGray8", new CodecSrc(path, CodecSrc::kNormal_Mode,
                        CodecSrc::kGrayscale_Always_DstColorType, scale));
                push_src("image", "scanline_kGray8", new CodecSrc(path, CodecSrc::kScanline_Mode,
                        CodecSrc::kGrayscale_Always_DstColorType, scale));
                push_src("image", "scanline_subset_kGray8", new CodecSrc(path,
                        CodecSrc::kScanline_Subset_Mode, CodecSrc::kGrayscale_Always_DstColorType,
                        scale));
                push_src("image", "stripe_kGray8", new CodecSrc(path, CodecSrc::kStripe_Mode,
                        CodecSrc::kGrayscale_Always_DstColorType, scale));
                // Intentional fall through
                // FIXME: Is this a long term solution for testing wbmps decodes to kIndex8?
                // Further discussion on this topic is at skbug.com/3683
            case kIndex_8_SkColorType:
                push_src("image", "codec_kIndex8", new CodecSrc(path, CodecSrc::kNormal_Mode,
                        CodecSrc::kIndex8_Always_DstColorType, scale));
                push_src("image", "scanline_kIndex8", new CodecSrc(path, CodecSrc::kScanline_Mode,
                        CodecSrc::kIndex8_Always_DstColorType, scale));
                push_src("image", "scanline_subset_kIndex8", new CodecSrc(path,
                        CodecSrc::kScanline_Subset_Mode, CodecSrc::kIndex8_Always_DstColorType,
                        scale));
                push_src("image", "stripe_kIndex8", new CodecSrc(path, CodecSrc::kStripe_Mode,
                        CodecSrc::kIndex8_Always_DstColorType, scale));
                break;
            default:
                // Do nothing
                break;
        }

        // Decode all images to the canvas color type
        push_src("image", "codec", new CodecSrc(path, CodecSrc::kNormal_Mode,
                CodecSrc::kGetFromCanvas_DstColorType, scale));
        push_src("image", "scanline", new CodecSrc(path, CodecSrc::kScanline_Mode,
                CodecSrc::kGetFromCanvas_DstColorType, scale));
        push_src("image", "scanline_subset", new CodecSrc(path, CodecSrc::kScanline_Subset_Mode,
                CodecSrc::kGetFromCanvas_DstColorType, scale));
        push_src("image", "stripe", new CodecSrc(path, CodecSrc::kStripe_Mode,
                CodecSrc::kGetFromCanvas_DstColorType, scale));
        // Note: The only codec which supports subsets natively is SkWebpCodec, which will never
        // report kIndex_8 or kGray_8, so there is no need to test kSubset_mode with those color
        // types specifically requested.
        push_src("image", "codec_subset", new CodecSrc(path, CodecSrc::kSubset_Mode,
                CodecSrc::kGetFromCanvas_DstColorType, scale));
    }
}

static bool codec_supported(const char* ext) {
    // FIXME: Once other versions of SkCodec are available, we can add them to this
    // list (and eventually we can remove this check once they are all supported).
    static const char* const exts[] = {
        "bmp", "gif", "jpg", "jpeg", "png", "ico", "wbmp", "webp",
        "BMP", "GIF", "JPG", "JPEG", "PNG", "ICO", "WBMP", "WEBP",
    };

    for (uint32_t i = 0; i < SK_ARRAY_COUNT(exts); i++) {
        if (0 == strcmp(exts[i], ext)) {
            return true;
        }
    }
    return false;
}

static void gather_srcs() {
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
    static const char* const exts[] = {
        "bmp", "gif", "jpg", "jpeg", "png", "webp", "ktx", "astc", "wbmp", "ico",
        "BMP", "GIF", "JPG", "JPEG", "PNG", "WEBP", "KTX", "ASTC", "WBMP", "ICO",
    };
    for (int i = 0; i < FLAGS_images.count(); i++) {
        const char* flag = FLAGS_images[i];
        if (sk_isdir(flag)) {
            for (size_t j = 0; j < SK_ARRAY_COUNT(exts); j++) {
                SkOSFile::Iter it(flag, exts[j]);
                for (SkString file; it.next(&file); ) {
                    SkString path = SkOSPath::Join(flag, file.c_str());
                    push_src("image", "decode", new ImageSrc(path)); // Decode entire image
                    push_src("image", "subset", new ImageSrc(path, 2)); // Decode into 2x2 subsets
                    if (codec_supported(exts[j])) {
                        push_codec_srcs(path);
                    }
                }
            }
        } else if (sk_exists(flag)) {
            // assume that FLAGS_images[i] is a valid image if it is a file.
            push_src("image", "decode", new ImageSrc(flag)); // Decode entire image.
            push_src("image", "subset", new ImageSrc(flag, 2)); // Decode into 2 x 2 subsets
            push_codec_srcs(flag);
        }
    }
}

static GrGLStandard get_gpu_api() {
    if (FLAGS_gpuAPI.contains("gl"))   { return kGL_GrGLStandard; }
    if (FLAGS_gpuAPI.contains("gles")) { return kGLES_GrGLStandard; }
    return kNone_GrGLStandard;
}

static void push_sink(const char* tag, Sink* s) {
    SkAutoTDelete<Sink> sink(s);
    if (!FLAGS_config.contains(tag)) {
        return;
    }
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
        SkDebugf("Could not run %s: %s\n", tag, err.c_str());
        exit(1);
    }

    TaggedSink& ts = gSinks.push_back();
    ts.reset(sink.detach());
    ts.tag = tag;
}

static bool gpu_supported() {
#if SK_SUPPORT_GPU
    return FLAGS_gpu;
#else
    return false;
#endif
}

static Sink* create_sink(const char* tag) {
#define SINK(t, sink, ...) if (0 == strcmp(t, tag)) { return new sink(__VA_ARGS__); }
    if (gpu_supported()) {
        typedef GrContextFactory Gr;
        const GrGLStandard api = get_gpu_api();
        SINK("gpunull",    GPUSink, Gr::kNull_GLContextType,   api,  0, false, FLAGS_gpu_threading);
        SINK("gpudebug",   GPUSink, Gr::kDebug_GLContextType,  api,  0, false, FLAGS_gpu_threading);
        SINK("gpu",        GPUSink, Gr::kNative_GLContextType, api,  0, false, FLAGS_gpu_threading);
        SINK("gpudft",     GPUSink, Gr::kNative_GLContextType, api,  0,  true, FLAGS_gpu_threading);
        SINK("msaa4",      GPUSink, Gr::kNative_GLContextType, api,  4, false, FLAGS_gpu_threading);
        SINK("msaa16",     GPUSink, Gr::kNative_GLContextType, api, 16, false, FLAGS_gpu_threading);
        SINK("nvprmsaa4",  GPUSink, Gr::kNVPR_GLContextType,   api,  4, false, FLAGS_gpu_threading);
        SINK("nvprmsaa16", GPUSink, Gr::kNVPR_GLContextType,   api, 16, false, FLAGS_gpu_threading);
    #if SK_ANGLE
        SINK("angle",      GPUSink, Gr::kANGLE_GLContextType,  api,  0, false, FLAGS_gpu_threading);
    #endif
    #if SK_MESA
        SINK("mesa",       GPUSink, Gr::kMESA_GLContextType,   api,  0, false, FLAGS_gpu_threading);
    #endif
    }

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    SINK("hwui",           HWUISink);
#endif

    if (FLAGS_cpu) {
        SINK("565",  RasterSink, kRGB_565_SkColorType);
        SINK("8888", RasterSink, kN32_SkColorType);
        SINK("pdf",  PDFSink);
        SINK("skp",  SKPSink);
        SINK("svg",  SVGSink);
        SINK("null", NullSink);
        SINK("xps",  XPSSink);
    }
#undef SINK
    return NULL;
}

static Sink* create_via(const char* tag, Sink* wrapped) {
#define VIA(t, via, ...) if (0 == strcmp(t, tag)) { return new via(__VA_ARGS__); }
    VIA("twice",     ViaTwice,         wrapped);
    VIA("pipe",      ViaPipe,          wrapped);
    VIA("serialize", ViaSerialization, wrapped);
    VIA("deferred",  ViaDeferred,      wrapped);
    VIA("2ndpic",    ViaSecondPicture, wrapped);
    VIA("sp",        ViaSingletonPictures, wrapped);
    VIA("tiles",     ViaTiles, 256, 256,               NULL, wrapped);
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
    return NULL;
}

static void gather_sinks() {
    for (int i = 0; i < FLAGS_config.count(); i++) {
        const char* config = FLAGS_config[i];
        SkTArray<SkString> parts;
        SkStrSplit(config, "-", &parts);

        Sink* sink = NULL;
        for (int i = parts.count(); i-- > 0;) {
            const char* part = parts[i].c_str();
            Sink* next = (sink == NULL) ? create_sink(part) : create_via(part, sink);
            if (next == NULL) {
                SkDebugf("Skipping %s: Don't understand '%s'.\n", config, part);
                delete sink;
                sink = NULL;
                break;
            }
            sink = next;
        }
        if (sink) {
            push_sink(config, sink);
        }
    }
}

static bool match(const char* needle, const char* haystack) {
    return 0 == strcmp("_", needle) || NULL != strstr(haystack, needle);
}

static ImplicitString is_blacklisted(const char* sink, const char* src,
                                     const char* srcOptions, const char* name) {
    for (int i = 0; i < FLAGS_blacklist.count() - 3; i += 4) {
        if (match(FLAGS_blacklist[i+0], sink) &&
            match(FLAGS_blacklist[i+1], src) &&
            match(FLAGS_blacklist[i+2], srcOptions) &&
            match(FLAGS_blacklist[i+3], name)) {
            return SkStringPrintf("%s %s %s %s",
                                  FLAGS_blacklist[i+0], FLAGS_blacklist[i+1],
                                  FLAGS_blacklist[i+2], FLAGS_blacklist[i+3]);
        }
    }
    return "";
}

// The finest-grained unit of work we can run: draw a single Src into a single Sink,
// report any errors, and perhaps write out the output: a .png of the bitmap, or a raw stream.
struct Task {
    Task(const TaggedSrc& src, const TaggedSink& sink) : src(src), sink(sink) {}
    const TaggedSrc&  src;
    const TaggedSink& sink;

    static void Run(Task* task) {
        SkString name = task->src->name();

        // We'll skip drawing this Src/Sink pair if:
        //   - the Src vetoes the Sink;
        //   - this Src / Sink combination is on the blacklist;
        //   - it's a dry run.
        SkString note(task->src->veto(task->sink->flags()) ? " (veto)" : "");
        SkString whyBlacklisted = is_blacklisted(task->sink.tag, task->src.tag,
                                                 task->src.options, name.c_str());
        if (!whyBlacklisted.isEmpty()) {
            note.appendf(" (--blacklist %s)", whyBlacklisted.c_str());
        }

        SkString log;
        WallTimer timer;
        timer.start();
        if (!FLAGS_dryRun && note.isEmpty()) {
            SkBitmap bitmap;
            SkDynamicMemoryWStream stream;
            if (FLAGS_pre_log) {
                SkDebugf("\nRunning %s->%s", name.c_str(), task->sink.tag);
            }
            start(task->sink.tag, task->src.tag, task->src.options, name.c_str());
            Error err = task->sink->draw(*task->src, &bitmap, &stream, &log);
            if (!err.isEmpty()) {
                timer.end();
                if (err.isFatal()) {
                    fail(SkStringPrintf("%s %s %s %s: %s",
                                        task->sink.tag,
                                        task->src.tag,
                                        task->src.options,
                                        name.c_str(),
                                        err.c_str()));
                } else {
                    note.appendf(" (skipped: %s)", err.c_str());
                }
                done(timer.fWall, task->sink.tag, task->src.tag, task->src.options,
                     name, note, log);
                return;
            }
            SkAutoTDelete<SkStreamAsset> data(stream.detachAsStream());

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
                !gGold.contains(Gold(task->sink.tag, task->src.tag,
                                     task->src.options, name, md5))) {
                fail(SkStringPrintf("%s not found for %s %s %s %s in %s",
                                    md5.c_str(),
                                    task->sink.tag,
                                    task->src.tag,
                                    task->src.options,
                                    name.c_str(),
                                    FLAGS_readPath[0]));
            }

            if (!FLAGS_writePath.isEmpty()) {
                const char* ext = task->sink->fileExtension();
                if (data->getLength()) {
                    WriteToDisk(*task, md5, ext, data, data->getLength(), NULL);
                    SkASSERT(bitmap.drawsNothing());
                } else if (!bitmap.drawsNothing()) {
                    WriteToDisk(*task, md5, ext, NULL, 0, &bitmap);
                }
            }
        }
        timer.end();
        done(timer.fWall, task->sink.tag, task->src.tag, task->src.options, name, note, log);
    }

    static void WriteToDisk(const Task& task,
                            SkString md5,
                            const char* ext,
                            SkStream* data, size_t len,
                            const SkBitmap* bitmap) {
        JsonWriter::BitmapResult result;
        result.name          = task.src->name();
        result.config        = task.sink.tag;
        result.sourceType    = task.src.tag;
        result.sourceOptions = task.src.options;
        result.ext           = ext;
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
            path = SkOSPath::Join(dir, task.sink.tag);
            sk_mkdir(path.c_str());
            path = SkOSPath::Join(path.c_str(), task.src.tag);
            sk_mkdir(path.c_str());
            if (strcmp(task.src.options, "") != 0) {
              path = SkOSPath::Join(path.c_str(), task.src.options);
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
            // We can't encode A8 bitmaps as PNGs.  Convert them to 8888 first.
            SkBitmap converted;
            if (bitmap->info().colorType() == kAlpha_8_SkColorType) {
                if (!bitmap->copyTo(&converted, kN32_SkColorType)) {
                    fail("Can't convert A8 to 8888.\n");
                    return;
                }
                bitmap = &converted;
            }
            if (!SkImageEncoder::EncodeStream(&file, *bitmap, SkImageEncoder::kPNG_Type, 100)) {
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

// Run all tasks in the same enclave serially on the same thread.
// They can't possibly run concurrently with each other.
static void run_enclave(SkTArray<Task>* tasks) {
    for (int i = 0; i < tasks->count(); i++) {
        Task::Run(tasks->begin() + i);
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

// Unit tests don't fit so well into the Src/Sink model, so we give them special treatment.

static SkTDArray<skiatest::Test> gThreadedTests, gGPUTests;

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
            (FLAGS_gpu_threading ? gThreadedTests : gGPUTests).push(test);
        } else if (!test.needsGpu && FLAGS_cpu) {
            gThreadedTests.push(test);
        }
    }
}

static void run_test(skiatest::Test* test) {
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

    SkString note;
    SkString whyBlacklisted = is_blacklisted("_", "tests", "_", test->name);
    if (!whyBlacklisted.isEmpty()) {
        note.appendf(" (--blacklist %s)", whyBlacklisted.c_str());
    }

    WallTimer timer;
    timer.start();
    if (!FLAGS_dryRun && whyBlacklisted.isEmpty()) {
        start("unit", "test", "", test->name);
        GrContextFactory factory;
        if (FLAGS_pre_log) {
            SkDebugf("\nRunning test %s", test->name);
        }
        test->proc(&reporter, &factory);
    }
    timer.end();
    done(timer.fWall, "unit", "test", "", test->name, note, "");
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

// If we're isolating all GPU-bound work to one thread (the default), this function runs all that.
static void run_enclave_and_gpu_tests(SkTArray<Task>* tasks) {
    run_enclave(tasks);
    for (int i = 0; i < gGPUTests.count(); i++) {
        run_test(&gGPUTests[i]);
    }
}

// Some runs (mostly, Valgrind) are so slow that the bot framework thinks we've hung.
// This prints something every once in a while so that it knows we're still working.
static void start_keepalive() {
    struct Loop {
        static void forever(void*) {
            for (;;) {
                static const int kSec = 300;
            #if defined(SK_BUILD_FOR_WIN)
                Sleep(kSec * 1000);
            #else
                sleep(kSec);
            #endif
                SkString running;
                {
                    SkAutoMutexAcquire lock(gRunningMutex);
                    for (int i = 0; i < gRunning.count(); i++) {
                        running.appendf("\n\t%s", gRunning[i].c_str());
                    }
                }
                SkDebugf("\nCurrently running:%s\n", running.c_str());
            }
        }
    };
    static SkThread* intentionallyLeaked = new SkThread(Loop::forever);
    intentionallyLeaked->start();
}

#define PORTABLE_FONT_PREFIX "Toy Liberation "

static SkTypeface* create_from_name(const char familyName[], SkTypeface::Style style) {
    if (familyName && strlen(familyName) > sizeof(PORTABLE_FONT_PREFIX)
            && !strncmp(familyName, PORTABLE_FONT_PREFIX, sizeof(PORTABLE_FONT_PREFIX) - 1)) {
        return sk_tool_utils::create_portable_typeface(familyName, style);
    }
    return NULL;
}

#undef PORTABLE_FONT_PREFIX

extern SkTypeface* (*gCreateTypefaceDelegate)(const char [], SkTypeface::Style );

int dm_main();
int dm_main() {
    SetupCrashHandler();
    SkAutoGraphics ag;
    SkTaskGroup::Enabler enabled(FLAGS_threads);
    gCreateTypefaceDelegate = &create_from_name;

    start_keepalive();

    gather_gold();
    gather_uninteresting_hashes();

    gather_srcs();
    gather_sinks();
    gather_tests();

    gPending = gSrcs.count() * gSinks.count() + gThreadedTests.count() + gGPUTests.count();
    SkDebugf("%d srcs * %d sinks + %d tests == %d tasks\n",
             gSrcs.count(), gSinks.count(), gThreadedTests.count() + gGPUTests.count(), gPending);

    // We try to exploit as much parallelism as is safe.  Most Src/Sink pairs run on any thread,
    // but Sinks that identify as part of a particular enclave run serially on a single thread.
    // CPU tests run on any thread.  GPU tests depend on --gpu_threading.
    SkTArray<Task> enclaves[kNumEnclaves];
    for (int j = 0; j < gSinks.count(); j++) {
        SkTArray<Task>& tasks = enclaves[gSinks[j]->enclave()];
        for (int i = 0; i < gSrcs.count(); i++) {
            tasks.push_back(Task(gSrcs[i], gSinks[j]));
        }
    }

    SkTaskGroup tg;
    tg.batch(run_test, gThreadedTests.begin(), gThreadedTests.count());
    for (int i = 0; i < kNumEnclaves; i++) {
        switch(i) {
            case kAnyThread_Enclave:
                tg.batch(Task::Run, enclaves[i].begin(), enclaves[i].count());
                break;
            case kGPU_Enclave:
                tg.add(run_enclave_and_gpu_tests, &enclaves[i]);
                break;
            default:
                tg.add(run_enclave, &enclaves[i]);
                break;
        }
    }
    tg.wait();
    // At this point we're back in single-threaded land.
    sk_tool_utils::release_portable_typefaces();

    SkDebugf("\n");
    if (gFailures.count() > 0) {
        SkDebugf("Failures:\n");
        for (int i = 0; i < gFailures.count(); i++) {
            SkDebugf("\t%s\n", gFailures[i].c_str());
        }
        SkDebugf("%d failures\n", gFailures.count());
        return 1;
    }
    if (gPending > 0) {
        SkDebugf("Hrm, we didn't seem to run everything we intended to!  Please file a bug.\n");
        return 1;
    }
    return 0;
}

#if !defined(SK_BUILD_FOR_IOS)
int main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
    return dm_main();
}
#endif
