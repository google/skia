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

#include <map>
#include <algorithm>

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
#include "sfntly/table/core/cmap_table.h"
#include "sfntly/port/refcount.h"
#include "gtest/gtest.h"

namespace sfntly {
TEST(CMapEditingTest, RemoveAllButOneCMap) {
  FontBuilderArray builders;
  FontFactoryPtr font_factory;
  font_factory.Attach(FontFactory::GetInstance());
  BuilderForFontFile(SAMPLE_TTF_FILE, font_factory, &builders);
  ASSERT_FALSE(builders.empty());
  FontBuilderPtr font_builder = builders[0];
  Ptr<CMapTable::Builder> cmap_table_builder =
      (CMapTable::Builder*)font_builder->GetTableBuilder(Tag::cmap);
  ASSERT_NE(cmap_table_builder, reinterpret_cast<CMapTable::Builder*>(NULL));
  CMapTable::CMapBuilderMap*
      cmap_builders = cmap_table_builder->GetCMapBuilders();
  ASSERT_FALSE(cmap_builders->empty());

  for (CMapTable::CMapBuilderMap::iterator
           it = cmap_builders->begin(); it != cmap_builders->end();) {
    if (it->second->cmap_id() == CMapTable::WINDOWS_BMP) {
      ++it;
    } else {
      cmap_builders->erase(it++);
    }
  }
  ASSERT_EQ(cmap_builders->size(), (uint32_t)1);
  Font* font = font_builder->Build();
  CMapTablePtr cmap_table = down_cast<CMapTable*>(font->GetTable(Tag::cmap));
  ASSERT_EQ(1, cmap_table->NumCMaps());
  CMapTable::CMapPtr cmap;
  cmap.Attach(cmap_table->GetCMap(CMapTable::WINDOWS_BMP));
  ASSERT_EQ(CMapTable::WINDOWS_BMP, cmap->cmap_id());
  delete font;
}

TEST(CMapEditingTest, CopyAllCMapsToNewFont) {
  FontArray fonts;
  FontFactoryPtr font_factory;
  font_factory.Attach(FontFactory::GetInstance());
  LoadFont(SAMPLE_TTF_FILE, font_factory, &fonts);

  ASSERT_FALSE(fonts.empty());
  ASSERT_FALSE(fonts[0] == NULL);
  FontPtr font = fonts[0];
  CMapTablePtr cmap_table = down_cast<CMapTable*>(font->GetTable(Tag::cmap));
  FontBuilderPtr font_builder;
  font_builder.Attach(font_factory->NewFontBuilder());
  Ptr<CMapTable::Builder> cmap_table_builder =
      (CMapTable::Builder*)font_builder->NewTableBuilder(Tag::cmap);

  CMapTable::CMapIterator cmap_iter(cmap_table, NULL);
  while (cmap_iter.HasNext()) {
    CMapTable::CMapPtr cmap;
    cmap.Attach(cmap_iter.Next());
    if (!cmap)
      continue;
    cmap_table_builder->NewCMapBuilder(cmap->cmap_id(), cmap->ReadFontData());
  }

  FontPtr new_font;
  new_font.Attach(font_builder->Build());
  CMapTablePtr new_cmap_table =
      down_cast<CMapTable*>(font->GetTable(Tag::cmap));
  ASSERT_EQ(cmap_table->NumCMaps(), new_cmap_table->NumCMaps());
  CMapTable::CMapPtr cmap;
  cmap.Attach(cmap_table->GetCMap(CMapTable::WINDOWS_BMP));
  ASSERT_NE(cmap, reinterpret_cast<CMapTable::CMap*>(NULL));
  ASSERT_EQ(CMapTable::WINDOWS_BMP, cmap->cmap_id());
}
}
