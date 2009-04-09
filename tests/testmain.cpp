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
    DebugfReporter(bool androidMode) : fAndroidMode(androidMode) {}

    void setIndexOfTotal(int index, int total) {
        fIndex = index;
        fTotal = total;
    }
protected:
    virtual void onStart(Test* test) {
        this->dumpState(test, kStarting_State);
    }
    virtual void onReport(const char desc[], Reporter::Result result) {
        if (!fAndroidMode) {
            SkDebugf("\t%s: %s\n", result2string(result), desc);
        }
    }
    virtual void onEnd(Test* test) {
        this->dumpState(test, this->getCurrSuccess() ?
                        kSucceeded_State : kFailed_State);
    }
private:
    enum State {
        kStarting_State = 1,
        kSucceeded_State = 0,
        kFailed_State = -2
    };

    void dumpState(Test* test, State state) {
        if (fAndroidMode) {
            SkDebugf("INSTRUMENTATION_STATUS: test=%s\n", test->getName());
            SkDebugf("INSTRUMENTATION_STATUS: class=com.skia\n");
            SkDebugf("INSTRUMENTATION_STATUS: current=%d\n", fIndex+1);
            SkDebugf("INSTRUMENTATION_STATUS: numtests=%d\n", fTotal);
            SkDebugf("INSTRUMENTATION_STATUS_CODE: %d\n", state);
        } else {
            if (kStarting_State == state) {
                SkDebugf("[%d/%d] %s...\n", fIndex+1, fTotal, test->getName());
            } else if (kFailed_State == state) {
                SkDebugf("---- FAILED\n");
            }
        }
    }

    int fIndex, fTotal;
    bool fAndroidMode;
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
    
    bool androidMode = false;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-android")) {
            androidMode = true;
        }
    }

    DebugfReporter reporter(androidMode);
    Iter iter(&reporter);
    Test* test;

    const int count = Iter::Count();
    int index = 0;
    int successCount = 0;
    while ((test = iter.next()) != NULL) {
        reporter.setIndexOfTotal(index, count);
        successCount += test->run();
        SkDELETE(test);
        index += 1;
    }

    if (!androidMode) {
        SkDebugf("Finished %d tests, %d failures.\n", count,
                 count - successCount);
    }
    return 0;
}
