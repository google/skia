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
#include "sfntly/font_factory.h"
#include "sfntly/table/core/font_header_table.h"
#include "sfntly/tag.h"
#include "sfntly/data/memory_byte_array.h"
#include "sfntly/port/endian.h"
#include "sfntly/port/file_input_stream.h"
#include "sfntly/port/memory_output_stream.h"
#include "test/test_data.h"
#include "test/test_font_utils.h"

namespace sfntly {

bool TestOTFBasicEditing() {
  FontFactoryPtr factory;
  factory.Attach(FontFactory::GetInstance());
  FontBuilderArray font_builder_array;
  BuilderForFontFile(SAMPLE_TTF_FILE, factory, &font_builder_array);
  FontBuilderPtr font_builder = font_builder_array[0];

  // ensure the builder is not bogus
  EXPECT_TRUE(font_builder != NULL);
  TableBuilderMap* builder_map = font_builder->table_builders();
  EXPECT_TRUE(builder_map != NULL);
  IntegerSet builder_tags;
  for (TableBuilderMap::iterator i = builder_map->begin(),
                                 e = builder_map->end(); i != e; ++i) {
    EXPECT_TRUE(i->second != NULL);
    if (i->second == NULL) {
      char tag[5] = {0};
      int32_t value = ToBE32(i->first);
      memcpy(tag, &value, 4);
      fprintf(stderr, "tag %s does not have valid builder\n", tag);
    } else {
      builder_tags.insert(i->first);
    }
  }

  FontHeaderTableBuilderPtr header_builder =
      down_cast<FontHeaderTable::Builder*>(
          font_builder->GetTableBuilder(Tag::head));
  int64_t mod_date = header_builder->Modified();
  header_builder->SetModified(mod_date + 1);
  FontPtr font;
  font.Attach(font_builder->Build());

  // ensure every table had a builder
  const TableMap* table_map = font->GetTableMap();
  for (TableMap::const_iterator i = table_map->begin(), e = table_map->end();
                                i != e; ++i) {
    TablePtr table = (*i).second;
    HeaderPtr header = table->header();
    EXPECT_TRUE(builder_tags.find(header->tag()) != builder_tags.end());
    builder_tags.erase(header->tag());
  }
  EXPECT_TRUE(builder_tags.empty());

  FontHeaderTablePtr header =
      down_cast<FontHeaderTable*>(font->GetTable(Tag::head));
  int64_t after_mod_date = header->Modified();
  EXPECT_EQ(mod_date + 1, after_mod_date);

  // Checksum correctness of builder.
  TablePtr post = font->GetTable(Tag::post);
  EXPECT_EQ(post->CalculatedChecksum(), TTF_CHECKSUM[SAMPLE_TTF_POST]);
  return true;
}

}  // namespace sfntly

TEST(OTFBasicEditing, All) {
  ASSERT_TRUE(sfntly::TestOTFBasicEditing());
}
