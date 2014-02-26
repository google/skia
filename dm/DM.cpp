// Main binary for DM.
// For a high-level overview, please see dm/README.

#include "GrContext.h"
#include "GrContextFactory.h"
#include "SkCommandLineFlags.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkString.h"
#include "Test.h"
#include "gm.h"

#include "DMCpuTask.h"
#include "DMGpuTask.h"
#include "DMReporter.h"
#include "DMTask.h"
#include "DMTaskRunner.h"
#include "DMTestTask.h"
#include "DMWriteTask.h"

#include <string.h>

using skiagm::GM;
using skiagm::GMRegistry;
using skiatest::Test;
using skiatest::TestRegistry;

DEFINE_int32(threads, -1, "Threads for CPU work. Default NUM_CPUS.");
DEFINE_string2(expectations, r, "",
               "If a directory, compare generated images against images under this path. "
               "If a file, compare generated images against JSON expectations at this path.");
DEFINE_string2(resources, i, "resources", "Path to resources directory.");
DEFINE_string(match, "",  "[~][^]substring[$] [...] of GM name to run.\n"
                          "Multiple matches may be separated by spaces.\n"
                          "~ causes a matching GM to always be skipped\n"
                          "^ requires the start of the GM to match\n"
                          "$ requires the end of the GM to match\n"
                          "^ and $ requires an exact match\n"
                          "If a GM does not match any list entry,\n"
                          "it is skipped unless some list entry starts with ~");
DEFINE_string(config, "565 8888 gpu",
        "Options: 565 8888 gpu msaa4 msaa16 gpunull gpudebug angle mesa"); // TODO(mtklein): pdf
DEFINE_bool(leaks, false, "Print leaked instance-counted objects at exit?");

DEFINE_bool(gms, true, "Run GMs?");
DEFINE_bool(tests, true, "Run tests?");

__SK_FORCE_IMAGE_DECODER_LINKING;

// "FooBar" -> "foobar".  Obviously, ASCII only.
static SkString lowercase(SkString s) {
    for (size_t i = 0; i < s.size(); i++) {
        s[i] = tolower(s[i]);
    }
    return s;
}

static void kick_off_gms(const SkTDArray<GMRegistry::Factory>& gms,
                         const SkTArray<SkString>& configs,
                         const DM::Expectations& expectations,
                         DM::Reporter* reporter,
                         DM::TaskRunner* tasks) {
    const SkColorType _565 = kRGB_565_SkColorType;
    const SkColorType _8888 = kPMColor_SkColorType;
    const GrContextFactory::GLContextType native = GrContextFactory::kNative_GLContextType;
    const GrContextFactory::GLContextType null   = GrContextFactory::kNull_GLContextType;
    const GrContextFactory::GLContextType debug  = GrContextFactory::kDebug_GLContextType;
    const GrContextFactory::GLContextType angle  =
    #if SK_ANGLE
        GrContextFactory::kANGLE_GLContextType;
    #else
        native;
    #endif
    const GrContextFactory::GLContextType mesa   =
    #if SK_MESA
        GLContextFactory::kMESA_GLContextType;
    #else
        native;
    #endif

    for (int i = 0; i < gms.count(); i++) {
#define START(name, type, ...)                                                     \
    if (lowercase(configs[j]).equals(name)) {                                      \
        tasks->add(SkNEW_ARGS(DM::type,                                            \
                    (name, reporter, tasks, expectations, gms[i], __VA_ARGS__)));  \
    }
        for (int j = 0; j < configs.count(); j++) {
            START("565",      CpuTask, _565);
            START("8888",     CpuTask, _8888);
            START("gpu",      GpuTask, _8888, native, 0);
            START("msaa4",    GpuTask, _8888, native, 4);
            START("msaa16",   GpuTask, _8888, native, 16);
            START("gpunull",  GpuTask, _8888, null,   0);
            START("gpudebug", GpuTask, _8888, debug,  0);
            START("angle",    GpuTask, _8888, angle,  0);
            START("mesa",     GpuTask, _8888, mesa,   0);
            //START("pdf",      PdfTask, _8888);
        }
    }
#undef START
}

static void kick_off_tests(const SkTDArray<TestRegistry::Factory>& tests,
                           DM::Reporter* reporter,
                           DM::TaskRunner* tasks) {
    for (int i = 0; i < tests.count(); i++) {
        tasks->add(SkNEW_ARGS(DM::TestTask, (reporter, tasks, tests[i])));
    }
}

static void report_failures(const DM::Reporter& reporter) {
    SkTArray<SkString> failures;
    reporter.getFailures(&failures);

    if (failures.count() == 0) {
        return;
    }

    SkDebugf("Failures:\n");
    for (int i = 0; i < failures.count(); i++) {
        SkDebugf("  %s\n", failures[i].c_str());
    }
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
#if SK_ENABLE_INST_COUNT
    gPrintInstCount = FLAGS_leaks;
#endif
    SkGraphics::Init();
    SkCommandLineFlags::Parse(argc, argv);
    GM::SetResourcePath(FLAGS_resources[0]);
    Test::SetResourcePath(FLAGS_resources[0]);

    SkTArray<SkString> configs;
    SkTDArray<GMRegistry::Factory> gms;
    SkAutoTDelete<DM::Expectations> expectations(SkNEW(DM::NoExpectations));

    if (FLAGS_gms) {
        for (int i = 0; i < FLAGS_config.count(); i++) {
            SkStrSplit(FLAGS_config[i], ", ", &configs);
        }

        for (const GMRegistry* reg = GMRegistry::Head(); reg != NULL; reg = reg->next()) {
            SkAutoTDelete<GM> gmForName(reg->factory()(NULL));
            if (!SkCommandLineFlags::ShouldSkip(FLAGS_match, gmForName->shortName())) {
                *gms.append() = reg->factory();
            }
        }

        if (FLAGS_expectations.count() > 0) {
            const char* path = FLAGS_expectations[0];
            if (sk_isdir(path)) {
                expectations.reset(SkNEW_ARGS(DM::WriteTask::Expectations, (path)));
            } else {
                expectations.reset(SkNEW_ARGS(DM::JsonExpectations, (path)));
            }
        }
    }

    SkTDArray<TestRegistry::Factory> tests;
    if (FLAGS_tests) {
        for (const TestRegistry* reg = TestRegistry::Head(); reg != NULL; reg = reg->next()) {
            SkAutoTDelete<Test> testForName(reg->factory()(NULL));
            if (!SkCommandLineFlags::ShouldSkip(FLAGS_match, testForName->getName())) {
                *tests.append() = reg->factory();
            }
        }
    }

    SkDebugf("%d GMs x %d configs, %d tests\n", gms.count(), configs.count(), tests.count());
    DM::Reporter reporter;
    DM::TaskRunner tasks(FLAGS_threads);
    kick_off_gms(gms, configs, *expectations, &reporter, &tasks);
    kick_off_tests(tests, &reporter, &tasks);
    tasks.wait();

    SkDebugf("\n");
    report_failures(reporter);

    SkGraphics::Term();

    return reporter.failed() > 0;
}

#if !defined(SK_BUILD_FOR_IOS) && !defined(SK_BUILD_FOR_NACL)
int main(int argc, char** argv) {
    return tool_main(argc, argv);
}
#endif
