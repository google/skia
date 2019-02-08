/*
 * Copyright 2017 Google, Inc.
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

#include "SkTextStyle.h"
#include "SkFontStyle.h"
#include "SkColor.h"

// TODO: Make it external so the other platforms (Android) could use it
#define DEFAULT_FONT_FAMILY "Arial"

SkTextStyle::SkTextStyle()
  : _fontStyle()
  , _fontFamily(DEFAULT_FONT_FAMILY) {

  _color = SK_ColorWHITE;
  _decoration = SkTextDecoration::kNone;
  // Does not make sense to draw a transparent object, so we use it as a default
  // value to indicate no decoration color was set.
  _decorationColor = SK_ColorTRANSPARENT;
  _decorationStyle = SkTextDecorationStyle::kSolid;
  // Thickness is applied as a multiplier to the default thickness of the font.
  _decorationThicknessMultiplier = 1.0;
  _fontSize = 14.0;
  _letterSpacing = 0.0;
  _wordSpacing = 0.0;
  _height = 1.0;
  _hasBackground = false;
  _hasForeground = false;
}

// TODO: use font provider to resolve the font
sk_sp<SkTypeface> SkTextStyle::getTypeface() const {
  /*
  if (_typeface == nullptr) {
    SkDebugf("MakeDefault!!!!!\n");
    _typeface = SkTypeface::MakeDefault();
  }
   */
  return _typeface; // SkTypeface::MakeFromName(_fontFamily.data(), SkFontStyle());
}

bool SkTextStyle::equals(const SkTextStyle& other) const {
  if (_color != other._color)
    return false;
  if (_decoration != other._decoration)
    return false;
  if (_decorationColor != other._decorationColor)
    return false;
  if (_decorationStyle != other._decorationStyle)
    return false;
  if (_decorationThicknessMultiplier != other._decorationThicknessMultiplier)
    return false;
  if (!(_fontStyle == other._fontStyle))
    return false;
  if (_fontFamily != other._fontFamily)
    return false;
  if (_letterSpacing != other._letterSpacing)
    return false;
  if (_wordSpacing != other._wordSpacing)
    return false;
  if (_height != other._height)
    return false;
  if (_locale != other._locale)
    return false;
  if (_foreground != other._foreground)
    return false;
  if (_textShadows.size() != other._textShadows.size())
    return false;

  for (size_t shadow_index = 0; shadow_index < _textShadows.size();
       ++shadow_index) {
    if (_textShadows[shadow_index] != other._textShadows[shadow_index])
      return false;
  }

  return true;
}
