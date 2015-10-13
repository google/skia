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

#include <stdio.h>
#include <stdlib.h>

#include <map>
#include <utility>

#include "sfntly/font.h"
#include "subtly/character_predicate.h"
#include "subtly/stats.h"
#include "subtly/subsetter.h"
#include "subtly/utils.h"

using namespace subtly;

void PrintUsage(const char* program_name) {
  fprintf(stdout, "Usage: %s <input_font_file> <output_font_file>"
          "<start_char> <end_char>\n", program_name);
}

int main(int argc, const char** argv) {
  const char* program_name = argv[0];
  if (argc < 5) {
    PrintUsage(program_name);
    exit(1);
  }

  const char* input_font_path = argv[1];
  const char* output_font_path = argv[2];
  FontPtr font;
  font.Attach(subtly::LoadFont(input_font_path));
  if (font->num_tables() == 0) {
    fprintf(stderr, "Could not load font %s.\n", input_font_path);
    exit(1);
  }

  const char* start_char = argv[3];
  const char* end_char = argv[4];
  if (start_char[1] != 0) {
    fprintf(stderr, "Start character %c invalid.\n", start_char[0]);
    exit(1);
  }
  if (end_char[1] != 0) {
    fprintf(stderr, "Start character %c invalid.\n", end_char[0]);
    exit(1);
  }
  int32_t original_size = TotalFontSize(font);


  Ptr<CharacterPredicate> range_predicate =
      new AcceptRange(start_char[0], end_char[0]);
  Ptr<Subsetter> subsetter = new Subsetter(font, range_predicate);
  Ptr<Font> new_font;
  new_font.Attach(subsetter->Subset());
  if (!new_font) {
    fprintf(stdout, "Cannot create subset.\n");
    return 0;
  }

  subtly::SerializeFont(output_font_path, new_font);
  subtly::PrintComparison(stdout, font, new_font);
  int32_t new_size = TotalFontSize(new_font);
  fprintf(stdout, "Went from %d to %d: %lf%% of original\n",
          original_size, new_size,
          static_cast<double>(new_size) / original_size * 100);
  return 0;
}
