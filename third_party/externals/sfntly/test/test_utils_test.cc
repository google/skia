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

// Must include at the first line to avoid ICU / stdint conflict.
#include "sfntly/port/type.h"

#include <stdio.h>
#include <unicode/ucnv.h>
#include <unicode/uchar.h>

#include "gtest/gtest.h"
#include "test/test_utils.h"

namespace sfntly {

// Check if proper encoding is being performed
// Conversion is done from UTF16 to UTF8, SJIS
bool TestEncoding() {
  UConverter* conv = TestUtils::GetEncoder("utf8");
  EXPECT_TRUE(conv != NULL);
  // Ūnĭcōde̽
  UChar from[8] = {0x016A, 0x006E, 0x012D, 0x0063, 0x014D, 0x0064, 0x0065,
                   0x033D};
  int32_t want[12] = {0xc5, 0xaa, 0x6e, 0xc4, 0xad, 0x63, 0xc5, 0x8d, 0x64,
                      0x65, 0xcc, 0xbd};
  int32_t i, j = 0;
  for (i = 0; i < 7; ++i) {
    int32_t encoded = TestUtils::EncodeOneChar(conv, (int16_t)from[i]);
    for (; encoded; encoded <<= 8) {
      byte_t b = (encoded & 0xff000000) >> 24;
      if (!b)
        continue;
      EXPECT_EQ(want[j], b);
      if (want[j++] != b) {
        ucnv_close(conv);
        return false;
      }
    }
  }
  ucnv_close(conv);
  return true;
}

// Check if the proper extension is obtained
bool TestExtension() {
  // usual file name
  const char *result;
  result = TestUtils::Extension("../data/ext/tuffy.ttf");
  EXPECT_EQ(strcmp(result, ".ttf"), 0);

  // more than one 'extension'
  result = TestUtils::Extension("tuffy.ttf.fake");
  EXPECT_EQ(strcmp(result, ".fake"), 0);

  // no extension
  result = TestUtils::Extension("tuffy");
  EXPECT_STREQ(result, NULL);

  // bogus extension
  result = TestUtils::Extension("tuffy.");
  EXPECT_EQ(strcmp(result, "."), 0);

  return true;
}

}  // namespace sfntly

TEST(TestUtils, All) {
  ASSERT_TRUE(sfntly::TestExtension());
  ASSERT_TRUE(sfntly::TestEncoding());
}
