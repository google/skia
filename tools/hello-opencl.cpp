/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a simple OpenCL Hello World that tests you have a functioning OpenCL setup.

#include <CL/cl.hpp>

extern "C" {
    #include "cl/assert_cl.h"   // for cl() macro
    #include "cl/find_cl.h"     // for clFindIdsByName
}

int main(int argc, char** argv) {
    // Find any OpenCL platform+device with these substrings.
    const char* match_platform = argc > 1 ? argv[1] : "";
    const char* match_device   = argc > 2 ? argv[2] : "";

    cl_platform_id platform;
    cl_device_id   device;

    char device_name[256];
    size_t device_name_len;

    // clFindIdsByName will narrate what it's doing when this is set.
    bool verbose = true;

    // The cl() macro prepends cl to its argument, calls it, and asserts that it succeeded,
    // printing out the file, line, and somewhat readable version of the error code on failure.
    //
    // It's generally used to call OpenCL APIs, but here we've written clFindIdsByName to match
    // the convention, as its error conditions are just going to be passed along from OpenCL.
    cl(FindIdsByName(match_platform,  match_device,
                          &platform,       &device,
                     sizeof(device_name), device_name, &device_name_len,
                     verbose));

    printf("picked %.*s\n", (int)device_name_len, device_name);

    return 0;
}
