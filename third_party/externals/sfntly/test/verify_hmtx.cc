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
#include "sfntly/table/core/horizontal_metrics_table.h"
#include "test/serialization_test.h"

namespace sfntly {

const int32_t HMTX_ENTRIES_COUNT = 1499;
const int32_t HMTX_LSB_COUNT = 3;

struct HmtxEntry {
  int32_t advance_width_;
  int32_t lsb_;

  HmtxEntry(int32_t advance_width, int32_t lsb)
      : advance_width_(advance_width), lsb_(lsb) {}
};

const HmtxEntry HMTX_ENTRIES[] = {
    HmtxEntry(748, 68),  // 0
    HmtxEntry(0, 0),  // 1
    HmtxEntry(682, 0),  // 2
    HmtxEntry(616, 0),  // 3
    HmtxEntry(421, 103),  // 4
    HmtxEntry(690, 129),  // 5
    HmtxEntry(1589, 129),  // 6
    HmtxEntry(1017, 25),  // 7
    HmtxEntry(1402, 104),  // 8
    HmtxEntry(1241, 100),  // 9
};
const int32_t NUM_HMTX_ENTRIES = 10;

static bool VerifyHMTX(Table* table) {
  HorizontalMetricsTablePtr hmtx = down_cast<HorizontalMetricsTable*>(table);
  if (hmtx == NULL) {
    return false;
  }

  EXPECT_EQ(hmtx->NumberOfHMetrics(), HMTX_ENTRIES_COUNT);
  EXPECT_EQ(hmtx->NumberOfLSBs(), HMTX_LSB_COUNT);

  for (int32_t i = 0; i < NUM_HMTX_ENTRIES; ++i) {
    EXPECT_EQ(hmtx->AdvanceWidth(i), HMTX_ENTRIES[i].advance_width_);
    EXPECT_EQ(hmtx->LeftSideBearing(i), HMTX_ENTRIES[i].lsb_);
  }

  // No such element case.
  EXPECT_EQ(hmtx->AdvanceWidth(HMTX_ENTRIES_COUNT),
            HMTX_ENTRIES[0].advance_width_);
  EXPECT_EQ(hmtx->LeftSideBearing(HMTX_ENTRIES_COUNT), HMTX_ENTRIES[0].lsb_);
  return true;
}

bool VerifyHMTX(Table* original, Table* target) {
  EXPECT_TRUE(VerifyHMTX(original));
  EXPECT_TRUE(VerifyHMTX(target));
  return true;
}

}  // namespace sfntly
