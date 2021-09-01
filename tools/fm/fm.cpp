// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "gm/gm.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/docs/SkPDFDocument.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/SkTHash.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkMD5.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkTaskGroup.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/utils/SkOSPath.h"
#include "tests/Test.h"
#include "tools/AutoreleasePool.h"
#include "tools/CrashHandler.h"
#include "tools/HashAndEncode.h"
#include "tools/ToolUtils.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/flags/CommonFlags.h"
#include "tools/gpu/BackendSurfaceFactory.h"
#include "tools/gpu/GrContextFactory.h"
#include "tools/gpu/MemoryCache.h"
#include "tools/trace/EventTracingPriv.h"

#include <chrono>
#include <functional>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(SK_ENABLE_SVG)
#include "modules/svg/include/SkSVGDOM.h"
#include "modules/svg/include/SkSVGNode.h"
#endif

#if defined(SK_ENABLE_SKOTTIE)
    #include "modules/skottie/include/Skottie.h"
    #include "modules/skresources/include/SkResources.h"
#endif

using sk_gpu_test::GrContextFactory;

static DEFINE_bool(listGMs  , false, "Print GM names and exit.");
static DEFINE_bool(listTests, false, "Print unit test names and exit.");

static DEFINE_string2(sources, s, "", "Which GMs, .skps, or images to draw.");
static DEFINE_string2(backend, b, "", "Backend used to create a canvas to draw into.");

static DEFINE_string(ct    ,   "8888", "The color type for any raster backend.");
static DEFINE_string(at    , "premul", "The alpha type for any raster backend.");
static DEFINE_string(gamut ,   "srgb", "The color gamut for any raster backend.");
static DEFINE_string(tf    ,   "srgb", "The transfer function for any raster backend.");
static DEFINE_bool  (legacy,    false, "Use a null SkColorSpace instead of --gamut and --tf?");

static DEFINE_bool  (skvm ,    false, "Use SkVMBlitter when supported?");
static DEFINE_bool  (jit  ,     true, "JIT SkVM?");
static DEFINE_bool  (dylib,    false, "JIT SkVM via dylib?");

static DEFINE_bool  (reducedshaders,    false, "Use reduced shader set for any GPU backend.");
static DEFINE_int   (samples       ,         0, "Samples per pixel in GPU backends.");
static DEFINE_bool  (stencils      ,      true, "If false, avoid stencil buffers in GPU backends.");
static DEFINE_bool  (dit           ,     false, "Use device-independent text in GPU backends.");
static DEFINE_string(surf          , "default", "Backing store for GPU backend surfaces.");

static DEFINE_bool(       preAbandonGpuContext, false, "Abandon the GrContext before drawing.");
static DEFINE_bool(          abandonGpuContext, false, "Abandon the GrContext after drawing.");
static DEFINE_bool(releaseAndAbandonGpuContext, false,
                   "Release all GPU resources and abandon the GrContext after drawing.");

static DEFINE_bool(decodeToDst, false,
                   "Decode images to destination format rather than suggested natural format.");

static DEFINE_double(rasterDPI, SK_ScalarDefaultRasterDPI,
                     "DPI for rasterized content in vector backends like --backend pdf.");
static DEFINE_bool(PDFA, false, "Create PDF/A with --backend pdf?");

static DEFINE_int(clipW, INT_MAX, "Limit source width.");
static DEFINE_int(clipH, INT_MAX, "Limit source height.");

static DEFINE_bool   (cpuDetect, true, "Detect CPU features for runtime optimizations?");
static DEFINE_string2(writePath, w, "", "Write .pngs to this directory if set.");
static DEFINE_bool   (quick, false, "Skip image hashing and encoding?");
static DEFINE_int    (race, 0, "If >0, use threads to induce race conditions?");

static DEFINE_string(writeShaders, "", "Write GLSL shaders to this directory if set.");

static DEFINE_string(key,        "", "Metadata passed through to .png encoder and .json output.");
static DEFINE_string(properties, "", "Metadata passed through to .png encoder and .json output.");

template <typename T>
struct FlagOption {
    const char* label;
    T           value;
};

template <typename T, int N>
static bool parse_flag(const CommandLineFlags::StringArray& flag,
                       const char* flag_name,
                       const FlagOption<T> (&array)[N],
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

struct Result {
    enum { Ok, Skip, Fail} status;
    SkString               failure;
};
static const Result ok = {Result::Ok,   {}},
                  skip = {Result::Skip, {}};

static Result fail(const char* why) {
    return { Result::Fail, SkString(why) };
}
template <typename... Args>
static Result fail(const char* whyFmt, Args... args) {
    return { Result::Fail, SkStringPrintf(whyFmt, args...) };
}


struct Source {
    SkString                               name;
    SkISize                                size;
    std::function<Result(SkCanvas*)>       draw;
    std::function<void(GrContextOptions*)> tweak = [](GrContextOptions*){};
};

static void init(Source* source, std::shared_ptr<skiagm::GM> gm) {
    source->size  = gm->getISize();
    source->tweak = [gm](GrContextOptions* options) { gm->modifyGrContextOptions(options); };
    source->draw  = [gm](SkCanvas* canvas) {
        auto direct = GrAsDirectContext(canvas->recordingContext());

        SkString err;
        switch (gm->gpuSetup(direct, canvas, &err)) {
            case skiagm::DrawResult::kOk  : break;
            case skiagm::DrawResult::kSkip: return skip;
            case skiagm::DrawResult::kFail: return fail(err.c_str());
        }

        switch (gm->draw(canvas, &err)) {
            case skiagm::DrawResult::kOk:   break;
            case skiagm::DrawResult::kSkip: return skip;
            case skiagm::DrawResult::kFail: return fail(err.c_str());
        }
        return ok;
    };
}

static void init(Source* source, sk_sp<SkPicture> pic) {
    source->size = pic->cullRect().roundOut().size();
    source->draw = [pic](SkCanvas* canvas) {
        canvas->drawPicture(pic);
        return ok;
    };
}

static void init(Source* source, std::shared_ptr<SkCodec> codec) {
    source->size = codec->dimensions();
    source->draw = [codec](SkCanvas* canvas) {
        SkImageInfo info = codec->getInfo();
        if (FLAGS_decodeToDst) {
            info = canvas->imageInfo().makeDimensions(info.dimensions());
        }

        auto [image, result] = codec->getImage(info);
        if (image) {
            canvas->drawImage(image, 0,0);
            return ok;
        }
        return fail("codec->getPixels() failed: %d\n", result);
    };
}

#if defined(SK_ENABLE_SVG)
static void init(Source* source, sk_sp<SkSVGDOM> svg) {
    if (svg->containerSize().isEmpty()) {
        svg->setContainerSize({1000,1000});
    }
    source->size = svg->containerSize().toCeil();
    source->draw = [svg](SkCanvas* canvas) {
        svg->render(canvas);
        return ok;
    };
}
#endif

#if defined(SK_ENABLE_SKOTTIE)
static void init(Source* source, sk_sp<skottie::Animation> animation) {
    source->size = {1000,1000};
    source->draw = [animation](SkCanvas* canvas) {
        canvas->clear(SK_ColorWHITE);

        // Draw frames in a shuffled order to exercise nonlinear frame progression.
        // The film strip will still be in time order, just drawn out of order.
        const int order[] = { 4, 0, 3, 1, 2 };
        const int tiles = SK_ARRAY_COUNT(order);
        const float dim = 1000.0f / tiles;

        const float dt = 1.0f / (tiles*tiles - 1);

        for (int y : order)
        for (int x : order) {
            SkRect dst = {x*dim, y*dim, (x+1)*dim, (y+1)*dim};

            SkAutoCanvasRestore _(canvas, /*doSave=*/true);
            canvas->clipRect(dst, /*doAntiAlias=*/true);
            canvas->concat(SkMatrix::RectToRect(SkRect::MakeSize(animation->size()), dst,
                                                SkMatrix::kCenter_ScaleToFit));
            float t = (y*tiles + x) * dt;
            animation->seek(t);
            animation->render(canvas);
        }
        return ok;
    };
}
#endif

static void init(Source* source, const skiatest::Test& test) {
    source->size  = {1,1};
    source->draw  = [test](SkCanvas* canvas) {
        struct Reporter : public skiatest::Reporter {
            SkString msg;

            void reportFailed(const skiatest::Failure& failure) override {
                msg += failure.toString();
                msg += "\n";
            }
        } reporter;

        test.run(&reporter, GrContextOptions{});

        if (reporter.msg.isEmpty()) {
            canvas->clear(SK_ColorGREEN);
            return ok;
        }

        canvas->clear(SK_ColorRED);
        return fail(reporter.msg.c_str());
    };
}

static sk_sp<SkImage> draw_with_cpu(std::function<bool(SkCanvas*)> draw,
                                    SkImageInfo info) {
    if (sk_sp<SkSurface> surface = SkSurface::MakeRaster(info)) {
        if (draw(surface->getCanvas())) {
            return surface->makeImageSnapshot();
        }
    }
    return nullptr;
}

static sk_sp<SkData> draw_as_skp(std::function<bool(SkCanvas*)> draw,
                                 SkImageInfo info) {
    SkPictureRecorder recorder;
    if (draw(recorder.beginRecording(info.width(), info.height()))) {
        return recorder.finishRecordingAsPicture()->serialize();
    }
    return nullptr;
}

static sk_sp<SkData> draw_as_pdf(std::function<bool(SkCanvas*)> draw,
                                 SkImageInfo info,
                                 SkString name) {
    SkPDF::Metadata metadata;
    metadata.fTitle     = name;
    metadata.fCreator   = "Skia/FM";
    metadata.fRasterDPI = FLAGS_rasterDPI;
    metadata.fPDFA      = FLAGS_PDFA;

    SkDynamicMemoryWStream stream;
    if (sk_sp<SkDocument> doc = SkPDF::MakeDocument(&stream, metadata)) {
        if (draw(doc->beginPage(info.width(), info.height()))) {
            doc->endPage();
            doc->close();
            return stream.detachAsData();
        }
    }
    return nullptr;
}

static sk_sp<SkImage> draw_with_gpu(std::function<bool(SkCanvas*)> draw,
                                    SkImageInfo info,
                                    GrContextFactory::ContextType api,
                                    GrContextFactory* factory) {
    enum class SurfaceType { kDefault, kBackendTexture, kBackendRenderTarget };
    const FlagOption<SurfaceType> kSurfaceTypes[] = {
        { "default", SurfaceType::kDefault },
        { "betex"  , SurfaceType::kBackendTexture },
        { "bert"   , SurfaceType::kBackendRenderTarget },
    };
    SurfaceType surfaceType;
    if (!parse_flag(FLAGS_surf, "surf", kSurfaceTypes, &surfaceType)) {
        return nullptr;
    }

    auto overrides = GrContextFactory::ContextOverrides::kNone;
    if (!FLAGS_stencils) { overrides |= GrContextFactory::ContextOverrides::kAvoidStencilBuffers; }

    auto context = factory->getContextInfo(api, overrides).directContext();

    uint32_t flags = FLAGS_dit ? SkSurfaceProps::kUseDeviceIndependentFonts_Flag
                               : 0;
    SkSurfaceProps props(flags, kRGB_H_SkPixelGeometry);

    sk_sp<SkSurface> surface;

    switch (surfaceType) {
        case SurfaceType::kDefault:
            surface = SkSurface::MakeRenderTarget(context,
                                                  SkBudgeted::kNo,
                                                  info,
                                                  FLAGS_samples,
                                                  &props);
            break;

        case SurfaceType::kBackendTexture:
            surface = sk_gpu_test::MakeBackendTextureSurface(context,
                                                             info,
                                                             kTopLeft_GrSurfaceOrigin,
                                                             FLAGS_samples,
                                                             GrMipmapped::kNo,
                                                             GrProtected::kNo,
                                                             &props);
            break;

        case SurfaceType::kBackendRenderTarget:
            surface = sk_gpu_test::MakeBackendRenderTargetSurface(context,
                                                                  info,
                                                                  kBottomLeft_GrSurfaceOrigin,
                                                                  FLAGS_samples,
                                                                  GrProtected::kNo,
                                                                  &props);
            break;
    }

    if (!surface) {
        fprintf(stderr, "Could not create GPU surface.\n");
        return nullptr;
    }

    if (FLAGS_preAbandonGpuContext) {
        factory->abandonContexts();
    }

    sk_sp<SkImage> image;
    if (draw(surface->getCanvas())) {
        image = surface->makeImageSnapshot();
    }

    if (FLAGS_abandonGpuContext) {
        factory->abandonContexts();
    } else if (FLAGS_releaseAndAbandonGpuContext) {
        factory->releaseResourcesAndAbandonContexts();
    }

    return image;
}

extern bool gUseSkVMBlitter;
extern bool gSkVMAllowJIT;
extern bool gSkVMJITViaDylib;

int main(int argc, char** argv) {
    CommandLineFlags::Parse(argc, argv);
    SetupCrashHandler();
    SkTaskGroup::Enabler enabled(FLAGS_race);

    if (FLAGS_cpuDetect) {
        SkGraphics::Init();
    }
    gUseSkVMBlitter  = FLAGS_skvm;
    gSkVMAllowJIT    = FLAGS_jit;
    gSkVMJITViaDylib = FLAGS_dylib;

    initializeEventTracingForTools();
    ToolUtils::SetDefaultFontMgr();
    SetAnalyticAAFromCommonFlags();

    GrContextOptions baseOptions;
    SetCtxOptionsFromCommonFlags(&baseOptions);
    baseOptions.fReducedShaderVariations = FLAGS_reducedshaders;

    sk_gpu_test::MemoryCache memoryCache;
    if (!FLAGS_writeShaders.isEmpty()) {
        baseOptions.fPersistentCache = &memoryCache;
        baseOptions.fShaderCacheStrategy = GrContextOptions::ShaderCacheStrategy::kBackendSource;
    }

    SkTHashMap<SkString, skiagm::GMFactory> gm_factories;
    for (skiagm::GMFactory factory : skiagm::GMRegistry::Range()) {
        std::unique_ptr<skiagm::GM> gm{factory()};
        if (FLAGS_listGMs) {
            fprintf(stdout, "%s\n", gm->getName());
        } else {
            gm_factories.set(SkString{gm->getName()}, factory);
        }
    }

    SkTHashMap<SkString, const skiatest::Test*> tests;
    for (const skiatest::Test& test : skiatest::TestRegistry::Range()) {
        if (test.needsGpu) {
            continue;  // TODO
        }
        if (FLAGS_listTests) {
            fprintf(stdout, "%s\n", test.name);
        } else {
            tests.set(SkString{test.name}, &test);
        }
    }

    if (FLAGS_listGMs || FLAGS_listTests) {
        return 0;
    }
    if (FLAGS_sources.isEmpty()) {
        fprintf(stderr, "Please give me something to run using -s/--sources!\n");
        return 1;
    }

    const int replicas = std::max(1, FLAGS_race);

    SkTArray<Source> sources;
    for (const SkString& name : FLAGS_sources)
    for (int replica = 0; replica < replicas; replica++) {
        Source* source = &sources.push_back();
        source->name = name;

        if (skiagm::GMFactory* factory = gm_factories.find(name)) {
            std::shared_ptr<skiagm::GM> gm{(*factory)()};
            init(source, std::move(gm));
            continue;
        }

        if (const skiatest::Test** test = tests.find(name)) {
            init(source, **test);
            continue;
        }

        if (sk_sp<SkData> blob = SkData::MakeFromFileName(name.c_str())) {
            if (name.endsWith(".skp")) {
                if (sk_sp<SkPicture> pic = SkPicture::MakeFromData(blob.get())) {
                    init(source, pic);
                    continue;
                }
            }
#if defined(SK_ENABLE_SVG)
            else if (name.endsWith(".svg")) {
                SkMemoryStream stream{blob};
                if (sk_sp<SkSVGDOM> svg = SkSVGDOM::MakeFromStream(stream)) {
                    init(source, svg);
                    continue;
                }
            }
#endif
#if defined(SK_ENABLE_SKOTTIE)
            else if (name.endsWith(".json")) {
                const SkString dir  = SkOSPath::Dirname(name.c_str());
                if (sk_sp<skottie::Animation> animation = skottie::Animation::Builder()
                        .setResourceProvider(skresources::FileResourceProvider::Make(dir))
                        .make((const char*)blob->data(), blob->size())) {
                    init(source, animation);
                    continue;
                }
            }
#endif
            else if (std::shared_ptr<SkCodec> codec = SkCodec::MakeFromData(blob)) {
                init(source, codec);
                continue;
            }
        }

        fprintf(stderr, "Don't understand source '%s'... bailing out.\n", name.c_str());
        return 1;
    }

    enum NonGpuBackends {
        kCPU_Backend = -1,
        kSKP_Backend = -2,
        kPDF_Backend = -3,
    };
    const FlagOption<int> kBackends[] = {
        { "cpu"            , kCPU_Backend },
        { "skp"            , kSKP_Backend },
        { "pdf"            , kPDF_Backend },
        { "gl"             , GrContextFactory::kGL_ContextType },
        { "gles"           , GrContextFactory::kGLES_ContextType },
        { "angle_d3d9_es2" , GrContextFactory::kANGLE_D3D9_ES2_ContextType },
        { "angle_d3d11_es2", GrContextFactory::kANGLE_D3D11_ES2_ContextType },
        { "angle_d3d11_es3", GrContextFactory::kANGLE_D3D11_ES3_ContextType },
        { "angle_gl_es2"   , GrContextFactory::kANGLE_GL_ES2_ContextType },
        { "angle_gl_es3"   , GrContextFactory::kANGLE_GL_ES3_ContextType },
        { "cmdbuffer_es2"  , GrContextFactory::kCommandBuffer_ES2_ContextType },
        { "cmdbuffer_es3"  , GrContextFactory::kCommandBuffer_ES3_ContextType },
        { "vk"             , GrContextFactory::kVulkan_ContextType },
        { "mtl"            , GrContextFactory::kMetal_ContextType },
        { "mock"           , GrContextFactory::kMock_ContextType },
    };
    const FlagOption<SkColorType> kColorTypes[] = {
        { "a8",                  kAlpha_8_SkColorType },
        { "g8",                   kGray_8_SkColorType },
        { "565",                 kRGB_565_SkColorType },
        { "4444",              kARGB_4444_SkColorType },
        { "8888",                    kN32_SkColorType },
        { "888x",               kRGB_888x_SkColorType },
        { "1010102",        kRGBA_1010102_SkColorType },
        { "101010x",         kRGB_101010x_SkColorType },
        { "bgra1010102",    kBGRA_1010102_SkColorType },
        { "bgr101010x",      kBGR_101010x_SkColorType },
        { "f16norm",        kRGBA_F16Norm_SkColorType },
        { "f16",                kRGBA_F16_SkColorType },
        { "f32",                kRGBA_F32_SkColorType },
        { "rgba",              kRGBA_8888_SkColorType },
        { "bgra",              kBGRA_8888_SkColorType },
        { "srgba",            kSRGBA_8888_SkColorType },
        { "16161616", kR16G16B16A16_unorm_SkColorType },
    };
    const FlagOption<SkAlphaType> kAlphaTypes[] = {
        {   "premul",   kPremul_SkAlphaType },
        { "unpremul", kUnpremul_SkAlphaType },
    };
    const FlagOption<skcms_Matrix3x3> kGamuts[] = {
        { "srgb",    SkNamedGamut::kSRGB },
        { "p3",      SkNamedGamut::kDisplayP3 },
        { "rec2020", SkNamedGamut::kRec2020 },
        { "adobe",   SkNamedGamut::kAdobeRGB },
        { "narrow",  gNarrow_toXYZD50},
    };
    const FlagOption<skcms_TransferFunction> kTransferFunctions[] = {
        { "srgb"   , SkNamedTransferFn::kSRGB },
        { "rec2020", SkNamedTransferFn::kRec2020 },
        { "2.2"    , SkNamedTransferFn::k2Dot2 },
        { "linear" , SkNamedTransferFn::kLinear },
    };


    int                      backend;
    SkColorType              ct;
    SkAlphaType              at;
    skcms_Matrix3x3          gamut;
    skcms_TransferFunction   tf;

    if (!parse_flag(FLAGS_backend, "backend", kBackends         , &backend) ||
        !parse_flag(FLAGS_ct     , "ct"     , kColorTypes       , &ct)      ||
        !parse_flag(FLAGS_at     , "at"     , kAlphaTypes       , &at)      ||
        !parse_flag(FLAGS_gamut  , "gamut"  , kGamuts           , &gamut)   ||
        !parse_flag(FLAGS_tf     , "tf"     , kTransferFunctions, &tf)) {
        return 1;
    }

    sk_sp<SkColorSpace> cs = FLAGS_legacy ? nullptr
                                          : SkColorSpace::MakeRGB(tf,gamut);
    const SkColorInfo color_info{ct,at,cs};

    for (int i = 0; i < sources.count(); i += replicas)
    SkTaskGroup{}.batch(replicas, [=](int replica) {
        Source source = sources[i+replica];

        AutoreleasePool pool;
        const auto start = std::chrono::steady_clock::now();

        auto [w,h] = source.size;
        w = std::min(w, FLAGS_clipW);
        h = std::min(h, FLAGS_clipH);
        const SkImageInfo info = SkImageInfo::Make({w,h}, color_info);

        auto draw = [&source](SkCanvas* canvas) {
            Result result = source.draw(canvas);
            switch (result.status) {
                case Result::Ok:   break;
                case Result::Skip: return false;
                case Result::Fail:
                    SK_ABORT("%s", result.failure.c_str());
            }
            return true;
        };

        GrContextOptions options = baseOptions;
        source.tweak(&options);
        GrContextFactory factory(options);  // N.B. factory must outlive image

        sk_sp<SkImage> image;
        sk_sp<SkData>  blob;
        const char*    ext = ".png";
        switch (backend) {
            case kCPU_Backend:
                image = draw_with_cpu(draw, info);
                break;
            case kSKP_Backend:
                blob = draw_as_skp(draw, info);
                ext  = ".skp";
                break;
            case kPDF_Backend:
                blob = draw_as_pdf(draw, info, source.name);
                ext  = ".pdf";
                break;
            default:
                image = draw_with_gpu(draw, info, (GrContextFactory::ContextType)backend, &factory);
                break;
        }

        // We read back a bitmap even when --quick is set and we won't use it,
        // to keep us honest about deferred work, flushing pipelines, etc.
        SkBitmap bitmap;
        if (image && !image->asLegacyBitmap(&bitmap)) {
            SK_ABORT("SkImage::asLegacyBitmap() failed.");
        }

        // Our --race replicas have done their job by now if they're going to catch anything.
        if (replica != 0) {
            return;
        }

        if (!image && !blob) {
            fprintf(stdout, "%50s  skipped\n", source.name.c_str());
            fflush(stdout);
            return;
        }

        SkString md5;
        if (!FLAGS_quick) {
            HashAndEncode hashAndEncode{bitmap};
            {
                SkMD5 hash;
                if (image) {
                    hashAndEncode.feedHash(&hash);
                } else {
                    hash.write(blob->data(), blob->size());
                }

                SkMD5::Digest digest = hash.finish();
                for (int j = 0; j < 16; j++) {
                    md5.appendf("%02x", digest.data[j]);
                }
            }

            if (!FLAGS_writePath.isEmpty()) {
                SkString path = SkStringPrintf("%s/%s%s",
                                               FLAGS_writePath[0], source.name.c_str(), ext);
                for (char* it = path.writable_str(); *it != '\0'; it++) {
                    if (*it == '/' || *it == '\\') {
                        char prev = std::exchange(*it, '\0');
                        sk_mkdir(path.c_str());
                        *it = prev;
                    }
                }

                SkFILEWStream file(path.c_str());
                if (image) {
                    if (!hashAndEncode.encodePNG(&file, md5.c_str(),
                                                 FLAGS_key, FLAGS_properties)) {
                        SK_ABORT("Could not write .png.");
                    }
                } else {
                    file.write(blob->data(), blob->size());
                }
            }
        }

        const auto elapsed = std::chrono::steady_clock::now() - start;
        fprintf(stdout, "%50s  %s  %7dms\n",
                source.name.c_str(),
                md5.c_str(),
                (int)std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
        fflush(stdout);
    });


    if (!FLAGS_writeShaders.isEmpty()) {
        sk_mkdir(FLAGS_writeShaders[0]);
        GrBackendApi api =
                GrContextFactory::ContextTypeBackend((GrContextFactory::ContextType)backend);
        memoryCache.writeShadersToDisk(FLAGS_writeShaders[0], api);

    }

    return 0;
}
