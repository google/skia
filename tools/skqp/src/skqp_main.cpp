/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <sys/stat.h>

#include "skqp.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wused-but-marked-unused"
#endif

#include "gtest/gtest.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include "Resources.h"
#include "SkStream.h"
#include "SkString.h"

////////////////////////////////////////////////////////////////////////////////

struct GMTestCase {
    SkQP* fSkQP;
    SkQP::GMFactory fGMFactory;
    SkQP::SkiaBackend fBackend;
};

struct GMTest : public testing::Test {
    GMTestCase fTest;
    GMTest(GMTestCase t) : fTest(t) {}
    void TestBody() override {
        int maxError;
        int errorCount;
        std::string except;
        std::tie(maxError, errorCount, except) =
            fTest.fSkQP->evaluateGM(fTest.fBackend, fTest.fGMFactory);
        EXPECT_TRUE(except.empty());
        if (except.empty()) {
            EXPECT_EQ(maxError, 0);
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
    SkQP* fSkQP;
    SkQP::UnitTest fTest;
    UnitTestTest(SkQP* ptr, SkQP::UnitTest test) : fSkQP(ptr), fTest(test) {}
    void TestBody() override {
        std::vector<std::string> errors = fSkQP->executeTest(fTest);
        for (const std::string& error : errors) {
            GTEST_NONFATAL_FAILURE_(error.c_str());
        }
    }
};

struct UnitTestFactory : public testing::internal::TestFactoryBase {
    SkQP* fSkQP;
    SkQP::UnitTest fTest;
    UnitTestFactory(SkQP* ptr, SkQP::UnitTest test) : fSkQP(ptr), fTest(test) {}
    testing::Test* CreateTest() override { return new UnitTestTest(fSkQP, fTest); }
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

static void register_skia_tests(SkQP* skqp) {
    // Rendering Tests
    for (auto backend : skqp->getSupportedBackends()) {
        std::string test = std::string("skqp_") + SkQP::GetBackendName(backend);
        for (auto gmFactory : skqp->getGMs()) {
            reg_test(test.c_str(), SkQP::GetGMName(gmFactory).c_str(),
                     new GMTestFactory(GMTestCase{skqp, gmFactory, backend}));
        }
    }

    for (auto test : skqp->getUnitTests()) {
        reg_test("skqp_unitTest", SkQP::GetUnitTestName(test), new UnitTestFactory{skqp, test});
    }
}

namespace {
struct StdAssetManager : public SkQPAssetManager {
    std::string fPrefix;
    StdAssetManager(const char* p) : fPrefix(p) {
        SkASSERT(!fPrefix.empty());
        fPrefix += "/";
    }
    std::unique_ptr<SkStreamAsset> open(const char* path) override {
        return SkStream::MakeFromFile((fPrefix + path).c_str());
    }
};
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    if (argc < 3) {
        std::cerr << "Usage:\n  " << argv[0]
                  << " [GTEST_ARGUMENTS] ASSET_DIRECTORY_PATH SKQP_REPORT_PATH\n\n";
        return 1;
    }
    SetResourcePath((std::string(argv[1]) + "/resources").c_str());
    (void)mkdir(argv[2], 0777);
    std::unique_ptr<SkQPAssetManager> mgr(new StdAssetManager(argv[1]));
    SkQP skqp;
    skqp.init(std::move(mgr), argv[2], false);
    register_skia_tests(&skqp);
    int ret = RUN_ALL_TESTS();
    skqp.makeReport();
    return ret;
}
