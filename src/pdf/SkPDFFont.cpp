/*
 * Copyright (C) 2010 The Android Open Source Project
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

#include "SkPaint.h"
#include "SkPDFFont.h"
#include "SkUtils.h"

namespace {

uint16_t unicharToWinAnsiGlyphID(SkUnichar uniChar) {
    // TODO(vandebo) Quick hack to get text working.  Either finish the
    // implementation of this, or remove it and use the encoding built into
    // the real font.
    if (uniChar < ' ')
        return 0;
    if (uniChar < '~')
        return uniChar;
    return 0;
}

}

SkPDFFont::~SkPDFFont() {
    SkAutoMutexAcquire lock(canonicalFontsMutex());
    int index = find(fFontID);
    SkASSERT(index >= 0);
    canonicalFonts().removeShuffle(index);
}

void SkPDFFont::getResources(SkTDArray<SkPDFObject*>* resourceList) {
    resourceList->setReserve(resourceList->count() + fResources.count());
    for (int i = 0; i < fResources.count(); i++) {
        resourceList->push(fResources[i]);
        fResources[i]->ref();
        fResources[i]->getResources(resourceList);
    }
}

uint32_t SkPDFFont::fontID() {
    return fFontID;
}

bool SkPDFFont::multiByteGlyphs() {
    return fMultiByteGlyphs;
}

// TODO(vandebo) This function goes or stays with unicharToWinAnsiGlyphID.
int SkPDFFont::textToPDFGlyphs(const void* text, size_t byteLength,
                               const SkPaint& paint, uint16_t glyphs[],
                               size_t glyphsLength) const {
    // For a regular, non built-in font, just ask the paint.
    if (fResources.count() != 0) {
        SkASSERT(glyphsLength >= byteLength / 2);
        if (paint.getTextEncoding() == SkPaint::kGlyphID_TextEncoding) {
            memcpy(glyphs, text, byteLength / 2 * 2);
            return byteLength / 2;
        } else {
            return paint.textToGlyphs(text, byteLength, glyphs);
        }
    }

    const char* data = (const char*)text;
    const char* stop = data + byteLength;
    const uint16_t* data16 = (const uint16_t*)data;
    const uint16_t* stop16 = (const uint16_t*)stop;
    uint16_t* gPtr = glyphs;
    uint16_t* gEnd = glyphs + glyphsLength;

    // For a built-in font (no resources), we may have to undo glyph encoding
    // before converting to the standard pdf encoding.
    switch(paint.getTextEncoding()) {
        case SkPaint::kUTF8_TextEncoding:
            while (data < stop && gPtr < gEnd)
                *gPtr++ = unicharToWinAnsiGlyphID(SkUTF8_NextUnichar(&data));
            SkASSERT(data >= stop);
            break;
        case SkPaint::kUTF16_TextEncoding:
            while (data16 < stop16 && gPtr < gEnd)
                *gPtr++ = unicharToWinAnsiGlyphID(SkUTF16_NextUnichar(&data16));
            SkASSERT(data16 >= stop16);
            break;
        case SkPaint::kGlyphID_TextEncoding:
            while (data16 < stop16 && gPtr < gEnd) {
                SkUnichar buf;
                paint.glyphsToUnichars(data16++, 1, &buf);
                *gPtr++ = unicharToWinAnsiGlyphID(buf);
            }
            SkASSERT(data16 >= stop16);
            break;
        default:
            SkASSERT(!"Unknown text encoding");
            break;
    }
    return gPtr - glyphs;
}

// static
SkPDFFont* SkPDFFont::getFontResouceByID(uint32_t fontID) {
    SkAutoMutexAcquire lock(canonicalFontsMutex());
    int index = find(fontID);
    if (index >= 0) {
        canonicalFonts()[index].fFont->ref();
        return canonicalFonts()[index].fFont;
    }

    // TODO(vandebo) Lookup and create the font. For now, just use the built-in
    // Helevtica.
    SkPDFFont* font = new SkPDFFont(fontID, false);
    font->populateBuiltinFont("Helvetica");

    FontRec newEntry(font, fontID);
    canonicalFonts().push(newEntry);
    return font;  // Return the reference new SkPDFFont() created.
}

// static
SkTDArray<SkPDFFont::FontRec>& SkPDFFont::canonicalFonts() {
    // This initialization is only thread safe with gcc.
    static SkTDArray<FontRec> gCanonicalFonts;
    return gCanonicalFonts;
}

// static
SkMutex& SkPDFFont::canonicalFontsMutex() {
    // This initialization is only thread safe with gcc.
    static SkMutex gCanonicalFontsMutex;
    return gCanonicalFontsMutex;
}

// static
int SkPDFFont::find(uint32_t fontID) {
    FontRec search(NULL, fontID);
    return canonicalFonts().find(search);
}

SkPDFFont::SkPDFFont(uint32_t fontID, bool multiByteGlyphs)
    : fFontID(fontID),
      fMultiByteGlyphs(multiByteGlyphs) {
}

void SkPDFFont::populateBuiltinFont(const char fontName[]) {
    SkASSERT(strcmp(fontName, "Time-Roman") == 0 ||
             strcmp(fontName, "Time-Bold") == 0 ||
             strcmp(fontName, "Time-Italic") == 0 ||
             strcmp(fontName, "Time-BoldItalic") == 0 ||
             strcmp(fontName, "Helvetica") == 0 ||
             strcmp(fontName, "Helvetica-Bold") == 0 ||
             strcmp(fontName, "Helvetica-Oblique") == 0 ||
             strcmp(fontName, "Helvetica-BoldOblique") == 0 ||
             strcmp(fontName, "Courier") == 0 ||
             strcmp(fontName, "Courier-Bold") == 0 ||
             strcmp(fontName, "Courier-Oblique") == 0 ||
             strcmp(fontName, "Courier-BoldOblique") == 0 ||
             strcmp(fontName, "Symbol") == 0 ||
             strcmp(fontName, "ZapfDingbats") == 0);

    SkRefPtr<SkPDFName> type = new SkPDFName("Font");
    type->unref();  // SkRefPtr and new both took a reference.
    insert("Type", type.get());

    SkRefPtr<SkPDFName> subType = new SkPDFName("Type1");
    subType->unref();  // SkRefPtr and new both took a reference.
    insert("Subtype", subType.get());

    SkRefPtr<SkPDFName> baseFont = new SkPDFName(fontName);
    baseFont->unref();  // SkRefPtr and new both took a reference.
    insert("BaseFont", baseFont.get());

    SkRefPtr<SkPDFName> encoding = new SkPDFName("WinAnsiEncoding");
    encoding->unref();  // SkRefPtr and new both took a reference.
    insert("Encoding", encoding.get());
}

void SkPDFFont::populateFont(const char subType[], const char fontName[],
                             int firstChar, int lastChar, int widths[],
                             SkPDFObject* fontDescriptor) {
}

bool SkPDFFont::FontRec::operator==(const SkPDFFont::FontRec& b) const {
    return fFontID == b.fFontID;
}

SkPDFFont::FontRec::FontRec(SkPDFFont* font, uint32_t fontID)
    : fFont(font),
      fFontID(fontID) {
}
