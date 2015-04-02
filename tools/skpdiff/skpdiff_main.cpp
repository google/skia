/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// TODO(djsollen): Rename this whole package (perhaps to "SkMultiDiffer").
// It's not just for "pdiff" (perceptual diffs)--it's a harness that allows
// the execution of an arbitrary set of difference algorithms.
// See http://skbug.com/2711 ('rename skpdiff')

#include "SkTypes.h"

#if SK_SUPPORT_OPENCL

#define __NO_STD_VECTOR // Uses cl::vectpr instead of std::vectpr
#define __NO_STD_STRING // Uses cl::STRING_CLASS instead of std::string
#if defined(SK_BUILD_FOR_MAC)
// Note that some macs don't have this header and it can be downloaded from the Khronos registry
#   include <OpenCL/cl.hpp>
#else
#   include <CL/cl.hpp>
#endif

#endif

#include "SkCommandLineFlags.h"
#include "SkGraphics.h"
#include "SkStream.h"
#include "SkTDArray.h"
#include "SkTaskGroup.h"

#include "SkDifferentPixelsMetric.h"
#include "SkDiffContext.h"
#include "SkImageDiffer.h"
#include "SkPMetric.h"
#include "skpdiff_util.h"

#include "SkForceLinking.h"
__SK_FORCE_IMAGE_DECODER_LINKING;

// Command line argument definitions go here
DEFINE_bool2(list, l, false, "List out available differs");
DEFINE_string2(differs, d, "", "The names of the differs to use or all of them by default");
DEFINE_string2(folders, f, "", "Compare two folders with identical subfile names: <baseline folder> <test folder>");
DEFINE_string2(patterns, p, "", "Use two patterns to compare images: <baseline> <test>");
DEFINE_string2(output, o, "", "Writes a JSON summary of these diffs to file: <filepath>");
DEFINE_string(alphaDir, "", "If the differ can generate an alpha mask, write it into directory: <dirpath>");
DEFINE_string(rgbDiffDir, "", "If the differ can generate an image showing the RGB diff at each pixel, write it into directory: <dirpath>");
DEFINE_string(whiteDiffDir, "", "If the differ can generate an image showing every changed pixel in white, write it into directory: <dirpath>");
DEFINE_bool(jsonp, true, "Output JSON with padding");
DEFINE_string(csv, "", "Writes the output of these diffs to a csv file: <filepath>");
DEFINE_int32(threads, -1, "run N threads in parallel [default is derived from CPUs available]");
DEFINE_bool(longnames, false, "Output image names are a combination of baseline and test names");

#if SK_SUPPORT_OPENCL
/// A callback for any OpenCL errors
static void CL_CALLBACK error_notify(const char* errorInfo, const void* privateInfoSize, ::size_t cb, void* userData) {
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
    platform.getDevices(CL_DEVICE_TYPE_ALL, &deviceList);
    SkDebugf("The number of devices is %u\n", deviceList.size());

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
#endif

// TODO Find a better home for the diff registry. One possibility is to have the differs self
// register.

// List here every differ
SkDifferentPixelsMetric gDiffPixel;
SkPMetric gPDiff;

// A null terminated array of pointer to every differ declared above
SkImageDiffer* gDiffers[] = { &gDiffPixel, &gPDiff, NULL };

int tool_main(int argc, char * argv[]);
int tool_main(int argc, char * argv[]) {
    // Setup command line parsing
    SkCommandLineFlags::SetUsage("Compare images using various metrics.");
    SkCommandLineFlags::Parse(argc, argv);

    // Needed by various Skia components
    SkAutoGraphics ag;
    SkTaskGroup::Enabler enabled;

    if (FLAGS_list) {
        SkDebugf("Available Metrics:\n");
    }

    // Figure which differs the user chose, and optionally print them if the user requests it
    SkTDArray<SkImageDiffer*> chosenDiffers;
    for (int differIndex = 0; gDiffers[differIndex]; differIndex++) {
        SkImageDiffer* differ = gDiffers[differIndex];
        if (FLAGS_list) {
            SkDebugf("    %s", differ->getName());
            SkDebugf("\n");
        }

        // Check if this differ was chosen by any of the flags. Initialize them if they were chosen.
        if (FLAGS_differs.isEmpty()) {
            // If no differs were chosen, they all get added
            if (differ->requiresOpenCL()) {
#if SK_SUPPORT_OPENCL
                init_cl_diff(differ);
                chosenDiffers.push(differ);
#endif
            } else {
                chosenDiffers.push(differ);
            }
        } else {
            for (int flagIndex = 0; flagIndex < FLAGS_differs.count(); flagIndex++) {
                if (SkString(FLAGS_differs[flagIndex]).equals(differ->getName())) {
                    // Initialize OpenCL for the differ if it needs it and support was compiled in.
                    if (differ->requiresOpenCL()) {
#if SK_SUPPORT_OPENCL
                        init_cl_diff(differ);
                        chosenDiffers.push(differ);
#endif
                    } else {
                        chosenDiffers.push(differ);
                    }
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

    if (!FLAGS_csv.isEmpty()) {
        if (1 != FLAGS_csv.count()) {
            SkDebugf("csv flag expects one argument: <csv file>\n");
            return 1;
        }
    }

    if (!FLAGS_alphaDir.isEmpty()) {
        if (1 != FLAGS_alphaDir.count()) {
            SkDebugf("alphaDir flag expects one argument: <directory>\n");
            return 1;
        }
    }
    if (!FLAGS_rgbDiffDir.isEmpty()) {
        if (1 != FLAGS_rgbDiffDir.count()) {
            SkDebugf("rgbDiffDir flag expects one argument: <directory>\n");
            return 1;
        }
    }

    if (!FLAGS_whiteDiffDir.isEmpty()) {
        if (1 != FLAGS_whiteDiffDir.count()) {
            SkDebugf("whiteDiffDir flag expects one argument: <directory>\n");
            return 1;
        }
    }

    SkDiffContext ctx;
    ctx.setDiffers(chosenDiffers);
    ctx.setLongNames(FLAGS_longnames);

    if (!FLAGS_alphaDir.isEmpty()) {
        ctx.setAlphaMaskDir(SkString(FLAGS_alphaDir[0]));
    }
    if (!FLAGS_rgbDiffDir.isEmpty()) {
        ctx.setRgbDiffDir(SkString(FLAGS_rgbDiffDir[0]));
    }
    if (!FLAGS_whiteDiffDir.isEmpty()) {
        ctx.setWhiteDiffDir(SkString(FLAGS_whiteDiffDir[0]));
    }

    if (FLAGS_threads >= 0) {
        ctx.setThreadCount(FLAGS_threads);
    }

    // Perform a folder diff if one is requested
    if (!FLAGS_folders.isEmpty()) {
        ctx.diffDirectories(FLAGS_folders[0], FLAGS_folders[1]);
    }

    // Perform a pattern diff if one is requested
    if (!FLAGS_patterns.isEmpty()) {
        ctx.diffPatterns(FLAGS_patterns[0], FLAGS_patterns[1]);
    }

    // Output to the file specified
    if (!FLAGS_output.isEmpty()) {
        SkFILEWStream outputStream(FLAGS_output[0]);
        ctx.outputRecords(outputStream, FLAGS_jsonp);
    }

    if (!FLAGS_csv.isEmpty()) {
        SkFILEWStream outputStream(FLAGS_csv[0]);
        ctx.outputCsv(outputStream);
    }

    return 0;
}

#if !defined(SK_BUILD_FOR_IOS)
int main(int argc, char * argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
