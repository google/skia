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

// Remove VC++ nag on fopen.
#define _CRT_SECURE_NO_WARNINGS

#include "sample/subsetter/subset_util.h"

#include <stdio.h>

#include <vector>
#include <memory>

#include "sfntly/font.h"
#include "sfntly/data/memory_byte_array.h"
#include "sfntly/port/memory_output_stream.h"
#include "sfntly/port/type.h"
#include "sfntly/tag.h"
#include "sfntly/tools/subsetter/subsetter.h"

namespace sfntly {

SubsetUtil::SubsetUtil() {
}

SubsetUtil::~SubsetUtil() {
}

void SubsetUtil::Subset(const char *input_file_path,
                        const char *output_file_path) {
  UNREFERENCED_PARAMETER(output_file_path);
  ByteVector input_buffer;
  FILE* input_file = fopen(input_file_path, "rb");
  if (input_file == NULL) {
    fprintf(stderr, "file not found\n");
    return;
  }
  fseek(input_file, 0, SEEK_END);
  size_t file_size = ftell(input_file);
  fseek(input_file, 0, SEEK_SET);
  input_buffer.resize(file_size);
  size_t bytes_read = fread(&(input_buffer[0]), 1, file_size, input_file);
  UNREFERENCED_PARAMETER(bytes_read);
  fclose(input_file);

  FontFactoryPtr factory;
  factory.Attach(FontFactory::GetInstance());

  FontArray font_array;
  factory->LoadFonts(&input_buffer, &font_array);
  if (font_array.empty() || font_array[0] == NULL)
    return;

  IntegerList glyphs;
  for (int32_t i = 0; i < 10; i++) {
    glyphs.push_back(i);
  }
  glyphs.push_back(11);
  glyphs.push_back(10);

  Ptr<Subsetter> subsetter = new Subsetter(font_array[0], factory);
  subsetter->SetGlyphs(&glyphs);
  IntegerSet remove_tables;
  remove_tables.insert(Tag::DSIG);
  subsetter->SetRemoveTables(&remove_tables);

  FontBuilderPtr font_builder;
  font_builder.Attach(subsetter->Subset());

  FontPtr new_font;
  new_font.Attach(font_builder->Build());

  // TODO(arthurhsu): glyph renumbering/Loca table
  // TODO(arthurhsu): alter CMaps

  MemoryOutputStream output_stream;
  factory->SerializeFont(new_font, &output_stream);

  FILE* output_file = fopen(output_file_path, "wb");
  fwrite(output_stream.Get(), 1, output_stream.Size(), output_file);
  fflush(output_file);
  fclose(output_file);
}

}  // namespace sfntly
