/*
 * Copyright (C) 2009 The Android Open Source Project
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

#include "SkHarfBuzzFont.h"
#include "SkFontHost.h"
#include "SkPaint.h"
#include "SkPath.h"

// HB_Fixed is a 26.6 fixed point format.
static inline HB_Fixed SkScalarToHarfbuzzFixed(SkScalar value) {
#ifdef SK_SCALAR_IS_FLOAT
    return static_cast<HB_Fixed>(value * 64);
#else
    // convert .16 to .6
    return value >> (16 - 6);
#endif
}

static HB_Bool stringToGlyphs(HB_Font hbFont, const HB_UChar16* characters,
                              hb_uint32 length, HB_Glyph* glyphs,
                              hb_uint32* glyphsSize, HB_Bool isRTL) {
    SkHarfBuzzFont* font = reinterpret_cast<SkHarfBuzzFont*>(hbFont->userData);
    SkPaint paint;

    paint.setTypeface(font->getTypeface());
    paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
    int numGlyphs = paint.textToGlyphs(characters, length * sizeof(uint16_t),
                                       reinterpret_cast<uint16_t*>(glyphs));

    // HB_Glyph is 32-bit, but Skia outputs only 16-bit numbers. So our
    // |glyphs| array needs to be converted.
    for (int i = numGlyphs - 1; i >= 0; --i) {
        uint16_t value;
        // We use a memcpy to avoid breaking strict aliasing rules.
        memcpy(&value, reinterpret_cast<char*>(glyphs) + sizeof(uint16_t) * i, sizeof(uint16_t));
        glyphs[i] = value;
    }

    *glyphsSize = numGlyphs;
    return 1;
}

static void glyphsToAdvances(HB_Font hbFont, const HB_Glyph* glyphs,
                         hb_uint32 numGlyphs, HB_Fixed* advances, int flags) {
    SkHarfBuzzFont* font = reinterpret_cast<SkHarfBuzzFont*>(hbFont->userData);
    SkPaint paint;

    font->setupPaint(&paint);
    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

    SkAutoMalloc storage(numGlyphs * (sizeof(SkScalar) + sizeof(uint16_t)));
    SkScalar* scalarWidths = reinterpret_cast<SkScalar*>(storage.get());
    uint16_t* glyphs16 = reinterpret_cast<uint16_t*>(scalarWidths + numGlyphs);

    // convert HB 32bit glyphs to skia's 16bit
    for (hb_uint32 i = 0; i < numGlyphs; ++i) {
        glyphs16[i] = SkToU16(glyphs[i]);
    }
    paint.getTextWidths(glyphs16, numGlyphs * sizeof(uint16_t), scalarWidths);

    for (hb_uint32 i = 0; i < numGlyphs; ++i) {
        advances[i] = SkScalarToHarfbuzzFixed(scalarWidths[i]);
    }
}

static HB_Bool canRender(HB_Font hbFont, const HB_UChar16* characters,
                         hb_uint32 length) {
    SkHarfBuzzFont* font = reinterpret_cast<SkHarfBuzzFont*>(hbFont->userData);
    SkPaint paint;

    paint.setTypeface(font->getTypeface());
    paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
    return paint.containsText(characters, length * sizeof(uint16_t));
}

static HB_Error getOutlinePoint(HB_Font hbFont, HB_Glyph glyph, int flags,
                                hb_uint32 index, HB_Fixed* xPos, HB_Fixed* yPos,
                                hb_uint32* resultingNumPoints) {
    SkHarfBuzzFont* font = reinterpret_cast<SkHarfBuzzFont*>(hbFont->userData);
    SkPaint paint;

    font->setupPaint(&paint);
    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    if (flags & HB_ShaperFlag_UseDesignMetrics) {
        paint.setHinting(SkPaint::kNo_Hinting);
    }

    SkPath path;
    uint16_t glyph16 = SkToU16(glyph);
    paint.getTextPath(&glyph16, sizeof(glyph16), 0, 0, &path);
    int numPoints = path.countPoints();
    if (index >= numPoints) {
        return HB_Err_Invalid_SubTable;
    }

    SkPoint pt = path.getPoint(index);
    *xPos = SkScalarToHarfbuzzFixed(pt.fX);
    *yPos = SkScalarToHarfbuzzFixed(pt.fY);
    *resultingNumPoints = numPoints;

    return HB_Err_Ok;
}

static void getGlyphMetrics(HB_Font hbFont, HB_Glyph glyph,
                            HB_GlyphMetrics* metrics) {
    SkHarfBuzzFont* font = reinterpret_cast<SkHarfBuzzFont*>(hbFont->userData);
    SkPaint paint;

    font->setupPaint(&paint);
    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

    SkScalar width;
    SkRect bounds;
    uint16_t glyph16 = SkToU16(glyph);
    paint.getTextWidths(&glyph16, sizeof(glyph16), &width, &bounds);

    metrics->x = SkScalarToHarfbuzzFixed(bounds.fLeft);
    metrics->y = SkScalarToHarfbuzzFixed(bounds.fTop);
    metrics->width = SkScalarToHarfbuzzFixed(bounds.width());
    metrics->height = SkScalarToHarfbuzzFixed(bounds.height());

    metrics->xOffset = SkScalarToHarfbuzzFixed(width);
    // We can't actually get the |y| correct because Skia doesn't export
    // the vertical advance. However, nor we do ever render vertical text at
    // the moment so it's unimportant.
    metrics->yOffset = 0;
}

static HB_Fixed getFontMetric(HB_Font hbFont, HB_FontMetric metric)
{
    SkHarfBuzzFont* font = reinterpret_cast<SkHarfBuzzFont*>(hbFont->userData);
    SkPaint paint;
    SkPaint::FontMetrics skiaMetrics;

    font->setupPaint(&paint);
    paint.getFontMetrics(&skiaMetrics);

    switch (metric) {
    case HB_FontAscent:
        return SkScalarToHarfbuzzFixed(-skiaMetrics.fAscent);
    default:
        SkDebugf("--- unknown harfbuzz metric enum %d\n", metric);
        return 0;
    }
}

static HB_FontClass gSkHarfBuzzFontClass = {
    stringToGlyphs,
    glyphsToAdvances,
    canRender,
    getOutlinePoint,
    getGlyphMetrics,
    getFontMetric,
};

const HB_FontClass& SkHarfBuzzFont::GetFontClass() {
    return gSkHarfBuzzFontClass;
}

HB_Error SkHarfBuzzFont::GetFontTableFunc(void* voidface, const HB_Tag tag,
                                          HB_Byte* buffer, HB_UInt* len) {
    SkHarfBuzzFont* font = reinterpret_cast<SkHarfBuzzFont*>(voidface);
    uint32_t uniqueID = SkTypeface::UniqueID(font->getTypeface());

    const size_t tableSize = SkFontHost::GetTableSize(uniqueID, tag);
    if (!tableSize) {
        return HB_Err_Invalid_Argument;
    }
    // If Harfbuzz specified a NULL buffer then it's asking for the size.
    if (!buffer) {
        *len = tableSize;
        return HB_Err_Ok;
    }

    if (*len < tableSize) {
        // is this right, or should we just copy less than the full table?
        return HB_Err_Invalid_Argument;
    }
    SkFontHost::GetTableData(uniqueID, tag, 0, tableSize, buffer);
    return HB_Err_Ok;
}

