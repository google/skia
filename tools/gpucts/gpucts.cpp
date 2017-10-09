/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm_runner.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wused-but-marked-unused"
#endif

#include "gtest/gtest.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

////////////////////////////////////////////////////////////////////////////////

struct GMTestCase {
    gm_runner::GMFactory fGMFactory;
    gm_runner::SkiaBackend fBackend;
    sk_gpu_test::GrContextFactory* fGrContextFactory;
};

struct GMTest : public testing::Test {
    GMTestCase fTest;
    GMTest(GMTestCase t) : fTest(t) {}
    void TestBody() override {
        std::vector<uint32_t> pixels;
        GMK_ImageData imgData = gm_runner::Evaluate(
                fTest.fBackend, fTest.fGMFactory, fTest.fGrContextFactory, &pixels);
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

#if 0
#include "Test.h"

struct UnitTestData {
    sk_gpu_test::GrContextFactory* fContextFactory;
    skiatest::TestProc fProc;
};

struct UnitTest : public testing::Test {
    UnitTestData fUnitTestData;
    UnitTest(UnitTestData d) : fUnitTestData(d) {}
    void TestBody() override {
        struct : skiatest::Reporter {
            void reportFailed(const skiatest::Failure& failure) override {
                SkString desc = failure.toString();
                GTEST_NONFATAL_FAILURE_(desc.c_str());
            }
        } r;
        fUnitTestData.fProc(&r, fUnitTestData.fContextFactory);
    }
};

struct UnitTestFactory : testing::internal::TestFactoryBase {
    UnitTestData fUnitTestData;
    UnitTestFactory(UnitTestData d) : fUnitTestData(d) {}
    testing::Test* CreateTest() override { return new UnitTest(fUnitTestData); }
};

#endif

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    gm_runner::SkiaContext context;
    sk_gpu_test::GrContextFactory* grContextFactory = context.fGrContextFactory.get();

    #if 0
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
    for (auto test : tests) {
        testing::internal::MakeAndRegisterTestInfo(
                "Skia_Unit_Tests",
                test->name,
                nullptr,
                nullptr,
                testing::internal::CodeLocation(__FILE__, __LINE__),
                testing::internal::GetTestTypeId(),
                testing::Test::SetUpTestCase,
                testing::Test::TearDownTestCase,
                new UnitTestFactory(UnitTestData{grContextFactory, test->proc}));
    }
    #endif

    // Rendering Tests
    gm_runner::SkiaBackend backends[] = {
        gm_runner::SkiaBackend::kGL,
        gm_runner::SkiaBackend::kGLES,
        gm_runner::SkiaBackend::kVulkan,
    };
    std::vector<gm_runner::GMFactory> gms = gm_runner::GetGMFactories();
    for (auto backend : backends) {
        std::string test = std::string("SkiaGM_") + GetName(backend);
        for (auto gmFactory : gms) {
            std::string gmName = gm_runner::GetGMName(gmFactory);
            if (!GMK_IsGoodGM(gmName.c_str())) {
                continue;
            }
            testing::internal::MakeAndRegisterTestInfo(
                        test.c_str(),
                        gmName.c_str(),
                        nullptr,
                        nullptr,
                        testing::internal::CodeLocation(__FILE__, __LINE__),
                        testing::internal::GetTestTypeId(),
                        testing::Test::SetUpTestCase,
                        testing::Test::TearDownTestCase,
                        new GMTestFactory(GMTestCase{gmFactory, backend, grContextFactory}));
        }
    }
    return RUN_ALL_TESTS();
}

