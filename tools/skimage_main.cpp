/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCommandLineFlags.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkStream.h"
#include "SkTArray.h"
#include "SkTemplates.h"


DEFINE_string2(readPath, r, "", "Folder(s) and files to decode images. Required.");
DEFINE_string2(writePath, w, "",  "Write rendered images into this directory.");

// Store the names of the filenames to report later which ones failed, succeeded, and were
// invalid.
static SkTArray<SkString, false> invalids;
static SkTArray<SkString, false> nocodecs;
static SkTArray<SkString, false> failures;
static SkTArray<SkString, false> successes;

static bool decodeFile(SkBitmap* bitmap, const char srcPath[]) {
    SkFILEStream stream(srcPath);
    if (!stream.isValid()) {
        invalids.push_back().set(srcPath);
        return false;
    }

    SkImageDecoder* codec = SkImageDecoder::Factory(&stream);
    if (NULL == codec) {
        nocodecs.push_back().set(srcPath);
        return false;
    }

    SkAutoTDelete<SkImageDecoder> ad(codec);

    stream.rewind();
    if (!codec->decode(&stream, bitmap, SkBitmap::kARGB_8888_Config,
                       SkImageDecoder::kDecodePixels_Mode)) {
        failures.push_back().set(srcPath);
        return false;
    }

    successes.push_back().printf("%s [%d %d]", srcPath, bitmap->width(), bitmap->height());
    return true;
}

///////////////////////////////////////////////////////////////////////////////

static void make_outname(SkString* dst, const char outDir[], const char src[]) {
    dst->set(outDir);
    const char* start = strrchr(src, '/');
    if (start) {
        start += 1; // skip the actual last '/'
    } else {
        start = src;
    }
    dst->append(start);
    if (!dst->endsWith(".png")) {
        const char* cstyleDst = dst->c_str();
        const char* dot = strrchr(cstyleDst, '.');
        if (dot != NULL) {
            int32_t index = SkToS32(dot - cstyleDst);
            dst->remove(index, dst->size() - index);
        }
        dst->append(".png");
    }
}

// If strings is not empty, print title, followed by each string on its own line starting
// with a tab.
static void print_strings(const char* title, const SkTArray<SkString, false>& strings) {
    if (strings.count() > 0) {
        SkDebugf("%s:\n", title);
        for (int i = 0; i < strings.count(); i++) {
            SkDebugf("\t%s\n", strings[i].c_str());
        }
        SkDebugf("\n");
    }
}

static void decodeFileAndWrite(const char filePath[], const SkString* writePath) {
    SkBitmap bitmap;
    if (decodeFile(&bitmap, filePath)) {
        if (writePath != NULL) {
            SkString outPath;
            make_outname(&outPath, writePath->c_str(), filePath);
            successes.push_back().appendf("\twrote %s", outPath.c_str());
            SkImageEncoder::EncodeFile(outPath.c_str(), bitmap, SkImageEncoder::kPNG_Type, 100);
        }
    }
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkCommandLineFlags::SetUsage("Decode files, and optionally write the results to files.");
    SkCommandLineFlags::Parse(argc, argv);

    if (FLAGS_readPath.count() < 1) {
        SkDebugf("Folder(s) or image(s) to decode are required.\n");
        return -1;
    }


    SkAutoGraphics ag;

    SkString outDir;
    SkString* outDirPtr;

    if (FLAGS_writePath.count() == 1) {
        outDir.set(FLAGS_writePath[0]);
        if (outDir.c_str()[outDir.size() - 1] != '/') {
            outDir.append("/");
        }
        outDirPtr = &outDir;
    } else {
        outDirPtr = NULL;
    }

    for (int i = 0; i < FLAGS_readPath.count(); i++) {
        if (strlen(FLAGS_readPath[i]) < 1) {
            break;
        }
        SkOSFile::Iter iter(FLAGS_readPath[i]);
        SkString filename;
        if (iter.next(&filename)) {
            SkString directory(FLAGS_readPath[i]);
            if (directory[directory.size() - 1] != '/') {
                directory.append("/");
            }
            do {
                SkString fullname(directory);
                fullname.append(filename);
                decodeFileAndWrite(fullname.c_str(), outDirPtr);
            } while (iter.next(&filename));
        } else {
            decodeFileAndWrite(FLAGS_readPath[i], outDirPtr);
        }
    }

    // Add some space, since codecs may print warnings without newline.
    SkDebugf("\n\n");

    print_strings("Invalid files", invalids);
    print_strings("Missing codec", nocodecs);
    print_strings("Failed to decode", failures);
    print_strings("Decoded", successes);

    return 0;
}

void forceLinking();

void forceLinking() {
    SkDEBUGCODE(SkImageDecoder *creator = ) CreateJPEGImageDecoder();
    SkASSERT(creator);
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
