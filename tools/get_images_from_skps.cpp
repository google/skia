/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkCodec.h"
#include "SkCommandLineFlags.h"
#include "SkData.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkStream.h"

DEFINE_string2(skps, s, "", "A path to a directory of skps.");
DEFINE_string2(out, o, "", "A path to an output directory.");

static int gCtr = 0;
static int gSuccessCtr = 0;
static int gUnknownCtr = 0;
static int gFailureCtr = 0;
static const char* gOutputDir;

void setup_output_dirs() {
    const char* exts[] = { "jpg", "png", "gif", "webp", "bmp", "wbmp", "ico", "dng", "unknown" };
    for (const char* ext : exts) {
        sk_mkdir(SkOSPath::Join(gOutputDir, ext).c_str());
    }
}

bool store_encoded_to_file(const void* encoded, size_t length, SkBitmap* bitmap) {
    // Silence warnings about empty bitmaps.
    bitmap->allocN32Pixels(1, 1, true);

    SkString path;
    SkAutoTUnref<SkData> data(SkData::NewWithoutCopy(encoded, length));
    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromData(data));
    if (codec) {
        switch (codec->getEncodedFormat()) {
            case SkEncodedFormat::kJPEG_SkEncodedFormat:
                path = SkOSPath::Join(SkOSPath::Join(gOutputDir, "jpg").c_str(), "");
                path.appendS32(gCtr++);
                path.append(".jpg");
                break;
            case SkEncodedFormat::kPNG_SkEncodedFormat:
                path = SkOSPath::Join(SkOSPath::Join(gOutputDir, "png").c_str(), "");
                path.appendS32(gCtr++);
                path.append(".png");
                break;
            case SkEncodedFormat::kGIF_SkEncodedFormat:
                path = SkOSPath::Join(SkOSPath::Join(gOutputDir, "gif").c_str(), "");
                path.appendS32(gCtr++);
                path.append(".gif");
                break;
            case SkEncodedFormat::kWEBP_SkEncodedFormat:
                path = SkOSPath::Join(SkOSPath::Join(gOutputDir, "webp").c_str(), "");
                path.appendS32(gCtr++);
                path.append(".webp");
                break;
            case SkEncodedFormat::kBMP_SkEncodedFormat:
                path = SkOSPath::Join(SkOSPath::Join(gOutputDir, "bmp").c_str(), "");
                path.appendS32(gCtr++);
                path.append(".bmp");
                break;
            case SkEncodedFormat::kWBMP_SkEncodedFormat:
                path = SkOSPath::Join(SkOSPath::Join(gOutputDir, "wbmp").c_str(), "");
                path.appendS32(gCtr++);
                path.append(".wbmp");
                break;
            case SkEncodedFormat::kICO_SkEncodedFormat:
                path = SkOSPath::Join(SkOSPath::Join(gOutputDir, "ico").c_str(), "");
                path.appendS32(gCtr++);
                path.append(".ico");
                break;
            case SkEncodedFormat::kRAW_SkEncodedFormat:
                path = SkOSPath::Join(SkOSPath::Join(gOutputDir, "dng").c_str(), "");
                path.appendS32(gCtr++);
                path.append(".dng");
                break;
            default:
                path = SkOSPath::Join(gOutputDir, "unknown");
                path.appendS32(gUnknownCtr++);
                break;
        }
    } else {
        path = SkOSPath::Join(gOutputDir, "unknown");
        path.appendS32(gUnknownCtr++);
    }

    FILE* file = sk_fopen(path.c_str(), kWrite_SkFILE_Flag);
    if (file) {
        sk_fwrite(encoded, length, file);
        sk_fclose(file);
        gSuccessCtr++;
        return true;
    }

    gFailureCtr++;
    SkDebugf("Could not open %s\n", path.c_str());
    return false;
}

int main(int argc, char** argv) {
    SkCommandLineFlags::SetUsage(
            "Usage: get_images_from_skps -s <dir of skps> -o <dir for output images>\n");

    SkCommandLineFlags::Parse(argc, argv);
    if (FLAGS_skps.isEmpty() || FLAGS_out.isEmpty()) {
        SkCommandLineFlags::PrintUsage();
        return 1;
    }

    const char* inputs = FLAGS_skps[0];
    gOutputDir = FLAGS_out[0];
    if (!sk_isdir(inputs) || !sk_isdir(gOutputDir)) {
        SkCommandLineFlags::PrintUsage();
        return 1;
    }

    setup_output_dirs();
    SkOSFile::Iter iter(inputs, "skp");
    for (SkString file; iter.next(&file); ) {
        SkAutoTDelete<SkStream> stream =
                SkStream::NewFromFile(SkOSPath::Join(inputs, file.c_str()).c_str());

        // Rather than passing in a function that actually decodes the encoded data,
        // we pass in a function that saves the encoded data to a file.
        SkAutoTUnref<SkPicture> picture(SkPicture::CreateFromStream(stream, store_encoded_to_file));

        SkCanvas canvas;
        canvas.drawPicture(picture);
    }

    SkDebugf("Successfully saved %d recognized images and %d unrecognized images\n", gSuccessCtr,
            gUnknownCtr);
    SkDebugf("Failed to write %d images\n", gFailureCtr);
    return 0;
}
