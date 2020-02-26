/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImageInfo.h"
#include "include/private/SkTHash.h"
#include "src/core/SkColorSpacePriv.h"
#include "tools/flags/CommonFlagsConfig.h"

#include <stdlib.h>

using sk_gpu_test::GrContextFactory;

#if defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_IOS)
#define DEFAULT_GPU_CONFIG "gles"
#else
#define DEFAULT_GPU_CONFIG "gl"
#endif

static const char defaultConfigs[] = "8888 " DEFAULT_GPU_CONFIG
                                     " nonrendering "
#if SK_ANGLE && defined(SK_BUILD_FOR_WIN)
                                     " angle_d3d11_es2"
#endif
        ;

#undef DEFAULT_GPU_CONFIG

// clang-format off
static const struct {
    const char* predefinedConfig;
    const char* backend;
    const char* options;
} gPredefinedConfigs[] = {
    { "gl",                    "gpu", "api=gl" },
    { "gles",                  "gpu", "api=gles" },
    { "glmsaa4",               "gpu", "api=gl,samples=4" },
    { "glmsaa8" ,              "gpu", "api=gl,samples=8" },
    { "glesmsaa4",             "gpu", "api=gles,samples=4" },
    { "glbetex",               "gpu", "api=gl,surf=betex" },
    { "glesbetex",             "gpu", "api=gles,surf=betex" },
    { "glbert",                "gpu", "api=gl,surf=bert" },
    { "glesbert",              "gpu", "api=gles,surf=bert" },
    { "gl4444",                "gpu", "api=gl,color=4444" },
    { "gles4444",              "gpu", "api=gles,color=4444" },
    { "gl565",                 "gpu", "api=gl,color=565" },
    { "gl888x",                "gpu", "api=gl,color=888x" },
    { "gles888x",              "gpu", "api=gles,color=888x" },
    { "gl1010102",             "gpu", "api=gl,color=1010102" },
    { "gles1010102",           "gpu", "api=gles,color=1010102" },
    { "glsrgb",                "gpu", "api=gl,color=srgb" },
    { "glp3",                  "gpu", "api=gl,color=p3" },
    { "glesrgb",               "gpu", "api=gl,color=esrgb" },
    { "glnarrow",              "gpu", "api=gl,color=narrow" },
    { "glenarrow",             "gpu", "api=gl,color=enarrow" },
    { "glf16",                 "gpu", "api=gl,color=f16" },
    { "glf16norm",             "gpu", "api=gl,color=f16norm" },
    { "glessrgb",              "gpu", "api=gles,color=srgb" },
    { "glesesrgb",             "gpu", "api=gles,color=esrgb" },
    { "glesnarrow",            "gpu", "api=gles,color=narrow" },
    { "glesenarrow",           "gpu", "api=gles,color=enarrow" },
    { "glesf16",               "gpu", "api=gles,color=f16" },
    { "glnostencils",          "gpu", "api=gl,stencils=false" },
    { "gldft",                 "gpu", "api=gl,dit=true" },
    { "glesdft",               "gpu", "api=gles,dit=true" },
    { "gltestthreading",       "gpu", "api=gl,testThreading=true" },
    { "gltestpersistentcache", "gpu", "api=gl,testPersistentCache=1" },
    { "gltestglslcache",       "gpu", "api=gl,testPersistentCache=2" },
    { "gltestprecompile",      "gpu", "api=gl,testPrecompile=true" },
    { "glestestprecompile",    "gpu", "api=gles,testPrecompile=true" },
    { "glddl",                 "gpu", "api=gl,useDDLSink=true" },
    { "angle_d3d11_es2",       "gpu", "api=angle_d3d11_es2" },
    { "angle_d3d11_es3",       "gpu", "api=angle_d3d11_es3" },
    { "angle_d3d9_es2",        "gpu", "api=angle_d3d9_es2" },
    { "angle_d3d11_es2_msaa4", "gpu", "api=angle_d3d11_es2,samples=4" },
    { "angle_d3d11_es2_msaa8", "gpu", "api=angle_d3d11_es2,samples=8" },
    { "angle_d3d11_es3_msaa4", "gpu", "api=angle_d3d11_es3,samples=4" },
    { "angle_d3d11_es3_msaa8", "gpu", "api=angle_d3d11_es3,samples=8" },
    { "angle_gl_es2",          "gpu", "api=angle_gl_es2" },
    { "angle_gl_es3",          "gpu", "api=angle_gl_es3" },
    { "angle_gl_es2_msaa8",    "gpu", "api=angle_gl_es2,samples=8" },
    { "angle_gl_es3_msaa8",    "gpu", "api=angle_gl_es3,samples=8" },
    { "commandbuffer",         "gpu", "api=commandbuffer" },
    { "mock",                  "gpu", "api=mock" },
#ifdef SK_DAWN
    { "dawn",                  "gpu", "api=dawn" },
#endif
#ifdef SK_VULKAN
    { "vk",                    "gpu", "api=vulkan" },
    { "vknostencils",          "gpu", "api=vulkan,stencils=false" },
    { "vk1010102",             "gpu", "api=vulkan,color=1010102" },
    { "vksrgb",                "gpu", "api=vulkan,color=srgb" },
    { "vkesrgb",               "gpu", "api=vulkan,color=esrgb" },
    { "vknarrow",              "gpu", "api=vulkan,color=narrow" },
    { "vkenarrow",             "gpu", "api=vulkan,color=enarrow" },
    { "vkf16",                 "gpu", "api=vulkan,color=f16" },
    { "vkmsaa4",               "gpu", "api=vulkan,samples=4" },
    { "vkmsaa8",               "gpu", "api=vulkan,samples=8" },
    { "vkbetex",               "gpu", "api=vulkan,surf=betex" },
    { "vkbert",                "gpu", "api=vulkan,surf=bert" },
    { "vktestpersistentcache", "gpu", "api=vulkan,testPersistentCache=1" },
    { "vkddl",                 "gpu", "api=vulkan,useDDLSink=true" },
#endif
#ifdef SK_METAL
    { "mtl",                   "gpu", "api=metal" },
    { "mtl1010102",            "gpu", "api=metal,color=1010102" },
    { "mtlmsaa4",              "gpu", "api=metal,samples=4" },
    { "mtlmsaa8",              "gpu", "api=metal,samples=8" },
    { "mtlddl",                "gpu", "api=metal,useDDLSink=true" },
#endif
#ifdef SK_DIRECT3D
    { "d3d",                   "gpu", "api=direct3d" },
#endif
};
// clang-format on

static const char configHelp[] =
        "Options: 565 8888 srgb f16 nonrendering null pdf pdfa skp pipe svg xps";

static const char* config_help_fn() {
    static SkString helpString;
    helpString.set(configHelp);
    for (const auto& config : gPredefinedConfigs) {
        helpString.appendf(" %s", config.predefinedConfig);
    }
    helpString.append(" or use extended form 'backend[option=value,...]'.\n");
    return helpString.c_str();
}

static const char configExtendedHelp[] =
        "Extended form: 'backend(option=value,...)'\n\n"
        "Possible backends and options:\n"
        "\n"
        "gpu[api=string,color=string,dit=bool,samples=int]\n"
        "\tapi\ttype: string\trequired\n"
        "\t    Select graphics API to use with gpu backend.\n"
        "\t    Options:\n"
        "\t\tgl    \t\t\tUse OpenGL.\n"
        "\t\tgles  \t\t\tUse OpenGL ES.\n"
        "\t\tnullgl \t\t\tUse null OpenGL.\n"
        "\t\tangle_d3d9_es2\t\tUse OpenGL ES2 on the ANGLE Direct3D9 backend.\n"
        "\t\tangle_d3d11_es2\t\tUse OpenGL ES2 on the ANGLE Direct3D11 backend.\n"
        "\t\tangle_d3d11_es3\t\tUse OpenGL ES3 on the ANGLE Direct3D11 backend.\n"
        "\t\tangle_gl_es2\t\tUse OpenGL ES2 on the ANGLE OpenGL backend.\n"
        "\t\tangle_gl_es3\t\tUse OpenGL ES3 on the ANGLE OpenGL backend.\n"
        "\t\tcommandbuffer\t\tUse command buffer.\n"
        "\t\tmock\t\t\tUse mock context.\n"
#ifdef SK_VULKAN
        "\t\tvulkan\t\t\tUse Vulkan.\n"
#endif
#ifdef SK_METAL
        "\t\tmetal\t\t\tUse Metal.\n"
#endif
        "\tcolor\ttype: string\tdefault: 8888.\n"
        "\t    Select framebuffer color format.\n"
        "\t    Options:\n"
        "\t\t8888\t\t\tLinear 8888.\n"
        "\t\t888x\t\t\tLinear 888x.\n"
        "\t\t4444\t\t\tLinear 4444.\n"
        "\t\t565\t\t\tLinear 565.\n"
        "\t\t1010102\t\t\tLinear 1010102.\n"
        "\t\tsrgb\t\t\tsRGB 8888.\n"
        "\t\tesrgb\t\t\tsRGB 16-bit floating point.\n"
        "\t\tnarrow\t\t\tNarrow gamut 8888.\n"
        "\t\tenarrow\t\t\tNarrow gamut 16-bit floating point.\n"
        "\t\tf16\t\t\tLinearly blended 16-bit floating point.\n"
        "\tdit\ttype: bool\tdefault: false.\n"
        "\t    Use device independent text.\n"
        "\tsamples\ttype: int\tdefault: 0.\n"
        "\t    Use multisampling with N samples.\n"
        "\tstencils\ttype: bool\tdefault: true.\n"
        "\t    Allow the use of stencil buffers.\n"
        "\ttestThreading\ttype: bool\tdefault: false.\n"
        "\t    Run with and without worker threads, check that results match.\n"
        "\ttestPersistentCache\ttype: int\tdefault: 0.\n"
        "\t    1: Run using a pre-warmed binary GrContextOptions::fPersistentCache.\n"
        "\t    2: Run using a pre-warmed GLSL GrContextOptions::fPersistentCache.\n"
        "\tsurf\ttype: string\tdefault: default.\n"
        "\t    Controls the type of backing store for SkSurfaces.\n"
        "\t    Options:\n"
        "\t\tdefault\t\t\tA renderable texture created in Skia's resource cache.\n"
        "\t\tbetex\t\t\tA wrapped backend texture.\n"
        "\t\tbert\t\t\tA wrapped backend render target\n"
        "\n"
        "Predefined configs:\n\n"
        // Help text for pre-defined configs is auto-generated from gPredefinedConfigs
        ;

static const char* config_extended_help_fn() {
    static SkString helpString;
    helpString.set(configExtendedHelp);
    for (const auto& config : gPredefinedConfigs) {
        helpString.appendf("\t%-10s\t= gpu(%s)\n", config.predefinedConfig, config.options);
    }
    return helpString.c_str();
}

DEFINE_extended_string(config, defaultConfigs, config_help_fn(), config_extended_help_fn());

SkCommandLineConfig::SkCommandLineConfig(const SkString&           tag,
                                         const SkString&           backend,
                                         const SkTArray<SkString>& viaParts)
        : fTag(tag), fBackend(backend), fViaParts(viaParts) {}
SkCommandLineConfig::~SkCommandLineConfig() {}

static bool parse_option_int(const SkString& value, int* outInt) {
    if (value.isEmpty()) {
        return false;
    }
    char* endptr   = nullptr;
    long  intValue = strtol(value.c_str(), &endptr, 10);
    if (*endptr != '\0') {
        return false;
    }
    *outInt = static_cast<int>(intValue);
    return true;
}
static bool parse_option_bool(const SkString& value, bool* outBool) {
    if (value.equals("true")) {
        *outBool = true;
        return true;
    }
    if (value.equals("false")) {
        *outBool = false;
        return true;
    }
    return false;
}
static bool parse_option_gpu_api(const SkString&                      value,
                                 SkCommandLineConfigGpu::ContextType* outContextType) {
    if (value.equals("gl")) {
        *outContextType = GrContextFactory::kGL_ContextType;
        return true;
    }
    if (value.equals("gles")) {
        *outContextType = GrContextFactory::kGLES_ContextType;
        return true;
    }
    if (value.equals("angle_d3d9_es2")) {
        *outContextType = GrContextFactory::kANGLE_D3D9_ES2_ContextType;
        return true;
    }
    if (value.equals("angle_d3d11_es2")) {
        *outContextType = GrContextFactory::kANGLE_D3D11_ES2_ContextType;
        return true;
    }
    if (value.equals("angle_d3d11_es3")) {
        *outContextType = GrContextFactory::kANGLE_D3D11_ES3_ContextType;
        return true;
    }
    if (value.equals("angle_gl_es2")) {
        *outContextType = GrContextFactory::kANGLE_GL_ES2_ContextType;
        return true;
    }
    if (value.equals("angle_gl_es3")) {
        *outContextType = GrContextFactory::kANGLE_GL_ES3_ContextType;
        return true;
    }
    if (value.equals("commandbuffer")) {
        *outContextType = GrContextFactory::kCommandBuffer_ContextType;
        return true;
    }
    if (value.equals("mock")) {
        *outContextType = GrContextFactory::kMock_ContextType;
        return true;
    }
#ifdef SK_VULKAN
    if (value.equals("vulkan")) {
        *outContextType = GrContextFactory::kVulkan_ContextType;
        return true;
    }
#endif
#ifdef SK_METAL
    if (value.equals("metal")) {
        *outContextType = GrContextFactory::kMetal_ContextType;
        return true;
    }
#endif
#ifdef SK_DIRECT3D
    if (value.equals("direct3d")) {
        *outContextType = GrContextFactory::kDirect3D_ContextType;
        return true;
    }
#endif
#ifdef SK_DAWN
    if (value.equals("dawn")) {
        *outContextType = GrContextFactory::kDawn_ContextType;
        return true;
    }
#endif
    return false;
}

static bool parse_option_gpu_color(const SkString&      value,
                                   SkColorType*         outColorType,
                                   SkAlphaType*         alphaType,
                                   sk_sp<SkColorSpace>* outColorSpace) {
    // We always use premul unless the color type is 565.
    *alphaType = kPremul_SkAlphaType;

    if (value.equals("8888")) {
        *outColorType  = kRGBA_8888_SkColorType;
        *outColorSpace = nullptr;
    } else if (value.equals("888x")) {
        *outColorType  = kRGB_888x_SkColorType;
        *outColorSpace = nullptr;
    } else if (value.equals("8888s")) {
        *outColorType  = kRGBA_8888_SkColorType;
        *outColorSpace = SkColorSpace::MakeSRGB();
    } else if (value.equals("bgra8")) {
        *outColorType  = kBGRA_8888_SkColorType;
        *outColorSpace = nullptr;
    } else if (value.equals("bgra8s")) {
        *outColorType  = kBGRA_8888_SkColorType;
        *outColorSpace = SkColorSpace::MakeSRGB();
    } else if (value.equals("4444")) {
        *outColorType  = kARGB_4444_SkColorType;
        *outColorSpace = nullptr;
    } else if (value.equals("565")) {
        *outColorType  = kRGB_565_SkColorType;
        *alphaType     = kOpaque_SkAlphaType;
        *outColorSpace = nullptr;
    } else if (value.equals("1010102")) {
        *outColorType  = kRGBA_1010102_SkColorType;
        *outColorSpace = nullptr;
    } else if (value.equals("srgb")) {
        *outColorType  = kRGBA_8888_SkColorType;
        *outColorSpace = SkColorSpace::MakeSRGB();
    } else if (value.equals("p3")) {
        *outColorType  = kRGBA_8888_SkColorType;
        *outColorSpace = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDisplayP3);
    } else if (value.equals("esrgb")) {
        *outColorType  = kRGBA_F16_SkColorType;
        *outColorSpace = SkColorSpace::MakeSRGB();
    } else if (value.equals("narrow") || value.equals("enarrow")) {
        *outColorType  = value.equals("narrow") ? kRGBA_8888_SkColorType : kRGBA_F16_SkColorType;
        *outColorSpace = SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2, gNarrow_toXYZD50);
    } else if (value.equals("f16")) {
        *outColorType  = kRGBA_F16_SkColorType;
        *outColorSpace = SkColorSpace::MakeSRGBLinear();
    } else if (value.equals("f16norm")) {
        *outColorType  = kRGBA_F16Norm_SkColorType;
        *outColorSpace = SkColorSpace::MakeSRGB();
    } else {
        return false;
    }
    return true;
}

static bool parse_option_gpu_surf_type(const SkString&                   value,
                                       SkCommandLineConfigGpu::SurfType* surfType) {
    if (value.equals("default")) {
        *surfType = SkCommandLineConfigGpu::SurfType::kDefault;
        return true;
    }
    if (value.equals("betex")) {
        *surfType = SkCommandLineConfigGpu::SurfType::kBackendTexture;
        return true;
    }
    if (value.equals("bert")) {
        *surfType = SkCommandLineConfigGpu::SurfType::kBackendRenderTarget;
        return true;
    }
    return false;
}

// Extended options take form --config item[key1=value1,key2=value2,...]
// Example: --config gpu[api=gl,color=8888]
class ExtendedOptions {
public:
    ExtendedOptions(const SkString& optionsString, bool* outParseSucceeded) {
        SkTArray<SkString> optionParts;
        SkStrSplit(optionsString.c_str(), ",", kStrict_SkStrSplitMode, &optionParts);
        for (int i = 0; i < optionParts.count(); ++i) {
            SkTArray<SkString> keyValueParts;
            SkStrSplit(optionParts[i].c_str(), "=", kStrict_SkStrSplitMode, &keyValueParts);
            if (keyValueParts.count() != 2) {
                *outParseSucceeded = false;
                return;
            }
            const SkString& key   = keyValueParts[0];
            const SkString& value = keyValueParts[1];
            if (fOptionsMap.find(key) == nullptr) {
                fOptionsMap.set(key, value);
            } else {
                // Duplicate values are not allowed.
                *outParseSucceeded = false;
                return;
            }
        }
        *outParseSucceeded = true;
    }

    bool get_option_gpu_color(const char*          optionKey,
                              SkColorType*         outColorType,
                              SkAlphaType*         alphaType,
                              sk_sp<SkColorSpace>* outColorSpace,
                              bool                 optional = true) const {
        SkString* optionValue = fOptionsMap.find(SkString(optionKey));
        if (optionValue == nullptr) {
            return optional;
        }
        return parse_option_gpu_color(*optionValue, outColorType, alphaType, outColorSpace);
    }

    bool get_option_gpu_api(const char*                          optionKey,
                            SkCommandLineConfigGpu::ContextType* outContextType,
                            bool                                 optional = true) const {
        SkString* optionValue = fOptionsMap.find(SkString(optionKey));
        if (optionValue == nullptr) {
            return optional;
        }
        return parse_option_gpu_api(*optionValue, outContextType);
    }

    bool get_option_gpu_surf_type(const char*                       optionKey,
                                  SkCommandLineConfigGpu::SurfType* outSurfType,
                                  bool                              optional = true) const {
        SkString* optionValue = fOptionsMap.find(SkString(optionKey));
        if (optionValue == nullptr) {
            return optional;
        }
        return parse_option_gpu_surf_type(*optionValue, outSurfType);
    }

    bool get_option_int(const char* optionKey, int* outInt, bool optional = true) const {
        SkString* optionValue = fOptionsMap.find(SkString(optionKey));
        if (optionValue == nullptr) {
            return optional;
        }
        return parse_option_int(*optionValue, outInt);
    }

    bool get_option_bool(const char* optionKey, bool* outBool, bool optional = true) const {
        SkString* optionValue = fOptionsMap.find(SkString(optionKey));
        if (optionValue == nullptr) {
            return optional;
        }
        return parse_option_bool(*optionValue, outBool);
    }

private:
    SkTHashMap<SkString, SkString> fOptionsMap;
};

SkCommandLineConfigGpu::SkCommandLineConfigGpu(const SkString&           tag,
                                               const SkTArray<SkString>& viaParts,
                                               ContextType               contextType,
                                               bool                      useDIText,
                                               int                       samples,
                                               SkColorType               colorType,
                                               SkAlphaType               alphaType,
                                               sk_sp<SkColorSpace>       colorSpace,
                                               bool                      useStencilBuffers,
                                               bool                      testThreading,
                                               int                       testPersistentCache,
                                               bool                      testPrecompile,
                                               bool                      useDDLSink,
                                               SurfType                  surfType)
        : SkCommandLineConfig(tag, SkString("gpu"), viaParts)
        , fContextType(contextType)
        , fContextOverrides(ContextOverrides::kNone)
        , fUseDIText(useDIText)
        , fSamples(samples)
        , fColorType(colorType)
        , fAlphaType(alphaType)
        , fColorSpace(std::move(colorSpace))
        , fTestThreading(testThreading)
        , fTestPersistentCache(testPersistentCache)
        , fTestPrecompile(testPrecompile)
        , fUseDDLSink(useDDLSink)
        , fSurfType(surfType) {
    if (!useStencilBuffers) {
        fContextOverrides |= ContextOverrides::kAvoidStencilBuffers;
    }
}

SkCommandLineConfigGpu* parse_command_line_config_gpu(const SkString&           tag,
                                                      const SkTArray<SkString>& vias,
                                                      const SkString&           options) {
    // Defaults for GPU backend.
    SkCommandLineConfigGpu::ContextType contextType         = GrContextFactory::kGL_ContextType;
    bool                                useDIText           = false;
    int                                 samples             = 1;
    SkColorType                         colorType           = kRGBA_8888_SkColorType;
    SkAlphaType                         alphaType           = kPremul_SkAlphaType;
    sk_sp<SkColorSpace>                 colorSpace          = nullptr;
    bool                                useStencils         = true;
    bool                                testThreading       = false;
    int                                 testPersistentCache = 0;
    bool                                testPrecompile      = false;
    bool                                useDDLs             = false;
    SkCommandLineConfigGpu::SurfType    surfType = SkCommandLineConfigGpu::SurfType::kDefault;

    bool            parseSucceeded = false;
    ExtendedOptions extendedOptions(options, &parseSucceeded);
    if (!parseSucceeded) {
        return nullptr;
    }

    bool validOptions =
            extendedOptions.get_option_gpu_api("api", &contextType, false) &&
            extendedOptions.get_option_bool("dit", &useDIText) &&
            extendedOptions.get_option_int("samples", &samples) &&
            extendedOptions.get_option_gpu_color("color", &colorType, &alphaType, &colorSpace) &&
            extendedOptions.get_option_bool("stencils", &useStencils) &&
            extendedOptions.get_option_bool("testThreading", &testThreading) &&
            extendedOptions.get_option_int("testPersistentCache", &testPersistentCache) &&
            extendedOptions.get_option_bool("testPrecompile", &testPrecompile) &&
            extendedOptions.get_option_bool("useDDLSink", &useDDLs) &&
            extendedOptions.get_option_gpu_surf_type("surf", &surfType);

    // testing threading and the persistent cache are mutually exclusive.
    if (!validOptions || (testThreading && (testPersistentCache != 0))) {
        return nullptr;
    }

    return new SkCommandLineConfigGpu(tag,
                                      vias,
                                      contextType,
                                      useDIText,
                                      samples,
                                      colorType,
                                      alphaType,
                                      colorSpace,
                                      useStencils,
                                      testThreading,
                                      testPersistentCache,
                                      testPrecompile,
                                      useDDLs,
                                      surfType);
}

SkCommandLineConfigSvg::SkCommandLineConfigSvg(const SkString&           tag,
                                               const SkTArray<SkString>& viaParts,
                                               int                       pageIndex)
        : SkCommandLineConfig(tag, SkString("svg"), viaParts), fPageIndex(pageIndex) {}

SkCommandLineConfigSvg* parse_command_line_config_svg(const SkString&           tag,
                                                      const SkTArray<SkString>& vias,
                                                      const SkString&           options) {
    // Defaults for SVG backend.
    int pageIndex = 0;

    bool            parseSucceeded = false;
    ExtendedOptions extendedOptions(options, &parseSucceeded);
    if (!parseSucceeded) {
        return nullptr;
    }

    bool validOptions = extendedOptions.get_option_int("page", &pageIndex);

    if (!validOptions) {
        return nullptr;
    }

    return new SkCommandLineConfigSvg(tag, vias, pageIndex);
}

void ParseConfigs(const CommandLineFlags::StringArray& configs,
                  SkCommandLineConfigArray*            outResult) {
    outResult->reset();
    for (int i = 0; i < configs.count(); ++i) {
        SkString           extendedBackend;
        SkString           extendedOptions;
        SkString           simpleBackend;
        SkTArray<SkString> vias;

        SkString           tag(configs[i]);
        SkTArray<SkString> parts;
        SkStrSplit(tag.c_str(), "[", kStrict_SkStrSplitMode, &parts);
        if (parts.count() == 2) {
            SkTArray<SkString> parts2;
            SkStrSplit(parts[1].c_str(), "]", kStrict_SkStrSplitMode, &parts2);
            if (parts2.count() == 2 && parts2[1].isEmpty()) {
                SkStrSplit(parts[0].c_str(), "-", kStrict_SkStrSplitMode, &vias);
                if (vias.count()) {
                    extendedBackend = vias[vias.count() - 1];
                    vias.pop_back();
                } else {
                    extendedBackend = parts[0];
                }
                extendedOptions = parts2[0];
                simpleBackend.printf("%s[%s]", extendedBackend.c_str(), extendedOptions.c_str());
            }
        }

        if (extendedBackend.isEmpty()) {
            simpleBackend = tag;
            SkStrSplit(tag.c_str(), "-", kStrict_SkStrSplitMode, &vias);
            if (vias.count()) {
                simpleBackend = vias[vias.count() - 1];
                vias.pop_back();
            }
            for (auto& predefinedConfig : gPredefinedConfigs) {
                if (simpleBackend.equals(predefinedConfig.predefinedConfig)) {
                    extendedBackend = predefinedConfig.backend;
                    extendedOptions = predefinedConfig.options;
                    break;
                }
            }
        }
        SkCommandLineConfig* parsedConfig = nullptr;
        if (extendedBackend.equals("gpu")) {
            parsedConfig = parse_command_line_config_gpu(tag, vias, extendedOptions);
        }
        if (extendedBackend.equals("svg")) {
            parsedConfig = parse_command_line_config_svg(tag, vias, extendedOptions);
        }
        if (!parsedConfig) {
            parsedConfig = new SkCommandLineConfig(tag, simpleBackend, vias);
        }
        outResult->emplace_back(parsedConfig);
    }
}
