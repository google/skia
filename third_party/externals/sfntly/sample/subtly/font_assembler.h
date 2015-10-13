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

#ifndef TYPOGRAPHY_FONT_SFNTLY_SRC_SAMPLE_SUBTLY_FONT_ASSEMBLER_H_
#define TYPOGRAPHY_FONT_SFNTLY_SRC_SAMPLE_SUBTLY_FONT_ASSEMBLER_H_

#include <set>
#include <map>

#include "subtly/font_info.h"

#include "sfntly/tag.h"
#include "sfntly/font.h"
#include "sfntly/port/type.h"
#include "sfntly/port/refcount.h"
#include "sfntly/table/core/cmap_table.h"
#include "sfntly/table/truetype/glyph_table.h"
#include "sfntly/table/truetype/loca_table.h"

namespace subtly {
// Assembles FontInfo into font builders.
// Does not take ownership of data passed to it.
class FontAssembler : public sfntly::RefCounted<FontAssembler> {
 public:
  // font_info is the FontInfo which will be used for the new font
  // table_blacklist is used to decide which tables to exclude from the
  // final font.
  FontAssembler(FontInfo* font_info, sfntly::IntegerSet* table_blacklist);
  explicit FontAssembler(FontInfo* font_info);
  ~FontAssembler() { }

  // Assemble a new font from the font info object.
  virtual CALLER_ATTACH sfntly::Font* Assemble();

  sfntly::IntegerSet* table_blacklist() const { return table_blacklist_; }
  void set_table_blacklist(sfntly::IntegerSet* table_blacklist) {
    table_blacklist_ = table_blacklist;
  }

 protected:
  virtual bool AssembleCMapTable();
  virtual bool AssembleGlyphAndLocaTables();

  virtual void Initialize();

 private:
  sfntly::Ptr<FontInfo> font_info_;
  sfntly::Ptr<sfntly::FontFactory> font_factory_;
  sfntly::Ptr<sfntly::Font::Builder> font_builder_;
  sfntly::IntegerSet* table_blacklist_;
};
}

#endif  // TYPOGRAPHY_FONT_SFNTLY_SRC_SAMPLE_SUBTLY_FONT_ASSEMBLER_H_
