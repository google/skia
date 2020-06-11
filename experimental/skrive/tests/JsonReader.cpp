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
                                             "translation" : [ 24, 42 ],
                                             "width"       : 500,
                                             "height"      : 250,
                                             "origin"      : [ 100, 100 ],
                                             "clipContents": true,
                                             "color"       : [ 1, 1, 0, 1],
                                             "type"        : "artboard"
                                           }
                                         ]
                                     })";

    auto sr = StreamReader::Make(SkData::MakeWithoutCopy(json, strlen(json)));

    REPORTER_ASSERT(reporter, sr);
    REPORTER_ASSERT(reporter, sr->readUInt32("version") == 24);
    {
        StreamReader::AutoBlock ab(sr);
        REPORTER_ASSERT(reporter, ab.type() == StreamReader::BlockType::kArtboards);
        REPORTER_ASSERT(reporter, sr->readLength16() == 1);

        {
            StreamReader::AutoBlock ab(sr);
            REPORTER_ASSERT(reporter, ab.type() == StreamReader::BlockType::kActorArtboard);
            REPORTER_ASSERT(reporter, sr->readString("name").equals("artboard 1"));
            REPORTER_ASSERT(reporter, sr->readV2("translation") == (SkV2{24,42}));
            REPORTER_ASSERT(reporter, sr->readFloat("width" ) == 500);
            REPORTER_ASSERT(reporter, sr->readFloat("height") == 250);
            REPORTER_ASSERT(reporter, sr->readV2("origin") == (SkV2{100,100}));
            REPORTER_ASSERT(reporter, sr->readBool("clipContents"));
            REPORTER_ASSERT(reporter, sr->readColor("color") == (SkColor4f{1,1,0,1}));

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
