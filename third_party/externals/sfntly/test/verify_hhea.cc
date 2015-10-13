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
#include "sfntly/math/fixed1616.h"
#include "sfntly/table/core/horizontal_header_table.h"
#include "test/serialization_test.h"

namespace sfntly {

const int32_t HHEA_ASCENDER = 2023;
const int32_t HHEA_DESCENDER = -648;
const int32_t HHEA_LINE_GAP = 93;
const int32_t HHEA_ADVANCE_WIDTH_MAX = 2753;
const int32_t HHEA_MIN_LSB = -968;
const int32_t HHEA_MIN_RSB = -411;
const int32_t HHEA_X_MAX_EXTENT = 2628;
const int32_t HHEA_METRIC_DATA_FORMAT = 0;
const int32_t HHEA_NUM_METRICS = 1499;

static bool VerifyHHEA(Table* table) {
  HorizontalHeaderTablePtr hhea = down_cast<HorizontalHeaderTable*>(table);
  if (hhea == NULL) {
    return false;
  }

  EXPECT_EQ(hhea->TableVersion(), Fixed1616::Fixed(1, 0));
  EXPECT_EQ(hhea->Ascender(), HHEA_ASCENDER);
  EXPECT_EQ(hhea->Descender(), HHEA_DESCENDER);
  EXPECT_EQ(hhea->AdvanceWidthMax(), HHEA_ADVANCE_WIDTH_MAX);
  EXPECT_EQ(hhea->MinLeftSideBearing(), HHEA_MIN_LSB);
  EXPECT_EQ(hhea->MinRightSideBearing(), HHEA_MIN_RSB);
  EXPECT_EQ(hhea->XMaxExtent(), HHEA_X_MAX_EXTENT);
  // TODO(arthurhsu): CaretSlopeRise() not tested
  // TODO(arthurhsu): CaretSlopeRun() not tested
  // TODO(arthurhsu): CaretOffset() not tested
  EXPECT_EQ(hhea->MetricDataFormat(), HHEA_METRIC_DATA_FORMAT);
  EXPECT_EQ(hhea->NumberOfHMetrics(), HHEA_NUM_METRICS);

  return true;
}

bool VerifyHHEA(Table* original, Table* target) {
  EXPECT_TRUE(VerifyHHEA(original));
  EXPECT_TRUE(VerifyHHEA(target));
  return true;
}

}  // namespace sfntly
