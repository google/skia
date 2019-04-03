/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CommandLineFlags.h"
#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkColorSpace.h"
#include "SkData.h"
#include "SkJSONWriter.h"
#include "SkMD5.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkPicture.h"
#include "SkSerialProcs.h"
#include "SkStream.h"
#include "SkTHash.h"

#include <iostream>
#include <map>

static DEFINE_string2(skps, s, "skps", "A path to a directory of skps or a single skp.");
static DEFINE_string2(out, o, "img-out", "A path to an output directory.");
static DEFINE_bool(testDecode, false,
                   "Indicates if we want to test that the images decode successfully.");
static DEFINE_bool(writeImages, true,
                   "Indicates if we want to write out supported/decoded images.");
static DEFINE_bool(writeFailedImages, false,
                   "Indicates if we want to write out unsupported/failed to decode images.");
static DEFINE_string2(failuresJsonPath, j, "",
               "Dump SKP and count of unknown images to the specified JSON file. Will not be "
               "written anywhere if empty.");

static int gKnown;
static const char* gOutputDir;
static std::map<std::string, unsigned int> gSkpToUnknownCount = {};
static std::map<std::string, unsigned int> gSkpToUnsupportedCount;

static SkTHashSet<SkMD5::Digest> gSeen;

struct Sniffer {

    std::string skpName;

    Sniffer(std::string name) {
        skpName = name;
    }

    void sniff(const void* ptr, size_t len) {
        SkMD5 md5;
        md5.write(ptr, len);
        SkMD5::Digest digest = md5.finish();

        if (gSeen.contains(digest)) {
            return;
        }
        gSeen.add(digest);

        sk_sp<SkData> data(SkData::MakeWithoutCopy(ptr, len));
        std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(data);
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

        auto writeImage = [&] (const char* name, int num) {
            SkString path;
            path.appendf("%s/%s%d.%s", gOutputDir, name, num, ext.c_str());

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
            switch (result) {
                case SkCodec::kSuccess:
                case SkCodec::kIncompleteInput:
                case SkCodec::kErrorInInput:
                    break;
                default:
                    SkDebugf("Decoding failed for %s\n", skpName.c_str());
                    if (FLAGS_writeFailedImages) {
                        writeImage("unknown", gSkpToUnknownCount[skpName]);
                    }
                    gSkpToUnknownCount[skpName]++;
                    return;
            }
        }

        if (FLAGS_writeImages) {
            writeImage("", gKnown);
        }

        gKnown++;
    }
};

static bool get_images_from_file(const SkString& file) {
    Sniffer sniff(file.c_str());
    auto stream = SkStream::MakeFromFile(file.c_str());

    SkDeserialProcs procs;
    procs.fImageProc = [](const void* data, size_t size, void* ctx) -> sk_sp<SkImage> {
        ((Sniffer*)ctx)->sniff(data, size);
        return nullptr;
    };
    procs.fImageCtx = &sniff;
    return SkPicture::MakeFromStream(stream.get(), &procs) != nullptr;
}

int main(int argc, char** argv) {
    CommandLineFlags::SetUsage(
            "Usage: get_images_from_skps -s <dir of skps> -o <dir for output images> --testDecode "
            "-j <output JSON path> --writeImages, --writeFailedImages\n");

    CommandLineFlags::Parse(argc, argv);
    const char* inputs = FLAGS_skps[0];
    gOutputDir = FLAGS_out[0];

    if (!sk_isdir(gOutputDir)) {
        CommandLineFlags::PrintUsage();
        return 1;
    }

    if (sk_isdir(inputs)) {
        SkOSFile::Iter iter(inputs, "skp");
        for (SkString file; iter.next(&file); ) {
            if (!get_images_from_file(SkOSPath::Join(inputs, file.c_str()))) {
                return 2;
            }
        }
    } else {
        if (!get_images_from_file(SkString(inputs))) {
            return 2;
        }
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

    unsigned int totalFailures = 0,
              totalUnsupported = 0;
    SkDynamicMemoryWStream memStream;
    SkJSONWriter writer(&memStream, SkJSONWriter::Mode::kPretty);
    writer.beginObject();
    {
        writer.beginObject("failures");
        {
            for(const auto& failure : gSkpToUnknownCount) {
                SkDebugf("%s %d\n", failure.first.c_str(), failure.second);
                totalFailures += failure.second;
                writer.appendU32(failure.first.c_str(), failure.second);
            }
        }
        writer.endObject();
        writer.appendU32("totalFailures", totalFailures);

#ifdef SK_DEBUG
        writer.beginObject("unsupported");
        {
            for (const auto& unsupported : gSkpToUnsupportedCount) {
                SkDebugf("%s %d\n", unsupported.first.c_str(), unsupported.second);
                totalUnsupported += unsupported.second;
                writer.appendHexU32(unsupported.first.c_str(), unsupported.second);
            }

        }
        writer.endObject();
        writer.appendU32("totalUnsupported", totalUnsupported);
#endif

        writer.appendS32("totalSuccesses", gKnown);
        SkDebugf("%d known, %d failures, %d unsupported\n",
                 gKnown, totalFailures, totalUnsupported);
    }
    writer.endObject();
    writer.flush();

    if (totalFailures > 0 || totalUnsupported > 0) {
        if (!FLAGS_failuresJsonPath.isEmpty()) {
            SkDebugf("Writing failures to %s\n", FLAGS_failuresJsonPath[0]);
            SkFILEWStream stream(FLAGS_failuresJsonPath[0]);
            auto jsonStream = memStream.detachAsStream();
            stream.writeStream(jsonStream.get(), jsonStream->getLength());
        }
    }

    return 0;
}
