// Copyright 2021 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.


#include "include/core/SkGraphics.h"
#include "tools/flags/CommandLineFlags.h"

static DEFINE_int_2(option, o, 0, "An option");

static void exitf(const char* format, ...) SK_PRINTF_LIKE(1, 2);

static void exitf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    exit(1);
}

int main(int argc, char** argv) {
    CommandLineFlags::Parse(argc, argv);

    if (FLAGS_option) {
        exitf("Invalid option\n");
    }

    SkGraphics::Init();

    return 0;
}
