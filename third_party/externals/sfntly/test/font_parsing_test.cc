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

#include "sfntly/data/font_input_stream.h"
#include "sfntly/data/memory_byte_array.h"
#include "sfntly/font.h"
#include "sfntly/font_factory.h"
#include "sfntly/table/core/font_header_table.h"
#include "sfntly/table/table.h"
#include "sfntly/table/generic_table_builder.h"
#include "sfntly/table/table_based_table_builder.h"
#include "sfntly/tag.h"
#include "sfntly/port/file_input_stream.h"
#include "test/test_data.h"
#include "test/test_font_utils.h"

namespace sfntly {

bool TestFontParsing() {
  ByteVector input_buffer;
  LoadFile(SAMPLE_TTF_FILE, &input_buffer);

  FontFactoryPtr factory;
  factory.Attach(FontFactory::GetInstance());
  // File based
  FontBuilderArray font_builder_array;
  BuilderForFontFile(SAMPLE_TTF_FILE, factory, &font_builder_array);
  FontBuilderPtr font_builder = font_builder_array[0];
  // Memory based
  FontBuilderArray font_builder_array2;
  factory->LoadFontsForBuilding(&input_buffer, &font_builder_array2);
  FontBuilderPtr font_builder2 = font_builder_array2[0];

  for (size_t i = 0; i < SAMPLE_TTF_KNOWN_TAGS; ++i) {
    EXPECT_TRUE(font_builder->HasTableBuilder(TTF_KNOWN_TAGS[i]));
    EXPECT_TRUE(font_builder2->HasTableBuilder(TTF_KNOWN_TAGS[i]));
  }

  // Generic table
  Ptr<GenericTableBuilder> gdef_builder =
      down_cast<GenericTableBuilder*>(font_builder->GetTableBuilder(Tag::feat));
  HeaderPtr gdef_header = gdef_builder->header();
  EXPECT_EQ(gdef_header->length(), TTF_LENGTH[SAMPLE_TTF_FEAT]);
  EXPECT_EQ(gdef_header->offset(), TTF_OFFSET[SAMPLE_TTF_FEAT]);
  EXPECT_EQ(gdef_header->checksum(), TTF_CHECKSUM[SAMPLE_TTF_FEAT]);
  EXPECT_TRUE(gdef_header->checksum_valid());

  WritableFontDataPtr wfd;
  wfd.Attach(gdef_builder->Data());
  ByteVector b;
  b.resize(TTF_LENGTH[SAMPLE_TTF_FEAT]);
  wfd->ReadBytes(0, &(b[0]), 0, TTF_LENGTH[SAMPLE_TTF_FEAT]);
  EXPECT_EQ(memcmp(&(b[0]), TTF_FEAT_DATA, TTF_LENGTH[SAMPLE_TTF_FEAT]), 0);

  // Header table
  FontHeaderTableBuilderPtr header_builder =
      down_cast<FontHeaderTable::Builder*>(
          font_builder->GetTableBuilder(Tag::head));
  HeaderPtr header_header = header_builder->header();
  EXPECT_EQ(header_header->length(), TTF_LENGTH[SAMPLE_TTF_HEAD]);
  EXPECT_EQ(header_header->offset(), TTF_OFFSET[SAMPLE_TTF_HEAD]);
  EXPECT_EQ(header_header->checksum(), TTF_CHECKSUM[SAMPLE_TTF_HEAD]);
  EXPECT_TRUE(header_header->checksum_valid());

  // Data conformance
  for (size_t i = 0; i < SAMPLE_TTF_KNOWN_TAGS; ++i) {
    ByteVector b1, b2;
    b1.resize(TTF_LENGTH[i]);
    b2.resize(TTF_LENGTH[i]);
    TableBuilderPtr builder1 =
        font_builder->GetTableBuilder(TTF_KNOWN_TAGS[i]);
    TableBuilderPtr builder2 =
        font_builder2->GetTableBuilder(TTF_KNOWN_TAGS[i]);
    WritableFontDataPtr wfd1;
    wfd1.Attach(builder1->Data());
    WritableFontDataPtr wfd2;
    wfd2.Attach(builder2->Data());
    wfd1->ReadBytes(0, &(b1[0]), 0, TTF_LENGTH[i]);
    wfd2->ReadBytes(0, &(b2[0]), 0, TTF_LENGTH[i]);
    EXPECT_EQ(memcmp(&(b1[0]), &(b2[0]), TTF_LENGTH[i]), 0);
  }

  return true;
}

bool TestTTFReadWrite() {
  FontFactoryPtr factory;
  factory.Attach(FontFactory::GetInstance());
  FontBuilderArray font_builder_array;
  BuilderForFontFile(SAMPLE_TTF_FILE, factory, &font_builder_array);
  FontBuilderPtr font_builder = font_builder_array[0];
  FontPtr font;
  font.Attach(font_builder->Build());
  MemoryOutputStream output_stream;
  factory->SerializeFont(font, &output_stream);
  EXPECT_GE(output_stream.Size(), SAMPLE_TTF_SIZE);

  return true;
}

bool TestTTFMemoryBasedReadWrite() {
  ByteVector input_buffer;
  LoadFile(SAMPLE_TTF_FILE, &input_buffer);

  FontFactoryPtr factory;
  factory.Attach(FontFactory::GetInstance());
  FontBuilderArray font_builder_array;
  factory->LoadFontsForBuilding(&input_buffer, &font_builder_array);
  FontBuilderPtr font_builder = font_builder_array[0];
  FontPtr font;
  font.Attach(font_builder->Build());
  MemoryOutputStream output_stream;
  factory->SerializeFont(font, &output_stream);
  EXPECT_GE(output_stream.Size(), input_buffer.size());

  return true;
}

}  // namespace sfntly

TEST(FontParsing, All) {
  ASSERT_TRUE(sfntly::TestFontParsing());
  ASSERT_TRUE(sfntly::TestTTFReadWrite());
  ASSERT_TRUE(sfntly::TestTTFMemoryBasedReadWrite());
}
