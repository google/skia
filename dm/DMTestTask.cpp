#include "DMTestTask.h"
#include "DMUtil.h"
#include "SkCommandLineFlags.h"

DEFINE_bool2(pathOpsExtended,     x, false, "Run extended pathOps tests.");
DEFINE_bool2(pathOpsSingleThread, z, false, "Disallow pathOps tests from using threads.");
DEFINE_bool2(pathOpsVerbose,      V, false, "Tell pathOps tests to be verbose.");

namespace DM {

static SkString test_name(const char* name) {
    SkString result("test ");
    result.append(name);
    return result;
}

TestTask::TestTask(Reporter* reporter,
                   TaskRunner* taskRunner,
                   skiatest::TestRegistry::Factory factory)
    : Task(reporter, taskRunner)
    , fTest(factory(NULL))
    , fName(test_name(fTest->getName())) {}

void TestTask::draw() {
    if (this->usesGpu()) {
        fTest->setGrContextFactory(this->getGrContextFactory());
    }
    fTest->setReporter(&fTestReporter);
    fTest->run();
    if (!fTest->passed()) {
        this->fail(fTestReporter.failure());
    }
}

bool TestTask::TestReporter::allowExtendedTest() const { return FLAGS_pathOpsExtended; }
bool TestTask::TestReporter::allowThreaded()     const { return !FLAGS_pathOpsSingleThread; }
bool TestTask::TestReporter::verbose()           const { return FLAGS_pathOpsVerbose; }

}  // namespace DM
