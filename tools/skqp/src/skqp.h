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
    virtual std::unique_ptr<SkStreamAsset> open(const char* path) = 0;
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

    static std::string GetGMName(GMFactory);
    static const char* GetUnitTestName(UnitTest);
    static const char* GetBackendName(SkiaBackend);

    SkQP();
    ~SkQP();
    void init(std::unique_ptr<SkQPAssetManager>,
              const char* reportDirectory,
              bool experimentalMode);
    std::tuple<int, int, std::string> evaluateGM(SkiaBackend, GMFactory);
    const std::vector<std::string>& executeTest(UnitTest);
    void makeReport();

    const std::vector<SkiaBackend>& getSupportedBackends() { return fSupportedBackends; }
    const std::vector<GMFactory>& getGMs() { return fGMs; }
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
    std::unordered_set<std::string> fKnownGpuUnitTests;
    std::unordered_set<std::string> fKnownGMs;
    bool fExperimentalMode = false;
};
#endif  // skqp_DEFINED

