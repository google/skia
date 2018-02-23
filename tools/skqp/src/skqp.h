/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skqp_DEFINED
#define skqp_DEFINED

#include <memory>
#include <string>
#include <tuple>
#include <unordered_set>
#include <vector>

class SkData;
template <typename T> class sk_sp;

namespace skiagm {
class GM;
}

namespace skiatest {
struct Test;
}

class SkStreamAsset;

////////////////////////////////////////////////////////////////////////////////
class SkQPAssetManager {
public:
    virtual ~SkQPAssetManager() {}
    virtual sk_sp<SkData> open(const char* path) = 0;
};

class SkQP {
public:
    enum class SkiaBackend {
        kGL,
        kGLES,
        kVulkan,
    };
    using GMFactory = skiagm::GM* (*)(void*);
    using UnitTest = const skiatest::Test*;

    ////////////////////////////////////////////////////////////////////////////

    /** These functions provide a descriptive name for the given value.*/
    static std::string GetGMName(GMFactory);
    static const char* GetUnitTestName(UnitTest);
    static const char* GetBackendName(SkiaBackend);

    SkQP();
    ~SkQP();

    /**
        Initialize Skia and the SkQP.  Should be executed only once.

        @param assetManager - provides assets for the models.
        @param reportDirectory - where to write out report.
        @param experimentalMode - if true, use all models, but skip some
               crashing tests.  Never set for CTS runs, only for model
               validation.
    */
    void init(std::unique_ptr<SkQPAssetManager> assetManager,
              const char* reportDirectory,
              bool experimentalMode = false);

    /**
        @return non-negative integers representing how badly the GM failed (or
                zero for success).  Any error running or evaluating the GM will
                result in a non-empty error string.
    */
    std::tuple<int, int, std::string> evaluateGM(SkiaBackend, GMFactory);

    /** @return a (hopefully empty) list of errors produced by this unit test.  */
    std::vector<std::string> executeTest(UnitTest);

    /** Call this after running all checks.  */
    void makeReport();

    /** @return a list of backends that this version of SkQP supports.  */
    const std::vector<SkiaBackend>& getSupportedBackends() { return fSupportedBackends; }
    /** @return a list of all Skia GMs in lexicographic order.  */
    const std::vector<GMFactory>& getGMs() { return fGMs; }
    /** @return a list of all Skia GPU unit tests in lexicographic order.  */
    const std::vector<UnitTest>& getUnitTests() { return fUnitTests; }
    ////////////////////////////////////////////////////////////////////////////

private:
    struct RenderResult {
        SkiaBackend fBackend;
        GMFactory fGM;
        int fMaxerror;
        int fBadpixels;
    };
    struct UnitTestResult {
        UnitTest fUnitTest;
        std::vector<std::string> fErrors;
    };
    std::vector<RenderResult> fRenderResults;
    std::vector<UnitTestResult> fUnitTestResults;
    std::vector<SkiaBackend> fSupportedBackends;
    std::vector<GMFactory> fGMs;
    std::vector<UnitTest> fUnitTests;
    std::unique_ptr<SkQPAssetManager> fAssetManager;
    std::string fReportDirectory;
    std::unordered_set<std::string> fDoNotScoreInCompatibilityTestMode;
    std::unordered_set<std::string> fDoNotExecuteInExperimentalMode;
};
#endif  // skqp_DEFINED

