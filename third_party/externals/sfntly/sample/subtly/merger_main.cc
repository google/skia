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
#include "subtly/merger.h"
#include "subtly/stats.h"
#include "subtly/utils.h"

using namespace subtly;

void PrintUsage(const char* program_name) {
  fprintf(stdout, "Usage: %s <input_font_file1> <input_font_file2> ..."
          "<input_font_filen> <output_font_file>\n",
          program_name);
}

void CheckLoading(const char* font_path, Font* font) {
  if (!font || font->num_tables() == 0) {
    fprintf(stderr, "Could not load font %s. Terminating.\n", font_path);
    exit(1);
  }
}

int main(int argc, const char** argv) {
  if (argc < 3) {
    PrintUsage(argv[0]);
    exit(1);
  }

  FontArray fonts;
  for (int32_t i = 1; i < argc - 1; ++i) {
    Ptr<Font> font;
    font.Attach(LoadFont(argv[i]));
    CheckLoading(argv[i], font);
    fonts.push_back(font);
  }

  Ptr<Merger> merger = new Merger(&fonts);
  FontPtr new_font;
  new_font.Attach(merger->Merge());

  fprintf(stderr, "Serializing font to %s\n", argv[argc - 1]);
  SerializeFont(argv[argc - 1], new_font);
  if (!new_font) {
    fprintf(stdout, "Cannot create merged font.\n");
    return 1;
  }

  return 0;
}
