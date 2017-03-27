/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ok.h"
#include "Test.h"

// We're going to be a little cute in this file, registering TestStream and
// NullDst so that when you run "ok unit tests", it runs unit tests.

struct TestStream : Stream {
    const skiatest::TestRegistry* registry = skiatest::TestRegistry::Head();

    static std::unique_ptr<Stream> Create(Options) {
        TestStream stream;
        return move_unique(stream);
    }

    struct TestSrc : Src {
        skiatest::Test test {"", false, nullptr};

        std::string name() override { return test.name; }
        SkISize     size() override { return {0,0}; }

        bool draw(SkCanvas*) override {
            // TODO(mtklein): reporter, GrContext, return false on failure.
            test.proc(nullptr, nullptr);
            return true;
        }
    };

    std::unique_ptr<Src> next() override {
        if (!registry) {
            return nullptr;
        }
        TestSrc src;
        src.test = registry->factory();
        registry = registry->next();
        return move_unique(src);
    }
};

struct NullDst : Dst {
    static std::unique_ptr<Dst> Create(Options) {
        NullDst dst;
        return move_unique(dst);
    }

    bool draw(Src* src) override { return src->draw(nullptr); }
    sk_sp<SkImage> image() override { return nullptr; }
};

static Register  unit{"unit",  TestStream::Create};
static Register tests{"tests", NullDst::Create};

// Hey, now why were these defined in DM.cpp?  That's kind of weird.
namespace skiatest {
    bool IsGLContextType(sk_gpu_test::GrContextFactory::ContextType type) {
        return kOpenGL_GrBackend == sk_gpu_test::GrContextFactory::ContextTypeBackend(type);
    }
    bool IsRenderingGLContextType(sk_gpu_test::GrContextFactory::ContextType type) {
        return IsGLContextType(type) && sk_gpu_test::GrContextFactory::IsRenderingContext(type);
    }
    bool IsNullGLContextType(sk_gpu_test::GrContextFactory::ContextType type) {
        return type == sk_gpu_test::GrContextFactory::kNullGL_ContextType;
    }
    void RunWithGPUTestContexts(GrContextTestFn* test, GrContextTypeFilterFn* contextTypeFilter,
                                Reporter* reporter, sk_gpu_test::GrContextFactory* factory) {
        // TODO(bsalomon)
    }
}
