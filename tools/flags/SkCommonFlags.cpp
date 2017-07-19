/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCommonFlags.h"
#include "SkOSFile.h"
#include "SkOSPath.h"

DEFINE_bool(cpu, true, "master switch for running CPU-bound work.");

DEFINE_bool(dryRun, false,
            "just print the tests that would be run, without actually running them.");

DEFINE_bool(gpu, true, "master switch for running GPU-bound work.");

DEFINE_string(images, "", "List of images and/or directories to decode. A directory with no images"
                          " is treated as a fatal error.");

DEFINE_string(colorImages, "", "List of images and/or directories to decode with color correction. "
                               "A directory with no images is treated as a fatal error.");

DEFINE_bool(simpleCodec, false, "Runs of a subset of the codec tests.  "
                                "For DM, this means no scaling or subsetting, always using the "
                                "canvas color type.  "
                                "For nanobench, this means always N32, Premul or Opaque.");

DEFINE_string2(match, m, nullptr,
               "[~][^]substring[$] [...] of GM name to run.\n"
               "Multiple matches may be separated by spaces.\n"
               "~ causes a matching GM to always be skipped\n"
               "^ requires the start of the GM to match\n"
               "$ requires the end of the GM to match\n"
               "^ and $ requires an exact match\n"
               "If a GM does not match any list entry,\n"
               "it is skipped unless some list entry starts with ~");

DEFINE_bool2(quiet, q, false, "if true, don't print status updates.");

DEFINE_bool(preAbandonGpuContext, false, "Test abandoning the GrContext before running the test.");

DEFINE_bool(abandonGpuContext, false, "Test abandoning the GrContext after running each test.");

DEFINE_bool(releaseAndAbandonGpuContext, false,
            "Test releasing all gpu resources and abandoning the GrContext after running each "
            "test");

DEFINE_string(skps, "skps", "Directory to read skps from.");

DEFINE_string(svgs, "", "Directory to read SVGs from, or a single SVG file.");

DEFINE_int32_2(threads, j, -1, "Run threadsafe tests on a threadpool with this many extra threads, "
                               "defaulting to one extra thread per core.");

DEFINE_bool2(verbose, v, false, "enable verbose output from the test driver.");

DEFINE_bool2(veryVerbose, V, false, "tell individual tests to be verbose.");

DEFINE_string2(writePath, w, "", "If set, write bitmaps here as .pngs.");

DEFINE_string(key, "",
              "Space-separated key/value pairs to add to JSON identifying this builder.");
DEFINE_string(properties, "",
              "Space-separated key/value pairs to add to JSON identifying this run.");
DEFINE_bool2(pre_log, p, false, "Log before running each test. May be incomprehensible when threading");

DEFINE_bool(analyticAA, true, "If false, disable analytic anti-aliasing");

DEFINE_bool(forceAnalyticAA, false, "Force analytic anti-aliasing even if the path is complicated: "
                                    "whether it's concave or convex, we consider a path complicated"
                                    "if its number of points is comparable to its resolution.");

DEFINE_bool(trace, false, "Show trace events using SkDebugf.");

bool CollectImages(SkCommandLineFlags::StringArray images, SkTArray<SkString>* output) {
    SkASSERT(output);

    static const char* const exts[] = {
        "bmp", "gif", "jpg", "jpeg", "png", "webp", "ktx", "astc", "wbmp", "ico",
        "BMP", "GIF", "JPG", "JPEG", "PNG", "WEBP", "KTX", "ASTC", "WBMP", "ICO",
#ifdef SK_CODEC_DECODES_RAW
        "arw", "cr2", "dng", "nef", "nrw", "orf", "raf", "rw2", "pef", "srw",
        "ARW", "CR2", "DNG", "NEF", "NRW", "ORF", "RAF", "RW2", "PEF", "SRW",
#endif
    };

    for (int i = 0; i < images.count(); ++i) {
        const char* flag = images[i];
        if (!sk_exists(flag)) {
            SkDebugf("%s does not exist!\n", flag);
            return false;
        }

        if (sk_isdir(flag)) {
            // If the value passed in is a directory, add all the images
            bool foundAnImage = false;
            for (const char* ext : exts) {
                SkOSFile::Iter it(flag, ext);
                SkString file;
                while (it.next(&file)) {
                    foundAnImage = true;
                    output->push_back() = SkOSPath::Join(flag, file.c_str());
                }
            }
            if (!foundAnImage) {
                SkDebugf("No supported images found in %s!\n", flag);
                return false;
            }
        } else {
            // Also add the value if it is a single image
            output->push_back() = flag;
        }
    }
    return true;
}
