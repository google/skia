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

    static int Count() {
        const TestRegistry* reg = TestRegistry::Head();
        int count = 0;
        while (reg) {
            count += 1;
            reg = reg->next();
        }
        return count;
    }

private:
    Reporter* fReporter;
    const TestRegistry* fReg;
};

static const char* result2string(Reporter::Result result) {
    return result == Reporter::kPassed ? "passed" : "FAILED";
}

class DebugfReporter : public Reporter {
public:
    void setIndexOfTotal(int index, int total) {
        fIndex = index;
        fTotal = total;
    }
protected:
    virtual void onStart(Test* test) {
        SkDebugf("Running [%d/%d] %s...\n", fIndex+1, fTotal, test->getName());
    }
    virtual void onReport(const char desc[], Reporter::Result result) {
        SkDebugf("\t%s: %s\n", result2string(result), desc);
    }
    virtual void onEnd(Test* test) {}
private:
    int fIndex, fTotal;
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

    const int count = Iter::Count();
    int index = 0;
    while ((test = iter.next()) != NULL) {
        reporter.setIndexOfTotal(index, count);
        test->run();
        SkDELETE(test);
        index += 1;
    }
    SkDebugf("Finished %d tests.\n", count);

#if 0
    int total = reporter.countTests();
    int passed = reporter.countResults(Reporter::kPassed);
    int failed = reporter.countResults(Reporter::kFailed);
    SkDebugf("Tests=%d Passed=%d (%g%%) Failed=%d (%g%%)\n", total,
           passed, passed * 100.f / total,
           failed, failed * 100.f / total);
#endif
    return 0;
}
