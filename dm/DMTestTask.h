#ifndef DMTestTask_DEFINED
#define DMTestTask_DEFINED

#include "DMReporter.h"
#include "DMTask.h"
#include "DMTaskRunner.h"
#include "SkString.h"
#include "SkTemplates.h"
#include "Test.h"

// Runs a unit test.
namespace DM {

class TestTask : public Task {
public:
    TestTask(Reporter*, TaskRunner*, skiatest::TestRegistry::Factory);

    virtual void draw() SK_OVERRIDE;
    virtual bool usesGpu() const SK_OVERRIDE { return fTest->isGPUTest(); }
    virtual bool shouldSkip() const SK_OVERRIDE { return false; }
    virtual SkString name() const SK_OVERRIDE { return fName; }

private:
    class TestReporter : public skiatest::Reporter {
    public:
      TestReporter() {}

      const char* failure() const { return fFailure.c_str(); }

    private:
      virtual bool allowExtendedTest() const SK_OVERRIDE;
      virtual bool allowThreaded()     const SK_OVERRIDE;
      virtual bool verbose()           const SK_OVERRIDE;

      virtual void onReportFailed(const SkString& desc) SK_OVERRIDE {
          fFailure = desc;
      }

      SkString fFailure;
    };

    TestReporter fTestReporter;
    SkAutoTDelete<skiatest::Test> fTest;
    const SkString fName;
};

}  // namespace DM

#endif // DMTestTask_DEFINED
