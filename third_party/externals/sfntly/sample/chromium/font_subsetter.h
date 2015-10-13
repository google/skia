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
// File is originally from Chromium third_party/sfntly/src/subsetter.
// Use as test case in sfntly so that problems can be caught in upstream early.
#ifndef SFNTLY_CPP_SRC_TEST_FONT_SUBSETTER_H_
#define SFNTLY_CPP_SRC_TEST_FONT_SUBSETTER_H_

#include <stddef.h>

class SfntlyWrapper {
 public:

  // Font subsetting API
  //
  // Input TTF/TTC/OTF fonts, specify the glyph IDs to subset, and the subset
  // font is returned in |output_buffer| (caller to delete[]).  Return value is
  // the length of output_buffer allocated.
  //
  // If subsetting fails, a negative value is returned.  If none of the glyph
  // IDs specified is found, the function will return 0.
  //
  // |font_name|      Font name, required for TTC files.  If specified NULL,
  //                  the first available font is selected.
  // |original_font|  Original font file contents.
  // |font_size|      Size of |original_font| in bytes.
  // |glyph_ids|      Glyph IDs to subset.  If the specified glyph ID is not
  //                  found in the font file, it will be ignored silently.
  // |glyph_count|    Number of glyph IDs in |glyph_ids|
  // |output_buffer|  Generated subset font.  Caller to delete[].
  static int SubsetFont(const char* font_name,
                        const unsigned char* original_font,
                        size_t font_size,
                        const unsigned int* glyph_ids,
                        size_t glyph_count,
                        unsigned char** output_buffer);
};

#endif  // SFNTLY_CPP_SRC_TEST_FONT_SUBSETTER_H_
