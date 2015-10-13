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

#include "subsetter_impl.h"

#include <string.h>

#include <algorithm>
#include <iterator>
#include <map>
#include <set>

#include "sfntly/table/bitmap/eblc_table.h"
#include "sfntly/table/bitmap/ebdt_table.h"
#include "sfntly/table/bitmap/index_sub_table.h"
#include "sfntly/table/bitmap/index_sub_table_format1.h"
#include "sfntly/table/bitmap/index_sub_table_format2.h"
#include "sfntly/table/bitmap/index_sub_table_format3.h"
#include "sfntly/table/bitmap/index_sub_table_format4.h"
#include "sfntly/table/bitmap/index_sub_table_format5.h"
#include "sfntly/table/core/name_table.h"
#include "sfntly/tag.h"
#include "sfntly/data/memory_byte_array.h"
#include "sfntly/port/memory_input_stream.h"
#include "sfntly/port/memory_output_stream.h"

#if defined U_USING_ICU_NAMESPACE
  U_NAMESPACE_USE
#endif

namespace {

using namespace sfntly;

// The bitmap tables must be greater than 16KB to trigger bitmap subsetter.
static const int BITMAP_SIZE_THRESHOLD = 16384;

void ConstructName(UChar* name_part, UnicodeString* name, int32_t name_id) {
  switch (name_id) {
    case NameId::kFullFontName:
      *name = name_part;
      break;
    case NameId::kFontFamilyName:
    case NameId::kPreferredFamily:
    case NameId::kWWSFamilyName: {
      UnicodeString original = *name;
      *name = name_part;
      *name += original;
      break;
    }
    case NameId::kFontSubfamilyName:
    case NameId::kPreferredSubfamily:
    case NameId::kWWSSubfamilyName:
      *name += name_part;
      break;
    default:
      // This name part is not used to construct font name (e.g. copyright).
      // Simply ignore it.
      break;
  }
}

int32_t HashCode(int32_t platform_id, int32_t encoding_id, int32_t language_id,
                 int32_t name_id) {
  int32_t result = platform_id << 24 | encoding_id << 16 | language_id << 8;
  if (name_id == NameId::kFullFontName) {
    result |= 0xff;
  } else if (name_id == NameId::kPreferredFamily ||
             name_id == NameId::kPreferredSubfamily) {
    result |= 0xf;
  } else if (name_id == NameId::kWWSFamilyName ||
             name_id == NameId::kWWSSubfamilyName) {
    result |= 1;
  }
  return result;
}

bool HasName(const char* font_name, Font* font) {
  UnicodeString font_string = UnicodeString::fromUTF8(font_name);
  if (font_string.isEmpty())
    return false;
  UnicodeString regular_suffix = UnicodeString::fromUTF8(" Regular");
  UnicodeString alt_font_string = font_string;
  alt_font_string += regular_suffix;

  typedef std::map<int32_t, UnicodeString> NameMap;
  NameMap names;
  NameTablePtr name_table = down_cast<NameTable*>(font->GetTable(Tag::name));
  if (name_table == NULL) {
    return false;
  }

  for (int32_t i = 0; i < name_table->NameCount(); ++i) {
    switch (name_table->NameId(i)) {
      case NameId::kFontFamilyName:
      case NameId::kFontSubfamilyName:
      case NameId::kFullFontName:
      case NameId::kPreferredFamily:
      case NameId::kPreferredSubfamily:
      case NameId::kWWSFamilyName:
      case NameId::kWWSSubfamilyName: {
        UChar* name_part = name_table->Name(i);
        if (name_part == NULL) {
          continue;
        }
        int32_t hash_code = HashCode(name_table->PlatformId(i),
                                     name_table->EncodingId(i),
                                     name_table->LanguageId(i),
                                     name_table->NameId(i));
        ConstructName(name_part, &(names[hash_code]), name_table->NameId(i));
        delete[] name_part;
        break;
      }
      default:
        break;
    }
  }

  if (!names.empty()) {
    for (NameMap::iterator i = names.begin(), e = names.end(); i != e; ++i) {
      if (i->second.caseCompare(font_string, 0) == 0 ||
          i->second.caseCompare(alt_font_string, 0) == 0) {
        return true;
      }
    }
  }
  return false;
}

Font* FindFont(const char* font_name, const FontArray& font_array) {
  if (font_array.empty() || font_array[0] == NULL) {
    return NULL;
  }

  if (font_name && strlen(font_name)) {
    for (FontArray::const_iterator i = font_array.begin(), e = font_array.end();
         i != e; ++i) {
      if (HasName(font_name, i->p_)) {
        return i->p_;
      }
    }
  }

  return font_array[0].p_;
}

bool ResolveCompositeGlyphs(GlyphTable* glyph_table,
                            LocaTable* loca_table,
                            const unsigned int* glyph_ids,
                            size_t glyph_count,
                            IntegerSet* glyph_id_processed) {
  if (glyph_table == NULL || loca_table == NULL ||
      glyph_ids == NULL || glyph_count == 0 || glyph_id_processed == NULL) {
    return false;
  }

  // Sort and uniquify glyph ids.
  IntegerSet glyph_id_remaining;
  glyph_id_remaining.insert(0);  // Always include glyph id 0.
  for (size_t i = 0; i < glyph_count; ++i) {
    glyph_id_remaining.insert(glyph_ids[i]);
  }

  // Identify if any given glyph id maps to a composite glyph.  If so, include
  // the glyphs referenced by that composite glyph.
  while (!glyph_id_remaining.empty()) {
    IntegerSet comp_glyph_id;
    for (IntegerSet::iterator i = glyph_id_remaining.begin(),
                              e = glyph_id_remaining.end(); i != e; ++i) {
      if (*i < 0 || *i >= loca_table->num_glyphs()) {
        // Invalid glyph id, ignore.
        continue;
      }

      int32_t length = loca_table->GlyphLength(*i);
      if (length == 0) {
        // Empty glyph, ignore.
        continue;
      }
      int32_t offset = loca_table->GlyphOffset(*i);

      GlyphPtr glyph;
      glyph.Attach(glyph_table->GetGlyph(offset, length));
      if (glyph == NULL) {
        // Error finding glyph, ignore.
        continue;
      }

      if (glyph->GlyphType() == GlyphType::kComposite) {
        Ptr<GlyphTable::CompositeGlyph> comp_glyph =
            down_cast<GlyphTable::CompositeGlyph*>(glyph.p_);
        for (int32_t j = 0; j < comp_glyph->NumGlyphs(); ++j) {
          int32_t glyph_id = comp_glyph->GlyphIndex(j);
          if (glyph_id_processed->find(glyph_id) == glyph_id_processed->end() &&
              glyph_id_remaining.find(glyph_id) == glyph_id_remaining.end()) {
            comp_glyph_id.insert(comp_glyph->GlyphIndex(j));
          }
        }
      }

      glyph_id_processed->insert(*i);
    }

    glyph_id_remaining.clear();
    glyph_id_remaining = comp_glyph_id;
  }

  return true;
}

bool SetupGlyfBuilders(Font::Builder* font_builder,
                       GlyphTable* glyph_table,
                       LocaTable* loca_table,
                       const IntegerSet& glyph_ids) {
  if (!font_builder || !glyph_table || !loca_table) {
    return false;
  }

  GlyphTableBuilderPtr glyph_table_builder =
      down_cast<GlyphTable::Builder*>(font_builder->NewTableBuilder(Tag::glyf));
  LocaTableBuilderPtr loca_table_builder =
      down_cast<LocaTable::Builder*>(font_builder->NewTableBuilder(Tag::loca));
  if (glyph_table_builder == NULL || loca_table_builder == NULL) {
    // Out of memory.
    return false;
  }

  // Extract glyphs and setup loca list.
  IntegerList loca_list;
  loca_list.resize(loca_table->num_glyphs());
  loca_list.push_back(0);
  int32_t last_glyph_id = 0;
  int32_t last_offset = 0;
  GlyphTable::GlyphBuilderList* glyph_builders =
      glyph_table_builder->GlyphBuilders();
  for (IntegerSet::const_iterator i = glyph_ids.begin(), e = glyph_ids.end();
                                  i != e; ++i) {
    int32_t length = loca_table->GlyphLength(*i);
    int32_t offset = loca_table->GlyphOffset(*i);

    GlyphPtr glyph;
    glyph.Attach(glyph_table->GetGlyph(offset, length));

    // Add glyph to new glyf table.
    ReadableFontDataPtr data = glyph->ReadFontData();
    WritableFontDataPtr copy_data;
    copy_data.Attach(WritableFontData::CreateWritableFontData(data->Length()));
    data->CopyTo(copy_data);
    GlyphBuilderPtr glyph_builder;
    glyph_builder.Attach(glyph_table_builder->GlyphBuilder(copy_data));
    glyph_builders->push_back(glyph_builder);

    // Configure loca list.
    for (int32_t j = last_glyph_id + 1; j <= *i; ++j) {
      loca_list[j] = last_offset;
    }
    last_offset += length;
    loca_list[*i + 1] = last_offset;
    last_glyph_id = *i;
  }
  for (int32_t j = last_glyph_id + 1; j <= loca_table->num_glyphs(); ++j) {
    loca_list[j] = last_offset;
  }
  loca_table_builder->SetLocaList(&loca_list);

  return true;
}

bool HasOverlap(int32_t range_begin, int32_t range_end,
                const IntegerSet& glyph_ids) {
  if (range_begin == range_end) {
    return glyph_ids.find(range_begin) != glyph_ids.end();
  } else if (range_end > range_begin) {
    IntegerSet::const_iterator left = glyph_ids.lower_bound(range_begin);
    IntegerSet::const_iterator right = glyph_ids.lower_bound(range_end);
    return right != left;
  }
  return false;
}

// Initialize builder, returns false if glyph_id subset is not covered.
// Not thread-safe, caller to ensure object life-time.
bool InitializeBitmapBuilder(EbdtTable::Builder* ebdt, EblcTable::Builder* eblc,
                             const IntegerSet& glyph_ids) {
  BitmapLocaList loca_list;
  BitmapSizeTableBuilderList* strikes = eblc->BitmapSizeBuilders();

  // Note: Do not call eblc_builder->GenerateLocaList(&loca_list) and then
  //       ebdt_builder->SetLoca(loca_list).  For fonts like SimSun, there are
  //       >28K glyphs inside, where a typical usage will be <1K glyphs.  Doing
  //       the calls improperly will result in creation of >100K objects that
  //       will be destroyed immediately, inducing significant slowness.
  IntegerList removed_strikes;
  for (size_t i = 0; i < strikes->size(); i++) {
    if (!HasOverlap((*strikes)[i]->StartGlyphIndex(),
                    (*strikes)[i]->EndGlyphIndex(), glyph_ids)) {
      removed_strikes.push_back(i);
      continue;
    }

    IndexSubTableBuilderList* index_builders =
        (*strikes)[i]->IndexSubTableBuilders();
    IntegerList removed_indexes;
    BitmapGlyphInfoMap info_map;
    for (size_t j = 0; j < index_builders->size(); ++j) {
      if ((*index_builders)[j] == NULL) {
        // Subtable is malformed, let's just skip it.
        removed_indexes.push_back(j);
        continue;
      }
      int32_t first_glyph_id = (*index_builders)[j]->first_glyph_index();
      int32_t last_glyph_id = (*index_builders)[j]->last_glyph_index();
      if (!HasOverlap(first_glyph_id, last_glyph_id, glyph_ids)) {
        removed_indexes.push_back(j);
        continue;
      }
      for (IntegerSet::const_iterator gid = glyph_ids.begin(),
                                      gid_end = glyph_ids.end();
                                      gid != gid_end; gid++) {
        if (*gid < first_glyph_id) {
          continue;
        }
        if (*gid > last_glyph_id) {
          break;
        }
        BitmapGlyphInfoPtr info;
        info.Attach((*index_builders)[j]->GlyphInfo(*gid));
        if (info && info->length()) {  // Do not include gid without bitmap
          info_map[*gid] = info;
        }
      }
    }
    if (!info_map.empty()) {
      loca_list.push_back(info_map);
    } else {
      removed_strikes.push_back(i);  // Detected null entries.
    }

    // Remove unused index sub tables
    for (IntegerList::reverse_iterator j = removed_indexes.rbegin(),
                                       e = removed_indexes.rend();
                                       j != e; j++) {
      index_builders->erase(index_builders->begin() + *j);
    }
  }
  if (removed_strikes.size() == strikes->size() || loca_list.empty()) {
    return false;
  }

  for (IntegerList::reverse_iterator i = removed_strikes.rbegin(),
                                     e = removed_strikes.rend(); i != e; i++) {
    strikes->erase(strikes->begin() + *i);
  }

  if (strikes->empty()) {  // no glyph covered, can safely drop the builders.
    return false;
  }

  ebdt->SetLoca(&loca_list);
  ebdt->GlyphBuilders();  // Initialize the builder.
  return true;
}

void CopyBigGlyphMetrics(BigGlyphMetrics::Builder* source,
                         BigGlyphMetrics::Builder* target) {
  target->SetHeight(static_cast<byte_t>(source->Height()));
  target->SetWidth(static_cast<byte_t>(source->Width()));
  target->SetHoriBearingX(static_cast<byte_t>(source->HoriBearingX()));
  target->SetHoriBearingY(static_cast<byte_t>(source->HoriBearingY()));
  target->SetHoriAdvance(static_cast<byte_t>(source->HoriAdvance()));
  target->SetVertBearingX(static_cast<byte_t>(source->VertBearingX()));
  target->SetVertBearingY(static_cast<byte_t>(source->VertBearingY()));
  target->SetVertAdvance(static_cast<byte_t>(source->VertAdvance()));
}

CALLER_ATTACH IndexSubTable::Builder*
ConstructIndexFormat4(IndexSubTable::Builder* b, const BitmapGlyphInfoMap& loca,
                      int32_t* image_data_offset) {
  IndexSubTableFormat4BuilderPtr builder4;
  builder4.Attach(IndexSubTableFormat4::Builder::CreateBuilder());
  CodeOffsetPairBuilderList offset_pairs;

  size_t offset = 0;
  int32_t lower_bound = b->first_glyph_index();
  int32_t upper_bound = b->last_glyph_index();
  int32_t last_gid = -1;
  BitmapGlyphInfoMap::const_iterator i = loca.lower_bound(lower_bound);
  BitmapGlyphInfoMap::const_iterator end = loca.end();
  if (i != end) {
    last_gid = i->first;
    builder4->set_first_glyph_index(last_gid);
    builder4->set_image_format(b->image_format());
    builder4->set_image_data_offset(*image_data_offset);
  }
  for (; i != end; i++) {
    int32_t gid = i->first;
    if (gid > upper_bound) {
      break;
    }
    offset_pairs.push_back(
        IndexSubTableFormat4::CodeOffsetPairBuilder(gid, offset));
    offset += i->second->length();
    last_gid = gid;
  }
  offset_pairs.push_back(
      IndexSubTableFormat4::CodeOffsetPairBuilder(-1, offset));
  builder4->set_last_glyph_index(last_gid);
  *image_data_offset += offset;
  builder4->SetOffsetArray(offset_pairs);

  return builder4.Detach();
}

CALLER_ATTACH IndexSubTable::Builder*
ConstructIndexFormat5(IndexSubTable::Builder* b, const BitmapGlyphInfoMap& loca,
                      int32_t* image_data_offset) {
  IndexSubTableFormat5BuilderPtr new_builder;
  new_builder.Attach(IndexSubTableFormat5::Builder::CreateBuilder());

  // Copy BigMetrics
  int32_t image_size = 0;
  if (b->index_format() == IndexSubTable::Format::FORMAT_2) {
    IndexSubTableFormat2BuilderPtr builder2 =
      down_cast<IndexSubTableFormat2::Builder*>(b);
    CopyBigGlyphMetrics(builder2->BigMetrics(), new_builder->BigMetrics());
    image_size = builder2->ImageSize();
  } else {
    IndexSubTableFormat5BuilderPtr builder5 =
      down_cast<IndexSubTableFormat5::Builder*>(b);
    BigGlyphMetricsBuilderPtr metrics_builder;
    CopyBigGlyphMetrics(builder5->BigMetrics(), new_builder->BigMetrics());
    image_size = builder5->ImageSize();
  }

  IntegerList* glyph_array = new_builder->GlyphArray();
  size_t offset = 0;
  int32_t lower_bound = b->first_glyph_index();
  int32_t upper_bound = b->last_glyph_index();
  int32_t last_gid = -1;
  BitmapGlyphInfoMap::const_iterator i = loca.lower_bound(lower_bound);
  BitmapGlyphInfoMap::const_iterator end = loca.end();
  if (i != end) {
    last_gid = i->first;
    new_builder->set_first_glyph_index(last_gid);
    new_builder->set_image_format(b->image_format());
    new_builder->set_image_data_offset(*image_data_offset);
    new_builder->SetImageSize(image_size);
  }
  for (; i != end; i++) {
    int32_t gid = i->first;
    if (gid > upper_bound) {
      break;
    }
    glyph_array->push_back(gid);
    offset += i->second->length();
    last_gid = gid;
  }
  new_builder->set_last_glyph_index(last_gid);
  *image_data_offset += offset;
  return new_builder.Detach();
}

CALLER_ATTACH IndexSubTable::Builder*
SubsetIndexSubTable(IndexSubTable::Builder* builder,
                    const BitmapGlyphInfoMap& loca,
                    int32_t* image_data_offset) {
  switch (builder->index_format()) {
    case IndexSubTable::Format::FORMAT_1:
    case IndexSubTable::Format::FORMAT_3:
    case IndexSubTable::Format::FORMAT_4:
      return ConstructIndexFormat4(builder, loca, image_data_offset);
    case IndexSubTable::Format::FORMAT_2:
    case IndexSubTable::Format::FORMAT_5:
      return ConstructIndexFormat5(builder, loca, image_data_offset);
    default:
      assert(false);
      break;
  }
  return NULL;
}

}

namespace sfntly {

// Not thread-safe, caller to ensure object life-time.
void SubsetEBLC(EblcTable::Builder* eblc, const BitmapLocaList& new_loca) {
  BitmapSizeTableBuilderList* size_builders = eblc->BitmapSizeBuilders();
  if (size_builders == NULL) {
    return;
  }

  int32_t image_data_offset = EbdtTable::Offset::kHeaderLength;
  for (size_t strike = 0; strike < size_builders->size(); ++strike) {
    IndexSubTableBuilderList* index_builders =
        (*size_builders)[strike]->IndexSubTableBuilders();
    for (size_t index = 0; index < index_builders->size(); ++index) {
      IndexSubTable::Builder* new_builder_raw =
          SubsetIndexSubTable((*index_builders)[index], new_loca[strike],
                              &image_data_offset);
      if (NULL != new_builder_raw) {
        (*index_builders)[index].Attach(new_builder_raw);
      }
    }
  }
}

// EBLC structure (from stuartg)
//  header
//  bitmapSizeTable[]
//    one per strike
//    holds strike metrics - sbitLineMetrics
//    holds info about indexSubTableArray
//  indexSubTableArray[][]
//    one per strike and then one per indexSubTable for that strike
//    holds info about the indexSubTable
//    the indexSubTable entries pointed to can be of different formats
//  indexSubTable
//    one per indexSubTableArray entry
//    tells how to get the glyphs
//    may hold the glyph metrics if they are uniform for all the glyphs in range
// Please note that the structure can also be
//  {indexSubTableArray[], indexSubTables[]}[]
//  This way is also legal and in fact how Microsoft fonts are laid out.
//
// There is nothing that says that the indexSubTableArray entries and/or the
// indexSubTable items need to be unique. They may be shared between strikes.
//
// EBDT structure:
//  header
//  glyphs
//    amorphous blob of data
//    different glyphs that are only able to be figured out from the EBLC table
//    may hold metrics - depends on the EBLC entry that pointed to them

// Subsetting EBLC table (from arthurhsu)
//  Most pages use only a fraction (hundreds or less) glyphs out of a given font
//  (which can have >20K glyphs for CJK).  It's safe to assume that the subset
//  font will have sparse bitmap glyphs.  So we reconstruct the EBLC table as
//  format 4 or 5 here.

enum BuildersToRemove {
  kRemoveNone,
  kRemoveBDAT,
  kRemoveBDATAndEBDT,
  kRemoveEBDT
};

int SetupBitmapBuilders(Font* font, Font::Builder* font_builder,
                        const IntegerSet& glyph_ids) {
  if (!font || !font_builder) {
    return false;
  }

  // Check if bitmap table exists.
  EbdtTablePtr ebdt_table = down_cast<EbdtTable*>(font->GetTable(Tag::EBDT));
  EblcTablePtr eblc_table = down_cast<EblcTable*>(font->GetTable(Tag::EBLC));
  bool use_ebdt = (ebdt_table != NULL && eblc_table != NULL);
  if (!use_ebdt) {
    ebdt_table = down_cast<EbdtTable*>(font->GetTable(Tag::bdat));
    eblc_table = down_cast<EblcTable*>(font->GetTable(Tag::bloc));
    if (ebdt_table == NULL || eblc_table == NULL) {
      return kRemoveNone;
    }
  }

  // If the bitmap table's size is too small, skip subsetting.
  if (ebdt_table->DataLength() + eblc_table->DataLength() <
      BITMAP_SIZE_THRESHOLD) {
    return use_ebdt ? kRemoveBDAT : kRemoveNone;
  }

  // Get the builders.
  EbdtTableBuilderPtr ebdt_table_builder = down_cast<EbdtTable::Builder*>(
      font_builder->NewTableBuilder(use_ebdt ? Tag::EBDT : Tag::bdat,
                                    ebdt_table->ReadFontData()));
  EblcTableBuilderPtr eblc_table_builder = down_cast<EblcTable::Builder*>(
      font_builder->NewTableBuilder(use_ebdt ? Tag::EBLC : Tag::bloc,
                                    eblc_table->ReadFontData()));
  if (ebdt_table_builder == NULL || eblc_table_builder == NULL) {
    // Out of memory.
    return use_ebdt ? kRemoveBDAT : kRemoveNone;
  }

  if (!InitializeBitmapBuilder(ebdt_table_builder, eblc_table_builder,
                               glyph_ids)) {
    // Bitmap tables do not cover the glyphs in our subset.
    font_builder->RemoveTableBuilder(use_ebdt ? Tag::EBLC : Tag::bloc);
    font_builder->RemoveTableBuilder(use_ebdt ? Tag::EBDT : Tag::bdat);
    return use_ebdt ? kRemoveBDATAndEBDT : kRemoveEBDT;
  }

  BitmapLocaList new_loca;
  ebdt_table_builder->GenerateLocaList(&new_loca);
  SubsetEBLC(eblc_table_builder, new_loca);

  return use_ebdt ? kRemoveBDAT : kRemoveNone;
}

SubsetterImpl::SubsetterImpl() {
}

SubsetterImpl::~SubsetterImpl() {
}

bool SubsetterImpl::LoadFont(const char* font_name,
                             const unsigned char* original_font,
                             size_t font_size) {
  MemoryInputStream mis;
  mis.Attach(original_font, font_size);
  if (factory_ == NULL) {
    factory_.Attach(FontFactory::GetInstance());
  }

  FontArray font_array;
  factory_->LoadFonts(&mis, &font_array);
  font_ = FindFont(font_name, font_array);
  if (font_ == NULL) {
    return false;
  }

  return true;
}

int SubsetterImpl::SubsetFont(const unsigned int* glyph_ids,
                              size_t glyph_count,
                              unsigned char** output_buffer) {
  if (factory_ == NULL || font_ == NULL) {
    return -1;
  }

  // Find glyf and loca table.
  GlyphTablePtr glyph_table =
      down_cast<GlyphTable*>(font_->GetTable(Tag::glyf));
  LocaTablePtr loca_table = down_cast<LocaTable*>(font_->GetTable(Tag::loca));
  if (glyph_table == NULL || loca_table == NULL) {
    // We are not able to subset the font.
    return 0;
  }

  IntegerSet glyph_id_processed;
  if (!ResolveCompositeGlyphs(glyph_table, loca_table,
                              glyph_ids, glyph_count, &glyph_id_processed) ||
      glyph_id_processed.empty()) {
    return 0;
  }

  FontPtr new_font;
  new_font.Attach(Subset(glyph_id_processed, glyph_table, loca_table));
  if (new_font == NULL) {
    return 0;
  }

  MemoryOutputStream output_stream;
  factory_->SerializeFont(new_font, &output_stream);
  int length = static_cast<int>(output_stream.Size());
  if (length > 0) {
    *output_buffer = new unsigned char[length];
    memcpy(*output_buffer, output_stream.Get(), length);
  }

  return length;
}

// Long comments regarding TTF tables and PDF (from stuartg)
//
// According to PDF spec 1.4 (section 5.8), the following tables must be
// present:
//  head, hhea, loca, maxp, cvt, prep, glyf, hmtx, fpgm
//  cmap if font is used with a simple font dict and not a CIDFont dict
//
// Other tables we need to keep for PDF rendering to support zoom in/out:
//  bdat, bloc, ebdt, eblc, ebsc, gasp
//
// Special table:
//  CFF - if you have this table then you shouldn't have a glyf table and this
//        is the table with all the glyphs.  Shall skip subsetting completely
//        since sfntly is not capable of subsetting it for now.
//  post - extra info here for printing on PostScript printers but maybe not
//         enough to outweigh the space taken by the names
//
// Tables to break apart:
//  name - could throw away all but one language and one platform strings/ might
//         throw away some of the name entries
//  cmap - could strip out non-needed cmap subtables
//       - format 4 subtable can be subsetted as well using sfntly
//
// Graphite tables:
//  silf, glat, gloc, feat - should be okay to strip out
//
// Tables that can be discarded:
//  OS/2 - everything here is for layout and description of the font that is
//         elsewhere (some in the PDF objects)
//  BASE, GDEF, GSUB, GPOS, JSTF - all used for layout
//  kern - old style layout
//  DSIG - this will be invalid after subsetting
//  hdmx - layout
//  PCLT - metadata that's not needed
//  vmtx - layout
//  vhea - layout
//  VDMX
//  VORG - not used by TT/OT - used by CFF
//  hsty - would be surprised to see one of these - used on the Newton
//  AAT tables - mort, morx, feat, acnt, bsin, just, lcar, fdsc, fmtx, prop,
//               Zapf, opbd, trak, fvar, gvar, avar, cvar
//             - these are all layout tables and once layout happens are not
//               needed anymore
//  LTSH - layout

CALLER_ATTACH
Font* SubsetterImpl::Subset(const IntegerSet& glyph_ids, GlyphTable* glyf,
                            LocaTable* loca) {
  // The const is initialized here to workaround VC bug of rendering all Tag::*
  // as 0.  These tags represents the TTF tables that we will embed in subset
  // font.
  const int32_t TABLES_IN_SUBSET[] = {
    Tag::head, Tag::hhea, Tag::loca, Tag::maxp, Tag::cvt,
    Tag::prep, Tag::glyf, Tag::hmtx, Tag::fpgm, Tag::EBDT,
    Tag::EBLC, Tag::EBSC, Tag::bdat, Tag::bloc, Tag::bhed,
    Tag::cmap,  // Keep here for future tagged PDF development.
    Tag::name,  // Keep here due to legal concerns: copyright info inside.
  };

  // Setup font builders we need.
  FontBuilderPtr font_builder;
  font_builder.Attach(factory_->NewFontBuilder());
  IntegerSet remove_tags;

  if (SetupGlyfBuilders(font_builder, glyf, loca, glyph_ids)) {
    remove_tags.insert(Tag::glyf);
    remove_tags.insert(Tag::loca);
  }

  // For old Apple bitmap fonts, they have only bdats and bhed is identical
  // to head.  As a result, we can't remove bdat tables for those fonts.
  int setup_result = SetupBitmapBuilders(font_, font_builder, glyph_ids);
  if (setup_result == kRemoveBDATAndEBDT || setup_result == kRemoveEBDT) {
    remove_tags.insert(Tag::EBDT);
    remove_tags.insert(Tag::EBLC);
    remove_tags.insert(Tag::EBSC);
  }

  if (setup_result == kRemoveBDAT || setup_result == kRemoveBDATAndEBDT) {
    remove_tags.insert(Tag::bdat);
    remove_tags.insert(Tag::bloc);
    remove_tags.insert(Tag::bhed);
  }

  IntegerSet allowed_tags;
  for (size_t i = 0; i < sizeof(TABLES_IN_SUBSET) / sizeof(int32_t); ++i) {
    allowed_tags.insert(TABLES_IN_SUBSET[i]);
  }

  IntegerSet result;
  std::set_difference(allowed_tags.begin(), allowed_tags.end(),
                      remove_tags.begin(), remove_tags.end(),
                      std::inserter(result, result.end()));
  allowed_tags = result;

  // Setup remaining builders.
  for (IntegerSet::iterator i = allowed_tags.begin(), e = allowed_tags.end();
                            i != e; ++i) {
    Table* table = font_->GetTable(*i);
    if (table) {
      font_builder->NewTableBuilder(*i, table->ReadFontData());
    }
  }

  return font_builder->Build();
}

}  // namespace sfntly
