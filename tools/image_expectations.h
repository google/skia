/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef image_expectations_DEFINED
#define image_expectations_DEFINED

#include "SkBitmap.h"
#include "SkJSONCPP.h"

namespace sk_tools {

    /**
     * Class for collecting image results (checksums) as we go.
     */
    class ImageResultsSummary {
    public:
        /**
         * Adds this image to the summary of results.
         *
         * @param sourceName name of the source file that generated this result
         * @param fileName relative path to the image output file on local disk
         * @param hash hash to store
         * @param tileNumber if not NULL, ptr to tile number
         */
        void add(const char *sourceName, const char *fileName, uint64_t hash,
                 const int *tileNumber=NULL);

        /**
         * Adds this image to the summary of results.
         *
         * @param sourceName name of the source file that generated this result
         * @param fileName relative path to the image output file on local disk
         * @param bitmap bitmap to store the hash of
         * @param tileNumber if not NULL, ptr to tile number
         */
        void add(const char *sourceName, const char *fileName, const SkBitmap& bitmap,
                 const int *tileNumber=NULL);

        /**
         * Writes the summary (as constructed so far) to a file.
         *
         * @param filename path to write the summary to
         */
        void writeToFile(const char *filename);

    private:
        Json::Value fActualResults;
    };

} // namespace sk_tools

#endif  // image_expectations_DEFINED
