/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skqp_DEFINED
#define skqp_DEFINED

#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_set>
#include <unordered_map>
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
    SkQPAssetManager() {}
    virtual ~SkQPAssetManager() {}
    virtual sk_sp<SkData> open(const char* path) = 0;
private:
    SkQPAssetManager(const SkQPAssetManager&) = delete;
    SkQPAssetManager& operator=(const SkQPAssetManager&) = delete;
};

class SkQP {
public:
    enum class SkiaBackend {
        kGL,
        kGLES,
        kVulkan,
    };
    using GMFactory = std::unique_ptr<skiagm::GM> (*)();
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

        @param assetManager - provides assets for the models.  Does not take ownership.
        @param renderTests - file containing list of render tests.
        @param reportDirectory - where to write out report.
    */
    void init(SkQPAssetManager* assetManager, const char* renderTests, const char* reportDirectory);

    struct RenderOutcome {
        // All three values will be 0 if the test passes.
        int fMaxError = 0;        // maximum error of all pixel.
        int fBadPixelCount = 0;   // number of pixels with non-zero error.
        int64_t fTotalError = 0;  // sum of error for all bad pixels.
    };

    /**
        @return render outcome and error string.  Only errors running or
                evaluating the GM will result in a non-empty error string.
    */
    std::tuple<RenderOutcome, std::string> evaluateGM(SkiaBackend, GMFactory);

    /** @return a (hopefully empty) list of errors produced by this unit test.  */
    std::vector<std::string> executeTest(UnitTest);

    /** Call this after running all checks to write a report into the given
        report directory. */
    void makeReport();

    /** @return a list of backends that this version of SkQP supports.  */
    const std::vector<SkiaBackend>& getSupportedBackends() const { return fSupportedBackends; }
    /** @return a list of all Skia GMs in lexicographic order.  */
    const std::vector<GMFactory>& getGMs() const { return fGMs; }
    /** @return a list of all Skia GPU unit tests in lexicographic order.  */
    const std::vector<UnitTest>& getUnitTests() const { return fUnitTests; }
    ////////////////////////////////////////////////////////////////////////////

private:
    struct RenderResult {
        SkiaBackend fBackend;
        GMFactory fGM;
        RenderOutcome fOutcome;
   };
    struct UnitTestResult {
        UnitTest fUnitTest;
        std::vector<std::string> fErrors;
    };
    std::vector<RenderResult> fRenderResults;
    std::vector<UnitTestResult> fUnitTestResults;
    std::vector<SkiaBackend> fSupportedBackends;
    SkQPAssetManager* fAssetManager = nullptr;
    std::string fReportDirectory;
    std::vector<UnitTest> fUnitTests;
    std::vector<GMFactory> fGMs;
    std::unordered_map<std::string, int64_t> fGMThresholds;

    SkQP(const SkQP&) = delete;
    SkQP& operator=(const SkQP&) = delete;
};
#endif  // skqp_DEFINED

