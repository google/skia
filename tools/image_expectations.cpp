/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapHasher.h"
#include "SkJSONCPP.h"
#include "SkOSFile.h"
#include "SkStream.h"
#include "SkTypes.h"

#include "image_expectations.h"

/*
 * TODO(epoger): Make constant strings consistent instead of mixing hypenated and camel-caps.
 *
 * TODO(epoger): Similar constants are already maintained in 2 other places:
 * gm/gm_json.py and gm/gm_expectations.cpp. We shouldn't add yet a third place.
 * Figure out a way to share the definitions instead.
 *
 * Note that, as of https://codereview.chromium.org/226293002 , the JSON
 * schema used here has started to differ from the one in gm_expectations.cpp .
 * TODO(epoger): Consider getting GM and render_pictures to use the same JSON
 * output module.
 */
const static char kJsonKey_ActualResults[] = "actual-results";
const static char kJsonKey_Header[] = "header";
const static char kJsonKey_Header_Type[] = "type";
const static char kJsonKey_Header_Revision[] = "revision";  // unique within Type
const static char kJsonKey_Image_ChecksumAlgorithm[] = "checksumAlgorithm";
const static char kJsonKey_Image_ChecksumValue[] = "checksumValue";
const static char kJsonKey_Image_ComparisonResult[] = "comparisonResult";
const static char kJsonKey_Image_Filepath[] = "filepath";
const static char kJsonKey_Source_TiledImages[] = "tiled-images";
const static char kJsonKey_Source_WholeImage[] = "whole-image";
// Values (not keys) that are written out by this JSON generator
const static char kJsonValue_Header_Type[] = "ChecksummedImages";
const static int kJsonValue_Header_Revision = 1;
const static char kJsonValue_Image_ChecksumAlgorithm_Bitmap64bitMD5[] = "bitmap-64bitMD5";
const static char kJsonValue_Image_ComparisonResult_NoComparison[] = "no-comparison";

namespace sk_tools {

    void ImageResultsSummary::add(const char *sourceName, const char *fileName, uint64_t hash,
                                  const int *tileNumber) {
        Json::Value image;
        image[kJsonKey_Image_ChecksumAlgorithm] = kJsonValue_Image_ChecksumAlgorithm_Bitmap64bitMD5;
        image[kJsonKey_Image_ChecksumValue] = Json::UInt64(hash);
        image[kJsonKey_Image_ComparisonResult] = kJsonValue_Image_ComparisonResult_NoComparison;
        image[kJsonKey_Image_Filepath] = fileName;
        if (NULL == tileNumber) {
            fActualResults[sourceName][kJsonKey_Source_WholeImage] = image;
        } else {
            fActualResults[sourceName][kJsonKey_Source_TiledImages][*tileNumber] = image;
        }
    }

    void ImageResultsSummary::add(const char *sourceName, const char *fileName, const SkBitmap& bitmap,
                                  const int *tileNumber) {
        uint64_t hash;
        SkAssertResult(SkBitmapHasher::ComputeDigest(bitmap, &hash));
        this->add(sourceName, fileName, hash, tileNumber);
    }

    void ImageResultsSummary::writeToFile(const char *filename) {
        Json::Value header;
        header[kJsonKey_Header_Type] = kJsonValue_Header_Type;
        header[kJsonKey_Header_Revision] = kJsonValue_Header_Revision;
        Json::Value root;
        root[kJsonKey_Header] = header;
        root[kJsonKey_ActualResults] = fActualResults;
        std::string jsonStdString = root.toStyledString();
        SkFILEWStream stream(filename);
        stream.write(jsonStdString.c_str(), jsonStdString.length());
    }

} // namespace sk_tools
