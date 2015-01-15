/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DMJsonWriter_DEFINED
#define DMJsonWriter_DEFINED

#include "SkString.h"
#include "Test.h"

namespace DM {

/**
 *  Class for collecting results from DM and writing to a json file.
 *  All methods are thread-safe.
 */
class JsonWriter {
public:
    /**
     *  Info describing a single run.
     */
    struct BitmapResult {
        SkString name;            // E.g. "ninepatch-stretch", "desk-gws_skp"
        SkString config;          //      "gpu", "8888"
        SkString mode;            //      "direct", "default-tilegrid", "pipe"
        SkString sourceType;      //      "GM", "SKP"
        SkString md5;             // In ASCII, so 32 bytes long.
    };

    /**
     *  Add a result to the end of the list of results.
     */
    static void AddBitmapResult(const BitmapResult&);

    /**
     *  Add a Failure from a Test.
     */
    static void AddTestFailure(const skiatest::Failure&);

    /**
     *  Write all collected results to the file FLAGS_writePath[0]/dm.json.
     */
    static void DumpJson();
};

} // namespace DM
#endif // DMJsonWriter_DEFINED
