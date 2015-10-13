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

#include <string.h>

#include <vector>
#include <string>
#include <algorithm>

#include "sfntly/font.h"
#include "sfntly/font_factory.h"
#include "sfntly/table/core/cmap_table.h"
#include "sfntly/tag.h"
#include "sfntly/port/type.h"
#include "sfntly/port/refcount.h"
#include "test/test_data.h"
#include "test/test_font_utils.h"

#include "gtest/gtest.h"

#if GTEST_HAS_PARAM_TEST

namespace sfntly {
using ::testing::TestWithParam;
using ::testing::Values;

typedef std::vector<bool> BitSet;

class CMapIteratorTestCase {
 public:
  CMapIteratorTestCase(int32_t platform_id, int32_t encoding_id,
                       const char* file_name)
      : platform_id_(platform_id),
        encoding_id_(encoding_id),
        file_name_(file_name) {
  }
  ~CMapIteratorTestCase() {}
  int32_t platform_id() const { return platform_id_; }
  int32_t encoding_id() const { return encoding_id_; }
  const char* file_name() const { return file_name_; }

 private:
  int32_t platform_id_;
  int32_t encoding_id_;
  const char* file_name_;
};

class CMapIteratorTests
    : public ::testing::TestWithParam<CMapIteratorTestCase> {
 public:
  virtual void SetUp();
  virtual void TearDown() {}

  BitSet* GenerateCMapEntries(int32_t start, int32_t count);
  int32_t CompareCMapIterAndBitSet(CMapTable::CMap::CharacterIterator*
                                   character_iterator,
                                   BitSet* bit_set);

  Ptr<CMapTable::CMap> cmap_;
};

void CMapIteratorTests::SetUp() {
  FontArray fonts;
  Ptr<FontFactory> font_factory;
  const char* file_name = GetParam().file_name();
  LoadFont(file_name, font_factory, &fonts);
  Ptr<Font> font;
  font.Attach(fonts[0].Detach());
  Ptr<CMapTable> cmap_table = down_cast<CMapTable*>(font->GetTable(Tag::cmap));
  ASSERT_FALSE(cmap_table == NULL);
  cmap_.Attach(cmap_table->GetCMap(GetParam().platform_id(),
                                   GetParam().encoding_id()));
  ASSERT_FALSE(cmap_ == NULL);
}

BitSet* CMapIteratorTests::GenerateCMapEntries(int32_t start, int32_t count) {
  BitSet* entries = new BitSet(count);
  for (int32_t c = start; c < start + count; ++c) {
    int32_t g = cmap_->GlyphId(c);
    if (g != CMapTable::NOTDEF)
      (*entries)[c] = true;
  }
  return entries;
}

int32_t
CMapIteratorTests::
CompareCMapIterAndBitSet(CMapTable::CMap::CharacterIterator* character_iterator,
                         BitSet* bit_set) {
  int32_t iterator_not_bitset_count = 0;
  BitSet::iterator end = bit_set->end(),
      beginning = bit_set->begin(),
      init_beginning = beginning,
      current = std::find(beginning, end, true);
  for (int32_t next_bit = current - beginning;
       character_iterator->HasNext() && current != end;
       next_bit = current - init_beginning) {
    int32_t c = character_iterator->Next();
    EXPECT_TRUE(c <= next_bit || current == end);
    if (!(c <= next_bit || current == end))
      return -1;
    if (c == next_bit) {
      beginning = current + 1;
      current = std::find(beginning, end, true);
    } else {
      iterator_not_bitset_count++;
    }
  }
  EXPECT_EQ(end, current);
#if defined (SFNTLY_DEBUG_CMAP)
  fprintf(stderr, "%s %d: Differences between iterator and bitset: %d\n",
          cmap_->format(), GetParam().file_name(), iterator_not_bitset_count);
#endif
  return iterator_not_bitset_count;
}

TEST_P(CMapIteratorTests, IteratorTest) {
  BitSet* bit_set = GenerateCMapEntries(0, 0x10ffff);
  CMapTable::CMap::CharacterIterator* character_iterator = NULL;
  character_iterator = cmap_->Iterator();
  EXPECT_NE(character_iterator,
            reinterpret_cast<CMapTable::CMap::CharacterIterator*>(NULL));
  CompareCMapIterAndBitSet(character_iterator, bit_set);
  delete character_iterator;
  delete bit_set;
}

CMapIteratorTestCase kCMapIteratorTestsTestCases[] = {
  CMapIteratorTestCase(CMapTable::WINDOWS_BMP.platform_id,
                       CMapTable::WINDOWS_BMP.encoding_id,
                       SAMPLE_TTF_FILE)
};

INSTANTIATE_TEST_CASE_P(CMapIteratorTests,
                        CMapIteratorTests,
                        ::testing::ValuesIn(kCMapIteratorTestsTestCases));
}

#else

TEST(DummyTest, ValueParameterizedTestsAreNotSupportedOnThisPlatform) {}

#endif  // GTEST_HAS_PARAM
