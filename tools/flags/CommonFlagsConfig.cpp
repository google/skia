/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/flags/CommonFlagsConfig.h"

#include "include/core/SkImageInfo.h"
#include "include/core/SkSurfaceProps.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkStringUtils.h"
#include "src/core/SkSurfacePriv.h"
#include "src/core/SkTHash.h"

#include <stdlib.h>
#include <string_view>
#include <unordered_map>

using namespace skia_private;
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
    { "glf16",                 "gpu", "api=gl,color=f16" },
    { "glf16norm",             "gpu", "api=gl,color=f16norm" },
    { "glsrgba",               "gpu", "api=gl,color=srgba" },
    { "gl1010102",             "gpu", "api=gl,color=1010102" },
    { "gles",                  "gpu", "api=gles" },
    { "glesf16",               "gpu", "api=gles,color=f16" },
    { "glessrgba",             "gpu", "api=gles,color=srgba" },
    { "gles1010102",           "gpu", "api=gles,color=1010102" },
    { "glesfakev2",            "gpu", "api=glesfakev2" },
    { "gldmsaa",               "gpu", "api=gl,dmsaa=true" },
    { "glesdmsaa",             "gpu", "api=gles,dmsaa=true" },
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
    { "glr8",                  "gpu", "api=gl,color=r8" },
    { "glnostencils",          "gpu", "api=gl,stencils=false" },
    { "gldft",                 "gpu", "api=gl,dit=true" },
    { "glesdft",               "gpu", "api=gles,dit=true" },
    { "glslug",                "gpu", "api=gl,slug=true" },
    { "glserializeslug",       "gpu", "api=gl,serializeSlug=true" },
    { "glremoteslug",          "gpu", "api=gl,remoteSlug=true" },
    { "gltestpersistentcache", "gpu", "api=gl,testPersistentCache=1" },
    { "gltestglslcache",       "gpu", "api=gl,testPersistentCache=2" },
    { "gltestprecompile",      "gpu", "api=gl,testPrecompile=true" },
    { "glestestprecompile",    "gpu", "api=gles,testPrecompile=true" },
    { "glddl",                 "gpu", "api=gl,useDDLSink=true" },
    { "glreducedshaders",      "gpu", "api=gl,reducedShaders=true" },
    { "glesreducedshaders",    "gpu", "api=gles,reducedShaders=true" },
    { "angle_d3d11_es2",       "gpu", "api=angle_d3d11_es2" },
    { "angle_d3d11_es3",       "gpu", "api=angle_d3d11_es3" },
    { "angle_d3d9_es2",        "gpu", "api=angle_d3d9_es2" },
    { "angle_d3d11_es2_msaa4", "gpu", "api=angle_d3d11_es2,samples=4" },
    { "angle_d3d11_es2_msaa8", "gpu", "api=angle_d3d11_es2,samples=8" },
    { "angle_d3d11_es2_dmsaa", "gpu", "api=angle_d3d11_es2,dmsaa=true" },
    { "angle_d3d11_es3_msaa4", "gpu", "api=angle_d3d11_es3,samples=4" },
    { "angle_d3d11_es3_msaa8", "gpu", "api=angle_d3d11_es3,samples=8" },
    { "angle_d3d11_es3_dmsaa", "gpu", "api=angle_d3d11_es3,dmsaa=true" },
    { "angle_gl_es2",          "gpu", "api=angle_gl_es2" },
    { "angle_gl_es3",          "gpu", "api=angle_gl_es3" },
    { "angle_gl_es2_msaa4",    "gpu", "api=angle_gl_es2,samples=4" },
    { "angle_gl_es2_msaa8",    "gpu", "api=angle_gl_es2,samples=8" },
    { "angle_gl_es2_dmsaa",    "gpu", "api=angle_gl_es2,dmsaa=true" },
    { "angle_gl_es3_msaa4",    "gpu", "api=angle_gl_es3,samples=4" },
    { "angle_gl_es3_msaa8",    "gpu", "api=angle_gl_es3,samples=8" },
    { "angle_gl_es3_dmsaa",    "gpu", "api=angle_gl_es3,dmsaa=true" },
    { "angle_mtl_es2",         "gpu", "api=angle_mtl_es2" },
    { "angle_mtl_es3",         "gpu", "api=angle_mtl_es3" },
    { "mock",                  "gpu", "api=mock" },
#ifdef SK_VULKAN
    { "vk",                    "gpu", "api=vulkan" },
    { "vkf16",                 "gpu", "api=vulkan,color=f16" },
    { "vk1010102",             "gpu", "api=vulkan,color=1010102" },
    { "vkdmsaa",               "gpu", "api=vulkan,dmsaa=true" },
    { "vknostencils",          "gpu", "api=vulkan,stencils=false" },
    { "vkmsaa4",               "gpu", "api=vulkan,samples=4" },
    { "vkmsaa8",               "gpu", "api=vulkan,samples=8" },
    { "vkbetex",               "gpu", "api=vulkan,surf=betex" },
    { "vkbert",                "gpu", "api=vulkan,surf=bert" },
    { "vktestpersistentcache", "gpu", "api=vulkan,testPersistentCache=1" },
    { "vkddl",                 "gpu", "api=vulkan,useDDLSink=true" },
#endif
#ifdef SK_METAL
    { "mtl",                   "gpu", "api=metal" },
    { "mtlf16",                "gpu", "api=metal,color=f16" },
    { "mtlf16norm",            "gpu", "api=metal,color=f16norm" },
    { "mtlsrgba",              "gpu", "api=metal,color=srgba"},
    { "mtl1010102",            "gpu", "api=metal,color=1010102" },
    { "mtl_dmsaa",             "gpu", "api=metal,dmsaa=true" },
    { "mtlmsaa4",              "gpu", "api=metal,samples=4" },
    { "mtlmsaa8",              "gpu", "api=metal,samples=8" },
    { "mtlddl",                "gpu", "api=metal,useDDLSink=true" },
    { "mtltestprecompile",     "gpu", "api=metal,testPrecompile=true" },
    { "mtlreducedshaders",     "gpu", "api=metal,reducedShaders=true" },
#endif
#ifdef SK_DIRECT3D
    { "d3d",                   "gpu", "api=direct3d" },
    { "d3dmsaa4",              "gpu", "api=direct3d,samples=4" },
    { "d3dmsaa8",              "gpu", "api=direct3d,samples=8" },
#endif

#if defined(SK_GRAPHITE)
#ifdef SK_DIRECT3D
    { "grd3d",                 "graphite", "api=direct3d" },
#endif
#ifdef SK_DAWN
    { "grdawn_fakeWGPU",       "graphite", "api=dawn_mtl,fakeWGPU=true" },
    { "grdawn_d3d11",          "graphite", "api=dawn_d3d11" },
    { "grdawn_d3d12",          "graphite", "api=dawn_d3d12" },
    { "grdawn_mtl",            "graphite", "api=dawn_mtl" },
    { "grdawn_vk",             "graphite", "api=dawn_vk" },
    { "grdawn_gl",             "graphite", "api=dawn_gl" },
    { "grdawn_gles",           "graphite", "api=dawn_gles" },
#if defined(SK_ENABLE_PRECOMPILE)
    { "grdawn_mtlprecompile",  "graphite", "api=dawn_mtl,precompile=true" },
    { "grdawn_vkprecompile",   "graphite", "api=dawn_vk, precompile=true" },
#endif
#endif
#ifdef SK_METAL
    { "grmtl",                 "graphite", "api=metal" },
    { "grmtlf16",              "graphite", "api=metal,color=f16" },
    { "grmtlf16norm",          "graphite", "api=metal,color=f16norm" },
    { "grmtlsrgba",            "graphite", "api=metal,color=srgba"},
    { "grmtl1010102",          "graphite", "api=metal,color=1010102" },
#if defined(SK_ENABLE_PRECOMPILE)
    { "grmtlprecompile",       "graphite", "api=metal,precompile=true" },
#endif
#endif
#ifdef SK_VULKAN
    { "grvk",                  "graphite", "api=vulkan" },
#if defined(SK_ENABLE_PRECOMPILE)
    { "grvkprecompile",        "graphite", "api=vulkan,precompile=true" },
#endif
#endif
#endif

};
// clang-format on

static const char configHelp[] =
        "Options: 565 4444 8888 rgba bgra rgbx 1010102 101010x bgra1010102 bgr101010x f16 f16norm "
        "f32 nonrendering null pdf pdfa pdf300 skp svg xps";

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
        "gpu[api=string,color=string,dit=bool,dmsaa=bool,samples=int]\n"
        "\tapi\ttype: string\trequired\n"
        "\t    Select graphics API to use with gpu backend.\n"
        "\t    Options:\n"
        "\t\tgl    \t\t\tUse OpenGL.\n"
        "\t\tgles  \t\t\tUse OpenGL ES.\n"
        "\t\tglesfakev2  \t\t\tUse OpenGL ES with version faked as 2.0.\n"
        "\t\tnullgl \t\t\tUse null OpenGL.\n"
        "\t\tangle_d3d9_es2\t\tUse OpenGL ES2 on the ANGLE Direct3D9 backend.\n"
        "\t\tangle_d3d11_es2\t\tUse OpenGL ES2 on the ANGLE Direct3D11 backend.\n"
        "\t\tangle_d3d11_es3\t\tUse OpenGL ES3 on the ANGLE Direct3D11 backend.\n"
        "\t\tangle_gl_es2\t\tUse OpenGL ES2 on the ANGLE OpenGL backend.\n"
        "\t\tangle_gl_es3\t\tUse OpenGL ES3 on the ANGLE OpenGL backend.\n"
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
        "\t\tf16\t\t\t16-bit floating point.\n"
        "\tdit\ttype: bool\tdefault: false.\n"
        "\t    Use device independent text.\n"
        "\tdmsaa\ttype: bool\tdefault: false.\n"
        "\t    Use internal MSAA to render to non-MSAA surfaces.\n"
        "\tsamples\ttype: int\tdefault: 0.\n"
        "\t    Use multisampling with N samples.\n"
        "\tstencils\ttype: bool\tdefault: true.\n"
        "\t    Allow the use of stencil buffers.\n"
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

SkCommandLineConfig::SkCommandLineConfig(const SkString& tag,
                                         const SkString& backend,
                                         const TArray<SkString>& viaParts)
        : fTag(tag), fBackend(backend) {
    static const auto* kColorSpaces = new std::unordered_map<std::string_view, SkColorSpace*>{
        {"narrow", // 'narrow' has a gamut narrower than sRGB, and different transfer function.
         SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2, gNarrow_toXYZD50).release()},
        {"srgb",
         SkColorSpace::MakeSRGB().release()},
        {"srgb2",  // The same as "srgb" but works around ignoring prior images in Gold
         SkColorSpace::MakeSRGB().release()},
        {"linear",
         SkColorSpace::MakeSRGBLinear().release()},
        {"p3",
         SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDisplayP3).release()},
        {"spin",
         SkColorSpace::MakeSRGB()->makeColorSpin().release()},
        {"rec2020",
         SkColorSpace::MakeRGB(SkNamedTransferFn::kRec2020, SkNamedGamut::kRec2020).release()},
    };

    // Strip off any via parts that refer to color spaces (and remember the last one we see)
    for (const SkString& via : viaParts) {
        auto it = kColorSpaces->find(via.c_str());
        if (it == kColorSpaces->end()) {
            fViaParts.push_back(via);
        } else {
            fColorSpace = sk_ref_sp(it->second);
        }
    }
}

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
                                 SkCommandLineConfigGpu::ContextType* outContextType,
                                 bool*                                outFakeGLESVersion2) {
    *outFakeGLESVersion2 = false;
    if (value.equals("gl")) {
        *outContextType = skgpu::ContextType::kGL;
        return true;
    }
    if (value.equals("gles")) {
        *outContextType = skgpu::ContextType::kGLES;
        return true;
    }
    if (value.equals("glesfakev2")) {
        *outContextType = skgpu::ContextType::kGLES;
        *outFakeGLESVersion2 = true;
        return true;
    }
    if (value.equals("angle_d3d9_es2")) {
        *outContextType = skgpu::ContextType::kANGLE_D3D9_ES2;
        return true;
    }
    if (value.equals("angle_d3d11_es2")) {
        *outContextType = skgpu::ContextType::kANGLE_D3D11_ES2;
        return true;
    }
    if (value.equals("angle_d3d11_es3")) {
        *outContextType = skgpu::ContextType::kANGLE_D3D11_ES3;
        return true;
    }
    if (value.equals("angle_gl_es2")) {
        *outContextType = skgpu::ContextType::kANGLE_GL_ES2;
        return true;
    }
    if (value.equals("angle_gl_es3")) {
        *outContextType = skgpu::ContextType::kANGLE_GL_ES3;
        return true;
    }
    if (value.equals("angle_mtl_es2")) {
        *outContextType = skgpu::ContextType::kANGLE_Metal_ES2;
        return true;
    }
    if (value.equals("angle_mtl_es3")) {
        *outContextType = skgpu::ContextType::kANGLE_Metal_ES3;
        return true;
    }
    if (value.equals("mock")) {
        *outContextType = skgpu::ContextType::kMock;
        return true;
    }
#ifdef SK_VULKAN
    if (value.equals("vulkan")) {
        *outContextType = skgpu::ContextType::kVulkan;
        return true;
    }
#endif
#ifdef SK_METAL
    if (value.equals("metal")) {
        *outContextType = skgpu::ContextType::kMetal;
        return true;
    }
#endif
#ifdef SK_DIRECT3D
    if (value.equals("direct3d")) {
        *outContextType = skgpu::ContextType::kDirect3D;
        return true;
    }
#endif
    return false;
}

static bool parse_option_gpu_color(const SkString& value,
                                   SkColorType*    outColorType,
                                   SkAlphaType*    alphaType) {
    // We always use premul unless the color type is 565.
    *alphaType = kPremul_SkAlphaType;

    if (value.equals("8888")) {
        *outColorType  = kRGBA_8888_SkColorType;
    } else if (value.equals("888x")) {
        *outColorType  = kRGB_888x_SkColorType;
    } else if (value.equals("bgra8")) {
        *outColorType  = kBGRA_8888_SkColorType;
    } else if (value.equals("4444")) {
        *outColorType  = kARGB_4444_SkColorType;
    } else if (value.equals("565")) {
        *outColorType  = kRGB_565_SkColorType;
        *alphaType     = kOpaque_SkAlphaType;
    } else if (value.equals("1010102")) {
        *outColorType  = kRGBA_1010102_SkColorType;
    } else if (value.equals("f16")) {
        *outColorType  = kRGBA_F16_SkColorType;
    } else if (value.equals("f16norm")) {
        *outColorType  = kRGBA_F16Norm_SkColorType;
    } else if (value.equals("srgba")) {
        *outColorType = kSRGBA_8888_SkColorType;
    } else if (value.equals("r8")) {
        *outColorType = kR8_unorm_SkColorType;
        *alphaType = kOpaque_SkAlphaType;
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
        TArray<SkString> optionParts;
        SkStrSplit(optionsString.c_str(), ",", kStrict_SkStrSplitMode, &optionParts);
        for (int i = 0; i < optionParts.size(); ++i) {
            TArray<SkString> keyValueParts;
            SkStrSplit(optionParts[i].c_str(), "=", kStrict_SkStrSplitMode, &keyValueParts);
            if (keyValueParts.size() != 2) {
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

    bool get_option_gpu_color(const char*  optionKey,
                              SkColorType* outColorType,
                              SkAlphaType* alphaType,
                              bool         optional = true) const {
        SkString* optionValue = fOptionsMap.find(SkString(optionKey));
        if (optionValue == nullptr) {
            return optional;
        }
        return parse_option_gpu_color(*optionValue, outColorType, alphaType);
    }

    bool get_option_gpu_api(const char*                          optionKey,
                            SkCommandLineConfigGpu::ContextType* outContextType,
                            bool*                                outFakeGLESVersion2,
                            bool                                 optional = true) const {
        SkString* optionValue = fOptionsMap.find(SkString(optionKey));
        if (optionValue == nullptr) {
            return optional;
        }
        return parse_option_gpu_api(*optionValue, outContextType, outFakeGLESVersion2);
    }

#if defined(SK_GRAPHITE)
    bool get_option_graphite_api(const char*                               optionKey,
                                 SkCommandLineConfigGraphite::ContextType* outContextType) const {
        SkString* optionValue = fOptionsMap.find(SkString(optionKey));
        if (optionValue == nullptr) {
            return false;
        }
#ifdef SK_DAWN
        if (optionValue->equals("dawn_d3d11")) {
            *outContextType = skgpu::ContextType::kDawn_D3D11;
            return true;
        }
        if (optionValue->equals("dawn_d3d12")) {
            *outContextType = skgpu::ContextType::kDawn_D3D12;
            return true;
        }
        if (optionValue->equals("dawn_mtl")) {
            *outContextType = skgpu::ContextType::kDawn_Metal;
            return true;
        }
        if (optionValue->equals("dawn_vk")) {
            *outContextType = skgpu::ContextType::kDawn_Vulkan;
            return true;
        }
        if (optionValue->equals("dawn_gl")) {
            *outContextType = skgpu::ContextType::kDawn_OpenGL;
            return true;
        }
        if (optionValue->equals("dawn_gles")) {
            *outContextType = skgpu::ContextType::kDawn_OpenGLES;
            return true;
        }
#endif
#ifdef SK_DIRECT3D
        if (optionValue->equals("direct3d")) {
            *outContextType = skgpu::ContextType::kDirect3D;
            return true;
        }
#endif
#ifdef SK_METAL
        if (optionValue->equals("metal")) {
            *outContextType = skgpu::ContextType::kMetal;
            return true;
        }
#endif
#ifdef SK_VULKAN
        if (optionValue->equals("vulkan")) {
            *outContextType = skgpu::ContextType::kVulkan;
            return true;
        }
#endif

        return false;
    }
#endif

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
    THashMap<SkString, SkString> fOptionsMap;
};

SkCommandLineConfigGpu::SkCommandLineConfigGpu(const SkString&         tag,
                                               const TArray<SkString>& viaParts,
                                               ContextType             contextType,
                                               bool                    fakeGLESVersion2,
                                               uint32_t                surfaceFlags,
                                               int                     samples,
                                               SkColorType             colorType,
                                               SkAlphaType             alphaType,
                                               bool                    useStencilBuffers,
                                               int                     testPersistentCache,
                                               bool                    testPrecompile,
                                               bool                    useDDLSink,
                                               bool                    slug,
                                               bool                    serializeSlug,
                                               bool                    remoteSlug,
                                               bool                    reducedShaders,
                                               SurfType                surfType)
        : SkCommandLineConfig(tag, SkString("gpu"), viaParts)
        , fContextType(contextType)
        , fContextOverrides(ContextOverrides::kNone)
        , fSurfaceFlags(surfaceFlags)
        , fSamples(samples)
        , fColorType(colorType)
        , fAlphaType(alphaType)
        , fTestPersistentCache(testPersistentCache)
        , fTestPrecompile(testPrecompile)
        , fUseDDLSink(useDDLSink)
        , fSlug(slug)
        , fSerializeSlug(serializeSlug)
        , fRemoteSlug(remoteSlug)
        , fReducedShaders(reducedShaders)
        , fSurfType(surfType) {
    if (!useStencilBuffers) {
        fContextOverrides |= ContextOverrides::kAvoidStencilBuffers;
    }
    if (fakeGLESVersion2) {
        fContextOverrides |= ContextOverrides::kFakeGLESVersionAs2;
    }
    if (reducedShaders) {
        fContextOverrides |= ContextOverrides::kReducedShaders;
    }
}

SkCommandLineConfigGpu* parse_command_line_config_gpu(const SkString&         tag,
                                                      const TArray<SkString>& vias,
                                                      const SkString&         options) {
    // Defaults for GPU backend.
    SkCommandLineConfigGpu::ContextType contextType         = skgpu::ContextType::kGL;
    bool                                useDIText           = false;
    bool                                useDMSAA            = false;
    int                                 samples             = 1;
    SkColorType                         colorType           = kRGBA_8888_SkColorType;
    SkAlphaType                         alphaType           = kPremul_SkAlphaType;
    bool                                useStencils         = true;
    int                                 testPersistentCache = 0;
    bool                                testPrecompile      = false;
    bool                                useDDLs             = false;
    bool                                slug                = false;
    bool                                serializeSlug       = false;
    bool                                remoteSlug          = false;
    bool                                reducedShaders      = false;
    bool                                fakeGLESVersion2    = false;
    SkCommandLineConfigGpu::SurfType    surfType = SkCommandLineConfigGpu::SurfType::kDefault;

    bool            parseSucceeded = false;
    ExtendedOptions extendedOptions(options, &parseSucceeded);
    if (!parseSucceeded) {
        return nullptr;
    }

    bool validOptions =
            extendedOptions.get_option_gpu_api("api", &contextType, &fakeGLESVersion2, false) &&
            extendedOptions.get_option_bool("dit", &useDIText) &&
            extendedOptions.get_option_int("samples", &samples) &&
            extendedOptions.get_option_bool("dmsaa", &useDMSAA) &&
            extendedOptions.get_option_gpu_color("color", &colorType, &alphaType) &&
            extendedOptions.get_option_bool("stencils", &useStencils) &&
            extendedOptions.get_option_int("testPersistentCache", &testPersistentCache) &&
            extendedOptions.get_option_bool("testPrecompile", &testPrecompile) &&
            extendedOptions.get_option_bool("useDDLSink", &useDDLs) &&
            extendedOptions.get_option_bool("slug", &slug) &&
            extendedOptions.get_option_bool("serializeSlug", &serializeSlug) &&
            extendedOptions.get_option_bool("remoteSlug", &remoteSlug) &&
            extendedOptions.get_option_bool("reducedShaders", &reducedShaders) &&
            extendedOptions.get_option_gpu_surf_type("surf", &surfType);

    if (!validOptions) {
        return nullptr;
    }

    uint32_t surfaceFlags = 0;
    if (useDIText) {
        surfaceFlags |= SkSurfaceProps::kUseDeviceIndependentFonts_Flag;
    }
    if (useDMSAA) {
        surfaceFlags |= SkSurfaceProps::kDynamicMSAA_Flag;
    }

    return new SkCommandLineConfigGpu(tag,
                                      vias,
                                      contextType,
                                      fakeGLESVersion2,
                                      surfaceFlags,
                                      samples,
                                      colorType,
                                      alphaType,
                                      useStencils,
                                      testPersistentCache,
                                      testPrecompile,
                                      useDDLs,
                                      slug,
                                      serializeSlug,
                                      remoteSlug,
                                      reducedShaders,
                                      surfType);
}

#if defined(SK_GRAPHITE)

SkCommandLineConfigGraphite* parse_command_line_config_graphite(const SkString& tag,
                                                                const TArray<SkString>& vias,
                                                                const SkString& options) {
    using ContextType = skgpu::ContextType;

    ContextType contextType    = skgpu::ContextType::kMetal;
    SkColorType colorType      = kRGBA_8888_SkColorType;
    SkAlphaType alphaType      = kPremul_SkAlphaType;
    bool        testPrecompile = false;

    skiatest::graphite::TestOptions testOptions;

    bool parseSucceeded = false;
    ExtendedOptions extendedOptions(options, &parseSucceeded);
    if (!parseSucceeded) {
        return nullptr;
    }

    bool fakeWGPU = false;
    bool validOptions = extendedOptions.get_option_graphite_api("api", &contextType) &&
                        extendedOptions.get_option_gpu_color("color", &colorType, &alphaType) &&
                        extendedOptions.get_option_bool("fakeWGPU", &fakeWGPU) &&
                        extendedOptions.get_option_bool("precompile", &testPrecompile);
    if (!validOptions) {
        return nullptr;
    }

    auto surfaceType = SkCommandLineConfigGraphite::SurfaceType::kDefault;
    if (fakeWGPU) {
        testOptions.fNeverYieldToWebGPU = true;
        surfaceType = SkCommandLineConfigGraphite::SurfaceType::kWrapTextureView;
    }

    return new SkCommandLineConfigGraphite(tag,
                                           vias,
                                           contextType,
                                           surfaceType,
                                           testOptions,
                                           colorType,
                                           alphaType,
                                           testPrecompile);
}

#endif

SkCommandLineConfigSvg::SkCommandLineConfigSvg(const SkString& tag,
                                               const TArray<SkString>& viaParts,
                                               int pageIndex)
        : SkCommandLineConfig(tag, SkString("svg"), viaParts), fPageIndex(pageIndex) {}

SkCommandLineConfigSvg* parse_command_line_config_svg(const SkString& tag,
                                                      const TArray<SkString>& vias,
                                                      const SkString& options) {
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
    outResult->clear();
    for (int i = 0; i < configs.size(); ++i) {
        SkString         extendedBackend;
        SkString         extendedOptions;
        SkString         simpleBackend;
        TArray<SkString> vias;

        SkString         tag(configs[i]);
        TArray<SkString> parts;
        SkStrSplit(tag.c_str(), "[", kStrict_SkStrSplitMode, &parts);
        if (parts.size() == 2) {
            TArray<SkString> parts2;
            SkStrSplit(parts[1].c_str(), "]", kStrict_SkStrSplitMode, &parts2);
            if (parts2.size() == 2 && parts2[1].isEmpty()) {
                SkStrSplit(parts[0].c_str(), "-", kStrict_SkStrSplitMode, &vias);
                if (vias.size()) {
                    extendedBackend = vias[vias.size() - 1];
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
            if (vias.size()) {
                simpleBackend = vias[vias.size() - 1];
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
#if defined(SK_GRAPHITE)
        if (extendedBackend.equals("graphite")) {
            parsedConfig = parse_command_line_config_graphite(tag, vias, extendedOptions);
        }
#endif
        if (extendedBackend.equals("svg")) {
            parsedConfig = parse_command_line_config_svg(tag, vias, extendedOptions);
        }
        if (!parsedConfig) {
            parsedConfig = new SkCommandLineConfig(tag, simpleBackend, vias);
        }
        outResult->emplace_back(parsedConfig);
    }
}
