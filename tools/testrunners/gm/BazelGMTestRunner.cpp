/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * This program runs all GMs registered via macros such as DEF_GM, and for each GM, it saves the
 * resulting SkBitmap as a .png file to disk, along with a .json file with the hash of the pixels.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/encode/SkPngEncoder.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "src/core/SkMD5.h"
#include "src/utils/SkJSONWriter.h"
#include "src/utils/SkOSPath.h"
#include "tools/HashAndEncode.h"
#include "tools/testrunners/common/TestRunner.h"
#include "tools/testrunners/common/compilation_mode_keys/CompilationModeKeys.h"
#include "tools/testrunners/common/surface_manager/SurfaceManager.h"
#include "tools/testrunners/gm/vias/Draw.h"

#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <set>
#include <sstream>
#include <string>

// TODO(lovisolo): Add flag --omitDigestIfHashInFile (provides the known hashes file).

static DEFINE_string(skip, "", "Space-separated list of test cases (regexps) to skip.");
static DEFINE_string(
        match,
        "",
        "Space-separated list of test cases (regexps) to run. Will run all tests if omitted.");

// When running under Bazel and overriding the output directory, you might encounter errors such
// as "No such file or directory" and "Read-only file system". The former can happen when running
// on RBE because the passed in output dir might not exist on the remote worker, whereas the latter
// can happen when running locally in sandboxed mode, which is the default strategy when running
// outside of RBE. One possible workaround is to run the test as a local subprocess, which can be
// done by passing flag --strategy=TestRunner=local to Bazel.
//
// Reference: https://bazel.build/docs/user-manual#execution-strategy.
static DEFINE_string(outputDir,
                     "",
                     "Directory where to write any output .png and .json files. "
                     "Optional when running under Bazel "
                     "(e.g. \"bazel test //path/to:test\") as it defaults to "
                     "$TEST_UNDECLARED_OUTPUTS_DIR.");

static DEFINE_string(knownDigestsFile,
                     "",
                     "Plaintext file with one MD5 hash per line. This test runner will omit from "
                     "the output directory any images with an MD5 hash in this file.");

static DEFINE_string(key, "", "Space-separated key/value pairs common to all traces.");

// We named this flag --surfaceConfig rather than --config to avoid confusion with the --config
// Bazel flag.
static DEFINE_string(
        surfaceConfig,
        "",
        "Name of the Surface configuration to use (e.g. \"8888\"). This determines "
        "how we construct the SkSurface from which we get the SkCanvas that GMs will "
        "draw on. See file //tools/testrunners/common/surface_manager/SurfaceManager.h for "
        "details.");

static DEFINE_string(
        cpuName,
        "",
        "Contents of the \"cpu_or_gpu_value\" dimension for CPU-bound traces (e.g. \"AVX512\").");

static DEFINE_string(
        gpuName,
        "",
        "Contents of the \"cpu_or_gpu_value\" dimension for GPU-bound traces (e.g. \"RTX3060\").");

static DEFINE_string(via,
                     "direct",  // Equivalent to running DM without a via.
                     "Name of the \"via\" to use (e.g. \"picture_serialization\"). Optional.");

// Set in //bazel/devicesrc but only consumed by adb_test_runner.go. We cannot use the
// DEFINE_string macro because the flag name includes dashes.
[[maybe_unused]] static bool unused =
        SkFlagInfo::CreateStringFlag("device-specific-bazel-config",
                                     nullptr,
                                     new CommandLineFlags::StringArray(),
                                     nullptr,
                                     "Ignored by this test runner.",
                                     nullptr);

// Return type for function write_png_and_json_files().
struct WritePNGAndJSONFilesResult {
    enum { kSuccess, kSkippedKnownDigest, kError } status;
    std::string errorMsg = "";
    std::string skippedDigest = "";
};

// Takes a SkBitmap and writes the resulting PNG and MD5 hash into the given files.
static WritePNGAndJSONFilesResult write_png_and_json_files(
        std::string name,
        std::map<std::string, std::string> commonKeys,
        std::map<std::string, std::string> gmGoldKeys,
        std::map<std::string, std::string> surfaceGoldKeys,
        const SkBitmap& bitmap,
        const char* pngPath,
        const char* jsonPath,
        std::set<std::string> knownDigests) {
    HashAndEncode hashAndEncode(bitmap);

    // Compute MD5 hash.
    SkMD5 hash;
    hashAndEncode.feedHash(&hash);
    SkMD5::Digest digest = hash.finish();
    SkString md5 = digest.toLowercaseHexString();

    // Skip this digest if it's known.
    if (knownDigests.find(md5.c_str()) != knownDigests.end()) {
        return {
                .status = WritePNGAndJSONFilesResult::kSkippedKnownDigest,
                .skippedDigest = md5.c_str(),
        };
    }

    // Write PNG file.
    SkFILEWStream pngFile(pngPath);
    bool result = hashAndEncode.encodePNG(&pngFile,
                                          md5.c_str(),
                                          /* key= */ CommandLineFlags::StringArray(),
                                          /* properties= */ CommandLineFlags::StringArray());
    if (!result) {
        return {
                .status = WritePNGAndJSONFilesResult::kError,
                .errorMsg = "Error encoding or writing PNG to " + std::string(pngPath),
        };
    }

    // Validate GM-related Gold keys.
    if (gmGoldKeys.find("name") == gmGoldKeys.end()) {
        SK_ABORT("gmGoldKeys does not contain key \"name\"");
    }
    if (gmGoldKeys.find("source_type") == gmGoldKeys.end()) {
        SK_ABORT("gmGoldKeys does not contain key \"source_type\"");
    }

    // Validate surface-related Gold keys.
    if (surfaceGoldKeys.find("surface_config") == surfaceGoldKeys.end()) {
        SK_ABORT("surfaceGoldKeys does not contain key \"surface_config\"");
    }

    // Gather all Gold keys.
    std::map<std::string, std::string> keys = {
            {"build_system", "bazel"},
    };
    keys.merge(GetCompilationModeGoldAndPerfKeyValuePairs());
    keys.merge(commonKeys);
    keys.merge(surfaceGoldKeys);
    keys.merge(gmGoldKeys);

    // Write JSON file with MD5 hash and Gold key-value pairs.
    SkFILEWStream jsonFile(jsonPath);
    SkJSONWriter jsonWriter(&jsonFile, SkJSONWriter::Mode::kPretty);
    jsonWriter.beginObject();  // Root object.
    jsonWriter.appendString("md5", md5);
    jsonWriter.beginObject("keys");  // "keys" dictionary.
    for (auto const& [param, value] : keys) {
        jsonWriter.appendString(param.c_str(), SkString(value));
    }
    jsonWriter.endObject();  // "keys" dictionary.
    jsonWriter.endObject();  // Root object.

    return {.status = WritePNGAndJSONFilesResult::kSuccess};
}

static std::string draw_result_to_string(skiagm::DrawResult result) {
    switch (result) {
        case skiagm::DrawResult::kOk:
            return "Ok";
        case skiagm::DrawResult::kFail:
            return "Fail";
        case skiagm::DrawResult::kSkip:
            return "Skip";
        default:
            SkUNREACHABLE;
    }
}

static int gNumSuccessfulGMs = 0;
static int gNumFailedGMs = 0;
static int gNumSkippedGMs = 0;

static bool gMissingCpuOrGpuWarningLogged = false;

// Runs a GM under the given surface config, and saves its output PNG file (and accompanying JSON
// file with metadata) to the given output directory.
void run_gm(std::unique_ptr<skiagm::GM> gm,
            std::string config,
            std::map<std::string, std::string> keyValuePairs,
            std::string cpuName,
            std::string gpuName,
            std::string outputDir,
            std::set<std::string> knownDigests) {
    TestRunner::Log("GM: %s", gm->getName().c_str());

    // Create surface and canvas.
    std::unique_ptr<SurfaceManager> surfaceManager = SurfaceManager::FromConfig(
            config, SurfaceOptions{gm->getISize().width(), gm->getISize().height()});
    if (surfaceManager == nullptr) {
        SK_ABORT("Unknown --surfaceConfig flag value: %s.", config.c_str());
    }

    // Print warning about missing cpu_or_gpu key if necessary.
    if ((surfaceManager->isCpuOrGpuBound() == SurfaceManager::CpuOrGpu::kCPU && cpuName == "" &&
         !gMissingCpuOrGpuWarningLogged)) {
        TestRunner::Log(
                "\tWarning: The surface is CPU-bound, but flag --cpuName was not provided. "
                "Gold traces will omit keys \"cpu_or_gpu\" and \"cpu_or_gpu_value\".");
        gMissingCpuOrGpuWarningLogged = true;
    }
    if ((surfaceManager->isCpuOrGpuBound() == SurfaceManager::CpuOrGpu::kGPU && gpuName == "" &&
         !gMissingCpuOrGpuWarningLogged)) {
        TestRunner::Log(
                "\tWarning: The surface is GPU-bound, but flag --gpuName was not provided. "
                "Gold traces will omit keys \"cpu_or_gpu\" and \"cpu_or_gpu_value\".");
        gMissingCpuOrGpuWarningLogged = true;
    }

    // Set up GPU.
    TestRunner::Log("\tSetting up GPU...");
    SkString msg;
    skiagm::DrawResult result = gm->gpuSetup(surfaceManager->getSurface()->getCanvas(), &msg);

    // Draw GM into canvas if GPU setup was successful.
    SkBitmap bitmap;
    if (result == skiagm::DrawResult::kOk) {
        GMOutput output;
        std::string viaName = FLAGS_via.size() == 0 ? "" : (FLAGS_via[0]);
        TestRunner::Log("\tDrawing GM via \"%s\"...", viaName.c_str());
        output = draw(gm.get(), surfaceManager->getSurface().get(), viaName);
        result = output.result;
        msg = SkString(output.msg.c_str());
        bitmap = output.bitmap;
    }

    // Keep track of results. We will exit with a non-zero exit code in the case of failures.
    switch (result) {
        case skiagm::DrawResult::kOk:
            // We don't increment numSuccessfulGMs just yet. We still need to successfully save
            // its output bitmap to disk.
            TestRunner::Log("\tFlushing surface...");
            surfaceManager->flush();
            break;
        case skiagm::DrawResult::kFail:
            gNumFailedGMs++;
            break;
        case skiagm::DrawResult::kSkip:
            gNumSkippedGMs++;
            break;
        default:
            SkUNREACHABLE;
    }

    // Report GM result and optional message.
    TestRunner::Log("\tResult: %s", draw_result_to_string(result).c_str());
    if (!msg.isEmpty()) {
        TestRunner::Log("\tMessage: \"%s\"", msg.c_str());
    }

    // Save PNG and JSON file with MD5 hash to disk if the GM was successful.
    if (result == skiagm::DrawResult::kOk) {
        std::string name = std::string(gm->getName().c_str());
        SkString pngPath = SkOSPath::Join(outputDir.c_str(), (name + ".png").c_str());
        SkString jsonPath = SkOSPath::Join(outputDir.c_str(), (name + ".json").c_str());

        WritePNGAndJSONFilesResult pngAndJSONResult =
                write_png_and_json_files(gm->getName().c_str(),
                                         keyValuePairs,
                                         gm->getGoldKeys(),
                                         surfaceManager->getGoldKeyValuePairs(cpuName, gpuName),
                                         bitmap,
                                         pngPath.c_str(),
                                         jsonPath.c_str(),
                                         knownDigests);
        if (pngAndJSONResult.status == WritePNGAndJSONFilesResult::kError) {
            TestRunner::Log("\tERROR: %s", pngAndJSONResult.errorMsg.c_str());
            gNumFailedGMs++;
        } else if (pngAndJSONResult.status == WritePNGAndJSONFilesResult::kSkippedKnownDigest) {
            TestRunner::Log("\tSkipping known digest: %s", pngAndJSONResult.skippedDigest.c_str());
        } else {
            gNumSuccessfulGMs++;
            TestRunner::Log("\tPNG file written to: %s", pngPath.c_str());
            TestRunner::Log("\tJSON file written to: %s", jsonPath.c_str());
        }
    }
}

// Reads a plaintext file with "known digests" (i.e. digests that are known positives or negatives
// in Gold) and returns the digests (MD5 hashes) as a set of strings.
std::set<std::string> read_known_digests_file(std::string path) {
    std::set<std::string> hashes;
    std::regex md5HashRegex("^[a-fA-F0-9]{32}$");
    std::ifstream f(path);
    std::string line;
    for (int lineNum = 1; std::getline(f, line); lineNum++) {
        // Trim left and right (https://stackoverflow.com/a/217605).
        auto isSpace = [](unsigned char c) { return !std::isspace(c); };
        std::string md5 = line;
        md5.erase(md5.begin(), std::find_if(md5.begin(), md5.end(), isSpace));
        md5.erase(std::find_if(md5.rbegin(), md5.rend(), isSpace).base(), md5.end());

        if (md5 == "") continue;

        if (!std::regex_match(md5, md5HashRegex)) {
            SK_ABORT(
                    "File '%s' passed via --knownDigestsFile contains an invalid entry on line "
                    "%d: '%s'",
                    path.c_str(),
                    lineNum,
                    line.c_str());
        }
        hashes.insert(md5);
    }
    return hashes;
}

int main(int argc, char** argv) {
    TestRunner::InitAndLogCmdlineArgs(argc, argv);

    // When running under Bazel (e.g. "bazel test //path/to:test"), we'll store output files in
    // $TEST_UNDECLARED_OUTPUTS_DIR unless overridden via the --outputDir flag.
    //
    // See https://bazel.build/reference/test-encyclopedia#initial-conditions.
    std::string testUndeclaredOutputsDir;
    if (char* envVar = std::getenv("TEST_UNDECLARED_OUTPUTS_DIR")) {
        testUndeclaredOutputsDir = envVar;
    }
    bool isBazelTest = !testUndeclaredOutputsDir.empty();

    // Parse and validate flags.
    CommandLineFlags::Parse(argc, argv);
    if (!isBazelTest) {
        TestRunner::FlagValidators::StringNonEmpty("--outputDir", FLAGS_outputDir);
    }
    TestRunner::FlagValidators::StringAtMostOne("--outputDir", FLAGS_outputDir);
    TestRunner::FlagValidators::StringAtMostOne("--knownDigestsFile", FLAGS_knownDigestsFile);
    TestRunner::FlagValidators::StringEven("--key", FLAGS_key);
    TestRunner::FlagValidators::StringNonEmpty("--surfaceConfig", FLAGS_surfaceConfig);
    TestRunner::FlagValidators::StringAtMostOne("--surfaceConfig", FLAGS_surfaceConfig);
    TestRunner::FlagValidators::StringAtMostOne("--cpuName", FLAGS_cpuName);
    TestRunner::FlagValidators::StringAtMostOne("--gpuName", FLAGS_gpuName);
    TestRunner::FlagValidators::StringAtMostOne("--via", FLAGS_via);

    std::string outputDir =
            FLAGS_outputDir.isEmpty() ? testUndeclaredOutputsDir : FLAGS_outputDir[0];

    auto knownDigests = std::set<std::string>();
    if (!FLAGS_knownDigestsFile.isEmpty()) {
        knownDigests = read_known_digests_file(FLAGS_knownDigestsFile[0]);
        TestRunner::Log(
                "Read %zu known digests from: %s", knownDigests.size(), FLAGS_knownDigestsFile[0]);
    }

    std::map<std::string, std::string> keyValuePairs;
    for (int i = 1; i < FLAGS_key.size(); i += 2) {
        keyValuePairs[FLAGS_key[i - 1]] = FLAGS_key[i];
    }
    std::string config = FLAGS_surfaceConfig[0];
    std::string cpuName = FLAGS_cpuName.isEmpty() ? "" : FLAGS_cpuName[0];
    std::string gpuName = FLAGS_gpuName.isEmpty() ? "" : FLAGS_gpuName[0];

    // Execute all GM registerer functions, then run all registered GMs.
    for (const skiagm::GMRegistererFn& f : skiagm::GMRegistererFnRegistry::Range()) {
        std::string errorMsg = f();
        if (errorMsg != "") {
            SK_ABORT("Error while gathering GMs: %s", errorMsg.c_str());
        }
    }
    for (const skiagm::GMFactory& f : skiagm::GMRegistry::Range()) {
        std::unique_ptr<skiagm::GM> gm = f();

        if (!TestRunner::ShouldRunTestCase(gm->getName().c_str(), FLAGS_match, FLAGS_skip)) {
            TestRunner::Log("Skipping %s", gm->getName().c_str());
            gNumSkippedGMs++;
            continue;
        }

        run_gm(std::move(gm), config, keyValuePairs, cpuName, gpuName, outputDir, knownDigests);
    }

    // TODO(lovisolo): If running under Bazel, print command to display output files.

    TestRunner::Log(gNumFailedGMs > 0 ? "FAIL" : "PASS");
    TestRunner::Log(
            "%d successful GMs (images written to %s).", gNumSuccessfulGMs, outputDir.c_str());
    TestRunner::Log("%d failed GMs.", gNumFailedGMs);
    TestRunner::Log("%d skipped GMs.", gNumSkippedGMs);
    return gNumFailedGMs > 0 ? 1 : 0;
}
