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

namespace skiagm {

    // The actual type we use to represent a checksum is hidden in here.
    typedef Json::UInt64 Checksum;
    static inline Json::Value asJsonValue(Checksum checksum) {
        return checksum;
    }
    static inline Checksum asChecksum(Json::Value jsonValue) {
        return jsonValue.asUInt64();
    }

    void gm_fprintf(FILE *stream, const char format[], ...);

    /**
     * Assembles rootPath and relativePath into a single path, like this:
     * rootPath/relativePath
     *
     * Uses SkPATH_SEPARATOR, to work on all platforms.
     *
     * TODO(epoger): This should probably move into SkOSFile.h
     */
    SkString SkPathJoin(const char *rootPath, const char *relativePath);

    Json::Value ActualResultAsJsonValue(const SkHashDigest& result);

    Json::Value CreateJsonTree(Json::Value expectedResults,
                               Json::Value actualResultsFailed,
                               Json::Value actualResultsFailureIgnored,
                               Json::Value actualResultsNoComparison,
                               Json::Value actualResultsSucceeded);

    /**
     * Test expectations (allowed image checksums, etc.)
     */
    class Expectations {
    public:
        /**
         * No expectations at all.
         */
        Expectations(bool ignoreFailure=kDefaultIgnoreFailure);

        /**
         * Expect exactly one image (appropriate for the case when we
         * are comparing against a single PNG file).
         */
        Expectations(const SkBitmap& bitmap, bool ignoreFailure=kDefaultIgnoreFailure);

        /**
         * Create Expectations from a JSON element as found within the
         * kJsonKey_ExpectedResults section.
         *
         * It's fine if the jsonElement is null or empty; in that case, we just
         * don't have any expectations.
         */
        Expectations(Json::Value jsonElement);

        /**
         * Returns true iff we want to ignore failed expectations.
         */
        bool ignoreFailure() const { return this->fIgnoreFailure; }

        /**
         * Returns true iff there are no allowed checksums.
         */
        bool empty() const { return this->fAllowedBitmapChecksums.empty(); }

        /**
         * Returns true iff actualChecksum matches any allowedChecksum,
         * regardless of fIgnoreFailure.  (The caller can check
         * that separately.)
         */
        bool match(Checksum actualChecksum) const;

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

        SkTArray<Checksum> fAllowedBitmapChecksums;
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

        Expectations get(const char *testName) SK_OVERRIDE ;

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
        JsonExpectationsSource(const char *jsonPath);

        Expectations get(const char *testName) SK_OVERRIDE;

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
