/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCommonFlagsConfig.h"
#include "Test.h"
#include <initializer_list>

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
                    == GrContextFactory::kNative_GLContextType);
    REPORTER_ASSERT(reporter, configs[0]->asConfigGpu()->getUseNVPR() == false);
    REPORTER_ASSERT(reporter, configs[0]->asConfigGpu()->getUseDIText() == false);
    REPORTER_ASSERT(reporter, configs[0]->asConfigGpu()->getSamples() == 0);
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
}

DEF_TEST(ParseConfigs_DefaultConfigs, reporter) {
    // Parses all default configs and returns correct "tag".

    SkCommandLineFlags::StringArray config1 = make_string_array({
        "565", "8888", "debug", "gpu", "gpudebug", "gpudft", "gpunull", "msaa16", "msaa4",
        "nonrendering", "null", "nullgpu", "nvprmsaa16", "nvprmsaa4", "pdf", "pdf_poppler",
        "skp", "svg", "xps", "angle", "angle-gl", "commandbuffer", "mesa", "hwui"
    });

    SkCommandLineConfigArray configs;
    ParseConfigs(config1, &configs);

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
    REPORTER_ASSERT(reporter, configs[12]->asConfigGpu()->getUseDIText());
    REPORTER_ASSERT(reporter, configs[13]->asConfigGpu()->getSamples() == 4);
    REPORTER_ASSERT(reporter, configs[13]->asConfigGpu()->getUseNVPR());
    REPORTER_ASSERT(reporter, configs[13]->asConfigGpu()->getUseDIText());
    REPORTER_ASSERT(reporter, !configs[14]->asConfigGpu());
    REPORTER_ASSERT(reporter, !configs[15]->asConfigGpu());
    REPORTER_ASSERT(reporter, !configs[16]->asConfigGpu());
    REPORTER_ASSERT(reporter, !configs[17]->asConfigGpu());
    REPORTER_ASSERT(reporter, !configs[18]->asConfigGpu());
    REPORTER_ASSERT(reporter, !configs[23]->asConfigGpu());
#if SK_ANGLE
#ifdef SK_BUILD_FOR_WIN
    REPORTER_ASSERT(reporter, configs[19]->asConfigGpu());
#else
    REPORTER_ASSERT(reporter, !configs[19]->asConfigGpu());
#endif
    REPORTER_ASSERT(reporter, configs[20]->asConfigGpu());
#else
    REPORTER_ASSERT(reporter, !configs[19]->asConfigGpu());
    REPORTER_ASSERT(reporter, !configs[20]->asConfigGpu());
#endif
#if SK_COMMAND_BUFFER
    REPORTER_ASSERT(reporter, configs[21]->asConfigGpu());
#else
    REPORTER_ASSERT(reporter, !configs[21]->asConfigGpu());
#endif
#if SK_MESA
    REPORTER_ASSERT(reporter, configs[22]->asConfigGpu());
#else
    REPORTER_ASSERT(reporter, !configs[22]->asConfigGpu());
#endif
#endif
}

DEF_TEST(ParseConfigs_ExtendedGpuConfigsCorrect, reporter) {
    SkCommandLineFlags::StringArray config1 = make_string_array({
        "gpu(nvpr=true,dit=false)",
        "gpu(api=angle)",
        "gpu(api=angle-gl)",
        "gpu(api=mesa,samples=77)",
        "gpu(dit=true,api=commandbuffer)",
        "gpu()",
        "gpu(api=gles)"
    });

    SkCommandLineConfigArray configs;
    ParseConfigs(config1, &configs);
    REPORTER_ASSERT(reporter, configs.count() == config1.count());
    for (int i = 0; i < config1.count(); ++i) {
        REPORTER_ASSERT(reporter, configs[i]->getTag().equals(config1[i]));
    }
#if SK_SUPPORT_GPU
    REPORTER_ASSERT(reporter, configs[0]->asConfigGpu()->getContextType() ==
                    GrContextFactory::kNative_GLContextType);
    REPORTER_ASSERT(reporter, configs[0]->asConfigGpu()->getUseNVPR());
    REPORTER_ASSERT(reporter, !configs[0]->asConfigGpu()->getUseDIText());
    REPORTER_ASSERT(reporter, configs[0]->asConfigGpu()->getSamples() == 0);
#if SK_ANGLE
#ifdef SK_BUILD_FOR_WIN
    REPORTER_ASSERT(reporter, configs[1]->asConfigGpu()->getContextType() ==
                    GrContextFactory::kANGLE_GLContextType);
#else
    REPORTER_ASSERT(reporter, !configs[1]->asConfigGpu());
#endif
    REPORTER_ASSERT(reporter, configs[2]->asConfigGpu()->getContextType() ==
                    GrContextFactory::kANGLE_GL_GLContextType);
#else
    REPORTER_ASSERT(reporter, !configs[1]->asConfigGpu());
    REPORTER_ASSERT(reporter, !configs[2]->asConfigGpu());
#endif
#if SK_MESA
    REPORTER_ASSERT(reporter, configs[3]->asConfigGpu()->getContextType() ==
                    GrContextFactory::kMESA_GLContextType);
#else
    REPORTER_ASSERT(reporter, !configs[3]->asConfigGpu());
#endif
#if SK_COMMAND_BUFFER
    REPORTER_ASSERT(reporter, configs[4]->asConfigGpu()->getContextType() ==
                    GrContextFactory::kCommandBufferES2_GLContextType);

#else
    REPORTER_ASSERT(reporter, !configs[4]->asConfigGpu());
#endif
    REPORTER_ASSERT(reporter, configs[5]->asConfigGpu()->getContextType() ==
                    GrContextFactory::kNative_GLContextType);
    REPORTER_ASSERT(reporter, !configs[5]->asConfigGpu()->getUseNVPR());
    REPORTER_ASSERT(reporter, !configs[5]->asConfigGpu()->getUseDIText());
    REPORTER_ASSERT(reporter, configs[5]->asConfigGpu()->getSamples() == 0);
    REPORTER_ASSERT(reporter, configs[6]->asConfigGpu()->getContextType() ==
                    GrContextFactory::kGLES_GLContextType);
    REPORTER_ASSERT(reporter, !configs[6]->asConfigGpu()->getUseNVPR());
    REPORTER_ASSERT(reporter, !configs[6]->asConfigGpu()->getUseDIText());
    REPORTER_ASSERT(reporter, configs[6]->asConfigGpu()->getSamples() == 0);

#endif
}

DEF_TEST(ParseConfigs_ExtendedGpuConfigsIncorrect, reporter) {
    SkCommandLineFlags::StringArray config1 = make_string_array({
        "gpu(nvpr=1)", // Number as bool.
        "gpu(api=gl,)", // Trailing in comma.
        "gpu(api=angle-glu)", // Unknown api.
        "gpu(api=,samples=0)", // Empty api.
        "gpu(samples=true)", // Value true as a number.
        "gpu(samples=0,samples=0)", // Duplicate option key.
        "gpu(,samples=0)", // Leading comma.
        "gpu(samples=54", // Missing closing parenthesis.
        ",,",
        "gpu(", // Missing parenthesis.
        "samples=54" // No backend.
        "gpu(nvpr=true )", // Space.
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
        "gpu(nvpr=true,dit=true)", "gpu(dit=true,nvpr=true)",
        // API native is alias for gl or gles, but it's not canonized -> different tag.
        "gpu(api=native)", "gpu(api=gl)", "gpu(api=gles)", ""
        // Default values are not canonized -> different tag.
        "gpu", "gpu()", "gpu(samples=0)", "gpu(api=native,samples=0)"
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
        "a-angle-gl"
    });

    SkCommandLineConfigArray configs;
    ParseConfigs(config1, &configs);
    const struct {
        const char* backend;
        const char* vias[3];
    } expectedConfigs[] = {
        {"8888", {"a", "b", "c"}},
        {"gpu", {"zz", "qq", nullptr}},
        {"angle-gl", {"a", nullptr, nullptr}}  // The angle-gl tag is only tag that contains
                                               // hyphen.
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
        "zz-qq-gpu(api=gles)",
        "a-gpu(samples=1",
        "abc-def-angle-gl(samples=1)",
    });

    SkCommandLineConfigArray configs;
    ParseConfigs(config1, &configs);
    const struct {
        const char* backend;
        const char* vias[3];
    } expectedConfigs[] = {
#if SK_SUPPORT_GPU
        {"gpu", {"zz", "qq", nullptr}},
#else
        {"gpu(api=gles)", {"zz", "qq", nullptr}},
#endif
        {"gpu(samples=1", {"a", nullptr, nullptr}}, // This is not extended form, but via still
                                                    // works as expected.
        {"gl(samples=1)", {"abc", "def", "angle"}}  // This is not extended form.  Also
                                                    // angle-gl is not a "backend" in this case.
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
    REPORTER_ASSERT(reporter, !configs[1]->asConfigGpu());
    REPORTER_ASSERT(reporter, !configs[2]->asConfigGpu());
#endif
}
