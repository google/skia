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

#include "font_subsetter.h"

#include "subsetter_impl.h"

int SfntlyWrapper::SubsetFont(const char* font_name,
                              const unsigned char* original_font,
                              size_t font_size,
                              const unsigned int* glyph_ids,
                              size_t glyph_count,
                              unsigned char** output_buffer) {
  if (output_buffer == NULL ||
      original_font == NULL || font_size == 0 ||
      glyph_ids == NULL || glyph_count == 0) {
    return 0;
  }

  sfntly::SubsetterImpl subsetter;
  if (!subsetter.LoadFont(font_name, original_font, font_size)) {
    return -1;  // Load error or font not found.
  }

  return subsetter.SubsetFont(glyph_ids, glyph_count, output_buffer);
}
