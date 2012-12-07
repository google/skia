/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

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
    SkDebugf("                        [-p pathFile] [-t textureDir] [-h|--help]\n\n");
    SkDebugf("    -i inFile  : file to file.\n");
    SkDebugf("    -o outFile : result of filtering.\n");
    SkDebugf("    --input-dir : process all files in dir with .skp extension.\n");
    SkDebugf("    --output-dir : results of filtering the input dir.\n");
    SkDebugf("    -p pathFile : file in which to place compileable path data.\n");
    SkDebugf("    -t textureDir : directory in which to place textures. (only available w/ single file)\n");
    SkDebugf("    -h|--help  : Show this help message.\n");
}

// SkFilterRecord allows the filter to manipulate the read in SkPicture
class SkFilterRecord : public SkPictureRecord {
public:
    SkFilterRecord(uint32_t recordFlags, SkDevice* device, SkFILEWStream* pathStream)
        : INHERITED(recordFlags, device)
        , fTransSkipped(0)
        , fTransTot(0)
        , fScalesSkipped(0)
        , fScalesTot(0)
        , fPathStream(pathStream) {
    }

    virtual ~SkFilterRecord() {
    }

    virtual bool clipPath(const SkPath& path, SkRegion::Op op, bool doAntiAlias) SK_OVERRIDE {
        if (!path.isRect(NULL) && 4 < path.countPoints()) {
            sk_tools::dump_path(fPathStream, path);
        }
        return INHERITED::clipPath(path, op, doAntiAlias);
    }

    virtual void drawPath(const SkPath& path, const SkPaint& p) SK_OVERRIDE {
        if (!path.isRect(NULL) && 4 < path.countPoints()) {
            sk_tools::dump_path(fPathStream, path);
        }
        INHERITED::drawPath(path, p);
    }

    virtual bool translate(SkScalar dx, SkScalar dy) SK_OVERRIDE {
        ++fTransTot;

#if 0
        if (0 == dx && 0 == dy) {
            ++fTransSkipped;
            return true;
        }
#endif

        return INHERITED::translate(dx, dy);
    }

    virtual bool scale(SkScalar sx, SkScalar sy) SK_OVERRIDE {
        ++fScalesTot;

#if 0
        if (SK_Scalar1 == sx && SK_Scalar1 == sy) {
            ++fScalesSkipped;
            return true;
        }
#endif

        return INHERITED::scale(sx, sy);
    }

    void saveImages(const SkString& path) {
        SkTRefArray<SkBitmap>* bitmaps = fBitmapHeap->extractBitmaps();

        if (NULL != bitmaps) {
            for (int i = 0; i < bitmaps->count(); ++i) {
                SkString filename(path);
                if (!path.endsWith("\\")) {
                    filename.append("\\");
                }
                filename.append("image");
                filename.appendS32(i);
                filename.append(".png");

                SkImageEncoder::EncodeFile(filename.c_str(), (*bitmaps)[i],
                                           SkImageEncoder::kPNG_Type, 0);
            }
        }

        bitmaps->unref();
    }

    void report() {
        SkDebugf("%d Trans skipped (out of %d)\n", fTransSkipped, fTransTot);
        SkDebugf("%d Scales skipped (out of %d)\n", fScalesSkipped, fScalesTot);
    }

protected:
    int fTransSkipped;
    int fTransTot;

    int fScalesSkipped;
    int fScalesTot;

    SkFILEWStream* fPathStream;
private:
    typedef SkPictureRecord INHERITED;
};

// Wrap SkPicture to allow installation of a SkFilterRecord object
class SkFilterPicture : public SkPicture {
public:
    SkFilterPicture(int width, int height, SkPictureRecord* record) {
        fWidth = width;
        fHeight = height;
        fRecord = record;
        SkSafeRef(fRecord);
    }

private:
    typedef SkPicture INHERITED;
};

static bool PNGEncodeBitmapToStream(SkWStream* stream, const SkBitmap& bitmap) {
    return SkImageEncoder::EncodeStream(stream, bitmap, SkImageEncoder::kPNG_Type, 100);
}

int filter_picture(const SkString& inFile, const SkString& outFile, 
                   const SkString& textureDir, SkFILEWStream *pathStream) {
    SkPicture* inPicture = NULL;

    SkFILEStream inStream(inFile.c_str());
    if (inStream.isValid()) {
        inPicture = SkNEW_ARGS(SkPicture, (&inStream, NULL, &SkImageDecoder::DecodeStream));
    }

    if (NULL == inPicture) {
        SkDebugf("Could not read file %s\n", inFile.c_str());
        return -1;
    }

    SkBitmap bm;
    bm.setConfig(SkBitmap::kNo_Config, inPicture->width(), inPicture->height());
    SkAutoTUnref<SkDevice> dev(SkNEW_ARGS(SkDevice, (bm)));

    SkAutoTUnref<SkFilterRecord> filterRecord(SkNEW_ARGS(SkFilterRecord, (0, dev, pathStream)));

    // Playback the read in picture to the SkFilterRecorder to allow filtering
    filterRecord->beginRecording();
    inPicture->draw(filterRecord);
    filterRecord->endRecording();

    filterRecord->report();

    if (!outFile.isEmpty()) {
        SkFilterPicture outPicture(inPicture->width(), inPicture->height(), filterRecord);
        SkFILEWStream outStream(outFile.c_str());

        outPicture.serialize(&outStream);
    }

    if (!textureDir.isEmpty()) {
        filterRecord->saveImages(textureDir);
    }

    return 0;
}

// This function is not marked as 'static' so it can be referenced externally
// in the iOS build.
int tool_main(int argc, char** argv) {
    SkGraphics::Init();

    if (argc < 3) {
        usage();
        return -1;
    }

    SkString inFile, outFile, inDir, outDir, textureDir, pathFile;

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
        } else if (strcmp(*argv, "-p") == 0) {
            argv++;
            if (argv < stop && **argv) {
                pathFile.set(*argv);
            } else {
                SkDebugf("missing arg for -p\n");
                usage();
                return -1;
            }
        } else if (strcmp(*argv, "-t") == 0) {
            argv++;
            if (argv < stop && **argv) {
                textureDir.set(*argv);
            } else {
                SkDebugf("missing arg for -t\n");
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

    if(!inDir.isEmpty() && !textureDir.isEmpty()) {
        SkDebugf("ERROR: The textureDir option is not permitted when passing an input directory.\n");
        usage();
        return -1;
    }

    SkFILEWStream *pathStream = NULL;

    if (!pathFile.isEmpty()) {
        pathStream = new SkFILEWStream(pathFile.c_str());
        if (!pathStream->isValid()) {
            SkDebugf("Could open path file %s\n", pathFile.c_str());
            delete pathStream;
            return -1;
        }

        sk_tools::dump_path_prefix(pathStream);
    }

    SkOSFile::Iter iter(inDir.c_str(), "skp");
    int failures = 0;
    SkString inputFilename, outputFilename;
    if (iter.next(&inputFilename)) {

        do {
            sk_tools::make_filepath(&inFile, inDir, inputFilename);
            if (!outDir.isEmpty()) {
                sk_tools::make_filepath(&outFile, outDir, inputFilename);
            }
            SkDebugf("Executing %s\n", inputFilename.c_str());
            filter_picture(inFile, outFile, textureDir, pathStream);
        } while(iter.next(&inputFilename));

    } else if (!inFile.isEmpty()) {
        filter_picture(inFile, outFile, textureDir, pathStream);
    } else {
        usage();
        if (NULL != pathStream) {
            delete pathStream;
            pathStream = NULL;
        }
        return -1;
    }

    if (NULL != pathStream) {
        sk_tools::dump_path_suffix(pathStream);
        delete pathStream;
        pathStream = NULL;
    }

    SkGraphics::Term();
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
