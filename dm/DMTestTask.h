#ifndef DMTestTask_DEFINED
#define DMTestTask_DEFINED

#include "DMReporter.h"
#include "DMJsonWriter.h"
#include "DMTask.h"
#include "DMTaskRunner.h"
#include "SkString.h"
#include "SkTemplates.h"
#include "Test.h"

// Runs a unit test.
namespace DM {

class TestReporter : public skiatest::Reporter {
public:
  TestReporter() {}

  const SkTArray<SkString>& failures() const { return fFailures; }

private:
  virtual bool allowExtendedTest() const SK_OVERRIDE;
  virtual bool verbose()           const SK_OVERRIDE;

  virtual void onReportFailed(const skiatest::Failure& failure) SK_OVERRIDE {
      JsonWriter::AddTestFailure(failure);

      SkString newFailure;
      failure.getFailureString(&newFailure);
      fFailures.push_back(newFailure);
  }

  SkTArray<SkString> fFailures;
};

class CpuTestTask : public CpuTask {
public:
    CpuTestTask(Reporter*, TaskRunner*, skiatest::TestRegistry::Factory);

    virtual void draw() SK_OVERRIDE;
    virtual bool shouldSkip() const SK_OVERRIDE { return false; }
    virtual SkString name() const SK_OVERRIDE { return fName; }

private:
    TestReporter fTestReporter;
    SkAutoTDelete<skiatest::Test> fTest;
    const SkString fName;
};

class GpuTestTask : public GpuTask {
public:
    GpuTestTask(Reporter*, TaskRunner*, skiatest::TestRegistry::Factory);

    virtual void draw(GrContextFactory*) SK_OVERRIDE;
    virtual bool shouldSkip() const SK_OVERRIDE;
    virtual SkString name() const SK_OVERRIDE { return fName; }

private:
    TestReporter fTestReporter;
    SkAutoTDelete<skiatest::Test> fTest;
    const SkString fName;
};

}  // namespace DM

#endif // DMTestTask_DEFINED
