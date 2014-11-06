// Main binary for DM.
// For a high-level overview, please see dm/README.

#include "CrashHandler.h"
#include "LazyDecodeBitmap.h"
#include "SkCommonFlags.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkString.h"
#include "SkTaskGroup.h"
#include "Test.h"
#include "gm.h"
#include "sk_tool_utils.h"
#include "sk_tool_utils_flags.h"

#include "DMCpuGMTask.h"
#include "DMGpuGMTask.h"
#include "DMGpuSupport.h"
#include "DMJsonWriter.h"
#include "DMPDFTask.h"
#include "DMReporter.h"
#include "DMSKPTask.h"
#include "DMTask.h"
#include "DMTaskRunner.h"
#include "DMTestTask.h"

#ifdef SK_BUILD_POPPLER
#  include "SkPDFRasterizer.h"
#  define RASTERIZE_PDF_PROC SkPopplerRasterizePDF
#elif defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
#  include "SkCGUtils.h"
#  define RASTERIZE_PDF_PROC SkPDFDocumentToBitmap
#else
#  define RASTERIZE_PDF_PROC NULL
#endif

#include <ctype.h>

using skiagm::GM;
using skiagm::GMRegistry;
using skiatest::Test;
using skiatest::TestRegistry;

static const char kGpuAPINameGL[] = "gl";
static const char kGpuAPINameGLES[] = "gles";

DEFINE_bool(gms, true, "Run GMs?");
DEFINE_bool(tests, true, "Run tests?");
DEFINE_bool(reportUsedChars, false, "Output test font construction data to be pasted into"
                                    " create_test_font.cpp.");

__SK_FORCE_IMAGE_DECODER_LINKING;

// "FooBar" -> "foobar".  Obviously, ASCII only.
static SkString lowercase(SkString s) {
    for (size_t i = 0; i < s.size(); i++) {
        s[i] = tolower(s[i]);
    }
    return s;
}

static const GrContextFactory::GLContextType native = GrContextFactory::kNative_GLContextType;
static const GrContextFactory::GLContextType nvpr   = GrContextFactory::kNVPR_GLContextType;
static const GrContextFactory::GLContextType null   = GrContextFactory::kNull_GLContextType;
static const GrContextFactory::GLContextType debug  = GrContextFactory::kDebug_GLContextType;
#if SK_ANGLE
static const GrContextFactory::GLContextType angle  = GrContextFactory::kANGLE_GLContextType;
#endif
#if SK_MESA
static const GrContextFactory::GLContextType mesa   = GrContextFactory::kMESA_GLContextType;
#endif

static void kick_off_gms(const SkTDArray<GMRegistry::Factory>& gms,
                         const SkTArray<SkString>& configs,
                         GrGLStandard gpuAPI,
                         DM::Reporter* reporter,
                         DM::TaskRunner* tasks) {
#define START(name, type, ...)                                                              \
    if (lowercase(configs[j]).equals(name)) {                                               \
        tasks->add(SkNEW_ARGS(DM::type, (name, reporter, tasks, gms[i], ## __VA_ARGS__)));  \
    }
    for (int i = 0; i < gms.count(); i++) {
        for (int j = 0; j < configs.count(); j++) {

            START("565",        CpuGMTask, kRGB_565_SkColorType);
            START("8888",       CpuGMTask, kN32_SkColorType);
            START("gpu",        GpuGMTask, native, gpuAPI, 0);
            START("msaa4",      GpuGMTask, native, gpuAPI, 4);
            START("msaa16",     GpuGMTask, native, gpuAPI, 16);
            START("nvprmsaa4",  GpuGMTask, nvpr,   gpuAPI, 4);
            START("nvprmsaa16", GpuGMTask, nvpr,   gpuAPI, 16);
            START("gpunull",    GpuGMTask, null,   gpuAPI, 0);
            START("gpudebug",   GpuGMTask, debug,  gpuAPI, 0);
#if SK_ANGLE
            START("angle",      GpuGMTask, angle,  gpuAPI, 0);
#endif
#if SK_MESA
            START("mesa",       GpuGMTask, mesa,   gpuAPI, 0);
#endif
            START("pdf",        PDFTask,   RASTERIZE_PDF_PROC);
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

static void find_skps(SkTArray<SkString>* skps) {
    if (FLAGS_skps.isEmpty()) {
        return;
    }

    SkOSFile::Iter it(FLAGS_skps[0], ".skp");
    SkString filename;
    while (it.next(&filename)) {
        if (!SkCommandLineFlags::ShouldSkip(FLAGS_match, filename.c_str())) {
            skps->push_back(SkOSPath::Join(FLAGS_skps[0], filename.c_str()));
        }
    }
}

static void kick_off_skps(const SkTArray<SkString>& skps,
                          DM::Reporter* reporter,
                          DM::TaskRunner* tasks) {
    for (int i = 0; i < skps.count(); ++i) {
        SkAutoTUnref<SkStream> stream(SkStream::NewFromFile(skps[i].c_str()));
        if (stream.get() == NULL) {
            SkDebugf("Could not read %s.\n", skps[i].c_str());
            exit(1);
        }
        SkAutoTUnref<SkPicture> pic(
                SkPicture::CreateFromStream(stream.get(), &sk_tools::LazyDecodeBitmap));
        if (pic.get() == NULL) {
            SkDebugf("Could not read %s as an SkPicture.\n", skps[i].c_str());
            exit(1);
        }

        SkString filename = SkOSPath::Basename(skps[i].c_str());
        tasks->add(SkNEW_ARGS(DM::SKPTask, (reporter, tasks, pic, filename)));
        tasks->add(SkNEW_ARGS(DM::PDFTask, (reporter, tasks, pic, filename, RASTERIZE_PDF_PROC)));
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

static GrGLStandard get_gl_standard() {
  if (FLAGS_gpuAPI.contains(kGpuAPINameGL)) {
      return kGL_GrGLStandard;
  }
  if (FLAGS_gpuAPI.contains(kGpuAPINameGLES)) {
      return kGLES_GrGLStandard;
  }
  return kNone_GrGLStandard;
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

int dm_main();
int dm_main() {
    SetupCrashHandler();
    SkAutoGraphics ag;
    SkTaskGroup::Enabler enabled(FLAGS_threads);

    if (FLAGS_dryRun || FLAGS_veryVerbose) {
        FLAGS_verbose = true;
    }
#if SK_ENABLE_INST_COUNT
    gPrintInstCount = FLAGS_leaks;
#endif

    SkTArray<SkString> configs;
    for (int i = 0; i < FLAGS_config.count(); i++) {
        SkStrSplit(FLAGS_config[i], ", ", &configs);
    }

    GrGLStandard gpuAPI = get_gl_standard();

    SkTDArray<GMRegistry::Factory> gms;
    if (FLAGS_gms) {
        append_matching_factories<GM>(GMRegistry::Head(), &gms);
    }

    SkTDArray<TestRegistry::Factory> tests;
    if (FLAGS_tests) {
        append_matching_factories<Test>(TestRegistry::Head(), &tests);
    }

    SkTArray<SkString> skps;
    find_skps(&skps);

    SkDebugf("%d GMs x %d configs, %d tests, %d pictures\n",
             gms.count(), configs.count(), tests.count(), skps.count());
    DM::Reporter reporter;

    DM::TaskRunner tasks;
    kick_off_tests(tests, &reporter, &tasks);
    kick_off_gms(gms, configs, gpuAPI, &reporter, &tasks);
    kick_off_skps(skps, &reporter, &tasks);
    tasks.wait();

    DM::JsonWriter::DumpJson();

    SkDebugf("\n");
#ifdef SK_DEBUG
    if (FLAGS_portableFonts && FLAGS_reportUsedChars) {
        sk_tool_utils::report_used_chars();
    }
#endif

    SkTArray<SkString> failures;
    reporter.getFailures(&failures);
    report_failures(failures);
    return failures.count() > 0;
}

#if !defined(SK_BUILD_FOR_IOS) && !defined(SK_BUILD_FOR_NACL)
int main(int argc, char** argv) {
    SkCommandLineFlags::Parse(argc, argv);
    return dm_main();
}
#endif
