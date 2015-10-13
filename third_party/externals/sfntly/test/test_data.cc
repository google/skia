/*
 * Copyright 2011 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "sfntly/tag.cc"
#include "test/test_data.h"

namespace sfntly {

// If the TTF file used in test changed, the verify*.cc in test need to be
// changed also.
// TODO(arthurhsu): Refactor this into a test class and have all const inside.
//                  This way we can test multiple fonts using same set of
//                  code.

const char* SAMPLE_TTF_FILE = "Tuffy.ttf";
const char* SAMPLE_BITMAP_FONT = "AnonymousPro-Regular.ttf";

const size_t SAMPLE_TTF_SIZE = 183936;
const size_t SAMPLE_TTF_TABLES = 17;
const size_t SAMPLE_TTF_KNOWN_TAGS = 16;
const size_t SAMPLE_BITMAP_KNOWN_TAGS = 20;
const size_t SAMPLE_TTF_FEAT = 3;
const size_t SAMPLE_TTF_HEAD = 6;
const size_t SAMPLE_TTF_POST = 14;

const int32_t TTF_KNOWN_TAGS[] = {
    Tag::OS_2, Tag::cmap, Tag::cvt,  Tag::feat, Tag::gasp,
    Tag::glyf, Tag::head, Tag::hhea, Tag::hmtx, Tag::kern,
    Tag::loca, Tag::maxp, Tag::morx, Tag::name, Tag::post,
    Tag::prop };

const int32_t BITMAP_KNOWN_TAGS[] = {
    Tag::EBDT, Tag::EBLC, Tag::EBSC, Tag::LTSH, Tag::OS_2,
    Tag::VDMX, Tag::cmap, Tag::cvt,  Tag::fpgm, Tag::gasp,
    Tag::glyf, Tag::hdmx, Tag::head, Tag::hhea, Tag::hmtx,
    Tag::loca, Tag::maxp, Tag::name, Tag::post, Tag::prep };

const int64_t TTF_CHECKSUM[] = {
    0xD463FC48, 0x252028D1, 0x0065078A, 0xC01407B5, 0xFFFF0003,
    0x9544342B, 0xFC8F16AD, 0x0EC30C7A, 0xA029CD5D, 0x32513087,
    0x05C323B0, 0x06320195, 0x3B67E701, 0xE7DB08F3, 0xD46E5E89,
    0xE6EB4A27 };

const int64_t TTF_OFFSET[] = {
    0x00000198, 0x00001964, 0x000025B0, 0x0002CA74, 0x0002C854,
    0x00003D34, 0x0000011C, 0x00000154, 0x000001F0, 0x000245D8,
    0x000025B8, 0x00000178, 0x0002CAB4, 0x00024860, 0x00028854,
    0x0002C85C };

const int32_t TTF_LENGTH[] = {
            86,       3146,          8,         64,          8,
        133284,         54,         36,       6002,        648,
          6012,         32,        944,      16371,      16383,
           536 };

const unsigned char TTF_FEAT_DATA[] = {
    0, 1, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0x30, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0x34,
    0, 0, 1, 1, 0, 0xB, 0, 2, 0, 0, 0, 0x38, 0xC0, 0, 1, 2,
    0, 0, 1, 3, 0, 2, 1, 4, 0, 0, 1, 5, 0, 2, 1, 6 };

}  // namespace sfntly
