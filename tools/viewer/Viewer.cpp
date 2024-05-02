/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/Viewer.h"

#include "bench/GpuTools.h"
#include "gm/gm.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRect.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTextBlob.h"
#include "include/encode/SkPngEncoder.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTPin.h"
#include "include/private/base/SkTo.h"
#include "include/utils/SkPaintFilterCanvas.h"
#include "src/base/SkBase64.h"
#include "src/base/SkTLazy.h"
#include "src/base/SkTSort.h"
#include "src/base/SkUTF.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkLRUCache.h"
#include "src/core/SkMD5.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkScan.h"
#include "src/core/SkStringUtils.h"
#include "src/core/SkTaskGroup.h"
#include "src/core/SkTextBlobPriv.h"
#include "src/image/SkImage_Base.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLString.h"
#include "src/text/GlyphRun.h"
#include "src/utils/SkJSONWriter.h"
#include "src/utils/SkOSPath.h"
#include "src/utils/SkShaderUtils.h"
#include "tools/CodecUtils.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
#include "tools/RuntimeBlendUtils.h"
#include "tools/SkMetaData.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/flags/CommonFlags.h"
#include "tools/skui/InputState.h"
#include "tools/skui/Key.h"
#include "tools/skui/ModifierKey.h"
#include "tools/trace/EventTracingPriv.h"
#include "tools/viewer/BisectSlide.h"
#include "tools/viewer/GMSlide.h"
#include "tools/viewer/ImageSlide.h"
#include "tools/viewer/MSKPSlide.h"
#include "tools/viewer/SKPSlide.h"
#include "tools/viewer/SkSLDebuggerSlide.h"
#include "tools/viewer/SkSLSlide.h"
#include "tools/viewer/Slide.h"
#include "tools/viewer/SlideDir.h"

#include <algorithm>
#include <cfloat>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <map>
#include <memory>
#include <optional>
#include <ratio>
#include <regex>
#include <tuple>
#include <utility>
#include <vector>

#if defined(SK_GANESH)
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrPersistentCacheUtils.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/ops/AtlasPathRenderer.h"
#include "src/gpu/ganesh/ops/TessellationPathRenderer.h"
#endif

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#endif

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"  // For ImGui support of std::string

#if defined(SK_VULKAN)
#include "spirv-tools/libspirv.hpp"
#endif

#if defined(SK_ENABLE_SKOTTIE)
    #include "tools/viewer/SkottieSlide.h"
#endif

#if defined(SK_ENABLE_SVG)
#include "modules/svg/include/SkSVGOpenTypeSVGDecoder.h"
#include "tools/viewer/SvgSlide.h"
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

class CapturingShaderErrorHandler : public GrContextOptions::ShaderErrorHandler {
public:
    void compileError(const char* shader, const char* errors) override {
        fShaders.push_back(SkString(shader));
        fErrors.push_back(SkString(errors));
    }

    void reset() {
        fShaders.clear();
        fErrors.clear();
    }

    TArray<SkString> fShaders;
    TArray<SkString> fErrors;
};

static CapturingShaderErrorHandler gShaderErrorHandler;

GrContextOptions::ShaderErrorHandler* Viewer::ShaderErrorHandler() { return &gShaderErrorHandler; }

using namespace sk_app;
using SkSL::Compiler;
using OverrideFlag = SkSL::Compiler::OverrideFlag;

static std::map<GpuPathRenderers, std::string> gGaneshPathRendererNames;

Application* Application::Create(int argc, char** argv, void* platformData) {
    return new Viewer(argc, argv, platformData);
}

static DEFINE_string(slide, "", "Start on this sample.");
static DEFINE_bool(list, false, "List samples?");

#ifdef SK_GL
#define GL_BACKEND_STR ", \"gl\""
#else
#define GL_BACKEND_STR
#endif
#ifdef SK_VULKAN
#define VK_BACKEND_STR ", \"vk\""
#else
#define VK_BACKEND_STR
#endif
#ifdef SK_METAL
#define MTL_BACKEND_STR ", \"mtl\""
#else
#define MTL_BACKEND_STR
#endif
#ifdef SK_DIRECT3D
#define D3D_BACKEND_STR ", \"d3d\""
#else
#define D3D_BACKEND_STR
#endif
#ifdef SK_DAWN
#define DAWN_BACKEND_STR ", \"dawn\""
#else
#define DAWN_BACKEND_STR
#endif
#define BACKENDS_STR_EVALUATOR(sw, gl, vk, mtl, d3d, dawn) sw gl vk mtl d3d dawn
#define BACKENDS_STR BACKENDS_STR_EVALUATOR( \
    "\"sw\"", GL_BACKEND_STR, VK_BACKEND_STR, MTL_BACKEND_STR, D3D_BACKEND_STR, DAWN_BACKEND_STR)

static DEFINE_string2(backend, b, "sw", "Backend to use. Allowed values are " BACKENDS_STR ".");

static DEFINE_int(msaa, 1, "Number of subpixel samples. 0 for no HW antialiasing.");
static DEFINE_bool(dmsaa, false, "Use internal MSAA to render to non-MSAA surfaces?");

static DEFINE_string(bisect, "", "Path to a .skp or .svg file to bisect.");

static DEFINE_string2(file, f, "", "Open a single file for viewing.");

static DEFINE_string2(match, m, nullptr,
               "[~][^]substring[$] [...] of name to run.\n"
               "Multiple matches may be separated by spaces.\n"
               "~ causes a matching name to always be skipped\n"
               "^ requires the start of the name to match\n"
               "$ requires the end of the name to match\n"
               "^ and $ requires an exact match\n"
               "If a name does not match any list entry,\n"
               "it is skipped unless some list entry starts with ~");

#if defined(SK_GRAPHITE)
#ifdef SK_ENABLE_VELLO_SHADERS
#define COMPUTE_ANALYTIC_PATHSTRATEGY_STR ", \"compute-analytic\""
#define COMPUTE_MSAA16_PATHSTRATEGY_STR ", \"compute-msaa16\""
#define COMPUTE_MSAA8_PATHSTRATEGY_STR ", \"compute-msaa8\""
#else
#define COMPUTE_ANALYTIC_PATHSTRATEGY_STR
#define COMPUTE_MSAA16_PATHSTRATEGY_STR
#define COMPUTE_MSAA8_PATHSTRATEGY_STR
#endif
#define PATHSTRATEGY_STR_EVALUATOR(                                             \
        default, raster, compute_analytic, compute_msaa16, compute_msaa8, tess) \
    default raster compute_analytic compute_msaa16 tess
#define PATHSTRATEGY_STR                                          \
    PATHSTRATEGY_STR_EVALUATOR("\"default\"",                     \
                               "\"raster\"",                      \
                               COMPUTE_ANALYTIC_PATHSTRATEGY_STR, \
                               COMPUTE_MSAA16_PATHSTRATEGY_STR,   \
                               COMPUTE_MSAA8_PATHSTRATEGY_STR,    \
                               "\"tessellation\"")

static DEFINE_string(pathstrategy, "default",
                     "Path renderer strategy to use. Allowed values are " PATHSTRATEGY_STR ".");
#if defined(SK_DAWN)
static DEFINE_bool(disable_tint_symbol_renaming,
                   false,
                   "Disable Tint WGSL symbol renaming when using Dawn");
#endif
#endif

#if defined(SK_BUILD_FOR_ANDROID)
#   define PATH_PREFIX "/data/local/tmp/"
#else
#   define PATH_PREFIX ""
#endif

static DEFINE_string(jpgs   , PATH_PREFIX "jpgs"   , "Directory to read jpgs from.");
static DEFINE_string(jxls   , PATH_PREFIX "jxls"   , "Directory to read jxls from.");
static DEFINE_string(skps   , PATH_PREFIX "skps"   , "Directory to read skps from.");
static DEFINE_string(mskps  , PATH_PREFIX "mskps"  , "Directory to read mskps from.");
static DEFINE_string(lotties, PATH_PREFIX "lotties", "Directory to read (Bodymovin) jsons from.");
#undef PATH_PREFIX

static DEFINE_string(svgs, "", "Directory to read SVGs from, or a single SVG file.");

static DEFINE_string(rives, "", "Directory to read RIVs from, or a single .riv file.");

static DEFINE_int_2(threads, j, -1,
               "Run threadsafe tests on a threadpool with this many extra threads, "
               "defaulting to one extra thread per core.");

static DEFINE_bool(redraw, false, "Toggle continuous redraw.");

static DEFINE_bool(offscreen, false, "Force rendering to an offscreen surface.");
static DEFINE_bool(stats, false, "Display stats overlay on startup.");
static DEFINE_bool(createProtected, false, "Create a protected native backend (e.g., in EGL).");

#ifndef SK_GL
static_assert(false, "viewer requires GL backend for raster.")
#endif

static bool is_graphite_backend_type(sk_app::Window::BackendType type) {
#if defined(SK_GRAPHITE)
    switch (type) {
#ifdef SK_DAWN
        case sk_app::Window::kGraphiteDawn_BackendType:
#endif
#ifdef SK_METAL
        case sk_app::Window::kGraphiteMetal_BackendType:
#endif
#ifdef SK_VULKAN
        case sk_app::Window::kGraphiteVulkan_BackendType:
#endif
            return true;
        default:
            break;
    }
#endif
    return false;
}

#if defined(SK_GRAPHITE)
static const char*
        get_path_renderer_strategy_string(skgpu::graphite::PathRendererStrategy strategy) {
    using Strategy = skgpu::graphite::PathRendererStrategy;
    switch (strategy) {
        case Strategy::kDefault:
            return "Default";
        case Strategy::kComputeAnalyticAA:
            return "GPU Compute AA (Analytic)";
        case Strategy::kComputeMSAA16:
            return "GPU Compute AA (16xMSAA)";
        case Strategy::kComputeMSAA8:
            return "GPU Compute AA (8xMSAA)";
        case Strategy::kRasterAA:
            return "CPU Raster AA";
        case Strategy::kTessellation:
            return "Tessellation";
    }
    return "unknown";
}

static skgpu::graphite::PathRendererStrategy get_path_renderer_strategy_type(const char* str) {
    using Strategy = skgpu::graphite::PathRendererStrategy;
    if (0 == strcmp(str, "default")) {
        return Strategy::kDefault;
    } else if (0 == strcmp(str, "raster")) {
        return Strategy::kRasterAA;
#ifdef SK_ENABLE_VELLO_SHADERS
    } else if (0 == strcmp(str, "compute-analytic")) {
        return Strategy::kComputeAnalyticAA;
    } else if (0 == strcmp(str, "compute-msaa16")) {
        return Strategy::kComputeMSAA16;
    } else if (0 == strcmp(str, "compute-msaa8")) {
        return Strategy::kComputeMSAA8;
#endif
    } else if (0 == strcmp(str, "tessellation")) {
        return Strategy::kTessellation;
    } else {
        SkDebugf("Unknown path renderer strategy type, %s, defaulting to default.", str);
        return Strategy::kDefault;
    }
}
#endif

const char* get_backend_string(sk_app::Window::BackendType type) {
    switch (type) {
        case sk_app::Window::kNativeGL_BackendType: return "OpenGL";
#if SK_ANGLE && (defined(SK_BUILD_FOR_WIN) || defined(SK_BUILD_FOR_MAC))
        case sk_app::Window::kANGLE_BackendType: return "ANGLE";
#endif
#ifdef SK_DAWN
#if defined(SK_GRAPHITE)
        case sk_app::Window::kGraphiteDawn_BackendType: return "Dawn (Graphite)";
#endif
#endif
#ifdef SK_VULKAN
        case sk_app::Window::kVulkan_BackendType: return "Vulkan";
#if defined(SK_GRAPHITE)
        case sk_app::Window::kGraphiteVulkan_BackendType: return "Vulkan (Graphite)";
#endif
#endif
#ifdef SK_METAL
        case sk_app::Window::kMetal_BackendType: return "Metal";
#if defined(SK_GRAPHITE)
        case sk_app::Window::kGraphiteMetal_BackendType: return "Metal (Graphite)";
#endif
#endif
#ifdef SK_DIRECT3D
        case sk_app::Window::kDirect3D_BackendType: return "Direct3D";
#endif
        case sk_app::Window::kRaster_BackendType: return "Raster";
    }
    SkASSERT(false);
    return nullptr;
}

static sk_app::Window::BackendType get_backend_type(const char* str) {
#ifdef SK_DAWN
#if defined(SK_GRAPHITE)
    if (0 == strcmp(str, "grdawn")) {
        return sk_app::Window::kGraphiteDawn_BackendType;
    } else
#endif
#endif
#ifdef SK_VULKAN
    if (0 == strcmp(str, "vk")) {
        return sk_app::Window::kVulkan_BackendType;
    } else
#if defined(SK_GRAPHITE)
        if (0 == strcmp(str, "grvk")) {
            return sk_app::Window::kGraphiteVulkan_BackendType;
        } else
#endif
#endif
#if SK_ANGLE && (defined(SK_BUILD_FOR_WIN) || defined(SK_BUILD_FOR_MAC))
    if (0 == strcmp(str, "angle")) {
        return sk_app::Window::kANGLE_BackendType;
    } else
#endif
#ifdef SK_METAL
    if (0 == strcmp(str, "mtl")) {
        return sk_app::Window::kMetal_BackendType;
    } else
#if defined(SK_GRAPHITE)
    if (0 == strcmp(str, "grmtl")) {
        return sk_app::Window::kGraphiteMetal_BackendType;
    } else
#endif
#endif
#ifdef SK_DIRECT3D
    if (0 == strcmp(str, "d3d")) {
        return sk_app::Window::kDirect3D_BackendType;
    } else
#endif

    if (0 == strcmp(str, "gl")) {
        return sk_app::Window::kNativeGL_BackendType;
    } else if (0 == strcmp(str, "sw")) {
        return sk_app::Window::kRaster_BackendType;
    } else {
        SkDebugf("Unknown backend type, %s, defaulting to sw.", str);
        return sk_app::Window::kRaster_BackendType;
    }
}

static SkColorSpacePrimaries gSrgbPrimaries = {
    0.64f, 0.33f,
    0.30f, 0.60f,
    0.15f, 0.06f,
    0.3127f, 0.3290f };

static SkColorSpacePrimaries gAdobePrimaries = {
    0.64f, 0.33f,
    0.21f, 0.71f,
    0.15f, 0.06f,
    0.3127f, 0.3290f };

static SkColorSpacePrimaries gP3Primaries = {
    0.680f, 0.320f,
    0.265f, 0.690f,
    0.150f, 0.060f,
    0.3127f, 0.3290f };

static SkColorSpacePrimaries gRec2020Primaries = {
    0.708f, 0.292f,
    0.170f, 0.797f,
    0.131f, 0.046f,
    0.3127f, 0.3290f };

struct NamedPrimaries {
    const char* fName;
    SkColorSpacePrimaries* fPrimaries;
} gNamedPrimaries[] = {
    { "sRGB", &gSrgbPrimaries },
    { "AdobeRGB", &gAdobePrimaries },
    { "P3", &gP3Primaries },
    { "Rec. 2020", &gRec2020Primaries },
};

static bool primaries_equal(const SkColorSpacePrimaries& a, const SkColorSpacePrimaries& b) {
    return memcmp(&a, &b, sizeof(SkColorSpacePrimaries)) == 0;
}

static Window::BackendType backend_type_for_window(Window::BackendType backendType) {
    // In raster mode, we still use GL for the window.
    // This lets us render the GUI faster (and correct).
    return Window::kRaster_BackendType == backendType ? Window::kNativeGL_BackendType : backendType;
}

class NullSlide : public Slide {
    void draw(SkCanvas* canvas) override {
        canvas->clear(0xffff11ff);
    }
};

static const char kName[] = "name";
static const char kValue[] = "value";
static const char kOptions[] = "options";
static const char kSlideStateName[] = "Slide";
static const char kBackendStateName[] = "Backend";
static const char kMSAAStateName[] = "MSAA";
static const char kPathRendererStateName[] = "Path renderer";
static const char kSoftkeyStateName[] = "Softkey";
static const char kSoftkeyHint[] = "Please select a softkey";
static const char kON[] = "ON";
static const char kRefreshStateName[] = "Refresh";

Viewer::Viewer(int argc, char** argv, void* platformData)
    : fCurrentSlide(-1)
    , fRefresh(false)
    , fSaveToSKP(false)
    , fShowSlideDimensions(false)
    , fShowImGuiDebugWindow(false)
    , fShowSlidePicker(false)
    , fShowImGuiTestWindow(false)
    , fShowHistogramWindow(false)
    , fShowZoomWindow(false)
    , fZoomWindowFixed(false)
    , fZoomWindowLocation{0.0f, 0.0f}
    , fLastImage(nullptr)
    , fZoomUI(false)
    , fBackendType(sk_app::Window::kNativeGL_BackendType)
    , fColorMode(ColorMode::kLegacy)
    , fColorSpacePrimaries(gSrgbPrimaries)
    // Our UI can only tweak gamma (currently), so start out gamma-only
    , fColorSpaceTransferFn(SkNamedTransferFn::k2Dot2)
    , fApplyBackingScale(true)
    , fZoomLevel(0.0f)
    , fRotation(0.0f)
    , fOffset{0.5f, 0.5f}
    , fGestureDevice(GestureDevice::kNone)
    , fTiled(false)
    , fDrawTileBoundaries(false)
    , fTileScale{0.25f, 0.25f}
    , fPerspectiveMode(kPerspective_Off)
{
    SkGraphics::Init();
#if defined(SK_ENABLE_SVG)
    SkGraphics::SetOpenTypeSVGDecoderFactory(SkSVGOpenTypeSVGDecoder::Make);
#endif
    CodecUtils::RegisterAllAvailable();

    gGaneshPathRendererNames[GpuPathRenderers::kDefault] = "Default Path Renderers";
    gGaneshPathRendererNames[GpuPathRenderers::kAtlas] = "Atlas (tessellation)";
    gGaneshPathRendererNames[GpuPathRenderers::kTessellation] = "Tessellation";
    gGaneshPathRendererNames[GpuPathRenderers::kSmall] = "Small paths (cached sdf or alpha masks)";
    gGaneshPathRendererNames[GpuPathRenderers::kTriangulating] = "Triangulating";
    gGaneshPathRendererNames[GpuPathRenderers::kNone] = "Software masks";

    SkDebugf("Command line arguments: ");
    for (int i = 1; i < argc; ++i) {
        SkDebugf("%s ", argv[i]);
    }
    SkDebugf("\n");

    CommandLineFlags::Parse(argc, argv);
#ifdef SK_BUILD_FOR_ANDROID
    SetResourcePath("/data/local/tmp/resources");
#endif

    initializeEventTracingForTools();
    static SkTaskGroup::Enabler kTaskGroupEnabler(FLAGS_threads);

    fBackendType = get_backend_type(FLAGS_backend[0]);
    fWindow = Window::CreateNativeWindow(platformData);

    DisplayParams displayParams;
    displayParams.fMSAASampleCount = FLAGS_msaa;
    CommonFlags::SetCtxOptions(&displayParams.fGrContextOptions);
    displayParams.fGrContextOptions.fPersistentCache = &fPersistentCache;
    displayParams.fGrContextOptions.fShaderCacheStrategy =
            GrContextOptions::ShaderCacheStrategy::kSkSL;
    displayParams.fGrContextOptions.fShaderErrorHandler = &gShaderErrorHandler;
    displayParams.fGrContextOptions.fSuppressPrints = true;
    displayParams.fGrContextOptions.fSupportBilerpFromGlyphAtlas = true;
    if (FLAGS_dmsaa) {
        displayParams.fSurfaceProps = SkSurfaceProps(
                displayParams.fSurfaceProps.flags() | SkSurfaceProps::kDynamicMSAA_Flag,
                displayParams.fSurfaceProps.pixelGeometry());
    }
    displayParams.fCreateProtectedNativeBackend = FLAGS_createProtected;
#if defined(SK_GRAPHITE)
    displayParams.fGraphiteContextOptions.fPriv.fPathRendererStrategy =
            get_path_renderer_strategy_type(FLAGS_pathstrategy[0]);
#if defined(SK_DAWN)
    displayParams.fDisableTintSymbolRenaming = FLAGS_disable_tint_symbol_renaming;
#endif
#endif
    fWindow->setRequestedDisplayParams(displayParams);
    fDisplay = fWindow->getRequestedDisplayParams();
    fRefresh = FLAGS_redraw;

    fImGuiLayer.setScaleFactor(fWindow->scaleFactor());
    fStatsLayer.setDisplayScale((fZoomUI ? 2.0f : 1.0f) * fWindow->scaleFactor());

    // Configure timers
    fStatsLayer.setActive(FLAGS_stats);
    fAnimateTimer = fStatsLayer.addTimer("Animate", SK_ColorMAGENTA, 0xffff66ff);
    fPaintTimer = fStatsLayer.addTimer("Paint", SK_ColorGREEN);
    fFlushTimer = fStatsLayer.addTimer("Flush", SK_ColorRED, 0xffff6666);

    // register callbacks
    fCommands.attach(fWindow);
    fWindow->pushLayer(this);
    fWindow->pushLayer(&fStatsLayer);
    fWindow->pushLayer(&fImGuiLayer);

    // add key-bindings
    fCommands.addCommand(' ', "GUI", "Toggle Debug GUI", [this]() {
        this->fShowImGuiDebugWindow = !this->fShowImGuiDebugWindow;
        fWindow->inval();
    });
    // Command to jump directly to the slide picker and give it focus
    fCommands.addCommand('/', "GUI", "Jump to slide picker", [this]() {
        this->fShowImGuiDebugWindow = true;
        this->fShowSlidePicker = true;
        fWindow->inval();
    });
    // Alias that to Backspace, to match SampleApp
    fCommands.addCommand(skui::Key::kBack, "Backspace", "GUI", "Jump to slide picker", [this]() {
        this->fShowImGuiDebugWindow = true;
        this->fShowSlidePicker = true;
        fWindow->inval();
    });
    fCommands.addCommand('g', "GUI", "Toggle GUI Demo", [this]() {
        this->fShowImGuiTestWindow = !this->fShowImGuiTestWindow;
        fWindow->inval();
    });
    fCommands.addCommand('z', "GUI", "Toggle zoom window", [this]() {
        this->fShowZoomWindow = !this->fShowZoomWindow;
        fWindow->inval();
    });
    fCommands.addCommand('Z', "GUI", "Toggle zoom window state", [this]() {
        this->fZoomWindowFixed = !this->fZoomWindowFixed;
        fWindow->inval();
    });
    fCommands.addCommand('v', "Swapchain", "Toggle vsync on/off", [this]() {
        DisplayParams params = fWindow->getRequestedDisplayParams();
        params.fDisableVsync = !params.fDisableVsync;
        fWindow->setRequestedDisplayParams(params);
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('V', "Swapchain", "Toggle delayed acquire on/off (Metal only)", [this]() {
        DisplayParams params = fWindow->getRequestedDisplayParams();
        params.fDelayDrawableAcquisition = !params.fDelayDrawableAcquisition;
        fWindow->setRequestedDisplayParams(params);
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('r', "Redraw", "Toggle redraw", [this]() {
        fRefresh = !fRefresh;
        fWindow->inval();
    });
    fCommands.addCommand('s', "Overlays", "Toggle stats display", [this]() {
        fStatsLayer.setActive(!fStatsLayer.getActive());
        fWindow->inval();
    });
    fCommands.addCommand('0', "Overlays", "Reset stats", [this]() {
        fStatsLayer.resetMeasurements();
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('C', "GUI", "Toggle color histogram", [this]() {
        this->fShowHistogramWindow = !this->fShowHistogramWindow;
        fWindow->inval();
    });
    fCommands.addCommand('c', "Modes", "Cycle color mode", [this]() {
        switch (fColorMode) {
            case ColorMode::kLegacy:
                this->setColorMode(ColorMode::kColorManaged8888);
                break;
            case ColorMode::kColorManaged8888:
                this->setColorMode(ColorMode::kColorManagedF16);
                break;
            case ColorMode::kColorManagedF16:
                this->setColorMode(ColorMode::kColorManagedF16Norm);
                break;
            case ColorMode::kColorManagedF16Norm:
                this->setColorMode(ColorMode::kLegacy);
                break;
        }
    });
    fCommands.addCommand('w', "Modes", "Toggle wireframe", [this]() {
        DisplayParams params = fWindow->getRequestedDisplayParams();
        params.fGrContextOptions.fWireframeMode = !params.fGrContextOptions.fWireframeMode;
        fWindow->setRequestedDisplayParams(params);
        fWindow->inval();
    });
    fCommands.addCommand('w', "Modes", "Toggle reduced shaders", [this]() {
      DisplayParams params = fWindow->getRequestedDisplayParams();
      params.fGrContextOptions.fReducedShaderVariations =
              !params.fGrContextOptions.fReducedShaderVariations;
      fWindow->setRequestedDisplayParams(params);
      fWindow->inval();
    });
    fCommands.addCommand(skui::Key::kRight, "Right", "Navigation", "Next slide", [this]() {
        this->setCurrentSlide(fCurrentSlide < fSlides.size() - 1 ? fCurrentSlide + 1 : 0);
    });
    fCommands.addCommand(skui::Key::kLeft, "Left", "Navigation", "Previous slide", [this]() {
        this->setCurrentSlide(fCurrentSlide > 0 ? fCurrentSlide - 1 : fSlides.size() - 1);
    });
    fCommands.addCommand(skui::Key::kUp, "Up", "Transform", "Zoom in", [this]() {
        this->changeZoomLevel(1.f / 32.f);
        fWindow->inval();
    });
    fCommands.addCommand(skui::Key::kDown, "Down", "Transform", "Zoom out", [this]() {
        this->changeZoomLevel(-1.f / 32.f);
        fWindow->inval();
    });
    fCommands.addCommand('d', "Modes", "Change rendering backend", [this]() {
        sk_app::Window::BackendType newBackend = (sk_app::Window::BackendType)(
                (fBackendType + 1) % sk_app::Window::kBackendTypeCount);
        // Switching to and from Vulkan is problematic on Linux so disabled for now
#if defined(SK_BUILD_FOR_UNIX) && defined(SK_VULKAN)
        if (newBackend == sk_app::Window::kVulkan_BackendType) {
            newBackend = (sk_app::Window::BackendType)((newBackend + 1) %
                                                       sk_app::Window::kBackendTypeCount);
        } else if (fBackendType == sk_app::Window::kVulkan_BackendType) {
            newBackend = sk_app::Window::kVulkan_BackendType;
        }
#endif
        this->setBackend(newBackend);
    });
    fCommands.addCommand('K', "IO", "Save slide to SKP", [this]() {
        fSaveToSKP = true;
        fWindow->inval();
    });
    fCommands.addCommand('&', "Overlays", "Show slide dimensios", [this]() {
        fShowSlideDimensions = !fShowSlideDimensions;
        fWindow->inval();
    });
    fCommands.addCommand('G', "Modes", "Geometry", [this]() {
        DisplayParams params = fWindow->getRequestedDisplayParams();
        uint32_t flags = params.fSurfaceProps.flags();
        SkPixelGeometry defaultPixelGeometry = fDisplay.fSurfaceProps.pixelGeometry();
        if (!fDisplayOverrides.fSurfaceProps.fPixelGeometry) {
            fDisplayOverrides.fSurfaceProps.fPixelGeometry = true;
            params.fSurfaceProps = SkSurfaceProps(flags, kUnknown_SkPixelGeometry);
        } else {
            switch (params.fSurfaceProps.pixelGeometry()) {
                case kUnknown_SkPixelGeometry:
                    params.fSurfaceProps = SkSurfaceProps(flags, kRGB_H_SkPixelGeometry);
                    break;
                case kRGB_H_SkPixelGeometry:
                    params.fSurfaceProps = SkSurfaceProps(flags, kBGR_H_SkPixelGeometry);
                    break;
                case kBGR_H_SkPixelGeometry:
                    params.fSurfaceProps = SkSurfaceProps(flags, kRGB_V_SkPixelGeometry);
                    break;
                case kRGB_V_SkPixelGeometry:
                    params.fSurfaceProps = SkSurfaceProps(flags, kBGR_V_SkPixelGeometry);
                    break;
                case kBGR_V_SkPixelGeometry:
                    params.fSurfaceProps = SkSurfaceProps(flags, defaultPixelGeometry);
                    fDisplayOverrides.fSurfaceProps.fPixelGeometry = false;
                    break;
            }
        }
        fWindow->setRequestedDisplayParams(params);
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('H', "Font", "Hinting mode", [this]() {
        if (!fFontOverrides.fHinting) {
            fFontOverrides.fHinting = true;
            fFont.setHinting(SkFontHinting::kNone);
        } else {
            switch (fFont.getHinting()) {
                case SkFontHinting::kNone:
                    fFont.setHinting(SkFontHinting::kSlight);
                    break;
                case SkFontHinting::kSlight:
                    fFont.setHinting(SkFontHinting::kNormal);
                    break;
                case SkFontHinting::kNormal:
                    fFont.setHinting(SkFontHinting::kFull);
                    break;
                case SkFontHinting::kFull:
                    fFont.setHinting(SkFontHinting::kNone);
                    fFontOverrides.fHinting = false;
                    break;
            }
        }
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('D', "Modes", "DFT", [this]() {
        DisplayParams params = fWindow->getRequestedDisplayParams();
        uint32_t flags = params.fSurfaceProps.flags();
        flags ^= SkSurfaceProps::kUseDeviceIndependentFonts_Flag;
        params.fSurfaceProps = SkSurfaceProps(flags, params.fSurfaceProps.pixelGeometry());
        fWindow->setRequestedDisplayParams(params);
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('L', "Font", "Subpixel Antialias Mode", [this]() {
        if (!fFontOverrides.fEdging) {
            fFontOverrides.fEdging = true;
            fFont.setEdging(SkFont::Edging::kAlias);
        } else {
            switch (fFont.getEdging()) {
                case SkFont::Edging::kAlias:
                    fFont.setEdging(SkFont::Edging::kAntiAlias);
                    break;
                case SkFont::Edging::kAntiAlias:
                    fFont.setEdging(SkFont::Edging::kSubpixelAntiAlias);
                    break;
                case SkFont::Edging::kSubpixelAntiAlias:
                    fFont.setEdging(SkFont::Edging::kAlias);
                    fFontOverrides.fEdging = false;
                    break;
            }
        }
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('S', "Font", "Subpixel Position Mode", [this]() {
        if (!fFontOverrides.fSubpixel) {
            fFontOverrides.fSubpixel = true;
            fFont.setSubpixel(false);
        } else {
            if (!fFont.isSubpixel()) {
                fFont.setSubpixel(true);
            } else {
                fFontOverrides.fSubpixel = false;
            }
        }
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('B', "Font", "Baseline Snapping", [this]() {
        if (!fFontOverrides.fBaselineSnap) {
            fFontOverrides.fBaselineSnap = true;
            fFont.setBaselineSnap(false);
        } else {
            if (!fFont.isBaselineSnap()) {
                fFont.setBaselineSnap(true);
            } else {
                fFontOverrides.fBaselineSnap = false;
            }
        }
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('p', "Transform", "Toggle Perspective Mode", [this]() {
        fPerspectiveMode = (kPerspective_Real == fPerspectiveMode) ? kPerspective_Fake
                                                                   : kPerspective_Real;
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('P', "Transform", "Toggle Perspective", [this]() {
        fPerspectiveMode = (kPerspective_Off == fPerspectiveMode) ? kPerspective_Real
                                                                  : kPerspective_Off;
        this->updateTitle();
        fWindow->inval();
    });
    fCommands.addCommand('a', "Transform", "Toggle Animation", [this]() {
        fAnimTimer.togglePauseResume();
    });
    fCommands.addCommand('u', "GUI", "Zoom UI", [this]() {
        fZoomUI = !fZoomUI;
        fStatsLayer.setDisplayScale((fZoomUI ? 2.0f : 1.0f) * fWindow->scaleFactor());
        fWindow->inval();
    });
    fCommands.addCommand('$', "ViaSerialize", "Toggle ViaSerialize", [this]() {
        fDrawViaSerialize = !fDrawViaSerialize;
        this->updateTitle();
        fWindow->inval();
    });

    // set up slides
    this->initSlides();
    if (FLAGS_list) {
        this->listNames();
    }

    fPerspectivePoints[0].set(0, 0);
    fPerspectivePoints[1].set(1, 0);
    fPerspectivePoints[2].set(0, 1);
    fPerspectivePoints[3].set(1, 1);
    fAnimTimer.run();

    auto gamutImage = ToolUtils::GetResourceAsImage("images/gamut.png");
    if (gamutImage) {
        fImGuiGamutPaint.setShader(gamutImage->makeShader(SkSamplingOptions(SkFilterMode::kLinear)));
    }
    fImGuiGamutPaint.setColor(SK_ColorWHITE);

    fWindow->attach(backend_type_for_window(fBackendType));
    this->setCurrentSlide(this->startupSlide());
}

static sk_sp<SkData> data_from_file(FILE* fp) {
    SkDynamicMemoryWStream stream;
    char buf[4096];
    while (size_t bytesRead = fread(buf, 1, 4096, fp)) {
        stream.write(buf, bytesRead);
    }
    return stream.detachAsData();
}

static sk_sp<SkData> base64_string_to_data(const std::string& s) {
    size_t dataLen;
    if (SkBase64::Decode(s.c_str(), s.size(), nullptr, &dataLen) != SkBase64::kNoError) {
        return nullptr;
    }

    sk_sp<SkData> decodedData = SkData::MakeUninitialized(dataLen);
    void* rawData = decodedData->writable_data();
    if (SkBase64::Decode(s.c_str(), s.size(), rawData, &dataLen) != SkBase64::kNoError) {
        return nullptr;
    }

    return decodedData;
}

static std::vector<sk_sp<SkImage>> find_data_uri_images(sk_sp<SkData> data) {
    std::string str(reinterpret_cast<const char*>(data->data()), data->size());
    std::regex re("data:image/png;base64,([a-zA-Z0-9+/=]+)");
    std::sregex_iterator images_begin(str.begin(), str.end(), re);
    std::sregex_iterator images_end;
    std::vector<sk_sp<SkImage>> images;

    for (auto iter = images_begin; iter != images_end; ++iter) {
        const std::smatch& match = *iter;
        auto raw = base64_string_to_data(match[1].str());
        if (!raw) {
            continue;
        }
        auto image = SkImages::DeferredFromEncodedData(std::move(raw));
        if (image) {
            images.push_back(std::move(image));
        }
    }

    return images;
}

void Viewer::initSlides() {
    using SlideMaker = sk_sp<Slide> (*)(const SkString& name, const SkString& path);
    static const struct {
        const char*                            fExtension;
        const char*                            fDirName;
        const CommandLineFlags::StringArray&   fFlags;
        const SlideMaker                       fFactory;
    } gExternalSlidesInfo[] = {
        { ".mskp", "mskp-dir", FLAGS_mskps,
          [](const SkString& name, const SkString& path) -> sk_sp<Slide> {
            return sk_make_sp<MSKPSlide>(name, path);}
        },
        { ".skp", "skp-dir", FLAGS_skps,
            [](const SkString& name, const SkString& path) -> sk_sp<Slide> {
                return sk_make_sp<SKPSlide>(name, path);}
        },
        { ".jpg", "jpg-dir", FLAGS_jpgs,
            [](const SkString& name, const SkString& path) -> sk_sp<Slide> {
                return sk_make_sp<ImageSlide>(name, path);}
        },
        { ".jxl", "jxl-dir", FLAGS_jxls,
            [](const SkString& name, const SkString& path) -> sk_sp<Slide> {
                return sk_make_sp<ImageSlide>(name, path);}
        },
#if defined(SK_ENABLE_SKOTTIE)
        { ".json", "skottie-dir", FLAGS_lotties,
            [](const SkString& name, const SkString& path) -> sk_sp<Slide> {
                return sk_make_sp<SkottieSlide>(name, path);}
        },
#endif

#if defined(SK_ENABLE_SVG)
        { ".svg", "svg-dir", FLAGS_svgs,
            [](const SkString& name, const SkString& path) -> sk_sp<Slide> {
                return sk_make_sp<SvgSlide>(name, path);}
        },
#endif
    };

    TArray<sk_sp<Slide>> dirSlides;

    const auto addSlide = [&](const SkString& name, const SkString& path, const SlideMaker& fact) {
        if (CommandLineFlags::ShouldSkip(FLAGS_match, name.c_str())) {
            return;
        }

        if (auto slide = fact(name, path)) {
            dirSlides.push_back(slide);
            fSlides.push_back(std::move(slide));
        }
    };

    if (!FLAGS_file.isEmpty()) {
        // single file mode
        const SkString file(FLAGS_file[0]);

        // `--file stdin` parses stdin, looking for data URIs that encode images
        if (file.equals("stdin")) {
            sk_sp<SkData> data = data_from_file(stdin);
            std::vector<sk_sp<SkImage>> images = find_data_uri_images(std::move(data));
            // TODO: If there is an even number of images, create diff images from consecutive pairs
            // (Maybe do this optionally? Or add a dedicated diff-slide that can show diff stats?)
            for (auto image : images) {
                char imageID = 'A' + fSlides.size();
                fSlides.push_back(sk_make_sp<ImageSlide>(SkStringPrintf("Image %c", imageID),
                                                         std::move(image)));
            }
            if (!fSlides.empty()) {
                fShowZoomWindow = true;
                return;
            }
        }

        if (sk_exists(file.c_str(), kRead_SkFILE_Flag)) {
            for (const auto& sinfo : gExternalSlidesInfo) {
                if (file.endsWith(sinfo.fExtension)) {
                    addSlide(SkOSPath::Basename(file.c_str()), file, sinfo.fFactory);
                    return;
                }
            }

            fprintf(stderr, "Unsupported file type \"%s\"\n", file.c_str());
        } else {
            fprintf(stderr, "Cannot read \"%s\"\n", file.c_str());
        }

        return;
    }

    // Bisect slide.
    if (!FLAGS_bisect.isEmpty()) {
        sk_sp<BisectSlide> bisect = BisectSlide::Create(FLAGS_bisect[0]);
        if (bisect && !CommandLineFlags::ShouldSkip(FLAGS_match, bisect->getName().c_str())) {
            if (FLAGS_bisect.size() >= 2) {
                for (const char* ch = FLAGS_bisect[1]; *ch; ++ch) {
                    bisect->onChar(*ch);
                }
            }
            fSlides.push_back(std::move(bisect));
        }
    }

    // GMs
    int firstGM = fSlides.size();
    for (const skiagm::GMFactory& gmFactory : skiagm::GMRegistry::Range()) {
        std::unique_ptr<skiagm::GM> gm = gmFactory();
        if (!CommandLineFlags::ShouldSkip(FLAGS_match, gm->getName().c_str())) {
            auto slide = sk_make_sp<GMSlide>(std::move(gm));
            fSlides.push_back(std::move(slide));
        }
    }

    auto orderBySlideName = [](sk_sp<Slide> a, sk_sp<Slide> b) {
        return SK_strcasecmp(a->getName().c_str(), b->getName().c_str()) < 0;
    };
    std::sort(fSlides.begin() + firstGM, fSlides.end(), orderBySlideName);

    int firstRegisteredSlide = fSlides.size();

    // Registered slides are replacing Samples.
    for (const SlideFactory& factory : SlideRegistry::Range()) {
        auto slide = sk_sp<Slide>(factory());
        if (!CommandLineFlags::ShouldSkip(FLAGS_match, slide->getName().c_str())) {
            fSlides.push_back(slide);
        }
    }

    std::sort(fSlides.begin() + firstRegisteredSlide, fSlides.end(), orderBySlideName);

    // Runtime shader editor
    {
        auto slide = sk_make_sp<SkSLSlide>();
        if (!CommandLineFlags::ShouldSkip(FLAGS_match, slide->getName().c_str())) {
            fSlides.push_back(std::move(slide));
        }
    }

    // Runtime shader debugger
    {
        auto slide = sk_make_sp<SkSLDebuggerSlide>();
        if (!CommandLineFlags::ShouldSkip(FLAGS_match, slide->getName().c_str())) {
            fSlides.push_back(std::move(slide));
        }
    }

    for (const auto& info : gExternalSlidesInfo) {
        for (const auto& flag : info.fFlags) {
            if (SkStrEndsWith(flag.c_str(), info.fExtension)) {
                // single file
                addSlide(SkOSPath::Basename(flag.c_str()), flag, info.fFactory);
            } else {
                // directory
                SkString name;
                TArray<SkString> sortedFilenames;
                SkOSFile::Iter it(flag.c_str(), info.fExtension);
                while (it.next(&name)) {
                    sortedFilenames.push_back(name);
                }
                if (sortedFilenames.size()) {
                    SkTQSort(sortedFilenames.begin(), sortedFilenames.end(),
                             [](const SkString& a, const SkString& b) {
                                 return strcmp(a.c_str(), b.c_str()) < 0;
                             });
                }
                for (const SkString& filename : sortedFilenames) {
                    addSlide(filename, SkOSPath::Join(flag.c_str(), filename.c_str()),
                             info.fFactory);
                }
            }
            if (!dirSlides.empty()) {
                fSlides.push_back(
                    sk_make_sp<SlideDir>(SkStringPrintf("%s[%s]", info.fDirName, flag.c_str()),
                                         std::move(dirSlides)));
                dirSlides.clear();  // NOLINT(bugprone-use-after-move)
            }
        }
    }

    if (fSlides.empty()) {
        auto slide = sk_make_sp<NullSlide>();
        fSlides.push_back(std::move(slide));
    }
}


Viewer::~Viewer() {
    for(auto& slide : fSlides) {
        slide->gpuTeardown();
    }

    fWindow->detach();
    delete fWindow;
}

struct SkPaintTitleUpdater {
    SkPaintTitleUpdater(SkString* title) : fTitle(title), fCount(0) {}
    void append(const char* s) {
        if (fCount == 0) {
            fTitle->append(" {");
        } else {
            fTitle->append(", ");
        }
        fTitle->append(s);
        ++fCount;
    }
    void done() {
        if (fCount > 0) {
            fTitle->append("}");
        }
    }
    SkString* fTitle;
    int fCount;
};

void Viewer::updateTitle() {
    if (!fWindow) {
        return;
    }
    if (fWindow->sampleCount() < 1) {
        return; // Surface hasn't been created yet.
    }

    SkString title("Viewer: ");
    title.append(fSlides[fCurrentSlide]->getName());

    if (fDrawViaSerialize) {
        title.append(" <serialize>");
    }

    SkPaintTitleUpdater paintTitle(&title);
    auto paintFlag = [this, &paintTitle](bool SkPaintFields::* flag,
                                         bool (SkPaint::* isFlag)() const,
                                         const char* on, const char* off)
    {
        if (fPaintOverrides.*flag) {
            paintTitle.append((fPaint.*isFlag)() ? on : off);
        }
    };

    auto fontFlag = [this, &paintTitle](bool SkFontFields::* flag, bool (SkFont::* isFlag)() const,
                                        const char* on, const char* off)
    {
        if (fFontOverrides.*flag) {
            paintTitle.append((fFont.*isFlag)() ? on : off);
        }
    };

    paintFlag(&SkPaintFields::fAntiAlias, &SkPaint::isAntiAlias, "Antialias", "Alias");
    paintFlag(&SkPaintFields::fDither, &SkPaint::isDither, "DITHER", "No Dither");

    fontFlag(&SkFontFields::fForceAutoHinting, &SkFont::isForceAutoHinting,
             "Force Autohint", "No Force Autohint");
    fontFlag(&SkFontFields::fEmbolden, &SkFont::isEmbolden, "Fake Bold", "No Fake Bold");
    fontFlag(&SkFontFields::fBaselineSnap, &SkFont::isBaselineSnap, "BaseSnap", "No BaseSnap");
    fontFlag(&SkFontFields::fLinearMetrics, &SkFont::isLinearMetrics,
             "Linear Metrics", "Non-Linear Metrics");
    fontFlag(&SkFontFields::fEmbeddedBitmaps, &SkFont::isEmbeddedBitmaps,
             "Bitmap Text", "No Bitmap Text");
    fontFlag(&SkFontFields::fSubpixel, &SkFont::isSubpixel, "Subpixel Text", "Pixel Text");

    if (fFontOverrides.fEdging) {
        switch (fFont.getEdging()) {
            case SkFont::Edging::kAlias:
                paintTitle.append("Alias Text");
                break;
            case SkFont::Edging::kAntiAlias:
                paintTitle.append("Antialias Text");
                break;
            case SkFont::Edging::kSubpixelAntiAlias:
                paintTitle.append("Subpixel Antialias Text");
                break;
        }
    }

    if (fFontOverrides.fHinting) {
        switch (fFont.getHinting()) {
            case SkFontHinting::kNone:
                paintTitle.append("No Hinting");
                break;
            case SkFontHinting::kSlight:
                paintTitle.append("Slight Hinting");
                break;
            case SkFontHinting::kNormal:
                paintTitle.append("Normal Hinting");
                break;
            case SkFontHinting::kFull:
                paintTitle.append("Full Hinting");
                break;
        }
    }
    paintTitle.done();

    switch (fColorMode) {
        case ColorMode::kLegacy:
            title.append(" Legacy 8888");
            break;
        case ColorMode::kColorManaged8888:
            title.append(" ColorManaged 8888");
            break;
        case ColorMode::kColorManagedF16:
            title.append(" ColorManaged F16");
            break;
        case ColorMode::kColorManagedF16Norm:
            title.append(" ColorManaged F16 Norm");
            break;
    }

    if (ColorMode::kLegacy != fColorMode) {
        int curPrimaries = -1;
        for (size_t i = 0; i < std::size(gNamedPrimaries); ++i) {
            if (primaries_equal(*gNamedPrimaries[i].fPrimaries, fColorSpacePrimaries)) {
                curPrimaries = i;
                break;
            }
        }
        title.appendf(" %s Gamma %f",
                      curPrimaries >= 0 ? gNamedPrimaries[curPrimaries].fName : "Custom",
                      fColorSpaceTransferFn.g);
    }

    const DisplayParams& params = fWindow->getRequestedDisplayParams();
    if (fDisplayOverrides.fSurfaceProps.fPixelGeometry) {
        switch (params.fSurfaceProps.pixelGeometry()) {
            case kUnknown_SkPixelGeometry:
                title.append( " Flat");
                break;
            case kRGB_H_SkPixelGeometry:
                title.append( " RGB");
                break;
            case kBGR_H_SkPixelGeometry:
                title.append( " BGR");
                break;
            case kRGB_V_SkPixelGeometry:
                title.append( " RGBV");
                break;
            case kBGR_V_SkPixelGeometry:
                title.append( " BGRV");
                break;
        }
    }

    if (params.fSurfaceProps.isUseDeviceIndependentFonts()) {
        title.append(" DFT");
    }

    title.append(" [");
    title.append(get_backend_string(fBackendType));
    int msaa = fWindow->sampleCount();
    if (msaa > 1) {
        title.appendf(" MSAA: %i", msaa);
    }
    title.append("]");

    if (is_graphite_backend_type(fBackendType)) {
#if defined(SK_GRAPHITE)
        skgpu::graphite::PathRendererStrategy strategy =
                fWindow->getRequestedDisplayParams()
                        .fGraphiteContextOptions.fPriv.fPathRendererStrategy;
        if (skgpu::graphite::PathRendererStrategy::kDefault != strategy) {
            title.appendf(" [Path renderer strategy: %s]",
                          get_path_renderer_strategy_string(strategy));
        }
#endif
    } else {
        GpuPathRenderers pr =
                fWindow->getRequestedDisplayParams().fGrContextOptions.fGpuPathRenderers;
        if (GpuPathRenderers::kDefault != pr) {
            title.appendf(" [Path renderer: %s]", gGaneshPathRendererNames[pr].c_str());
        }
    }

    if (kPerspective_Real == fPerspectiveMode) {
        title.append(" Perspective (Real)");
    } else if (kPerspective_Fake == fPerspectiveMode) {
        title.append(" Perspective (Fake)");
    }

    fWindow->setTitle(title.c_str());
}

int Viewer::startupSlide() const {

    if (!FLAGS_slide.isEmpty()) {
        int count = fSlides.size();
        for (int i = 0; i < count; i++) {
            if (fSlides[i]->getName().equals(FLAGS_slide[0])) {
                return i;
            }
        }

        fprintf(stderr, "Unknown slide \"%s\"\n", FLAGS_slide[0]);
        this->listNames();
    }

    return 0;
}

void Viewer::listNames() const {
    SkDebugf("All Slides:\n");
    for (const auto& slide : fSlides) {
        SkDebugf("    %s\n", slide->getName().c_str());
    }
}

void Viewer::setCurrentSlide(int slide) {
    SkASSERT(slide >= 0 && slide < fSlides.size());

    if (slide == fCurrentSlide) {
        return;
    }

    if (fCurrentSlide >= 0) {
        fSlides[fCurrentSlide]->unload();
    }

    SkScalar scaleFactor = 1.0;
    if (fApplyBackingScale) {
        scaleFactor = fWindow->scaleFactor();
    }
    fSlides[slide]->load(SkIntToScalar(fWindow->width()) / scaleFactor,
                         SkIntToScalar(fWindow->height()) / scaleFactor);
    fCurrentSlide = slide;
    this->setupCurrentSlide();
}

SkISize Viewer::currentSlideSize() const {
    if (auto size = fSlides[fCurrentSlide]->getDimensions(); !size.isEmpty()) {
        return size;
    }
    return {fWindow->width(), fWindow->height()};
}

void Viewer::setupCurrentSlide() {
    if (fCurrentSlide >= 0) {
        // prepare dimensions for image slides
        fGesture.resetTouchState();
        fDefaultMatrix.reset();

        const SkRect slideBounds = SkRect::Make(this->currentSlideSize());
        const SkRect windowRect = SkRect::MakeIWH(fWindow->width(), fWindow->height());

        // Start with a matrix that scales the slide to the available screen space
        if (fWindow->scaleContentToFit()) {
            if (windowRect.width() > 0 && windowRect.height() > 0) {
                fDefaultMatrix = SkMatrix::RectToRect(slideBounds, windowRect,
                                                      SkMatrix::kStart_ScaleToFit);
            }
        }

        // Prevent the user from dragging content so far outside the window they can't find it again
        fGesture.setTransLimit(slideBounds, windowRect, this->computePreTouchMatrix());

        this->updateTitle();
        this->updateUIState();

        fStatsLayer.resetMeasurements();

        fWindow->inval();
    }
}

#define MAX_ZOOM_LEVEL  8.0f
#define MIN_ZOOM_LEVEL  -8.0f

void Viewer::changeZoomLevel(float delta) {
    fZoomLevel += delta;
    fZoomLevel = SkTPin(fZoomLevel, MIN_ZOOM_LEVEL, MAX_ZOOM_LEVEL);
    this->preTouchMatrixChanged();
}

void Viewer::preTouchMatrixChanged() {
    // Update the trans limit as the transform changes.
    const SkRect slideBounds = SkRect::Make(this->currentSlideSize());
    const SkRect windowRect = SkRect::MakeIWH(fWindow->width(), fWindow->height());
    fGesture.setTransLimit(slideBounds, windowRect, this->computePreTouchMatrix());
}

SkMatrix Viewer::computePerspectiveMatrix() {
    SkScalar w = fWindow->width(), h = fWindow->height();
    SkPoint orthoPts[4] = { { 0, 0 }, { w, 0 }, { 0, h }, { w, h } };
    SkPoint perspPts[4] = {
        { fPerspectivePoints[0].fX * w, fPerspectivePoints[0].fY * h },
        { fPerspectivePoints[1].fX * w, fPerspectivePoints[1].fY * h },
        { fPerspectivePoints[2].fX * w, fPerspectivePoints[2].fY * h },
        { fPerspectivePoints[3].fX * w, fPerspectivePoints[3].fY * h }
    };
    SkMatrix m;
    m.setPolyToPoly(orthoPts, perspPts, 4);
    return m;
}

SkMatrix Viewer::computePreTouchMatrix() {
    SkMatrix m = fDefaultMatrix;

    SkScalar zoomScale = exp(fZoomLevel);
    if (fApplyBackingScale) {
        zoomScale *= fWindow->scaleFactor();
    }
    m.preTranslate((fOffset.x() - 0.5f) * 2.0f, (fOffset.y() - 0.5f) * 2.0f);
    m.preScale(zoomScale, zoomScale);

    const SkISize slideSize = this->currentSlideSize();
    m.preRotate(fRotation, slideSize.width() * 0.5f, slideSize.height() * 0.5f);

    if (kPerspective_Real == fPerspectiveMode) {
        SkMatrix persp = this->computePerspectiveMatrix();
        m.postConcat(persp);
    }

    return m;
}

SkMatrix Viewer::computeMatrix() {
    SkMatrix m = fGesture.localM();
    m.preConcat(fGesture.globalM());
    m.preConcat(this->computePreTouchMatrix());
    return m;
}

void Viewer::setBackend(sk_app::Window::BackendType backendType) {
    fPersistentCache.reset();
    fCachedShaders.clear();
    fBackendType = backendType;

    // The active context is going away in 'detach'
    for(auto& slide : fSlides) {
        slide->gpuTeardown();
    }

    fWindow->detach();

#if defined(SK_BUILD_FOR_WIN)
    // Switching between OpenGL, Vulkan, and ANGLE in the same window is problematic at this point
    // on Windows, so we just delete the window and recreate it.
    DisplayParams params = fWindow->getRequestedDisplayParams();
    delete fWindow;
    fWindow = Window::CreateNativeWindow(nullptr);

    // re-register callbacks
    fCommands.attach(fWindow);
    fWindow->pushLayer(this);
    fWindow->pushLayer(&fStatsLayer);
    fWindow->pushLayer(&fImGuiLayer);

    // Don't allow the window to re-attach. If we're in MSAA mode, the params we grabbed above
    // will still include our correct sample count. But the re-created fWindow will lose that
    // information. On Windows, we need to re-create the window when changing sample count,
    // so we'll incorrectly detect that situation, then re-initialize the window in GL mode,
    // rendering this tear-down step pointless (and causing the Vulkan window context to fail
    // as if we had never changed windows at all).
    fWindow->setRequestedDisplayParams(params, false);
#endif

    fWindow->attach(backend_type_for_window(fBackendType));
}

void Viewer::setColorMode(ColorMode colorMode) {
    fColorMode = colorMode;
    this->updateTitle();
    fWindow->inval();
}

class OveridePaintFilterCanvas : public SkPaintFilterCanvas {
public:
    OveridePaintFilterCanvas(SkCanvas* canvas,
                             SkPaint* paint, Viewer::SkPaintFields* pfields,
                             SkFont* font, Viewer::SkFontFields* ffields)
        : SkPaintFilterCanvas(canvas)
        , fPaint(paint)
        , fPaintOverrides(pfields)
        , fFont(font)
        , fFontOverrides(ffields) {
    }

    const SkTextBlob* filterTextBlob(const SkPaint& paint,
                                     const SkTextBlob* blob,
                                     sk_sp<SkTextBlob>* cache) {
        bool blobWillChange = false;
        for (SkTextBlobRunIterator it(blob); !it.done(); it.next()) {
            SkTCopyOnFirstWrite<SkFont> filteredFont(it.font());
            bool shouldDraw = this->filterFont(&filteredFont);
            if (it.font() != *filteredFont || !shouldDraw) {
                blobWillChange = true;
                break;
            }
        }
        if (!blobWillChange) {
            return blob;
        }

        SkTextBlobBuilder builder;
        for (SkTextBlobRunIterator it(blob); !it.done(); it.next()) {
            SkTCopyOnFirstWrite<SkFont> filteredFont(it.font());
            bool shouldDraw = this->filterFont(&filteredFont);
            if (!shouldDraw) {
                continue;
            }

            SkFont font = *filteredFont;

            const SkTextBlobBuilder::RunBuffer& runBuffer
                = it.positioning() == SkTextBlobRunIterator::kDefault_Positioning
                    ? builder.allocRunText(font, it.glyphCount(), it.offset().x(),it.offset().y(),
                                           it.textSize())
                : it.positioning() == SkTextBlobRunIterator::kHorizontal_Positioning
                    ? builder.allocRunTextPosH(font, it.glyphCount(), it.offset().y(),
                                               it.textSize())
                : it.positioning() == SkTextBlobRunIterator::kFull_Positioning
                    ? builder.allocRunTextPos(font, it.glyphCount(), it.textSize())
                : it.positioning() == SkTextBlobRunIterator::kRSXform_Positioning
                    ? builder.allocRunTextRSXform(font, it.glyphCount(), it.textSize())
                : (SkASSERT_RELEASE(false), SkTextBlobBuilder::RunBuffer());
            uint32_t glyphCount = it.glyphCount();
            if (it.glyphs()) {
                size_t glyphSize = sizeof(decltype(*it.glyphs()));
                memcpy(runBuffer.glyphs, it.glyphs(), glyphCount * glyphSize);
            }
            if (it.pos()) {
                size_t posSize = sizeof(decltype(*it.pos()));
                unsigned posPerGlyph = it.scalarsPerGlyph();
                memcpy(runBuffer.pos, it.pos(), glyphCount * posPerGlyph * posSize);
            }
            if (it.text()) {
                size_t textSize = sizeof(decltype(*it.text()));
                uint32_t textCount = it.textSize();
                memcpy(runBuffer.utf8text, it.text(), textCount * textSize);
            }
            if (it.clusters()) {
                size_t clusterSize = sizeof(decltype(*it.clusters()));
                memcpy(runBuffer.clusters, it.clusters(), glyphCount * clusterSize);
            }
        }
        *cache = builder.make();
        return cache->get();
    }
    void onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                        const SkPaint& paint) override {
        sk_sp<SkTextBlob> cache;
        this->SkPaintFilterCanvas::onDrawTextBlob(
            this->filterTextBlob(paint, blob, &cache), x, y, paint);
    }

    void onDrawGlyphRunList(
            const sktext::GlyphRunList& glyphRunList, const SkPaint& paint) override {
        sk_sp<SkTextBlob> cache;
        sk_sp<SkTextBlob> blob = glyphRunList.makeBlob();
        this->filterTextBlob(paint, blob.get(), &cache);
        if (!cache) {
            this->SkPaintFilterCanvas::onDrawGlyphRunList(glyphRunList, paint);
            return;
        }
        sktext::GlyphRunBuilder builder;
        const sktext::GlyphRunList& filtered =
                builder.blobToGlyphRunList(*cache, glyphRunList.origin());
        this->SkPaintFilterCanvas::onDrawGlyphRunList(filtered, paint);
    }

    bool filterFont(SkTCopyOnFirstWrite<SkFont>* font) const {
        if (fFontOverrides->fTypeface) {
            font->writable()->setTypeface(fFont->refTypeface());
        }
        if (fFontOverrides->fSize) {
            font->writable()->setSize(fFont->getSize());
        }
        if (fFontOverrides->fScaleX) {
            font->writable()->setScaleX(fFont->getScaleX());
        }
        if (fFontOverrides->fSkewX) {
            font->writable()->setSkewX(fFont->getSkewX());
        }
        if (fFontOverrides->fHinting) {
            font->writable()->setHinting(fFont->getHinting());
        }
        if (fFontOverrides->fEdging) {
            font->writable()->setEdging(fFont->getEdging());
        }
        if (fFontOverrides->fSubpixel) {
            font->writable()->setSubpixel(fFont->isSubpixel());
        }
        if (fFontOverrides->fForceAutoHinting) {
            font->writable()->setForceAutoHinting(fFont->isForceAutoHinting());
        }
        if (fFontOverrides->fEmbeddedBitmaps) {
            font->writable()->setEmbeddedBitmaps(fFont->isEmbeddedBitmaps());
        }
        if (fFontOverrides->fLinearMetrics) {
            font->writable()->setLinearMetrics(fFont->isLinearMetrics());
        }
        if (fFontOverrides->fEmbolden) {
            font->writable()->setEmbolden(fFont->isEmbolden());
        }
        if (fFontOverrides->fBaselineSnap) {
            font->writable()->setBaselineSnap(fFont->isBaselineSnap());
        }

        return true; // we, currently, never elide a draw
    }

    bool onFilter(SkPaint& paint) const override {
        if (fPaintOverrides->fPathEffect) {
            paint.setPathEffect(fPaint->refPathEffect());
        }
        if (fPaintOverrides->fShader) {
            paint.setShader(fPaint->refShader());
        }
        if (fPaintOverrides->fMaskFilter) {
            paint.setMaskFilter(fPaint->refMaskFilter());
        }
        if (fPaintOverrides->fColorFilter) {
            paint.setColorFilter(fPaint->refColorFilter());
        }
        if (fPaintOverrides->fImageFilter) {
            paint.setImageFilter(fPaint->refImageFilter());
        }
        if (fPaintOverrides->fColor) {
            paint.setColor4f(fPaint->getColor4f());
        }
        if (fPaintOverrides->fStrokeWidth) {
            paint.setStrokeWidth(fPaint->getStrokeWidth());
        }
        if (fPaintOverrides->fMiterLimit) {
            paint.setStrokeMiter(fPaint->getStrokeMiter());
        }
        if (fPaintOverrides->fBlendMode) {
            paint.setBlendMode(fPaint->getBlendMode_or(SkBlendMode::kSrc));
        }
        if (fPaintOverrides->fAntiAlias) {
            paint.setAntiAlias(fPaint->isAntiAlias());
        }
        if (fPaintOverrides->fDither) {
            paint.setDither(fPaint->isDither());
        }
        if (fPaintOverrides->fForceRuntimeBlend) {
            if (std::optional<SkBlendMode> mode = paint.asBlendMode()) {
                paint.setBlender(GetRuntimeBlendForBlendMode(*mode));
            }
        }
        if (fPaintOverrides->fCapType) {
            paint.setStrokeCap(fPaint->getStrokeCap());
        }
        if (fPaintOverrides->fJoinType) {
            paint.setStrokeJoin(fPaint->getStrokeJoin());
        }
        if (fPaintOverrides->fStyle) {
            paint.setStyle(fPaint->getStyle());
        }
        return true; // we, currently, never elide a draw
    }
    SkPaint* fPaint;
    Viewer::SkPaintFields* fPaintOverrides;
    SkFont* fFont;
    Viewer::SkFontFields* fFontOverrides;
};

static SkSerialProcs serial_procs_using_png() {
    SkSerialProcs sProcs;
    sProcs.fImageProc = [](SkImage* img, void*) -> sk_sp<SkData> {
        return SkPngEncoder::Encode(as_IB(img)->directContext(), img, SkPngEncoder::Options{});
    };
    return sProcs;
}

void Viewer::drawSlide(SkSurface* surface) {
    if (fCurrentSlide < 0) {
        return;
    }

    SkAutoCanvasRestore autorestore(surface->getCanvas(), false);

    // By default, we render directly into the window's surface/canvas
    SkSurface* slideSurface = surface;
    SkCanvas* slideCanvas = surface->getCanvas();
    fLastImage.reset();

    // If we're in any of the color managed modes, construct the color space we're going to use
    sk_sp<SkColorSpace> colorSpace = nullptr;
    if (ColorMode::kLegacy != fColorMode) {
        skcms_Matrix3x3 toXYZ;
        SkAssertResult(fColorSpacePrimaries.toXYZD50(&toXYZ));
        colorSpace = SkColorSpace::MakeRGB(fColorSpaceTransferFn, toXYZ);
    }

    if (fSaveToSKP) {
        SkPictureRecorder recorder;
        SkCanvas* recorderCanvas = recorder.beginRecording(SkRect::Make(this->currentSlideSize()));
        fSlides[fCurrentSlide]->draw(recorderCanvas);
        sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());
        SkFILEWStream stream("sample_app.skp");
        SkSerialProcs sProcs = serial_procs_using_png();
        picture->serialize(&stream, &sProcs);
        fSaveToSKP = false;
    }

    // Grab some things we'll need to make surfaces (for tiling or general offscreen rendering)
    SkColorType colorType;
    switch (fColorMode) {
        case ColorMode::kLegacy:
        case ColorMode::kColorManaged8888:
            colorType = kN32_SkColorType;
            break;
        case ColorMode::kColorManagedF16:
            colorType = kRGBA_F16_SkColorType;
            break;
        case ColorMode::kColorManagedF16Norm:
            colorType = kRGBA_F16Norm_SkColorType;
            break;
    }

    // We need to render offscreen if we're...
    // ... in fake perspective or zooming (so we have a snapped copy of the results)
    // ... in any raster mode, because the window surface is actually GL
    // ... in any color managed mode, because we always make the window surface with no color space
    // ... or if the user explicitly requested offscreen rendering
    sk_sp<SkSurface> offscreenSurface = nullptr;
    if (kPerspective_Fake == fPerspectiveMode ||
        fShowZoomWindow ||
        fShowHistogramWindow ||
        Window::kRaster_BackendType == fBackendType ||
        colorSpace != nullptr ||
        FLAGS_offscreen) {
        SkSurfaceProps props(fWindow->getRequestedDisplayParams().fSurfaceProps);
        slideCanvas->getProps(&props);

        SkImageInfo info = SkImageInfo::Make(
                fWindow->width(), fWindow->height(), colorType, kPremul_SkAlphaType, colorSpace);
        offscreenSurface = Window::kRaster_BackendType == this->fBackendType
                                   ? SkSurfaces::Raster(info, &props)
                                   : slideCanvas->makeSurface(info, &props);

        slideSurface = offscreenSurface.get();
        slideCanvas = offscreenSurface->getCanvas();
    }

    SkPictureRecorder recorder;
    SkCanvas* recorderRestoreCanvas = nullptr;
    if (fDrawViaSerialize) {
        recorderRestoreCanvas = slideCanvas;
        slideCanvas = recorder.beginRecording(SkRect::Make(this->currentSlideSize()));
    }

    int count = slideCanvas->save();
    slideCanvas->clear(SK_ColorWHITE);
    // Time the painting logic of the slide
    fStatsLayer.beginTiming(fPaintTimer);
    if (fTiled) {
        int tileW = SkScalarCeilToInt(fWindow->width() * fTileScale.width());
        int tileH = SkScalarCeilToInt(fWindow->height() * fTileScale.height());
        for (int y = 0; y < fWindow->height(); y += tileH) {
            for (int x = 0; x < fWindow->width(); x += tileW) {
                SkAutoCanvasRestore acr(slideCanvas, true);
                slideCanvas->clipRect(SkRect::MakeXYWH(x, y, tileW, tileH));
                fSlides[fCurrentSlide]->draw(slideCanvas);
            }
        }

        // Draw borders between tiles
        if (fDrawTileBoundaries) {
            SkPaint border;
            border.setColor(0x60FF00FF);
            border.setStyle(SkPaint::kStroke_Style);
            for (int y = 0; y < fWindow->height(); y += tileH) {
                for (int x = 0; x < fWindow->width(); x += tileW) {
                    slideCanvas->drawRect(SkRect::MakeXYWH(x, y, tileW, tileH), border);
                }
            }
        }
    } else {
        slideCanvas->concat(this->computeMatrix());
        if (kPerspective_Real == fPerspectiveMode) {
            slideCanvas->clipRect(SkRect::MakeWH(fWindow->width(), fWindow->height()));
        }
        if (fPaintOverrides.overridesSomething() || fFontOverrides.overridesSomething()) {
            OveridePaintFilterCanvas filterCanvas(slideCanvas,
                                                  &fPaint, &fPaintOverrides,
                                                  &fFont, &fFontOverrides);
            fSlides[fCurrentSlide]->draw(&filterCanvas);
        } else {
            fSlides[fCurrentSlide]->draw(slideCanvas);
        }
    }
    fStatsLayer.endTiming(fPaintTimer);
    slideCanvas->restoreToCount(count);

    if (recorderRestoreCanvas) {
        sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());
        SkSerialProcs sProcs = serial_procs_using_png();
        auto data = picture->serialize(&sProcs);
        slideCanvas = recorderRestoreCanvas;
        slideCanvas->drawPicture(SkPicture::MakeFromData(data.get()));
    }

    // Force a flush so we can time that, too
    fStatsLayer.beginTiming(fFlushTimer);
    skgpu::FlushAndSubmit(slideSurface);
    fStatsLayer.endTiming(fFlushTimer);

    // If we rendered offscreen, snap an image and push the results to the window's canvas
    if (offscreenSurface) {
        fLastImage = offscreenSurface->makeImageSnapshot();

        SkCanvas* canvas = surface->getCanvas();
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);
        SkSamplingOptions sampling;
        int prePerspectiveCount = canvas->save();
        if (kPerspective_Fake == fPerspectiveMode) {
            sampling = SkSamplingOptions({1.0f/3, 1.0f/3});
            canvas->clear(SK_ColorWHITE);
            canvas->concat(this->computePerspectiveMatrix());
        }
        canvas->drawImage(fLastImage, 0, 0, sampling, &paint);
        canvas->restoreToCount(prePerspectiveCount);
    }

    if (fShowSlideDimensions) {
        SkCanvas* canvas = surface->getCanvas();
        SkAutoCanvasRestore acr(canvas, true);
        canvas->concat(this->computeMatrix());
        SkRect r = SkRect::Make(this->currentSlideSize());
        SkPaint paint;
        paint.setColor(0x40FFFF00);
        canvas->drawRect(r, paint);
    }
}

void Viewer::onBackendCreated() {
    this->setupCurrentSlide();
    fWindow->show();
}

void Viewer::onPaint(SkSurface* surface) {
    this->drawSlide(surface);

    fCommands.drawHelp(surface->getCanvas());

    this->drawImGui();

    fLastImage.reset();

    if (auto direct = fWindow->directContext()) {
        // Clean out cache items that haven't been used in more than 10 seconds.
        direct->performDeferredCleanup(std::chrono::seconds(10));
    }
}

void Viewer::onResize(int width, int height) {
    if (fCurrentSlide >= 0) {
        SkScalar scaleFactor = 1.0;
        if (fApplyBackingScale) {
            scaleFactor = fWindow->scaleFactor();
        }
        fSlides[fCurrentSlide]->resize(width / scaleFactor, height / scaleFactor);
    }
}

SkPoint Viewer::mapEvent(float x, float y) {
    const auto m = this->computeMatrix();
    SkMatrix inv;

    SkAssertResult(m.invert(&inv));

    return inv.mapXY(x, y);
}

bool Viewer::onTouch(intptr_t owner, skui::InputState state, float x, float y) {
    if (GestureDevice::kMouse == fGestureDevice) {
        return false;
    }

    const auto slidePt = this->mapEvent(x, y);
    if (fSlides[fCurrentSlide]->onMouse(slidePt.x(), slidePt.y(), state, skui::ModifierKey::kNone)) {
        fWindow->inval();
        return true;
    }

    void* castedOwner = reinterpret_cast<void*>(owner);
    switch (state) {
        case skui::InputState::kUp: {
            fGesture.touchEnd(castedOwner);
#if defined(SK_BUILD_FOR_IOS)
            // TODO: move IOS swipe detection higher up into the platform code
            SkPoint dir;
            if (fGesture.isFling(&dir)) {
                // swiping left or right
                if (SkTAbs(dir.fX) > SkTAbs(dir.fY)) {
                    if (dir.fX < 0) {
                        this->setCurrentSlide(fCurrentSlide < fSlides.size() - 1 ?
                                              fCurrentSlide + 1 : 0);
                    } else {
                        this->setCurrentSlide(fCurrentSlide > 0 ?
                                              fCurrentSlide - 1 : fSlides.size() - 1);
                    }
                }
                fGesture.reset();
            }
#endif
            break;
        }
        case skui::InputState::kDown: {
            fGesture.touchBegin(castedOwner, x, y);
            break;
        }
        case skui::InputState::kMove: {
            fGesture.touchMoved(castedOwner, x, y);
            break;
        }
        default: {
            // kLeft and kRight are only for swipes
            SkASSERT(false);
            break;
        }
    }
    fGestureDevice = fGesture.isBeingTouched() ? GestureDevice::kTouch : GestureDevice::kNone;
    fWindow->inval();
    return true;
}

bool Viewer::onMouse(int x, int y, skui::InputState state, skui::ModifierKey modifiers) {
    if (GestureDevice::kTouch == fGestureDevice) {
        return false;
    }

    const auto slidePt = this->mapEvent(x, y);
    if (fSlides[fCurrentSlide]->onMouse(slidePt.x(), slidePt.y(), state, modifiers)) {
        fWindow->inval();
        return true;
    }

    switch (state) {
        case skui::InputState::kUp: {
            fGesture.touchEnd(nullptr);
            break;
        }
        case skui::InputState::kDown: {
            fGesture.touchBegin(nullptr, x, y);
            break;
        }
        case skui::InputState::kMove: {
            fGesture.touchMoved(nullptr, x, y);
            break;
        }
        default: {
            SkASSERT(false); // shouldn't see kRight or kLeft here
            break;
        }
    }
    fGestureDevice = fGesture.isBeingTouched() ? GestureDevice::kMouse : GestureDevice::kNone;

    if (state != skui::InputState::kMove || fGesture.isBeingTouched()) {
        fWindow->inval();
    }
    return true;
}

bool Viewer::onMouseWheel(float delta, int x, int y, skui::ModifierKey) {
    // Rather than updating the fixed zoom level, treat a mouse wheel event as a gesture, which
    // applies a pre- and post-translation to the transform, resulting in a zoom effect centered at
    // the mouse cursor position.
    SkScalar scale = exp(delta * 0.001);
    fGesture.startZoom();
    fGesture.updateZoom(scale, x, y, x, y);
    fGesture.endZoom();
    fWindow->inval();
    return true;
}

bool Viewer::onFling(skui::InputState state) {
    if (skui::InputState::kRight == state) {
        this->setCurrentSlide(fCurrentSlide > 0 ? fCurrentSlide - 1 : fSlides.size() - 1);
        return true;
    } else if (skui::InputState::kLeft == state) {
        this->setCurrentSlide(fCurrentSlide < fSlides.size() - 1 ? fCurrentSlide + 1 : 0);
        return true;
    }
    return false;
}

bool Viewer::onPinch(skui::InputState state, float scale, float x, float y) {
    switch (state) {
        case skui::InputState::kDown:
            fGesture.startZoom();
            return true;
        case skui::InputState::kMove:
            fGesture.updateZoom(scale, x, y, x, y);
            return true;
        case skui::InputState::kUp:
            fGesture.endZoom();
            return true;
        default:
            SkASSERT(false);
            break;
    }

    return false;
}

static void ImGui_Primaries(SkColorSpacePrimaries* primaries, SkPaint* gamutPaint) {
    // The gamut image covers a (0.8 x 0.9) shaped region
    ImGui::DragCanvas dc(primaries, { 0.0f, 0.9f }, { 0.8f, 0.0f });

    // Background image. Only draw a subset of the image, to avoid the regions less than zero.
    // Simplifes re-mapping math, clipping behavior, and increases resolution in the useful area.
    // Magic numbers are pixel locations of the origin and upper-right corner.
    dc.fDrawList->AddImage(gamutPaint, dc.fPos,
                           ImVec2(dc.fPos.x + dc.fSize.x, dc.fPos.y + dc.fSize.y),
                           ImVec2(242, 61), ImVec2(1897, 1922));

    dc.dragPoint((SkPoint*)(&primaries->fRX), true, 0xFF000040);
    dc.dragPoint((SkPoint*)(&primaries->fGX), true, 0xFF004000);
    dc.dragPoint((SkPoint*)(&primaries->fBX), true, 0xFF400000);
    dc.dragPoint((SkPoint*)(&primaries->fWX), true);
    dc.fDrawList->AddPolyline(dc.fScreenPoints.begin(), 3, 0xFFFFFFFF, true, 1.5f);
}

static bool ImGui_DragLocation(SkPoint* pt) {
    ImGui::DragCanvas dc(pt);
    dc.fillColor(IM_COL32(0, 0, 0, 128));
    dc.dragPoint(pt);
    return dc.fDragging;
}

static bool ImGui_DragQuad(SkPoint* pts) {
    ImGui::DragCanvas dc(pts);
    dc.fillColor(IM_COL32(0, 0, 0, 128));

    for (int i = 0; i < 4; ++i) {
        dc.dragPoint(pts + i);
    }

    dc.fDrawList->AddLine(dc.fScreenPoints[0], dc.fScreenPoints[1], 0xFFFFFFFF);
    dc.fDrawList->AddLine(dc.fScreenPoints[1], dc.fScreenPoints[3], 0xFFFFFFFF);
    dc.fDrawList->AddLine(dc.fScreenPoints[3], dc.fScreenPoints[2], 0xFFFFFFFF);
    dc.fDrawList->AddLine(dc.fScreenPoints[2], dc.fScreenPoints[0], 0xFFFFFFFF);

    return dc.fDragging;
}

static std::string build_sksl_highlight_shader() {
    return std::string("void main() { sk_FragColor = half4(1, 0, 1, 0.5); }");
}

static std::string build_metal_highlight_shader(const std::string& inShader) {
    // Metal fragment shaders need a lot of non-trivial boilerplate that we don't want to recompute
    // here. So keep all shader code, but right before `return *_out;`, swap out the sk_FragColor.
    size_t pos = inShader.rfind("return _out;\n");
    if (pos == std::string::npos) {
        return inShader;
    }

    std::string replacementShader = inShader;
    replacementShader.insert(pos, "_out.sk_FragColor = float4(1.0, 0.0, 1.0, 0.5); ");
    return replacementShader;
}

static std::string build_glsl_highlight_shader(const GrShaderCaps& shaderCaps) {
    const char* versionDecl = shaderCaps.fVersionDeclString;
    std::string highlight = versionDecl ? versionDecl : "";
    if (shaderCaps.fUsesPrecisionModifiers) {
        highlight.append("precision mediump float;\n");
    }
    SkSL::String::appendf(&highlight, "out vec4 sk_FragColor;\n"
                                      "void main() { sk_FragColor = vec4(1, 0, 1, 0.5); }");
    return highlight;
}

void Viewer::drawImGui() {
    // Support drawing the ImGui demo window. Superfluous, but gives a good idea of what's possible
    if (fShowImGuiTestWindow) {
        ImGui::ShowDemoWindow(&fShowImGuiTestWindow);
    }

    if (fShowImGuiDebugWindow) {
        // We have some dynamic content that sizes to fill available size. If the scroll bar isn't
        // always visible, we can end up in a layout feedback loop.
        ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
        DisplayParams params = fWindow->getRequestedDisplayParams();
        bool displayParamsChanged = false; // heavy-weight, might recreate entire context
        bool uiParamsChanged = false;      // light weight, just triggers window invalidation
        GrDirectContext* ctx = fWindow->directContext();

        if (ImGui::Begin("Tools", &fShowImGuiDebugWindow,
                         ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
            if (ImGui::CollapsingHeader("Backend")) {
                int newBackend = static_cast<int>(fBackendType);
                ImGui::RadioButton("Raster", &newBackend, sk_app::Window::kRaster_BackendType);
                ImGui::SameLine();
                ImGui::RadioButton("OpenGL", &newBackend, sk_app::Window::kNativeGL_BackendType);
#if SK_ANGLE && (defined(SK_BUILD_FOR_WIN) || defined(SK_BUILD_FOR_MAC))
                ImGui::SameLine();
                ImGui::RadioButton("ANGLE", &newBackend, sk_app::Window::kANGLE_BackendType);
#endif
#if defined(SK_DAWN)
#if defined(SK_GRAPHITE)
                ImGui::SameLine();
                ImGui::RadioButton("Dawn (Graphite)", &newBackend,
                                   sk_app::Window::kGraphiteDawn_BackendType);
#endif
#endif
#if defined(SK_VULKAN) && !defined(SK_BUILD_FOR_MAC)
                ImGui::SameLine();
                ImGui::RadioButton("Vulkan", &newBackend, sk_app::Window::kVulkan_BackendType);
#if defined(SK_GRAPHITE)
                ImGui::SameLine();
                ImGui::RadioButton("Vulkan (Graphite)", &newBackend,
                                   sk_app::Window::kGraphiteVulkan_BackendType);
#endif
#endif
#if defined(SK_METAL)
                ImGui::SameLine();
                ImGui::RadioButton("Metal", &newBackend, sk_app::Window::kMetal_BackendType);
#if defined(SK_GRAPHITE)
                ImGui::SameLine();
                ImGui::RadioButton("Metal (Graphite)", &newBackend,
                                   sk_app::Window::kGraphiteMetal_BackendType);
#endif
#endif
#if defined(SK_DIRECT3D)
                ImGui::SameLine();
                ImGui::RadioButton("Direct3D", &newBackend, sk_app::Window::kDirect3D_BackendType);
#endif
                if (newBackend != fBackendType) {
                    fDeferredActions.push_back([newBackend, this]() {
                        this->setBackend(static_cast<sk_app::Window::BackendType>(newBackend));
                    });
                }

                if (ctx) {
                    bool* wire = &params.fGrContextOptions.fWireframeMode;
                    if (ImGui::Checkbox("Wireframe Mode", wire)) {
                        displayParamsChanged = true;
                    }

                    bool* reducedShaders = &params.fGrContextOptions.fReducedShaderVariations;
                    if (ImGui::Checkbox("Reduced shaders", reducedShaders)) {
                        displayParamsChanged = true;
                    }

                    // Determine the context's max sample count for MSAA radio buttons.
                    int sampleCount = fWindow->sampleCount();
                    int maxMSAA = (fBackendType != sk_app::Window::kRaster_BackendType) ?
                            ctx->maxSurfaceSampleCountForColorType(kRGBA_8888_SkColorType) :
                            1;

                    // Only display the MSAA radio buttons when there are options above 1x MSAA.
                    if (maxMSAA >= 4) {
                        ImGui::Text("MSAA: ");

                        for (int curMSAA = 1; curMSAA <= maxMSAA; curMSAA *= 2) {
                            // 2x MSAA works, but doesn't offer much of a visual improvement, so we
                            // don't show it in the list.
                            if (curMSAA == 2) {
                                continue;
                            }
                            ImGui::SameLine();
                            ImGui::RadioButton(SkStringPrintf("%d", curMSAA).c_str(),
                                               &sampleCount, curMSAA);
                        }
                    }

                    if (sampleCount != params.fMSAASampleCount) {
                        params.fMSAASampleCount = sampleCount;
                        displayParamsChanged = true;
                    }
                }

                int pixelGeometryIdx = 0;
                if (fDisplayOverrides.fSurfaceProps.fPixelGeometry) {
                    pixelGeometryIdx = params.fSurfaceProps.pixelGeometry() + 1;
                }
                if (ImGui::Combo("Pixel Geometry", &pixelGeometryIdx,
                                 "Default\0Flat\0RGB\0BGR\0RGBV\0BGRV\0\0"))
                {
                    uint32_t flags = params.fSurfaceProps.flags();
                    if (pixelGeometryIdx == 0) {
                        fDisplayOverrides.fSurfaceProps.fPixelGeometry = false;
                        SkPixelGeometry pixelGeometry = fDisplay.fSurfaceProps.pixelGeometry();
                        params.fSurfaceProps = SkSurfaceProps(flags, pixelGeometry);
                    } else {
                        fDisplayOverrides.fSurfaceProps.fPixelGeometry = true;
                        SkPixelGeometry pixelGeometry = SkTo<SkPixelGeometry>(pixelGeometryIdx - 1);
                        params.fSurfaceProps = SkSurfaceProps(flags, pixelGeometry);
                    }
                    displayParamsChanged = true;
                }

                bool useDFT = params.fSurfaceProps.isUseDeviceIndependentFonts();
                if (ImGui::Checkbox("DFT", &useDFT)) {
                    uint32_t flags = params.fSurfaceProps.flags();
                    if (useDFT) {
                        flags |= SkSurfaceProps::kUseDeviceIndependentFonts_Flag;
                    } else {
                        flags &= ~SkSurfaceProps::kUseDeviceIndependentFonts_Flag;
                    }
                    SkPixelGeometry pixelGeometry = params.fSurfaceProps.pixelGeometry();
                    params.fSurfaceProps = SkSurfaceProps(flags, pixelGeometry);
                    displayParamsChanged = true;
                }

                if (ImGui::TreeNode("Path Renderers")) {
                    skgpu::graphite::Context* gctx = fWindow->graphiteContext();
                    if (is_graphite_backend_type(fBackendType) && gctx) {
#if defined(SK_GRAPHITE)
                        using skgpu::graphite::PathRendererStrategy;
                        skgpu::graphite::ContextOptionsPriv* opts =
                                &params.fGraphiteContextOptions.fPriv;
                        auto prevPrs = opts->fPathRendererStrategy;
                        auto prsButton = [&](skgpu::graphite::PathRendererStrategy s) {
                            if (ImGui::RadioButton(get_path_renderer_strategy_string(s),
                                                   prevPrs == s)) {
                                if (s != opts->fPathRendererStrategy) {
                                    opts->fPathRendererStrategy = s;
                                    displayParamsChanged = true;
                                }
                            }
                        };

                        prsButton(PathRendererStrategy::kDefault);

                        PathRendererStrategy strategies[] = {
                                PathRendererStrategy::kComputeAnalyticAA,
                                PathRendererStrategy::kComputeMSAA16,
                                PathRendererStrategy::kComputeMSAA8,
                                PathRendererStrategy::kRasterAA,
                                PathRendererStrategy::kTessellation,
                        };
                        for (size_t i = 0; i < std::size(strategies); ++i) {
                            if (gctx->priv().supportsPathRendererStrategy(strategies[i])) {
                                prsButton(strategies[i]);
                            }
                        }
#endif
                    } else if (ctx) {
                        GpuPathRenderers prevPr = params.fGrContextOptions.fGpuPathRenderers;
                        auto prButton = [&](GpuPathRenderers x) {
                            if (ImGui::RadioButton(gGaneshPathRendererNames[x].c_str(),
                                                   prevPr == x)) {
                                if (x != params.fGrContextOptions.fGpuPathRenderers) {
                                    params.fGrContextOptions.fGpuPathRenderers = x;
                                    displayParamsChanged = true;
                                }
                            }
                        };

                        prButton(GpuPathRenderers::kDefault);
#if defined(SK_GANESH)
                        if (fWindow->sampleCount() > 1 || FLAGS_dmsaa) {
                            const auto* caps = ctx->priv().caps();
                            if (skgpu::ganesh::AtlasPathRenderer::IsSupported(ctx)) {
                                prButton(GpuPathRenderers::kAtlas);
                            }
                            if (skgpu::ganesh::TessellationPathRenderer::IsSupported(*caps)) {
                                prButton(GpuPathRenderers::kTessellation);
                            }
                        }
#endif
                        if (1 == fWindow->sampleCount()) {
                            prButton(GpuPathRenderers::kSmall);
                        }
                        prButton(GpuPathRenderers::kTriangulating);
                        prButton(GpuPathRenderers::kNone);
                    } else {
                        ImGui::RadioButton("Software", true);
                    }
                    ImGui::TreePop();
                }
            }

            if (ImGui::CollapsingHeader("Tiling")) {
                ImGui::Checkbox("Enable", &fTiled);
                ImGui::Checkbox("Draw Boundaries", &fDrawTileBoundaries);
                ImGui::SliderFloat("Horizontal", &fTileScale.fWidth, 0.1f, 1.0f);
                ImGui::SliderFloat("Vertical", &fTileScale.fHeight, 0.1f, 1.0f);
            }

            if (ImGui::CollapsingHeader("Transform")) {
                if (ImGui::Checkbox("Apply Backing Scale", &fApplyBackingScale)) {
                    this->preTouchMatrixChanged();
                    this->onResize(fWindow->width(), fWindow->height());
                    // This changes how we manipulate the canvas transform, it's not changing the
                    // window's actual parameters.
                    uiParamsChanged = true;
                }

                float zoom = fZoomLevel;
                if (ImGui::SliderFloat("Zoom", &zoom, MIN_ZOOM_LEVEL, MAX_ZOOM_LEVEL)) {
                    fZoomLevel = zoom;
                    this->preTouchMatrixChanged();
                    uiParamsChanged = true;
                }
                float deg = fRotation;
                if (ImGui::SliderFloat("Rotate", &deg, -30, 360, "%.3f deg")) {
                    fRotation = deg;
                    this->preTouchMatrixChanged();
                    uiParamsChanged = true;
                }
                if (ImGui::CollapsingHeader("Subpixel offset", ImGuiTreeNodeFlags_NoTreePushOnOpen)) {
                    if (ImGui_DragLocation(&fOffset)) {
                        this->preTouchMatrixChanged();
                        uiParamsChanged = true;
                    }
                } else if (fOffset != SkVector{0.5f, 0.5f}) {
                    this->preTouchMatrixChanged();
                    uiParamsChanged = true;
                    fOffset = {0.5f, 0.5f};
                }
                int perspectiveMode = static_cast<int>(fPerspectiveMode);
                if (ImGui::Combo("Perspective", &perspectiveMode, "Off\0Real\0Fake\0\0")) {
                    fPerspectiveMode = static_cast<PerspectiveMode>(perspectiveMode);
                    this->preTouchMatrixChanged();
                    uiParamsChanged = true;
                }
                if (perspectiveMode != kPerspective_Off && ImGui_DragQuad(fPerspectivePoints)) {
                    this->preTouchMatrixChanged();
                    uiParamsChanged = true;
                }
            }

            if (ImGui::CollapsingHeader("Paint")) {
                auto paintFlag = [this, &uiParamsChanged](const char* label, const char* items,
                                                          bool SkPaintFields::* flag,
                                                          bool (SkPaint::* isFlag)() const,
                                                          void (SkPaint::* setFlag)(bool) )
                {
                    int itemIndex = 0;
                    if (fPaintOverrides.*flag) {
                        itemIndex = (fPaint.*isFlag)() ? 2 : 1;
                    }
                    if (ImGui::Combo(label, &itemIndex, items)) {
                        if (itemIndex == 0) {
                            fPaintOverrides.*flag = false;
                        } else {
                            fPaintOverrides.*flag = true;
                            (fPaint.*setFlag)(itemIndex == 2);
                        }
                        uiParamsChanged = true;
                    }
                };

                paintFlag("Antialias",
                          "Default\0No AA\0AA\0\0",
                          &SkPaintFields::fAntiAlias,
                          &SkPaint::isAntiAlias, &SkPaint::setAntiAlias);

                paintFlag("Dither",
                          "Default\0No Dither\0Dither\0\0",
                          &SkPaintFields::fDither,
                          &SkPaint::isDither, &SkPaint::setDither);

                int styleIdx = 0;
                if (fPaintOverrides.fStyle) {
                    styleIdx = SkTo<int>(fPaint.getStyle()) + 1;
                }
                if (ImGui::Combo("Style", &styleIdx,
                                 "Default\0Fill\0Stroke\0Stroke and Fill\0\0"))
                {
                    if (styleIdx == 0) {
                        fPaintOverrides.fStyle = false;
                        fPaint.setStyle(SkPaint::kFill_Style);
                    } else {
                        fPaint.setStyle(SkTo<SkPaint::Style>(styleIdx - 1));
                        fPaintOverrides.fStyle = true;
                    }
                    uiParamsChanged = true;
                }

                ImGui::Checkbox("Force Runtime Blends", &fPaintOverrides.fForceRuntimeBlend);

                ImGui::Checkbox("Override Stroke Width", &fPaintOverrides.fStrokeWidth);
                if (fPaintOverrides.fStrokeWidth) {
                    float width = fPaint.getStrokeWidth();
                    if (ImGui::SliderFloat("Stroke Width", &width, 0, 20)) {
                        fPaint.setStrokeWidth(width);
                        uiParamsChanged = true;
                    }
                }

                ImGui::Checkbox("Override Miter Limit", &fPaintOverrides.fMiterLimit);
                if (fPaintOverrides.fMiterLimit) {
                    float miterLimit = fPaint.getStrokeMiter();
                    if (ImGui::SliderFloat("Miter Limit", &miterLimit, 0, 20)) {
                        fPaint.setStrokeMiter(miterLimit);
                        uiParamsChanged = true;
                    }
                }

                int capIdx = 0;
                if (fPaintOverrides.fCapType) {
                    capIdx = SkTo<int>(fPaint.getStrokeCap()) + 1;
                }
                if (ImGui::Combo("Cap Type", &capIdx,
                                 "Default\0Butt\0Round\0Square\0\0"))
                {
                    if (capIdx == 0) {
                        fPaintOverrides.fCapType = false;
                        fPaint.setStrokeCap(SkPaint::kDefault_Cap);
                    } else {
                        fPaint.setStrokeCap(SkTo<SkPaint::Cap>(capIdx - 1));
                        fPaintOverrides.fCapType = true;
                    }
                    uiParamsChanged = true;
                }

                int joinIdx = 0;
                if (fPaintOverrides.fJoinType) {
                    joinIdx = SkTo<int>(fPaint.getStrokeJoin()) + 1;
                }
                if (ImGui::Combo("Join Type", &joinIdx,
                                 "Default\0Miter\0Round\0Bevel\0\0"))
                {
                    if (joinIdx == 0) {
                        fPaintOverrides.fJoinType = false;
                        fPaint.setStrokeJoin(SkPaint::kDefault_Join);
                    } else {
                        fPaint.setStrokeJoin(SkTo<SkPaint::Join>(joinIdx - 1));
                        fPaintOverrides.fJoinType = true;
                    }
                    uiParamsChanged = true;
                }
            }

            if (ImGui::CollapsingHeader("Font")) {
                int hintingIdx = 0;
                if (fFontOverrides.fHinting) {
                    hintingIdx = SkTo<int>(fFont.getHinting()) + 1;
                }
                if (ImGui::Combo("Hinting", &hintingIdx,
                                 "Default\0None\0Slight\0Normal\0Full\0\0"))
                {
                    if (hintingIdx == 0) {
                        fFontOverrides.fHinting = false;
                        fFont.setHinting(SkFontHinting::kNone);
                    } else {
                        fFont.setHinting(SkTo<SkFontHinting>(hintingIdx - 1));
                        fFontOverrides.fHinting = true;
                    }
                    uiParamsChanged = true;
                }

                auto fontFlag = [this, &uiParamsChanged](const char* label, const char* items,
                                                        bool SkFontFields::* flag,
                                                        bool (SkFont::* isFlag)() const,
                                                        void (SkFont::* setFlag)(bool) )
                {
                    int itemIndex = 0;
                    if (fFontOverrides.*flag) {
                        itemIndex = (fFont.*isFlag)() ? 2 : 1;
                    }
                    if (ImGui::Combo(label, &itemIndex, items)) {
                        if (itemIndex == 0) {
                            fFontOverrides.*flag = false;
                        } else {
                            fFontOverrides.*flag = true;
                            (fFont.*setFlag)(itemIndex == 2);
                        }
                        uiParamsChanged = true;
                    }
                };

                fontFlag("Fake Bold Glyphs",
                         "Default\0No Fake Bold\0Fake Bold\0\0",
                         &SkFontFields::fEmbolden,
                         &SkFont::isEmbolden, &SkFont::setEmbolden);

                fontFlag("Baseline Snapping",
                         "Default\0No Baseline Snapping\0Baseline Snapping\0\0",
                         &SkFontFields::fBaselineSnap,
                         &SkFont::isBaselineSnap, &SkFont::setBaselineSnap);

                fontFlag("Linear Text",
                         "Default\0No Linear Text\0Linear Text\0\0",
                         &SkFontFields::fLinearMetrics,
                         &SkFont::isLinearMetrics, &SkFont::setLinearMetrics);

                fontFlag("Subpixel Position Glyphs",
                         "Default\0Pixel Text\0Subpixel Text\0\0",
                         &SkFontFields::fSubpixel,
                         &SkFont::isSubpixel, &SkFont::setSubpixel);

                fontFlag("Embedded Bitmap Text",
                         "Default\0No Embedded Bitmaps\0Embedded Bitmaps\0\0",
                         &SkFontFields::fEmbeddedBitmaps,
                         &SkFont::isEmbeddedBitmaps, &SkFont::setEmbeddedBitmaps);

                fontFlag("Force Auto-Hinting",
                         "Default\0No Force Auto-Hinting\0Force Auto-Hinting\0\0",
                         &SkFontFields::fForceAutoHinting,
                         &SkFont::isForceAutoHinting, &SkFont::setForceAutoHinting);

                int edgingIdx = 0;
                if (fFontOverrides.fEdging) {
                    edgingIdx = SkTo<int>(fFont.getEdging()) + 1;
                }
                if (ImGui::Combo("Edging", &edgingIdx,
                                 "Default\0Alias\0Antialias\0Subpixel Antialias\0\0"))
                {
                    if (edgingIdx == 0) {
                        fFontOverrides.fEdging = false;
                        fFont.setEdging(SkFont::Edging::kAlias);
                    } else {
                        fFont.setEdging(SkTo<SkFont::Edging>(edgingIdx-1));
                        fFontOverrides.fEdging = true;
                    }
                    uiParamsChanged = true;
                }

                ImGui::Checkbox("Override Size", &fFontOverrides.fSize);
                if (fFontOverrides.fSize) {
                    ImGui::DragFloat2("TextRange", fFontOverrides.fSizeRange,
                                      0.001f, -10.0f, 300.0f, "%.6f", ImGuiSliderFlags_Logarithmic);
                    float textSize = fFont.getSize();
                    if (ImGui::DragFloat("TextSize", &textSize, 0.001f,
                                         fFontOverrides.fSizeRange[0],
                                         fFontOverrides.fSizeRange[1],
                                         "%.6f", ImGuiSliderFlags_Logarithmic))
                    {
                        fFont.setSize(textSize);
                        uiParamsChanged = true;
                    }
                }

                ImGui::Checkbox("Override ScaleX", &fFontOverrides.fScaleX);
                if (fFontOverrides.fScaleX) {
                    float scaleX = fFont.getScaleX();
                    if (ImGui::SliderFloat("ScaleX", &scaleX, MIN_ZOOM_LEVEL, MAX_ZOOM_LEVEL)) {
                        fFont.setScaleX(scaleX);
                        uiParamsChanged = true;
                    }
                }

                ImGui::Checkbox("Override SkewX", &fFontOverrides.fSkewX);
                if (fFontOverrides.fSkewX) {
                    float skewX = fFont.getSkewX();
                    if (ImGui::SliderFloat("SkewX", &skewX, MIN_ZOOM_LEVEL, MAX_ZOOM_LEVEL)) {
                        fFont.setSkewX(skewX);
                        uiParamsChanged = true;
                    }
                }
            }

            {
                SkMetaData controls;
                if (fSlides[fCurrentSlide]->onGetControls(&controls)) {
                    if (ImGui::CollapsingHeader("Current Slide")) {
                        SkMetaData::Iter iter(controls);
                        const char* name;
                        SkMetaData::Type type;
                        int count;
                        while ((name = iter.next(&type, &count)) != nullptr) {
                            if (type == SkMetaData::kScalar_Type) {
                                float val[3];
                                SkASSERT(count == 3);
                                controls.findScalars(name, &count, val);
                                if (ImGui::SliderFloat(name, &val[0], val[1], val[2])) {
                                    controls.setScalars(name, 3, val);
                                }
                            } else if (type == SkMetaData::kBool_Type) {
                                bool val;
                                SkASSERT(count == 1);
                                controls.findBool(name, &val);
                                if (ImGui::Checkbox(name, &val)) {
                                    controls.setBool(name, val);
                                }
                            }
                        }
                        fSlides[fCurrentSlide]->onSetControls(controls);
                    }
                }
            }

            if (fShowSlidePicker) {
                ImGui::SetNextTreeNodeOpen(true);
            }
            if (ImGui::CollapsingHeader("Slide")) {
                static ImGuiTextFilter filter;
                static ImVector<const char*> filteredSlideNames;
                static ImVector<int> filteredSlideIndices;

                if (fShowSlidePicker) {
                    ImGui::SetKeyboardFocusHere();
                    fShowSlidePicker = false;
                }

                filter.Draw();
                filteredSlideNames.clear();
                filteredSlideIndices.clear();
                int filteredIndex = 0;
                for (int i = 0; i < fSlides.size(); ++i) {
                    const char* slideName = fSlides[i]->getName().c_str();
                    if (filter.PassFilter(slideName) || i == fCurrentSlide) {
                        if (i == fCurrentSlide) {
                            filteredIndex = filteredSlideIndices.size();
                        }
                        filteredSlideNames.push_back(slideName);
                        filteredSlideIndices.push_back(i);
                    }
                }

                if (ImGui::ListBox("", &filteredIndex, filteredSlideNames.begin(),
                                   filteredSlideNames.size(), 20)) {
                    this->setCurrentSlide(filteredSlideIndices[filteredIndex]);
                }
            }

            if (ImGui::CollapsingHeader("Color Mode")) {
                ColorMode newMode = fColorMode;
                auto cmButton = [&](ColorMode mode, const char* label) {
                    if (ImGui::RadioButton(label, mode == fColorMode)) {
                        newMode = mode;
                    }
                };

                cmButton(ColorMode::kLegacy, "Legacy 8888");
                cmButton(ColorMode::kColorManaged8888, "Color Managed 8888");
                cmButton(ColorMode::kColorManagedF16, "Color Managed F16");
                cmButton(ColorMode::kColorManagedF16Norm, "Color Managed F16 Norm");

                if (newMode != fColorMode) {
                    this->setColorMode(newMode);
                }

                // Pick from common gamuts:
                int primariesIdx = 4; // Default: Custom
                for (size_t i = 0; i < std::size(gNamedPrimaries); ++i) {
                    if (primaries_equal(*gNamedPrimaries[i].fPrimaries, fColorSpacePrimaries)) {
                        primariesIdx = i;
                        break;
                    }
                }

                // Let user adjust the gamma
                ImGui::SliderFloat("Gamma", &fColorSpaceTransferFn.g, 0.5f, 3.5f);

                if (ImGui::Combo("Primaries", &primariesIdx,
                                 "sRGB\0AdobeRGB\0P3\0Rec. 2020\0Custom\0\0")) {
                    if (primariesIdx >= 0 && primariesIdx <= 3) {
                        fColorSpacePrimaries = *gNamedPrimaries[primariesIdx].fPrimaries;
                    }
                }

                if (ImGui::Button("Spin")) {
                    float rx = fColorSpacePrimaries.fRX,
                          ry = fColorSpacePrimaries.fRY;
                    fColorSpacePrimaries.fRX = fColorSpacePrimaries.fGX;
                    fColorSpacePrimaries.fRY = fColorSpacePrimaries.fGY;
                    fColorSpacePrimaries.fGX = fColorSpacePrimaries.fBX;
                    fColorSpacePrimaries.fGY = fColorSpacePrimaries.fBY;
                    fColorSpacePrimaries.fBX = rx;
                    fColorSpacePrimaries.fBY = ry;
                }

                // Allow direct editing of gamut
                ImGui_Primaries(&fColorSpacePrimaries, &fImGuiGamutPaint);
            }

            if (ImGui::CollapsingHeader("Animation")) {
                bool isPaused = AnimTimer::kPaused_State == fAnimTimer.state();
                if (ImGui::Checkbox("Pause", &isPaused)) {
                    fAnimTimer.togglePauseResume();
                }

                float speed = fAnimTimer.getSpeed();
                if (ImGui::DragFloat("Speed", &speed, 0.1f)) {
                    fAnimTimer.setSpeed(speed);
                }
            }

            if (ImGui::CollapsingHeader("Shaders")) {
                bool sksl = params.fGrContextOptions.fShaderCacheStrategy ==
                            GrContextOptions::ShaderCacheStrategy::kSkSL;

#if defined(SK_VULKAN)
                const bool isVulkan = fBackendType == sk_app::Window::kVulkan_BackendType;
#else
                const bool isVulkan = false;
#endif

                // To re-load shaders from the currently active programs, we flush all
                // caches on one frame, then set a flag to poll the cache on the next frame.
                static bool gLoadPending = false;
                if (gLoadPending) {
                    fCachedShaders.clear();

                    if (ctx) {
                        fPersistentCache.foreach([this](sk_sp<const SkData> key,
                                                        sk_sp<SkData> data,
                                                        const SkString& description,
                                                        int hitCount) {
                            CachedShader& entry(fCachedShaders.push_back());
                            entry.fKey = key;
                            SkMD5 hash;
                            hash.write(key->bytes(), key->size());
                            entry.fKeyString = hash.finish().toHexString();
                            entry.fKeyDescription = description;

                            SkReadBuffer reader(data->data(), data->size());
                            entry.fShaderType = GrPersistentCacheUtils::GetType(&reader);
                            GrPersistentCacheUtils::UnpackCachedShaders(&reader, entry.fShader,
                                                                        entry.fInterfaces,
                                                                        kGrShaderTypeCount);
                        });
                    }
#if defined(SK_GRAPHITE)
                    if (skgpu::graphite::Context* gctx = fWindow->graphiteContext()) {
                        int index = 1;
                        auto callback = [&](const skgpu::UniqueKey& key,
                                            const skgpu::graphite::GraphicsPipeline* pipeline) {
                            // Retrieve the shaders from the pipeline.
                            const skgpu::graphite::GraphicsPipeline::PipelineInfo& pipelineInfo =
                                    pipeline->getPipelineInfo();
                            const skgpu::graphite::ShaderCodeDictionary* dict =
                                    gctx->priv().shaderCodeDictionary();
                            skgpu::graphite::PaintParamsKey paintKey =
                                    dict->lookup(pipelineInfo.fPaintID);

                            CachedShader& entry(fCachedShaders.push_back());
                            entry.fKey = nullptr;
                            entry.fKeyString = SkStringPrintf("#%-3d RenderStep: %u, Paint: ",
                                                              index++,
                                                              pipelineInfo.fRenderStepID);
                            entry.fKeyString.append(paintKey.toString(dict));

                            if (sksl) {
                                entry.fShader[kVertex_GrShaderType] =
                                        pipelineInfo.fSkSLVertexShader;
                                entry.fShader[kFragment_GrShaderType] =
                                        pipelineInfo.fSkSLFragmentShader;
                                entry.fShaderType = SkSetFourByteTag('S', 'K', 'S', 'L');
                            } else {
                                entry.fShader[kVertex_GrShaderType] =
                                        pipelineInfo.fNativeVertexShader;
                                entry.fShader[kFragment_GrShaderType] =
                                        pipelineInfo.fNativeFragmentShader;
                                // We could derive the shader type from the GraphicsPipeline's type
                                // if there is ever a need to.
                                entry.fShaderType = SkSetFourByteTag('?', '?', '?', '?');
                            }
                        };
                        gctx->priv().globalCache()->forEachGraphicsPipeline(callback);
                    }
#endif

                    gLoadPending = false;

#if defined(SK_VULKAN)
                    if (isVulkan && !sksl) {
                        // Disassemble the SPIR-V into its textual form.
                        spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_0);
                        for (auto& entry : fCachedShaders) {
                            for (int i = 0; i < kGrShaderTypeCount; ++i) {
                                const std::string& spirv(entry.fShader[i]);
                                std::string disasm;
                                tools.Disassemble((const uint32_t*)spirv.c_str(), spirv.size() / 4,
                                                  &disasm);
                                entry.fShader[i].assign(disasm);
                            }
                        }
                    } else
#endif
                    {
                        // Reformat the SkSL with proper indentation.
                        for (auto& entry : fCachedShaders) {
                            for (int i = 0; i < kGrShaderTypeCount; ++i) {
                                entry.fShader[i] = SkShaderUtils::PrettyPrint(entry.fShader[i]);
                            }
                        }
                    }
                }

                // Defer actually doing the View/Apply logic so that we can trigger an Apply when we
                // start or finish hovering on a tree node in the list below:
                bool doView  = ImGui::Button("View"); ImGui::SameLine();
                bool doApply = false;
                bool doDump  = false;
                if (ctx) {
                    // TODO(skia:14418): we only have Ganesh implementations of Apply/Dump
                    doApply  = ImGui::Button("Apply Changes"); ImGui::SameLine();
                    doDump   = ImGui::Button("Dump SkSL to resources/sksl/");
                }
                int newOptLevel = fOptLevel;
                ImGui::RadioButton("SkSL", &newOptLevel, kShaderOptLevel_Source);
                ImGui::SameLine();
                ImGui::RadioButton("Compile", &newOptLevel, kShaderOptLevel_Compile);
                ImGui::SameLine();
                ImGui::RadioButton("Optimize", &newOptLevel, kShaderOptLevel_Optimize);
                ImGui::SameLine();
                ImGui::RadioButton("Inline", &newOptLevel, kShaderOptLevel_Inline);

                // If we are changing the compile mode, we want to reset the cache and redo
                // everything.
                static bool sDoDeferredView = false;
                if (doView || doDump || newOptLevel != fOptLevel) {
                    sksl = doDump || (newOptLevel == kShaderOptLevel_Source);
                    fOptLevel = (ShaderOptLevel)newOptLevel;
                    switch (fOptLevel) {
                        case kShaderOptLevel_Source:
                            Compiler::EnableOptimizer(OverrideFlag::kOff);
                            Compiler::EnableInliner(OverrideFlag::kOff);
                            break;
                        case kShaderOptLevel_Compile:
                            Compiler::EnableOptimizer(OverrideFlag::kOff);
                            Compiler::EnableInliner(OverrideFlag::kOff);
                            break;
                        case kShaderOptLevel_Optimize:
                            Compiler::EnableOptimizer(OverrideFlag::kOn);
                            Compiler::EnableInliner(OverrideFlag::kOff);
                            break;
                        case kShaderOptLevel_Inline:
                            Compiler::EnableOptimizer(OverrideFlag::kOn);
                            Compiler::EnableInliner(OverrideFlag::kOn);
                            break;
                    }

                    params.fGrContextOptions.fShaderCacheStrategy =
                            sksl ? GrContextOptions::ShaderCacheStrategy::kSkSL
                                 : GrContextOptions::ShaderCacheStrategy::kBackendSource;
                    displayParamsChanged = true;

                    fDeferredActions.push_back([doDump, this]() {
                        // Reset the cache.
                        fPersistentCache.reset();
                        sDoDeferredView = true;

                        // Dump the cache once we have drawn a frame with it.
                        if (doDump) {
                            fDeferredActions.push_back([this]() {
                                this->dumpShadersToResources();
                            });
                        }
                    });
                }

                ImGui::BeginChild("##ScrollingRegion");
                for (auto& entry : fCachedShaders) {
                    bool inTreeNode = ImGui::TreeNode(entry.fKeyString.c_str());
                    bool hovered = ImGui::IsItemHovered();
                    if (hovered != entry.fHovered) {
                        // Force an Apply to patch the highlight shader in/out
                        entry.fHovered = hovered;
                        doApply = true;
                    }
                    if (inTreeNode) {
                        auto stringBox = [](const char* label, std::string* str) {
                            // Full width, and not too much space for each shader
                            int lines = std::count(str->begin(), str->end(), '\n') + 2;
                            ImVec2 boxSize(-1.0f, ImGui::GetTextLineHeight() * std::min(lines, 30));
                            ImGui::InputTextMultiline(label, str, boxSize);
                        };
                        if (ImGui::TreeNode("Key")) {
                            ImGui::TextWrapped("%s", entry.fKeyDescription.c_str());
                            ImGui::TreePop();
                        }
                        stringBox("##VP", &entry.fShader[kVertex_GrShaderType]);
                        stringBox("##FP", &entry.fShader[kFragment_GrShaderType]);
                        ImGui::TreePop();
                    }
                }
                ImGui::EndChild();

                if (doView || sDoDeferredView) {
                    fPersistentCache.reset();
                    if (ctx) {
                        ctx->priv().getGpu()->resetShaderCacheForTesting();
                    }
#if defined(SK_GRAPHITE)
                    if (skgpu::graphite::Context* gctx = fWindow->graphiteContext()) {
                        gctx->priv().globalCache()->deleteResources();
                    }
#endif
                    gLoadPending = true;
                    sDoDeferredView = false;
                }

                // We don't support updating SPIRV shaders. We could re-assemble them (with edits),
                // but I'm not sure anyone wants to do that.
                if (isVulkan && !sksl) {
                    doApply = false;
                }
                if (ctx && doApply) {
                    fPersistentCache.reset();
                    ctx->priv().getGpu()->resetShaderCacheForTesting();
                    for (auto& entry : fCachedShaders) {
                        std::string backup = entry.fShader[kFragment_GrShaderType];
                        if (entry.fHovered) {
                            // The hovered item (if any) gets a special shader to make it
                            // identifiable.
                            std::string& fragShader = entry.fShader[kFragment_GrShaderType];
                            switch (entry.fShaderType) {
                                case SkSetFourByteTag('S', 'K', 'S', 'L'): {
                                    fragShader = build_sksl_highlight_shader();
                                    break;
                                }
                                case SkSetFourByteTag('G', 'L', 'S', 'L'): {
                                    fragShader = build_glsl_highlight_shader(
                                        *ctx->priv().caps()->shaderCaps());
                                    break;
                                }
                                case SkSetFourByteTag('M', 'S', 'L', ' '): {
                                    fragShader = build_metal_highlight_shader(fragShader);
                                    break;
                                }
                            }
                        }

                        auto data = GrPersistentCacheUtils::PackCachedShaders(entry.fShaderType,
                                                                              entry.fShader,
                                                                              entry.fInterfaces,
                                                                              kGrShaderTypeCount);
                        fPersistentCache.store(*entry.fKey, *data, entry.fKeyDescription);

                        entry.fShader[kFragment_GrShaderType] = backup;
                    }
                }
            }
        }
        if (displayParamsChanged || uiParamsChanged) {
            fDeferredActions.push_back([displayParamsChanged, params, this]() {
                if (displayParamsChanged) {
                    fWindow->setRequestedDisplayParams(params);
                }
                fWindow->inval();
                this->updateTitle();
            });
        }
        ImGui::End();
    }

    if (gShaderErrorHandler.fErrors.size()) {
        ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin("Shader Errors", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);
        for (int i = 0; i < gShaderErrorHandler.fErrors.size(); ++i) {
            ImGui::TextWrapped("%s", gShaderErrorHandler.fErrors[i].c_str());
            std::string sksl(gShaderErrorHandler.fShaders[i].c_str());
            SkShaderUtils::VisitLineByLine(sksl, [](int lineNumber, const char* lineText) {
                ImGui::TextWrapped("%4i\t%s\n", lineNumber, lineText);
            });
        }
        ImGui::End();
        gShaderErrorHandler.reset();
    }

    if (fShowZoomWindow && fLastImage) {
        ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Zoom", &fShowZoomWindow)) {
            static int zoomFactor = 8;
            if (ImGui::Button("<<")) {
                zoomFactor = std::max(zoomFactor / 2, 4);
            }
            ImGui::SameLine(); ImGui::Text("%2d", zoomFactor); ImGui::SameLine();
            if (ImGui::Button(">>")) {
                zoomFactor = std::min(zoomFactor * 2, 32);
            }

            if (!fZoomWindowFixed) {
                ImVec2 mousePos = ImGui::GetMousePos();
                fZoomWindowLocation = SkPoint::Make(mousePos.x, mousePos.y);
            }
            SkScalar x = fZoomWindowLocation.x();
            SkScalar y = fZoomWindowLocation.y();
            int xInt = SkScalarRoundToInt(x);
            int yInt = SkScalarRoundToInt(y);
            ImVec2 avail = ImGui::GetContentRegionAvail();

            uint32_t pixel = 0;
            SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
            bool didGraphiteRead = false;
            if (is_graphite_backend_type(fBackendType)) {
#if defined(GRAPHITE_TEST_UTILS)
                SkBitmap bitmap;
                bitmap.allocPixels(info);
                SkPixmap pixels;
                SkAssertResult(bitmap.peekPixels(&pixels));
                didGraphiteRead = fLastImage->readPixelsGraphite(fWindow->graphiteRecorder(),
                                                                 pixels,
                                                                 xInt,
                                                                 yInt);
                pixel = *pixels.addr32();
                ImGui::SameLine();
                ImGui::Text("(X, Y): %d, %d RGBA: %X %X %X %X",
                            xInt, yInt,
                            SkGetPackedR32(pixel), SkGetPackedG32(pixel),
                            SkGetPackedB32(pixel), SkGetPackedA32(pixel));
#endif
            }
            auto dContext = fWindow->directContext();
            if (fLastImage->readPixels(dContext,
                                       info,
                                       &pixel,
                                       info.minRowBytes(),
                                       xInt,
                                       yInt)) {
                ImGui::SameLine();
                ImGui::Text("(X, Y): %d, %d RGBA: %X %X %X %X",
                            xInt, yInt,
                            SkGetPackedR32(pixel), SkGetPackedG32(pixel),
                            SkGetPackedB32(pixel), SkGetPackedA32(pixel));
            } else {
                if (!didGraphiteRead) {
                    ImGui::SameLine();
                    ImGui::Text("Failed to readPixels");
                }
            }

            fImGuiLayer.skiaWidget(avail, [=, lastImage = fLastImage](SkCanvas* c) {
                // Translate so the region of the image that's under the mouse cursor is centered
                // in the zoom canvas:
                c->scale(zoomFactor, zoomFactor);
                c->translate(avail.x * 0.5f / zoomFactor - x - 0.5f,
                             avail.y * 0.5f / zoomFactor - y - 0.5f);
                c->drawImage(lastImage, 0, 0);

                SkPaint outline;
                outline.setStyle(SkPaint::kStroke_Style);
                c->drawRect(SkRect::MakeXYWH(x, y, 1, 1), outline);
            });
        }

        ImGui::End();
    }

    if (fShowHistogramWindow && fLastImage) {
        ImGui::SetNextWindowSize(ImVec2(450, 500));
        ImGui::SetNextWindowBgAlpha(0.5f);
        if (ImGui::Begin("Color Histogram (R,G,B)", &fShowHistogramWindow)) {
            const auto info = SkImageInfo::MakeN32Premul(fWindow->width(), fWindow->height());
            SkAutoPixmapStorage pixmap;
            pixmap.alloc(info);

            if (fLastImage->readPixels(fWindow->directContext(), info, pixmap.writable_addr(),
                                       info.minRowBytes(), 0, 0)) {
                std::vector<float> r(256), g(256), b(256);
                for (int y = 0; y < info.height(); ++y) {
                    for (int x = 0; x < info.width(); ++x) {
                        const auto pmc = *pixmap.addr32(x, y);
                        r[SkGetPackedR32(pmc)]++;
                        g[SkGetPackedG32(pmc)]++;
                        b[SkGetPackedB32(pmc)]++;
                    }
                }

                ImGui::PushItemWidth(-1);
                ImGui::PlotHistogram("R", r.data(), r.size(), 0, nullptr,
                                     FLT_MAX, FLT_MAX, ImVec2(0, 150));
                ImGui::PlotHistogram("G", g.data(), g.size(), 0, nullptr,
                                     FLT_MAX, FLT_MAX, ImVec2(0, 150));
                ImGui::PlotHistogram("B", b.data(), b.size(), 0, nullptr,
                                     FLT_MAX, FLT_MAX, ImVec2(0, 150));
                ImGui::PopItemWidth();
            }
        }

        ImGui::End();
    }
}

void Viewer::dumpShadersToResources() {
    // Sort the list of cached shaders so we can maintain some minimal level of consistency.
    // It doesn't really matter, but it will keep files from switching places unpredictably.
    std::vector<const CachedShader*> shaders;
    shaders.reserve(fCachedShaders.size());
    for (const CachedShader& shader : fCachedShaders) {
        shaders.push_back(&shader);
    }

    std::sort(shaders.begin(), shaders.end(), [](const CachedShader* a, const CachedShader* b) {
        return std::tie(a->fShader[kFragment_GrShaderType], a->fShader[kVertex_GrShaderType]) <
               std::tie(b->fShader[kFragment_GrShaderType], b->fShader[kVertex_GrShaderType]);
    });

    // Make the resources/sksl/SlideName/ directory.
    SkString directory = SkStringPrintf("%ssksl/%s",
                                        GetResourcePath().c_str(),
                                        fSlides[fCurrentSlide]->getName().c_str());
    if (!sk_mkdir(directory.c_str())) {
        SkDEBUGFAILF("Unable to create directory '%s'", directory.c_str());
        return;
    }

    int index = 0;
    for (const auto& entry : shaders) {
        SkString vertPath = SkStringPrintf("%s/Vertex_%02d.vert", directory.c_str(), index);
        FILE* vertFile = sk_fopen(vertPath.c_str(), kWrite_SkFILE_Flag);
        if (vertFile) {
            const std::string& vertText = entry->fShader[kVertex_GrShaderType];
            SkAssertResult(sk_fwrite(vertText.c_str(), vertText.size(), vertFile));
            sk_fclose(vertFile);
        } else {
            SkDEBUGFAILF("Unable to write shader to path '%s'", vertPath.c_str());
        }

        SkString fragPath = SkStringPrintf("%s/Fragment_%02d.frag", directory.c_str(), index);
        FILE* fragFile = sk_fopen(fragPath.c_str(), kWrite_SkFILE_Flag);
        if (fragFile) {
            const std::string& fragText = entry->fShader[kFragment_GrShaderType];
            SkAssertResult(sk_fwrite(fragText.c_str(), fragText.size(), fragFile));
            sk_fclose(fragFile);
        } else {
            SkDEBUGFAILF("Unable to write shader to path '%s'", fragPath.c_str());
        }

        ++index;
    }
}

void Viewer::onIdle() {
    TArray<std::function<void()>> actionsToRun;
    actionsToRun.swap(fDeferredActions);

    for (const auto& fn : actionsToRun) {
        fn();
    }

    fStatsLayer.beginTiming(fAnimateTimer);
    fAnimTimer.updateTime();
    bool animateWantsInval = fSlides[fCurrentSlide]->animate(fAnimTimer.nanos());
    fStatsLayer.endTiming(fAnimateTimer);

    ImGuiIO& io = ImGui::GetIO();
    // ImGui always has at least one "active" window, which is the default "Debug" window. It may
    // not be visible, though. So we need to redraw if there is at least one visible window, or
    // more than one active window. Newly created windows are active but not visible for one frame
    // while they determine their layout and sizing.
    if (animateWantsInval || fStatsLayer.getActive() || fRefresh ||
        io.MetricsActiveWindows > 1 || io.MetricsRenderWindows > 0) {
        fWindow->inval();
    }
}

template <typename OptionsFunc>
static void WriteStateObject(SkJSONWriter& writer, const char* name, const char* value,
                             OptionsFunc&& optionsFunc) {
    writer.beginObject();
    {
        writer.appendCString(kName , name);
        writer.appendCString(kValue, value);

        writer.beginArray(kOptions);
        {
            optionsFunc(writer);
        }
        writer.endArray();
    }
    writer.endObject();
}


void Viewer::updateUIState() {
    if (!fWindow) {
        return;
    }
    if (fWindow->sampleCount() < 1) {
        return; // Surface hasn't been created yet.
    }

    SkDynamicMemoryWStream memStream;
    SkJSONWriter writer(&memStream);
    writer.beginArray();

    // Slide state
    WriteStateObject(writer, kSlideStateName, fSlides[fCurrentSlide]->getName().c_str(),
        [this](SkJSONWriter& writer) {
            for(const auto& slide : fSlides) {
                writer.appendString(slide->getName());
            }
        });

    // Backend state
    WriteStateObject(writer, kBackendStateName, get_backend_string(fBackendType),
        [](SkJSONWriter& writer) {
            for (int i = 0; i < sk_app::Window::kBackendTypeCount; ++i) {
                auto backendType = static_cast<sk_app::Window::BackendType>(i);
                writer.appendCString(get_backend_string(backendType));
            }
        });

    // MSAA state
    const auto countString = SkStringPrintf("%d", fWindow->sampleCount());
    WriteStateObject(writer, kMSAAStateName, countString.c_str(),
        [this](SkJSONWriter& writer) {
            writer.appendS32(0);

            if (sk_app::Window::kRaster_BackendType == fBackendType) {
                return;
            }

            for (int msaa : {4, 8, 16}) {
                writer.appendS32(msaa);
            }
        });

    // TODO: Store Graphite path renderer strategy
    // Path renderer state
    GpuPathRenderers pr = fWindow->getRequestedDisplayParams().fGrContextOptions.fGpuPathRenderers;
    WriteStateObject(writer, kPathRendererStateName, gGaneshPathRendererNames[pr].c_str(),
        [this](SkJSONWriter& writer) {
            auto ctx = fWindow->directContext();
            if (!ctx) {
                writer.appendNString("Software");
            } else {
                writer.appendString(gGaneshPathRendererNames[GpuPathRenderers::kDefault]);
#if defined(SK_GANESH)
                if (fWindow->sampleCount() > 1 || FLAGS_dmsaa) {
                    const auto* caps = ctx->priv().caps();
                    if (skgpu::ganesh::AtlasPathRenderer::IsSupported(ctx)) {
                        writer.appendString(gGaneshPathRendererNames[GpuPathRenderers::kAtlas]);
                    }
                    if (skgpu::ganesh::TessellationPathRenderer::IsSupported(*caps)) {
                        writer.appendString(gGaneshPathRendererNames[GpuPathRenderers::kTessellation]);
                    }
                }
#endif
                if (1 == fWindow->sampleCount()) {
                    writer.appendString(gGaneshPathRendererNames[GpuPathRenderers::kSmall]);
                }
                writer.appendString(gGaneshPathRendererNames[GpuPathRenderers::kTriangulating]);
                writer.appendString(gGaneshPathRendererNames[GpuPathRenderers::kNone]);
            }
        });

    // Softkey state
    WriteStateObject(writer, kSoftkeyStateName, kSoftkeyHint,
        [this](SkJSONWriter& writer) {
            writer.appendNString(kSoftkeyHint);
            for (const auto& softkey : fCommands.getCommandsAsSoftkeys()) {
                writer.appendString(softkey);
            }
        });

    writer.endArray();
    writer.flush();

    auto data = memStream.detachAsData();

    // TODO: would be cool to avoid this copy
    const SkString cstring(static_cast<const char*>(data->data()), data->size());

    fWindow->setUIState(cstring.c_str());
}

void Viewer::onUIStateChanged(const SkString& stateName, const SkString& stateValue) {
    // For those who will add more features to handle the state change in this function:
    // After the change, please call updateUIState no notify the frontend (e.g., Android app).
    // For example, after slide change, updateUIState is called inside setupCurrentSlide;
    // after backend change, updateUIState is called in this function.
    if (stateName.equals(kSlideStateName)) {
        for (int i = 0; i < fSlides.size(); ++i) {
            if (fSlides[i]->getName().equals(stateValue)) {
                this->setCurrentSlide(i);
                return;
            }
        }

        SkDebugf("Slide not found: %s", stateValue.c_str());
    } else if (stateName.equals(kBackendStateName)) {
        for (int i = 0; i < sk_app::Window::kBackendTypeCount; i++) {
            auto backendType = static_cast<sk_app::Window::BackendType>(i);
            if (stateValue.equals(get_backend_string(backendType))) {
                if (fBackendType != i) {
                    fBackendType = backendType;
                    for(auto& slide : fSlides) {
                        slide->gpuTeardown();
                    }
                    fWindow->detach();
                    fWindow->attach(backend_type_for_window(fBackendType));
                }
                break;
            }
        }
    } else if (stateName.equals(kMSAAStateName)) {
        DisplayParams params = fWindow->getRequestedDisplayParams();
        int sampleCount = atoi(stateValue.c_str());
        if (sampleCount != params.fMSAASampleCount) {
            params.fMSAASampleCount = sampleCount;
            fWindow->setRequestedDisplayParams(params);
            fWindow->inval();
            this->updateTitle();
            this->updateUIState();
        }
    } else if (stateName.equals(kPathRendererStateName)) {
        DisplayParams params = fWindow->getRequestedDisplayParams();
        for (const auto& pair : gGaneshPathRendererNames) {
            if (pair.second == stateValue.c_str()) {
                if (params.fGrContextOptions.fGpuPathRenderers != pair.first) {
                    params.fGrContextOptions.fGpuPathRenderers = pair.first;
                    fWindow->setRequestedDisplayParams(params);
                    fWindow->inval();
                    this->updateTitle();
                    this->updateUIState();
                }
                break;
            }
        }
    } else if (stateName.equals(kSoftkeyStateName)) {
        if (!stateValue.equals(kSoftkeyHint)) {
            fCommands.onSoftkey(stateValue);
            this->updateUIState(); // This is still needed to reset the value to kSoftkeyHint
        }
    } else if (stateName.equals(kRefreshStateName)) {
        // This state is actually NOT in the UI state.
        // We use this to allow Android to quickly set bool fRefresh.
        fRefresh = stateValue.equals(kON);
    } else {
        SkDebugf("Unknown stateName: %s", stateName.c_str());
    }
}

bool Viewer::onKey(skui::Key key, skui::InputState state, skui::ModifierKey modifiers) {
    return fCommands.onKey(key, state, modifiers);
}

bool Viewer::onChar(SkUnichar c, skui::ModifierKey modifiers) {
    if (fSlides[fCurrentSlide]->onChar(c)) {
        fWindow->inval();
        return true;
    } else {
        return fCommands.onChar(c, modifiers);
    }
}
