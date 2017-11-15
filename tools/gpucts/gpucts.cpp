/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGraphics.h"
#include "gm_runner.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wused-but-marked-unused"
#endif

#include "gtest/gtest.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include "Test.h"

////////////////////////////////////////////////////////////////////////////////

struct GMTestCase {
    gm_runner::GMFactory fGMFactory;
    gm_runner::SkiaBackend fBackend;
};

struct GMTest : public testing::Test {
    GMTestCase fTest;
    GMTest(GMTestCase t) : fTest(t) {}
    void TestBody() override {
        if (!fTest.fGMFactory) {
            EXPECT_TRUE(gm_runner::BackendSupported(fTest.fBackend));
            return;
        }
        std::vector<uint32_t> pixels;
        GMK_ImageData imgData = gm_runner::Evaluate(fTest.fBackend, fTest.fGMFactory, &pixels);
        EXPECT_TRUE(imgData.pix);
        if (!imgData.pix) {
            return;
        }
        std::string gmName = gm_runner::GetGMName(fTest.fGMFactory);
        float result = GMK_Check(imgData, gmName.c_str());
        EXPECT_EQ(result, 0);
    }
};

struct GMTestFactory : public testing::internal::TestFactoryBase {
    GMTestCase fTest;
    GMTestFactory(GMTestCase t) : fTest(t) {}
    testing::Test* CreateTest() override { return new GMTest(fTest); }
};

////////////////////////////////////////////////////////////////////////////////

#if !SK_SUPPORT_GPU
struct GrContextOptions {};
#endif

struct UnitTest : public testing::Test {
    skiatest::TestProc fProc;
    UnitTest(skiatest::TestProc proc) : fProc(proc) {}
    void TestBody() override {
        struct : skiatest::Reporter {
            void reportFailed(const skiatest::Failure& failure) override {
                SkString desc = failure.toString();
                SK_ABORT("");
                GTEST_NONFATAL_FAILURE_(desc.c_str());
            }
        } r;
        fProc(&r, GrContextOptions());
    }
};

struct UnitTestFactory : testing::internal::TestFactoryBase {
    skiatest::TestProc fProc;
    UnitTestFactory(skiatest::TestProc proc) : fProc(proc) {}
    testing::Test* CreateTest() override { return new UnitTest(fProc); }
};

std::vector<const skiatest::Test*> GetUnitTests() {
    // Unit Tests
    std::vector<const skiatest::Test*> tests;
    for (const skiatest::TestRegistry* r = skiatest::TestRegistry::Head(); r; r = r->next()) {
        tests.push_back(&r->factory());
    }
    struct {
        bool operator()(const skiatest::Test* u, const skiatest::Test* v) const {
            return strcmp(u->name, v->name) < 0;
        }
    } less;
    std::sort(tests.begin(), tests.end(), less);
    return tests;
}

////////////////////////////////////////////////////////////////////////////////

static void reg_test(const char* test, const char* testCase,
                     testing::internal::TestFactoryBase* fact) {
    testing::internal::MakeAndRegisterTestInfo(
                        test,
                        testCase,
                        nullptr,
                        nullptr,
                        testing::internal::CodeLocation(__FILE__, __LINE__),
                        testing::internal::GetTestTypeId(),
                        testing::Test::SetUpTestCase,
                        testing::Test::TearDownTestCase,
                        fact);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    SkGraphics::Init();

    // Rendering Tests
    gm_runner::SkiaBackend backends[] = {
        #ifndef SK_BUILD_FOR_ANDROID
        gm_runner::SkiaBackend::kGL,  // Used for testing on desktop machines.
        #endif
        gm_runner::SkiaBackend::kGLES,
        gm_runner::SkiaBackend::kVulkan,
    };
    std::vector<gm_runner::GMFactory> gms = gm_runner::GetGMFactories();
    for (auto backend : backends) {
        const char* backendName = GetBackendName(backend);
        std::string test = std::string("SkiaGM_") + backendName;
        reg_test(test.c_str(), "BackendSupported", new GMTestFactory(GMTestCase{nullptr, backend}));

        if (!gm_runner::BackendSupported(backend)) {
            continue;
        }
        for (auto gmFactory : gms) {
            std::string gmName = gm_runner::GetGMName(gmFactory);
            if (!GMK_IsGoodGM(gmName.c_str())) {
                continue;
            }
            #ifdef SK_DEBUG
                // The following test asserts on my phone.
                // TODO(halcanary):  fix this.
                if(gmName == std::string("complexclip3_simple") &&
                   backend == gm_runner::SkiaBackend::kGLES) {
                    continue;
                }
            #endif
                reg_test(test.c_str(), gmName.c_str(),
                         new GMTestFactory(GMTestCase{gmFactory, backend}));
      }
    }

    for (const skiatest::Test* test : GetUnitTests()) {
        reg_test("Skia_Unit_Tests", test->name, new UnitTestFactory(test->proc));
    }
    return RUN_ALL_TESTS();
}

