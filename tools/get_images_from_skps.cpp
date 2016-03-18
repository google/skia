/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkCommandLineFlags.h"
#include "SkData.h"
#include "SkMD5.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkPixelSerializer.h"
#include "SkStream.h"
#include "SkTHash.h"

DEFINE_string2(skps, s, "skps", "A path to a directory of skps.");
DEFINE_string2(out, o, "img-out", "A path to an output directory.");

static int gKnown;
static int gUnknown;
static const char* gOutputDir;

static SkTHashSet<SkMD5::Digest> gSeen;

struct Sniffer : public SkPixelSerializer {

    void sniff(const void* ptr, size_t len) {
        SkMD5 md5;
        md5.write(ptr, len);
        SkMD5::Digest digest;
        md5.finish(digest);

        if (gSeen.contains(digest)) {
            return;
        }
        gSeen.add(digest);

        SkAutoTUnref<SkData> data(SkData::NewWithoutCopy(ptr, len));
        SkAutoTDelete<SkCodec> codec(SkCodec::NewFromData(data));
        if (!codec) {
            gUnknown++;
            return;
        }
        SkString ext;
        switch (codec->getEncodedFormat()) {
            case SkEncodedFormat::kBMP_SkEncodedFormat:  ext =  "bmp"; break;
            case SkEncodedFormat::kGIF_SkEncodedFormat:  ext =  "gif"; break;
            case SkEncodedFormat::kICO_SkEncodedFormat:  ext =  "ico"; break;
            case SkEncodedFormat::kJPEG_SkEncodedFormat: ext =  "jpg"; break;
            case SkEncodedFormat::kPNG_SkEncodedFormat:  ext =  "png"; break;
            case SkEncodedFormat::kDNG_SkEncodedFormat:  ext =  "dng"; break;
            case SkEncodedFormat::kWBMP_SkEncodedFormat: ext = "wbmp"; break;
            case SkEncodedFormat::kWEBP_SkEncodedFormat: ext = "webp"; break;
            default: gUnknown++; return;
        }

        SkString path;
        path.appendf("%s/%d.%s", gOutputDir, gKnown++, ext.c_str());

        SkFILEWStream file(path.c_str());
        file.write(ptr, len);

        SkDebugf("%s\n", path.c_str());
    }

    bool onUseEncodedData(const void* ptr, size_t len) override {
        this->sniff(ptr, len);
        return true;
    }
    SkData* onEncode(const SkPixmap&) override { return nullptr; }
};


int main(int argc, char** argv) {
    SkCommandLineFlags::SetUsage(
            "Usage: get_images_from_skps -s <dir of skps> -o <dir for output images>\n");

    SkCommandLineFlags::Parse(argc, argv);
    const char* inputs = FLAGS_skps[0];
    gOutputDir = FLAGS_out[0];

    if (!sk_isdir(inputs) || !sk_isdir(gOutputDir)) {
        SkCommandLineFlags::PrintUsage();
        return 1;
    }

    SkOSFile::Iter iter(inputs, "skp");
    for (SkString file; iter.next(&file); ) {
        SkAutoTDelete<SkStream> stream =
                SkStream::NewFromFile(SkOSPath::Join(inputs, file.c_str()).c_str());
        sk_sp<SkPicture> picture(SkPicture::MakeFromStream(stream));

        SkDynamicMemoryWStream scratch;
        Sniffer sniff;
        picture->serialize(&scratch, &sniff);
    }
    SkDebugf("%d known, %d unknown\n", gKnown, gUnknown);

    return 0;
}
