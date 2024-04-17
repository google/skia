/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPicture.h"
#include "include/core/SkStream.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkPictureData.h"
#include "src/core/SkPicturePriv.h"
#include "tools/flags/CommandLineFlags.h"

static DEFINE_string2(input, i, "", "skp on which to report");
static DEFINE_bool2(version, v, true, "version");
static DEFINE_bool2(cullRect, c, true, "cullRect");
static DEFINE_bool2(flags, f, true, "flags");
static DEFINE_bool2(tags, t, true, "tags");
static DEFINE_bool2(quiet, q, false, "quiet");

// This tool can print simple information about an SKP but its main use
// is just to check if an SKP has been truncated during the recording
// process.
// return codes:
static const int kSuccess = 0;
static const int kTruncatedFile = 1;
static const int kNotAnSKP = 2;
static const int kInvalidTag = 3;
static const int kMissingInput = 4;
static const int kIOError = 5;

int main(int argc, char** argv) {
    CommandLineFlags::SetUsage("Prints information about an skp file");
    CommandLineFlags::Parse(argc, argv);

    if (FLAGS_input.size() != 1) {
        if (!FLAGS_quiet) {
            SkDebugf("Missing input file\n");
        }
        return kMissingInput;
    }

    SkFILEStream stream(FLAGS_input[0]);
    if (!stream.isValid()) {
        if (!FLAGS_quiet) {
            SkDebugf("Couldn't open file\n");
        }
        return kIOError;
    }

    size_t totStreamSize = stream.getLength();

    SkPictInfo info;
    if (!SkPicture_StreamIsSKP(&stream, &info)) {
        SkDebugf("Unsupported version %u\n", info.getVersion());
        return kNotAnSKP;
    }

    if (FLAGS_version && !FLAGS_quiet) {
        SkDebugf("Version: %u\n", info.getVersion());
    }
    if (FLAGS_cullRect && !FLAGS_quiet) {
        SkDebugf("Cull Rect: %f,%f,%f,%f\n",
                 info.fCullRect.fLeft, info.fCullRect.fTop,
                 info.fCullRect.fRight, info.fCullRect.fBottom);
    }

    bool hasData;
    if (!stream.readBool(&hasData)) { return kTruncatedFile; }
    if (!hasData) {
        // If we read true there's a picture playback object flattened
        // in the file; if false, there isn't a playback, so we're done
        // reading the file.
        return kSuccess;
    }

    for (;;) {
        uint32_t tag;
        if (!stream.readU32(&tag)) { return kTruncatedFile; }
        if (SK_PICT_EOF_TAG == tag) {
            break;
        }

        uint32_t chunkSize;
        if (!stream.readU32(&chunkSize)) { return kTruncatedFile; }
        size_t curPos = stream.getPosition();

        // "move" doesn't error out when seeking beyond the end of file
        // so we need a preemptive check here.
        if (curPos+chunkSize > totStreamSize) {
            if (!FLAGS_quiet) {
                SkDebugf("truncated file\n");
            }
            return kTruncatedFile;
        }

        // Not all the tags store the chunk size (in bytes). Three
        // of them store tag-specific size information (e.g., number of
        // fonts) instead. This forces us to early exit when those
        // chunks are encountered.
        switch (tag) {
        case SK_PICT_READER_TAG:
            if (FLAGS_tags && !FLAGS_quiet) {
                SkDebugf("SK_PICT_READER_TAG %u\n", chunkSize);
            }
            break;
        case SK_PICT_FACTORY_TAG:
            if (FLAGS_tags && !FLAGS_quiet) {
                SkDebugf("SK_PICT_FACTORY_TAG %u\n", chunkSize);
            }
            break;
        case SK_PICT_TYPEFACE_TAG: {
            if (FLAGS_tags && !FLAGS_quiet) {
                SkDebugf("SK_PICT_TYPEFACE_TAG %u\n", chunkSize);
            }

            const int count = SkToInt(chunkSize);
            for (int i = 0; i < count; i++) {
                SkFontDescriptor desc;
                if (!SkFontDescriptor::Deserialize(&stream, &desc)) {
                    if (!FLAGS_quiet) {
                        SkDebugf("File corruption in SkFontDescriptor\n");
                    }
                    return kInvalidTag;
                }
            }

            // clear this since we've consumed all the typefaces
            chunkSize = 0;
            break;
        }
        case SK_PICT_PICTURE_TAG:
            if (FLAGS_tags && !FLAGS_quiet) {
                SkDebugf("SK_PICT_PICTURE_TAG %u\n", chunkSize);
                SkDebugf("Exiting early due to format limitations\n");
            }
            return kSuccess;       // TODO: need to store size in bytes
        case SK_PICT_BUFFER_SIZE_TAG:
            if (FLAGS_tags && !FLAGS_quiet) {
                SkDebugf("SK_PICT_BUFFER_SIZE_TAG %u\n", chunkSize);
            }
            break;
        default:
            if (!FLAGS_quiet) {
                SkDebugf("Unknown tag %u\n", chunkSize);
            }
            return kInvalidTag;
        }

        if (!stream.move(chunkSize)) {
            if (!FLAGS_quiet) {
                SkDebugf("seek error\n");
            }
            return kTruncatedFile;
        }
    }

    return kSuccess;
}
