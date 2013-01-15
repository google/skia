/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef gm_expectations_DEFINED
#define gm_expectations_DEFINED

#include "gm.h"
#include "SkBitmapChecksummer.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkRefCnt.h"
#include "SkTArray.h"
#include "json/value.h"

namespace skiagm {

    // The actual type we use to represent a checksum is hidden in here.
    typedef uint64_t Checksum;
    static inline Json::Value asJsonValue(Checksum checksum) {
        return Json::UInt64(checksum);
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
        Expectations(bool ignoreFailure=false) :
            fAllowedChecksums(SkTArray<Checksum>()),
            fIgnoreFailure(ignoreFailure) {}

        /**
         * Allow exactly one checksum (appropriate for the case when we
         * are comparing against a single PNG file).
         *
         * By default, DO NOT ignore failures.
         */
        Expectations(Checksum singleChecksum, bool ignoreFailure=false) :
            fAllowedChecksums(SkTArray<Checksum>(&singleChecksum, 1)),
            fIgnoreFailure(ignoreFailure) {
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
        const SkTArray<Checksum> fAllowedChecksums;
        const bool fIgnoreFailure;
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
                Checksum checksum = SkBitmapChecksummer::Compute64(
                    referenceBitmap);
                return Expectations(checksum);
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
