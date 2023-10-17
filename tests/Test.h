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
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkTraceEvent.h"
#include "tests/CtsEnforcement.h"
#include "tools/Registry.h"

#if defined(SK_GANESH) || defined(SK_GRAPHITE)
namespace skgpu { enum class ContextType; }
#endif

#if defined(SK_GANESH)
#include "tools/gpu/GrContextFactory.h" // IWYU pragma: export (because it is used by a macro)
#else
namespace sk_gpu_test { class ContextInfo; }
#endif

#include <atomic>
#include <cstdint>
#include <string>

struct GrContextOptions;

namespace skgpu::graphite {
class Context;
struct ContextOptions;
}

namespace skiatest {
namespace graphite {
class GraphiteTestContext;
}

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

    /**
     * Show additional context (e.g. subtest name) on failure assertions.
     */
    void push(const SkString& message) {
        fContextStack.push_back(message);
    }
    void push(const std::string& message) {
        fContextStack.push_back(SkString(message));
    }

    /**
     * Remove additional context from failure assertions.
     */
    void pop() {
        fContextStack.pop_back();
    }

private:
    skia_private::TArray<SkString> fContextStack;
};

#define REPORT_FAILURE(reporter, cond, message) \
    reporter->reportFailedWithContext(skiatest::Failure(__FILE__, __LINE__, cond, message))

/**
 * Use this stack-allocated object to push and then automatically pop the context
 * (e.g. subtest name) for a test.
 */
class ReporterContext : SkNoncopyable {
public:
    ReporterContext(Reporter* reporter, const SkString& message) : fReporter(reporter) {
        fReporter->push(message);
    }
    ReporterContext(Reporter* reporter, const std::string& message) : fReporter(reporter) {
        fReporter->push(message);
    }
    ~ReporterContext() {
        fReporter->pop();
    }

private:
    Reporter* fReporter;
};

using CPUTestProc                = void (*)(skiatest::Reporter*);
using GaneshTestProc             = void (*)(skiatest::Reporter*, const GrContextOptions&);
using GaneshContextOptionsProc   = void (*)(GrContextOptions*);
using GraphiteTestProc           = void (*)(skiatest::Reporter*,
                                            const skgpu::graphite::ContextOptions&);
using GraphiteContextOptionsProc = void (*)(skgpu::graphite::ContextOptions*);

enum class TestType : uint8_t { kCPU, kGanesh, kGraphite };

struct Test {
    static Test MakeCPU(const char* name, CPUTestProc proc) {
        return Test{name, TestType::kCPU, CtsEnforcement::kNever,
                    proc, nullptr, nullptr, nullptr, nullptr};
    }

    static Test MakeGanesh(const char* name,
                           CtsEnforcement ctsEnforcement,
                           GaneshTestProc proc,
                           GaneshContextOptionsProc optionsProc = nullptr) {
        return Test{name, TestType::kGanesh, ctsEnforcement,
                    nullptr, proc, nullptr, optionsProc, nullptr};
    }

    static Test MakeGraphite(const char* name,
                             CtsEnforcement ctsEnforcement,
                             GraphiteTestProc proc,
                             GraphiteContextOptionsProc optionsProc = nullptr) {
        return Test{name, TestType::kGraphite, ctsEnforcement,
                    nullptr, nullptr, proc, nullptr, optionsProc};
    }

    const char* fName;
    TestType fTestType;
    CtsEnforcement fCTSEnforcement;
    CPUTestProc fCPUProc = nullptr;
    GaneshTestProc fGaneshProc = nullptr;
    GraphiteTestProc fGraphiteProc = nullptr;
    GaneshContextOptionsProc fGaneshContextOptionsProc = nullptr;
    GraphiteContextOptionsProc fGraphiteContextOptionsProc = nullptr;

    void modifyGrContextOptions(GrContextOptions* options) {
        if (fGaneshContextOptionsProc) {
            (*fGaneshContextOptionsProc)(options);
        }
    }

    void modifyGraphiteContextOptions(skgpu::graphite::ContextOptions* options) {
        if (fGraphiteContextOptionsProc) {
            (*fGraphiteContextOptionsProc)(options);
        }
    }

    void cpu(skiatest::Reporter* r) const {
        SkASSERT(this->fTestType == TestType::kCPU);
        TRACE_EVENT1("test_cpu", TRACE_FUNC, "name", this->fName/*these are static*/);
        this->fCPUProc(r);
    }

    void ganesh(skiatest::Reporter* r, const GrContextOptions& options) const {
        SkASSERT(this->fTestType == TestType::kGanesh);
        TRACE_EVENT1("test_ganesh", TRACE_FUNC, "name", this->fName/*these are static*/);
        this->fGaneshProc(r, options);
    }

    void graphite(skiatest::Reporter* r, const skgpu::graphite::ContextOptions& options) const {
        SkASSERT(this->fTestType == TestType::kGraphite);
        TRACE_EVENT1("test_graphite", TRACE_FUNC, "name", this->fName/*these are static*/);
        this->fGraphiteProc(r, options);
    }

private:
    Test(const char* name,
         TestType testType,
         CtsEnforcement ctsEnforcement,
         CPUTestProc cpuProc,
         GaneshTestProc ganeshProc,
         GraphiteTestProc graphiteProc,
         GaneshContextOptionsProc ganeshOptionsProc,
         GraphiteContextOptionsProc graphiteOptionsProc)
            : fName(name)
            , fTestType(testType)
            , fCTSEnforcement(ctsEnforcement)
            , fCPUProc(cpuProc)
            , fGaneshProc(ganeshProc)
            , fGraphiteProc(graphiteProc)
            , fGaneshContextOptionsProc(ganeshOptionsProc)
            , fGraphiteContextOptionsProc(graphiteOptionsProc) {}
};

using TestRegistry = sk_tools::Registry<Test>;

#if defined(SK_GANESH)
using GpuContextType = skgpu::ContextType;
#else
using GpuContextType = nullptr_t;
#endif

typedef void GrContextTestFn(Reporter*, const sk_gpu_test::ContextInfo&);
typedef bool ContextTypeFilterFn(GpuContextType);

// We want to run the same test against potentially multiple Ganesh backends. Test runners should
// implement this function by calling the testFn with a fresh ContextInfo if that backend matches
// the provided filter. If filter is nullptr, then all compiled-in Ganesh backends should be used.
// The reporter and opts arguments are piped in from Test::run.
void RunWithGaneshTestContexts(GrContextTestFn* testFn, ContextTypeFilterFn* filter,
                               Reporter* reporter, const GrContextOptions& options);

// These context filters should be implemented by test runners and return true if the backend was
// compiled in (i.e. is supported) and matches the criteria indicated by the name of the filter.
extern bool IsGLContextType(GpuContextType);
extern bool IsVulkanContextType(GpuContextType);
extern bool IsMetalContextType(GpuContextType);
extern bool IsDawnContextType(GpuContextType);
extern bool IsDirect3DContextType(GpuContextType);
extern bool IsMockContextType(GpuContextType);

namespace graphite {

using GraphiteTestFn = void(Reporter*,
                            skgpu::graphite::Context*,
                            skiatest::graphite::GraphiteTestContext*);

void RunWithGraphiteTestContexts(GraphiteTestFn*,
                                 ContextTypeFilterFn* filter,
                                 Reporter*,
                                 const skgpu::graphite::ContextOptions&);

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

using skiatest::Test;

#define DEF_CONDITIONAL_TEST(name, reporter, condition)                                      \
    static void test_##name(skiatest::Reporter*);                                            \
    skiatest::TestRegistry name##TestRegistry(Test::MakeCPU(#name, test_##name), condition); \
    void test_##name(skiatest::Reporter* reporter)

#define DEF_TEST(name, reporter) DEF_CONDITIONAL_TEST(name, reporter, true)

#define DEF_TEST_DISABLED(name, reporter) DEF_CONDITIONAL_TEST(name, reporter, false)

#ifdef SK_BUILD_FOR_UNIX
    #define UNIX_ONLY_TEST DEF_TEST
#else
    #define UNIX_ONLY_TEST DEF_TEST_DISABLED
#endif

#define DEF_GRAPHITE_TEST(name, reporter, ctsEnforcement)                                \
    static void test_##name(skiatest::Reporter*);                                        \
    static void test_graphite_##name(skiatest::Reporter* reporter,                       \
                                     const skgpu::graphite::ContextOptions&) {           \
        test_##name(reporter);                                                           \
    }                                                                                    \
    skiatest::TestRegistry name##TestRegistry(Test::MakeGraphite(#name, ctsEnforcement,  \
                                                                 test_graphite_##name)); \
    void test_##name(skiatest::Reporter* reporter)

#define DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(                                                \
        name, context_filter, reporter, graphite_ctx, test_ctx, opt_filter, cond, ctsEnforcement)  \
    static void test_##name(skiatest::Reporter*, skgpu::graphite::Context*,                        \
                            skiatest::graphite::GraphiteTestContext*);                             \
    static void test_graphite_contexts_##name(skiatest::Reporter* _reporter,                       \
                                              const skgpu::graphite::ContextOptions& ctxOptions) { \
        skiatest::graphite::RunWithGraphiteTestContexts(test_##name, context_filter,               \
                                                        _reporter, ctxOptions);                    \
    }                                                                                              \
    skiatest::TestRegistry name##TestRegistry(                                                     \
            Test::MakeGraphite(#name, ctsEnforcement, test_graphite_contexts_##name, opt_filter),  \
            cond);                                                                                 \
    void test_##name(skiatest::Reporter* reporter, skgpu::graphite::Context* graphite_ctx,         \
                     skiatest::graphite::GraphiteTestContext* test_ctx)

#define DEF_CONDITIONAL_GRAPHITE_TEST_FOR_ALL_CONTEXTS(name, reporter, graphite_ctx,            \
                                                       test_ctx, cond, ctsEnforcement)          \
    DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(name, nullptr, reporter, graphite_ctx, test_ctx, \
                                               nullptr, cond, ctsEnforcement)

#define DEF_CONDITIONAL_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(                 \
        name, reporter, graphite_context, test_context, cond, ctsEnforcement) \
    DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(name,                          \
                                               skgpu::IsRenderingContext,     \
                                               reporter,                      \
                                               graphite_context,              \
                                               test_context,                  \
                                               nullptr,                       \
                                               cond,                          \
                                               ctsEnforcement)

#define DEF_GRAPHITE_TEST_FOR_CONTEXTS(name, context_filter, reporter, graphite_ctx,         \
                                       test_ctx, ctsEnforcement)                             \
    DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(name, context_filter, reporter, graphite_ctx, \
                                               test_ctx, nullptr, true, ctsEnforcement)

#define DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(name, reporter, graphite_ctx, ctsEnforcement)         \
    DEF_CONDITIONAL_GRAPHITE_TEST_FOR_ALL_CONTEXTS(name, reporter, graphite_ctx,                 \
                                                   /*anonymous test_ctx*/, true, ctsEnforcement)

#define DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(name, reporter, graphite_context, ctsEnforcement) \
    DEF_CONDITIONAL_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(                                          \
            name, reporter, graphite_context, /*anonymous test_ctx*/, true, ctsEnforcement)

#define DEF_GRAPHITE_TEST_FOR_VULKAN_CONTEXT(name, reporter, graphite_context, ctsEnforcement) \
    DEF_GRAPHITE_TEST_FOR_CONTEXTS(name, skiatest::IsVulkanContextType, reporter,              \
                                   graphite_context, /*anonymous test_ctx*/, ctsEnforcement)

#define DEF_GRAPHITE_TEST_FOR_METAL_CONTEXT(name, reporter, graphite_context)                      \
    DEF_GRAPHITE_TEST_FOR_CONTEXTS(name, skiatest::IsMetalContextType, reporter, graphite_context, \
                                   /*anonymous test_ctx*/, CtsEnforcement::kNever)

#define DEF_GRAPHITE_TEST_FOR_DAWN_CONTEXT(name, reporter, graphite_context) \
    DEF_GRAPHITE_TEST_FOR_CONTEXTS(name, skiatest::IsDawnContextType, reporter, graphite_context, \
                                   /*anonymous test_ctx*/, CtsEnforcement::kNever)

#define DEF_GANESH_TEST(name, reporter, options, ctsEnforcement)            \
    static void test_##name(skiatest::Reporter*, const GrContextOptions&);  \
    skiatest::TestRegistry name##TestRegistry(                              \
            Test::MakeGanesh(#name, ctsEnforcement, test_##name, nullptr)); \
    void test_##name(skiatest::Reporter* reporter, const GrContextOptions& options)

#define DEF_CONDITIONAL_GANESH_TEST_FOR_CONTEXTS(                                                \
        name, context_filter, reporter, context_info, options_filter, condition, ctsEnforcement) \
    static void test_##name(skiatest::Reporter*, const sk_gpu_test::ContextInfo&);               \
    static void test_gpu_contexts_##name(skiatest::Reporter* reporter,                           \
                                         const GrContextOptions& options) {                      \
        skiatest::RunWithGaneshTestContexts(test_##name, context_filter, reporter, options);     \
    }                                                                                            \
    skiatest::TestRegistry name##TestRegistry(                                                   \
            Test::MakeGanesh(#name, ctsEnforcement, test_gpu_contexts_##name, options_filter),   \
            condition);                                                                          \
    void test_##name(skiatest::Reporter* reporter, const sk_gpu_test::ContextInfo& context_info)

#define DEF_CONDITIONAL_GANESH_TEST_FOR_ALL_CONTEXTS(            \
        name, reporter, context_info, condition, ctsEnforcement) \
    DEF_CONDITIONAL_GANESH_TEST_FOR_CONTEXTS(                    \
            name, nullptr, reporter, context_info, nullptr, condition, ctsEnforcement)

#define DEF_CONDITIONAL_GANESH_TEST_FOR_RENDERING_CONTEXTS(             \
        name, reporter, context_info, condition, ctsEnforcement)        \
    DEF_CONDITIONAL_GANESH_TEST_FOR_CONTEXTS(name,                      \
                                             skgpu::IsRenderingContext, \
                                             reporter,                  \
                                             context_info,              \
                                             nullptr,                   \
                                             condition,                 \
                                             ctsEnforcement)

#define DEF_GANESH_TEST_FOR_CONTEXTS(                                                 \
        name, context_filter, reporter, context_info, options_filter, ctsEnforcement) \
    DEF_CONDITIONAL_GANESH_TEST_FOR_CONTEXTS(                                         \
            name, context_filter, reporter, context_info, options_filter, true, ctsEnforcement)

#define DEF_GANESH_TEST_FOR_ALL_CONTEXTS(name, reporter, context_info, ctsEnforcement) \
    DEF_GANESH_TEST_FOR_CONTEXTS(name, nullptr, reporter, context_info, nullptr, ctsEnforcement)

#define DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(name, reporter, context_info, ctsEnforcement) \
    DEF_GANESH_TEST_FOR_CONTEXTS(                                                            \
            name, skgpu::IsRenderingContext, reporter, context_info, nullptr, ctsEnforcement)

#define DEF_GANESH_TEST_FOR_ALL_GL_CONTEXTS(name, reporter, context_info, ctsEnforcement) \
    DEF_GANESH_TEST_FOR_CONTEXTS(                                                         \
            name, skiatest::IsGLContextType, reporter, context_info, nullptr, ctsEnforcement)

#define DEF_GANESH_TEST_FOR_GL_CONTEXT(name, reporter, context_info, ctsEnforcement) \
    DEF_GANESH_TEST_FOR_CONTEXTS(name,                                               \
                                 &skiatest::IsGLContextType,                         \
                                 reporter,                                           \
                                 context_info,                                       \
                                 nullptr,                                            \
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
