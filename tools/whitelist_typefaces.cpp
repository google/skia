/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGraphics.h"

extern bool CheckChecksums();
extern bool GenerateChecksums();

int main(int argc, char** argv) {
    if (argc == 2) {
        SkAutoGraphics ag;  // Enable use of SkRTConfig
        if (!strcmp(argv[1], "--check")) {
            return (int) !CheckChecksums();
        }
        if (!strcmp(argv[1], "--generate")) {
            if (!GenerateChecksums()) {
                return 2;
            }
            return 0;
        }
    }
    SkDebugf("Usage:\n %s [--check] [--generate]\n\n", argv[0]);
    return 3;
}
