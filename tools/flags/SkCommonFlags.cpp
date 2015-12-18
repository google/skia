/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCommonFlags.h"

DEFINE_bool(cpu, true, "master switch for running CPU-bound work.");

DEFINE_bool(dryRun, false,
            "just print the tests that would be run, without actually running them.");

DEFINE_bool(gpu, true, "master switch for running GPU-bound work.");

DEFINE_string(images, "", "Directory of images to decode.");

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

DEFINE_bool(preAbandonGpuContext, false, "Abandons the GrContext before running the test.");

DEFINE_bool(abandonGpuContext, false, "Abandon the GrContext after running each test.");

DEFINE_string(skps, "skps", "Directory to read skps from.");

DEFINE_int32(threads, -1, "Run threadsafe tests on a threadpool with this many extra threads, "
                          "defaulting to one extra thread per core.");

DEFINE_bool2(verbose, v, false, "enable verbose output from the test driver.");

DEFINE_bool2(veryVerbose, V, false, "tell individual tests to be verbose.");

DEFINE_string2(writePath, w, "", "If set, write bitmaps here as .pngs.");

DEFINE_string(key, "",
              "Space-separated key/value pairs to add to JSON identifying this builder.");
DEFINE_string(properties, "",
              "Space-separated key/value pairs to add to JSON identifying this run.");

