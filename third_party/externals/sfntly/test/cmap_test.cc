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

#include "sfntly/port/type.h"
#include <assert.h>
#include <unicode/ucnv.h>

#include <string>
#include <iostream>

#include "gtest/gtest.h"
#include "sfntly/font.h"
#include "sfntly/font_factory.h"
#include "sfntly/table/core/cmap_table.h"
#include "sfntly/data/memory_byte_array.h"
#include "sfntly/table/core/font_header_table.h"
#include "sfntly/tag.h"

#include "test/test_utils.h"
#include "test/test_font_utils.h"
#include "test/test_data.h"

#if GTEST_HAS_PARAM_TEST

namespace sfntly {
using ::testing::TestWithParam;
using ::testing::Values;

class CMapTestCase {
 public:
  CMapTestCase(const char* font_name,
               int32_t first_platform_id,
               int32_t first_encoding_id,
               const char* first_charset_name,
               int32_t second_platform_id,
               int32_t second_encoding_id,
               const char* second_charset_name,
               int32_t low_char,
               int32_t high_char)
      : font_name_(font_name),
        first_platform_id_(first_platform_id),
        first_encoding_id_(first_encoding_id),
        first_charset_name_(first_charset_name),
        second_platform_id_(second_platform_id),
        second_encoding_id_(second_encoding_id),
        second_charset_name_(second_charset_name),
        low_char_(low_char),
        high_char_(high_char) {
  }

  const char* font_name() const { return font_name_; }
  int32_t first_platform_id() const { return first_platform_id_; }
  int32_t first_encoding_id() const { return first_encoding_id_; }
  const char* first_charset_name() const { return first_charset_name_; }
  int32_t second_platform_id() const { return second_platform_id_; }
  int32_t second_encoding_id() const { return second_encoding_id_; }
  const char* second_charset_name() const { return second_charset_name_; }
  int32_t low_char() const { return low_char_; }
  int32_t high_char() const { return high_char_; }

 private:
  const char* font_name_;
  int32_t first_platform_id_;
  int32_t first_encoding_id_;
  const char* first_charset_name_;
  int32_t second_platform_id_;
  int32_t second_encoding_id_;
  const char* second_charset_name_;
  int32_t low_char_;
  int32_t high_char_;
};

class CMapTests : public :: testing::TestWithParam<CMapTestCase> {
 public:
  CMapTests() : encoder1_(NULL), encoder2_(NULL), successful_setup_(false) {
  }
  virtual void SetUp() {}
  virtual void TearDown();

  void CommonSetUp(FontArray* font_array);

  void CompareCMaps();

  Ptr<CMapTable::CMap> cmap1_;
  Ptr<CMapTable::CMap> cmap2_;
  UConverter* encoder1_;
  UConverter* encoder2_;
  bool successful_setup_;
};

::std::ostream& operator<<(::std::ostream& os, const CMapTestCase *test_case) {
  return os << "("
            << test_case->font_name() << ", "
            << test_case->first_platform_id() << ", "
            << test_case->first_encoding_id() << ", "
            << test_case->first_charset_name() << ", "
            << test_case->second_platform_id() << ", "
            << test_case->second_encoding_id() << ", "
            << test_case->second_charset_name() << ", "
            << test_case->low_char() << ", "
            << test_case->high_char() << ")";
}

void CMapTests::CommonSetUp(FontArray* font_array) {
  ASSERT_NE(font_array, reinterpret_cast<FontArray*>(NULL));
  ASSERT_FALSE(font_array->empty());
  Ptr<Font> font;
  font = font_array->at(0);
  ASSERT_NE(font, reinterpret_cast<Font*>(NULL));
  Ptr<CMapTable> cmap_table =
      down_cast<CMapTable*>(font->GetTable(Tag::cmap));
  cmap1_.Attach(cmap_table->GetCMap(GetParam().first_platform_id(),
                                    GetParam().first_encoding_id()));
  ASSERT_NE((cmap1_), reinterpret_cast<CMapTable::CMap*>(NULL));
  cmap2_.Attach(cmap_table->GetCMap(GetParam().second_platform_id(),
                                    GetParam().second_encoding_id()));
  ASSERT_NE((cmap2_), reinterpret_cast<CMapTable::CMap*>(NULL));
  encoder1_ = TestUtils::GetEncoder(GetParam().first_charset_name());
  encoder2_ = TestUtils::GetEncoder(GetParam().second_charset_name());
  successful_setup_ = true;
}

void CMapTests::TearDown() {
  if (encoder1_)
    ucnv_close(encoder1_);
  if (encoder2_)
    ucnv_close(encoder2_);
}

void CMapTests::CompareCMaps() {
  ASSERT_TRUE(successful_setup_);
  for (int32_t uchar = GetParam().low_char();
       uchar <= GetParam().high_char(); ++uchar) {
    int32_t c1 = uchar;
    if (encoder1_ != NULL)
      c1 = TestUtils::EncodeOneChar(encoder1_, (int16_t)uchar);
    int32_t c2 = uchar;
    if (encoder2_ != NULL)
      c2 = TestUtils::EncodeOneChar(encoder2_, (int16_t)uchar);
    int32_t glyph_id1 = cmap1_->GlyphId(c1);
    int32_t glyph_id2 = cmap2_->GlyphId(c2);
#ifdef SFNTLY_DEBUG_CMAP
    if (glyph_id1 != glyph_id2)
      fprintf(stderr, "%x: g1=%x, %x: g2=%x\n", c1, glyph_id1, c2, glyph_id2);
#endif
    ASSERT_EQ(glyph_id1, glyph_id2);
  }
#ifdef SFNTLY_SFNTLY_DEBUG_CMAPCMAP
  fprintf(stderr, "\n");
#endif
}

TEST_P(CMapTests, GlyphsBetweenCMapsFingerprint) {
  Ptr<FontFactory> font_factory;
  font_factory.Attach(FontFactory::GetInstance());
  font_factory->FingerprintFont(true);
  FontArray font_array;
  LoadFont(GetParam().font_name(), font_factory, &font_array);
  CommonSetUp(&font_array);
  CompareCMaps();
}

TEST_P(CMapTests, GlyphsBetweenCMapsNoFingerprint) {
  Ptr<FontFactory> font_factory;
  font_factory.Attach(FontFactory::GetInstance());
  FontArray font_array;
  LoadFont(GetParam().font_name(), font_factory, &font_array);
  CommonSetUp(&font_array);
  CompareCMaps();
}

TEST_P(CMapTests, GlyphsBetweenCMapsUsingByteVector) {
  FontArray font_array;
  LoadFontUsingByteVector(GetParam().font_name(), true, &font_array);
  CommonSetUp(&font_array);
  CompareCMaps();
}

CMapTestCase kCMapTestsTestCases[] = {
  CMapTestCase(SAMPLE_TTF_FILE,
               PlatformId::kWindows,
               WindowsEncodingId::kUnicodeUCS2,
               NULL,
               PlatformId::kUnicode,
               UnicodeEncodingId::kUnicode2_0_BMP,
               NULL,
               (int32_t)0x20,
               (int32_t)0x7f),
};

INSTANTIATE_TEST_CASE_P(CMapTests,
                        CMapTests,
                        ::testing::ValuesIn(kCMapTestsTestCases));
}

#else

TEST(DummyTest, ValueParameterizedTestsAreNotSupportedOnThisPlatform) {}

#endif  // GTEST_HAS_PARAM
