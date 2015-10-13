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

#include "subtly/font_info.h"

#include <stdio.h>

#include <set>
#include <map>

#include "subtly/character_predicate.h"

#include "sfntly/tag.h"
#include "sfntly/font.h"
#include "sfntly/font_factory.h"
#include "sfntly/table/core/cmap_table.h"
#include "sfntly/table/truetype/loca_table.h"
#include "sfntly/table/truetype/glyph_table.h"
#include "sfntly/table/core/maximum_profile_table.h"
#include "sfntly/port/type.h"
#include "sfntly/port/refcount.h"

namespace subtly {
using namespace sfntly;
/******************************************************************************
 * GlyphId class
 ******************************************************************************/
GlyphId::GlyphId(int32_t glyph_id, FontId font_id)
    : glyph_id_(glyph_id),
      font_id_(font_id) {
}

bool GlyphId::operator==(const GlyphId& other) const {
  return glyph_id_ == other.glyph_id();
}

bool GlyphId::operator<(const GlyphId& other) const {
  return glyph_id_ < other.glyph_id();
}

/******************************************************************************
 * FontInfo class
 ******************************************************************************/
FontInfo::FontInfo()
    : chars_to_glyph_ids_(new CharacterMap),
      resolved_glyph_ids_(new GlyphIdSet),
      fonts_(new FontIdMap) {
}

FontInfo::FontInfo(CharacterMap* chars_to_glyph_ids,
                   GlyphIdSet* resolved_glyph_ids,
                   FontIdMap* fonts) {
  chars_to_glyph_ids_ = new CharacterMap(chars_to_glyph_ids->begin(),
                                         chars_to_glyph_ids->end());
  resolved_glyph_ids_ = new GlyphIdSet(resolved_glyph_ids->begin(),
                                       resolved_glyph_ids->end());
  fonts_ = new FontIdMap(fonts->begin(), fonts->end());
}

FontInfo::~FontInfo() {
  delete chars_to_glyph_ids_;
  delete resolved_glyph_ids_;
  delete fonts_;
}

FontDataTable* FontInfo::GetTable(FontId font_id, int32_t tag) {
  if (!fonts_)
    return NULL;
  FontIdMap::iterator it = fonts_->find(font_id);
  if (it == fonts_->end())
    return NULL;
  return it->second->GetTable(tag);
}

const TableMap* FontInfo::GetTableMap(FontId font_id) {
  if (!fonts_)
    return NULL;
  FontIdMap::iterator it = fonts_->find(font_id);
  if (it == fonts_->end())
    return NULL;
  return it->second->GetTableMap();
}

void FontInfo::set_chars_to_glyph_ids(CharacterMap* chars_to_glyph_ids) {
  *chars_to_glyph_ids_ = *chars_to_glyph_ids;
}

void FontInfo::set_resolved_glyph_ids(GlyphIdSet* resolved_glyph_ids) {
  *resolved_glyph_ids_ = *resolved_glyph_ids;
}

void FontInfo::set_fonts(FontIdMap* fonts) {
  *fonts_ = *fonts;
}

/******************************************************************************
 * FontSourcedInfoBuilder class
 ******************************************************************************/
FontSourcedInfoBuilder::FontSourcedInfoBuilder(Font* font, FontId font_id)
    : font_(font),
      font_id_(font_id),
      predicate_(NULL) {
  Initialize();
}

FontSourcedInfoBuilder::FontSourcedInfoBuilder(Font* font,
                                               FontId font_id,
                                               CharacterPredicate* predicate)
    : font_(font),
      font_id_(font_id),
      predicate_(predicate) {
  Initialize();
}

void FontSourcedInfoBuilder::Initialize() {
  Ptr<CMapTable> cmap_table = down_cast<CMapTable*>(font_->GetTable(Tag::cmap));
  // We prefer Windows BMP format 4 cmaps.
  cmap_.Attach(cmap_table->GetCMap(CMapTable::WINDOWS_BMP));
  // But if none is found,
  if (!cmap_) {
    return;
  }
  loca_table_ = down_cast<LocaTable*>(font_->GetTable(Tag::loca));
  glyph_table_ = down_cast<GlyphTable*>(font_->GetTable(Tag::glyf));
}

CALLER_ATTACH FontInfo* FontSourcedInfoBuilder::GetFontInfo() {
  CharacterMap* chars_to_glyph_ids = new CharacterMap;
  bool success = GetCharacterMap(chars_to_glyph_ids);
  if (!success) {
    delete chars_to_glyph_ids;
#if defined (SUBTLY_DEBUG)
    fprintf(stderr, "Error creating character map.\n");
#endif
    return NULL;
  }
  GlyphIdSet* resolved_glyph_ids = new GlyphIdSet;
  success = ResolveCompositeGlyphs(chars_to_glyph_ids, resolved_glyph_ids);
  if (!success) {
    delete chars_to_glyph_ids;
    delete resolved_glyph_ids;
#if defined (SUBTLY_DEBUG)
    fprintf(stderr, "Error resolving composite glyphs.\n");
#endif
    return NULL;
  }
  Ptr<FontInfo> font_info = new FontInfo;
  font_info->set_chars_to_glyph_ids(chars_to_glyph_ids);
  font_info->set_resolved_glyph_ids(resolved_glyph_ids);
  FontIdMap* font_id_map = new FontIdMap;
  font_id_map->insert(std::make_pair(font_id_, font_));
  font_info->set_fonts(font_id_map);
  delete chars_to_glyph_ids;
  delete resolved_glyph_ids;
  delete font_id_map;
  return font_info.Detach();
}

bool FontSourcedInfoBuilder::GetCharacterMap(CharacterMap* chars_to_glyph_ids) {
  if (!cmap_ || !chars_to_glyph_ids)
    return false;
  chars_to_glyph_ids->clear();
  CMapTable::CMap::CharacterIterator* character_iterator = cmap_->Iterator();
  if (!character_iterator)
    return false;
  while (character_iterator->HasNext()) {
    int32_t character = character_iterator->Next();
    if (!predicate_ || (*predicate_)(character)) {
      chars_to_glyph_ids->insert
          (std::make_pair(character,
                          GlyphId(cmap_->GlyphId(character), font_id_)));
    }
  }
  delete character_iterator;
  return true;
}

bool
FontSourcedInfoBuilder::ResolveCompositeGlyphs(CharacterMap* chars_to_glyph_ids,
                                               GlyphIdSet* resolved_glyph_ids) {
  if (!chars_to_glyph_ids || !resolved_glyph_ids)
    return false;
  resolved_glyph_ids->clear();
  resolved_glyph_ids->insert(GlyphId(0, font_id_));
  IntegerSet* unresolved_glyph_ids = new IntegerSet;
  // Since composite glyph elements might themselves be composite, we would need
  // to recursively resolve the elements too. To avoid the recursion we
  // create two sets, |unresolved_glyph_ids| for the unresolved glyphs,
  // initially containing all the ids and |resolved_glyph_ids|, initially empty.
  // We'll remove glyph ids from |unresolved_glyph_ids| until it is empty and,
  // if the glyph is composite, add its elements to the unresolved set.
  for (CharacterMap::iterator it = chars_to_glyph_ids->begin(),
           e = chars_to_glyph_ids->end(); it != e; ++it) {
    unresolved_glyph_ids->insert(it->second.glyph_id());
  }
  // As long as there are unresolved glyph ids.
  while (!unresolved_glyph_ids->empty()) {
    // Get the corresponding glyph.
    int32_t glyph_id = *(unresolved_glyph_ids->begin());
    unresolved_glyph_ids->erase(unresolved_glyph_ids->begin());
    if (glyph_id < 0 || glyph_id > loca_table_->num_glyphs()) {
#if defined (SUBTLY_DEBUG)
      fprintf(stderr, "%d larger than %d or smaller than 0\n", glyph_id,
              loca_table_->num_glyphs());
#endif
      continue;
    }
    int32_t length = loca_table_->GlyphLength(glyph_id);
    if (length == 0) {
#if defined (SUBTLY_DEBUG)
      fprintf(stderr, "Zero length glyph %d\n", glyph_id);
#endif
      continue;
    }
    int32_t offset = loca_table_->GlyphOffset(glyph_id);
    GlyphPtr glyph;
    glyph.Attach(glyph_table_->GetGlyph(offset, length));
    if (glyph == NULL) {
#if defined (SUBTLY_DEBUG)
      fprintf(stderr, "GetGlyph returned NULL for %d\n", glyph_id);
#endif
      continue;
    }
    // Mark the glyph as resolved.
    resolved_glyph_ids->insert(GlyphId(glyph_id, font_id_));
    // If it is composite, add all its components to the unresolved glyph set.
    if (glyph->GlyphType() == GlyphType::kComposite) {
      Ptr<GlyphTable::CompositeGlyph> composite_glyph =
          down_cast<GlyphTable::CompositeGlyph*>(glyph.p_);
      int32_t num_glyphs = composite_glyph->NumGlyphs();
      for (int32_t i = 0; i < num_glyphs; ++i) {
        int32_t glyph_id = composite_glyph->GlyphIndex(i);
        if (resolved_glyph_ids->find(GlyphId(glyph_id, -1))
            == resolved_glyph_ids->end()) {
          unresolved_glyph_ids->insert(glyph_id);
        }
      }
    }
  }
  delete unresolved_glyph_ids;
  return true;
}
}
