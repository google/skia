/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skiatest_Test_DEFINED
#define skiatest_Test_DEFINED

#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "src/core/SkTraceEvent.h"
#include "tests/CtsEnforcement.h"
#include "tools/Registry.h"
#include "tools/gpu/GrContextFactory.h"

namespace skgpu::graphite { class Context; }

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

    void reportFailedWithContext(const skiatest::Failure&);

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

typedef void (*TestProc)(skiatest::Reporter*, const GrContextOptions&);
typedef void (*ContextOptionsProc)(GrContextOptions*);

enum class TestType : uint8_t { kCPU, kGanesh, kGraphite };

struct Test {
    Test(const char* name,
         TestType testType,
         CtsEnforcement ctsEnforcement,
         TestProc proc,
         ContextOptionsProc optionsProc = nullptr)
            : fName(name)
            , fTestType(testType)
            , fCTSEnforcement(ctsEnforcement)
            , fProc(proc)
            , fContextOptionsProc(optionsProc) {}
    const char* fName;
    TestType fTestType;
    CtsEnforcement fCTSEnforcement;
    TestProc fProc;
    ContextOptionsProc fContextOptionsProc;

    void modifyGrContextOptions(GrContextOptions* options) {
        if (fContextOptionsProc) {
            (*fContextOptionsProc)(options);
        }
    }

    void run(skiatest::Reporter* r, const GrContextOptions& options) const {
        TRACE_EVENT1("test", TRACE_FUNC, "name", this->fName/*these are static*/);
        this->fProc(r, options);
    }
};

using TestRegistry = sk_tools::Registry<Test>;

using GrContextFactoryContextType = sk_gpu_test::GrContextFactory::ContextType;

typedef void GrContextTestFn(Reporter*, const sk_gpu_test::ContextInfo&);
typedef bool GrContextTypeFilterFn(GrContextFactoryContextType);

// We want to run the same test against potentially multiple Ganesh backends. Test runners should
// implement this function by calling the testFn with a fresh ContextInfo if that backend matches
// the provided filter. If filter is nullptr, then all compiled-in Ganesh backends should be used.
// The reporter and opts arguments are piped in from Test::run.
void RunWithGaneshTestContexts(GrContextTestFn* testFn, GrContextTypeFilterFn* filter,
                               Reporter* reporter, const GrContextOptions& options);

// These context filters should be implemented by test runners and return true if the backend was
// compiled in (i.e. is supported) and matches the criteria indicated by the name of the filter.
extern bool IsGLContextType(GrContextFactoryContextType);
extern bool IsVulkanContextType(GrContextFactoryContextType);
extern bool IsMetalContextType(GrContextFactoryContextType);
extern bool IsDawnContextType(GrContextFactoryContextType);
extern bool IsDirect3DContextType(GrContextFactoryContextType);
extern bool IsRenderingGLContextType(GrContextFactoryContextType);
extern bool IsMockContextType(GrContextFactoryContextType);

namespace graphite {

typedef void GraphiteTestFn(Reporter*, skgpu::graphite::Context*);

void RunWithGraphiteTestContexts(GraphiteTestFn*, Reporter*);

} // namespace graphite

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

/*
    Use the following macros to make use of the skiatest classes, e.g.

    #include "tests/Test.h"

    DEF_TEST(TestName, reporter) {
        ...
        REPORTER_ASSERT(reporter, x == 15);
        ...
        REPORTER_ASSERT(reporter, x == 15, "x should be 15");
        ...
        if (x != 15) {
            ERRORF(reporter, "x should be 15, but is %d", x);
            return;
        }
        ...
    }
*/

#define REPORTER_ASSERT(r, cond, ...)                              \
    do {                                                           \
        if (!(cond)) {                                             \
            REPORT_FAILURE(r, #cond, SkStringPrintf(__VA_ARGS__)); \
        }                                                          \
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

#define DEF_CONDITIONAL_TEST(name, reporter, condition)                                \
    static void test_##name(skiatest::Reporter*, const GrContextOptions&);             \
    skiatest::TestRegistry name##TestRegistry(skiatest::Test(#name,                    \
                                                             skiatest::TestType::kCPU, \
                                                             CtsEnforcement::kNever,   \
                                                             test_##name),             \
                                              condition);                              \
    void test_##name(skiatest::Reporter* reporter, const GrContextOptions&)

#define DEF_TEST(name, reporter) DEF_CONDITIONAL_TEST(name, reporter, true)

#define DEF_TEST_DISABLED(name, reporter) DEF_CONDITIONAL_TEST(name, reporter, false)

#ifdef SK_BUILD_FOR_UNIX
    #define UNIX_ONLY_TEST DEF_TEST
#else
    #define UNIX_ONLY_TEST DEF_TEST_DISABLED
#endif

// TODO update all the callsites to support CtsEnforcement
#define DEF_GRAPHITE_TEST(name, reporter)                                                     \
    static void test_##name(skiatest::Reporter*);                                             \
    static void test_graphite_##name(skiatest::Reporter* reporter,                            \
                                     const GrContextOptions& /*unused*/) {                    \
        test_##name(reporter);                                                                \
    }                                                                                         \
    skiatest::TestRegistry name##TestRegistry(skiatest::Test(#name,                           \
                                                             skiatest::TestType::kGraphite,   \
                                                             CtsEnforcement::kNever,          \
                                                             test_graphite_##name));          \
    void test_##name(skiatest::Reporter* reporter)

// TODO update all the callsites to support CtsEnforcement
#define DEF_GRAPHITE_TEST_FOR_CONTEXTS(name, reporter, graphite_context)                      \
    static void test_##name(skiatest::Reporter*, skgpu::graphite::Context*);                  \
    static void test_graphite_contexts_##name(skiatest::Reporter* _reporter,                  \
                                              const GrContextOptions& /*unused*/) {           \
        skiatest::graphite::RunWithGraphiteTestContexts(test_##name, _reporter);              \
    }                                                                                         \
    skiatest::TestRegistry name##TestRegistry(skiatest::Test(#name,                           \
                                                             skiatest::TestType::kGraphite,   \
                                                             CtsEnforcement::kNever,          \
                                                             test_graphite_contexts_##name)); \
    void test_##name(skiatest::Reporter* reporter, skgpu::graphite::Context* graphite_context)

#define DEF_GANESH_TEST(name, reporter, options, ctsEnforcement)                        \
    static void test_##name(skiatest::Reporter*, const GrContextOptions&);              \
    skiatest::TestRegistry name##TestRegistry(skiatest::Test(                           \
            #name, skiatest::TestType::kGanesh, ctsEnforcement, test_##name, nullptr)); \
    void test_##name(skiatest::Reporter* reporter, const GrContextOptions& options)

#define DEF_CONDITIONAL_GANESH_TEST_FOR_CONTEXTS(                                                \
        name, context_filter, reporter, context_info, options_filter, condition, ctsEnforcement) \
    static void test_##name(skiatest::Reporter*, const sk_gpu_test::ContextInfo&);               \
    static void test_gpu_contexts_##name(skiatest::Reporter* reporter,                           \
                                         const GrContextOptions& options) {                      \
        skiatest::RunWithGaneshTestContexts(test_##name, context_filter, reporter, options);     \
    }                                                                                            \
    skiatest::TestRegistry name##TestRegistry(skiatest::Test(#name,                              \
                                                             skiatest::TestType::kGanesh,        \
                                                             ctsEnforcement,                     \
                                                             test_gpu_contexts_##name,           \
                                                             options_filter),                    \
                                              condition);                                        \
    void test_##name(skiatest::Reporter* reporter, const sk_gpu_test::ContextInfo& context_info)

#define DEF_CONDITIONAL_GANESH_TEST_FOR_ALL_CONTEXTS(            \
        name, reporter, context_info, condition, ctsEnforcement) \
    DEF_CONDITIONAL_GANESH_TEST_FOR_CONTEXTS(                    \
            name, nullptr, reporter, context_info, nullptr, condition, ctsEnforcement)

#define DEF_CONDITIONAL_GANESH_TEST_FOR_RENDERING_CONTEXTS(                                     \
        name, reporter, context_info, condition, ctsEnforcement)                                \
    DEF_CONDITIONAL_GANESH_TEST_FOR_CONTEXTS(name,                                              \
                                             sk_gpu_test::GrContextFactory::IsRenderingContext, \
                                             reporter,                                          \
                                             context_info,                                      \
                                             nullptr,                                           \
                                             condition,                                         \
                                             ctsEnforcement)

#define DEF_GANESH_TEST_FOR_CONTEXTS(                                                 \
        name, context_filter, reporter, context_info, options_filter, ctsEnforcement) \
    DEF_CONDITIONAL_GANESH_TEST_FOR_CONTEXTS(                                         \
            name, context_filter, reporter, context_info, options_filter, true, ctsEnforcement)

#define DEF_GANESH_TEST_FOR_ALL_CONTEXTS(name, reporter, context_info, ctsEnforcement) \
    DEF_GANESH_TEST_FOR_CONTEXTS(name, nullptr, reporter, context_info, nullptr, ctsEnforcement)

#define DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(name, reporter, context_info, ctsEnforcement) \
    DEF_GANESH_TEST_FOR_CONTEXTS(name,                                                       \
                                 sk_gpu_test::GrContextFactory::IsRenderingContext,          \
                                 reporter,                                                   \
                                 context_info,                                               \
                                 nullptr,                                                    \
                                 ctsEnforcement)
#define DEF_GANESH_TEST_FOR_ALL_GL_CONTEXTS(name, reporter, context_info, ctsEnforcement) \
    DEF_GANESH_TEST_FOR_CONTEXTS(                                                         \
            name, &skiatest::IsGLContextType, reporter, context_info, nullptr, ctsEnforcement)
#define DEF_GANESH_TEST_FOR_GL_RENDERING_CONTEXTS(name, reporter, context_info, ctsEnforcement) \
    DEF_GANESH_TEST_FOR_CONTEXTS(name,                                                          \
                                 &skiatest::IsRenderingGLContextType,                           \
                                 reporter,                                                      \
                                 context_info,                                                  \
                                 nullptr,                                                       \
                                 ctsEnforcement)
#define DEF_GANESH_TEST_FOR_MOCK_CONTEXT(name, reporter, context_info) \
    DEF_GANESH_TEST_FOR_CONTEXTS(name,                                 \
                                 &skiatest::IsMockContextType,         \
                                 reporter,                             \
                                 context_info,                         \
                                 nullptr,                              \
                                 CtsEnforcement::kNever)
#define DEF_GANESH_TEST_FOR_VULKAN_CONTEXT(name, reporter, context_info, ctsEnforcement) \
    DEF_GANESH_TEST_FOR_CONTEXTS(                                                        \
            name, &skiatest::IsVulkanContextType, reporter, context_info, nullptr, ctsEnforcement)
#define DEF_GANESH_TEST_FOR_METAL_CONTEXT(name, reporter, context_info) \
    DEF_GANESH_TEST_FOR_CONTEXTS(name,                                  \
                                 &skiatest::IsMetalContextType,         \
                                 reporter,                              \
                                 context_info,                          \
                                 nullptr,                               \
                                 CtsEnforcement::kNever)
#define DEF_GANESH_TEST_FOR_D3D_CONTEXT(name, reporter, context_info) \
    DEF_GANESH_TEST_FOR_CONTEXTS(name,                                \
                                 &skiatest::IsDirect3DContextType,    \
                                 reporter,                            \
                                 context_info,                        \
                                 nullptr,                             \
                                 CtsEnforcement::kNever)
#define DEF_GANESH_TEST_FOR_DAWN_CONTEXT(name, reporter, context_info) \
    DEF_GANESH_TEST_FOR_CONTEXTS(name,                                 \
                                 &skiatest::IsDawnContextType,         \
                                 reporter,                             \
                                 context_info,                         \
                                 nullptr,                              \
                                 CtsEnforcement::kNever)

#define REQUIRE_PDF_DOCUMENT(TEST_NAME, REPORTER)                          \
    do {                                                                   \
        SkNullWStream testStream;                                          \
        auto testDoc = SkPDF::MakeDocument(&testStream);                   \
        if (!testDoc) {                                                    \
            INFOF(REPORTER, "PDF disabled; %s test skipped.", #TEST_NAME); \
            return;                                                        \
        }                                                                  \
    } while (false)

#endif
