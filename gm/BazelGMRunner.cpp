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
#include "include/private/base/SkDebug.h"
#include "src/core/SkMD5.h"
#include "src/utils/SkJSONWriter.h"
#include "src/utils/SkOSPath.h"
#include "tools/HashAndEncode.h"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

struct tm;

// TODO(lovisolo): Add flag --config.
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

// Simulates a RasterSink[1] with 8888 color type and sRGB color space.
//
// [1]
// https://skia.googlesource.com/skia/+/4a8198df9c6b0529be35c7070151efd5968bb9b6/dm/DMSrcSink.h#539
static sk_sp<SkSurface> make_surface(int width, int height) {
    return SkSurfaces::Raster(
            SkImageInfo::MakeN32(width, height, kPremul_SkAlphaType, SkColorSpace::MakeSRGB()));
}

// Takes a SkBitmap and writes the resulting PNG and MD5 hash into the given files. Returns an
// empty string on success, or an error message in the case of failures.
static std::string write_png_and_json_files(std::string name,
                                            SkBitmap& bitmap,
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

    // Write JSON file with MD5 hash.
    SkFILEWStream jsonFile(jsonPath);
    SkJSONWriter jsonWriter(&jsonFile, SkJSONWriter::Mode::kPretty);
    jsonWriter.beginObject();  // Root object.
    jsonWriter.appendString("name", name);
    jsonWriter.appendString("md5", md5);
    // TODO(lovisolo): Add config name (requires defining the --config flag first).
    jsonWriter.endObject();

    return "";
}

static std::string now() {
    std::time_t t = std::time(nullptr);
    std::tm* now = std::gmtime(&t);

    std::ostringstream oss;
    oss << std::put_time(now, "%Y-%m-%d %H:%M:%S UTC");
    return oss.str();
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
        std::ostringstream oss;
        oss << "GM runner invoked with arguments:";
        for (int i = 1; i < argc; i++) {
            oss << " " << argv[i];
        }
        SkDebugf("%s\n", oss.str().c_str());
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
        SkDebugf("Flag --outputDir cannot be empty.\n");
        return 1;
    }
    if (FLAGS_outputDir.size() > 1) {
        SkDebugf("Flag --outputDir takes one single value, got %d.\n", FLAGS_outputDir.size());
        return 1;
    }

    // Iterate over all registered GMs.
    bool failures = false;
    for (skiagm::GMFactory f : skiagm::GMRegistry::Range()) {
        std::unique_ptr<skiagm::GM> gm(f());
        SkDebugf("[%s] Drawing GM: %s\n", now().c_str(), gm->getName());

        // Create surface and canvas.
        sk_sp<SkSurface> surface = make_surface(gm->getISize().width(), gm->getISize().height());
        SkCanvas* canvas = surface->getCanvas();

        // Draw GM into canvas.
        SkString msg;
        skiagm::DrawResult result = gm->draw(canvas, &msg);

        // Report GM result and optional message.
        std::string resultAsStr;
        if (result == skiagm::DrawResult::kOk) {
            resultAsStr = "OK";
        } else if (result == skiagm::DrawResult::kFail) {
            resultAsStr = "Fail";
            failures = true;
        } else if (result == skiagm::DrawResult::kSkip) {
            resultAsStr = "Skip.";
        } else {
            resultAsStr = "Unknown.";
        }
        SkDebugf("[%s] Result: %s\n", now().c_str(), resultAsStr.c_str());
        if (!msg.isEmpty()) {
            SkDebugf("[%s] Message: \"%s\"\n", now().c_str(), msg.c_str());
        }

        // Maybe save PNG and JSON file with MD5 hash to disk.
        if (result == skiagm::DrawResult::kOk) {
            std::string name = std::string(gm->getName());
            std::string outputDir =
                    FLAGS_outputDir.isEmpty() ? testUndeclaredOutputsDir : FLAGS_outputDir[0];
            SkString pngPath = SkOSPath::Join(outputDir.c_str(), (name + ".png").c_str());
            SkString jsonPath = SkOSPath::Join(outputDir.c_str(), (name + ".json").c_str());

            SkBitmap bitmap;
            bitmap.allocPixelsFlags(canvas->imageInfo(), SkBitmap::kZeroPixels_AllocFlag);
            surface->readPixels(bitmap, 0, 0);

            std::string pngAndJSONResult = write_png_and_json_files(
                    gm->getName(), bitmap, pngPath.c_str(), jsonPath.c_str());
            if (pngAndJSONResult != "") {
                SkDebugf("[%s] %s\n", now().c_str(), pngAndJSONResult.c_str());
                failures = true;
            } else {
                SkDebugf("[%s] PNG file written to: %s\n", now().c_str(), pngPath.c_str());
                SkDebugf("[%s] JSON file written to: %s\n", now().c_str(), jsonPath.c_str());
            }
        }
    }

    // TODO(lovisolo): If running under Bazel, print command to display output files.

    if (failures) {
        SkDebugf("FAIL\n");
        return 1;
    }
    SkDebugf("PASS\n");
    return 0;
}
