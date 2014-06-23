// Main binary for DM.
// For a high-level overview, please see dm/README.

#include "Benchmark.h"
#include "CrashHandler.h"
#include "SkCommandLineFlags.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkPicture.h"
#include "SkString.h"
#include "Test.h"
#include "gm.h"

#include "DMBenchTask.h"
#include "DMCpuGMTask.h"
#include "DMGpuGMTask.h"
#include "DMGpuSupport.h"
#include "DMPDFTask.h"
#include "DMReporter.h"
#include "DMSKPTask.h"
#include "DMTask.h"
#include "DMTaskRunner.h"
#include "DMTestTask.h"
#include "DMWriteTask.h"

#ifdef SK_BUILD_POPPLER
#  include "SkPDFRasterizer.h"
#  define RASTERIZE_PDF_PROC SkPopplerRasterizePDF
#else
#  define RASTERIZE_PDF_PROC NULL
#endif

#include <ctype.h>

using skiagm::GM;
using skiagm::GMRegistry;
using skiatest::Test;
using skiatest::TestRegistry;

DEFINE_int32(threads, -1, "Threads for CPU work. Default NUM_CPUS.");
DEFINE_int32(gpuThreads, 1, "Threads for GPU work.");
DEFINE_string2(expectations, r, "",
               "If a directory, compare generated images against images under this path. "
               "If a file, compare generated images against JSON expectations at this path."
);
DEFINE_string2(resources, i, "resources", "Path to resources directory.");
DEFINE_string(match, "",  "[~][^]substring[$] [...] of GM name to run.\n"
                          "Multiple matches may be separated by spaces.\n"
                          "~ causes a matching GM to always be skipped\n"
                          "^ requires the start of the GM to match\n"
                          "$ requires the end of the GM to match\n"
                          "^ and $ requires an exact match\n"
                          "If a GM does not match any list entry,\n"
                          "it is skipped unless some list entry starts with ~");
DEFINE_string(config, "565 8888 pdf gpu nonrendering",
              "Options: 565 8888 pdf gpu nonrendering msaa4 msaa16 nvprmsaa4 nvprmsaa16 "
              "gpunull gpudebug angle mesa");
DEFINE_bool(dryRun, false,
            "Just print the tests that would be run, without actually running them.");
DEFINE_bool(leaks, false, "Print leaked instance-counted objects at exit?");
DEFINE_string(skps, "", "Directory to read skps from.");

DEFINE_bool(gms, true, "Run GMs?");
DEFINE_bool(benches, true, "Run benches?  Does not run GMs-as-benches.");
DEFINE_bool(tests, true, "Run tests?");

DECLARE_bool(verbose);

__SK_FORCE_IMAGE_DECODER_LINKING;

// "FooBar" -> "foobar".  Obviously, ASCII only.
static SkString lowercase(SkString s) {
    for (size_t i = 0; i < s.size(); i++) {
        s[i] = tolower(s[i]);
    }
    return s;
}

static const GrContextFactory::GLContextType native = GrContextFactory::kNative_GLContextType;
static const GrContextFactory::GLContextType nvpr = GrContextFactory::kNVPR_GLContextType;
static const GrContextFactory::GLContextType null   = GrContextFactory::kNull_GLContextType;
static const GrContextFactory::GLContextType debug  = GrContextFactory::kDebug_GLContextType;
static const GrContextFactory::GLContextType angle  =
#if SK_ANGLE
GrContextFactory::kANGLE_GLContextType;
#else
native;
#endif
static const GrContextFactory::GLContextType mesa   =
#if SK_MESA
GrContextFactory::kMESA_GLContextType;
#else
native;
#endif

static void kick_off_gms(const SkTDArray<GMRegistry::Factory>& gms,
                         const SkTArray<SkString>& configs,
                         const DM::Expectations& expectations,
                         DM::Reporter* reporter,
                         DM::TaskRunner* tasks) {
#define START(name, type, ...)                                                              \
    if (lowercase(configs[j]).equals(name)) {                                               \
        tasks->add(SkNEW_ARGS(DM::type, (name, reporter, tasks, gms[i], ## __VA_ARGS__)));  \
    }
    for (int i = 0; i < gms.count(); i++) {
        for (int j = 0; j < configs.count(); j++) {
            START("565",        CpuGMTask, expectations, kRGB_565_SkColorType);
            START("8888",       CpuGMTask, expectations, kN32_SkColorType);
            START("gpu",        GpuGMTask, expectations, native, 0);
            START("msaa4",      GpuGMTask, expectations, native, 4);
            START("msaa16",     GpuGMTask, expectations, native, 16);
            START("nvprmsaa4",  GpuGMTask, expectations, nvpr,   4);
            START("nvprmsaa16", GpuGMTask, expectations, nvpr,   16);
            START("gpunull",    GpuGMTask, expectations, null,   0);
            START("gpudebug",   GpuGMTask, expectations, debug,  0);
            START("angle",      GpuGMTask, expectations, angle,  0);
            START("mesa",       GpuGMTask, expectations, mesa,   0);
            START("pdf",        PDFTask,   RASTERIZE_PDF_PROC);
        }
    }
#undef START
}

static void kick_off_benches(const SkTDArray<BenchRegistry::Factory>& benches,
                             const SkTArray<SkString>& configs,
                             DM::Reporter* reporter,
                             DM::TaskRunner* tasks) {
#define START(name, type, ...)                                                                 \
    if (lowercase(configs[j]).equals(name)) {                                                  \
        tasks->add(SkNEW_ARGS(DM::type, (name, reporter, tasks, benches[i], ## __VA_ARGS__))); \
    }
    for (int i = 0; i < benches.count(); i++) {
        for (int j = 0; j < configs.count(); j++) {
            START("nonrendering", NonRenderingBenchTask);
            START("565",          CpuBenchTask, kRGB_565_SkColorType);
            START("8888",         CpuBenchTask, kN32_SkColorType);
            START("gpu",          GpuBenchTask, native, 0);
            START("msaa4",        GpuBenchTask, native, 4);
            START("msaa16",       GpuBenchTask, native, 16);
            START("nvprmsaa4",    GpuBenchTask, nvpr,   4);
            START("nvprmsaa16",   GpuBenchTask, nvpr,   16);
            START("gpunull",      GpuBenchTask, null,   0);
            START("gpudebug",     GpuBenchTask, debug,  0);
            START("angle",        GpuBenchTask, angle,  0);
            START("mesa",         GpuBenchTask, mesa,   0);
        }
    }
#undef START
}

static void kick_off_tests(const SkTDArray<TestRegistry::Factory>& tests,
                           DM::Reporter* reporter,
                           DM::TaskRunner* tasks) {
    for (int i = 0; i < tests.count(); i++) {
        SkAutoTDelete<Test> test(tests[i](NULL));
        if (test->isGPUTest()) {
            tasks->add(SkNEW_ARGS(DM::GpuTestTask, (reporter, tasks, tests[i])));
        } else {
            tasks->add(SkNEW_ARGS(DM::CpuTestTask, (reporter, tasks, tests[i])));
        }
    }
}

static void kick_off_skps(DM::Reporter* reporter, DM::TaskRunner* tasks) {
    if (FLAGS_skps.isEmpty()) {
        return;
    }

    SkOSFile::Iter it(FLAGS_skps[0], ".skp");
    SkString filename;
    while (it.next(&filename)) {
        if (SkCommandLineFlags::ShouldSkip(FLAGS_match, filename.c_str())) {
            continue;
        }

        const SkString path = SkOSPath::SkPathJoin(FLAGS_skps[0], filename.c_str());

        SkAutoTDelete<SkStream> stream(SkStream::NewFromFile(path.c_str()));
        if (stream.get() == NULL) {
            SkDebugf("Could not read %s.\n", path.c_str());
            exit(1);
        }
        SkAutoTUnref<SkPicture> pic(SkPicture::CreateFromStream(stream.get()));
        if (pic.get() == NULL) {
            SkDebugf("Could not read %s as an SkPicture.\n", path.c_str());
            exit(1);
        }

        tasks->add(SkNEW_ARGS(DM::SKPTask, (reporter, tasks, pic->clone(), filename)));
        tasks->add(SkNEW_ARGS(DM::PDFTask, (reporter, tasks, pic->clone(), filename,
                                            RASTERIZE_PDF_PROC)));
    }
}

static void report_failures(const SkTArray<SkString>& failures) {
    if (failures.count() == 0) {
        return;
    }

    SkDebugf("Failures:\n");
    for (int i = 0; i < failures.count(); i++) {
        SkDebugf("  %s\n", failures[i].c_str());
    }
    SkDebugf("%d failures.\n", failures.count());
}

template <typename T, typename Registry>
static void append_matching_factories(Registry* head, SkTDArray<typename Registry::Factory>* out) {
    for (const Registry* reg = head; reg != NULL; reg = reg->next()) {
        SkAutoTDelete<T> forName(reg->factory()(NULL));
        if (!SkCommandLineFlags::ShouldSkip(FLAGS_match, forName->getName())) {
            *out->append() = reg->factory();
        }
    }
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SetupCrashHandler();
    SkAutoGraphics ag;
    SkCommandLineFlags::Parse(argc, argv);

    if (FLAGS_dryRun) {
        FLAGS_verbose = true;
    }
#if SK_ENABLE_INST_COUNT
    gPrintInstCount = FLAGS_leaks;
#endif

    SkTArray<SkString> configs;
    for (int i = 0; i < FLAGS_config.count(); i++) {
        SkStrSplit(FLAGS_config[i], ", ", &configs);
    }

    SkTDArray<GMRegistry::Factory> gms;
    SkAutoTDelete<DM::Expectations> expectations(SkNEW(DM::NoExpectations));
    if (FLAGS_gms) {
        append_matching_factories<GM>(GMRegistry::Head(), &gms);

        if (FLAGS_expectations.count() > 0) {
            const char* path = FLAGS_expectations[0];
            if (sk_isdir(path)) {
                expectations.reset(SkNEW_ARGS(DM::WriteTask::Expectations, (path)));
            } else {
                expectations.reset(SkNEW_ARGS(DM::JsonExpectations, (path)));
            }
        }
    }

    SkTDArray<BenchRegistry::Factory> benches;
    if (FLAGS_benches) {
        append_matching_factories<Benchmark>(BenchRegistry::Head(), &benches);
    }

    SkTDArray<TestRegistry::Factory> tests;
    if (FLAGS_tests) {
        append_matching_factories<Test>(TestRegistry::Head(), &tests);
    }

    SkDebugf("(%d GMs, %d benches) x %d configs, %d tests\n",
             gms.count(), benches.count(), configs.count(), tests.count());
    DM::Reporter reporter;
    DM::TaskRunner tasks(FLAGS_threads, FLAGS_gpuThreads);
    kick_off_gms(gms, configs, *expectations, &reporter, &tasks);
    kick_off_benches(benches, configs, &reporter, &tasks);
    kick_off_tests(tests, &reporter, &tasks);
    kick_off_skps(&reporter, &tasks);
    tasks.wait();

    SkDebugf("\n");

    SkTArray<SkString> failures;
    reporter.getFailures(&failures);
    report_failures(failures);
    return failures.count() > 0;
}

#if !defined(SK_BUILD_FOR_IOS) && !defined(SK_BUILD_FOR_NACL)
int main(int argc, char** argv) {
    return tool_main(argc, argv);
}
#endif
