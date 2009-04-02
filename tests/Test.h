#ifndef skiatest_Test_DEFINED
#define skiatest_Test_DEFINED

#include "SkRefCnt.h"
#include "SkString.h"
#include "SkTRegistry.h"

namespace skiatest {

    class Test;

    class Reporter : public SkRefCnt {
    public:
        Reporter();

        enum Result {
            kPassed,    // must begin with 0
            kFailed,
            /////
            kLastResult = kFailed
        };

        void resetReporting();
        int countTests() const { return fTestCount; }
        int countResults(Result r) {
            SkASSERT((unsigned)r <= kLastResult);
            return fResultCount[r];
        }

        void startTest(Test*);
        void report(const char testDesc[], Result);
        void endTest(Test*);

        // helpers for tests
        void assertTrue(bool cond, const char desc[]) {
            if (!cond) {
                this->report(desc, kFailed);
            }
        }
        void assertFalse(bool cond, const char desc[]) {
            if (cond) {
                this->report(desc, kFailed);
            }
        }
        void reportFailed(const char desc[]) {
            this->report(desc, kFailed);
        }
        void reportFailed(const SkString& desc) {
            this->report(desc.c_str(), kFailed);
        }

    protected:
        virtual void onStart(Test*) {}
        virtual void onReport(const char desc[], Result) {}
        virtual void onEnd(Test*) {}

    private:
        Test* fCurrTest;
        int fTestCount;
        int fResultCount[kLastResult+1];

        typedef SkRefCnt INHERITED;
    };

    class Test {
    public:
        Test();
        virtual ~Test();

        Reporter* getReporter() const { return fReporter; }
        void setReporter(Reporter*);

        const char* getName();
        void run();

    protected:
        virtual void onGetName(SkString*) = 0;
        virtual void onRun(Reporter*) = 0;

    private:
        Reporter*   fReporter;
        SkString    fName;
    };

    typedef SkTRegistry<Test*, void*> TestRegistry;
}

#define REPORTER_ASSERT(r, cond)                                        \
    do {                                                                \
        if (!(cond)) {                                                  \
            SkString desc;                                              \
            desc.printf("%s:%d: %s", __FILE__, __LINE__, #cond);      \
            r->reportFailed(desc);                                      \
        }                                                               \
    } while(0)


#endif
