/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm_expectations.h"
#include "SkBitmapHasher.h"
#include "SkImageDecoder.h"

#define DEBUGFAIL_SEE_STDERR SkDEBUGFAIL("see stderr for message")

const static char kJsonKey_ActualResults[]   = "actual-results";
const static char kJsonKey_ActualResults_Failed[]        = "failed";
const static char kJsonKey_ActualResults_FailureIgnored[]= "failure-ignored";
const static char kJsonKey_ActualResults_NoComparison[]  = "no-comparison";
const static char kJsonKey_ActualResults_Succeeded[]     = "succeeded";
const static char kJsonKey_ActualResults_AnyStatus_BitmapHash[]  = "bitmap-64bitMD5";

const static char kJsonKey_ExpectedResults[] = "expected-results";
const static char kJsonKey_ExpectedResults_AllowedBitmapHashes[] = "allowed-bitmap-64bitMD5s";
const static char kJsonKey_ExpectedResults_IgnoreFailure[]           = "ignore-failure";

namespace skiagm {

    void gm_fprintf(FILE *stream, const char format[], ...) {
        va_list args;
        va_start(args, format);
        fprintf(stream, "GM: ");
        vfprintf(stream, format, args);
        va_end(args);
    }

    SkString make_filename(const char path[],
                           const char renderModeDescriptor[],
                           const char *name,
                           const char suffix[]) {
        SkString filename(path);
        if (filename.endsWith(SkPATH_SEPARATOR)) {
            filename.remove(filename.size() - 1, 1);
        }
        filename.appendf("%c%s%s.%s", SkPATH_SEPARATOR,
                         name, renderModeDescriptor, suffix);
        return filename;
    }

    // TODO(epoger): This currently assumes that the result SkHashDigest was
    // generated as an SkHashDigest of an SkBitmap.  We'll need to allow for other
    // hash types to cover non-bitmaps.
    Json::Value ActualResultAsJsonValue(const SkHashDigest& result) {
        Json::Value jsonValue;
        jsonValue[kJsonKey_ActualResults_AnyStatus_BitmapHash] = asJsonValue(result);
        return jsonValue;
    }

    Json::Value CreateJsonTree(Json::Value expectedResults,
                               Json::Value actualResultsFailed,
                               Json::Value actualResultsFailureIgnored,
                               Json::Value actualResultsNoComparison,
                               Json::Value actualResultsSucceeded) {
        Json::Value actualResults;
        actualResults[kJsonKey_ActualResults_Failed] = actualResultsFailed;
        actualResults[kJsonKey_ActualResults_FailureIgnored] = actualResultsFailureIgnored;
        actualResults[kJsonKey_ActualResults_NoComparison] = actualResultsNoComparison;
        actualResults[kJsonKey_ActualResults_Succeeded] = actualResultsSucceeded;
        Json::Value root;
        root[kJsonKey_ActualResults] = actualResults;
        root[kJsonKey_ExpectedResults] = expectedResults;
        return root;
    }


    // Expectations class...

    Expectations::Expectations(bool ignoreFailure) {
        fIgnoreFailure = ignoreFailure;
    }

    Expectations::Expectations(const SkBitmap& bitmap, bool ignoreFailure) {
        fBitmap = bitmap;
        fIgnoreFailure = ignoreFailure;
        SkHashDigest digest;
        // TODO(epoger): Better handling for error returned by ComputeDigest()?
        // For now, we just report a digest of 0 in error cases, like before.
        if (!SkBitmapHasher::ComputeDigest(bitmap, &digest)) {
            digest = 0;
        }
        fAllowedBitmapChecksums.push_back() = digest;
    }

    Expectations::Expectations(Json::Value jsonElement) {
        if (jsonElement.empty()) {
            fIgnoreFailure = kDefaultIgnoreFailure;
        } else {
            Json::Value ignoreFailure = jsonElement[kJsonKey_ExpectedResults_IgnoreFailure];
            if (ignoreFailure.isNull()) {
                fIgnoreFailure = kDefaultIgnoreFailure;
            } else if (!ignoreFailure.isBool()) {
                gm_fprintf(stderr, "found non-boolean json value"
                           " for key '%s' in element '%s'\n",
                           kJsonKey_ExpectedResults_IgnoreFailure,
                           jsonElement.toStyledString().c_str());
                DEBUGFAIL_SEE_STDERR;
                fIgnoreFailure = kDefaultIgnoreFailure;
            } else {
                fIgnoreFailure = ignoreFailure.asBool();
            }

            Json::Value allowedChecksums =
                jsonElement[kJsonKey_ExpectedResults_AllowedBitmapHashes];
            if (allowedChecksums.isNull()) {
                // ok, we'll just assume there aren't any expected checksums to compare against
            } else if (!allowedChecksums.isArray()) {
                gm_fprintf(stderr, "found non-array json value"
                           " for key '%s' in element '%s'\n",
                           kJsonKey_ExpectedResults_AllowedBitmapHashes,
                           jsonElement.toStyledString().c_str());
                DEBUGFAIL_SEE_STDERR;
            } else {
                for (Json::ArrayIndex i=0; i<allowedChecksums.size(); i++) {
                    Json::Value checksumElement = allowedChecksums[i];
                    if (!checksumElement.isIntegral()) {
                        gm_fprintf(stderr, "found non-integer checksum"
                                   " in json element '%s'\n",
                                   jsonElement.toStyledString().c_str());
                        DEBUGFAIL_SEE_STDERR;
                    } else {
                        fAllowedBitmapChecksums.push_back() = asChecksum(checksumElement);
                    }
                }
            }
        }
    }

    bool Expectations::match(Checksum actualChecksum) const {
        for (int i=0; i < this->fAllowedBitmapChecksums.count(); i++) {
            Checksum allowedChecksum = this->fAllowedBitmapChecksums[i];
            if (allowedChecksum == actualChecksum) {
                return true;
            }
        }
        return false;
    }

    Json::Value Expectations::asJsonValue() const {
        Json::Value allowedChecksumArray;
        if (!this->fAllowedBitmapChecksums.empty()) {
            for (int i=0; i < this->fAllowedBitmapChecksums.count(); i++) {
                Checksum allowedChecksum = this->fAllowedBitmapChecksums[i];
                allowedChecksumArray.append(Json::UInt64(allowedChecksum));
            }
        }

        Json::Value jsonValue;
        jsonValue[kJsonKey_ExpectedResults_AllowedBitmapHashes] = allowedChecksumArray;
        jsonValue[kJsonKey_ExpectedResults_IgnoreFailure] = this->ignoreFailure();
        return jsonValue;
    }


    // IndividualImageExpectationsSource class...

    Expectations IndividualImageExpectationsSource::get(const char *testName) {
        SkString path = make_filename(fRootDir.c_str(), "", testName,
                                      "png");
        SkBitmap referenceBitmap;
        bool decodedReferenceBitmap =
            SkImageDecoder::DecodeFile(path.c_str(), &referenceBitmap,
                                       SkBitmap::kARGB_8888_Config,
                                       SkImageDecoder::kDecodePixels_Mode,
                                       NULL);
        if (decodedReferenceBitmap) {
            return Expectations(referenceBitmap);
        } else {
            return Expectations();
        }
    }


    // JsonExpectationsSource class...

    JsonExpectationsSource::JsonExpectationsSource(const char *jsonPath) {
        Parse(jsonPath, &fJsonRoot);
        fJsonExpectedResults = fJsonRoot[kJsonKey_ExpectedResults];
    }

    Expectations JsonExpectationsSource::get(const char *testName) {
        return Expectations(fJsonExpectedResults[testName]);
    }

    /*static*/ SkData* JsonExpectationsSource::ReadIntoSkData(SkStream &stream, size_t maxBytes) {
        if (0 == maxBytes) {
            return SkData::NewEmpty();
        }
        char* bufStart = reinterpret_cast<char *>(sk_malloc_throw(maxBytes));
        char* bufPtr = bufStart;
        size_t bytesRemaining = maxBytes;
        while (bytesRemaining > 0) {
            size_t bytesReadThisTime = stream.read(bufPtr, bytesRemaining);
            if (0 == bytesReadThisTime) {
                break;
            }
            bytesRemaining -= bytesReadThisTime;
            bufPtr += bytesReadThisTime;
        }
        return SkData::NewFromMalloc(bufStart, maxBytes - bytesRemaining);
    }

    /*static*/ bool JsonExpectationsSource::Parse(const char *jsonPath, Json::Value *jsonRoot) {
        SkFILEStream inFile(jsonPath);
        if (!inFile.isValid()) {
            gm_fprintf(stderr, "unable to read JSON file %s\n", jsonPath);
            DEBUGFAIL_SEE_STDERR;
            return false;
        }

        SkAutoDataUnref dataRef(ReadFileIntoSkData(inFile));
        if (NULL == dataRef.get()) {
            gm_fprintf(stderr, "error reading JSON file %s\n", jsonPath);
            DEBUGFAIL_SEE_STDERR;
            return false;
        }

        const char *bytes = reinterpret_cast<const char *>(dataRef.get()->data());
        size_t size = dataRef.get()->size();
        Json::Reader reader;
        if (!reader.parse(bytes, bytes+size, *jsonRoot)) {
            gm_fprintf(stderr, "error parsing JSON file %s\n", jsonPath);
            DEBUGFAIL_SEE_STDERR;
            return false;
        }
        return true;
    }

}
