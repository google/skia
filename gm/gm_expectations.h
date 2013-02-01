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
#include "SkBitmapChecksummer.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkRefCnt.h"
#include "SkTArray.h"

#ifdef SK_BUILD_FOR_WIN
    // json includes xlocale which generates warning 4530 because we're compiling without
    // exceptions; see https://code.google.com/p/skia/issues/detail?id=1067
    #pragma warning(push)
    #pragma warning(disable : 4530)
#endif
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
         *
         * We set ignoreFailure to false by default, but it doesn't really
         * matter... the result will always be "no-comparison" anyway.
         */
        Expectations(bool ignoreFailure=false) {
            fIgnoreFailure = ignoreFailure;
        }

        /**
         * Expect exactly one image (appropriate for the case when we
         * are comparing against a single PNG file).
         *
         * By default, DO NOT ignore failures.
         */
        Expectations(const SkBitmap& bitmap, bool ignoreFailure=false) {
            fBitmap = bitmap;
            fIgnoreFailure = ignoreFailure;
            fAllowedChecksums.push_back() = SkBitmapChecksummer::Compute64(bitmap);
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
         * notifyOfMissingFiles: whether to log a message to stderr if an image
         *                       file cannot be found
         */
        IndividualImageExpectationsSource(const char *rootDir,
                                          bool notifyOfMissingFiles) :
            fRootDir(rootDir), fNotifyOfMissingFiles(notifyOfMissingFiles) {}

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
                if (fNotifyOfMissingFiles) {
                    fprintf(stderr, "FAILED to read %s\n", path.c_str());
                }
                return Expectations();
            }
        }

    private:
        const SkString fRootDir;
        const bool fNotifyOfMissingFiles;
    };

}
#endif
