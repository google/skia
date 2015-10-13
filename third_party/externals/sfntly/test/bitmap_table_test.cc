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
#include "sfntly/port/file_input_stream.h"
#include "sfntly/port/memory_input_stream.h"
#include "sfntly/port/memory_output_stream.h"
#include "sfntly/table/bitmap/ebdt_table.h"
#include "sfntly/table/bitmap/eblc_table.h"
#include "sfntly/table/bitmap/index_sub_table_format3.h"
#include "sfntly/table/bitmap/index_sub_table_format4.h"
#include "test/test_data.h"
#include "test/test_font_utils.h"

namespace sfntly {

const int32_t NUM_STRIKES = 4;
const int32_t STRIKE1_ARRAY_OFFSET = 0xc8;
const int32_t STRIKE1_INDEX_TABLE_SIZE = 0x4f4;
const int32_t STRIKE1_NUM_INDEX_TABLES = 1;
const int32_t STRIKE1_COLOR_REF = 0;
const int32_t STRIKE1_START_GLYPH_INDEX = 0;
const int32_t STRIKE1_END_GLYPH_INDEX = 623;
const int32_t STRIKE1_PPEM_X = 10;
const int32_t STRIKE1_PPEM_Y = 10;
const int32_t STRIKE1_BIT_DEPTH = 1;
const int32_t STRIKE1_FLAGS = 0x01;

const int32_t STRIKE4_SUB1_INDEX_FORMAT = 3;
const int32_t STRIKE4_SUB1_IMAGE_FORMAT = 1;
const int32_t STRIKE4_SUB1_IMAGE_DATA_OFFSET = 0x00005893;
const int32_t STRIKE4_SUB1_GLYPH_OFFSET[] = {
    0x00005893, 0x00005898, 0x0000589d, 0x000058a2, 0x000058a7,
    0x000058b2, 0x000058c2, 0x000058d0, 0x000058de, 0x000058e6 };
const int32_t NUM_STRIKE4_SUB1_GLYPH_OFFSET = 10;
const int32_t STRIKE4_SUB1_GLYPH2_LENGTH = 0x58a2 - 0x589d;

bool CommonReadingTest(Font* raw_font) {
  FontPtr font = raw_font;

  EblcTablePtr bitmap_loca = down_cast<EblcTable*>(font->GetTable(Tag::EBLC));
  EbdtTablePtr bitmap_table = down_cast<EbdtTable*>(font->GetTable(Tag::EBDT));

  EXPECT_FALSE(bitmap_loca == NULL);
  EXPECT_FALSE(bitmap_table == NULL);

  EXPECT_EQ(bitmap_loca->NumSizes(), NUM_STRIKES);

  // Strike 1
  BitmapSizeTablePtr strike1 = bitmap_loca->GetBitmapSizeTable(0);
  EXPECT_FALSE(strike1 == NULL);
  EXPECT_EQ(strike1->IndexSubTableArrayOffset(), STRIKE1_ARRAY_OFFSET);
  EXPECT_EQ(strike1->NumberOfIndexSubTables(), STRIKE1_NUM_INDEX_TABLES);
  EXPECT_EQ(strike1->ColorRef(), STRIKE1_COLOR_REF);
  EXPECT_EQ(strike1->StartGlyphIndex(), STRIKE1_START_GLYPH_INDEX);
  EXPECT_EQ(strike1->EndGlyphIndex(), STRIKE1_END_GLYPH_INDEX);
  EXPECT_EQ(strike1->PpemX(), STRIKE1_PPEM_X);
  EXPECT_EQ(strike1->PpemY(), STRIKE1_PPEM_Y);
  EXPECT_EQ(strike1->BitDepth(), STRIKE1_BIT_DEPTH);
  EXPECT_EQ(strike1->FlagsAsInt(), STRIKE1_FLAGS);

  // Strike 4
  // In this test font, all strikes and all subtables have same glyphs.
  BitmapSizeTablePtr strike4 = bitmap_loca->GetBitmapSizeTable(3);
  EXPECT_FALSE(strike4 == NULL);
  EXPECT_EQ(strike4->StartGlyphIndex(), STRIKE1_START_GLYPH_INDEX);
  EXPECT_EQ(strike4->EndGlyphIndex(), STRIKE1_END_GLYPH_INDEX);
  IndexSubTablePtr sub1 = strike4->GetIndexSubTable(0);
  EXPECT_FALSE(sub1 == NULL);
  EXPECT_EQ(sub1->image_format(), STRIKE4_SUB1_IMAGE_FORMAT);
  EXPECT_EQ(sub1->first_glyph_index(), STRIKE1_START_GLYPH_INDEX);
  EXPECT_EQ(sub1->last_glyph_index(), STRIKE1_END_GLYPH_INDEX);
  EXPECT_EQ(sub1->image_data_offset(), STRIKE4_SUB1_IMAGE_DATA_OFFSET);

  for (int32_t i = 0; i < NUM_STRIKE4_SUB1_GLYPH_OFFSET; ++i) {
      EXPECT_EQ(sub1->GlyphOffset(i), STRIKE4_SUB1_GLYPH_OFFSET[i]);
  }
  return true;
}

bool TestReadingBitmapTable() {
  FontFactoryPtr factory;
  factory.Attach(FontFactory::GetInstance());
  FontArray font_array;
  LoadFont(SAMPLE_BITMAP_FONT, factory, &font_array);
  FontPtr font = font_array[0];
  EXPECT_TRUE(CommonReadingTest(font));

  EblcTablePtr bitmap_loca = down_cast<EblcTable*>(font->GetTable(Tag::EBLC));
  BitmapSizeTablePtr strike1 = bitmap_loca->GetBitmapSizeTable(0);
  BitmapSizeTablePtr strike4 = bitmap_loca->GetBitmapSizeTable(3);
  IndexSubTablePtr sub1 = strike4->GetIndexSubTable(0);

  EXPECT_EQ(strike1->IndexTableSize(), STRIKE1_INDEX_TABLE_SIZE);
  EXPECT_EQ(sub1->index_format(), STRIKE4_SUB1_INDEX_FORMAT);

  // Strike 4 Index Sub Table 1 is a Format 3
  IndexSubTableFormat3Ptr sub3 =
      down_cast<IndexSubTableFormat3*>(strike4->GetIndexSubTable(0));
  EXPECT_FALSE(sub3 == NULL);
  BitmapGlyphInfoPtr info;
  info.Attach(sub3->GlyphInfo(2));
  EXPECT_EQ(info->glyph_id(), 2);
  EXPECT_EQ(info->block_offset(), STRIKE4_SUB1_IMAGE_DATA_OFFSET);
  EXPECT_EQ(info->start_offset(),
            STRIKE4_SUB1_GLYPH_OFFSET[2] - STRIKE4_SUB1_GLYPH_OFFSET[0]);
  EXPECT_EQ(info->format(), STRIKE4_SUB1_IMAGE_FORMAT);
  EXPECT_EQ(info->length(), STRIKE4_SUB1_GLYPH2_LENGTH);

  return true;
}

// Function in subset_impl.cc
extern
void SubsetEBLC(EblcTable::Builder* eblc, const BitmapLocaList& new_loca);

bool TestIndexFormatConversion() {
  FontFactoryPtr factory;
  factory.Attach(FontFactory::GetInstance());
  FontBuilderArray builder_array;
  BuilderForFontFile(SAMPLE_BITMAP_FONT, factory, &builder_array);

  FontBuilderPtr font_builder;
  font_builder = builder_array[0];
  EblcTableBuilderPtr eblc_builder =
      down_cast<EblcTable::Builder*>(font_builder->GetTableBuilder(Tag::EBLC));
  BitmapLocaList new_loca;
  eblc_builder->GenerateLocaList(&new_loca);
  SubsetEBLC(eblc_builder, new_loca);  // Format 3 -> 4

  FontPtr new_font;
  new_font.Attach(font_builder->Build());

  // Serialize and reload the serialized font.
  MemoryOutputStream os;
  factory->SerializeFont(new_font, &os);

#if defined (SFNTLY_DEBUG_BITMAP)
  SerializeToFile(&os, "anon-mod.ttf");
#endif

  MemoryInputStream is;
  is.Attach(os.Get(), os.Size());
  FontArray font_array;
  factory->LoadFonts(&is, &font_array);
  new_font = font_array[0];

  EXPECT_TRUE(CommonReadingTest(new_font));

  // Strike 4 Index Sub Table 1 is a Format 4
  EblcTablePtr bitmap_loca =
      down_cast<EblcTable*>(new_font->GetTable(Tag::EBLC));
  BitmapSizeTablePtr strike4 = bitmap_loca->GetBitmapSizeTable(3);
  IndexSubTableFormat4Ptr sub4 =
      down_cast<IndexSubTableFormat4*>(strike4->GetIndexSubTable(0));
  EXPECT_FALSE(sub4 == NULL);

  // And this subtable shall have exactly the same offset as original table
  // since no subsetting happens.
  FontArray original_font_array;
  LoadFont(SAMPLE_BITMAP_FONT, factory, &original_font_array);
  FontPtr font = original_font_array[0];
  EXPECT_FALSE(font == NULL);
  EblcTablePtr original_loca = down_cast<EblcTable*>(font->GetTable(Tag::EBLC));
  EXPECT_FALSE(original_loca == NULL);
  BitmapSizeTablePtr original_strike4 = bitmap_loca->GetBitmapSizeTable(3);
  EXPECT_FALSE(original_strike4 == NULL);
  IndexSubTableFormat3Ptr sub3 =
      down_cast<IndexSubTableFormat3*>(strike4->GetIndexSubTable(0));
  EXPECT_FALSE(sub3 == NULL);
  EXPECT_EQ(strike4->StartGlyphIndex(), original_strike4->StartGlyphIndex());
  EXPECT_EQ(strike4->EndGlyphIndex(), original_strike4->EndGlyphIndex());
  for (int32_t i = strike4->StartGlyphIndex();
               i <= strike4->EndGlyphIndex(); ++i) {
    BitmapGlyphInfoPtr info, original_info;
    info.Attach(sub4->GlyphInfo(i));
    original_info.Attach(sub3->GlyphInfo(i));
    EXPECT_EQ(info->format(), original_info->format());
    EXPECT_EQ(info->glyph_id(), original_info->glyph_id());
    EXPECT_EQ(info->length(), original_info->length());
    EXPECT_EQ(info->offset(), original_info->offset());
  }

  return true;
}

}  // namespace sfntly

TEST(BitmapTable, Reading) {
  ASSERT_TRUE(sfntly::TestReadingBitmapTable());
}

TEST(BitmapTable, IndexFormatConversion) {
  ASSERT_TRUE(sfntly::TestIndexFormatConversion());
}
