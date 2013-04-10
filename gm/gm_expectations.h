/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef gm_expectations_DEFINED
#define gm_expectations_DEFINED

#include <stdarg.h>
#include "gm.h"
#include "SkBitmap.h"
#include "SkBitmapChecksummer.h"
#include "SkData.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkRefCnt.h"
#include "SkStream.h"
#include "SkTArray.h"

#ifdef SK_BUILD_FOR_WIN
    // json includes xlocale which generates warning 4530 because we're compiling without
    // exceptions; see https://code.google.com/p/skia/issues/detail?id=1067
    #pragma warning(push)
    #pragma warning(disable : 4530)
#endif
#include "json/reader.h"
#include "json/value.h"
#ifdef SK_BUILD_FOR_WIN
    #pragma warning(pop)
#endif

#define DEBUGFAIL_SEE_STDERR SkDEBUGFAIL("see stderr for message")

const static char kJsonKey_ActualResults[]   = "actual-results";
const static char kJsonKey_ActualResults_Failed[]        = "failed";
const static char kJsonKey_ActualResults_FailureIgnored[]= "failure-ignored";
const static char kJsonKey_ActualResults_NoComparison[]  = "no-comparison";
const static char kJsonKey_ActualResults_Succeeded[]     = "succeeded";
const static char kJsonKey_ActualResults_AnyStatus_Checksum[]    = "checksum";

const static char kJsonKey_ExpectedResults[] = "expected-results";
const static char kJsonKey_ExpectedResults_Checksums[]     = "checksums";
const static char kJsonKey_ExpectedResults_IgnoreFailure[] = "ignore-failure";

namespace skiagm {

    // The actual type we use to represent a checksum is hidden in here.
    typedef Json::UInt64 Checksum;
    static inline Json::Value asJsonValue(Checksum checksum) {
        return checksum;
    }
    static inline Checksum asChecksum(Json::Value jsonValue) {
        return jsonValue.asUInt64();
    }

    static void gm_fprintf(FILE *stream, const char format[], ...) {
        va_list args;
        va_start(args, format);
        fprintf(stream, "GM: ");
        vfprintf(stream, format, args);
        va_end(args);
    }

    static SkString make_filename(const char path[],
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

    /**
     * Test expectations (allowed image checksums, etc.)
     */
    class Expectations {
    public:
        /**
         * No expectations at all.
         */
        Expectations(bool ignoreFailure=kDefaultIgnoreFailure) {
            fIgnoreFailure = ignoreFailure;
        }

        /**
         * Expect exactly one image (appropriate for the case when we
         * are comparing against a single PNG file).
         */
        Expectations(const SkBitmap& bitmap, bool ignoreFailure=kDefaultIgnoreFailure) {
            fBitmap = bitmap;
            fIgnoreFailure = ignoreFailure;
            fAllowedChecksums.push_back() = SkBitmapChecksummer::Compute64(bitmap);
        }

        /**
         * Create Expectations from a JSON element as found within the
         * kJsonKey_ExpectedResults section.
         *
         * It's fine if the jsonElement is null or empty; in that case, we just
         * don't have any expectations.
         */
        Expectations(Json::Value jsonElement) {
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

                Json::Value allowedChecksums = jsonElement[kJsonKey_ExpectedResults_Checksums];
                if (allowedChecksums.isNull()) {
                    // ok, we'll just assume there aren't any expected checksums to compare against
                } else if (!allowedChecksums.isArray()) {
                    gm_fprintf(stderr, "found non-array json value"
                               " for key '%s' in element '%s'\n",
                               kJsonKey_ExpectedResults_Checksums,
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
                            fAllowedChecksums.push_back() = asChecksum(checksumElement);
                        }
                    }
                }
            }
        }

        /**
         * Returns true iff we want to ignore failed expectations.
         */
        bool ignoreFailure() const { return this->fIgnoreFailure; }

        /**
         * Returns true iff there are no allowed checksums.
         */
        bool empty() const { return this->fAllowedChecksums.empty(); }

        /**
         * Returns true iff actualChecksum matches any allowedChecksum,
         * regardless of fIgnoreFailure.  (The caller can check
         * that separately.)
         */
        bool match(Checksum actualChecksum) const {
            for (int i=0; i < this->fAllowedChecksums.count(); i++) {
                Checksum allowedChecksum = this->fAllowedChecksums[i];
                if (allowedChecksum == actualChecksum) {
                    return true;
                }
            }
            return false;
        }

        /**
         * If this Expectation is based on a single SkBitmap, return a
         * pointer to that SkBitmap. Otherwise (if the Expectation is
         * empty, or if it was based on a list of checksums rather
         * than a single bitmap), returns NULL.
         */
        const SkBitmap *asBitmap() const {
            return (SkBitmap::kNo_Config == fBitmap.config()) ? NULL : &fBitmap;
        }

        /**
         * Return a JSON representation of the allowed checksums.
         * This does NOT include any information about whether to
         * ignore failures.
         */
        Json::Value allowedChecksumsAsJson() const {
            Json::Value allowedChecksumArray;
            if (!this->fAllowedChecksums.empty()) {
                for (int i=0; i < this->fAllowedChecksums.count(); i++) {
                    Checksum allowedChecksum = this->fAllowedChecksums[i];
                    allowedChecksumArray.append(asJsonValue(allowedChecksum));
                }
            }
            return allowedChecksumArray;
        }

    private:
        const static bool kDefaultIgnoreFailure = false;

        SkTArray<Checksum> fAllowedChecksums;
        bool fIgnoreFailure;
        SkBitmap fBitmap;
    };

    /**
     * Abstract source of Expectations objects for individual tests.
     */
    class ExpectationsSource : public SkRefCnt {
    public:
        virtual Expectations get(const char *testName) = 0;
    };

    /**
     * Return Expectations based on individual image files on disk.
     */
    class IndividualImageExpectationsSource : public ExpectationsSource {
    public:
        /**
         * Create an ExpectationsSource that will return Expectations based on
         * image files found within rootDir.
         *
         * rootDir: directory under which to look for image files
         *          (this string will be copied to storage within this object)
         */
        IndividualImageExpectationsSource(const char *rootDir) : fRootDir(rootDir) {}

        Expectations get(const char *testName) SK_OVERRIDE {
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

    private:
        const SkString fRootDir;
    };

    /**
     * Return Expectations based on JSON summary file.
     */
    class JsonExpectationsSource : public ExpectationsSource {
    public:
        /**
         * Create an ExpectationsSource that will return Expectations based on
         * a JSON file.
         *
         * jsonPath: path to JSON file to read
         */
        JsonExpectationsSource(const char *jsonPath) {
            parse(jsonPath, &fJsonRoot);
            fJsonExpectedResults = fJsonRoot[kJsonKey_ExpectedResults];
        }

        Expectations get(const char *testName) SK_OVERRIDE {
            return Expectations(fJsonExpectedResults[testName]);
        }

    private:

        /**
         * Read as many bytes as possible (up to maxBytes) from the stream into
         * an SkData object.
         *
         * If the returned SkData contains fewer than maxBytes, then EOF has been
         * reached and no more data would be available from subsequent calls.
         * (If EOF has already been reached, then this call will return an empty
         * SkData object immediately.)
         *
         * If there are fewer than maxBytes bytes available to read from the
         * stream, but the stream has not been closed yet, this call will block
         * until there are enough bytes to read or the stream has been closed.
         *
         * It is up to the caller to call unref() on the returned SkData object
         * once the data is no longer needed, so that the underlying buffer will
         * be freed.  For example:
         *
         * {
         *   size_t maxBytes = 256;
         *   SkAutoDataUnref dataRef(readIntoSkData(stream, maxBytes));
         *   if (NULL != dataRef.get()) {
         *     size_t bytesActuallyRead = dataRef.get()->size();
         *     // use the data...
         *   }
         * }
         * // underlying buffer has been freed, thanks to auto unref
         *
         */
        // TODO(epoger): Move this, into SkStream.[cpp|h] as attempted in
        // https://codereview.appspot.com/7300071 ?
        // And maybe readFileIntoSkData() also?
        static SkData* readIntoSkData(SkStream &stream, size_t maxBytes) {
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

        /**
         * Wrapper around readIntoSkData for files: reads the entire file into
         * an SkData object.
         */
        static SkData* readFileIntoSkData(SkFILEStream &stream) {
            return readIntoSkData(stream, stream.getLength());
        }

        /**
         * Read the file contents from jsonPath and parse them into jsonRoot.
         *
         * Returns true if successful.
         */
        static bool parse(const char *jsonPath, Json::Value *jsonRoot) {
            SkFILEStream inFile(jsonPath);
            if (!inFile.isValid()) {
                gm_fprintf(stderr, "unable to read JSON file %s\n", jsonPath);
                DEBUGFAIL_SEE_STDERR;
                return false;
            }

            SkAutoDataUnref dataRef(readFileIntoSkData(inFile));
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

        Json::Value fJsonRoot;
        Json::Value fJsonExpectedResults;
    };

}
#endif
