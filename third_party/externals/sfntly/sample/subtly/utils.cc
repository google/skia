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

#include "subtly/utils.h"

#include "sfntly/data/growable_memory_byte_array.h"
#include "sfntly/data/memory_byte_array.h"
#include "sfntly/font.h"
#include "sfntly/font_factory.h"
#include "sfntly/port/file_input_stream.h"
#include "sfntly/port/memory_output_stream.h"

namespace subtly {
using namespace sfntly;

CALLER_ATTACH Font* LoadFont(const char* font_path) {
  Ptr<FontFactory> font_factory;
  font_factory.Attach(FontFactory::GetInstance());
  FontArray fonts;
  LoadFonts(font_path, font_factory, &fonts);
  return fonts[0].Detach();
}

CALLER_ATTACH Font::Builder* LoadFontBuilder(const char* font_path) {
  FontFactoryPtr font_factory;
  font_factory.Attach(FontFactory::GetInstance());
  FontBuilderArray builders;
  LoadFontBuilders(font_path, font_factory, &builders);
  return builders[0].Detach();
}

void LoadFonts(const char* font_path, FontFactory* factory, FontArray* fonts) {
  FileInputStream input_stream;
  input_stream.Open(font_path);
  factory->LoadFonts(&input_stream, fonts);
  input_stream.Close();
}

void LoadFontBuilders(const char* font_path,
                      FontFactory* factory,
                      FontBuilderArray* builders) {
  FileInputStream input_stream;
  input_stream.Open(font_path);
  factory->LoadFontsForBuilding(&input_stream, builders);
  input_stream.Close();
}

bool SerializeFont(const char* font_path, Font* font) {
  if (!font_path)
    return false;
  FontFactoryPtr font_factory;
  font_factory.Attach(FontFactory::GetInstance());
  return SerializeFont(font_path, font_factory, font);
}

bool SerializeFont(const char* font_path, FontFactory* factory, Font* font) {
  if (!font_path || !factory || !font)
    return false;
  // Serializing the font to a stream.
  MemoryOutputStream output_stream;
  factory->SerializeFont(font, &output_stream);
  // Serializing the stream to a file.
  FILE* output_file = NULL;
#if defined WIN32
  fopen_s(&output_file, font_path, "wb");
#else
  output_file = fopen(font_path, "wb");
#endif
  if (output_file == reinterpret_cast<FILE*>(NULL))
    return false;
  for (size_t i = 0; i < output_stream.Size(); ++i) {
    fwrite(&(output_stream.Get()[i]), 1, 1, output_file);
  }
  fflush(output_file);
  fclose(output_file);
  return true;
}
};
