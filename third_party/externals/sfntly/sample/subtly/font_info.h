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

#ifndef TYPOGRAPHY_FONT_SFNTLY_SRC_SAMPLE_SUBTLY_FONT_INFO_H_
#define TYPOGRAPHY_FONT_SFNTLY_SRC_SAMPLE_SUBTLY_FONT_INFO_H_

#include <map>
#include <set>

#include "sfntly/font.h"
#include "sfntly/port/type.h"
#include "sfntly/port/refcount.h"
#include "sfntly/table/core/cmap_table.h"
#include "sfntly/table/truetype/glyph_table.h"
#include "sfntly/table/truetype/loca_table.h"

namespace subtly {
class CharacterPredicate;

typedef int32_t FontId;
typedef std::map<FontId, sfntly::Ptr<sfntly::Font> > FontIdMap;

// Glyph id pair that contains the loca table glyph id as well as the
// font id that has the glyph table this glyph belongs to.
class GlyphId {
 public:
  GlyphId(int32_t glyph_id, FontId font_id);
  ~GlyphId() {}

  bool operator==(const GlyphId& other) const;
  bool operator<(const GlyphId& other) const;

  int32_t glyph_id() const { return glyph_id_; }
  void set_glyph_id(const int32_t glyph_id) { glyph_id_ = glyph_id; }
  FontId font_id() const { return font_id_; }
  void set_font_id(const FontId font_id) { font_id_ = font_id; }

 private:
  int32_t glyph_id_;
  FontId font_id_;
};

typedef std::map<int32_t, GlyphId> CharacterMap;
typedef std::set<GlyphId> GlyphIdSet;

// Font information used for FontAssembler in the construction of a new font.
// Will make copies of character map, glyph id set and font id map.
class FontInfo : public sfntly::RefCounted<FontInfo> {
 public:
  // Empty FontInfo object.
  FontInfo();
  // chars_to_glyph_ids maps characters to GlyphIds for CMap construction
  // resolved_glyph_ids defines GlyphIds which should be in the final font
  // fonts is a map of font ids to fonts to reference any needed table
  FontInfo(CharacterMap* chars_to_glyph_ids,
           GlyphIdSet* resolved_glyph_ids,
           FontIdMap* fonts);
  virtual ~FontInfo();

  // Gets the table with the specified tag from the font corresponding to
  // font_id or NULL if there is no such font/table.
  // font_id is the id of the font that contains the table
  // tag identifies the table to be obtained
  virtual sfntly::FontDataTable* GetTable(FontId font_id, int32_t tag);
  // Gets the table map of the font whose id is font_id
  virtual const sfntly::TableMap* GetTableMap(FontId);

  CharacterMap* chars_to_glyph_ids() const { return chars_to_glyph_ids_; }
  // Takes ownership of the chars_to_glyph_ids CharacterMap.
  void set_chars_to_glyph_ids(CharacterMap* chars_to_glyph_ids);
  GlyphIdSet* resolved_glyph_ids() const { return resolved_glyph_ids_; }
  // Takes ownership of the glyph_ids GlyphIdSet.
  void set_resolved_glyph_ids(GlyphIdSet* glyph_ids);
  FontIdMap* fonts() const { return fonts_; }
  // Takes ownership of the fonts FontIdMap.
  void set_fonts(FontIdMap* fonts);

 private:
  CharacterMap* chars_to_glyph_ids_;
  GlyphIdSet* resolved_glyph_ids_;
  FontIdMap* fonts_;
};

// FontSourcedInfoBuilder is used to create a FontInfo object from a Font
// optionally specifying a CharacterPredicate to filter out some of
// the font's characters.
// It does not take ownership or copy the values its constructor receives.
class FontSourcedInfoBuilder :
      public sfntly::RefCounted<FontSourcedInfoBuilder> {
 public:
  FontSourcedInfoBuilder(sfntly::Font* font, FontId font_id);
  FontSourcedInfoBuilder(sfntly::Font* font,
                         FontId font_id,
                         CharacterPredicate* predicate);
  virtual ~FontSourcedInfoBuilder() { }

  virtual CALLER_ATTACH FontInfo* GetFontInfo();

 protected:
  bool GetCharacterMap(CharacterMap* chars_to_glyph_ids);
  bool ResolveCompositeGlyphs(CharacterMap* chars_to_glyph_ids,
                              GlyphIdSet* resolved_glyph_ids);
  void Initialize();

 private:
  sfntly::Ptr<sfntly::Font> font_;
  FontId font_id_;
  CharacterPredicate* predicate_;

  sfntly::Ptr<sfntly::CMapTable::CMap> cmap_;
  sfntly::Ptr<sfntly::LocaTable> loca_table_;
  sfntly::Ptr<sfntly::GlyphTable> glyph_table_;
};
}
#endif  // TYPOGRAPHY_FONT_SFNTLY_SRC_SAMPLE_SUBTLY_FONT_INFO_H_
