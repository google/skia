/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * TODO(epoger): Combine this with tools/image_expectations.h, or eliminate one of the two.
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
            return (kUnknown_SkColorType == fBitmap.colorType()) ? NULL : &fBitmap;
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

        Expectations get(const char *testName) const override ;

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

        Expectations get(const char *testName) const override;

    private:

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
