
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skiatest_Test_DEFINED
#define skiatest_Test_DEFINED

#include "SkString.h"
#include "SkTRegistry.h"
#include "SkTypes.h"

class GrContextFactory;

namespace skiatest {

SkString GetTmpDir();

struct Failure {
    Failure(const char* f, int l, const char* c, const SkString& m)
        : fileName(f), lineNo(l), condition(c), message(m) {}
    const char* fileName;
    int lineNo;
    const char* condition;
    SkString message;
    SkString toString() const;
};

class Reporter : SkNoncopyable {
public:
    virtual ~Reporter() {}
    virtual void bumpTestCount();
    virtual void reportFailed(const skiatest::Failure&) = 0;
    virtual bool allowExtendedTest() const;
    virtual bool verbose() const;
};

#define REPORT_FAILURE(reporter, cond, message) \
    reporter->reportFailed(skiatest::Failure(__FILE__, __LINE__, cond, message))

typedef void (*TestProc)(skiatest::Reporter*, GrContextFactory*);

struct Test {
    Test(const char* n, bool g, TestProc p) : name(n), needsGpu(g), proc(p) {}
    const char* name;
    bool needsGpu;
    TestProc proc;
};

typedef SkTRegistry<Test> TestRegistry;

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
}  // namespace skiatest

#define REPORTER_ASSERT(r, cond)                  \
    do {                                          \
        if (!(cond)) {                            \
            REPORT_FAILURE(r, #cond, SkString()); \
        }                                         \
    } while (0)

#define REPORTER_ASSERT_MESSAGE(r, cond, message)        \
    do {                                                 \
        if (!(cond)) {                                   \
            REPORT_FAILURE(r, #cond, SkString(message)); \
        }                                                \
    } while (0)

#define ERRORF(r, ...)                                      \
    do {                                                    \
        REPORT_FAILURE(r, "", SkStringPrintf(__VA_ARGS__)); \
    } while (0)

#define DEF_TEST(name, reporter)                                     \
    static void test_##name(skiatest::Reporter*, GrContextFactory*); \
    skiatest::TestRegistry name##TestRegistry(                       \
            skiatest::Test(#name, false, test_##name));              \
    void test_##name(skiatest::Reporter* reporter, GrContextFactory*)

#define DEF_GPUTEST(name, reporter, factory)                         \
    static void test_##name(skiatest::Reporter*, GrContextFactory*); \
    skiatest::TestRegistry name##TestRegistry(                       \
            skiatest::Test(#name, true, test_##name));               \
    void test_##name(skiatest::Reporter* reporter, GrContextFactory* factory)

#endif
