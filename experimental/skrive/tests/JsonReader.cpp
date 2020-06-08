/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/skrive/src/reader/StreamReader.h"
#include "tests/Test.h"

using namespace skrive::internal;

DEF_TEST(SkRive_JsonReader, reporter) {
    static constexpr char json[] = R"({
                                         "version": 24,
                                         "artboards": [
                                           {
                                             "name"        : "artboard 1",
                                             "width"       : 500,
                                             "height"      : 250,
                                             "clipContents": true,
                                             "type"        : "artboard"
                                           }
                                         ]
                                     })";

    auto sr = StreamReader::Make(json, strlen(json));

    REPORTER_ASSERT(reporter, sr);
    {
        StreamReader::AutoBlock ab(sr);
        REPORTER_ASSERT(reporter, ab.type() == StreamReader::BlockType::kArtboards);

        {
            StreamReader::AutoBlock ab(sr);
            REPORTER_ASSERT(reporter, ab.type() == StreamReader::BlockType::kActorArtboard);
            REPORTER_ASSERT(reporter, sr->readString("name").equals("artboard 1"));
            REPORTER_ASSERT(reporter, sr->readFloat("width" ) == 500);
            REPORTER_ASSERT(reporter, sr->readFloat("height") == 250);
            REPORTER_ASSERT(reporter, sr->readBool("clipContents"));

            REPORTER_ASSERT(reporter, sr->readString("INVALID").equals(""));
            REPORTER_ASSERT(reporter, sr->readFloat("INVALID" ) == 0);
            REPORTER_ASSERT(reporter, !sr->readBool("INVALID"));
        }
        {
            StreamReader::AutoBlock ab(sr);
            REPORTER_ASSERT(reporter, ab.type() == StreamReader::BlockType::kEoB);
        }
    }
    {
        StreamReader::AutoBlock ab(sr);
        REPORTER_ASSERT(reporter, ab.type() == StreamReader::BlockType::kEoB);
    }
}
