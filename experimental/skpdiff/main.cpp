/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define __NO_STD_VECTOR // Uses cl::vectpr instead of std::vectpr
#define __NO_STD_STRING // Uses cl::STRING_CLASS instead of std::string
#include <CL/cl.hpp>

#include "SkOSFile.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTArray.h"
#include "SkTDArray.h"

#include "SkImageDiffer.h"
#include "SkCLImageDiffer.h"
#include "skpdiff_util.h"

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
        SkDebugf("%s\n", baseFilename);

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
            }
        } else {
            SkDebugf("Baseline file \"%s\" has no corresponding test file\n", baselineFile.c_str());
        }
    }
}

static void print_help()
{
    SkDebugf(
    "Usage:\n" \
    "skpdiff <baseline directory> <test directory>\n\n"
    );
}

int main(int argc, char** argv) {
    if (argc != 3)
    {
        print_help();
        return 1;
    }

    // Setup OpenCL
    cl::Device device;
    cl::Context context;
    if (!init_device_and_context(&device, &context)) {
        return 1;
    }

    // Setup our differ of choice
    SkCLImageDiffer* differ = SkNEW(SkDifferentPixelsImageDiffer);
    if (!differ->init(device(), context())) {
        return 1;
    }

    // Diff our folders
    diff_directories(argv[1], argv[2], differ);

    return 0;
}
