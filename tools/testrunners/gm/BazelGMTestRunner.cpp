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
#include "tools/testrunners/common/surface_manager/SurfaceManager.h"
#include "tools/testrunners/gm/vias/Draw.h"

#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

// TODO(lovisolo): Add flag --skip.
// TODO(lovisolo): Add flag --omitDigestIfHashInFile (provides the known hashes file).

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

// We named this flag --surfaceConfig rather than --config to avoid confusion with the --config
// Bazel flag.
static DEFINE_string(surfaceConfig,
                     "",
                     "Name of the Surface configuration to use (e.g. \"8888\"). This determines "
                     "how we construct the SkSurface from which we get the SkCanvas that GMs will "
                     "draw on. See file //tools/testrunners/common/surface_manager/SurfaceManager.h for "
                     "details.");

static DEFINE_string(via,
                     "direct",  // Equivalent to running DM without a via.
                     "Name of the \"via\" to use (e.g. \"picture_serialization\"). Optional.");

// Takes a SkBitmap and writes the resulting PNG and MD5 hash into the given files. Returns an
// empty string on success, or an error message in the case of failures.
static std::string write_png_and_json_files(std::string name,
                                            std::map<std::string, std::string> gmGoldKeys,
                                            std::map<std::string, std::string> surfaceGoldKeys,
                                            const SkBitmap& bitmap,
                                            const char* pngPath,
                                            const char* jsonPath) {
    HashAndEncode hashAndEncode(bitmap);

    // Compute MD5 hash.
    SkMD5 hash;
    hashAndEncode.feedHash(&hash);
    SkMD5::Digest digest = hash.finish();
    SkString md5 = digest.toLowercaseHexString();

    // Write PNG file.
    SkFILEWStream pngFile(pngPath);
    bool result = hashAndEncode.encodePNG(&pngFile,
                                          md5.c_str(),
                                          /* key= */ CommandLineFlags::StringArray(),
                                          /* properties= */ CommandLineFlags::StringArray());
    if (!result) {
        return "Error encoding or writing PNG to " + std::string(pngPath);
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

    return "";
}

static std::string now() {
    std::time_t t = std::time(nullptr);
    std::tm* now = std::gmtime(&t);

    std::ostringstream oss;
    oss << std::put_time(now, "%Y-%m-%d %H:%M:%S UTC");
    return oss.str();
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

// Runs a GM under the given surface config, and saves its output PNG file (and accompanying JSON
// file with metadata) to the given output directory.
void run_gm(std::unique_ptr<skiagm::GM> gm, std::string config, std::string outputDir) {
    SkDebugf("[%s] GM: %s\n", now().c_str(), gm->getName().c_str());

    // Create surface and canvas.
    std::unique_ptr<SurfaceManager> surfaceManager = SurfaceManager::FromConfig(
            config, SurfaceOptions{gm->getISize().width(), gm->getISize().height()});
    if (surfaceManager == nullptr) {
        SK_ABORT("Unknown --surfaceConfig flag value: %s.", config.c_str());
    }

    // Set up GPU.
    SkDebugf("[%s]     Setting up GPU...\n", now().c_str());
    SkString msg;
    skiagm::DrawResult result = gm->gpuSetup(surfaceManager->getSurface()->getCanvas(), &msg);

    // Draw GM into canvas if GPU setup was successful.
    SkBitmap bitmap;
    if (result == skiagm::DrawResult::kOk) {
        GMOutput output;
        std::string viaName = FLAGS_via.size() == 0 ? "" : (FLAGS_via[0]);
        SkDebugf("[%s]     Drawing GM via \"%s\"...\n", now().c_str(), viaName.c_str());
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
            SkDebugf("[%s]     Flushing surface...\n", now().c_str());
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
    SkDebugf("[%s]     Result: %s\n", now().c_str(), draw_result_to_string(result).c_str());
    if (!msg.isEmpty()) {
        SkDebugf("[%s]     Message: \"%s\"\n", now().c_str(), msg.c_str());
    }

    // Save PNG and JSON file with MD5 hash to disk if the GM was successful.
    if (result == skiagm::DrawResult::kOk) {
        std::string name = std::string(gm->getName().c_str());
        SkString pngPath = SkOSPath::Join(outputDir.c_str(), (name + ".png").c_str());
        SkString jsonPath = SkOSPath::Join(outputDir.c_str(), (name + ".json").c_str());

        std::string pngAndJSONResult = write_png_and_json_files(gm->getName().c_str(),
                                                                gm->getGoldKeys(),
                                                                surfaceManager->getGoldKeys(),
                                                                bitmap,
                                                                pngPath.c_str(),
                                                                jsonPath.c_str());
        if (pngAndJSONResult != "") {
            SkDebugf("[%s] %s\n", now().c_str(), pngAndJSONResult.c_str());
            gNumFailedGMs++;
        } else {
            gNumSuccessfulGMs++;
            SkDebugf("[%s]     PNG file written to: %s\n", now().c_str(), pngPath.c_str());
            SkDebugf("[%s]     JSON file written to: %s\n", now().c_str(), jsonPath.c_str());
        }
    }
}

int main(int argc, char** argv) {
#ifdef SK_BUILD_FOR_ANDROID
    extern bool gSkDebugToStdOut;  // If true, sends SkDebugf to stdout as well.
    gSkDebugToStdOut = true;
#endif

    // Print command-line for debugging purposes.
    if (argc < 2) {
        SkDebugf("GM runner invoked with no arguments.\n");
    } else {
        SkDebugf("GM runner invoked with arguments:");
        for (int i = 1; i < argc; i++) {
            SkDebugf(" %s", argv[i]);
        }
        SkDebugf("\n");
    }

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
    if (!isBazelTest && FLAGS_outputDir.isEmpty()) {
        SK_ABORT("Flag --outputDir cannot be empty.");
    }
    if (FLAGS_outputDir.size() > 1) {
        SK_ABORT("Flag --outputDir takes one single value, got %d.", FLAGS_outputDir.size());
    }
    if (FLAGS_surfaceConfig.isEmpty()) {
        SK_ABORT("Flag --surfaceConfig cannot be empty.");
    }
    if (FLAGS_surfaceConfig.size() > 1) {
        SK_ABORT("Flag --surfaceConfig takes one single value, got %d.",
                 FLAGS_surfaceConfig.size());
    }
    if (FLAGS_via.size() > 1) {
        SK_ABORT("Flag --via takes at most one value, got %d.", FLAGS_via.size());
    }

    std::string outputDir =
            FLAGS_outputDir.isEmpty() ? testUndeclaredOutputsDir : FLAGS_outputDir[0];
    std::string config = FLAGS_surfaceConfig[0];

    // Execute all GM registerer functions, then run all registered GMs.
    for (const skiagm::GMRegistererFn& f : skiagm::GMRegistererFnRegistry::Range()) {
        std::string errorMsg = f();
        if (errorMsg != "") {
            SK_ABORT("Error while gathering GMs: %s", errorMsg.c_str());
        }
    }
    for (const skiagm::GMFactory& f : skiagm::GMRegistry::Range()) {
        run_gm(f(), config, outputDir);
    }

    // TODO(lovisolo): If running under Bazel, print command to display output files.

    SkDebugf(gNumFailedGMs > 0 ? "FAIL\n" : "PASS\n");
    SkDebugf("%d successful GMs (images written to %s).\n", gNumSuccessfulGMs, outputDir.c_str());
    SkDebugf("%d failed GMs.\n", gNumFailedGMs);
    SkDebugf("%d skipped GMs.\n", gNumSkippedGMs);
    return gNumFailedGMs > 0 ? 1 : 0;
}
