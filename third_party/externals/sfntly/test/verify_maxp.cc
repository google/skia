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
#include "sfntly/table/core/maximum_profile_table.h"
#include "test/serialization_test.h"

namespace sfntly {

const int32_t MAXP_NUM_GLYPHS = 1502;
const int32_t MAXP_MAX_POINTS = 181;
const int32_t MAXP_MAX_CONTOURS = 9;
const int32_t MAXP_MAX_COMPOSITE_POINTS = 172;
const int32_t MAXP_MAX_COMPOSITE_CONTOURS = 5;
const int32_t MAXP_MAX_ZONES = 2;
const int32_t MAXP_MAX_TWILIGHT_POINTS = 0;
const int32_t MAXP_MAX_STORAGE = 1;
const int32_t MAXP_MAX_FUNCTION_DEFS = 1;
const int32_t MAXP_MAX_INSTR_DEFS = 0;
const int32_t MAXP_MAX_STACK_ELEMENTS = 64;
const int32_t MAXP_MAX_INSTR_SIZE = 46;
const int32_t MAXP_MAX_COMPONENT_ELEMENTS = 4;
const int32_t MAXP_MAX_COMPONENT_DEPTH = 3;

static bool VerifyMAXP(Table* table) {
  MaximumProfileTablePtr maxp = down_cast<MaximumProfileTable*>(table);
  if (maxp == NULL) {
    return false;
  }

  EXPECT_EQ(maxp->TableVersion(), Fixed1616::Fixed(1, 0));
  EXPECT_EQ(maxp->NumGlyphs(), MAXP_NUM_GLYPHS);
  EXPECT_EQ(maxp->MaxPoints(), MAXP_MAX_POINTS);
  EXPECT_EQ(maxp->MaxContours(), MAXP_MAX_CONTOURS);
  EXPECT_EQ(maxp->MaxCompositePoints(), MAXP_MAX_COMPOSITE_POINTS);
  EXPECT_EQ(maxp->MaxCompositeContours(), MAXP_MAX_COMPOSITE_CONTOURS);
  EXPECT_EQ(maxp->MaxZones(), MAXP_MAX_ZONES);
  EXPECT_EQ(maxp->MaxTwilightPoints(), MAXP_MAX_TWILIGHT_POINTS);
  EXPECT_EQ(maxp->MaxStorage(), MAXP_MAX_STORAGE);
  EXPECT_EQ(maxp->MaxFunctionDefs(), MAXP_MAX_FUNCTION_DEFS);
  // TODO(arthurhsu): maxInstructionDefs observed in Microsoft TTF report.
  //                  Check with stuartg and see if this is a miss.
  EXPECT_EQ(maxp->MaxStackElements(), MAXP_MAX_STACK_ELEMENTS);
  EXPECT_EQ(maxp->MaxSizeOfInstructions(), MAXP_MAX_INSTR_SIZE);
  EXPECT_EQ(maxp->MaxComponentElements(), MAXP_MAX_COMPONENT_ELEMENTS);
  EXPECT_EQ(maxp->MaxComponentDepth(), MAXP_MAX_COMPONENT_DEPTH);

  return true;
}

bool VerifyMAXP(Table* original, Table* target) {
  EXPECT_TRUE(VerifyMAXP(original));
  EXPECT_TRUE(VerifyMAXP(target));
  return true;
}

}  // namespace sfntly
