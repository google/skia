/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DMJsonWriter_DEFINED
#define DMJsonWriter_DEFINED

#include "CommandLineFlags.h"
#include "SkString.h"

namespace DM {

/**
 *  Class for collecting results from DM and writing to a json file.
 */
class JsonWriter {
public:
    /**
     *  Info describing a single run.
     */
    struct BitmapResult {
        SkString name;            // E.g. "ninepatch-stretch", "desk_gws.skp"
        SkString config;          //      "gpu", "8888", "serialize", "pipe"
        SkString sourceType;      //      "gm", "skp", "image"
        SkString sourceOptions;   //      "image", "codec", "subset", "scanline"
        SkString md5;             // In ASCII, so 32 bytes long.
        SkString ext;             // Extension of file we wrote: "png", "pdf", ...
        SkString gamut;
        SkString transferFn;
        SkString colorType;
        SkString alphaType;
        SkString colorDepth;
    };

    /**
     *  Add a result to the end of the list of results.
     *  Not thread safe.
     */
    static void AddBitmapResult(const BitmapResult&);

    /**
     *  Write all collected results to path.
     *  Not thread safe.
     */
    static void DumpJson(const char* path,
                         CommandLineFlags::StringArray key,
                         CommandLineFlags::StringArray properties);

    /**
     * Read JSON file at path written by DumpJson, calling callback for each
     * BitmapResult recorded in the file.  Return success.
     */
    static bool ReadJson(const char* path, void(*callback)(BitmapResult));
};


} // namespace DM
#endif // DMJsonWriter_DEFINED
