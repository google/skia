
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "BenchTimer.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrRenderTarget.h"
#if SK_ANGLE
#include "gl/SkANGLEGLContext.h"
#endif // SK_ANGLE
#include "gl/SkNativeGLContext.h"
#include "gl/SkNullGLContext.h"
#include "gl/SkDebugGLContext.h"
#include "SkGpuDevice.h"
#endif // SK_SUPPORT_GPU

#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkDeferredCanvas.h"
#include "SkDevice.h"
#include "SkColorPriv.h"
#include "SkGraphics.h"
#include "SkImageEncoder.h"
#include "SkNWayCanvas.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkString.h"

template <typename T> const T& Min(const T& a, const T& b) {
    return (a < b) ? a : b;
}

class SkBenchLogger {
public:
    SkBenchLogger() : fFileStream(NULL) {}
    ~SkBenchLogger() {
        if (fFileStream)
            SkDELETE(fFileStream);
    }

    bool SetLogFile(const char file[]) {
        fFileStream = SkNEW_ARGS(SkFILEWStream, (file));
        return fFileStream->isValid();
    }

    void logError(const char msg[]) { nativeLogError(msg); }
    void logError(const SkString& str) { nativeLogError(str.c_str()); }

    void logProgress(const char msg[]) {
        nativeLogProgress(msg);
        fileWrite(msg, strlen(msg));
    }
    void logProgress(const SkString& str) {
        nativeLogProgress(str.c_str());
        fileWrite(str.c_str(), str.size());
    }

private:
#ifdef SK_BUILD_FOR_ANDROID
    void nativeLogError(const char msg[]) { SkDebugf("%s", msg); }
    void nativeLogProgress(const char msg[]) { SkDebugf("%s", msg); }
#else
    void nativeLogError(const char msg[]) { fprintf(stderr, "%s", msg); }
    void nativeLogProgress(const char msg[]) { printf("%s", msg); }
#endif

    void fileWrite(const char msg[], size_t size) {
        if (fFileStream && fFileStream->isValid())
            fFileStream->write(msg, size);
    }
    SkFILEWStream* fFileStream;

} logger;

///////////////////////////////////////////////////////////////////////////////

enum benchModes {
    kNormal_benchModes,
    kDeferred_benchModes,
    kRecord_benchModes,
    kPictureRecord_benchModes
};

///////////////////////////////////////////////////////////////////////////////

static void erase(SkBitmap& bm) {
    if (bm.config() == SkBitmap::kA8_Config) {
        bm.eraseColor(0);
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
    kRaster_Backend,
    kGPU_Backend,
    kPDF_Backend,
};

#if SK_SUPPORT_GPU
class GLHelper {
public:
    GLHelper() {
    }

    bool init(SkGLContext* glCtx, int width, int height) {
        GrContext* grCtx;
        if (!glCtx->init(width, height)) {
            return false;
        }
        GrPlatform3DContext ctx =
            reinterpret_cast<GrPlatform3DContext>(glCtx->gl());
        grCtx = GrContext::Create(kOpenGL_Shaders_GrEngine, ctx);
        if (NULL != grCtx) {
            GrPlatformRenderTargetDesc desc;
            desc.fConfig = kSkia8888_PM_GrPixelConfig;
            desc.fWidth = width;
            desc.fHeight = height;
            desc.fStencilBits = 8;
            desc.fRenderTargetHandle = glCtx->getFBOID();
            GrRenderTarget* rt = grCtx->createPlatformRenderTarget(desc);
            if (NULL == rt) {
                grCtx->unref();
                return false;
            }
            glCtx->ref();
            fGLContext.reset(glCtx);
            fGrContext.reset(grCtx);
            fRenderTarget.reset(rt);
        }
        return true;
    }

    void cleanup() {
        fGLContext.reset(NULL);
        fGrContext.reset(NULL);
        fRenderTarget.reset(NULL);
    }

    bool isValid() {
        return NULL != fGLContext.get();
    }

    SkGLContext* glContext() {
        return fGLContext.get();
    }

    GrRenderTarget* renderTarget() {
        return fRenderTarget.get();
    }

    GrContext* grContext() {
        return fGrContext.get();
    }
private:
    SkAutoTUnref<SkGLContext> fGLContext;
    SkAutoTUnref<GrContext> fGrContext;
    SkAutoTUnref<GrRenderTarget> fRenderTarget;
};

static GLHelper gRealGLHelper;
static GLHelper gNullGLHelper;
static GLHelper gDebugGLHelper;
#if SK_ANGLE
static GLHelper gANGLEGLHelper;
#endif // SK_ANGLE
#else  // !SK_SUPPORT_GPU
class GLHelper;
class SkGLContext;
#endif // !SK_SUPPORT_GPU
static SkDevice* make_device(SkBitmap::Config config, const SkIPoint& size,
                             Backend backend, GLHelper* glHelper) {
    SkDevice* device = NULL;
    SkBitmap bitmap;
    bitmap.setConfig(config, size.fX, size.fY);

    switch (backend) {
        case kRaster_Backend:
            bitmap.allocPixels();
            erase(bitmap);
            device = new SkDevice(bitmap);
            break;
#if SK_SUPPORT_GPU
        case kGPU_Backend:
            device = new SkGpuDevice(glHelper->grContext(),
                                     glHelper->renderTarget());
            break;
#endif
        case kPDF_Backend:
        default:
            SkASSERT(!"unsupported");
    }
    return device;
}

static const struct {
    SkBitmap::Config    fConfig;
    const char*         fName;
    Backend             fBackend;
    GLHelper*           fGLHelper;
} gConfigs[] = {
    { SkBitmap::kARGB_8888_Config,  "8888",     kRaster_Backend, NULL },
    { SkBitmap::kRGB_565_Config,    "565",      kRaster_Backend, NULL },
#if SK_SUPPORT_GPU
    { SkBitmap::kARGB_8888_Config,  "GPU",      kGPU_Backend, &gRealGLHelper },
#if SK_ANGLE
    { SkBitmap::kARGB_8888_Config,  "ANGLE",    kGPU_Backend, &gANGLEGLHelper },
#endif // SK_ANGLE
#ifdef SK_DEBUG
    { SkBitmap::kARGB_8888_Config,  "Debug",    kGPU_Backend, &gDebugGLHelper },
#endif // SK_DEBUG
    { SkBitmap::kARGB_8888_Config,  "NULLGPU",  kGPU_Backend, &gNullGLHelper },
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

static void determine_gpu_context_size(SkTDict<const char*>& defineDict,
                                       int* contextWidth,
                                       int* contextHeight) {
    Iter iter(&defineDict);
    SkBenchmark* bench;
    while ((bench = iter.next()) != NULL) {
        SkIPoint dim = bench->getSize();
        if (*contextWidth < dim.fX) {
            *contextWidth = dim.fX;
        }
        if (*contextHeight < dim.fY) {
            *contextHeight = dim.fY;
        }
        bench->unref();
    }
}

static bool skip_name(const SkTDArray<const char*> array, const char name[]) {
    if (0 == array.count()) {
        // no names, so don't skip anything
        return false;
    }
    for (int i = 0; i < array.count(); ++i) {
        if (strstr(name, array[i])) {
            // found the name, so don't skip
            return false;
        }
    }
    return true;
}

static void help() {
    SkDebugf("Usage: bench [-o outDir] [-repeat nr] [-logPerIter 1|0] "
                          "[-timers [wcg]*] [-rotate]\n"
             "    [-scale] [-clip] [-min] [-forceAA 1|0] [-forceFilter 1|0]\n"
             "    [-forceDither 1|0] [-forceBlend 1|0] [-strokeWidth width]\n"
             "    [-match name] [-mode normal|deferred|record|picturerecord]\n"
             "    [-config 8888|565|GPU|ANGLE|NULLGPU] [-Dfoo bar]\n"
             "    [-h|--help]");
    SkDebugf("\n\n");
    SkDebugf("    -o outDir : Image of each bench will be put in outDir.\n");
    SkDebugf("    -repeat nr : Each bench repeats for nr times.\n");
    SkDebugf("    -logPerIter 1|0 : "
             "Log each repeat timer instead of mean, default is disabled.\n");
    SkDebugf("    -timers [wcg]* : "
             "Display wall time, cpu time or gpu time for each bench.\n");
    SkDebugf("    -rotate : Rotate before each bench runs.\n");
    SkDebugf("    -scale : Scale before each bench runs.\n");
    SkDebugf("    -clip : Clip before each bench runs.\n");
    SkDebugf("    -min : Print the minimum times (instead of average).\n");
    SkDebugf("    -forceAA 1|0 : "
             "Enable/disable anti-aliased, default is enabled.\n");
    SkDebugf("    -forceFilter 1|0 : "
             "Enable/disable bitmap filtering, default is disabled.\n");
    SkDebugf("    -forceDither 1|0 : "
             "Enable/disable dithering, default is disabled.\n");
    SkDebugf("    -forceBlend 1|0 : "
             "Enable/disable dithering, default is disabled.\n");
    SkDebugf("    -strokeWidth width : The width for path stroke.\n");
    SkDebugf("    -match name : Only run bench whose name is matched.\n");
    SkDebugf("    -mode normal|deferred|record|picturerecord : Run in the corresponding mode\n"
             "                 normal, Use a normal canvas to draw to;\n"
             "                 deferred, Use a deferrred canvas when drawing;\n"
             "                 record, Benchmark the time to record to an SkPicture;\n"
             "                 picturerecord, Benchmark the time to do record from a \n"
             "                                SkPicture to a SkPicture.\n");
    SkDebugf("    -logFile : destination for writing log output, in addition to stdout.\n");
#if SK_SUPPORT_GPU
    SkDebugf("    -config 8888|565|GPU|ANGLE|NULLGPU : "
             "Run bench in corresponding config mode.\n");
#else
    SkDebugf("    -config 8888|565: "
             "Run bench in corresponding config mode.\n");
#endif
    SkDebugf("    -Dfoo bar : Add extra definition to bench.\n");
    SkDebugf("    -h|--help : Show this help message.\n");
}

int main (int argc, char * const argv[]) {
#ifdef SK_ENABLE_INST_COUNT
    gPrintInstCount = true;
#endif
    SkAutoGraphics ag;

    SkTDict<const char*> defineDict(1024);
    int repeatDraw = 1;
    bool logPerIter = false;
    int forceAlpha = 0xFF;
    bool forceAA = true;
    bool forceFilter = false;
    SkTriState::State forceDither = SkTriState::kDefault;
    bool timerWall = false;
    bool timerCpu = true;
    bool timerGpu = true;
    bool doScale = false;
    bool doRotate = false;
    bool doClip = false;
    bool printMin = false;
    bool hasStrokeWidth = false;
    float strokeWidth;
    SkTDArray<const char*> fMatches;
    benchModes benchMode = kNormal_benchModes;
    SkString perIterTimeformat("%.2f");
    SkString normalTimeFormat("%6.2f");

    SkString outDir;
    SkBitmap::Config outConfig = SkBitmap::kNo_Config;
    GLHelper* glHelper = NULL;
    const char* configName = "";
    Backend backend = kRaster_Backend;  // for warning
    SkTDArray<int> configs;
    bool userConfig = false;

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
        } else if (strcmp(*argv, "-repeat") == 0) {
            argv++;
            if (argv < stop) {
                repeatDraw = atoi(*argv);
                if (repeatDraw < 1) {
                    repeatDraw = 1;
                }
            } else {
                logger.logError("missing arg for -repeat\n");
                help();
                return -1;
            }
        } else if (strcmp(*argv, "-logPerIter") == 0) {
            if (!parse_bool_arg(++argv, stop, &logPerIter)) {
                logger.logError("missing arg for -logPerIter\n");
                help();
                return -1;
            }
        } else if (strcmp(*argv, "-timers") == 0) {
            argv++;
            if (argv < stop) {
                timerWall = false;
                timerCpu = false;
                timerGpu = false;
                for (char* t = *argv; *t; ++t) {
                    switch (*t) {
                    case 'w': timerWall = true; break;
                    case 'c': timerCpu = true; break;
                    case 'g': timerGpu = true; break;
                    }
                }
            } else {
                logger.logError("missing arg for -timers\n");
                help();
                return -1;
            }
        } else if (!strcmp(*argv, "-rotate")) {
            doRotate = true;
        } else if (!strcmp(*argv, "-scale")) {
            doScale = true;
        } else if (!strcmp(*argv, "-clip")) {
            doClip = true;
        } else if (!strcmp(*argv, "-min")) {
            printMin = true;
        } else if (strcmp(*argv, "-forceAA") == 0) {
            if (!parse_bool_arg(++argv, stop, &forceAA)) {
                logger.logError("missing arg for -forceAA\n");
                help();
                return -1;
            }
        } else if (strcmp(*argv, "-forceFilter") == 0) {
            if (!parse_bool_arg(++argv, stop, &forceFilter)) {
                logger.logError("missing arg for -forceFilter\n");
                help();
                return -1;
            }
        } else if (strcmp(*argv, "-forceDither") == 0) {
            bool tmp;
            if (!parse_bool_arg(++argv, stop, &tmp)) {
                logger.logError("missing arg for -forceDither\n");
                help();
                return -1;
            }
            forceDither = tmp ? SkTriState::kTrue : SkTriState::kFalse;
        } else if (strcmp(*argv, "-forceBlend") == 0) {
            bool wantAlpha = false;
            if (!parse_bool_arg(++argv, stop, &wantAlpha)) {
                logger.logError("missing arg for -forceBlend\n");
                help();
                return -1;
            }
            forceAlpha = wantAlpha ? 0x80 : 0xFF;
        } else if (strcmp(*argv, "-mode") == 0) {
            argv++;
            if (argv < stop) {
                if (strcmp(*argv, "normal") == 0) {
                    benchMode = kNormal_benchModes;
                } else if (strcmp(*argv, "deferred") == 0) {
                    benchMode = kDeferred_benchModes;
                } else if (strcmp(*argv, "record") == 0) {
                    benchMode = kRecord_benchModes;
                } else if (strcmp(*argv, "picturerecord") == 0) {
                    benchMode = kPictureRecord_benchModes;
                } else {
                    logger.logError("bad arg for -mode\n");
                    help();
                    return -1;
                }
            } else {
                logger.logError("missing arg for -mode\n");
                help();
                return -1;
            }
        } else if (strcmp(*argv, "-strokeWidth") == 0) {
            argv++;
            if (argv < stop) {
                const char *strokeWidthStr = *argv;
                if (sscanf(strokeWidthStr, "%f", &strokeWidth) != 1) {
                  logger.logError("bad arg for -strokeWidth\n");
                  help();
                  return -1;
                }
                hasStrokeWidth = true;
            } else {
                logger.logError("missing arg for -strokeWidth\n");
                help();
                return -1;
            }
        } else if (strcmp(*argv, "-match") == 0) {
            argv++;
            if (argv < stop) {
                *fMatches.append() = *argv;
            } else {
                logger.logError("missing arg for -match\n");
                help();
                return -1;
            }
        } else if (strcmp(*argv, "-config") == 0) {
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
                logger.logError("missing arg for -config\n");
                help();
                return -1;
            }
        } else if (strcmp(*argv, "-logFile") == 0) {
            argv++;
            if (argv < stop) {
                if (!logger.SetLogFile(*argv)) {
                    SkString str;
                    str.printf("Could not open %s for writing.", *argv);
                    return -1;
                }
            } else {
                logger.logError("missing arg for -logFile\n");
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
        logger.logError("'-mode record' and '-mode picturerecord' are not"
                  " compatible with -o.\n");
        return -1;
    }
    if ((benchMode == kRecord_benchModes || benchMode == kPictureRecord_benchModes)) {
        perIterTimeformat.set("%.4f");
        normalTimeFormat.set("%6.4f");
    }
    if (!userConfig) {
        // if no config is specified by user, we add them all.
        for (unsigned int i = 0; i < SK_ARRAY_COUNT(gConfigs); ++i) {
            *configs.append() = i;
        }
    }

    // report our current settings
    {
        SkString str;
        str.printf("skia bench: alpha=0x%02X antialias=%d filter=%d "
                   "deferred=%d logperiter=%d",
                   forceAlpha, forceAA, forceFilter, benchMode == kDeferred_benchModes,
                   logPerIter);
        str.appendf(" rotate=%d scale=%d clip=%d min=%d",
                   doRotate, doScale, doClip, printMin);
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

    SkGLContext* timerCtx = NULL;
    //Don't do GL when fixed.
#if !defined(SK_SCALAR_IS_FIXED) && SK_SUPPORT_GPU
    int contextWidth = 1024;
    int contextHeight = 1024;
    determine_gpu_context_size(defineDict, &contextWidth, &contextHeight);
    SkAutoTUnref<SkGLContext> realGLCtx(new SkNativeGLContext);
    SkAutoTUnref<SkGLContext> nullGLCtx(new SkNullGLContext);
    SkAutoTUnref<SkGLContext> debugGLCtx(new SkDebugGLContext);
    gRealGLHelper.init(realGLCtx.get(), contextWidth, contextHeight);
    gNullGLHelper.init(nullGLCtx.get(), contextWidth, contextHeight);
    gDebugGLHelper.init(debugGLCtx.get(), contextWidth, contextHeight);
#if SK_ANGLE
    SkAutoTUnref<SkGLContext> angleGLCtx(new SkANGLEGLContext);
    gANGLEGLHelper.init(angleGLCtx.get(), contextWidth, contextHeight);
#endif // SK_ANGLE
    timerCtx = gRealGLHelper.glContext();
#endif // !defined(SK_SCALAR_IS_FIXED) && SK_SUPPORT_GPU

    BenchTimer timer = BenchTimer(timerCtx);
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
        if (skip_name(fMatches, bench->getName())) {
            continue;
        }

        {
            SkString str;
            str.printf("running bench [%d %d] %28s", dim.fX, dim.fY,
                       bench->getName());
            logger.logProgress(str);
        }

        AutoPrePostDraw appd(bench);

        for (int x = 0; x < configs.count(); ++x) {
            int configIndex = configs[x];

            outConfig = gConfigs[configIndex].fConfig;
            configName = gConfigs[configIndex].fName;
            backend = gConfigs[configIndex].fBackend;
            glHelper = gConfigs[configIndex].fGLHelper;

#if SK_SUPPORT_GPU
            if (kGPU_Backend == backend &&
                (NULL == glHelper || !glHelper->isValid())) {
                continue;
            }
#endif
            SkDevice* device = make_device(outConfig, dim, backend, glHelper);
            SkCanvas* canvas = NULL;
            SkPicture pictureRecordFrom;
            SkPicture pictureRecordTo;
            switch(benchMode) {
                case kDeferred_benchModes:
                    canvas = new SkDeferredCanvas(device);
                    break;
                case kRecord_benchModes:
                    canvas = pictureRecordTo.beginRecording(dim.fX, dim.fY);
                    canvas->ref();
                    break;
                case kPictureRecord_benchModes: {
                    // This sets up picture-to-picture recording.
                    // The C++ drawing calls for the benchmark are recorded into
                    // pictureRecordFrom. As the benchmark, we will time how
                    // long it takes to playback pictureRecordFrom into
                    // pictureRecordTo.
                    SkCanvas* tempCanvas = pictureRecordFrom.beginRecording(dim.fX, dim.fY);
                    bench->draw(tempCanvas);
                    pictureRecordFrom.endRecording();
                    canvas = pictureRecordTo.beginRecording(dim.fX, dim.fY);
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
            SkAutoUnref canvasUnref(canvas);

            if (doClip) {
                performClip(canvas, dim.fX, dim.fY);
            }
            if (doScale) {
                performScale(canvas, dim.fX, dim.fY);
            }
            if (doRotate) {
                performRotate(canvas, dim.fX, dim.fY);
            }

            // warm up caches if needed
            if (repeatDraw > 1) {
                SkAutoCanvasRestore acr(canvas, true);
                if (benchMode == kPictureRecord_benchModes) {
                    pictureRecordFrom.draw(canvas);
                } else {
                    bench->draw(canvas);
                }
                canvas->flush();
#if SK_SUPPORT_GPU
                if (glHelper) {
                    glHelper->grContext()->flush();
                    SK_GL(*glHelper->glContext(), Finish());
                }
#endif
            }

            // record timer values for each repeat, and their sum
            SkString fWallStr(" msecs = ");
            SkString fCpuStr(" cmsecs = ");
            SkString fGpuStr(" gmsecs = ");
            double fWallSum = 0.0, fWallMin;
            double fCpuSum = 0.0, fCpuMin;
            double fGpuSum = 0.0, fGpuMin;
            for (int i = 0; i < repeatDraw; i++) {
                if ((benchMode == kRecord_benchModes
                     || benchMode == kPictureRecord_benchModes)) {
                    // This will clear the recorded commands so that they do not
                    // acculmulate.
                    canvas = pictureRecordTo.beginRecording(dim.fX, dim.fY);
                }

                timer.start();
                SkAutoCanvasRestore acr(canvas, true);
                if (benchMode == kPictureRecord_benchModes) {
                    pictureRecordFrom.draw(canvas);
                } else {
                    bench->draw(canvas);
                }
                canvas->flush();
#if SK_SUPPORT_GPU
                if (glHelper) {
                    glHelper->grContext()->flush();
                }
#endif
                timer.end();

                if (i == repeatDraw - 1) {
                    // no comma after the last value
                    fWallStr.appendf(perIterTimeformat.c_str(), timer.fWall);
                    fCpuStr.appendf(perIterTimeformat.c_str(), timer.fCpu);
                    fGpuStr.appendf(perIterTimeformat.c_str(), timer.fGpu);
                } else {
                    fWallStr.appendf(perIterTimeformat.c_str(), timer.fWall);
                    fWallStr.appendf(",");
                    fCpuStr.appendf(perIterTimeformat.c_str(), timer.fCpu);
                    fCpuStr.appendf(",");
                    fGpuStr.appendf(perIterTimeformat.c_str(), timer.fGpu);
                    fGpuStr.appendf(",");
                }

                if (0 == i) {
                    fWallMin = timer.fWall;
                    fCpuMin  = timer.fCpu;
                    fGpuMin  = timer.fGpu;
                } else {
                    fWallMin = Min(fWallMin, timer.fWall);
                    fCpuMin  = Min(fCpuMin,  timer.fCpu);
                    fGpuMin  = Min(fGpuMin,  timer.fGpu);
                }

                fWallSum += timer.fWall;
                fCpuSum += timer.fCpu;
                fGpuSum += timer.fGpu;
            }
#if SK_SUPPORT_GPU
           if (glHelper) {
                SK_GL(*glHelper->glContext(), Finish());
           }
#endif
            if (repeatDraw > 1) {
                // output each repeat (no average) if logPerIter is set,
                // otherwise output only the average
                if (!logPerIter) {
                    fWallStr.set(" msecs = ");
                    fWallStr.appendf(normalTimeFormat.c_str(), 
                        printMin ? fWallMin : fWallSum / repeatDraw);
                    fCpuStr.set(" cmsecs = ");
                    fCpuStr.appendf(normalTimeFormat.c_str(), 
                        printMin ? fCpuMin : fCpuSum / repeatDraw);
                    fGpuStr.set(" gmsecs = ");
                    fGpuStr.appendf(normalTimeFormat.c_str(), 
                        printMin ? fGpuMin : fGpuSum / repeatDraw);
                }
                SkString str;
                str.printf("  %4s:", configName);
                if (timerWall) {
                    str += fWallStr;
                }
                if (timerCpu) {
                    str += fCpuStr;
                }
                if (timerGpu && glHelper && fGpuSum > 0) {
                    str += fGpuStr;
                }
                logger.logProgress(str);
            }
            if (outDir.size() > 0) {
                saveFile(bench->getName(), configName, outDir.c_str(),
                         device->accessBitmap(false));
            }
        }
        logger.logProgress(SkString("\n"));
    }
#if SK_SUPPORT_GPU
    // need to clean up here rather than post-main to allow leak detection to work
    gDebugGLHelper.cleanup();
#endif
    return 0;
}
