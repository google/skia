/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef gm_expectations_DEFINED
#define gm_expectations_DEFINED

#include "gm.h"
#include "SkBitmap.h"
#include "SkBitmapHasher.h"
#include "SkData.h"
#include "SkJSONCPP.h"
#include "SkOSFile.h"
#include "SkRefCnt.h"
#include "SkStream.h"
#include "SkTArray.h"


namespace skiagm {

    void gm_fprintf(FILE *stream, const char format[], ...);

    Json::Value CreateJsonTree(Json::Value expectedResults,
                               Json::Value actualResultsFailed,
                               Json::Value actualResultsFailureIgnored,
                               Json::Value actualResultsNoComparison,
                               Json::Value actualResultsSucceeded);

    /**
     * The digest of a GM test result.
     *
     * Currently, this is always a uint64_t hash digest of an SkBitmap...
     * but we will add other flavors soon.
     */
    class GmResultDigest {
    public:
        /**
         * Create a ResultDigest representing an actual image result.
         */
        explicit GmResultDigest(const SkBitmap &bitmap);

        /**
         * Create a ResultDigest representing an allowed result
         * checksum within JSON expectations file, in the form
         * ["bitmap-64bitMD5", 12345].
         */
        explicit GmResultDigest(const Json::Value &jsonTypeValuePair);

        /**
         * Returns true if this GmResultDigest was fully and successfully
         * created.
         */
        bool isValid() const;

        /**
         * Returns true if this and other GmResultDigest could
         * represent identical results.
         */
        bool equals(const GmResultDigest &other) const;

        /**
         * Returns a JSON type/value pair representing this result,
         * such as ["bitmap-64bitMD5", 12345].
         */
        Json::Value asJsonTypeValuePair() const;

        /**
         * Returns the hashtype, such as "bitmap-64bitMD5", as an SkString.
         */
        SkString getHashType() const;

        /**
         * Returns the hash digest value, such as "12345", as an SkString.
         */
        SkString getDigestValue() const;

    private:
        bool fIsValid; // always check this first--if it's false, other fields are meaningless
        uint64_t fHashDigest;
    };

    /**
     * Encapsulates an SkBitmap and its GmResultDigest, guaranteed to keep them in sync.
     */
    class BitmapAndDigest {
    public:
        explicit BitmapAndDigest(const SkBitmap &bitmap) : fBitmap(bitmap), fDigest(bitmap) {}

        const SkBitmap fBitmap;
        const GmResultDigest fDigest;
    };

    /**
     * Test expectations (allowed image results, etc.)
     */
    class Expectations {
    public:
        /**
         * No expectations at all.
         */
        explicit Expectations(bool ignoreFailure=kDefaultIgnoreFailure);

        /**
         * Expect exactly one image (appropriate for the case when we
         * are comparing against a single PNG file).
         */
        explicit Expectations(const SkBitmap& bitmap, bool ignoreFailure=kDefaultIgnoreFailure);

        /**
         * Expect exactly one image, whose digest has already been computed.
         */
        explicit Expectations(const BitmapAndDigest& bitmapAndDigest);

        /**
         * Create Expectations from a JSON element as found within the
         * kJsonKey_ExpectedResults section.
         *
         * It's fine if the jsonElement is null or empty; in that case, we just
         * don't have any expectations.
         */
        explicit Expectations(Json::Value jsonElement);

        /**
         * Returns true iff we want to ignore failed expectations.
         */
        bool ignoreFailure() const { return this->fIgnoreFailure; }

        /**
         * Override default setting of fIgnoreFailure.
         */
        void setIgnoreFailure(bool val) { this->fIgnoreFailure = val; }

        /**
         * Returns true iff there are no allowed results.
         */
        bool empty() const { return this->fAllowedResultDigests.empty(); }

        /**
         * Returns true iff resultDigest matches any allowed result,
         * regardless of fIgnoreFailure.  (The caller can check
         * that separately.)
         */
        bool match(GmResultDigest resultDigest) const;

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
         * Return a JSON representation of the expectations.
         */
        Json::Value asJsonValue() const;

    private:
        const static bool kDefaultIgnoreFailure = false;

        SkTArray<GmResultDigest> fAllowedResultDigests;
        bool fIgnoreFailure;
        SkBitmap fBitmap;
    };

    /**
     * Abstract source of Expectations objects for individual tests.
     */
    class ExpectationsSource : public SkRefCnt {
    public:
        SK_DECLARE_INST_COUNT(ExpectationsSource)

        virtual Expectations get(const char *testName) const = 0;

    private:
        typedef SkRefCnt INHERITED;
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
        explicit IndividualImageExpectationsSource(const char *rootDir) : fRootDir(rootDir) {}

        Expectations get(const char *testName) const SK_OVERRIDE ;

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
        explicit JsonExpectationsSource(const char *jsonPath);

        Expectations get(const char *testName) const SK_OVERRIDE;

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
        // And maybe ReadFileIntoSkData() also?
        static SkData* ReadIntoSkData(SkStream &stream, size_t maxBytes);

        /**
         * Wrapper around ReadIntoSkData for files: reads the entire file into
         * an SkData object.
         */
        static SkData* ReadFileIntoSkData(SkFILEStream &stream) {
            return ReadIntoSkData(stream, stream.getLength());
        }

        /**
         * Read the file contents from jsonPath and parse them into jsonRoot.
         *
         * Returns true if successful.
         */
        static bool Parse(const char *jsonPath, Json::Value *jsonRoot);

        Json::Value fJsonRoot;
        Json::Value fJsonExpectedResults;
    };

}
#endif
