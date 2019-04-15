// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "PngTool.h"

#include <vector>
#include <cstdio>
#include <cstdlib>

static std::vector<unsigned char> readfile(const char* path) {
    std::vector<unsigned char> result;
    if (FILE* f = fopen(path, "rb")) {
        if (fseek(f, 0, SEEK_END) == 0) {
            long fsize = ftell(f);
            if (fsize > 0) {
                fseek(f, 0, SEEK_SET);
                result.resize((size_t)fsize);
                fread(result.data(), 1, fsize, f);
            }
        }
        fclose(f);
    }  // return empty on error.
    return result;
}

int main(int argc, char** argv) {
    if (argc < 5) {
        fprintf(stderr, "Usage:\n  %s ICC_PROFILE_NAME ICC_PROFILE_PATH IN_PNG OUT_PNG\n", argv[0]);
        return 1;
    }
    std::vector<unsigned char> icc = readfile(argv[2]);
    if (icc.empty()) { return 2; }
    PngTool::Pixmap pixmap = PngTool::Read(argv[3], &malloc);
    if (!pixmap.pixels) { return 3; }
    PngTool::WriteOptions options;
    options.iccName = argv[1];
    options.iccData = icc.data();
    options.iccDataLength = icc.size();
    options.fast = false;
    bool success = PngTool::Write(argv[4], pixmap, options);
    free(pixmap.pixels);
    return success ? 0 : 4;
}
