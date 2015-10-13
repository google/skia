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
#include "sfntly/table/truetype/loca_table.h"
#include "test/serialization_test.h"

namespace sfntly {

const int32_t LOCA_NUM_LOCAS = 1503;
const int32_t LOCAS[] = {
    0x00000,  // 0
    0x00058,  // 1
    0x00058,  // 2
    0x00058,  // 3
    0x00058,  // 4
    0x000B8,  // 5
    0x00138,  // 6
    0x001A4,  // 7
    0x0025C,  // 8
    0x00328,  // 9
    0x003B8,  // 10
};
const int32_t NUM_TEST_LOCAS = 11;

static bool VerifyLOCA(Table* table) {
  LocaTablePtr loca = down_cast<LocaTable*>(table);
  if (loca == NULL) {
    return false;
  }

  EXPECT_EQ(loca->NumLocas(), LOCA_NUM_LOCAS);
  EXPECT_EQ(loca->num_glyphs(), LOCA_NUM_LOCAS - 1);

  for (int32_t i = 0; i < NUM_TEST_LOCAS - 1; ++i) {
    EXPECT_EQ(loca->GlyphOffset(i), LOCAS[i]);
    EXPECT_EQ(loca->GlyphLength(i), LOCAS[i + 1] - LOCAS[i]);
  }
  return true;
}

bool VerifyLOCA(Table* original, Table* target) {
  EXPECT_TRUE(VerifyLOCA(original));
  EXPECT_TRUE(VerifyLOCA(target));
  return true;
}

}  // namespace sfntly
