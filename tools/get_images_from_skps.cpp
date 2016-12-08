/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkColorSpace.h"
#include "SkCommandLineFlags.h"
#include "SkData.h"
#include "SkJSONCPP.h"
#include "SkMD5.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkPicture.h"
#include "SkPixelSerializer.h"
#include "SkStream.h"
#include "SkTHash.h"


#include <iostream>
#include <map>

DEFINE_string2(skps, s, "skps", "A path to a directory of skps.");
DEFINE_string2(out, o, "img-out", "A path to an output directory.");
DEFINE_bool(testDecode, false, "Indicates if we want to test that the images decode successfully.");
DEFINE_bool(writeImages, true,
            "Indicates if we want to write out supported/decoded images.");
DEFINE_bool(writeFailedImages, false,
            "Indicates if we want to write out unsupported/failed to decode images.");
DEFINE_bool(testICCSupport, false,
            "Indicates if we want to test that the images with ICC profiles are supported");
DEFINE_string2(failuresJsonPath, j, "",
               "Dump SKP and count of unknown images to the specified JSON file. Will not be "
               "written anywhere if empty.");

static int gKnown;
static const char* gOutputDir;
static std::map<std::string, unsigned int> gSkpToUnknownCount = {};
static std::map<std::string, unsigned int> gSkpToUnsupportedCount;

static SkTHashSet<SkMD5::Digest> gSeen;

struct Sniffer : public SkPixelSerializer {

    std::string skpName;

    Sniffer(std::string name) {
        skpName = name;
    }

    void sniff(const void* ptr, size_t len) {
        SkMD5 md5;
        md5.write(ptr, len);
        SkMD5::Digest digest;
        md5.finish(digest);

        if (gSeen.contains(digest)) {
            return;
        }
        gSeen.add(digest);

        sk_sp<SkData> data(SkData::MakeWithoutCopy(ptr, len));
        std::unique_ptr<SkCodec> codec(SkCodec::NewFromData(data));
        if (!codec) {
            // FIXME: This code is currently unreachable because we create an empty generator when
            //        we fail to create a codec.
            SkDebugf("Codec could not be created for %s\n", skpName.c_str());
            gSkpToUnknownCount[skpName]++;
            return;
        }
        SkString ext;
        switch (codec->getEncodedFormat()) {
            case SkEncodedImageFormat::kBMP:  ext =  "bmp"; break;
            case SkEncodedImageFormat::kGIF:  ext =  "gif"; break;
            case SkEncodedImageFormat::kICO:  ext =  "ico"; break;
            case SkEncodedImageFormat::kJPEG: ext =  "jpg"; break;
            case SkEncodedImageFormat::kPNG:  ext =  "png"; break;
            case SkEncodedImageFormat::kDNG:  ext =  "dng"; break;
            case SkEncodedImageFormat::kWBMP: ext = "wbmp"; break;
            case SkEncodedImageFormat::kWEBP: ext = "webp"; break;
            default:
                // This should be unreachable because we cannot create a codec if we do not know
                // the image type.
                SkASSERT(false);
        }

        auto writeImage = [&] {
            SkString path;
            path.appendf("%s/%d.%s", gOutputDir, gKnown, ext.c_str());

            SkFILEWStream file(path.c_str());
            file.write(ptr, len);

            SkDebugf("%s\n", path.c_str());
        };


        if (FLAGS_testDecode) {
            SkBitmap bitmap;
            SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType);
            bitmap.allocPixels(info);
            const SkCodec::Result result = codec->getPixels(
                info, bitmap.getPixels(),  bitmap.rowBytes());
            if (SkCodec::kIncompleteInput != result && SkCodec::kSuccess != result) {
                SkDebugf("Decoding failed for %s\n", skpName.c_str());
                gSkpToUnknownCount[skpName]++;
                if (FLAGS_writeFailedImages) {
                    writeImage();
                }
                return;
            }
        }

#ifdef SK_DEBUG
        if (FLAGS_testICCSupport) {
            if (codec->fUnsupportedICC) {
                SkDebugf("Color correction failed for %s\n", skpName.c_str());
                gSkpToUnsupportedCount[skpName]++;
                if (FLAGS_writeFailedImages) {
                    writeImage();
                }
                return;
            }
        }
#endif

        if (FLAGS_writeImages) {
            writeImage();
        }


        gKnown++;
    }

    bool onUseEncodedData(const void* ptr, size_t len) override {
        this->sniff(ptr, len);
        return true;
    }
    SkData* onEncode(const SkPixmap&) override { return nullptr; }
};


int main(int argc, char** argv) {
    SkCommandLineFlags::SetUsage(
            "Usage: get_images_from_skps -s <dir of skps> -o <dir for output images> --testDecode "
            "-j <output JSON path> --testICCSupport --writeImages, --writeFailedImages\n");

    SkCommandLineFlags::Parse(argc, argv);
    const char* inputs = FLAGS_skps[0];
    gOutputDir = FLAGS_out[0];

    if (!sk_isdir(inputs) || !sk_isdir(gOutputDir)) {
        SkCommandLineFlags::PrintUsage();
        return 1;
    }
#ifndef SK_DEBUG
    if (FLAGS_testICCSupport) {
        std::cerr << "--testICCSupport unavailable outside of SK_DEBUG builds" << std::endl;
        return 1;
    }
#endif

    SkOSFile::Iter iter(inputs, "skp");
    for (SkString file; iter.next(&file); ) {
        std::unique_ptr<SkStream> stream =
                SkStream::MakeFromFile(SkOSPath::Join(inputs, file.c_str()).c_str());
        sk_sp<SkPicture> picture(SkPicture::MakeFromStream(stream.get()));

        SkDynamicMemoryWStream scratch;
        Sniffer sniff(file.c_str());
        picture->serialize(&scratch, &sniff);
    }
    /**
     JSON results are written out in the following format:
     {
       "failures": {
         "skp1": 12,
         "skp4": 2,
         ...
       },
       "unsupported": {
        "skp9": 13,
        "skp17": 3,
        ...
       }
       "totalFailures": 32,
       "totalUnsupported": 9,
       "totalSuccesses": 21,
     }
     */
    Json::Value fRoot;
    int totalFailures = 0;
    for(auto it = gSkpToUnknownCount.cbegin(); it != gSkpToUnknownCount.cend(); ++it)
    {
        SkDebugf("%s %d\n", it->first.c_str(), it->second);
        totalFailures += it->second;
        fRoot["failures"][it->first.c_str()] = it->second;
    }
    fRoot["totalFailures"] = totalFailures;
    int totalUnsupported = 0;
#ifdef SK_DEBUG
    for (const auto& unsupported : gSkpToUnsupportedCount) {
        SkDebugf("%s %d\n", unsupported.first.c_str(), unsupported.second);
        totalUnsupported += unsupported.second;
        fRoot["unsupported"][unsupported.first] = unsupported.second;
    }
    fRoot["totalUnsupported"] = totalUnsupported;
#endif
    fRoot["totalSuccesses"] = gKnown;
    SkDebugf("%d known, %d failures, %d unsupported\n", gKnown, totalFailures, totalUnsupported);
    if (totalFailures > 0 || totalUnsupported > 0) {
        if (!FLAGS_failuresJsonPath.isEmpty()) {
            SkDebugf("Writing failures to %s\n", FLAGS_failuresJsonPath[0]);
            SkFILEWStream stream(FLAGS_failuresJsonPath[0]);
            stream.writeText(Json::StyledWriter().write(fRoot).c_str());
            stream.flush();
        }
        return -1;
    }
    return 0;
}
