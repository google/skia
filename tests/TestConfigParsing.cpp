/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCommonFlagsConfig.h"
#include "SkColorSpace_Base.h"
#include "Test.h"
#include <initializer_list>

using sk_gpu_test::GrContextFactory;

namespace {
// The code
//   SkCommandLineFlags::StringArray FLAGS_config1 = make_string_array({"a", "b"})
// can be used to construct string array that one gets with command line flags.
// For example, the call above is equivalent of
//   DEFINE_string(config1, "a b", "");
// in cases where the default command line flag value ("a b") is used.
// make_string_array can be used to construct StringArray strings that have spaces in
// them.
SkCommandLineFlags::StringArray make_string_array(std::initializer_list<const char*> strings) {
    SkTArray<SkString> array;
    for (auto& s : strings) {
        array.push_back(SkString(s));
    }
    return SkCommandLineFlags::StringArray(array);
}
}
DEF_TEST(ParseConfigs_Gpu, reporter) {
    // Parses a normal config and returns correct "tag".
    // Gpu config defaults work.
    SkCommandLineFlags::StringArray config1 = make_string_array({"gpu"});
    SkCommandLineConfigArray configs;
    ParseConfigs(config1, &configs);

    REPORTER_ASSERT(reporter, configs.count() == 1);
    REPORTER_ASSERT(reporter, configs[0]->getTag().equals("gpu"));
    REPORTER_ASSERT(reporter, configs[0]->getViaParts().count() == 0);
#if SK_SUPPORT_GPU
    REPORTER_ASSERT(reporter, configs[0]->asConfigGpu());
    REPORTER_ASSERT(reporter, configs[0]->asConfigGpu()->getContextType()
                    == GrContextFactory::kNativeGL_ContextType);
    REPORTER_ASSERT(reporter, configs[0]->asConfigGpu()->getUseNVPR() == false);
    REPORTER_ASSERT(reporter, configs[0]->asConfigGpu()->getUseInstanced() == false);
    REPORTER_ASSERT(reporter, configs[0]->asConfigGpu()->getUseDIText() == false);
    REPORTER_ASSERT(reporter, configs[0]->asConfigGpu()->getSamples() == 0);
    REPORTER_ASSERT(reporter, configs[0]->asConfigGpu()->getColorType() == kRGBA_8888_SkColorType);
    REPORTER_ASSERT(reporter, configs[0]->asConfigGpu()->getColorSpace() == nullptr);
#endif
}

DEF_TEST(ParseConfigs_OutParam, reporter) {
    // Clears the out parameter.
    SkCommandLineFlags::StringArray config1 = make_string_array({"gpu"});
    SkCommandLineConfigArray configs;
    ParseConfigs(config1, &configs);
    REPORTER_ASSERT(reporter, configs.count() == 1);
    REPORTER_ASSERT(reporter, configs[0]->getTag().equals("gpu"));

    SkCommandLineFlags::StringArray config2 = make_string_array({"8888"});
    ParseConfigs(config2, &configs);
    REPORTER_ASSERT(reporter, configs.count() == 1);
    REPORTER_ASSERT(reporter, configs[0]->getTag().equals("8888"));

    SkCommandLineFlags::StringArray config3 = make_string_array({"gl"});
    ParseConfigs(config3, &configs);
    REPORTER_ASSERT(reporter, configs.count() == 1);
    REPORTER_ASSERT(reporter, configs[0]->getTag().equals("gl"));
}

DEF_TEST(ParseConfigs_DefaultConfigs, reporter) {
    // Parses all default configs and returns correct "tag".

    SkCommandLineFlags::StringArray config1 = make_string_array({
        "565", "8888", "debug", "gpu", "gpudebug", "gpudft", "gpunull", "msaa16", "msaa4",
        "nonrendering", "null", "nullgpu", "nvpr16", "nvpr4", "nvprdit16", "nvprdit4", "pdf", "skp",
        "svg", "xps", "angle_d3d11_es2", "angle_gl_es2", "commandbuffer", "mesa", "hwui", "gpuf16",
        "gpusrgb", "gl", "glnvpr4", "glnvprdit4", "glsrgb", "glmsaa4", "vk", "glinst", "glinst4",
        "glinstdit4", "glinst16", "glinstdit16", "esinst", "esinst4", "esinstdit4", "glwide",
        "glnarrow"
    });

    SkCommandLineConfigArray configs;
    ParseConfigs(config1, &configs);

    auto srgbColorSpace = SkColorSpace::MakeSRGB();

    REPORTER_ASSERT(reporter, configs.count() == config1.count());
    for (int i = 0; i < config1.count(); ++i) {
        REPORTER_ASSERT(reporter, configs[i]->getTag().equals(config1[i]));
        REPORTER_ASSERT(reporter, configs[i]->getViaParts().count() == 0);
    }
#if SK_SUPPORT_GPU
    REPORTER_ASSERT(reporter, !configs[0]->asConfigGpu());
    REPORTER_ASSERT(reporter, !configs[1]->asConfigGpu());
    REPORTER_ASSERT(reporter, configs[2]->asConfigGpu());
    REPORTER_ASSERT(reporter, configs[3]->asConfigGpu());
    REPORTER_ASSERT(reporter, configs[4]->asConfigGpu());
    REPORTER_ASSERT(reporter, configs[5]->asConfigGpu()->getUseDIText());
    REPORTER_ASSERT(reporter, configs[6]->asConfigGpu());
    REPORTER_ASSERT(reporter, configs[7]->asConfigGpu()->getSamples() == 16);
    REPORTER_ASSERT(reporter, configs[8]->asConfigGpu()->getSamples() == 4);
    REPORTER_ASSERT(reporter, !configs[9]->asConfigGpu());
    REPORTER_ASSERT(reporter, !configs[10]->asConfigGpu());
    REPORTER_ASSERT(reporter, configs[11]->asConfigGpu());
    REPORTER_ASSERT(reporter, configs[12]->asConfigGpu()->getSamples() == 16);
    REPORTER_ASSERT(reporter, configs[12]->asConfigGpu()->getUseNVPR());
    REPORTER_ASSERT(reporter, !configs[12]->asConfigGpu()->getUseDIText());
    REPORTER_ASSERT(reporter, configs[13]->asConfigGpu()->getSamples() == 4);
    REPORTER_ASSERT(reporter, configs[13]->asConfigGpu()->getUseNVPR());
    REPORTER_ASSERT(reporter, !configs[13]->asConfigGpu()->getUseDIText());
    REPORTER_ASSERT(reporter, configs[14]->asConfigGpu()->getSamples() == 16);
    REPORTER_ASSERT(reporter, configs[14]->asConfigGpu()->getUseNVPR());
    REPORTER_ASSERT(reporter, configs[14]->asConfigGpu()->getUseDIText());
    REPORTER_ASSERT(reporter, configs[15]->asConfigGpu()->getSamples() == 4);
    REPORTER_ASSERT(reporter, configs[15]->asConfigGpu()->getUseNVPR());
    REPORTER_ASSERT(reporter, configs[15]->asConfigGpu()->getUseDIText());
    REPORTER_ASSERT(reporter, !configs[16]->asConfigGpu());
    REPORTER_ASSERT(reporter, !configs[17]->asConfigGpu());
    REPORTER_ASSERT(reporter, !configs[18]->asConfigGpu());
    REPORTER_ASSERT(reporter, !configs[19]->asConfigGpu());
    REPORTER_ASSERT(reporter, !configs[24]->asConfigGpu());
    REPORTER_ASSERT(reporter, configs[25]->asConfigGpu()->getColorType() == kRGBA_F16_SkColorType);
    REPORTER_ASSERT(reporter, configs[25]->asConfigGpu()->getColorSpace());
    REPORTER_ASSERT(reporter, configs[25]->asConfigGpu()->getColorSpace()->gammaIsLinear());
    const SkMatrix44* srgbXYZ = as_CSB(srgbColorSpace)->toXYZD50();
    SkASSERT(srgbXYZ);
    const SkMatrix44* config25XYZ =
            as_CSB(configs[25]->asConfigGpu()->getColorSpace())->toXYZD50();
    SkASSERT(config25XYZ);
    REPORTER_ASSERT(reporter, *config25XYZ == *srgbXYZ);
    REPORTER_ASSERT(reporter, configs[26]->asConfigGpu()->getColorType() == kRGBA_8888_SkColorType);
    REPORTER_ASSERT(reporter, configs[26]->asConfigGpu()->getColorSpace() == srgbColorSpace.get());
    REPORTER_ASSERT(reporter, configs[41]->asConfigGpu()->getColorType() == kRGBA_F16_SkColorType);
    REPORTER_ASSERT(reporter, configs[41]->asConfigGpu()->getColorSpace());
    REPORTER_ASSERT(reporter, configs[41]->asConfigGpu()->getColorSpace()->gammaIsLinear());
    const SkMatrix44* config41XYZ =
            as_CSB(configs[41]->asConfigGpu()->getColorSpace())->toXYZD50();
    SkASSERT(config41XYZ);
    REPORTER_ASSERT(reporter, *config41XYZ != *srgbXYZ);
    REPORTER_ASSERT(reporter, configs[33]->asConfigGpu()->getContextType() ==
                              GrContextFactory::kGL_ContextType);
    REPORTER_ASSERT(reporter, configs[42]->asConfigGpu()->getColorType() == kRGBA_F16_SkColorType);
    REPORTER_ASSERT(reporter, configs[42]->asConfigGpu()->getColorSpace());
    REPORTER_ASSERT(reporter, configs[42]->asConfigGpu()->getColorSpace()->gammaIsLinear());
    REPORTER_ASSERT(reporter, *as_CSB(configs[42]->asConfigGpu()->getColorSpace())->toXYZD50() !=
                    *as_CSB(srgbColorSpace)->toXYZD50());
    REPORTER_ASSERT(reporter, configs[33]->asConfigGpu()->getUseInstanced());
    REPORTER_ASSERT(reporter, configs[34]->asConfigGpu()->getContextType() ==
                              GrContextFactory::kGL_ContextType);
    REPORTER_ASSERT(reporter, configs[34]->asConfigGpu()->getUseInstanced());
    REPORTER_ASSERT(reporter, configs[34]->asConfigGpu()->getSamples() == 4);
    REPORTER_ASSERT(reporter, configs[35]->asConfigGpu()->getContextType() ==
                              GrContextFactory::kGL_ContextType);
    REPORTER_ASSERT(reporter, configs[35]->asConfigGpu()->getUseInstanced());
    REPORTER_ASSERT(reporter, configs[35]->asConfigGpu()->getUseDIText());
    REPORTER_ASSERT(reporter, configs[35]->asConfigGpu()->getSamples() == 4);
    REPORTER_ASSERT(reporter, configs[36]->asConfigGpu()->getContextType() ==
                              GrContextFactory::kGL_ContextType);
    REPORTER_ASSERT(reporter, configs[36]->asConfigGpu()->getUseInstanced());
    REPORTER_ASSERT(reporter, configs[36]->asConfigGpu()->getSamples() == 16);
    REPORTER_ASSERT(reporter, configs[37]->asConfigGpu()->getContextType() ==
                              GrContextFactory::kGL_ContextType);
    REPORTER_ASSERT(reporter, configs[37]->asConfigGpu()->getUseInstanced());
    REPORTER_ASSERT(reporter, configs[37]->asConfigGpu()->getUseDIText());
    REPORTER_ASSERT(reporter, configs[37]->asConfigGpu()->getSamples() == 16);
    REPORTER_ASSERT(reporter, configs[38]->asConfigGpu()->getContextType() ==
                              GrContextFactory::kGLES_ContextType);
    REPORTER_ASSERT(reporter, configs[38]->asConfigGpu()->getUseInstanced());
    REPORTER_ASSERT(reporter, configs[39]->asConfigGpu()->getContextType() ==
                              GrContextFactory::kGLES_ContextType);
    REPORTER_ASSERT(reporter, configs[39]->asConfigGpu()->getUseInstanced());
    REPORTER_ASSERT(reporter, configs[39]->asConfigGpu()->getSamples() == 4);
    REPORTER_ASSERT(reporter, configs[40]->asConfigGpu()->getContextType() ==
                              GrContextFactory::kGLES_ContextType);
    REPORTER_ASSERT(reporter, configs[40]->asConfigGpu()->getUseInstanced());
    REPORTER_ASSERT(reporter, configs[40]->asConfigGpu()->getUseDIText());
    REPORTER_ASSERT(reporter, configs[40]->asConfigGpu()->getSamples() == 4);
    REPORTER_ASSERT(reporter, configs[20]->asConfigGpu());
    REPORTER_ASSERT(reporter, configs[21]->asConfigGpu());
    REPORTER_ASSERT(reporter, configs[22]->asConfigGpu());
#if SK_MESA
    REPORTER_ASSERT(reporter, configs[23]->asConfigGpu());
#else
    REPORTER_ASSERT(reporter, !configs[23]->asConfigGpu());
#endif
    REPORTER_ASSERT(reporter, configs[27]->asConfigGpu());
    REPORTER_ASSERT(reporter, configs[28]->asConfigGpu());
    REPORTER_ASSERT(reporter, configs[28]->asConfigGpu()->getSamples() == 4);
    REPORTER_ASSERT(reporter, configs[28]->asConfigGpu()->getUseNVPR());
    REPORTER_ASSERT(reporter, configs[29]->asConfigGpu());
    REPORTER_ASSERT(reporter, configs[29]->asConfigGpu()->getSamples() == 4);
    REPORTER_ASSERT(reporter, configs[29]->asConfigGpu()->getUseNVPR());
    REPORTER_ASSERT(reporter, configs[29]->asConfigGpu()->getUseDIText());
    REPORTER_ASSERT(reporter, configs[30]->asConfigGpu());
    REPORTER_ASSERT(reporter, configs[30]->asConfigGpu()->getColorType()  == kRGBA_8888_SkColorType);
    REPORTER_ASSERT(reporter, configs[30]->asConfigGpu()->getColorSpace() == srgbColorSpace.get());
    REPORTER_ASSERT(reporter, configs[31]->asConfigGpu());
    REPORTER_ASSERT(reporter, configs[31]->asConfigGpu()->getSamples() == 4);
#ifdef SK_VULKAN
    REPORTER_ASSERT(reporter, configs[32]->asConfigGpu());
#endif
#endif
}

DEF_TEST(ParseConfigs_ExtendedGpuConfigsCorrect, reporter) {
    SkCommandLineFlags::StringArray config1 = make_string_array({
        "gpu[nvpr=true,dit=false]",
        "gpu[api=angle_d3d9_es2]",
        "gpu[api=angle_gl_es3]",
        "gpu[api=mesa,samples=77]",
        "gpu[dit=true,api=commandbuffer]",
        "gpu[]",
        "gpu[api=gles]",
        "gpu[api=gl]",
        "gpu[api=vulkan]",
    });

    SkCommandLineConfigArray configs;
    ParseConfigs(config1, &configs);
    REPORTER_ASSERT(reporter, configs.count() == config1.count());
    for (int i = 0; i < config1.count(); ++i) {
        REPORTER_ASSERT(reporter, configs[i]->getTag().equals(config1[i]));
    }
#if SK_SUPPORT_GPU
    REPORTER_ASSERT(reporter, configs[0]->asConfigGpu()->getContextType() ==
                    GrContextFactory::kNativeGL_ContextType);
    REPORTER_ASSERT(reporter, configs[0]->asConfigGpu()->getUseNVPR());
    REPORTER_ASSERT(reporter, !configs[0]->asConfigGpu()->getUseDIText());
    REPORTER_ASSERT(reporter, configs[0]->asConfigGpu()->getSamples() == 0);
    REPORTER_ASSERT(reporter, configs[1]->asConfigGpu()->getContextType() ==
                    GrContextFactory::kANGLE_D3D9_ES2_ContextType);
    REPORTER_ASSERT(reporter, configs[1]->asConfigGpu());
    REPORTER_ASSERT(reporter, configs[2]->asConfigGpu()->getContextType() ==
                    GrContextFactory::kANGLE_GL_ES3_ContextType);
    REPORTER_ASSERT(reporter, configs[2]->asConfigGpu());
#if SK_MESA
    REPORTER_ASSERT(reporter, configs[3]->asConfigGpu()->getContextType() ==
                    GrContextFactory::kMESA_ContextType);
#else
    REPORTER_ASSERT(reporter, !configs[3]->asConfigGpu());
#endif
    REPORTER_ASSERT(reporter, configs[4]->asConfigGpu()->getContextType() ==
                    GrContextFactory::kCommandBuffer_ContextType);

    REPORTER_ASSERT(reporter, configs[5]->asConfigGpu()->getContextType() ==
                    GrContextFactory::kNativeGL_ContextType);
    REPORTER_ASSERT(reporter, !configs[5]->asConfigGpu()->getUseNVPR());
    REPORTER_ASSERT(reporter, !configs[5]->asConfigGpu()->getUseDIText());
    REPORTER_ASSERT(reporter, configs[5]->asConfigGpu()->getSamples() == 0);
    REPORTER_ASSERT(reporter, configs[6]->asConfigGpu()->getContextType() ==
                    GrContextFactory::kGLES_ContextType);
    REPORTER_ASSERT(reporter, !configs[6]->asConfigGpu()->getUseNVPR());
    REPORTER_ASSERT(reporter, !configs[6]->asConfigGpu()->getUseDIText());
    REPORTER_ASSERT(reporter, configs[6]->asConfigGpu()->getSamples() == 0);
    REPORTER_ASSERT(reporter, configs[7]->asConfigGpu()->getContextType() ==
                              GrContextFactory::kGL_ContextType);
    REPORTER_ASSERT(reporter, !configs[7]->asConfigGpu()->getUseNVPR());
    REPORTER_ASSERT(reporter, !configs[7]->asConfigGpu()->getUseDIText());
    REPORTER_ASSERT(reporter, configs[7]->asConfigGpu()->getSamples() == 0);
#ifdef SK_VULKAN
    REPORTER_ASSERT(reporter, configs[8]->asConfigGpu()->getContextType() ==
                              GrContextFactory::kVulkan_ContextType);
    REPORTER_ASSERT(reporter, !configs[7]->asConfigGpu()->getUseNVPR());
    REPORTER_ASSERT(reporter, !configs[7]->asConfigGpu()->getUseDIText());
    REPORTER_ASSERT(reporter, configs[7]->asConfigGpu()->getSamples() == 0);
#endif
#endif
}

DEF_TEST(ParseConfigs_ExtendedGpuConfigsIncorrect, reporter) {
    SkCommandLineFlags::StringArray config1 = make_string_array({
        "gpu[nvpr=1]", // Number as bool.
        "gpu[api=gl,]", // Trailing in comma.
        "gpu[api=angle_glu]", // Unknown api.
        "gpu[api=,samples=0]", // Empty api.
        "gpu[samples=true]", // Value true as a number.
        "gpu[samples=0,samples=0]", // Duplicate option key.
        "gpu[,samples=0]", // Leading comma.
        "gpu[samples=54", // Missing closing parenthesis.
        ",,",
        "gpu[", // Missing bracket.
        "samples=54" // No backend.
        "gpu[nvpr=true ]", // Space.
    });

    SkCommandLineConfigArray configs;
    ParseConfigs(config1, &configs);
    REPORTER_ASSERT(reporter, configs.count() == config1.count());
    for (int i = 0; i < config1.count(); ++i) {
        REPORTER_ASSERT(reporter, configs[i]->getTag().equals(config1[i]));
        REPORTER_ASSERT(reporter, configs[i]->getBackend().equals(config1[i]));
#if SK_SUPPORT_GPU
        REPORTER_ASSERT(reporter, !configs[i]->asConfigGpu());
#endif
    }
}

DEF_TEST(ParseConfigs_ExtendedGpuConfigsSurprises, reporter) {
    // These just list explicitly some properties of the system.
    SkCommandLineFlags::StringArray config1 = make_string_array({
        // Options are not canonized -> two same configs have a different tag.
        "gpu[nvpr=true,dit=true]", "gpu[dit=true,nvpr=true]",
        "gpu[api=debug]", "gpu[api=gl]", "gpu[api=gles]", ""
        "gpu", "gpu[]", "gpu[samples=0]", "gpu[api=gles,samples=0]"
    });
    SkCommandLineConfigArray configs;
    ParseConfigs(config1, &configs);
    REPORTER_ASSERT(reporter, configs.count() == config1.count());
    for (int i = 0; i < config1.count(); ++i) {
        REPORTER_ASSERT(reporter, configs[i]->getTag().equals(config1[i]));
#if SK_SUPPORT_GPU
        REPORTER_ASSERT(reporter, configs[i]->getBackend().equals("gpu"));
        REPORTER_ASSERT(reporter, configs[i]->asConfigGpu());
#else
        REPORTER_ASSERT(reporter, configs[i]->getBackend().equals(config1[i]));
#endif
    }
}
DEF_TEST(ParseConfigs_ViaParsing, reporter) {
    SkCommandLineFlags::StringArray config1 = make_string_array({
        "a-b-c-8888",
        "zz-qq-gpu",
        "a-angle_gl_es2"
    });

    SkCommandLineConfigArray configs;
    ParseConfigs(config1, &configs);
    const struct {
        const char* backend;
        const char* vias[3];
    } expectedConfigs[] = {
        {"8888", {"a", "b", "c"}},
        {"gpu", {"zz", "qq", nullptr}},
        {"gpu", { "a", nullptr, nullptr }}
    };
    for (int i = 0; i < config1.count(); ++i) {
        REPORTER_ASSERT(reporter, configs[i]->getTag().equals(config1[i]));
        REPORTER_ASSERT(reporter, configs[i]->getBackend().equals(expectedConfigs[i].backend));
        for (int j = 0; j < static_cast<int>(SK_ARRAY_COUNT(expectedConfigs[i].vias)); ++j) {
            if (!expectedConfigs[i].vias[j]) {
                REPORTER_ASSERT(reporter, configs[i]->getViaParts().count() == j);
                break;
            }
            REPORTER_ASSERT(reporter,
                            configs[i]->getViaParts()[j].equals(expectedConfigs[i].vias[j]));
        }
    }
}

DEF_TEST(ParseConfigs_ViaParsingExtendedForm, reporter) {
    SkCommandLineFlags::StringArray config1 = make_string_array({
        "zz-qq-gpu[api=gles]",
        "abc-nbc-cbs-gpu[api=angle_d3d9_es2,samples=1]",
        "a-gpu[samples=1",
        "abc-def-angle_gl_es2[samples=1]",
    });

    SkCommandLineConfigArray configs;
    ParseConfigs(config1, &configs);
    const struct {
        const char* backend;
        const char* vias[3];
    } expectedConfigs[] = {
#if SK_SUPPORT_GPU
        {"gpu", {"zz", "qq", nullptr}},
        {"gpu", {"abc", "nbc", "cbs"}},
#else
        {"gpu[api=gles]", {"zz", "qq", nullptr}},
        {"gpu[api=angle_d3d9_es2,samples=1]", {"abc", "nbc", "cbs"}},
#endif
        {"gpu[samples=1", {"a", nullptr, nullptr}}, // Missing bracket makes this is not extended
                                                    // form but via still works as expected.
        {"angle_gl_es2[samples=1]", {"abc", "def", nullptr}}  // This is not extended form.
                                                              // angle_gl_es2 is an api type not a
                                                              // backend.
    };
    for (int i = 0; i < config1.count(); ++i) {
        REPORTER_ASSERT(reporter, configs[i]->getTag().equals(config1[i]));
        REPORTER_ASSERT(reporter, configs[i]->getBackend().equals(expectedConfigs[i].backend));
        for (int j = 0; j < static_cast<int>(SK_ARRAY_COUNT(expectedConfigs[i].vias)); ++j) {
            if (!expectedConfigs[i].vias[j]) {
                REPORTER_ASSERT(reporter, configs[i]->getViaParts().count() ==
                                static_cast<int>(j));
                break;
            }
            REPORTER_ASSERT(reporter,
                            configs[i]->getViaParts()[j].equals(expectedConfigs[i].vias[j]));
        }
    }
#if SK_SUPPORT_GPU
    REPORTER_ASSERT(reporter, configs[0]->asConfigGpu());
    REPORTER_ASSERT(reporter, configs[1]->asConfigGpu());
    REPORTER_ASSERT(reporter, !configs[2]->asConfigGpu());
    REPORTER_ASSERT(reporter, !configs[3]->asConfigGpu());
#endif
}
