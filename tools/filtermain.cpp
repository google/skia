/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGraphics.h"
#include "SkPicture.h"
#include "SkStream.h"

///////////////////////////////////////////////////////////////////////////////
static void usage() {
    SkDebugf("Usage: filter -i inFile -o outFile [-h|--help]");
    SkDebugf("\n\n");
    SkDebugf("    -i inFile  : file to file.\n");
    SkDebugf("    -o outFile : result of filtering.\n");
    SkDebugf("    -h|--help  : Show this help message.\n");
}

int filter_main(int argc, char** argv);
int filter_main(int argc, char** argv) {

    SkGraphics::Init();

    SkString inFile, outFile;

    if (argc < 5) {
        usage();
        return -1;
    }

    char* const* stop = argv + argc;
    for (++argv; argv < stop; ++argv) {
        if (strcmp(*argv, "-i") == 0) {
            argv++;
            if (argv < stop && **argv) {
                inFile.set(*argv);
            } else {
                SkDebugf("missing arg for --i\n");
                usage();
                return -1;
            }
        } else if (strcmp(*argv, "-o") == 0) {
            argv++;
            if (argv < stop && **argv) {
                outFile.set(*argv);
            } else {
                SkDebugf("missing arg for --o\n");
                usage();
                return -1;
            }
        } else if (strcmp(*argv, "--help") == 0 || strcmp(*argv, "-h") == 0) {
            usage();
            return 0;
        } else {
            SkDebugf("unknown arg %s\n", *argv);
            usage();
            return -1;
        }
    }

    SkPicture* inPicture = NULL;

    SkFILEStream inStream(inFile.c_str());
    if (inStream.isValid()) {
        inPicture = SkNEW_ARGS(SkPicture, (&inStream));
    }

    if (NULL == inPicture) {
        SkDebugf("Could not read file %s\n", inFile.c_str());
        return -1;
    }

    SkPicture outPicture;
    inPicture->draw(outPicture.beginRecording(inPicture->width(), inPicture->height()));
    outPicture.endRecording();

    SkFILEWStream outStream(outFile.c_str());
    outPicture.serialize(&outStream);

    SkGraphics::Term();

    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return filter_main(argc, (char**) argv);
}
#endif

