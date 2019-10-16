/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkTypeface.h"

#include <memory>

#include "include/c/sk_typeface.h"

#include "src/c/sk_types_priv.h"

// typeface

sk_typeface_t* sk_typeface_create_default(void) {
    return ToTypeface(SkTypeface::MakeDefault().release());
}

sk_typeface_t* sk_typeface_ref_default(void) {
    return ToTypeface(SkTypeface::RefDefault().release());
}

void sk_typeface_unref(sk_typeface_t* tf) {
    SkSafeUnref(AsTypeface(tf));
}

sk_typeface_t* sk_typeface_create_from_name_with_font_style(const char *familyName, sk_fontstyle_t* style) {
    return ToTypeface(SkTypeface::MakeFromName (familyName, *AsFontStyle(style)).release());
}

sk_typeface_t* sk_typeface_create_from_file(const char* path, int index) {
    return ToTypeface(SkTypeface::MakeFromFile (path, index).release());
}

sk_typeface_t* sk_typeface_create_from_stream(sk_stream_asset_t* stream, int index) {
    std::unique_ptr<SkStreamAsset> skstream(AsStreamAsset(stream));
    return ToTypeface(SkTypeface::MakeFromStream (std::move(skstream), index).release());
}

sk_stream_asset_t* sk_typeface_open_stream(sk_typeface_t* typeface, int* ttcIndex) {
    return ToStreamAsset(AsTypeface(typeface)->openStream(ttcIndex).release());
}

int sk_typeface_get_units_per_em(sk_typeface_t* typeface) {
    return AsTypeface(typeface)->getUnitsPerEm();
}

int sk_typeface_glyph_count (sk_typeface_t* typeface) {
    return AsTypeface(typeface)->countGlyphs();
}

sk_string_t* sk_typeface_get_family_name(sk_typeface_t* typeface) {
    SkString* family_name = new SkString();
    AsTypeface(typeface)->getFamilyName(family_name);
    return ToString(family_name);
}

sk_fontstyle_t* sk_typeface_get_fontstyle(sk_typeface_t* typeface) {
    SkFontStyle fs = AsTypeface(typeface)->fontStyle();
    return ToFontStyle(new SkFontStyle(fs.weight(), fs.width(), fs.slant()));
}

int sk_typeface_get_font_weight(sk_typeface_t* typeface) {
    return AsTypeface(typeface)->fontStyle().weight();
}

int sk_typeface_get_font_width(sk_typeface_t* typeface) {
    return AsTypeface(typeface)->fontStyle().width();
}

sk_font_style_slant_t sk_typeface_get_font_slant(sk_typeface_t* typeface) {
    return (sk_font_style_slant_t)AsTypeface(typeface)->fontStyle().slant();
}

int sk_typeface_count_tables(sk_typeface_t* typeface) {
    return AsTypeface(typeface)->countTables();
}

int sk_typeface_get_table_tags(sk_typeface_t* typeface, sk_font_table_tag_t tags[]) {
    return AsTypeface(typeface)->getTableTags(tags);
}

size_t sk_typeface_get_table_size(sk_typeface_t* typeface, sk_font_table_tag_t tag) {
    return AsTypeface(typeface)->getTableSize(tag);
}

size_t sk_typeface_get_table_data(sk_typeface_t* typeface, sk_font_table_tag_t tag, size_t offset, size_t length, void* data) {
    return AsTypeface(typeface)->getTableData(tag, offset, length, data);
}


// font manager

sk_fontmgr_t* sk_fontmgr_create_default(void) {
    return ToFontMgr(SkFontMgr::MakeDefault().release());
}

sk_fontmgr_t* sk_fontmgr_ref_default(void) {
    return ToFontMgr(SkFontMgr::RefDefault().release());
}

void sk_fontmgr_unref(sk_fontmgr_t* fontmgr) {
    AsFontMgr(fontmgr)->unref();
}

int sk_fontmgr_count_families(sk_fontmgr_t* fontmgr) {
    return AsFontMgr(fontmgr)->countFamilies();
}

void sk_fontmgr_get_family_name(sk_fontmgr_t* fontmgr, int index, sk_string_t* familyName) {
    AsFontMgr(fontmgr)->getFamilyName(index, AsString(familyName));
}

sk_fontstyleset_t* sk_fontmgr_create_styleset(sk_fontmgr_t* fontmgr, int index) {
    return ToFontStyleSet(AsFontMgr(fontmgr)->createStyleSet(index));
}

sk_fontstyleset_t* sk_fontmgr_match_family(sk_fontmgr_t* fontmgr, const char* familyName) {
    return ToFontStyleSet(AsFontMgr(fontmgr)->matchFamily(familyName));
}

sk_typeface_t* sk_fontmgr_match_family_style(sk_fontmgr_t* fontmgr, const char* familyName, sk_fontstyle_t* style) {
    return ToTypeface(AsFontMgr(fontmgr)->matchFamilyStyle(familyName, *AsFontStyle(style)));
}

sk_typeface_t* sk_fontmgr_match_family_style_character(sk_fontmgr_t* fontmgr, const char* familyName, sk_fontstyle_t* style, const char** bcp47, int bcp47Count, int32_t character) {
    return ToTypeface(AsFontMgr(fontmgr)->matchFamilyStyleCharacter(familyName, *AsFontStyle(style), bcp47, bcp47Count, character));
}

sk_typeface_t* sk_fontmgr_match_face_style(sk_fontmgr_t* fontmgr, const sk_typeface_t* face, sk_fontstyle_t* style) {
    return ToTypeface(AsFontMgr(fontmgr)->matchFaceStyle(AsTypeface(face), *AsFontStyle(style)));
}

sk_typeface_t* sk_fontmgr_create_from_data(sk_fontmgr_t* fontmgr, sk_data_t* data, int index) {
    return ToTypeface(AsFontMgr(fontmgr)->makeFromData(sk_ref_sp(AsData(data)), index).release());
}

sk_typeface_t* sk_fontmgr_create_from_stream(sk_fontmgr_t* fontmgr, sk_stream_asset_t* stream, int index) {
    std::unique_ptr<SkStreamAsset> skstream(AsStreamAsset(stream));
    return ToTypeface(AsFontMgr(fontmgr)->makeFromStream(std::move(skstream), index).release());
}

sk_typeface_t* sk_fontmgr_create_from_file(sk_fontmgr_t* fontmgr, const char* path, int index) {
    return ToTypeface(AsFontMgr(fontmgr)->makeFromFile(path, index).release());
}


// font style

sk_fontstyle_t* sk_fontstyle_new(int weight, int width, sk_font_style_slant_t slant) {
    return ToFontStyle(new SkFontStyle(weight, width, (SkFontStyle::Slant)slant));
}

void sk_fontstyle_delete(sk_fontstyle_t* fs) {
    delete AsFontStyle(fs);
}

int sk_fontstyle_get_weight(const sk_fontstyle_t* fs) {
    return AsFontStyle(fs)->weight();
}

int sk_fontstyle_get_width(const sk_fontstyle_t* fs) {
    return AsFontStyle(fs)->width();
}

sk_font_style_slant_t sk_fontstyle_get_slant(const sk_fontstyle_t* fs) {
    return (sk_font_style_slant_t)AsFontStyle(fs)->slant();
}


// font style set

sk_fontstyleset_t* sk_fontstyleset_create_empty() {
    return ToFontStyleSet(SkFontStyleSet::CreateEmpty());
}

void sk_fontstyleset_unref(sk_fontstyleset_t* fss) {
    AsFontStyleSet(fss)->unref();
}

int sk_fontstyleset_get_count(sk_fontstyleset_t* fss) {
    return AsFontStyleSet(fss)->count();
}

void sk_fontstyleset_get_style(sk_fontstyleset_t* fss, int index, sk_fontstyle_t* fs, sk_string_t* style) {
    return AsFontStyleSet(fss)->getStyle(index, AsFontStyle(fs), AsString(style));
}

sk_typeface_t* sk_fontstyleset_create_typeface(sk_fontstyleset_t* fss, int index) {
    return ToTypeface(AsFontStyleSet(fss)->createTypeface(index));
}

sk_typeface_t* sk_fontstyleset_match_style(sk_fontstyleset_t* fss, sk_fontstyle_t* style) {
    return ToTypeface(AsFontStyleSet(fss)->matchStyle(*AsFontStyle(style)));
}
