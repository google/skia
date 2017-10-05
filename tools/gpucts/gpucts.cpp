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

struct GMTestCase {
    gm_runner::GMFactory fGMFactory;
    gm_runner::SkiaBackend* fSurfaceFactory;
};

struct GMTest : public ::testing::Test {
    GMTestCase fTest;
    GMTest(GMTestCase t) : fTest(t) {}
    void TestBody() override {
        std::vector<uint32_t> pixels;
        GMK_ImageData imgData =
                gm_runner::Evaluate(fTest.fSurfaceFactory, fTest.fGMFactory, &pixels);
        EXPECT_TRUE(imgData.pix);
        if (!imgData.pix) {
            return;
        }
        std::string gmName = gm_runner::GetName(fTest.fGMFactory);
        float result = GMK_Check(imgData, gmName.c_str());
        EXPECT_EQ(result, 0);
    }
};

struct GMTestFactory : public ::testing::internal::TestFactoryBase {
    GMTestCase fTest;
    GMTestFactory(GMTestCase t) : fTest(t) {}
    ::testing::Test* CreateTest() override { return new GMTest(fTest); }
};

void register_test(gm_runner::SkiaBackend* surfaceFactory, gm_runner::GMFactory gmFactory) {
    std::string gmName = gm_runner::GetName(gmFactory);
    std::string test = std::string("SkiaGM_") + GetName(surfaceFactory);
    ::testing::internal::MakeAndRegisterTestInfo(
                test.c_str(),
                gmName.c_str(),
                nullptr,
                nullptr,
                ::testing::internal::CodeLocation(__FILE__, __LINE__),
                ::testing::internal::GetTestTypeId(),
                ::testing::Test::SetUpTestCase,
                ::testing::Test::TearDownTestCase,
                new GMTestFactory(GMTestCase{gmFactory, surfaceFactory}));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    gm_runner::GMRunner runner;
    runner.ConsumeGMs(&register_test);
    return RUN_ALL_TESTS();
}

