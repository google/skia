// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "experimental/svg/model/SkSVGDOM.h"
#include "gm/gm.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/docs/SkPDFDocument.h"
#include "include/gpu/GrContextOptions.h"
#include "include/private/SkTHash.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkMD5.h"
#include "src/core/SkOSFile.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/utils/SkOSPath.h"
#include "tools/CrashHandler.h"
#include "tools/HashAndEncode.h"
#include "tools/ToolUtils.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/flags/CommonFlags.h"
#include "tools/gpu/GrContextFactory.h"
#include "tools/gpu/MemoryCache.h"
#include "tools/trace/EventTracingPriv.h"
#include <chrono>
#include <functional>
#include <stdio.h>
#include <stdlib.h>

#if defined(SK_ENABLE_SKOTTIE)
    #include "modules/skottie/include/Skottie.h"
    #include "modules/skottie/utils/SkottieUtils.h"
#endif

using sk_gpu_test::GrContextFactory;

static DEFINE_string2(sources, s, "", "Which GMs, .skps, or images to draw.");
static DEFINE_string2(backend, b, "", "Backend used to create a canvas to draw into.");

static DEFINE_string(ct    ,   "8888", "The color type for any raster backend.");
static DEFINE_string(at    , "premul", "The alpha type for any raster backend.");
static DEFINE_string(gamut ,   "srgb", "The color gamut for any raster backend.");
static DEFINE_string(tf    ,   "srgb", "The transfer function for any raster backend.");
static DEFINE_bool  (legacy,    false, "Use a null SkColorSpace instead of --gamut and --tf?");

static DEFINE_int   (samples ,         0, "Samples per pixel in GPU backends.");
static DEFINE_bool  (nvpr    ,     false, "Use NV_path_rendering in GPU backends?");
static DEFINE_bool  (stencils,      true, "If false, avoid stencil buffers in GPU backends.");
static DEFINE_bool  (dit     ,     false, "Use device-independent text in GPU backends.");
static DEFINE_string(surf    , "default", "Backing store for GPU backend surfaces.");

static DEFINE_bool(       preAbandonGpuContext, false, "Abandon the GrContext before drawing.");
static DEFINE_bool(          abandonGpuContext, false, "Abandon the GrContext after drawing.");
static DEFINE_bool(releaseAndAbandonGpuContext, false,
                   "Release all GPU resources and abandon the GrContext after drawing.");

static DEFINE_bool(decodeToDst, false,
                   "Decode images to destination format rather than suggested natural format.");

static DEFINE_double(rasterDPI, SK_ScalarDefaultRasterDPI,
                     "DPI for rasterized content in vector backends like --backend pdf.");
static DEFINE_bool(PDFA, false, "Create PDF/A with --backend pdf?");

static DEFINE_bool   (cpuDetect, true, "Detect CPU features for runtime optimizations?");
static DEFINE_string2(writePath, w, "", "Write .pngs to this directory if set.");

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

template <typename... Args>
static Result fail(const char* why, Args... args) {
    return { Result::Fail, SkStringPrintf(why, args...) };
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
        SkString err;
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
            info = canvas->imageInfo().makeWH(info.width(),
                                              info.height());
        }

        SkBitmap bm;
        bm.allocPixels(info);
        switch (SkCodec::Result result = codec->getPixels(info, bm.getPixels(), bm.rowBytes())) {
            case SkCodec::kSuccess:
            case SkCodec::kErrorInInput:
            case SkCodec::kIncompleteInput: canvas->drawBitmap(bm, 0,0);
                                            break;
            default: return fail("codec->getPixels() failed: %d\n", result);
        }
        return ok;
    };
}

static void init(Source* source, sk_sp<SkSVGDOM> svg) {
    source->size = svg->containerSize().isEmpty() ? SkISize{1000,1000}
                                                  : svg->containerSize().toCeil();
    source->draw = [svg](SkCanvas* canvas) {
        svg->render(canvas);
        return ok;
    };
}

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

            SkAutoCanvasRestore _(canvas, true/*save now*/);
            canvas->clipRect(dst, /*aa=*/true);
            canvas->concat(SkMatrix::MakeRectToRect(SkRect::MakeSize(animation->size()),
                                                    dst,
                                                    SkMatrix::kCenter_ScaleToFit));
            float t = (y*tiles + x) * dt;
            animation->seek(t);
            animation->render(canvas);
        }
        return ok;
    };
}
#endif

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

    auto overrides = FLAGS_nvpr ? GrContextFactory::ContextOverrides::kRequireNVPRSupport
                                : GrContextFactory::ContextOverrides::kDisableNVPR;
    if (!FLAGS_stencils) { overrides |= GrContextFactory::ContextOverrides::kAvoidStencilBuffers; }

    GrContext* context = factory->getContextInfo(api, overrides)
                                 .grContext();

    uint32_t flags = FLAGS_dit ? SkSurfaceProps::kUseDeviceIndependentFonts_Flag
                               : 0;
    SkSurfaceProps props(flags, SkSurfaceProps::kLegacyFontHost_InitType);

    sk_sp<SkSurface> surface;
    GrBackendTexture backendTexture;
    GrBackendRenderTarget backendRT;

    switch (surfaceType) {
        case SurfaceType::kDefault:
            surface = SkSurface::MakeRenderTarget(context,
                                                  SkBudgeted::kNo,
                                                  info,
                                                  FLAGS_samples,
                                                  &props);
            break;

        case SurfaceType::kBackendTexture:
            backendTexture = context->createBackendTexture(info.width(),
                                                           info.height(),
                                                           info.colorType(),
                                                           GrMipMapped::kNo,
                                                           GrRenderable::kYes);
            surface = SkSurface::MakeFromBackendTexture(context,
                                                        backendTexture,
                                                        kTopLeft_GrSurfaceOrigin,
                                                        FLAGS_samples,
                                                        info.colorType(),
                                                        info.refColorSpace(),
                                                        &props);
            break;

        case SurfaceType::kBackendRenderTarget:
            backendRT = context->priv().getGpu()
                ->createTestingOnlyBackendRenderTarget(info.width(),
                                                       info.height(),
                                                       SkColorTypeToGrColorType(info.colorType()));
            surface = SkSurface::MakeFromBackendRenderTarget(context,
                                                             backendRT,
                                                             kBottomLeft_GrSurfaceOrigin,
                                                             info.colorType(),
                                                             info.refColorSpace(),
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

    if (!context->abandoned()) {
        surface.reset();
        if (backendTexture.isValid()) {
            context->deleteBackendTexture(backendTexture);
        }
        if (backendRT.isValid()) {
            context->priv().getGpu()->deleteTestingOnlyBackendRenderTarget(backendRT);
        }
    }

    return image;
}

int main(int argc, char** argv) {
    CommandLineFlags::Parse(argc, argv);
    SetupCrashHandler();

    if (FLAGS_cpuDetect) {
        SkGraphics::Init();
    }
    initializeEventTracingForTools();
    ToolUtils::SetDefaultFontMgr();
    SetAnalyticAAFromCommonFlags();

    GrContextOptions baseOptions;
    SetCtxOptionsFromCommonFlags(&baseOptions);

    sk_gpu_test::MemoryCache memoryCache;
    if (!FLAGS_writeShaders.isEmpty()) {
        baseOptions.fPersistentCache = &memoryCache;
        baseOptions.fDisallowGLSLBinaryCaching = true;
    }

    SkTHashMap<SkString, skiagm::GMFactory> gm_factories;
    for (skiagm::GMFactory factory : skiagm::GMRegistry::Range()) {
        std::unique_ptr<skiagm::GM> gm{factory(nullptr)};
        if (FLAGS_sources.isEmpty()) {
            fprintf(stdout, "%s\n", gm->getName());
        } else {
            gm_factories.set(SkString{gm->getName()}, factory);
        }
    }
    if (FLAGS_sources.isEmpty()) {
        return 0;
    }

    SkTArray<Source> sources;
    for (const SkString& name : FLAGS_sources) {
        Source* source = &sources.push_back();

        if (skiagm::GMFactory* factory = gm_factories.find(name)) {
            std::shared_ptr<skiagm::GM> gm{(*factory)(nullptr)};
            source->name = name;
            init(source, gm);
            continue;
        }

        if (sk_sp<SkData> blob = SkData::MakeFromFileName(name.c_str())) {
            source->name = SkOSPath::Basename(name.c_str());

            if (name.endsWith(".skp")) {
                if (sk_sp<SkPicture> pic = SkPicture::MakeFromData(blob.get())) {
                    init(source, pic);
                    continue;
                }
            } else if (name.endsWith(".svg")) {
                SkMemoryStream stream{blob};
                if (sk_sp<SkSVGDOM> svg = SkSVGDOM::MakeFromStream(stream)) {
                    init(source, svg);
                    continue;
                }
            }
#if defined(SK_ENABLE_SKOTTIE)
            else if (name.endsWith(".json")) {
                const SkString dir  = SkOSPath::Dirname(name.c_str());
                if (sk_sp<skottie::Animation> animation = skottie::Animation::Builder()
                        .setResourceProvider(skottie_utils::FileResourceProvider::Make(dir))
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
        { "commandbuffer"  , GrContextFactory::kCommandBuffer_ContextType },
        { "vk"             , GrContextFactory::kVulkan_ContextType },
        { "mtl"            , GrContextFactory::kMetal_ContextType },
        { "mock"           , GrContextFactory::kMock_ContextType },
    };
    const FlagOption<SkColorType> kColorTypes[] = {
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
    const FlagOption<SkAlphaType> kAlphaTypes[] = {
        {   "premul",   kPremul_SkAlphaType },
        { "unpremul", kUnpremul_SkAlphaType },
    };
    const FlagOption<skcms_Matrix3x3> kGamuts[] = {
        { "srgb",    SkNamedGamut::kSRGB },
        { "p3",      SkNamedGamut::kDCIP3 },
        { "rec2020", SkNamedGamut::kRec2020 },
        { "adobe",   SkNamedGamut::kAdobeRGB },
        { "narrow",  gNarrow_toXYZD50},
    };
    const FlagOption<skcms_TransferFunction> kTransferFunctions[] = {
        { "srgb"   , SkNamedTransferFn::kSRGB },
        { "rec2020", {2.22222f, 0.909672f, 0.0903276f, 0.222222f, 0.0812429f, 0, 0} },
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
    const SkImageInfo unsized_info = SkImageInfo::Make(0,0, ct,at,cs);

    for (auto source : sources) {
        const auto start = std::chrono::steady_clock::now();
        fprintf(stdout, "%50s", source.name.c_str());
        fflush(stdout);

        const SkImageInfo info = unsized_info.makeWH(source.size.width(),
                                                     source.size.height());

        auto draw = [&source](SkCanvas* canvas) {
            Result result = source.draw(canvas);
            switch (result.status) {
                case Result::Ok:   break;
                case Result::Skip: return false;
                case Result::Fail:
                    SK_ABORT(result.failure.c_str());
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

        if (!image && !blob) {
            fprintf(stdout, "\tskipped\n");
            continue;
        }

        SkBitmap bitmap;
        if (image && !image->asLegacyBitmap(&bitmap)) {
            SK_ABORT("SkImage::asLegacyBitmap() failed.");
        }

        HashAndEncode hashAndEncode{bitmap};
        SkString md5;
        {
            SkMD5 hash;
            if (image) {
                hashAndEncode.write(&hash);
            } else {
                hash.write(blob->data(), blob->size());
            }

            SkMD5::Digest digest = hash.finish();
            for (int i = 0; i < 16; i++) {
                md5.appendf("%02x", digest.data[i]);
            }
        }

        if (!FLAGS_writePath.isEmpty()) {
            sk_mkdir(FLAGS_writePath[0]);
            SkString path = SkStringPrintf("%s/%s%s", FLAGS_writePath[0], source.name.c_str(), ext);

            if (image) {
                if (!hashAndEncode.writePngTo(path.c_str(), md5.c_str(),
                                              FLAGS_key, FLAGS_properties)) {
                    SK_ABORT("Could not write .png.");
                }
            } else {
                SkFILEWStream file(path.c_str());
                file.write(blob->data(), blob->size());
            }
        }

        const auto elapsed = std::chrono::steady_clock::now() - start;
        fprintf(stdout, "\t%s\t%7dms\n",
                md5.c_str(),
                (int)std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
    }

    if (!FLAGS_writeShaders.isEmpty()) {
        sk_mkdir(FLAGS_writeShaders[0]);
        GrBackendApi api =
                GrContextFactory::ContextTypeBackend((GrContextFactory::ContextType)backend);
        memoryCache.writeShadersToDisk(FLAGS_writeShaders[0], api);

    }

    return 0;
}
