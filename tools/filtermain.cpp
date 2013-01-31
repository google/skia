/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDebugCanvas.h"
#include "SkDevice.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkPicturePlayback.h"
#include "SkPictureRecord.h"
#include "SkStream.h"
#include "picture_utils.h"
#include "path_utils.h"

static void usage() {
    SkDebugf("Usage: filter -i inFile [-o outFile] [--input-dir path] [--output-dir path]\n");
    SkDebugf("                        [-h|--help]\n\n");
    SkDebugf("    -i inFile  : file to file.\n");
    SkDebugf("    -o outFile : result of filtering.\n");
    SkDebugf("    --input-dir : process all files in dir with .skp extension.\n");
    SkDebugf("    --output-dir : results of filtering the input dir.\n");
    SkDebugf("    -h|--help  : Show this help message.\n");
}

// Is the supplied paint simply a color?
static bool is_simple(const SkPaint& p) {
    return NULL == p.getPathEffect() &&
           NULL == p.getShader() &&
           NULL == p.getXfermode() &&
           NULL == p.getMaskFilter() &&
           NULL == p.getColorFilter() &&
           NULL == p.getRasterizer() &&
           NULL == p.getLooper() &&
           NULL == p.getImageFilter();
}

static int filter_picture(const SkString& inFile, const SkString& outFile) {
    SkPicture* inPicture = NULL;

    SkFILEStream inStream(inFile.c_str());
    if (inStream.isValid()) {
        inPicture = SkNEW_ARGS(SkPicture, (&inStream, NULL, &SkImageDecoder::DecodeStream));
    }

    if (NULL == inPicture) {
        SkDebugf("Could not read file %s\n", inFile.c_str());
        return -1;
    }

    SkDebugCanvas debugCanvas(inPicture->width(), inPicture->height());
    debugCanvas.setBounds(inPicture->width(), inPicture->height());
    inPicture->draw(&debugCanvas);

    const SkTDArray<SkDrawCommand*>& commands = debugCanvas.getDrawCommands();

    for (int i = 0; i < commands.count(); ++i) {
        // Check for:
        //    SAVE_LAYER
        //      DRAW_BITMAP_RECT_TO_RECT
        //    RESTORE
        // where the saveLayer's color can be moved into the drawBitmapRect
        if (SAVE_LAYER == commands[i]->getType() && commands.count() > i+2) {
            if (DRAW_BITMAP_RECT_TO_RECT == commands[i+1]->getType() &&
                RESTORE == commands[i+2]->getType()) {
                SaveLayer* sl = (SaveLayer*) commands[i];
                DrawBitmapRect* dbmr = (DrawBitmapRect*) commands[i+1];

                const SkPaint* p0 = sl->paint();
                SkPaint* p1 = dbmr->paint();

                if (NULL == p0) {
                    commands[i]->setVisible(false);
                    commands[i+2]->setVisible(false);
                } else if (NULL == p1) {
                    commands[i]->setVisible(false);
                    dbmr->setPaint(*p0);
                    commands[i+2]->setVisible(false);
                } else if (is_simple(*p0) &&
                           (SkColorGetR(p0->getColor()) == SkColorGetR(p1->getColor())) &&
                           (SkColorGetG(p0->getColor()) == SkColorGetG(p1->getColor())) &&
                           (SkColorGetB(p0->getColor()) == SkColorGetB(p1->getColor()))) {
                    commands[i]->setVisible(false);
                    SkColor newColor = SkColorSetA(p1->getColor(),
                                                   SkColorGetA(p0->getColor()));
                    p1->setColor(newColor);
                    commands[i+2]->setVisible(false);
                }
            }
        }
    }

    if (!outFile.isEmpty()) {
        SkPicture outPicture;

        SkCanvas* canvas = outPicture.beginRecording(inPicture->width(), inPicture->height());
        debugCanvas.draw(canvas);
        outPicture.endRecording();

        SkFILEWStream outStream(outFile.c_str());

        outPicture.serialize(&outStream);
    }

    return 0;
}

// This function is not marked as 'static' so it can be referenced externally
// in the iOS build.
int tool_main(int argc, char** argv); // suppress a warning on mac

int tool_main(int argc, char** argv) {
    SkGraphics::Init();

    if (argc < 3) {
        usage();
        return -1;
    }

    SkString inFile, outFile, inDir, outDir;

    char* const* stop = argv + argc;
    for (++argv; argv < stop; ++argv) {
        if (strcmp(*argv, "-i") == 0) {
            argv++;
            if (argv < stop && **argv) {
                inFile.set(*argv);
            } else {
                SkDebugf("missing arg for -i\n");
                usage();
                return -1;
            }
        } else if (strcmp(*argv, "--input-dir") == 0) {
            argv++;
            if (argv < stop && **argv) {
                inDir.set(*argv);
            } else {
                SkDebugf("missing arg for --input-dir\n");
                usage();
                return -1;
            }
        } else if (strcmp(*argv, "--output-dir") == 0) {
            argv++;
            if (argv < stop && **argv) {
                outDir.set(*argv);
            } else {
                SkDebugf("missing arg for --output-dir\n");
                usage();
                return -1;
            }
        } else if (strcmp(*argv, "-o") == 0) {
            argv++;
            if (argv < stop && **argv) {
                outFile.set(*argv);
            } else {
                SkDebugf("missing arg for -o\n");
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

    SkOSFile::Iter iter(inDir.c_str(), "skp");

    SkString inputFilename, outputFilename;
    if (iter.next(&inputFilename)) {

        do {
            sk_tools::make_filepath(&inFile, inDir, inputFilename);
            if (!outDir.isEmpty()) {
                sk_tools::make_filepath(&outFile, outDir, inputFilename);
            }
            SkDebugf("Executing %s\n", inputFilename.c_str());
            filter_picture(inFile, outFile);
        } while(iter.next(&inputFilename));

    } else if (!inFile.isEmpty()) {
        filter_picture(inFile, outFile);
    } else {
        usage();
        return -1;
    }

    SkGraphics::Term();
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
