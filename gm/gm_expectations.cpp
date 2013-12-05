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

// See gm_json.py for descriptions of each of these JSON keys.
// These constants must be kept in sync with the ones in that Python file!
const static char kJsonKey_ActualResults[]   = "actual-results";
const static char kJsonKey_ActualResults_Failed[]        = "failed";
const static char kJsonKey_ActualResults_FailureIgnored[]= "failure-ignored";
const static char kJsonKey_ActualResults_NoComparison[]  = "no-comparison";
const static char kJsonKey_ActualResults_Succeeded[]     = "succeeded";
const static char kJsonKey_ExpectedResults[] = "expected-results";
const static char kJsonKey_ExpectedResults_AllowedDigests[] = "allowed-digests";
const static char kJsonKey_ExpectedResults_IgnoreFailure[]  = "ignore-failure";

// Types of result hashes we support in the JSON file.
const static char kJsonKey_Hashtype_Bitmap_64bitMD5[]  = "bitmap-64bitMD5";


namespace skiagm {
    void gm_fprintf(FILE *stream, const char format[], ...) {
        va_list args;
        va_start(args, format);
        fprintf(stream, "GM: ");
        vfprintf(stream, format, args);
#ifdef SK_BUILD_FOR_WIN
        if (stderr == stream || stdout == stream) {
            fflush(stream);
        }
#endif
        va_end(args);
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


    // GmResultDigest class...

    GmResultDigest::GmResultDigest(const SkBitmap &bitmap) {
        fIsValid = SkBitmapHasher::ComputeDigest(bitmap, &fHashDigest);
    }

    GmResultDigest::GmResultDigest(const Json::Value &jsonTypeValuePair) {
        fIsValid = false;
        if (!jsonTypeValuePair.isArray()) {
            gm_fprintf(stderr, "found non-array json value when parsing GmResultDigest: %s\n",
                       jsonTypeValuePair.toStyledString().c_str());
            DEBUGFAIL_SEE_STDERR;
        } else if (2 != jsonTypeValuePair.size()) {
            gm_fprintf(stderr, "found json array with wrong size when parsing GmResultDigest: %s\n",
                       jsonTypeValuePair.toStyledString().c_str());
            DEBUGFAIL_SEE_STDERR;
        } else {
            // TODO(epoger): The current implementation assumes that the
            // result digest is always of type kJsonKey_Hashtype_Bitmap_64bitMD5
            Json::Value jsonHashValue = jsonTypeValuePair[1];
            if (!jsonHashValue.isIntegral()) {
                gm_fprintf(stderr,
                           "found non-integer jsonHashValue when parsing GmResultDigest: %s\n",
                           jsonTypeValuePair.toStyledString().c_str());
                DEBUGFAIL_SEE_STDERR;
            } else {
                fHashDigest = jsonHashValue.asUInt64();
                fIsValid = true;
            }
        }
    }

    bool GmResultDigest::isValid() const {
        return fIsValid;
    }

    bool GmResultDigest::equals(const GmResultDigest &other) const {
        // TODO(epoger): The current implementation assumes that this
        // and other are always of type kJsonKey_Hashtype_Bitmap_64bitMD5
        return (this->fIsValid && other.fIsValid && (this->fHashDigest == other.fHashDigest));
    }

    Json::Value GmResultDigest::asJsonTypeValuePair() const {
        // TODO(epoger): The current implementation assumes that the
        // result digest is always of type kJsonKey_Hashtype_Bitmap_64bitMD5
        Json::Value jsonTypeValuePair;
        if (fIsValid) {
            jsonTypeValuePair.append(Json::Value(kJsonKey_Hashtype_Bitmap_64bitMD5));
            jsonTypeValuePair.append(Json::UInt64(fHashDigest));
        } else {
            jsonTypeValuePair.append(Json::Value("INVALID"));
        }
        return jsonTypeValuePair;
    }

    SkString GmResultDigest::getHashType() const {
        // TODO(epoger): The current implementation assumes that the
        // result digest is always of type kJsonKey_Hashtype_Bitmap_64bitMD5
        return SkString(kJsonKey_Hashtype_Bitmap_64bitMD5);
    }

    SkString GmResultDigest::getDigestValue() const {
        // TODO(epoger): The current implementation assumes that the
        // result digest is always of type kJsonKey_Hashtype_Bitmap_64bitMD5
        SkString retval;
        retval.appendU64(fHashDigest);
        return retval;
    }


    // Expectations class...

    Expectations::Expectations(bool ignoreFailure) {
        fIgnoreFailure = ignoreFailure;
    }

    Expectations::Expectations(const SkBitmap& bitmap, bool ignoreFailure) {
        fBitmap = bitmap;
        fIgnoreFailure = ignoreFailure;
        fAllowedResultDigests.push_back(GmResultDigest(bitmap));
    }

    Expectations::Expectations(const BitmapAndDigest& bitmapAndDigest) {
        fBitmap = bitmapAndDigest.fBitmap;
        fIgnoreFailure = false;
        fAllowedResultDigests.push_back(bitmapAndDigest.fDigest);
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

            Json::Value allowedDigests = jsonElement[kJsonKey_ExpectedResults_AllowedDigests];
            if (allowedDigests.isNull()) {
                // ok, we'll just assume there aren't any AllowedDigests to compare against
            } else if (!allowedDigests.isArray()) {
                gm_fprintf(stderr, "found non-array json value"
                           " for key '%s' in element '%s'\n",
                           kJsonKey_ExpectedResults_AllowedDigests,
                           jsonElement.toStyledString().c_str());
                DEBUGFAIL_SEE_STDERR;
            } else {
                for (Json::ArrayIndex i=0; i<allowedDigests.size(); i++) {
                    fAllowedResultDigests.push_back(GmResultDigest(allowedDigests[i]));
                }
            }
        }
    }

    bool Expectations::match(GmResultDigest actualGmResultDigest) const {
        for (int i=0; i < this->fAllowedResultDigests.count(); i++) {
            GmResultDigest allowedResultDigest = this->fAllowedResultDigests[i];
            if (allowedResultDigest.equals(actualGmResultDigest)) {
                return true;
            }
        }
        return false;
    }

    Json::Value Expectations::asJsonValue() const {
        Json::Value allowedDigestArray;
        if (!this->fAllowedResultDigests.empty()) {
            for (int i=0; i < this->fAllowedResultDigests.count(); i++) {
                allowedDigestArray.append(this->fAllowedResultDigests[i].asJsonTypeValuePair());
            }
        }

        Json::Value jsonExpectations;
        jsonExpectations[kJsonKey_ExpectedResults_AllowedDigests] = allowedDigestArray;
        jsonExpectations[kJsonKey_ExpectedResults_IgnoreFailure]  = this->ignoreFailure();
        return jsonExpectations;
    }


    // IndividualImageExpectationsSource class...

    Expectations IndividualImageExpectationsSource::get(const char *testName) const {
        SkString path = SkOSPath::SkPathJoin(fRootDir.c_str(), testName);
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

    Expectations JsonExpectationsSource::get(const char *testName) const {
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
