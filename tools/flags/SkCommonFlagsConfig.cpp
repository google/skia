/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCommonFlagsConfig.h"

#include <stdlib.h>

static const char defaultConfigs[] =
    "565 8888 gpu nonrendering"
#if SK_ANGLE
#ifdef SK_BUILD_FOR_WIN
    " angle"
#endif
#endif
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    " hwui"
#endif
    ;

static const char configHelp[] =
    "Options: 565 8888 debug gpu gpudebug gpudft gpunull "
    "msaa16 msaa4 nonrendering null nullgpu nvprmsaa16 nvprmsaa4 "
    "pdf pdf_poppler skp svg xps"
#if SK_ANGLE
#ifdef SK_BUILD_FOR_WIN
    " angle"
#endif
    " angle-gl"
#endif
#if SK_COMMAND_BUFFER
    " commandbuffer"
#endif
#if SK_MESA
    " mesa"
#endif
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    " hwui"
#endif
    " or use extended form 'backend(option=value,...)'.\n";

static const char configExtendedHelp[] =
    "Extended form: 'backend(option=value,...)'\n\n"
    "Possible backends and options:\n"
#if SK_SUPPORT_GPU
    "\n"
    "gpu(api=string,dit=bool,nvpr=bool,samples=int)\tGPU backend\n"
    "\tapi\ttype: string\tdefault: native.\n"
    "\t    Select graphics API to use with gpu backend.\n"
    "\t    Options:\n"
    "\t\tnative\t\t\tUse platform default OpenGL or OpenGL ES backend.\n"
    "\t\tgl    \t\t\tUse OpenGL.\n"
    "\t\tgles  \t\t\tUse OpenGL ES.\n"
    "\t\tdebug \t\t\tUse debug OpenGL.\n"
    "\t\tnull  \t\t\tUse null OpenGL.\n"
#if SK_ANGLE
#ifdef SK_BUILD_FOR_WIN
    "\t\tangle\t\t\tUse ANGLE DirectX.\n"
#endif
    "\t\tangle-gl\t\t\tUse ANGLE OpenGL.\n"
#endif
#if SK_COMMAND_BUFFER
    "\t\tcommandbuffer\t\tUse command buffer.\n"
#endif
#if SK_MESA
    "\t\tmesa\t\t\tUse MESA.\n"
#endif
    "\tdit\ttype: bool\tdefault: false.\n"
    "\t    Use device independent text.\n"
    "\tnvpr\ttype: bool\tdefault: false.\n"
    "\t    Use NV_path_rendering OpenGL and OpenGL ES extension.\n"
    "\tsamples\ttype: int\tdefault: 0.\n"
    "\t    Use multisampling with N samples.\n"
    "\n"
    "Predefined configs:\n\n"
    "\tgpu      \t= gpu()\n"
    "\tmsaa4    \t= gpu(samples=4)\n"
    "\tmsaa16   \t= gpu(samples=16)\n"
    "\tnvprmsaa4\t= gpu(nvpr=true,samples=4)\n"
    "\tnvprmsaa16\t= gpu(nvpr=true,samples=16)\n"
    "\tgpudft    \t= gpu(dit=true)\n"
    "\tgpudebug  \t= gpu(api=debug)\n"
    "\tgpunull   \t= gpu(api=null)\n"
    "\tdebug     \t= gpu(api=debug)\n"
    "\tnullgpu   \t= gpu(api=null)\n"
#if SK_ANGLE
#ifdef SK_BUILD_FOR_WIN
    "\tangle     \t= gpu(api=angle)\n"
#endif
    "\tangle-gl  \t= gpu(api=angle-gl)\n"
#endif
#if SK_COMMAND_BUFFER
    "\tcommandbuffer\t= gpu(api=commandbuffer)\n"
#endif
#if SK_MESA
    "\tmesa      \t= gpu(api=mesa)\n"
#endif
#endif
    ;

DEFINE_extended_string(config, defaultConfigs, configHelp, configExtendedHelp);

static const struct {
    const char* predefinedConfig;
    const char* backend;
    const char* options;
} gPredefinedConfigs[] = {
#if SK_SUPPORT_GPU
    { "gpu",        "gpu", "" },
    { "msaa4",      "gpu", "samples=4" },
    { "msaa16",     "gpu", "samples=16" },
    { "nvprmsaa4",  "gpu", "nvpr=true,samples=4,dit=true" },
    { "nvprmsaa16", "gpu", "nvpr=true,samples=16,dit=true" },
    { "gpudft",     "gpu", "dit=true" },
    { "gpudebug",   "gpu", "api=debug" },
    { "gpunull",    "gpu", "api=null" },
    { "debug",      "gpu", "api=debug" },
    { "nullgpu",    "gpu", "api=null" }
#if SK_ANGLE
#ifdef SK_BUILD_FOR_WIN
    , { "angle",      "gpu", "api=angle" }
#endif
    , { "angle-gl",   "gpu", "api=angle-gl" }
#endif
#if SK_COMMAND_BUFFER
    , { "commandbuffer", "gpu", "api=commandbuffer" }
#endif
#if SK_MESA
    , { "mesa", "gpu", "api=mesa" }
#endif
#else
    { "", "", "" }
#endif
};

SkCommandLineConfig::SkCommandLineConfig(const SkString& tag, const SkString& backend,
                                         const SkTArray<SkString>& viaParts)
        : fTag(tag)
        , fBackend(backend)
        , fViaParts(viaParts) {
}
SkCommandLineConfig::~SkCommandLineConfig() {
}

#if SK_SUPPORT_GPU
SkCommandLineConfigGpu::SkCommandLineConfigGpu(
    const SkString& tag, const SkTArray<SkString>& viaParts,
    ContextType contextType, bool useNVPR, bool useDIText, int samples)
        : SkCommandLineConfig(tag, SkString("gpu"), viaParts)
        , fContextType(contextType)
        , fUseNVPR(useNVPR)
        , fUseDIText(useDIText)
        , fSamples(samples) {
}
static bool parse_option_int(const SkString& value, int* outInt) {
    if (value.isEmpty()) {
        return false;
    }
    char* endptr = nullptr;
    long intValue = strtol(value.c_str(), &endptr, 10);
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
static bool parse_option_gpu_api(const SkString& value,
                                 SkCommandLineConfigGpu::ContextType* outContextType) {
    if (value.equals("native")) {
        *outContextType = GrContextFactory::kNative_GLContextType;
        return true;
    }
    if (value.equals("gl")) {
        *outContextType = GrContextFactory::kGL_GLContextType;
        return true;
    }
    if (value.equals("gles")) {
        *outContextType = GrContextFactory::kGLES_GLContextType;
        return true;
    }
    if (value.equals("debug")) {
        *outContextType = GrContextFactory::kDebug_GLContextType;
        return true;
    }
    if (value.equals("null")) {
        *outContextType = GrContextFactory::kNull_GLContextType;
        return true;
    }
#if SK_ANGLE
#ifdef SK_BUILD_FOR_WIN
    if (value.equals("angle")) {
        *outContextType = GrContextFactory::kANGLE_GLContextType;
        return true;
    }
#endif
    if (value.equals("angle-gl")) {
        *outContextType = GrContextFactory::kANGLE_GL_GLContextType;
        return true;
    }
#endif
#if SK_COMMAND_BUFFER
    if (value.equals("commandbuffer")) {
        *outContextType = GrContextFactory::kCommandBuffer_GLContextType;
        return true;
    }
#endif
#if SK_MESA
    if (value.equals("mesa")) {
        *outContextType = GrContextFactory::kMESA_GLContextType;
        return true;
    }
#endif
    return false;
}

SkCommandLineConfigGpu* parse_command_line_config_gpu(const SkString& tag,
                                                      const SkTArray<SkString>& vias,
                                                      const SkString& options) {
    // Defaults for GPU backend.
    bool seenAPI = false;
    SkCommandLineConfigGpu::ContextType contextType = GrContextFactory::kNative_GLContextType;
    bool seenUseNVPR = false;
    bool useNVPR = false;
    bool seenUseDIText =false;
    bool useDIText = false;
    bool seenSamples = false;
    int samples = 0;

    SkTArray<SkString> optionParts;
    SkStrSplit(options.c_str(), ",", kStrict_SkStrSplitMode, &optionParts);
    for (int i = 0; i < optionParts.count(); ++i) {
        SkTArray<SkString> keyValueParts;
        SkStrSplit(optionParts[i].c_str(), "=", kStrict_SkStrSplitMode, &keyValueParts);
        if (keyValueParts.count() != 2) {
            return nullptr;
        }
        const SkString& key = keyValueParts[0];
        const SkString& value = keyValueParts[1];
        bool valueOk = false;
        if (key.equals("api") && !seenAPI) {
            valueOk = parse_option_gpu_api(value, &contextType);
            seenAPI = true;
        } else if (key.equals("nvpr") && !seenUseNVPR) {
            valueOk = parse_option_bool(value, &useNVPR);
            seenUseNVPR = true;
        } else if (key.equals("dit") && !seenUseDIText) {
            valueOk = parse_option_bool(value, &useDIText);
            seenUseDIText = true;
        } else if (key.equals("samples") && !seenSamples) {
            valueOk = parse_option_int(value, &samples);
            seenSamples = true;
        }
        if (!valueOk) {
            return nullptr;
        }
    }
    return new SkCommandLineConfigGpu(tag, vias, contextType, useNVPR, useDIText, samples);
}
#endif

void ParseConfigs(const SkCommandLineFlags::StringArray& configs,
                  SkCommandLineConfigArray* outResult) {
    outResult->reset();
    for (int i = 0; i < configs.count(); ++i) {
        SkString extendedBackend;
        SkString extendedOptions;
        SkString simpleBackend;
        SkTArray<SkString> vias;

        SkString tag(configs[i]);
        SkTArray<SkString> parts;
        SkStrSplit(tag.c_str(), "(", kStrict_SkStrSplitMode, &parts);
        if (parts.count() == 2) {
            SkTArray<SkString> parts2;
            SkStrSplit(parts[1].c_str(), ")", kStrict_SkStrSplitMode, &parts2);
            if (parts2.count() == 2 && parts2[1].isEmpty()) {
                SkStrSplit(parts[0].c_str(), "-", kStrict_SkStrSplitMode, &vias);
                if (vias.count()) {
                    extendedBackend = vias[vias.count() - 1];
                    vias.pop_back();
                } else {
                    extendedBackend = parts[0];
                }
                extendedOptions = parts2[0];
                simpleBackend.printf("%s(%s)", extendedBackend.c_str(), extendedOptions.c_str());
            }
        }

        if (extendedBackend.isEmpty()) {
            simpleBackend = tag;
            SkStrSplit(tag.c_str(), "-", kStrict_SkStrSplitMode, &vias);
            if (vias.count()) {
                simpleBackend = vias[vias.count() - 1];
                vias.pop_back();
            }
            // Note: no #if SK_ANGLE: this is a special rule in the via-tag grammar.
            if (vias.count() && simpleBackend.equals("gl") &&
                vias[vias.count() - 1].equals("angle")) {
                simpleBackend = "angle-gl";
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
#if SK_SUPPORT_GPU
        if (extendedBackend.equals("gpu")) {
            parsedConfig = parse_command_line_config_gpu(tag, vias, extendedOptions);
        }
#endif
        if (!parsedConfig) {
            parsedConfig = new SkCommandLineConfig(tag, simpleBackend, vias);
        }
        outResult->emplace_back(parsedConfig);
    }
}
