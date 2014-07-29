/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkCommandLineFlags.h"
#include "SkData.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkRandom.h"
#include "SkStream.h"
#include "SkTArray.h"
#include "SkTemplates.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

DEFINE_string2(readPath, r, "", "Folder(s) and files to decode images. Required.");

struct Format {
    SkImageEncoder::Type    fType;
    SkImageDecoder::Format  fFormat;
    const char*             fSuffix;
};

/*
static const Format gFormats[] = {
    { SkImageEncoder::kBMP_Type, SkImageDecoder::kBMP_Format, ".bmp" },
    { SkImageEncoder::kGIF_Type, SkImageDecoder::kGIF_Format, ".gif" },
    { SkImageEncoder::kICO_Type, SkImageDecoder::kICO_Format, ".ico" },
    { SkImageEncoder::kJPEG_Type, SkImageDecoder::kJPEG_Format, ".jpg" },
    { SkImageEncoder::kPNG_Type, SkImageDecoder::kPNG_Format, ".png" },
    { SkImageEncoder::kWBMP_Type, SkImageDecoder::kWBMP_Format, ".wbmp" },
    { SkImageEncoder::kWEBP_Type, SkImageDecoder::kWEBP_Format, ".webp" }
};
*/

static SkISize opaqueSize(const SkBitmap& bm) {
    int width = 1;
    int height = 1;
    for (int y = 0 ; y < bm.height(); y++) {
        for (int x = 0 ; x < bm.width(); x++) {
            SkColor color = bm.getColor(x, y);
            if (SkColorGetA(color) != 0) {
                height = y + 1;
                width = width > (x + 1) ? width : x + 1;
            }
        }
    }

    return SkISize::Make(width, height);
}

static void setup_bitmap(SkBitmap* bitmap, int width, int height) {
    bitmap->allocN32Pixels(width, height);
}


static bool write_bitmap(const char outName[], const SkBitmap& bm) {
    SkISize size = opaqueSize(bm);
    SkBitmap dst;
    setup_bitmap(&dst, size.width(), size.height());

    for (int y = 0 ; y < dst.height(); y++) {
        for (int x = 0 ; x < dst.width(); x++) {
            SkColor color = bm.getColor(x, y);
            if (SkColorGetA(color) != 0xff) {
                int a = SkColorGetA(color);
                int r = SkColorGetR(color);
                int g = SkColorGetG(color);
                int b = SkColorGetB(color);
                if (a == 0) {
                    r = g = b = 0;
                } else {
                    r = (r * a) / 255;
                    g = (g * a) / 255;
                    b = (b * a) / 255;
                    a = 255;
                }
                color = SkColorSetARGB((U8CPU)a, (U8CPU)r, (U8CPU)g, (U8CPU)b);
            }
            *dst.getAddr32(x, y) = color;
        }
    }

    return SkImageEncoder::EncodeFile(outName, dst, SkImageEncoder::kPNG_Type, 100);
}

static void decodeFileAndWrite(const char srcPath[]) {
    SkBitmap bitmap;
    SkFILEStream stream(srcPath);
    if (!stream.isValid()) {
        return;
    }

    SkImageDecoder* codec = SkImageDecoder::Factory(&stream);
    if (NULL == codec) {
        return;
    }

    SkAutoTDelete<SkImageDecoder> ad(codec);

    stream.rewind();

    if (!codec->decode(&stream, &bitmap, kN32_SkColorType, SkImageDecoder::kDecodePixels_Mode)) {
        return;
    }

    write_bitmap(srcPath, bitmap);
}

/**
 *  Return true if the filename represents an image.
 */
static bool is_image_file(const char* filename) {
    const char* gImageExtensions[] = {
        ".png", ".PNG", ".jpg", ".JPG", ".jpeg", ".JPEG", ".bmp", ".BMP",
        ".webp", ".WEBP", ".ico", ".ICO", ".wbmp", ".WBMP", ".gif", ".GIF"
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(gImageExtensions); ++i) {
        if (SkStrEndsWith(filename, gImageExtensions[i])) {
            return true;
        }
    }
    return false;
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

    for (int i = 0; i < FLAGS_readPath.count(); i++) {
        const char* readPath = FLAGS_readPath[i];
        if (strlen(readPath) < 1) {
            break;
        }
        if (sk_isdir(readPath)) {
            const char* dir = readPath;
            SkOSFile::Iter iter(dir);
            SkString filename;
            while (iter.next(&filename)) {
                if (!is_image_file(filename.c_str())) {
                    continue;
                }
                SkString fullname = SkOSPath::Join(dir, filename.c_str());
                decodeFileAndWrite(fullname.c_str());
            }
        } else if (sk_exists(readPath) && is_image_file(readPath)) {
            decodeFileAndWrite(readPath);
        }
    }

    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
