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
#include "sfntly/table/truetype/glyph_table.h"
#include "test/serialization_test.h"

namespace sfntly {

// We spot check only glyph id 33.
const int32_t GLYPH33_OFFSET = 0xAC8;
const int32_t GLYPH33_LENGTH = 40;
const int32_t GLYPH33_XMIN = 92;
const int32_t GLYPH33_YMIN = 20;
const int32_t GLYPH33_XMAX = 797;
const int32_t GLYPH33_YMAX = 1235;

// TODO(arthurhsu): Tuffy does not have composite glyphs.  Need better testing.
static bool VerifyGLYF(Table* table) {
  GlyphTablePtr glyf_table = down_cast<GlyphTable*>(table);
  if (glyf_table == NULL) {
    return false;
  }

  GlyphPtr glyf;
  glyf.Attach(glyf_table->GetGlyph(GLYPH33_OFFSET, GLYPH33_LENGTH));
  if (glyf == NULL) {
    return false;
  }

  EXPECT_EQ(glyf->XMin(), GLYPH33_XMIN);
  EXPECT_EQ(glyf->YMin(), GLYPH33_YMIN);
  EXPECT_EQ(glyf->XMax(), GLYPH33_XMAX);
  EXPECT_EQ(glyf->YMax(), GLYPH33_YMAX);

  return true;
}

bool VerifyGLYF(Table* original, Table* target) {
  EXPECT_TRUE(VerifyGLYF(original));
  EXPECT_TRUE(VerifyGLYF(target));
  return true;
}

}  // namespace sfntly
