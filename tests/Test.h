
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

    /**
     *  Information about a single failure from a Test.
     *
     *  Not intended to be created/modified directly. To create one, use one of
     *
     *  REPORTER_ASSERT
     *  REPORTER_ASSERT_MESSAGE
     *  ERRORF
     *
     *  described in more detail further down in this file.
     */
    struct Failure {
        const char* fileName;
        int lineNo;
        const char* condition;
        SkString message;

        // Helper to combine the failure info into one string.
        void getFailureString(SkString* result) const {
            if (!result) {
                return;
            }
            result->printf("%s:%d\t", fileName, lineNo);
            if (!message.isEmpty()) {
                result->append(message);
                if (strlen(condition) > 0) {
                    result->append(": ");
                }
            }
            result->append(condition);
        }
    };


    class Reporter : public SkRefCnt {
    public:
        SK_DECLARE_INST_COUNT(Reporter)
        Reporter();

        int countTests() const { return fTestCount; }

        void startTest(Test*);
        void reportFailed(const Failure&);
        void endTest(Test*);

        virtual bool allowExtendedTest() const { return false; }
        virtual bool verbose() const { return false; }
        virtual void bumpTestCount() { sk_atomic_inc(&fTestCount); }

    protected:
        virtual void onStart(Test*) {}
        virtual void onReportFailed(const Failure&) {}
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

        virtual bool isGPUTest() const { return false; }
        virtual void setGrContextFactory(GrContextFactory* factory) {}

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
        GpuTest() : Test(), fGrContextFactory(NULL) {}

        virtual bool isGPUTest() const { return true; }
        virtual void setGrContextFactory(GrContextFactory* factory) {
            fGrContextFactory = factory;
        }

    protected:
        GrContextFactory* fGrContextFactory;  // Unowned.
    };

    typedef SkTRegistry<Test*(*)(void*)> TestRegistry;
}  // namespace skiatest

/*
    Use the following macros to make use of the skiatest classes, e.g.

    #include "Test.h"

    DEF_TEST(TestName, reporter) {
        ...
        REPORTER_ASSERT(reporter, x == 15);
        ...
        REPORTER_ASSERT_MESSAGE(reporter, x == 15, "x should be 15");
        ...
        if (x != 15) {
            ERRORF(reporter, "x should be 15, but is %d", x);
            return;
        }
        ...
    }
*/

#define REPORTER_ASSERT(r, cond)                                        \
    do {                                                                \
        if (!(cond)) {                                                  \
            skiatest::Failure failure = { __FILE__, __LINE__,           \
                                          #cond, SkString() };          \
            r->reportFailed(failure);                                   \
        }                                                               \
    } while(0)

#define REPORTER_ASSERT_MESSAGE(r, cond, message)                       \
    do {                                                                \
        if (!(cond)) {                                                  \
            skiatest::Failure failure = { __FILE__, __LINE__,           \
                                          #cond, SkString(message) };   \
            r->reportFailed(failure);                                   \
        }                                                               \
    } while(0)

#define ERRORF(r, ...)                                                  \
    do {                                                                \
        SkString desc;                                                  \
        desc.appendf(__VA_ARGS__) ;                                     \
        skiatest::Failure failure = { __FILE__, __LINE__,               \
                                      "", SkString(desc) };             \
        r->reportFailed(failure);                                       \
    } while(0)

#define DEF_TEST(name, reporter)                                        \
    static void test_##name(skiatest::Reporter*);                       \
    namespace skiatest {                                                \
    class name##Class : public Test {                                   \
    public:                                                             \
        static Test* Factory(void*) { return SkNEW(name##Class); }      \
    protected:                                                          \
        virtual void onGetName(SkString* name) SK_OVERRIDE {            \
            name->set(#name);                                           \
        }                                                               \
        virtual void onRun(Reporter* r) SK_OVERRIDE { test_##name(r); } \
    };                                                                  \
    static TestRegistry gReg_##name##Class(name##Class::Factory);       \
    }                                                                   \
    static void test_##name(skiatest::Reporter* reporter)

#define DEF_GPUTEST(name, reporter, factory)                          \
    static void test_##name(skiatest::Reporter*, GrContextFactory*);  \
    namespace skiatest {                                              \
    class name##Class : public GpuTest {                              \
    public:                                                           \
        static Test* Factory(void*) { return SkNEW(name##Class); }    \
    protected:                                                        \
        virtual void onGetName(SkString* name) SK_OVERRIDE {          \
            name->set(#name);                                         \
        }                                                             \
        virtual void onRun(Reporter* r) SK_OVERRIDE {                 \
            test_##name(r, fGrContextFactory);                        \
        }                                                             \
    };                                                                \
    static TestRegistry gReg_##name##Class(name##Class::Factory);     \
    }                                                                 \
    static void test_##name(skiatest::Reporter* reporter, GrContextFactory* factory)

#endif
