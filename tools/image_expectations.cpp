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
        if (NULL == jsonPath) {
            SkDebugf("JSON expectations filename not specified\n");
            return false;
        }
        SkFILE* filePtr = sk_fopen(jsonPath, kRead_SkFILE_Flag);
        if (NULL == filePtr) {
            SkDebugf("JSON expectations file '%s' does not exist\n", jsonPath);
            return false;
        }
        size_t size = sk_fgetsize(filePtr);
        if (0 == size) {
            SkDebugf("JSON expectations file '%s' is empty, so no expectations\n", jsonPath);
            sk_fclose(filePtr);
            fExpectedResults.clear();
            return true;
        }
        bool parsedJson = Parse(filePtr, &fExpectedJsonRoot);
        sk_fclose(filePtr);
        if (!parsedJson) {
            SkDebugf("Failed to parse JSON expectations file '%s'\n", jsonPath);
            return false;
        }
        Json::Value header = fExpectedJsonRoot[kJsonKey_Header];
        Json::Value headerType = header[kJsonKey_Header_Type];
        Json::Value headerRevision = header[kJsonKey_Header_Revision];
        if (strcmp(headerType.asCString(), kJsonValue_Header_Type)) {
            SkDebugf("JSON expectations file '%s': expected headerType '%s', found '%s'\n",
                     jsonPath, kJsonValue_Header_Type, headerType.asCString());
            return false;
        }
        if (headerRevision.asInt() != kJsonValue_Header_Revision) {
            SkDebugf("JSON expectations file '%s': expected headerRevision %d, found %d\n",
                     jsonPath, kJsonValue_Header_Revision, headerRevision.asInt());
            return false;
        }
        fExpectedResults = fExpectedJsonRoot[kJsonKey_ExpectedResults];
        return true;
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

    bool ImageResultsAndExpectations::matchesExpectation(const char *sourceName,
                                                         const ImageDigest &digest,
                                                         const int *tileNumber) {
        if (fExpectedResults.isNull()) {
            return false;
        }

        Json::Value expectedImage;
        if (NULL == tileNumber) {
            expectedImage = fExpectedResults[sourceName][kJsonKey_Source_WholeImage];
        } else {
            expectedImage = fExpectedResults[sourceName][kJsonKey_Source_TiledImages][*tileNumber];
        }
        if (expectedImage.isNull()) {
            return false;
        }

        Json::Value actualChecksumAlgorithm = digest.getHashType().c_str();
        Json::Value actualChecksumValue = Json::UInt64(digest.getHashValue());
        return ((actualChecksumAlgorithm == expectedImage[kJsonKey_Image_ChecksumAlgorithm]) &&
                (actualChecksumValue == expectedImage[kJsonKey_Image_ChecksumValue]));
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

    /*static*/ bool ImageResultsAndExpectations::Parse(SkFILE *filePtr,
                                                       Json::Value *jsonRoot) {
        SkAutoDataUnref dataRef(SkData::NewFromFILE(filePtr));
        if (NULL == dataRef.get()) {
            return false;
        }

        const char *bytes = reinterpret_cast<const char *>(dataRef.get()->data());
        size_t size = dataRef.get()->size();
        Json::Reader reader;
        if (!reader.parse(bytes, bytes+size, *jsonRoot)) {
            return false;
        }

        return true;
    }

} // namespace sk_tools
