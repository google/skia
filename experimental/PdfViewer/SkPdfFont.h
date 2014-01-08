/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// TODO(edisonn): this file not commented much on purpose.
// It will probably need heavy refactoring soon anyway to support all encodings, fonts and
// proper text sizing and spacing

#ifndef SkPdfFont_DEFINED
#define SkPdfFont_DEFINED

#include "SkPdfContext.h"
#include "SkPdfHeaders_autogen.h"
#include "SkPdfMapper_autogen.h"
#include "SkPdfUtils.h"
#include "SkTypeface.h"
#include "SkTDict.h"
#include "SkUtils.h"

class SkPdfType0Font;
class SkPdfType1Font;
class SkPdfType3Font;
class SkPdfTrueTypeFont;
class SkPdfMultiMasterFont;
class SkPdfFont;

struct SkPdfStandardFontEntry {
    // We don't own this pointer!
    const char* fName;
    bool fIsBold;
    bool fIsItalic;
    SkPdfStandardFontEntry()
    : fName(NULL),
      fIsBold(false),
      fIsItalic(false) {}

    SkPdfStandardFontEntry(const char* name, bool bold, bool italic)
        : fName(name),
          fIsBold(bold),
          fIsItalic(italic) {}
};

SkTDict<SkPdfStandardFontEntry>& getStandardFonts();
SkTypeface* SkTypefaceFromPdfStandardFont(const char* fontName, bool bold, bool italic);
SkPdfFont* fontFromName(SkPdfNativeDoc* doc, SkPdfNativeObject* obj, const char* fontName);

struct SkUnencodedText {
    void* text;
    int len;

public:
    SkUnencodedText(const SkPdfString* obj) {
        text = (void*)obj->c_str();
        len = (int) obj->lenstr();
    }
};

struct SkDecodedText {
    uint16_t* text;
    int len;
public:
    unsigned int operator[](int i) const { return text[i]; }
    int size() const { return len; }
};

struct SkUnicodeText {
    uint16_t* text;
    int len;

public:
    unsigned int operator[](int i) const { return text[i]; }
    int size() const { return len; }
};

class SkPdfEncoding {
public:
    virtual ~SkPdfEncoding() {}
    virtual bool decodeText(const SkUnencodedText& textIn, SkDecodedText* textOut) const = 0;
    static SkPdfEncoding* fromName(const char* name);
};

SkTDict<SkPdfEncoding*>& getStandardEncodings();

class SkPdfToUnicode {
    // TODO(edisonn): hide public members
public:
    unsigned short* fCMapEncoding;
    unsigned char* fCMapEncodingFlag;

    SkPdfToUnicode(SkPdfNativeDoc* parsed, SkPdfStream* stream);
};


class SkPdfIdentityHEncoding : public SkPdfEncoding {
public:
    virtual ~SkPdfIdentityHEncoding() {}
    virtual bool decodeText(const SkUnencodedText& textIn, SkDecodedText* textOut) const {
        // TODO(edisonn): SkASSERT(textIn.len % 2 == 0); or report error?

        uint16_t* text = (uint16_t*)textIn.text;
        textOut->text = new uint16_t[textIn.len / 2];
        textOut->len = textIn.len / 2;

        for (int i = 0; i < textOut->len; i++) {
            textOut->text[i] = ((text[i] << 8) & 0xff00) | ((text[i] >> 8) & 0x00ff);
        }

        return true;
    }

    static SkPdfIdentityHEncoding* instance() {
        static SkPdfIdentityHEncoding* inst = new SkPdfIdentityHEncoding();
        return inst;
    }
};

// TODO(edisonn): using this one when no encoding is specified
class SkPdfDefaultEncoding : public SkPdfEncoding {
public:
    virtual ~SkPdfDefaultEncoding() {}
    virtual bool decodeText(const SkUnencodedText& textIn, SkDecodedText* textOut) const {
        // TODO(edisonn): SkASSERT(textIn.len % 2 == 0); or report error?

        unsigned char* text = (unsigned char*)textIn.text;
        textOut->text = new uint16_t[textIn.len];
        textOut->len = textIn.len;

        for (int i = 0; i < textOut->len; i++) {
            textOut->text[i] = text[i];
        }

        return true;
    }

    static SkPdfDefaultEncoding* instance() {
        static SkPdfDefaultEncoding* inst = new SkPdfDefaultEncoding();
        return inst;
    }
};

class SkPdfCIDToGIDMapIdentityEncoding : public SkPdfEncoding {
public:
    virtual ~SkPdfCIDToGIDMapIdentityEncoding() {}
    virtual bool decodeText(const SkUnencodedText& textIn, SkDecodedText* textOut) const {
        // TODO(edisonn): SkASSERT(textIn.len % 2 == 0); or report error?

        uint16_t* text = (uint16_t*)textIn.text;
        textOut->text = new uint16_t[textIn.len / 2];
        textOut->len = textIn.len / 2;

        for (int i = 0; i < textOut->len; i++) {
            textOut->text[i] = ((text[i] << 8) & 0xff00) | ((text[i] >> 8) & 0x00ff);
        }

        return true;
    }

    static SkPdfCIDToGIDMapIdentityEncoding* instance() {
        static SkPdfCIDToGIDMapIdentityEncoding* inst = new SkPdfCIDToGIDMapIdentityEncoding();
        return inst;
    }
};

class SkPdfFont {
public:
    SkPdfFont* fBaseFont;
    SkPdfEncoding* fEncoding;
    SkPdfToUnicode* fToUnicode;


public:
    SkPdfFont() : fBaseFont(NULL), fEncoding(SkPdfDefaultEncoding::instance()), fToUnicode(NULL) {}

    virtual ~SkPdfFont() {
        // TODO(edisonn): NYI (will leak for now)
    }

    const SkPdfEncoding* encoding() const {return fEncoding;}

    void drawText(const SkDecodedText& text, SkPaint* paint, SkPdfContext* pdfContext,
                  SkCanvas* canvas) {
        for (int i = 0 ; i < text.size(); i++) {
            canvas->setMatrix(pdfContext->fGraphicsState.fMatrixTm);
#ifdef PDF_TRACE
            SkPoint point = SkPoint::Make(SkDoubleToScalar(0), SkDoubleToScalar(0));
            pdfContext->fGraphicsState.fMatrixTm.mapPoints(&point, 1);
            printf("DrawText at (%f, %f)\n", SkScalarToDouble(point.x()),
                                             SkScalarToDouble(point.y()));
#endif  // PDF_TRACE

#ifdef PDF_TRACE_DRAWTEXT
            SkPaint col;
            col.setColor(SK_ColorMAGENTA);
            SkRect rect = SkRect::MakeXYWH(SkDoubleToScalar(0.0),
                                           SkDoubleToScalar(0.0),
                                           SkDoubleToScalar(10.0),
                                           SkDoubleToScalar(10.0));
            canvas->save();
            canvas->setMatrix(pdfContext->fGraphicsState.fMatrixTm);
            canvas->drawRect(rect, col);
            canvas->restore();
#endif
            double width = drawOneChar(text[i], paint, pdfContext, canvas);
            pdfContext->fGraphicsState.fMatrixTm.preTranslate(SkDoubleToScalar(width),
                                                              SkDoubleToScalar(0.0));
        }
    }

    void ToUnicode(const SkDecodedText& textIn, SkUnicodeText* textOut) const {
        if (fToUnicode) {
            textOut->text = new uint16_t[textIn.len];
            textOut->len = textIn.len;
            for (int i = 0; i < textIn.len; i++) {
                textOut->text[i] = fToUnicode->fCMapEncoding[textIn.text[i]];
            }
        } else {
            textOut->text = textIn.text;
            textOut->len = textIn.len;
        }
    };

    inline unsigned int ToUnicode(unsigned int ch) const {
        if (fToUnicode && fToUnicode->fCMapEncoding) {
            return fToUnicode->fCMapEncoding[ch];
        } else {
            return ch;
        }
    };

    static SkPdfFont* fontFromPdfDictionary(SkPdfNativeDoc* doc, SkPdfFontDictionary* dict);
    static SkPdfFont* Default() {return fontFromName(NULL, NULL, "TimesNewRoman");}

    static SkPdfType0Font* fontFromType0FontDictionary(SkPdfNativeDoc* doc,
                                                       SkPdfType0FontDictionary* dict);
    static SkPdfType1Font* fontFromType1FontDictionary(SkPdfNativeDoc* doc,
                                                       SkPdfType1FontDictionary* dict);
    static SkPdfType3Font* fontFromType3FontDictionary(SkPdfNativeDoc* doc,
                                                       SkPdfType3FontDictionary* dict);
    static SkPdfTrueTypeFont* fontFromTrueTypeFontDictionary(SkPdfNativeDoc* doc,
                                                             SkPdfTrueTypeFontDictionary* dict);
    static SkPdfMultiMasterFont* fontFromMultiMasterFontDictionary(
            SkPdfNativeDoc* doc, SkPdfMultiMasterFontDictionary* dict);

    static SkPdfFont* fontFromFontDescriptor(SkPdfNativeDoc* doc,
                                             SkPdfFontDescriptorDictionary* fd,
                                             bool loadFromName = true);

public:
    virtual double drawOneChar(unsigned int ch, SkPaint* paint, SkPdfContext* pdfContext,
                               SkCanvas* canvas) = 0;
    virtual void afterWord(SkPaint* paint, SkMatrix* matrix) = 0;

private:
    static SkPdfFont* fontFromPdfDictionaryOnce(SkPdfNativeDoc* doc, SkPdfFontDictionary* dict);
};

class SkPdfStandardFont : public SkPdfFont {
    SkTypeface* fTypeface;

public:
    SkPdfStandardFont(SkTypeface* typeface) : fTypeface(typeface) {}

public:
    virtual double drawOneChar(unsigned int ch, SkPaint* paint, SkPdfContext* pdfContext,
                               SkCanvas* canvas) {
        paint->setTypeface(fTypeface);
        paint->setTextEncoding(SkPaint::kUTF8_TextEncoding);

        unsigned long ch4 = ch;
        char utf8[10];
        size_t len = SkUTF8_FromUnichar((SkUnichar) ch4, utf8);

        canvas->drawText(utf8, len, SkDoubleToScalar(0), SkDoubleToScalar(0), *paint);

        SkScalar textWidth = paint->measureText(utf8, len);
        return SkScalarToDouble(textWidth);
    }

    virtual void afterWord(SkPaint* paint, SkMatrix* matrix) {}
};

class SkPdfType0Font : public SkPdfFont {
public:
    SkPdfType0Font(SkPdfNativeDoc* doc, SkPdfType0FontDictionary* dict);

public:

    virtual double drawOneChar(unsigned int ch, SkPaint* paint, SkPdfContext* pdfContext,
                               SkCanvas* canvas) {
        return fBaseFont->drawOneChar(ToUnicode(ch), paint, pdfContext, canvas);
    }

    virtual void afterWord(SkPaint* paint, SkMatrix* matrix) {
    }
};

class SkPdfType1Font : public SkPdfFont {
public:
    SkPdfType1Font(SkPdfNativeDoc* doc, SkPdfType1FontDictionary* dict) {
        if (dict->has_FontDescriptor()) {
            fBaseFont = SkPdfFont::fontFromFontDescriptor(doc, dict->FontDescriptor(doc));
        } else {
            fBaseFont = fontFromName(doc, dict, dict->BaseFont(doc).c_str());
        }

        if (dict->isEncodingAName(doc)) {
            fEncoding = SkPdfEncoding::fromName(dict->getEncodingAsName(doc).c_str());
        } else if (dict->isEncodingADictionary(doc)) {
            //SkPdfDictionary* dictEnc = dict->getEncodingAsDictionary(doc);
        }
        dict->FontDescriptor(doc);
    }

public:
      virtual double drawOneChar(unsigned int ch, SkPaint* paint, SkPdfContext* pdfContext,
                                 SkCanvas* canvas) {
          return fBaseFont->drawOneChar(ToUnicode(ch), paint, pdfContext, canvas);
      }

      virtual void afterWord(SkPaint* paint, SkMatrix* matrix) {

      }
};

class SkPdfTrueTypeFont : public SkPdfType1Font {
public:
    SkPdfTrueTypeFont(SkPdfNativeDoc* doc, SkPdfTrueTypeFontDictionary* dict)
            : SkPdfType1Font(doc, dict) {}
};

class SkPdfMultiMasterFont : public SkPdfType1Font {
public:
    SkPdfMultiMasterFont(SkPdfNativeDoc* doc, SkPdfMultiMasterFontDictionary* dict)
            : SkPdfType1Font(doc, dict) {}
};
/*
class CIDToGIDMap {
    virtual unsigned int map(unsigned int cid) = 0;
    static CIDToGIDMap* fromName(const char* name);
};

class CIDToGIDMap_Identity {
    virtual unsigned int map(unsigned int cid) { return cid; }

    static CIDToGIDMap_Identity* instance() {
        static CIDToGIDMap_Identity* inst = new CIDToGIDMap_Identity();
        return inst;
    }
};

CIDToGIDMap* CIDToGIDMap::fromName(const char* name) {
    // The only one supported right now is Identity
    if (strcmp(name, "Identity") == 0) {
        return CIDToGIDMap_Identity::instance();
    }

#ifdef PDF_TRACE
    // TODO(edisonn): warning/report
    printf("Unknown CIDToGIDMap: %s\n", name);
#endif
    return NULL;
}
CIDToGIDMap* fCidToGid;
*/

class SkPdfType3Font : public SkPdfFont {
    struct Type3FontChar {
        SkPdfNativeObject* fObj;
        double fWidth;
    };

    SkPdfDictionary* fCharProcs;
    SkPdfEncodingDictionary* fEncodingDict;
    unsigned int fFirstChar;
    unsigned int fLastChar;

    SkRect fFontBBox;
    SkMatrix fFonMatrix;

    Type3FontChar* fChars;

public:
    SkPdfType3Font(SkPdfNativeDoc* parsed, SkPdfType3FontDictionary* dict) {
        fBaseFont = fontFromName(parsed, dict, dict->BaseFont(parsed).c_str());

        if (dict->has_Encoding()) {
            if (dict->isEncodingAName(parsed)) {
                 fEncoding = SkPdfEncoding::fromName(dict->getEncodingAsName(parsed).c_str());
            } else if (dict->isEncodingAEncodingdictionary(parsed)) {
                 // No encoding.
                 fEncoding = SkPdfDefaultEncoding::instance();
                 fEncodingDict = dict->getEncodingAsEncodingdictionary(parsed);
            }
        }

        // null?
        fCharProcs = dict->CharProcs(parsed);

        fToUnicode = NULL;
        if (dict->has_ToUnicode()) {
            fToUnicode = new SkPdfToUnicode(parsed, dict->ToUnicode(parsed));
        }

        fFirstChar = (unsigned int)dict->FirstChar(parsed);
        fLastChar = (unsigned int)dict->LastChar(parsed);
        fFonMatrix = dict->has_FontMatrix() ? dict->FontMatrix(parsed) : SkMatrix::I();

        if (dict->has_FontBBox()) {
            fFontBBox = dict->FontBBox(parsed);
        }

        fChars = new Type3FontChar[fLastChar - fFirstChar + 1];

        memset(fChars, 0, sizeof(fChars[0]) * (fLastChar - fFirstChar + 1));

        const SkPdfArray* widths = dict->Widths(parsed);
        for (unsigned int i = 0 ; i < widths->size(); i++) {
            if ((fFirstChar + i) >= fFirstChar && (fFirstChar + i) <= fLastChar) {
                fChars[i].fWidth = (*widths)[i]->numberValue();
            } else {
                // TODO(edisonn): report pdf corruption
            }
        }

        const SkPdfArray* diffs = fEncodingDict->Differences(parsed);
        unsigned int j = fFirstChar;
        for (unsigned int i = 0 ; i < diffs->size(); i++) {
            if ((*diffs)[i]->isInteger()) {
                j = (unsigned int)(*diffs)[i]->intValue();
            } else if ((*diffs)[i]->isName()) {
                if (j >= fFirstChar && j <= fLastChar) {
                    fChars[j - fFirstChar].fObj = fCharProcs->get((*diffs)[i]);
                } else {
                    // TODO(edisonn): report pdf corruption
                }
                j++;
            } else {
                // TODO(edisonn): report bad pdf
            }
        }
    }

public:
    virtual double drawOneChar(unsigned int ch, SkPaint* paint, SkPdfContext* pdfContext,
                               SkCanvas* canvas) {
        if (ch < fFirstChar || ch > fLastChar || !fChars[ch - fFirstChar].fObj) {
            return fBaseFont->drawOneChar(ToUnicode(ch), paint, pdfContext, canvas);
        }

#ifdef PDF_TRACE
        printf("Type 3 char to unicode: %c\n", ToUnicode(ch));
        if (ToUnicode(ch) == 'A') {
            printf("break;\n");
        }
#endif

        // TODO(edisonn): is it better to resolve the reference at load time, or now?
        doType3Char(pdfContext,
                    canvas,
                    pdfContext->fPdfDoc->resolveReference(fChars[ch - fFirstChar].fObj),
                    fFontBBox,
                    fFonMatrix,
                    pdfContext->fGraphicsState.fCurFontSize);

        // TODO(edisonn): verify/test translate code, not tested yet
        pdfContext->fGraphicsState.fMatrixTm.preTranslate(
                SkDoubleToScalar(pdfContext->fGraphicsState.fCurFontSize *
                                     fChars[ch - fFirstChar].fWidth),
                SkDoubleToScalar(0.0));
        return fChars[ch - fFirstChar].fWidth;
    }

    virtual void afterWord(SkPaint* paint, SkMatrix* matrix) {}
};

#endif  // SkPdfFont_DEFINED
