#include "SkGraphics.h"
#include "Test.h"

using namespace skiatest;

// need to explicitly declare this, or we get some weird infinite loop llist
template TestRegistry* TestRegistry::gHead;

class Iter {
public:
    Iter(Reporter* r) : fReporter(r) {
        r->ref();
        fReg = TestRegistry::Head();
    }
    
    ~Iter() {
        fReporter->unref();
    }
    
    Test* next() {
        if (fReg) {
            TestRegistry::Factory fact = fReg->factory();
            fReg = fReg->next();
            Test* test = fact(NULL);
            test->setReporter(fReporter);
            return test;
        }
        return NULL;
    }
    
private:
    Reporter* fReporter;
    const TestRegistry* fReg;
};

static const char* result2string(Reporter::Result result) {
    return result == Reporter::kPassed ? "passed" : "FAILED";
}

class DebugfReporter : public Reporter {
protected:
    virtual void onStart(Test* test) {
        SkDebugf("Running %s...\n", test->getName());
    }
    virtual void onReport(const char desc[], Reporter::Result result) {
        SkDebugf("\t%s: %s\n", result2string(result), desc);
    }
    virtual void onEnd(Test* test) {}
};

class SkAutoGraphics {
public:
    SkAutoGraphics() {
        SkGraphics::Init();
    }
    ~SkAutoGraphics() {
        SkGraphics::Term();
    }
};

int main (int argc, char * const argv[]) {
    SkAutoGraphics ag;

    DebugfReporter reporter;
    Iter iter(&reporter);
    Test* test;
    
    while ((test = iter.next()) != NULL) {
        test->run();
        SkDELETE(test);
    }
    
    int total = reporter.countTests();
    int passed = reporter.countResults(Reporter::kPassed);
    int failed = reporter.countResults(Reporter::kFailed);
    SkDebugf("Tests=%d Passed=%d (%g%%) Failed=%d (%g%%)\n", total,
           passed, passed * 100.f / total,
           failed, failed * 100.f / total);

    return 0;
}
