/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkString.h"

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkAutoGraphics ag;

    for (int i = 1; i < argc; ++i) {
        SkString src(argv[i]);
        if (src.endsWith(".png")) {
            SkString dst(src.c_str(), src.size() - 4);
            dst.append(".jpg");

            SkBitmap bm;
            if (SkImageDecoder::DecodeFile(src.c_str(), &bm)) {
                if (SkImageEncoder::EncodeFile(dst.c_str(), bm, SkImageEncoder::kJPEG_Type, 100)) {
                    SkDebugf("converted %s to %s\n", src.c_str(), dst.c_str());
                } else {
                    SkDebugf("failed to encode %s\n", src.c_str());
                }
            } else {
                SkDebugf("failed to decode %s\n", src.c_str());
            }
        }
    }
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
