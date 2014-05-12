/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapHasher.h"
#include "SkData.h"
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
const static char kJsonKey_ExpectedResults[] = "expected-results";
const static char kJsonKey_Header[] = "header";
const static char kJsonKey_Header_Type[] = "type";
const static char kJsonKey_Header_Revision[] = "revision";
const static char kJsonKey_Image_ChecksumAlgorithm[] = "checksumAlgorithm";
const static char kJsonKey_Image_ChecksumValue[] = "checksumValue";
const static char kJsonKey_Image_ComparisonResult[] = "comparisonResult";
const static char kJsonKey_Image_Filepath[] = "filepath";
const static char kJsonKey_Image_IgnoreFailure[] = "ignoreFailure";
const static char kJsonKey_Source_TiledImages[] = "tiled-images";
const static char kJsonKey_Source_WholeImage[] = "whole-image";
// Values (not keys) that are written out by this JSON generator
const static char kJsonValue_Header_Type[] = "ChecksummedImages";
const static int kJsonValue_Header_Revision = 1;
const static char kJsonValue_Image_ChecksumAlgorithm_Bitmap64bitMD5[] = "bitmap-64bitMD5";
const static char kJsonValue_Image_ComparisonResult_Failed[] = "failed";
const static char kJsonValue_Image_ComparisonResult_FailureIgnored[] = "failure-ignored";
const static char kJsonValue_Image_ComparisonResult_NoComparison[] = "no-comparison";
const static char kJsonValue_Image_ComparisonResult_Succeeded[] = "succeeded";

namespace sk_tools {

    // ImageDigest class...

    ImageDigest::ImageDigest(const SkBitmap &bitmap) {
        if (!SkBitmapHasher::ComputeDigest(bitmap, &fHashValue)) {
            SkFAIL("unable to compute image digest");
        }
    }

    ImageDigest::ImageDigest(const SkString &hashType, uint64_t hashValue) {
        if (!hashType.equals(kJsonValue_Image_ChecksumAlgorithm_Bitmap64bitMD5)) {
            SkFAIL((SkString("unsupported hashType ")+=hashType).c_str());
        } else {
            fHashValue = hashValue;
        }
    }

    SkString ImageDigest::getHashType() const {
        // TODO(epoger): The current implementation assumes that the
        // result digest is always of type kJsonValue_Image_ChecksumAlgorithm_Bitmap64bitMD5 .
        return SkString(kJsonValue_Image_ChecksumAlgorithm_Bitmap64bitMD5);
    }

    uint64_t ImageDigest::getHashValue() const {
        return fHashValue;
    }

    // BitmapAndDigest class...

    BitmapAndDigest::BitmapAndDigest(const SkBitmap &bitmap) : fBitmap(bitmap) {
    }

    const ImageDigest *BitmapAndDigest::getImageDigestPtr() {
        if (NULL == fImageDigestRef.get()) {
            fImageDigestRef.reset(SkNEW_ARGS(ImageDigest, (fBitmap)));
        }
        return fImageDigestRef.get();
    }

    const SkBitmap *BitmapAndDigest::getBitmapPtr() const {
        return &fBitmap;
    }

    // ImageResultsAndExpectations class...

    bool ImageResultsAndExpectations::readExpectationsFile(const char *jsonPath) {
        if (Parse(jsonPath, &fExpectedJsonRoot)) {
            fExpectedResults = fExpectedJsonRoot[kJsonKey_ExpectedResults];
            return true;
        } else {
            return false;
        }
    }

    void ImageResultsAndExpectations::add(const char *sourceName, const char *fileName,
                                  const ImageDigest &digest, const int *tileNumber) {
        // Get expectation, if any.
        Json::Value expectedImage;
        if (!fExpectedResults.isNull()) {
            if (NULL == tileNumber) {
                expectedImage = fExpectedResults[sourceName][kJsonKey_Source_WholeImage];
            } else {
                expectedImage = fExpectedResults[sourceName][kJsonKey_Source_TiledImages]
                                                [*tileNumber];
            }
        }

        // Fill in info about the actual result itself.
        Json::Value actualChecksumAlgorithm = digest.getHashType().c_str();
        Json::Value actualChecksumValue = Json::UInt64(digest.getHashValue());
        Json::Value actualImage;
        actualImage[kJsonKey_Image_ChecksumAlgorithm] = actualChecksumAlgorithm;
        actualImage[kJsonKey_Image_ChecksumValue] = actualChecksumValue;
        actualImage[kJsonKey_Image_Filepath] = fileName;

        // Compare against expectedImage to fill in comparisonResult.
        Json::Value comparisonResult = kJsonValue_Image_ComparisonResult_NoComparison;
        if (!expectedImage.isNull()) {
            if ((actualChecksumAlgorithm == expectedImage[kJsonKey_Image_ChecksumAlgorithm]) &&
                (actualChecksumValue == expectedImage[kJsonKey_Image_ChecksumValue])) {
                comparisonResult = kJsonValue_Image_ComparisonResult_Succeeded;
            } else if (expectedImage[kJsonKey_Image_IgnoreFailure] == true) {
                comparisonResult = kJsonValue_Image_ComparisonResult_FailureIgnored;
            } else {
                comparisonResult = kJsonValue_Image_ComparisonResult_Failed;
            }
        }
        actualImage[kJsonKey_Image_ComparisonResult] = comparisonResult;

        // Add this actual result to our collection.
        if (NULL == tileNumber) {
            fActualResults[sourceName][kJsonKey_Source_WholeImage] = actualImage;
        } else {
            fActualResults[sourceName][kJsonKey_Source_TiledImages][*tileNumber] = actualImage;
        }
    }

    void ImageResultsAndExpectations::writeToFile(const char *filename) const {
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

    /*static*/ bool ImageResultsAndExpectations::Parse(const char *jsonPath,
                                                       Json::Value *jsonRoot) {
        SkAutoDataUnref dataRef(SkData::NewFromFileName(jsonPath));
        if (NULL == dataRef.get()) {
            SkDebugf("error reading JSON file %s\n", jsonPath);
            return false;
        }

        const char *bytes = reinterpret_cast<const char *>(dataRef.get()->data());
        size_t size = dataRef.get()->size();
        Json::Reader reader;
        if (!reader.parse(bytes, bytes+size, *jsonRoot)) {
            SkDebugf("error parsing JSON file %s\n", jsonPath);
            return false;
        }

        return true;
    }

} // namespace sk_tools
