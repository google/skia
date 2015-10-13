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

#include "gtest/gtest.h"
#include "sfntly/font.h"
#include "sfntly/table/core/os2_table.h"
#include "test/serialization_test.h"

namespace sfntly {

const int32_t OS2_VERSION = 1;
const int32_t OS2_XAVG_CHAR_WIDTH = 863;
const int32_t OS2_US_WEIGHT_CLASS = 500;
const int32_t OS2_US_WIDTH_CLASS = 5;
const int32_t OS2_FS_TYPE = 0;
const int32_t OS2_YSUBS_XSIZE = 0;
const int32_t OS2_YSUBS_YSIZE = 2;
const int32_t OS2_YSUBS_XOFFSET = -16560;
const int32_t OS2_YSUBS_YOFFSET = 0;
const int32_t OS2_YSUPS_XSIZE = -25944;
const int32_t OS2_YSUPS_YSIZE = -27176;
const int32_t OS2_YSUPS_XOFFSET = -16376;
const int32_t OS2_YSUPS_YOFFSET = 1;
const int32_t OS2_YSTRIKEOUT_SIZE = 12312;
const int32_t OS2_YSTRIKEOUT_POS = -16224;
const int32_t OS2_SFAMILY_CLASS = 0;
const byte_t OS2_PANOSE[] = { 2, 11, 6, 3, 6, 1, 0, 0, 0, 0 };
const int64_t OS2_UL_UNICODE_RANGE1 = 0xE00002FFL;
const int64_t OS2_UL_UNICODE_RANGE2 = 0x520020FBL;
const int64_t OS2_UL_UNICODE_RANGE3 = 0L;
const int64_t OS2_UL_UNICODE_RANGE4 = 0L;
const byte_t OS2_ACH_VEND_ID[] = { 'P', 'f', 'E', 'd' };
const int32_t OS2_FS_SELECTION = 0x0040;
const int32_t OS2_US_FIRST_CHAR_IDX = 0x0020;
const int32_t OS2_US_LAST_CHAR_IDX = 0xFFFF;
const int32_t OS2_STYPO_ASCENDER = 1597;
const int32_t OS2_STYPO_DESCENDER = -451;
const int32_t OS2_STYPO_LINE_GAP = 0;
const int32_t OS2_US_WIN_ASCENT = 2023;
const int32_t OS2_US_WIN_DESCENT = 648;
const int64_t OS2_UL_CODE_PAGE_RANGE1 = 0x2000019FL;
const int64_t OS2_UL_CODE_PAGE_RANGE2 = 0x00000000L;

static bool VerifyOS_2(Table* table) {
  OS2TablePtr os2 = down_cast<OS2Table*>(table);
  if (os2 == NULL) {
    return false;
  }

  EXPECT_EQ(os2->TableVersion(), OS2_VERSION);
  EXPECT_EQ(os2->XAvgCharWidth(), OS2_XAVG_CHAR_WIDTH);
  EXPECT_EQ(os2->UsWeightClass(), OS2_US_WEIGHT_CLASS);
  EXPECT_EQ(os2->UsWidthClass(), OS2_US_WIDTH_CLASS);
  EXPECT_EQ(os2->FsType(), OS2_FS_TYPE);
  EXPECT_EQ(os2->YSubscriptXSize(), OS2_YSUBS_XSIZE);
  EXPECT_EQ(os2->YSubscriptYSize(), OS2_YSUBS_YSIZE);
  EXPECT_EQ(os2->YSubscriptXOffset(), OS2_YSUBS_XOFFSET);
  EXPECT_EQ(os2->YSubscriptYOffset(), OS2_YSUBS_YOFFSET);
  EXPECT_EQ(os2->YSuperscriptXSize(), OS2_YSUPS_XSIZE);
  EXPECT_EQ(os2->YSuperscriptYSize(), OS2_YSUPS_YSIZE);
  EXPECT_EQ(os2->YSuperscriptXOffset(), OS2_YSUPS_XOFFSET);
  EXPECT_EQ(os2->YSuperscriptYOffset(), OS2_YSUPS_YOFFSET);
  EXPECT_EQ(os2->YStrikeoutSize(), OS2_YSTRIKEOUT_SIZE);
  EXPECT_EQ(os2->YStrikeoutPosition(), OS2_YSTRIKEOUT_POS);
  EXPECT_EQ(os2->SFamilyClass(), OS2_SFAMILY_CLASS);

  ByteVector panose;
  os2->Panose(&panose);
  EXPECT_EQ(panose.size(), sizeof(OS2_PANOSE));
  for (size_t i = 0; i < panose.size(); ++i) {
    EXPECT_EQ(panose[i], OS2_PANOSE[i]);
  }

  EXPECT_EQ(os2->UlUnicodeRange1(), OS2_UL_UNICODE_RANGE1);
  EXPECT_EQ(os2->UlUnicodeRange2(), OS2_UL_UNICODE_RANGE2);
  EXPECT_EQ(os2->UlUnicodeRange3(), OS2_UL_UNICODE_RANGE3);
  EXPECT_EQ(os2->UlUnicodeRange4(), OS2_UL_UNICODE_RANGE4);

  ByteVector vend_id;
  os2->AchVendId(&vend_id);
  EXPECT_EQ(vend_id.size(), sizeof(OS2_ACH_VEND_ID));
  for (size_t i = 0; i < vend_id.size(); ++i) {
    EXPECT_EQ(vend_id[i], OS2_ACH_VEND_ID[i]);
  }

  EXPECT_EQ(os2->FsSelection(), OS2_FS_SELECTION);
  EXPECT_EQ(os2->UsFirstCharIndex(), OS2_US_FIRST_CHAR_IDX);
  EXPECT_EQ(os2->UsLastCharIndex(), OS2_US_LAST_CHAR_IDX);
  EXPECT_EQ(os2->STypoAscender(), OS2_STYPO_ASCENDER);
  EXPECT_EQ(os2->STypoDescender(), OS2_STYPO_DESCENDER);
  EXPECT_EQ(os2->STypoLineGap(), OS2_STYPO_LINE_GAP);
  EXPECT_EQ(os2->UsWinAscent(), OS2_US_WIN_ASCENT);
  EXPECT_EQ(os2->UsWinDescent(), OS2_US_WIN_DESCENT);
  EXPECT_EQ(os2->UlCodePageRange1(), OS2_UL_CODE_PAGE_RANGE1);
  EXPECT_EQ(os2->UlCodePageRange2(), OS2_UL_CODE_PAGE_RANGE2);

  // TODO(arthurhsu): SxHeight() not tested
  // TODO(arthurhsu): SCapHeight() not tested
  // TODO(arthurhsu): UsDefaultChar() not tested
  // TODO(arthurhsu): UsBreakChar() not tested
  // TODO(arthurhsu): UsMaxContext() not tested

  return true;
}

bool VerifyOS_2(Table* original, Table* target) {
  EXPECT_TRUE(VerifyOS_2(original));
  EXPECT_TRUE(VerifyOS_2(target));
  return true;
}

}  // namespace sfntly
