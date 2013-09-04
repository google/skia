
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skiatest_Test_DEFINED
#define skiatest_Test_DEFINED

#include "SkRefCnt.h"
#include "SkString.h"
#include "SkTRegistry.h"
#include "SkThread.h"
#include "SkTypes.h"

class GrContextFactory;

namespace skiatest {

    class Test;

    class Reporter : public SkRefCnt {
    public:
        SK_DECLARE_INST_COUNT(Reporter)
        Reporter();

        int countTests() const { return fTestCount; }

        void startTest(Test*);
        void reportFailed(const SkString& desc);
        void endTest(Test*);

        virtual bool allowExtendedTest() const { return false; }
        virtual bool allowThreaded() const { return false; }
        virtual bool verbose() const { return false; }
        virtual void bumpTestCount() { sk_atomic_inc(&fTestCount); }

    protected:
        virtual void onStart(Test*) {}
        virtual void onReportFailed(const SkString& desc) {}
        virtual void onEnd(Test*) {}

    private:
        int32_t fTestCount;

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
        bool passed() const { return fPassed; }
        SkMSec elapsedMs() const { return fElapsed; }

        static SkString GetTmpDir();

        static SkString GetResourcePath();

        virtual bool isThreadsafe() const { return true; }

    protected:
        virtual void onGetName(SkString*) = 0;
        virtual void onRun(Reporter*) = 0;

    private:
        Reporter*   fReporter;
        SkString    fName;
        bool        fPassed;
        SkMSec      fElapsed;
    };

    class GpuTest : public Test{
    public:
        GpuTest() : Test() {}
        static GrContextFactory* GetGrContextFactory();
        static void DestroyContexts();
        virtual bool isThreadsafe() const { return false; }
    private:
    };

    typedef SkTRegistry<Test*(*)(void*)> TestRegistry;
}

#define REPORTER_ASSERT(r, cond)                                        \
    do {                                                                \
        if (!(cond)) {                                                  \
            SkString desc;                                              \
            desc.printf("%s:%d: %s", __FILE__, __LINE__, #cond);        \
            r->reportFailed(desc);                                      \
        }                                                               \
    } while(0)

#define REPORTER_ASSERT_MESSAGE(r, cond, message)                            \
    do {                                                                     \
        if (!(cond)) {                                                       \
            SkString desc;                                                   \
            desc.printf("%s %s:%d: %s", message, __FILE__, __LINE__, #cond); \
            r->reportFailed(desc);                                           \
        }                                                                    \
    } while(0)


#endif
