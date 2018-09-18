/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef gm_runner_DEFINED
#define gm_runner_DEFINED

#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "skqp_asset_manager.h"

/**
A Skia GM is a single rendering test that can be executed on any Skia backend Canvas.
*/
namespace skiagm {
class GM;
}

namespace skiatest {
struct Test;
}

namespace gm_runner {

using GMFactory = skiagm::GM* (*)(void*);

using UnitTest = const skiatest::Test*;

enum class SkiaBackend {
    kGL,
    kGLES,
    kVulkan,
};

enum class Mode {
    /** This mode is set when used by Android CTS.  All known tests are executed.  */
    kCompatibilityTestMode,
    /** This mode is set when used in the test lab.  Some tests are skipped, if
        they are known to cause crashes in older devices.  All GMs are evaluated
        with stricter requirements. */
    kExperimentalMode,

};

/**
Initialize Skia
*/
void InitSkia(Mode, skqp::AssetManager*);

std::vector<SkiaBackend> GetSupportedBackends();

/**
@return a list of all Skia GMs in lexicographic order.
*/
std::vector<GMFactory> GetGMFactories(skqp::AssetManager*);

/**
@return a list of all Skia GPU unit tests in lexicographic order.
*/
std::vector<UnitTest> GetUnitTests();

/**
@return a descriptive name for the GM.
*/
std::string GetGMName(GMFactory);

/**
@return a descriptive name for the unit test.
*/
const char* GetUnitTestName(UnitTest);

/**
@return a descriptive name for the backend.
*/
const char* GetBackendName(SkiaBackend);

enum class Error {
    None = 0,
    BadSkiaOutput = 1,
    BadGMKBData = 2,
    SkiaFailure = 3,
};

const char* GetErrorString(Error);

/**
@return A non-negative float representing how badly the GM failed (or zero for
        success).  Any error running or evaluating the GM will result in a non-zero
        error code.
*/
std::tuple<float, Error> EvaluateGM(SkiaBackend backend,
                                    GMFactory gmFact,
                                    skqp::AssetManager* assetManager,
                                    const char* reportDirectoryPath);

/**
@return a (hopefully empty) list of errors produced by this unit test.
*/
std::vector<std::string> ExecuteTest(UnitTest);

}  // namespace gm_runner

#endif  // gm_runner_DEFINED
