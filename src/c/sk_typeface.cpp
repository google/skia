/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypeface.h"
#include "SkFontMgr.h"
#include "SkFontStyle.h"

#include "sk_typeface.h"

#include "sk_types_priv.h"

void sk_typeface_unref(sk_typeface_t* tf)
{
    SkSafeUnref(AsTypeface(tf));
}

sk_typeface_t* sk_typeface_create_from_name(const char *familyName, sk_typeface_style_t sstyle)
{
    return ToTypeface(SkTypeface::MakeFromName (familyName, SkFontStyle::FromOldStyle((SkTypeface::Style)sstyle)).release());
}

sk_typeface_t* sk_typeface_create_from_name_with_font_style(const char *familyName, int weight, int width, sk_font_style_slant_t slant)
{
    return ToTypeface(SkTypeface::MakeFromName (familyName, SkFontStyle(weight, width, (SkFontStyle::Slant)slant)).release());
}

sk_typeface_t* sk_typeface_create_from_typeface(sk_typeface_t* typeface, sk_typeface_style_t sstyle)
{
    return ToTypeface(SkTypeface::MakeFromTypeface (AsTypeface(typeface), (SkTypeface::Style)sstyle).release());
}

sk_typeface_t* sk_typeface_create_from_file(const char* path, int index)
{
    return ToTypeface(SkTypeface::MakeFromFile (path, index).release());
}

sk_typeface_t* sk_typeface_create_from_stream(sk_stream_asset_t* stream, int index)
{
    return ToTypeface(SkTypeface::MakeFromStream (AsStreamAsset(stream), index).release());
}

int sk_typeface_chars_to_glyphs (sk_typeface_t* typeface, const char *chars, sk_encoding_t encoding, uint16_t glyphs [], int glyphCount)
{
    return (AsTypeface(typeface))->charsToGlyphs(chars, (SkTypeface::Encoding)encoding, glyphs, glyphCount);
}

sk_stream_asset_t* sk_typeface_open_stream(sk_typeface_t* typeface, int* ttcIndex)
{
    return ToStreamAsset(AsTypeface(typeface)->openStream(ttcIndex));
}

int sk_typeface_get_units_per_em(sk_typeface_t* typeface)
{
    return AsTypeface(typeface)->getUnitsPerEm();
}

int sk_typeface_glyph_count (sk_typeface_t* typeface)
{
    return AsTypeface(typeface)->countGlyphs();
}

sk_string_t* sk_typeface_get_family_name(sk_typeface_t* typeface)
{
    SkString* family_name = new SkString();
    AsTypeface(typeface)->getFamilyName(family_name);
    return ToString(family_name);
}

int sk_typeface_get_font_weight(sk_typeface_t* typeface)
{
    return AsTypeface(typeface)->fontStyle().weight();
}

int sk_typeface_get_font_width(sk_typeface_t* typeface)
{
    return AsTypeface(typeface)->fontStyle().width();
}

sk_font_style_slant_t sk_typeface_get_font_slant(sk_typeface_t* typeface)
{
    return (sk_font_style_slant_t)AsTypeface(typeface)->fontStyle().slant();
}

sk_typeface_style_t sk_typeface_get_style(sk_typeface_t* typeface)
{
    return (sk_typeface_style_t)AsTypeface(typeface)->style();
}

int sk_typeface_count_tables(sk_typeface_t* typeface)
{
    return AsTypeface(typeface)->countTables();
}

int sk_typeface_get_table_tags(sk_typeface_t* typeface, sk_font_table_tag_t tags[])
{
    return AsTypeface(typeface)->getTableTags(tags);
}

size_t sk_typeface_get_table_size(sk_typeface_t* typeface, sk_font_table_tag_t tag)
{
    return AsTypeface(typeface)->getTableSize(tag);
}

size_t sk_typeface_get_table_data(sk_typeface_t* typeface, sk_font_table_tag_t tag, size_t offset, size_t length, void* data)
{
    return AsTypeface(typeface)->getTableData(tag, offset, length, data);
}

sk_fontmgr_t* sk_fontmgr_ref_default()
{
    return ToFontMgr(SkFontMgr::RefDefault().release());
}

void sk_fontmgr_unref(sk_fontmgr_t* fontmgr)
{
    AsFontMgr(fontmgr)->unref();
}

int sk_fontmgr_count_families(sk_fontmgr_t* fontmgr)
{
    return AsFontMgr(fontmgr)->countFamilies();
}

void sk_fontmgr_get_family_name(sk_fontmgr_t* fontmgr, int index, sk_string_t* familyName)
{
    AsFontMgr(fontmgr)->getFamilyName(index, AsString(familyName));
}

sk_typeface_t* sk_fontmgr_match_family_style_character(sk_fontmgr_t* fontmgr, const char* familyName, int weight, int width, sk_font_style_slant_t slant, const char** bcp47, int bcp47Count, int32_t character)
{
    SkFontStyle style = SkFontStyle(weight, width, (SkFontStyle::Slant)slant);
    SkTypeface* typeface = AsFontMgr(fontmgr)->matchFamilyStyleCharacter(familyName, style, bcp47, bcp47Count, character);
    return ToTypeface(typeface);
}

