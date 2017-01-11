/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skiatest_Test_DEFINED
#define skiatest_Test_DEFINED

#include "SkString.h"
#include "../tools/Registry.h"
#include "SkTypes.h"
#include "SkClipOpPriv.h"

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#else
namespace sk_gpu_test {
class GrContextFactory;
class ContextInfo;
class GLTestContext;
}  // namespace sk_gpu_test
class GrContext;
#endif

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
    virtual void* stats() const { return nullptr; }

    void reportFailedWithContext(const skiatest::Failure& f) {
        SkString fullMessage = f.message;
        if (!fContextStack.empty()) {
            fullMessage.append(" [");
            for (int i = 0; i < fContextStack.count(); ++i) {
                if (i > 0) {
                    fullMessage.append(", ");
                }
                fullMessage.append(fContextStack[i]);
            }
            fullMessage.append("]");
        }
        this->reportFailed(skiatest::Failure(f.fileName, f.lineNo, f.condition, fullMessage));
    }
    void push(const SkString& message) {
        fContextStack.push_back(message);
    }
    void pop() {
        fContextStack.pop_back();
    }

private:
    SkTArray<SkString> fContextStack;
};

#define REPORT_FAILURE(reporter, cond, message) \
    reporter->reportFailedWithContext(skiatest::Failure(__FILE__, __LINE__, cond, message))

class ReporterContext : SkNoncopyable {
public:
    ReporterContext(Reporter* reporter, const SkString& message) : fReporter(reporter) {
        fReporter->push(message);
    }
    ~ReporterContext() {
        fReporter->pop();
    }

private:
    Reporter* fReporter;
};

typedef void (*TestProc)(skiatest::Reporter*, sk_gpu_test::GrContextFactory*);

struct Test {
    Test(const char* n, bool g, TestProc p) : name(n), needsGpu(g), proc(p) {}
    const char* name;
    bool needsGpu;
    TestProc proc;
};

typedef sk_tools::Registry<Test> TestRegistry;

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

#if SK_SUPPORT_GPU
using GrContextFactoryContextType = sk_gpu_test::GrContextFactory::ContextType;
#else
using GrContextFactoryContextType = int;
#endif

typedef void GrContextTestFn(Reporter*, const sk_gpu_test::ContextInfo&);
typedef bool GrContextTypeFilterFn(GrContextFactoryContextType);

extern bool IsGLContextType(GrContextFactoryContextType);
extern bool IsVulkanContextType(GrContextFactoryContextType);
extern bool IsRenderingGLContextType(GrContextFactoryContextType);
extern bool IsNullGLContextType(GrContextFactoryContextType);
void RunWithGPUTestContexts(GrContextTestFn*, GrContextTypeFilterFn*,
                            Reporter*, sk_gpu_test::GrContextFactory*);

/** Timer provides wall-clock duration since its creation. */
class Timer {
public:
    /** Starts the timer. */
    Timer();

    /** Nanoseconds since creation. */
    double elapsedNs() const;

    /** Milliseconds since creation. */
    double elapsedMs() const;

    /** Milliseconds since creation as an integer.
        Behavior is undefined for durations longer than SK_MSecMax.
    */
    SkMSec elapsedMsInt() const;
private:
    double fStartNanos;
};

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

#define DEF_TEST(name, reporter)                                                  \
    static void test_##name(skiatest::Reporter*, sk_gpu_test::GrContextFactory*); \
    skiatest::TestRegistry name##TestRegistry(                                    \
            skiatest::Test(#name, false, test_##name));                           \
    void test_##name(skiatest::Reporter* reporter, sk_gpu_test::GrContextFactory*)


#define DEF_GPUTEST(name, reporter, factory)                                                 \
    static void test_##name(skiatest::Reporter*, sk_gpu_test::GrContextFactory*);            \
    skiatest::TestRegistry name##TestRegistry(                                               \
            skiatest::Test(#name, true, test_##name));                                       \
    void test_##name(skiatest::Reporter* reporter, sk_gpu_test::GrContextFactory* factory)

#define DEF_GPUTEST_FOR_CONTEXTS(name, context_filter, reporter, context_info)            \
    static void test_##name(skiatest::Reporter*,                                          \
                            const sk_gpu_test::ContextInfo& context_info);                \
    static void test_gpu_contexts_##name(skiatest::Reporter* reporter,                    \
                                         sk_gpu_test::GrContextFactory* factory) {        \
        skiatest::RunWithGPUTestContexts(test_##name, context_filter, reporter, factory); \
    }                                                                                     \
    skiatest::TestRegistry name##TestRegistry(                                            \
            skiatest::Test(#name, true, test_gpu_contexts_##name));                       \
    void test_##name(skiatest::Reporter* reporter,                                        \
                     const sk_gpu_test::ContextInfo& context_info)

#define DEF_GPUTEST_FOR_ALL_CONTEXTS(name, reporter, context_info)                          \
        DEF_GPUTEST_FOR_CONTEXTS(name, nullptr, reporter, context_info)
#define DEF_GPUTEST_FOR_RENDERING_CONTEXTS(name, reporter, context_info)                    \
        DEF_GPUTEST_FOR_CONTEXTS(name, sk_gpu_test::GrContextFactory::IsRenderingContext,   \
                                 reporter, context_info)
#define DEF_GPUTEST_FOR_ALL_GL_CONTEXTS(name, reporter, context_info)                       \
        DEF_GPUTEST_FOR_CONTEXTS(name, &skiatest::IsGLContextType, reporter, context_info)
#define DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(name, reporter, context_info)                 \
        DEF_GPUTEST_FOR_CONTEXTS(name, &skiatest::IsRenderingGLContextType, reporter, context_info)
#define DEF_GPUTEST_FOR_NULLGL_CONTEXT(name, reporter, context_info)                        \
        DEF_GPUTEST_FOR_CONTEXTS(name, &skiatest::IsNullGLContextType, reporter, context_info)
#define DEF_GPUTEST_FOR_VULKAN_CONTEXT(name, reporter, context_info)                        \
        DEF_GPUTEST_FOR_CONTEXTS(name, &skiatest::IsVulkanContextType, reporter, context_info)

#define REQUIRE_PDF_DOCUMENT(TEST_NAME, REPORTER)                          \
    do {                                                                   \
        SkDynamicMemoryWStream testStream;                                 \
        sk_sp<SkDocument> testDoc(SkDocument::MakePDF(&testStream));       \
        if (!testDoc) {                                                    \
            INFOF(REPORTER, "PDF disabled; %s test skipped.", #TEST_NAME); \
            return;                                                        \
        }                                                                  \
    } while (false)

#endif
