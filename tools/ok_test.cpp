/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ok.h"
#include "Test.h"

#if SK_SUPPORT_GPU
    #include "GrContextFactory.h"
#endif

struct TestStream : Stream {
    const skiatest::TestRegistry* registry = skiatest::TestRegistry::Head();
    bool extended = false, verbose = false;

    static std::unique_ptr<Stream> Create(Options options) {
        TestStream stream;
        if (options("extended") != "") { stream.extended = true; }
        if (options("verbose" ) != "") { stream.verbose  = true; }

        return move_unique(stream);
    }

    struct TestSrc : Src {
        skiatest::Test test {"", false, nullptr};
        bool extended, verbose;

        std::string name() override { return test.name; }
        SkISize     size() override { return {0,0}; }

        Status draw(SkCanvas*) override {

            struct : public skiatest::Reporter {
                Status status = Status::OK;
                bool extended, verbose_;

                void reportFailed(const skiatest::Failure& failure) override {
                    ok_log(failure.toString().c_str());
                    status = Status::Failed;
                }
                bool allowExtendedTest() const override { return extended; }
                bool           verbose() const override { return verbose_; }
            } reporter;
            reporter.extended = extended;
            reporter.verbose_ = verbose;

            sk_gpu_test::GrContextFactory* factory = nullptr;
        #if SK_SUPPORT_GPU
            GrContextOptions options;
            sk_gpu_test::GrContextFactory a_real_factory(options);
            factory = &a_real_factory;
        #endif

            test.proc(&reporter, factory);
            return reporter.status;
        }
    };

    std::unique_ptr<Src> next() override {
        if (!registry) {
            return nullptr;
        }
        TestSrc src;
        src.test = registry->factory();
        src.extended = extended;
        src.verbose  = verbose;
        registry = registry->next();
        return move_unique(src);
    }
};
static Register test{"test", "run unit tests linked into this binary", TestStream::Create};

// Hey, now why were these defined in DM.cpp?  That's kind of weird.
namespace skiatest {
#if SK_SUPPORT_GPU
    bool IsGLContextType(sk_gpu_test::GrContextFactory::ContextType type) {
        return kOpenGL_GrBackend == sk_gpu_test::GrContextFactory::ContextTypeBackend(type);
    }
    bool IsVulkanContextType(sk_gpu_test::GrContextFactory::ContextType type) {
        return kVulkan_GrBackend == sk_gpu_test::GrContextFactory::ContextTypeBackend(type);
    }
    bool IsRenderingGLContextType(sk_gpu_test::GrContextFactory::ContextType type) {
        return IsGLContextType(type) && sk_gpu_test::GrContextFactory::IsRenderingContext(type);
    }
    bool IsNullGLContextType(sk_gpu_test::GrContextFactory::ContextType type) {
        return type == sk_gpu_test::GrContextFactory::kNullGL_ContextType;
    }
#else
    bool IsGLContextType         (int) { return false; }
    bool IsVulkanContextType     (int) { return false; }
    bool IsRenderingGLContextType(int) { return false; }
    bool IsNullGLContextType     (int) { return false; }
#endif

    void RunWithGPUTestContexts(GrContextTestFn* test, GrContextTypeFilterFn* contextTypeFilter,
                                Reporter* reporter, sk_gpu_test::GrContextFactory* factory) {
        // TODO(bsalomon)
    }
}
