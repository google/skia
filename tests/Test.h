
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
class GrContext;
class SkGLContext;

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
enum GPUTestContexts {
    kNone_GPUTestContexts         = 0,
    kNull_GPUTestContexts         = 1,
    kDebug_GPUTestContexts        = 1 << 1,
    kNative_GPUTestContexts       = 1 << 2,
    kOther_GPUTestContexts        = 1 << 3, // Other than native, used only for below.
    kAllRendering_GPUTestContexts = kNative_GPUTestContexts | kOther_GPUTestContexts,
    kAll_GPUTestContexts          = kAllRendering_GPUTestContexts
                                       | kNull_GPUTestContexts
                                       | kDebug_GPUTestContexts
};
template<typename T>
void RunWithGPUTestContexts(T testFunction, GPUTestContexts contexts, Reporter* reporter,
                            GrContextFactory* factory);
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

#define INFOF(REPORTER, ...)         \
    do {                             \
        if ((REPORTER)->verbose()) { \
            SkDebugf(__VA_ARGS__);   \
        }                            \
    } while (0)

#define DEF_TEST(name, reporter)                                     \
    static void test_##name(skiatest::Reporter*, GrContextFactory*); \
    skiatest::TestRegistry name##TestRegistry(                       \
            skiatest::Test(#name, false, test_##name));              \
    void test_##name(skiatest::Reporter* reporter, GrContextFactory*)

#define GPUTEST_EXPAND_MSVC(x) x
#define GPUTEST_APPLY(C, ...) GPUTEST_EXPAND_MSVC(C(__VA_ARGS__))
#define GPUTEST_SELECT(a1, a2, N, ...) N

#define GPUTEST_CONTEXT_ARGS1(a1) GrContext* a1
#define GPUTEST_CONTEXT_ARGS2(a1, a2) GrContext* a1, SkGLContext* a2
#define GPUTEST_CONTEXT_ARGS(...)                                                            \
    GPUTEST_APPLY(GPUTEST_SELECT(__VA_ARGS__, GPUTEST_CONTEXT_ARGS2, GPUTEST_CONTEXT_ARGS1), \
                  __VA_ARGS__)

#define DEF_GPUTEST(name, reporter, factory)                         \
    static void test_##name(skiatest::Reporter*, GrContextFactory*); \
    skiatest::TestRegistry name##TestRegistry(                       \
            skiatest::Test(#name, true, test_##name));               \
    void test_##name(skiatest::Reporter* reporter, GrContextFactory* factory)

#define DEF_GPUTEST_FOR_CONTEXTS(name, contexts, reporter, ...)                      \
    static void test_##name(skiatest::Reporter*, GPUTEST_CONTEXT_ARGS(__VA_ARGS__)); \
    static void test_gpu_contexts_##name(skiatest::Reporter* reporter,               \
                                         GrContextFactory* factory) {                \
        skiatest::RunWithGPUTestContexts(test_##name, contexts, reporter, factory);  \
    }                                                                                \
    skiatest::TestRegistry name##TestRegistry(                                       \
            skiatest::Test(#name, true, test_gpu_contexts_##name));                  \
    void test_##name(skiatest::Reporter* reporter, GPUTEST_CONTEXT_ARGS(__VA_ARGS__))

#define DEF_GPUTEST_FOR_ALL_CONTEXTS(name, reporter, ...) \
        DEF_GPUTEST_FOR_CONTEXTS(name, skiatest::kAll_GPUTestContexts, reporter, __VA_ARGS__)
#define DEF_GPUTEST_FOR_RENDERING_CONTEXTS(name, reporter, ...) \
        DEF_GPUTEST_FOR_CONTEXTS(name, skiatest::kAllRendering_GPUTestContexts, reporter, __VA_ARGS__)
#define DEF_GPUTEST_FOR_NULL_CONTEXT(name, reporter, ...) \
        DEF_GPUTEST_FOR_CONTEXTS(name, skiatest::kNull_GPUTestContexts, reporter, __VA_ARGS__)
#define DEF_GPUTEST_FOR_NATIVE_CONTEXT(name, reporter, ...) \
        DEF_GPUTEST_FOR_CONTEXTS(name, skiatest::kNative_GPUTestContexts, reporter, __VA_ARGS__)

#define REQUIRE_PDF_DOCUMENT(TEST_NAME, REPORTER)                             \
    do {                                                                      \
        SkDynamicMemoryWStream testStream;                                    \
        SkAutoTUnref<SkDocument> testDoc(SkDocument::CreatePDF(&testStream)); \
        if (!testDoc) {                                                       \
            INFOF(REPORTER, "PDF disabled; %s test skipped.", #TEST_NAME);    \
            return;                                                           \
        }                                                                     \
    } while (false)

#endif
