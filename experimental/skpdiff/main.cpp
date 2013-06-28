/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define __NO_STD_VECTOR // Uses cl::vectpr instead of std::vectpr
#define __NO_STD_STRING // Uses cl::STRING_CLASS instead of std::string
#include <CL/cl.hpp>

#include "SkCommandLineFlags.h"
#include "SkGraphics.h"
#include "SkPoint.h"
#include "SkOSFile.h"
#include "SkString.h"
#include "SkTArray.h"
#include "SkTDArray.h"

#include "SkImageDiffer.h"
#include "SkCLImageDiffer.h"
#include "SkPMetric.h"
#include "skpdiff_util.h"

#include "SkForceLinking.h"
__SK_FORCE_IMAGE_DECODER_LINKING;

// Command line argument definitions go here
DEFINE_bool2(list, l, false, "List out available differs");
DEFINE_string2(differs, d, "", "The names of the differs to use or all of them by default");
DEFINE_string2(folders, f, "", "Compare two folders with identical subfile names: <baseline folder> <test folder>");
DEFINE_string2(patterns, p, "", "Use two patterns to compare images: <baseline> <test>");

/// A callback for any OpenCL errors
CL_CALLBACK void error_notify(const char* errorInfo, const void* privateInfoSize, ::size_t cb, void* userData) {
    SkDebugf("OpenCL error notify: %s\n", errorInfo);
    exit(1);
}

/// Creates a device and context with OpenCL
static bool init_device_and_context(cl::Device* device, cl::Context* context) {
    // Query for a platform
    cl::vector<cl::Platform> platformList;
    cl::Platform::get(&platformList);
    SkDebugf("The number of platforms is %u\n", platformList.size());

    // Print some information about the platform for debugging
    cl::Platform& platform = platformList[0];
    cl::STRING_CLASS platformName;
    platform.getInfo(CL_PLATFORM_NAME, &platformName);
    SkDebugf("Platform index 0 is named %s\n", platformName.c_str());

    // Query for a device
    cl::vector<cl::Device> deviceList;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &deviceList);
    SkDebugf("The number of GPU devices is %u\n", deviceList.size());

    // Print some information about the device for debugging
    *device = deviceList[0];
    cl::STRING_CLASS deviceName;
    device->getInfo(CL_DEVICE_NAME, &deviceName);
    SkDebugf("Device index 0 is named %s\n", deviceName.c_str());

    // Create a CL context and check for all errors
    cl_int contextErr = CL_SUCCESS;
    *context = cl::Context(deviceList, NULL, error_notify, NULL, &contextErr);
    if (contextErr != CL_SUCCESS) {
        SkDebugf("Context creation failed: %s\n", cl_error_to_string(contextErr));
        return false;
    }

    return true;
}

/// Compares two directories of images with the given differ
static void diff_directories(const char baselinePath[], const char testPath[], SkImageDiffer* differ) {
    // Get the files in the baseline, we will then look for those inside the test path
    SkTArray<SkString> baselineEntries;
    if (!get_directory(baselinePath, &baselineEntries)) {
        SkDebugf("Unable to open path \"%s\"\n", baselinePath);
        return;
    }

    SkTDArray<int> queuedDiffIDs;
    for (int baselineIndex = 0; baselineIndex < baselineEntries.count(); baselineIndex++) {
        const char* baseFilename = baselineEntries[baselineIndex].c_str();
        SkDebugf("\n%s\n", baseFilename);

        // Find the real location of each file to compare
        SkString baselineFile = SkOSPath::SkPathJoin(baselinePath, baseFilename);
        SkString testFile = SkOSPath::SkPathJoin(testPath, baseFilename);

        // Check that the test file exists and is a file
        if (sk_exists(testFile.c_str()) && !sk_isdir(testFile.c_str())) {
            // Queue up the comparison with the differ
            int diffID = differ->queueDiffOfFile(baselineFile.c_str(), testFile.c_str());
            if (diffID >= 0) {
                queuedDiffIDs.push(diffID);
                SkDebugf("Result: %f\n", differ->getResult(diffID));
                SkDebugf("POI Count: %i\n", differ->getPointsOfInterestCount(diffID));
                differ->deleteDiff(diffID);
            }
        } else {
            SkDebugf("Baseline file \"%s\" has no corresponding test file\n", baselineFile.c_str());
        }
    }
}


/// Compares two sets of images identified by glob style patterns with the given differ
static void diff_patterns(const char baselinePattern[], const char testPattern[], SkImageDiffer* differ) {
    // Get the files in the baseline and test patterns. Because they are in sorted order, it's easy
    // to find corresponding images by matching entry indices.

    SkTArray<SkString> baselineEntries;
    if (!glob_files(baselinePattern, &baselineEntries)) {
        SkDebugf("Unable to get pattern \"%s\"\n", baselinePattern);
        return;
    }

    SkTArray<SkString> testEntries;
    if (!glob_files(testPattern, &testEntries)) {
        SkDebugf("Unable to get pattern \"%s\"\n", testPattern);
        return;
    }

    if (baselineEntries.count() != testEntries.count()) {
        SkDebugf("Baseline and test patterns do not yield corresponding number of files\n");
        return;
    }

    SkTDArray<int> queuedDiffIDs;
    for (int entryIndex = 0; entryIndex < baselineEntries.count(); entryIndex++) {
        const char* baselineFilename = baselineEntries[entryIndex].c_str();
        const char* testFilename     = testEntries    [entryIndex].c_str();
        SkDebugf("\n%s %s\n", baselineFilename, testFilename);

        int diffID = differ->queueDiffOfFile(baselineFilename, testFilename);
        if (diffID >= 0) {
            queuedDiffIDs.push(diffID);
            SkDebugf("Result: %f\n", differ->getResult(diffID));
            SkDebugf("POI Count: %i\n", differ->getPointsOfInterestCount(diffID));
            differ->deleteDiff(diffID);
        }
    }
}


static bool init_cl_diff(SkImageDiffer* differ) {
    // Setup OpenCL
    cl::Device device;
    cl::Context context;
    if (!init_device_and_context(&device, &context)) {
        return false;
    }

    // Setup our differ of choice
    SkCLImageDiffer* clDiffer = (SkCLImageDiffer*)differ;
    return clDiffer->init(device(), context());
}

static bool init_dummy(SkImageDiffer* differ) {
    return true;
}


// TODO Find a better home for the diff registry. One possibility is to have the differs self
// register.

// List here every differ
SkDifferentPixelsImageDiffer gDiffPixel;
SkPMetric gPDiff;

// A null terminated array of pointer to every differ declared above
SkImageDiffer* gDiffers[] = { &gDiffPixel, &gPDiff, NULL };

// A parallel array of functions to initialize the above differs. The reason we don't initialize
// everything immediately is that certain differs may require special initialization, but we still
// want to construct all of them globally so they can be queried for things like their name and
// description.
bool (*gDiffInits[])(SkImageDiffer*) = { init_cl_diff, init_dummy, NULL };


int main(int argc, char** argv) {
    // Setup command line parsing
    SkCommandLineFlags::SetUsage("Compare images using various metrics.");
    SkCommandLineFlags::Parse(argc, argv);

    // Needed by various Skia components
    SkAutoGraphics ag;

    if (FLAGS_list) {
        SkDebugf("Available Metrics:\n");
    }

    // Figure which differs the user chose, and optionally print them if the user requests it
    SkTDArray<int> chosenDiffers;
    for (int differIndex = 0; NULL != gDiffers[differIndex]; differIndex++) {
        if (FLAGS_list) {
            SkDebugf("    %s", gDiffers[differIndex]->getName());
            SkDebugf("\n");
        }

        // Check if this differ was chosen by any of the flags
        if (FLAGS_differs.isEmpty()) {
            // If no differs were chosen, they all get added
            chosenDiffers.push(differIndex);
        } else {
            for (int flagIndex = 0; flagIndex < FLAGS_differs.count(); flagIndex++) {
                if (SkString(FLAGS_differs[flagIndex]).equals(gDiffers[differIndex]->getName())) {
                    chosenDiffers.push(differIndex);
                    break;
                }
            }
        }
    }

    // Don't attempt to initialize the differ if we aren't going to use it
    if (FLAGS_folders.isEmpty() && FLAGS_patterns.isEmpty()) {
        return 0;
    }

    // Validate command line flags
    if (!FLAGS_folders.isEmpty()) {
        if (2 != FLAGS_folders.count()) {
            SkDebugf("Folders flag expects two arguments: <baseline folder> <test folder>\n");
            return 1;
        }
    }

    if (!FLAGS_patterns.isEmpty()) {
        if (2 != FLAGS_patterns.count()) {
            SkDebugf("Patterns flag expects two arguments: <baseline pattern> <test pattern>\n");
            return 1;
        }
    }

    // TODO Move the differ loop to after the bitmaps are decoded and/or uploaded to the OpenCL
    // device. Those are often the slowest processes and should not be done more than once if it can
    // be helped.

    // Perform each requested diff
    for (int chosenDifferIndex = 0; chosenDifferIndex < chosenDiffers.count(); chosenDifferIndex++) {
        int differIndex = chosenDiffers[chosenDifferIndex];

        // Get the chosen differ and say which one they chose
        SkImageDiffer * differ = gDiffers[differIndex];
        SkDebugf("Using metric \"%s\"\n", differ->getName());

        // Initialize the differ using the global list of init functions that match the list of
        // differs
        gDiffInits[differIndex](differ);

        // Perform a folder diff if one is requested
        if (!FLAGS_folders.isEmpty()) {
            diff_directories(FLAGS_folders[0], FLAGS_folders[1], differ);
        }

        // Perform a pattern diff if one is requested
        if (!FLAGS_patterns.isEmpty()) {
            diff_patterns(FLAGS_patterns[0], FLAGS_patterns[1], differ);
        }
    }

    return 0;
}
