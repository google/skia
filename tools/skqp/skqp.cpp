/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm_knowledge.h"
#include "gm_runner.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wused-but-marked-unused"
#endif

#include "gtest/gtest.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include "SkStream.h"
#include "SkString.h"

////////////////////////////////////////////////////////////////////////////////

static std::string gReportDirectoryPath;
static std::unique_ptr<skqp::AssetManager> gAssetMgr;

////////////////////////////////////////////////////////////////////////////////

struct GMTestCase {
    gm_runner::GMFactory fGMFactory;
    gm_runner::SkiaBackend fBackend;
};

struct GMTest : public testing::Test {
    GMTestCase fTest;
    GMTest(GMTestCase t) : fTest(t) {}
    void TestBody() override {
        float result;
        gm_runner::Error error;
        std::tie(result, error) =
                gm_runner::EvaluateGM(fTest.fBackend, fTest.fGMFactory,
                                      gAssetMgr.get(), gReportDirectoryPath.c_str());
        EXPECT_EQ(error, gm_runner::Error::None);
        if (gm_runner::Error::None == error) {
            EXPECT_EQ(result, 0);
        }
    }
};

struct GMTestFactory : public testing::internal::TestFactoryBase {
    GMTestCase fTest;
    GMTestFactory(GMTestCase t) : fTest(t) {}
    testing::Test* CreateTest() override { return new GMTest(fTest); }
};

////////////////////////////////////////////////////////////////////////////////

struct UnitTestTest : public testing::Test {
    gm_runner::UnitTest fTest;
    UnitTestTest(gm_runner::UnitTest test) : fTest(test) {}
    void TestBody() override {
        std::vector<std::string> errors = gm_runner::ExecuteTest(fTest);
        for (const std::string& error : errors) {
            GTEST_NONFATAL_FAILURE_(error.c_str());
        }
    }
};

struct UnitTestFactory : public testing::internal::TestFactoryBase {
    gm_runner::UnitTest fTest;
    UnitTestFactory(gm_runner::UnitTest test) : fTest(test) {}
    testing::Test* CreateTest() override { return new UnitTestTest(fTest); }
};

////////////////////////////////////////////////////////////////////////////////

static void reg_test(const char* test, const char* testCase,
                     testing::internal::TestFactoryBase* fact) {
    testing::internal::MakeAndRegisterTestInfo(test,
                                               testCase,
                                               nullptr,
                                               nullptr,
                                               testing::internal::CodeLocation(__FILE__, __LINE__),
                                               testing::internal::GetTestTypeId(),
                                               testing::Test::SetUpTestCase,
                                               testing::Test::TearDownTestCase,
                                               fact);
}


void register_skia_tests() {
    gm_runner::InitSkia();

    // Rendering Tests
    std::vector<gm_runner::SkiaBackend> backends = gm_runner::GetSupportedBackends();
    std::vector<gm_runner::GMFactory> gms = gm_runner::GetGMFactories(gAssetMgr.get());
    for (auto backend : backends) {
        const char* backendName = GetBackendName(backend);
        std::string test = std::string("SkiaGM_") + backendName;
        for (auto gmFactory : gms) {
            std::string gmName = gm_runner::GetGMName(gmFactory);
            reg_test(test.c_str(), gmName.c_str(),
                     new GMTestFactory(GMTestCase{gmFactory, backend}));
        }
    }

    for (gm_runner::UnitTest test : gm_runner::GetUnitTests()) {
        reg_test("Skia_Unit_Tests", gm_runner::GetUnitTestName(test), new UnitTestFactory{test});
    }
}

namespace {
struct StdAssetManager : public skqp::AssetManager {
    SkString fPrefix;
    StdAssetManager(const char* p) : fPrefix(p) {}
    std::unique_ptr<SkStreamAsset> open(const char* path) override {
        SkString fullPath = fPrefix.isEmpty()
                          ? SkString(path)
                          : SkStringPrintf("%s/%s", fPrefix.c_str(), path);
        return SkStream::MakeFromFile(fullPath.c_str());
    }
};
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    if (argc < 2) {
        std::cerr << "Usage:\n  " << argv[0]
                  << " [GTEST_ARGUMENTS] GMKB_DIRECTORY_PATH GMKB_REPORT_PATH\n\n";
        return 1;
    }
    gAssetMgr.reset(new StdAssetManager(argv[1]));
    if (argc > 2) {
        gReportDirectoryPath = argv[2];
    }
    register_skia_tests();
    int ret = RUN_ALL_TESTS();
    (void)gmkb::MakeReport(gReportDirectoryPath.c_str());
    return ret;
}
