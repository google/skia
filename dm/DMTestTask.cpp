#include "DMTestTask.h"
#include "DMUtil.h"
#include "SkCommandLineFlags.h"
#include "SkCommonFlags.h"

DEFINE_bool2(pathOpsExtended, x, false, "Run extended pathOps tests.");

namespace DM {

bool TestReporter::allowExtendedTest() const { return FLAGS_pathOpsExtended; }
bool TestReporter::verbose()           const { return FLAGS_veryVerbose; }

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
        const SkTArray<SkString>& failures = fTestReporter.failures();
        for (int i = 0; i < failures.count(); i++) {
            this->fail(failures[i].c_str());
        }
    }
}

void GpuTestTask::draw(GrContextFactory* grFactory) {
    fTest->setGrContextFactory(grFactory);
    fTest->setReporter(&fTestReporter);
    fTest->run();
    if (!fTest->passed()) {
        const SkTArray<SkString>& failures = fTestReporter.failures();
        for (int i = 0; i < failures.count(); i++) {
            this->fail(failures[i].c_str());
        }
    }
}

bool GpuTestTask::shouldSkip() const {
    return kGPUDisabled;
}

}  // namespace DM
