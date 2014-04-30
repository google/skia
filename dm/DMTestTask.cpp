#include "DMTestTask.h"
#include "DMUtil.h"
#include "SkCommandLineFlags.h"

// When PathOps threaded tests get going, they're briefly a big consumer of lots of RAM.
// We disable the internal threading there by default on 32-bit builds.
static const bool is32Bit = sizeof(void*) == 4;

DEFINE_bool2(pathOpsExtended,     x, false, "Run extended pathOps tests.");
DEFINE_bool2(pathOpsSingleThread, z, is32Bit, "Disallow pathOps tests from using threads.");
DEFINE_bool2(pathOpsVerbose,      V, false, "Tell pathOps tests to be verbose.");

namespace DM {

bool TestReporter::allowExtendedTest() const { return FLAGS_pathOpsExtended; }
bool TestReporter::allowThreaded()     const { return !FLAGS_pathOpsSingleThread; }
bool TestReporter::verbose()           const { return FLAGS_pathOpsVerbose; }

static SkString test_name(const char* name) {
    SkString result("test ");
    result.append(name);
    return result;
}

CpuTestTask::CpuTestTask(Reporter* reporter,
                         TaskRunner* taskRunner,
                         skiatest::TestRegistry::Factory factory)
    : CpuTask(reporter, taskRunner)
    , fTest(factory(NULL))
    , fName(test_name(fTest->getName())) {}

GpuTestTask::GpuTestTask(Reporter* reporter,
                         TaskRunner* taskRunner,
                         skiatest::TestRegistry::Factory factory)
    : GpuTask(reporter, taskRunner)
    , fTest(factory(NULL))
    , fName(test_name(fTest->getName())) {}


void CpuTestTask::draw() {
    fTest->setReporter(&fTestReporter);
    fTest->run();
    if (!fTest->passed()) {
        this->fail(fTestReporter.failure());
    }
}

void GpuTestTask::draw(GrContextFactory* grFactory) {
    fTest->setGrContextFactory(grFactory);
    fTest->setReporter(&fTestReporter);
    fTest->run();
    if (!fTest->passed()) {
        this->fail(fTestReporter.failure());
    }
}

bool GpuTestTask::shouldSkip() const {
    return kGPUDisabled;
}

}  // namespace DM
