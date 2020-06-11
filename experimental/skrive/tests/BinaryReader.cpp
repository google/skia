/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/skrive/src/reader/StreamReader.h"
#include "tests/Test.h"

using namespace skrive::internal;

DEF_TEST(SkRive_BinaryReader, reporter) {
    static constexpr uint8_t bin[] = {
        0x46, 0x4c, 0x41, 0x52, 0x45,   // 'FLARE'
        0x12, 0x00, 0x00, 0x00,         // version: 18
        0x73,                           // block type: kArtboards (115)
        0x38, 0x00, 0x00, 0x00,         // block size: 56
        0x01, 0x00,                     // container count: 1
        0x72,                           // block type: kActorArtboard (114)
        0x31, 0x00, 0x00, 0x00,         // block size: 49
        0x04, 0x00, 0x00, 0x00,         // name len: 4
        0x46, 0x6f, 0x6f, 0x6f,         // name: 'Fooo'
                                        // translation:
        0x00, 0x00, 0x00, 0x00,         //   0
        0x00, 0x00, 0x00, 0x00,         //   0
        0x00, 0xc0, 0x57, 0x44,         // width:  863.0
        0x00, 0xc0, 0x60, 0x44,         // height: 899.0
                                        // origin:
        0x00, 0x00, 0x00, 0x00,         //   0
        0x00, 0x00, 0x00, 0x00,         //   0
        0x01,                           // clipContents: true
                                        // color:
        0x00, 0x00, 0x00, 0x3f,         //   0.5
        0x00, 0x00, 0x00, 0x3f,         //   0.5
        0x00, 0x00, 0x00, 0x3f,         //   0.5
        0x00, 0x00, 0x80, 0x3f,         //   1.0
    };

    auto sr = StreamReader::Make(SkData::MakeWithoutCopy(bin, sizeof(bin)));

    REPORTER_ASSERT(reporter, sr);
    REPORTER_ASSERT(reporter, sr->readUInt32("version") == 18);
    {
        StreamReader::AutoBlock ab(sr);
        REPORTER_ASSERT(reporter, ab.type() == StreamReader::BlockType::kArtboards);
        REPORTER_ASSERT(reporter, sr->readLength16() == 1);

        {
            StreamReader::AutoBlock ab(sr);
            REPORTER_ASSERT(reporter, ab.type() == StreamReader::BlockType::kActorArtboard);
            REPORTER_ASSERT(reporter, sr->readString("name").equals("Fooo"));
            REPORTER_ASSERT(reporter, sr->readV2("translation") == (SkV2{0,0}));
            REPORTER_ASSERT(reporter, sr->readFloat("width" ) == 863);
            REPORTER_ASSERT(reporter, sr->readFloat("height") == 899);
            REPORTER_ASSERT(reporter, sr->readV2("origin") == (SkV2{0,0}));
            REPORTER_ASSERT(reporter, sr->readBool("clipContents"));
            REPORTER_ASSERT(reporter, sr->readColor("color") == (SkColor4f{0.5f,0.5f,0.5f,1}));

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
