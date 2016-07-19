/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypeface.h"
#include "SkFontStyle.h"

#include "sk_x_types_priv.h"

#if __cplusplus >= 199711L
static_assert ((int)SkTypeface::kNormal == (int)NORMAL_TYPEFACE_STYLE, "ABI changed, you must write a enumeration mapper for SkTypeface::Style to sk_typeface_style_t");
static_assert ((int)SkTypeface::kBold == (int)BOLD_TYPEFACE_STYLE, "ABI changed, you must write a enumeration mapper for SkTypeface::Style to sk_typeface_style_t");
static_assert ((int)SkTypeface::kItalic == (int)ITALIC_TYPEFACE_STYLE, "ABI changed, you must write a enumeration mapper for SkTypeface::Style to sk_typeface_style_t");
static_assert ((int)SkTypeface::kBoldItalic == (int)BOLD_ITALIC_TYPEFACE_STYLE, "ABI changed, you must write a enumeration mapper for SkTypeface::Style to sk_typeface_style_t");
#endif

#if __cplusplus >= 199711L
static_assert ((int)SkTypeface::kUTF8_Encoding == (int)UTF8_ENCODING, "ABI changed, you must write a enumeration mapper for SkTypeface::Encoding to sk_encoding_t");
static_assert ((int)SkTypeface::kUTF16_Encoding == (int)UTF16_ENCODING, "ABI changed, you must write a enumeration mapper for SkTypeface::Encoding to sk_encoding_t");
static_assert ((int)SkTypeface::kUTF32_Encoding == (int)UTF32_ENCODING, "ABI changed, you must write a enumeration mapper for SkTypeface::Encoding to sk_encoding_t");
#endif

#if __cplusplus >= 199711L
static_assert ((int)SkFontStyle::kUpright_Slant == (int)UPRIGHT_SK_FONT_STYLE_SLANT, "ABI changed, you must write a enumeration mapper for SkFontStyle::Slant to sk_font_style_slant_t");
static_assert ((int)SkFontStyle::kItalic_Slant == (int)ITALIC_SK_FONT_STYLE_SLANT, "ABI changed, you must write a enumeration mapper for SkFontStyle::Slant to sk_font_style_slant_t");
static_assert ((int)SkFontStyle::kOblique_Slant == (int)OBLIQUE_SK_FONT_STYLE_SLANT, "ABI changed, you must write a enumeration mapper for SkFontStyle::Slant to sk_font_style_slant_t");
#endif

#if __cplusplus >= 199711L
static_assert ((int)SkPath::kMove_Verb == (int)MOVE_PATH_VERB, "ABI changed, you must write a enumeration mapper for SkPath::Verb to sk_path_verb_t");
static_assert ((int)SkPath::kLine_Verb == (int)LINE_PATH_VERB, "ABI changed, you must write a enumeration mapper for SkPath::Verb to sk_path_verb_t");
static_assert ((int)SkPath::kQuad_Verb == (int)QUAD_PATH_VERB, "ABI changed, you must write a enumeration mapper for SkPath::Verb to sk_path_verb_t");
static_assert ((int)SkPath::kConic_Verb == (int)CONIC_PATH_VERB, "ABI changed, you must write a enumeration mapper for SkPath::Verb to sk_path_verb_t");
static_assert ((int)SkPath::kCubic_Verb == (int)CUBIC_PATH_VERB, "ABI changed, you must write a enumeration mapper for SkPath::Verb to sk_path_verb_t");
static_assert ((int)SkPath::kClose_Verb == (int)CLOSE_PATH_VERB, "ABI changed, you must write a enumeration mapper for SkPath::Verb to sk_path_verb_t");
static_assert ((int)SkPath::kDone_Verb == (int)DONE_PATH_VERB, "ABI changed, you must write a enumeration mapper for SkPath::Verb to sk_path_verb_t");
#endif

#if __cplusplus >= 199711L
static_assert ((int)SkPath::kAppend_AddPathMode == (int)APPEND_ADD_MODE, "ABI changed, you must write a enumeration mapper for SkPath::AddPathMode to sk_path_add_mode_t");
static_assert ((int)SkPath::kExtend_AddPathMode == (int)EXTEND_ADD_MODE, "ABI changed, you must write a enumeration mapper for SkPath::AddPathMode to sk_path_add_mode_t");
#endif
