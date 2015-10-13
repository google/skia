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

#include "subtly/merger.h"

#include <stdio.h>

#include "sfntly/font.h"
#include "sfntly/font_factory.h"
#include "subtly/character_predicate.h"
#include "subtly/font_assembler.h"
#include "subtly/font_info.h"
#include "subtly/utils.h"

namespace subtly {
using namespace sfntly;

/******************************************************************************
 * Merger class
 ******************************************************************************/
Merger::Merger(FontArray* fonts) {
  if (!fonts) {
    return;
  }
  int32_t num_fonts = fonts->size();
  for (int32_t i = 0; i < num_fonts; ++i) {
    fonts_.insert(std::make_pair(i, fonts->at(i)));
  }
}

CALLER_ATTACH Font* Merger::Merge() {
  Ptr<FontInfo> merged_info;
  merged_info.Attach(MergeFontInfos());
  if (!merged_info) {
#if defined (SUBTLY_DEBUG)
    fprintf(stderr, "Could not create merged font info\n");
#endif
    return NULL;
  }
  Ptr<FontAssembler> font_assembler = new FontAssembler(merged_info);
  return font_assembler->Assemble();
}

CALLER_ATTACH FontInfo* Merger::MergeFontInfos() {
  Ptr<FontInfo> font_info = new FontInfo;
  font_info->set_fonts(&fonts_);
  for (FontIdMap::iterator it = fonts_.begin(),
           e = fonts_.end(); it != e; ++it) {
    Ptr<FontSourcedInfoBuilder> info_builder =
        new FontSourcedInfoBuilder(it->second, it->first, NULL);
    Ptr<FontInfo> current_font_info;
    current_font_info.Attach(info_builder->GetFontInfo());
    if (!current_font_info) {
#if defined (SUBTLY_DEBUG)
      fprintf(stderr, "Couldn't create font info. "
              "No subset will be generated.\n");
#endif
      return NULL;
    }
    font_info->chars_to_glyph_ids()->insert(
        current_font_info->chars_to_glyph_ids()->begin(),
        current_font_info->chars_to_glyph_ids()->end());
    font_info->resolved_glyph_ids()->insert(
        current_font_info->resolved_glyph_ids()->begin(),
        current_font_info->resolved_glyph_ids()->end());
#if defined (SUBTLY_DEBUG)
    fprintf(stderr, "Counts: chars_to_glyph_ids: %d; resoved_glyph_ids: %d\n",
            font_info->chars_to_glyph_ids()->size(),
            font_info->resolved_glyph_ids()->size());
#endif
  }
  return font_info.Detach();
}
}
