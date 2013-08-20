
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "BenchTimer.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrContextFactory.h"
#include "gl/GrGLDefines.h"
#include "GrRenderTarget.h"
#include "SkGpuDevice.h"
#else
class GrContext;
#endif // SK_SUPPORT_GPU

#include "SkBenchLogger.h"
#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkDeferredCanvas.h"
#include "SkDevice.h"
#include "SkColorPriv.h"
#include "SkGraphics.h"
#include "SkImageEncoder.h"
#include "SkNWayCanvas.h"
#include "SkPicture.h"
#include "SkString.h"
#include "SkTArray.h"
#include "TimerData.h"

enum benchModes {
    kNormal_benchModes,
    kDeferred_benchModes,
    kDeferredSilent_benchModes,
    kRecord_benchModes,
    kPictureRecord_benchModes
};

///////////////////////////////////////////////////////////////////////////////

static void erase(SkBitmap& bm) {
    if (bm.config() == SkBitmap::kA8_Config) {
        bm.eraseColor(SK_ColorTRANSPARENT);
    } else {
        bm.eraseColor(SK_ColorWHITE);
    }
}

#if 0
static bool equal(const SkBitmap& bm1, const SkBitmap& bm2) {
    if (bm1.width() != bm2.width() ||
        bm1.height() != bm2.height() ||
        bm1.config() != bm2.config()) {
        return false;
    }

    size_t pixelBytes = bm1.width() * bm1.bytesPerPixel();
    for (int y = 0; y < bm1.height(); y++) {
        if (memcmp(bm1.getAddr(0, y), bm2.getAddr(0, y), pixelBytes)) {
            return false;
        }
    }
    return true;
}
#endif

class Iter {
public:
    Iter(void* param) {
        fBench = BenchRegistry::Head();
        fParam = param;
    }

    SkBenchmark* next() {
        if (fBench) {
            BenchRegistry::Factory f = fBench->factory();
            fBench = fBench->next();
            return f(fParam);
        }
        return NULL;
    }

private:
    const BenchRegistry* fBench;
    void* fParam;
};

class AutoPrePostDraw {
public:
    AutoPrePostDraw(SkBenchmark* bench) : fBench(bench) {
        fBench->preDraw();
    }
    ~AutoPrePostDraw() {
        fBench->postDraw();
    }
private:
    SkBenchmark* fBench;
};

static void make_filename(const char name[], SkString* path) {
    path->set(name);
    for (int i = 0; name[i]; i++) {
        switch (name[i]) {
            case '/':
            case '\\':
            case ' ':
            case ':':
                path->writable_str()[i] = '-';
                break;
            default:
                break;
        }
    }
}

static void saveFile(const char name[], const char config[], const char dir[],
                     const SkBitmap& bm) {
    SkBitmap copy;
    if (!bm.copyTo(&copy, SkBitmap::kARGB_8888_Config)) {
        return;
    }

    if (bm.config() == SkBitmap::kA8_Config) {
        // turn alpha into gray-scale
        size_t size = copy.getSize() >> 2;
        SkPMColor* p = copy.getAddr32(0, 0);
        for (size_t i = 0; i < size; i++) {
            int c = (*p >> SK_A32_SHIFT) & 0xFF;
            c = 255 - c;
            c |= (c << 24) | (c << 16) | (c << 8);
            *p++ = c | (SK_A32_MASK << SK_A32_SHIFT);
        }
    }

    SkString str;
    make_filename(name, &str);
    str.appendf("_%s.png", config);
    str.prepend(dir);
    ::remove(str.c_str());
    SkImageEncoder::EncodeFile(str.c_str(), copy, SkImageEncoder::kPNG_Type,
                               100);
}

static void performClip(SkCanvas* canvas, int w, int h) {
    SkRect r;

    r.set(SkIntToScalar(10), SkIntToScalar(10),
          SkIntToScalar(w*2/3), SkIntToScalar(h*2/3));
    canvas->clipRect(r, SkRegion::kIntersect_Op);

    r.set(SkIntToScalar(w/3), SkIntToScalar(h/3),
          SkIntToScalar(w-10), SkIntToScalar(h-10));
    canvas->clipRect(r, SkRegion::kXOR_Op);
}

static void performRotate(SkCanvas* canvas, int w, int h) {
    const SkScalar x = SkIntToScalar(w) / 2;
    const SkScalar y = SkIntToScalar(h) / 2;

    canvas->translate(x, y);
    canvas->rotate(SkIntToScalar(35));
    canvas->translate(-x, -y);
}

static void performScale(SkCanvas* canvas, int w, int h) {
    const SkScalar x = SkIntToScalar(w) / 2;
    const SkScalar y = SkIntToScalar(h) / 2;

    canvas->translate(x, y);
    // just enough so we can't take the sprite case
    canvas->scale(SK_Scalar1 * 99/100, SK_Scalar1 * 99/100);
    canvas->translate(-x, -y);
}

static bool parse_bool_arg(char * const* argv, char* const* stop, bool* var) {
    if (argv < stop) {
        *var = atoi(*argv) != 0;
        return true;
    }
    return false;
}

enum Backend {
    kNonRendering_Backend,
    kRaster_Backend,
    kGPU_Backend,
    kPDF_Backend,
};

static SkDevice* make_device(SkBitmap::Config config, const SkIPoint& size,
                             Backend backend, int sampleCount, GrContext* context) {
    SkDevice* device = NULL;
    SkBitmap bitmap;
    bitmap.setConfig(config, size.fX, size.fY);

    switch (backend) {
        case kRaster_Backend:
            bitmap.allocPixels();
            erase(bitmap);
            device = SkNEW_ARGS(SkDevice, (bitmap));
            break;
#if SK_SUPPORT_GPU
        case kGPU_Backend: {
            GrTextureDesc desc;
            desc.fConfig = kSkia8888_GrPixelConfig;
            desc.fFlags = kRenderTarget_GrTextureFlagBit;
            desc.fWidth = size.fX;
            desc.fHeight = size.fY;
            desc.fSampleCnt = sampleCount;
            SkAutoTUnref<GrTexture> texture(context->createUncachedTexture(desc, NULL, 0));
            if (!texture) {
                return NULL;
            }
            device = SkNEW_ARGS(SkGpuDevice, (context, texture.get()));
            break;
        }
#endif
        case kPDF_Backend:
        default:
            SkASSERT(!"unsupported");
    }
    return device;
}

#if SK_SUPPORT_GPU
GrContextFactory gContextFactory;
typedef GrContextFactory::GLContextType GLContextType;
static const GLContextType kDontCareGLCtxType = GrContextFactory::kNative_GLContextType;
#else
typedef int GLContextType;
static const GLContextType kDontCareGLCtxType = 0;
#endif

static const struct {
    SkBitmap::Config    fConfig;
    const char*         fName;
    int                 fSampleCnt;
    Backend             fBackend;
    GLContextType       fContextType;
    bool                fRunByDefault;
} gConfigs[] = {
    { SkBitmap::kNo_Config,         "NONRENDERING", 0, kNonRendering_Backend, kDontCareGLCtxType,                      true     },
    { SkBitmap::kARGB_8888_Config,  "8888",         0, kRaster_Backend,       kDontCareGLCtxType,                      true     },
    { SkBitmap::kRGB_565_Config,    "565",          0, kRaster_Backend,       kDontCareGLCtxType,                      true     },
#if SK_SUPPORT_GPU
    { SkBitmap::kARGB_8888_Config,  "GPU",          0, kGPU_Backend,          GrContextFactory::kNative_GLContextType, true     },
    { SkBitmap::kARGB_8888_Config,  "MSAA4",        4, kGPU_Backend,          GrContextFactory::kNative_GLContextType, false    },
    { SkBitmap::kARGB_8888_Config,  "MSAA16",      16, kGPU_Backend,          GrContextFactory::kNative_GLContextType, false    },
#if SK_ANGLE
    { SkBitmap::kARGB_8888_Config,  "ANGLE",        0, kGPU_Backend,          GrContextFactory::kANGLE_GLContextType,  true     },
#endif // SK_ANGLE
#ifdef SK_DEBUG
    { SkBitmap::kARGB_8888_Config,  "Debug",        0, kGPU_Backend,          GrContextFactory::kDebug_GLContextType,  GR_DEBUG },
#endif // SK_DEBUG
    { SkBitmap::kARGB_8888_Config,  "NULLGPU",      0, kGPU_Backend,          GrContextFactory::kNull_GLContextType,   true     },
#endif // SK_SUPPORT_GPU
};

static int findConfig(const char config[]) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gConfigs); i++) {
        if (!strcmp(config, gConfigs[i].fName)) {
            return i;
        }
    }
    return -1;
}

static void help() {
    SkString configsStr;
    static const size_t kConfigCount = SK_ARRAY_COUNT(gConfigs);
    for (size_t i = 0; i < kConfigCount; ++i) {
        configsStr.appendf("%s%s", gConfigs[i].fName, ((i == kConfigCount - 1) ? "" : "|"));
    }

    SkDebugf("Usage: bench [-o outDir] [--repeat nr] [--logPerIter] "
                          "[--timers [wcgWC]*] [--rotate]\n"
             "    [--scale] [--clip] [--min] [--forceAA 1|0] [--forceFilter 1|0]\n"
             "    [--forceDither 1|0] [--forceBlend 1|0]"
#if SK_SUPPORT_GPU
             " [--gpuCacheSize <bytes> <count>]"
#endif
             "\n"
             "    [--strokeWidth width] [--match name]\n"
             "    [--mode normal|deferred|deferredSilent|record|picturerecord]\n"
             "    [--config ");
    SkDebugf("%s]\n", configsStr.c_str());
    SkDebugf("    [-Dfoo bar] [--logFile filename] [-h|--help]");
    SkDebugf("\n\n");
    SkDebugf("    -o outDir : Image of each bench will be put in outDir.\n");
    SkDebugf("    --repeat nr : Each bench repeats for nr times.\n");
    SkDebugf("    --logPerIter : "
             "Log each repeat timer instead of mean, default is disabled.\n");
    SkDebugf("    --timers [wcgWC]* : "
             "Display wall, cpu, gpu, truncated wall or truncated cpu time for each bench.\n");
    SkDebugf("    --rotate : Rotate before each bench runs.\n");
    SkDebugf("    --scale : Scale before each bench runs.\n");
    SkDebugf("    --clip : Clip before each bench runs.\n");
    SkDebugf("    --min : Print the minimum times (instead of average).\n");
    SkDebugf("    --forceAA 1|0 : "
             "Enable/disable anti-aliased, default is enabled.\n");
    SkDebugf("    --forceFilter 1|0 : "
             "Enable/disable bitmap filtering, default is disabled.\n");
    SkDebugf("    --forceDither 1|0 : "
             "Enable/disable dithering, default is disabled.\n");
    SkDebugf("    --forceBlend 1|0 : "
             "Enable/disable dithering, default is disabled.\n");
#if SK_SUPPORT_GPU
    SkDebugf("    --gpuCacheSize <bytes> <count>: "
             "limits gpu cache to  bytes size or object count.\n");
    SkDebugf("      -1 for either value means use the default. 0 for either disables the cache.\n");
#endif
    SkDebugf("    --strokeWidth width : The width for path stroke.\n");
    SkDebugf("    --match [~][^]substring[$] [...] of test name to run.\n"
             "             Multiple matches may be separated by spaces.\n"
             "             ~ causes a matching test to always be skipped\n"
             "             ^ requires the start of the test to match\n"
             "             $ requires the end of the test to match\n"
             "             ^ and $ requires an exact match\n"
             "             If a test does not match any list entry,\n"
             "             it is skipped unless some list entry starts with ~\n");
    SkDebugf("    --mode normal|deferred|deferredSilent|record|picturerecord :\n"
             "             Run in the corresponding mode\n"
             "                 normal, Use a normal canvas to draw to;\n"
             "                 deferred, Use a deferrred canvas when drawing;\n"
             "                 deferredSilent, deferred with silent playback;\n"
             "                 record, Benchmark the time to record to an SkPicture;\n"
             "                 picturerecord, Benchmark the time to do record from a \n"
             "                                SkPicture to a SkPicture.\n");
    SkDebugf("    --logFile filename : destination for writing log output, in addition to stdout.\n");
    SkDebugf("    --config %s:\n", configsStr.c_str());
    SkDebugf("             Run bench in corresponding config mode.\n");
    SkDebugf("    -Dfoo bar : Add extra definition to bench.\n");
    SkDebugf("    -h|--help : Show this help message.\n");
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
#if SK_ENABLE_INST_COUNT
    gPrintInstCount = true;
#endif
    SkAutoGraphics ag;

    SkTDict<const char*> defineDict(1024);
    int repeatDraw = 1;

    int forceAlpha = 0xFF;
    bool forceAA = true;
    bool forceFilter = false;
    SkTriState::State forceDither = SkTriState::kDefault;

    static const uint32_t kDefaultTimerTypes = TimerData::kCpu_Flag | TimerData::kGpu_Flag;
    static const TimerData::Result kDefaultTimerResult = TimerData::kAvg_Result;
    uint32_t timerTypes = kDefaultTimerTypes;
    TimerData::Result timerResult = kDefaultTimerResult;

    bool doScale = false;
    bool doRotate = false;
    bool doClip = false;
    bool hasStrokeWidth = false;

#if SK_SUPPORT_GPU
    struct {
        int     fBytes;
        int     fCount;
    } gpuCacheSize = { -1, -1 }; // -1s mean use the default
#endif

    float strokeWidth;
    SkTDArray<const char*> fMatches;
    benchModes benchMode = kNormal_benchModes;
    SkString perIterTimeformat("%.2f");
    SkString normalTimeFormat("%6.2f");

    SkString outDir;
    SkBitmap::Config outConfig = SkBitmap::kNo_Config;
    const char* configName = "";
    Backend backend = kRaster_Backend;  // for warning
    int sampleCount = 0;
    SkTDArray<int> configs;
    bool userConfig = false;

    SkBenchLogger logger;

    char* const* stop = argv + argc;
    for (++argv; argv < stop; ++argv) {
        if (strcmp(*argv, "-o") == 0) {
            argv++;
            if (argv < stop && **argv) {
                outDir.set(*argv);
                if (outDir.c_str()[outDir.size() - 1] != '/') {
                    outDir.append("/");
                }
            }
        } else if (strcmp(*argv, "--repeat") == 0) {
            argv++;
            if (argv < stop) {
                repeatDraw = atoi(*argv);
                if (repeatDraw < 1) {
                    repeatDraw = 1;
                }
            } else {
                logger.logError("missing arg for --repeat\n");
                help();
                return -1;
            }
        } else if (strcmp(*argv, "--logPerIter") == 0) {
            timerResult = TimerData::kPerIter_Result;
        } else if (strcmp(*argv, "--timers") == 0) {
            argv++;
            if (argv < stop) {
                timerTypes = 0;
                for (char* t = *argv; *t; ++t) {
                    switch (*t) {
                    case 'w': timerTypes |= TimerData::kWall_Flag; break;
                    case 'c': timerTypes |= TimerData::kCpu_Flag; break;
                    case 'W': timerTypes |= TimerData::kTruncatedWall_Flag; break;
                    case 'C': timerTypes |= TimerData::kTruncatedCpu_Flag; break;
                    case 'g': timerTypes |= TimerData::kGpu_Flag; break;
                    }
                }
            } else {
                logger.logError("missing arg for --timers\n");
                help();
                return -1;
            }
        } else if (!strcmp(*argv, "--rotate")) {
            doRotate = true;
        } else if (!strcmp(*argv, "--scale")) {
            doScale = true;
        } else if (!strcmp(*argv, "--clip")) {
            doClip = true;
        } else if (!strcmp(*argv, "--min")) {
            timerResult = TimerData::kMin_Result;
        } else if (strcmp(*argv, "--forceAA") == 0) {
            if (!parse_bool_arg(++argv, stop, &forceAA)) {
                logger.logError("missing arg for --forceAA\n");
                help();
                return -1;
            }
        } else if (strcmp(*argv, "--forceFilter") == 0) {
            if (!parse_bool_arg(++argv, stop, &forceFilter)) {
                logger.logError("missing arg for --forceFilter\n");
                help();
                return -1;
            }
        } else if (strcmp(*argv, "--forceDither") == 0) {
            bool tmp;
            if (!parse_bool_arg(++argv, stop, &tmp)) {
                logger.logError("missing arg for --forceDither\n");
                help();
                return -1;
            }
            forceDither = tmp ? SkTriState::kTrue : SkTriState::kFalse;
        } else if (strcmp(*argv, "--forceBlend") == 0) {
            bool wantAlpha = false;
            if (!parse_bool_arg(++argv, stop, &wantAlpha)) {
                logger.logError("missing arg for --forceBlend\n");
                help();
                return -1;
            }
            forceAlpha = wantAlpha ? 0x80 : 0xFF;
#if SK_SUPPORT_GPU
        } else if (strcmp(*argv, "--gpuCacheSize") == 0) {
            if (stop - argv > 2) {
                gpuCacheSize.fBytes = atoi(*++argv);
                gpuCacheSize.fCount = atoi(*++argv);
            } else {
                SkDebugf("missing arg for --gpuCacheSize\n");
                help();
                return -1;
            }
#endif
        } else if (strcmp(*argv, "--mode") == 0) {
            argv++;
            if (argv < stop) {
                if (strcmp(*argv, "normal") == 0) {
                    benchMode = kNormal_benchModes;
                } else if (strcmp(*argv, "deferred") == 0) {
                    benchMode = kDeferred_benchModes;
                } else if (strcmp(*argv, "deferredSilent") == 0) {
                    benchMode = kDeferredSilent_benchModes;
                } else if (strcmp(*argv, "record") == 0) {
                    benchMode = kRecord_benchModes;
                } else if (strcmp(*argv, "picturerecord") == 0) {
                    benchMode = kPictureRecord_benchModes;
                } else {
                    logger.logError("bad arg for --mode\n");
                    help();
                    return -1;
                }
            } else {
                logger.logError("missing arg for --mode\n");
                help();
                return -1;
            }
        } else if (strcmp(*argv, "--strokeWidth") == 0) {
            argv++;
            if (argv < stop) {
                const char *strokeWidthStr = *argv;
                if (sscanf(strokeWidthStr, "%f", &strokeWidth) != 1) {
                  logger.logError("bad arg for --strokeWidth\n");
                  help();
                  return -1;
                }
                hasStrokeWidth = true;
            } else {
                logger.logError("missing arg for --strokeWidth\n");
                help();
                return -1;
            }
        } else if (strcmp(*argv, "--match") == 0) {
            argv++;
            while (argv < stop && (*argv)[0] != '-') {
                *fMatches.append() = *argv++;
            }
            argv--;
            if (!fMatches.count()) {
                logger.logError("missing arg for --match\n");
                help();
                return -1;
            }
        } else if (strcmp(*argv, "--config") == 0) {
            argv++;
            if (argv < stop) {
                int index = findConfig(*argv);
                if (index >= 0) {
                    *configs.append() = index;
                    userConfig = true;
                } else {
                    SkString str;
                    str.printf("unrecognized config %s\n", *argv);
                    logger.logError(str);
                    help();
                    return -1;
                }
            } else {
                logger.logError("missing arg for --config\n");
                help();
                return -1;
            }
        } else if (strcmp(*argv, "--logFile") == 0) {
            argv++;
            if (argv < stop) {
                if (!logger.SetLogFile(*argv)) {
                    SkString str;
                    str.printf("Could not open %s for writing.", *argv);
                    logger.logError(str);
                    return -1;
                }
            } else {
                logger.logError("missing arg for --logFile\n");
                help();
                return -1;
            }
        } else if (strlen(*argv) > 2 && strncmp(*argv, "-D", 2) == 0) {
            argv++;
            if (argv < stop) {
                defineDict.set(argv[-1] + 2, *argv);
            } else {
                logger.logError("incomplete '-Dfoo bar' definition\n");
                help();
                return -1;
            }
        } else if (strcmp(*argv, "--help") == 0 || strcmp(*argv, "-h") == 0) {
            help();
            return 0;
        } else {
            SkString str;
            str.printf("unrecognized arg %s\n", *argv);
            logger.logError(str);
            help();
            return -1;
        }
    }
    if ((benchMode == kRecord_benchModes || benchMode == kPictureRecord_benchModes)
            && !outDir.isEmpty()) {
        logger.logError("'--mode record' and '--mode picturerecord' are not"
                  " compatible with -o.\n");
        return -1;
    }
    if ((benchMode == kRecord_benchModes || benchMode == kPictureRecord_benchModes)) {
        perIterTimeformat.set("%.4f");
        normalTimeFormat.set("%6.4f");
    }
    if (!userConfig) {
        // if no config is specified by user, add the default configs
        for (unsigned int i = 0; i < SK_ARRAY_COUNT(gConfigs); ++i) {
            if (gConfigs[i].fRunByDefault) {
                *configs.append() = i;
            }
        }
    }
    if (kNormal_benchModes != benchMode) {
        // Non-rendering configs only run in normal mode
        for (int i = 0; i < configs.count(); ++i) {
            int configIdx = configs[i];
            if (kNonRendering_Backend == gConfigs[configIdx].fBackend) {
                configs.remove(i, 1);
                --i;
            }
        }
    }

#if SK_SUPPORT_GPU
    for (int i = 0; i < configs.count(); ++i) {
        int configIdx = configs[i];

        if (kGPU_Backend == gConfigs[configIdx].fBackend && gConfigs[configIdx].fSampleCnt > 0) {
            GrContext* context = gContextFactory.get(gConfigs[configIdx].fContextType);
            if (NULL == context) {
                SkString error;
                error.printf("Error creating GrContext for config %s. Config will be skipped.\n",
                             gConfigs[configIdx].fName);
                logger.logError(error.c_str());
                configs.remove(i);
                --i;
                continue;
            }
            if (gConfigs[configIdx].fSampleCnt > context->getMaxSampleCount()){
                SkString error;
                error.printf("Sample count (%d) for config %s is unsupported. "
                             "Config will be skipped.\n",
                             gConfigs[configIdx].fSampleCnt, gConfigs[configIdx].fName);
                logger.logError(error.c_str());
                configs.remove(i);
                --i;
                continue;
            }
        }
    }
#endif

    // report our current settings
    {
        SkString str;
        const char* deferredMode = benchMode == kDeferred_benchModes ? "yes" :
            (benchMode == kDeferredSilent_benchModes ? "silent" : "no");
        str.printf("skia bench: alpha=0x%02X antialias=%d filter=%d "
                   "deferred=%s logperiter=%d",
                   forceAlpha, forceAA, forceFilter, deferredMode,
                   TimerData::kPerIter_Result == timerResult);
        str.appendf(" rotate=%d scale=%d clip=%d min=%d",
                   doRotate, doScale, doClip, TimerData::kMin_Result == timerResult);
        str.appendf(" record=%d picturerecord=%d",
                    benchMode == kRecord_benchModes,
                    benchMode == kPictureRecord_benchModes);
        const char * ditherName;
        switch (forceDither) {
            case SkTriState::kDefault: ditherName = "default"; break;
            case SkTriState::kTrue: ditherName = "true"; break;
            case SkTriState::kFalse: ditherName = "false"; break;
            default: ditherName = "<invalid>"; break;
        }
        str.appendf(" dither=%s", ditherName);

        if (hasStrokeWidth) {
            str.appendf(" strokeWidth=%f", strokeWidth);
        } else {
            str.append(" strokeWidth=none");
        }

#if defined(SK_SCALAR_IS_FLOAT)
        str.append(" scalar=float");
#elif defined(SK_SCALAR_IS_FIXED)
        str.append(" scalar=fixed");
#endif

#if defined(SK_BUILD_FOR_WIN32)
        str.append(" system=WIN32");
#elif defined(SK_BUILD_FOR_MAC)
        str.append(" system=MAC");
#elif defined(SK_BUILD_FOR_ANDROID)
        str.append(" system=ANDROID");
#elif defined(SK_BUILD_FOR_UNIX)
        str.append(" system=UNIX");
#else
        str.append(" system=other");
#endif

#if defined(SK_DEBUG)
        str.append(" DEBUG");
#endif
        str.append("\n");
        logger.logProgress(str);
    }

    SkTArray<BenchTimer*> timers(SK_ARRAY_COUNT(gConfigs));
    for (size_t i = 0; i < SK_ARRAY_COUNT(gConfigs); ++i) {
#if SK_SUPPORT_GPU
        SkGLContextHelper* glCtx = NULL;
        if (kGPU_Backend == gConfigs[i].fBackend) {
            GrContext* context = gContextFactory.get(gConfigs[i].fContextType);
            if (NULL != context) {
                // Set the user specified cache limits if non-default.
                size_t bytes;
                int count;
                context->getTextureCacheLimits(&count, &bytes);
                if (-1 != gpuCacheSize.fBytes) {
                    bytes = static_cast<size_t>(gpuCacheSize.fBytes);
                }
                if (-1 != gpuCacheSize.fCount) {
                    count = gpuCacheSize.fCount;
                }
                context->setTextureCacheLimits(count, bytes);
            }
            glCtx = gContextFactory.getGLContext(gConfigs[i].fContextType);
        }
        timers.push_back(SkNEW_ARGS(BenchTimer, (glCtx)));
#else
        timers.push_back(SkNEW(BenchTimer));
#endif
    }

    Iter iter(&defineDict);
    SkBenchmark* bench;
    while ((bench = iter.next()) != NULL) {
        SkAutoTUnref<SkBenchmark> benchUnref(bench);

        SkIPoint dim = bench->getSize();
        if (dim.fX <= 0 || dim.fY <= 0) {
            continue;
        }

        bench->setForceAlpha(forceAlpha);
        bench->setForceAA(forceAA);
        bench->setForceFilter(forceFilter);
        bench->setDither(forceDither);
        if (hasStrokeWidth) {
            bench->setStrokeWidth(strokeWidth);
        }

        // only run benchmarks if their name contains matchStr
        if (SkCommandLineFlags::ShouldSkip(fMatches, bench->getName())) {
            continue;
        }

        bool loggedBenchStart = false;

        AutoPrePostDraw appd(bench);

        for (int x = 0; x < configs.count(); ++x) {
            int configIndex = configs[x];

            bool setupFailed = false;

            if (kNonRendering_Backend == gConfigs[configIndex].fBackend) {
                if (bench->isRendering()) {
                    continue;
                }
            } else {
                if (!bench->isRendering()) {
                    continue;
                }
            }

            outConfig = gConfigs[configIndex].fConfig;
            configName = gConfigs[configIndex].fName;
            backend = gConfigs[configIndex].fBackend;
            sampleCount = gConfigs[configIndex].fSampleCnt;
            GrContext* context = NULL;
            BenchTimer* timer = timers[configIndex];

#if SK_SUPPORT_GPU
            SkGLContextHelper* glContext = NULL;
            if (kGPU_Backend == backend) {
                context = gContextFactory.get(gConfigs[configIndex].fContextType);
                if (NULL == context) {
                    continue;
                }
                glContext = gContextFactory.getGLContext(gConfigs[configIndex].fContextType);
            }
#endif
            SkDevice* device = NULL;
            SkCanvas* canvas = NULL;
            SkPicture pictureRecordFrom;
            SkPicture pictureRecordTo;

            if (kNonRendering_Backend != backend) {
                device = make_device(outConfig, dim, backend, sampleCount, context);
                if (NULL == device) {
                    SkString error;
                    error.printf("Device creation failure for config %s. Will skip.\n", configName);
                    logger.logError(error.c_str());
                    setupFailed = true;
                } else {
                    switch(benchMode) {
                        case kDeferredSilent_benchModes:
                        case kDeferred_benchModes:
                            canvas = SkDeferredCanvas::Create(device);
                            break;
                        case kRecord_benchModes:
                            canvas = pictureRecordTo.beginRecording(dim.fX, dim.fY,
                                SkPicture::kUsePathBoundsForClip_RecordingFlag);
                            canvas->ref();
                            break;
                        case kPictureRecord_benchModes: {
                            // This sets up picture-to-picture recording.
                            // The C++ drawing calls for the benchmark are recorded into
                            // pictureRecordFrom. As the benchmark, we will time how
                            // long it takes to playback pictureRecordFrom into
                            // pictureRecordTo.
                            SkCanvas* tempCanvas = pictureRecordFrom.beginRecording(dim.fX, dim.fY,
                                SkPicture::kUsePathBoundsForClip_RecordingFlag);
                            bench->draw(tempCanvas);
                            pictureRecordFrom.endRecording();
                            canvas = pictureRecordTo.beginRecording(dim.fX, dim.fY,
                                SkPicture::kUsePathBoundsForClip_RecordingFlag);
                            canvas->ref();
                            break;
                        }
                        case kNormal_benchModes:
                            canvas = new SkCanvas(device);
                            break;
                        default:
                            SkASSERT(0);
                    }
                    device->unref();
                    canvas->clear(SK_ColorWHITE);
                }
            }
            SkAutoUnref canvasUnref(canvas);
            if (!setupFailed) {
                if (NULL != canvas) {
                    if (doClip) {
                        performClip(canvas, dim.fX, dim.fY);
                    }
                    if (doScale) {
                        performScale(canvas, dim.fX, dim.fY);
                    }
                    if (doRotate) {
                        performRotate(canvas, dim.fX, dim.fY);
                    }
                }

                if (!loggedBenchStart) {
                    loggedBenchStart = true;
                    SkString str;
                    str.printf("running bench [%d %d] %28s", dim.fX, dim.fY, bench->getName());
                    logger.logProgress(str);
                }

                // warm up caches if needed
                if (repeatDraw > 1 && NULL != canvas) {
#if SK_SUPPORT_GPU
                    // purge the GPU resources to reduce variance
                    if (NULL != context) {
                        context->freeGpuResources();
                    }
#endif
                    SkAutoCanvasRestore acr(canvas, true);
                    if (benchMode == kPictureRecord_benchModes) {
                        pictureRecordFrom.draw(canvas);
                    } else {
                        bench->draw(canvas);
                    }

                    if (kDeferredSilent_benchModes == benchMode) {
                        static_cast<SkDeferredCanvas*>(canvas)->silentFlush();
                    } else {
                        canvas->flush();
                    }
#if SK_SUPPORT_GPU
                    if (NULL != context) {
                        context->flush();
                        SK_GL(*glContext, Finish());
                    }
#endif
                }

                // record timer values for each repeat, and their sum
                TimerData timerData(repeatDraw);
                for (int i = 0; i < repeatDraw; i++) {
                    if ((benchMode == kRecord_benchModes || benchMode == kPictureRecord_benchModes)) {
                        // This will clear the recorded commands so that they do not
                        // accumulate.
                        canvas = pictureRecordTo.beginRecording(dim.fX, dim.fY,
                            SkPicture::kUsePathBoundsForClip_RecordingFlag);
                    }

                    timer->start(bench->getDurationScale());
                    if (NULL != canvas) {
                        canvas->save();
                    }
                    if (benchMode == kPictureRecord_benchModes) {
                        pictureRecordFrom.draw(canvas);
                    } else {
                        bench->draw(canvas);
                    }

                    if (kDeferredSilent_benchModes == benchMode) {
                        static_cast<SkDeferredCanvas*>(canvas)->silentFlush();
                    } else if (NULL != canvas) {
                        canvas->flush();
                    }

                    if (NULL != canvas) {
                        canvas->restore();
                    }

                    // stop the truncated timer after the last canvas call but
                    // don't wait for all the GL calls to complete
                    timer->truncatedEnd();
#if SK_SUPPORT_GPU
                    if (NULL != glContext) {
                        context->flush();
                        SK_GL(*glContext, Finish());
                    }
#endif
                    // stop the inclusive and gpu timers once all the GL calls
                    // have completed
                    timer->end();

                    SkAssertResult(timerData.appendTimes(timer));

                }
                if (repeatDraw > 1) {
                    const char* timeFormat;
                    if (TimerData::kPerIter_Result == timerResult) {
                        timeFormat = perIterTimeformat.c_str();
                    } else {
                        timeFormat = normalTimeFormat.c_str();
                    }
                    uint32_t filteredTimerTypes = timerTypes;
                    if (NULL == context) {
                        filteredTimerTypes &= ~TimerData::kGpu_Flag;
                    }
                    SkString result = timerData.getResult(timeFormat,
                                        timerResult,
                                        configName,
                                        filteredTimerTypes);
                    logger.logProgress(result);
                }
                if (outDir.size() > 0 && kNonRendering_Backend != backend) {
                    saveFile(bench->getName(), configName, outDir.c_str(),
                             device->accessBitmap(false));
                }
            }
        }
        if (loggedBenchStart) {
            logger.logProgress(SkString("\n"));
        }
    }
#if SK_SUPPORT_GPU
#if GR_CACHE_STATS
    for (int i = 0; i <= GrContextFactory::kLastGLContextType; ++i) {
        GrContextFactory::GLContextType ctxType = (GrContextFactory::GLContextType)i;
        GrContext* context = gContextFactory.get(ctxType);
        if (NULL != context) {
            SkDebugf("Cache Stats for %s context:\n", GrContextFactory::GLContextTypeName(ctxType));
            context->printCacheStats();
            SkDebugf("\n");
        }
    }
#endif
    // Destroy the GrContext before the inst tracking printing at main() exit occurs.
    gContextFactory.destroyContexts();
#endif
    for (size_t i = 0; i < SK_ARRAY_COUNT(gConfigs); ++i) {
        SkDELETE(timers[i]);
    }

    return 0;
}

#if !defined(SK_BUILD_FOR_IOS) && !defined(SK_BUILD_FOR_NACL)
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
