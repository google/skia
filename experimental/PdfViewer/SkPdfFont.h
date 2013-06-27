#ifndef __DEFINED__SkPdfFont
#define __DEFINED__SkPdfFont

#include "SkPdfHeaders_autogen.h"
#include "SkPdfPodofoMapper_autogen.h"

#include <map>
#include <string>

#include "SkTypeface.h"
#include "SkUtils.h"
#include "SkPdfBasics.h"
#include "SkPdfUtils.h"


class SkPdfType0Font;
class SkPdfType1Font;
class SkPdfType3Font;
class SkPdfTrueTypeFont;
class SkPdfMultiMasterFont;
class SkPdfFont;


struct SkPdfStandardFontEntry {
    const char* fName;
    bool fIsBold;
    bool fIsItalic;
};

std::map<std::string, SkPdfStandardFontEntry>& getStandardFonts();
SkTypeface* SkTypefaceFromPdfStandardFont(const char* fontName, bool bold, bool italic);
SkPdfFont* fontFromName(SkPdfObject* obj, const char* fontName);

struct SkUnencodedText {
    void* text;
    int len;

public:
    SkUnencodedText(const SkPdfObject* obj) {
        text = (void*)obj->podofo()->GetString().GetString();
        len = obj->podofo()->GetString().GetLength();
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
    virtual bool decodeText(const SkUnencodedText& textIn, SkDecodedText* textOut) const = 0;
    static SkPdfEncoding* fromName(const char* name);
};

std::map<std::string, SkPdfEncoding*>& getStandardEncodings();

class SkPdfToUnicode {
    // TODO(edisonn): hide public members
public:
    unsigned short* fCMapEncoding;
    unsigned char* fCMapEncodingFlag;

    SkPdfToUnicode(const SkPdfStream* stream);
};


class SkPdfIdentityHEncoding : public SkPdfEncoding {
public:
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

    const SkPdfEncoding* encoding() const {return fEncoding;}

    void drawText(const SkDecodedText& text, SkPaint* paint, PdfContext* pdfContext, SkCanvas* canvas) {
        for (int i = 0 ; i < text.size(); i++) {
            double width = drawOneChar(text[i], paint, pdfContext, canvas);
            pdfContext->fGraphicsState.fMatrixTm.preTranslate(SkDoubleToScalar(width), SkDoubleToScalar(0.0));
            canvas->translate(SkDoubleToScalar(width), SkDoubleToScalar(0.0));
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
        if (fToUnicode) {
            return fToUnicode->fCMapEncoding[ch];
        } else {
            return ch;
        }
    };

    static SkPdfFont* fontFromPdfDictionary(SkPdfFontDictionary* dict);
    static SkPdfFont* Default() {return fontFromName(NULL, "TimesNewRoman");}

    static SkPdfType0Font* fontFromType0FontDictionary(SkPdfType0FontDictionary* dict);
    static SkPdfType1Font* fontFromType1FontDictionary(SkPdfType1FontDictionary* dict);
    static SkPdfType3Font* fontFromType3FontDictionary(SkPdfType3FontDictionary* dict);
    static SkPdfTrueTypeFont* fontFromTrueTypeFontDictionary(SkPdfTrueTypeFontDictionary* dict);
    static SkPdfMultiMasterFont* fontFromMultiMasterFontDictionary(SkPdfMultiMasterFontDictionary* dict);

    static SkPdfFont* fontFromFontDescriptor(SkPdfFontDescriptorDictionary* fd, bool loadFromName = true);

public:
    virtual double drawOneChar(unsigned int ch, SkPaint* paint, PdfContext* pdfContext, SkCanvas* canvas) = 0;
    virtual void afterWord(SkPaint* paint, SkMatrix* matrix) = 0;
};

class SkPdfStandardFont : public SkPdfFont {
    SkTypeface* fTypeface;

public:
    SkPdfStandardFont(SkTypeface* typeface) : fTypeface(typeface) {}

public:
    virtual double drawOneChar(unsigned int ch, SkPaint* paint, PdfContext* pdfContext, SkCanvas* canvas) {
        paint->setTypeface(fTypeface);
        paint->setTextEncoding(SkPaint::kUTF8_TextEncoding);

        unsigned long ch4 = ch;
        char utf8[10];
        int len = SkUTF8_FromUnichar(ch4, utf8);

        canvas->drawText(utf8, len, SkDoubleToScalar(0), SkDoubleToScalar(0), *paint);

        SkScalar textWidth = paint->measureText(utf8, len);
        return SkScalarToDouble(textWidth);
    }

    virtual void afterWord(SkPaint* paint, SkMatrix* matrix) {}
};

class SkPdfType0Font : public SkPdfFont {
public:
    SkPdfType0Font(SkPdfType0FontDictionary* dict);

public:

    virtual double drawOneChar(unsigned int ch, SkPaint* paint, PdfContext* pdfContext, SkCanvas* canvas) {
        return fBaseFont->drawOneChar(ToUnicode(ch), paint, pdfContext, canvas);
    }

    virtual void afterWord(SkPaint* paint, SkMatrix* matrix) {
    }
};

class SkPdfType1Font : public SkPdfFont {
public:
    SkPdfType1Font(SkPdfType1FontDictionary* dict) {
        if (dict->has_FontDescriptor()) {
            fBaseFont = SkPdfFont::fontFromFontDescriptor(dict->FontDescriptor());
        } else {
            fBaseFont = fontFromName(dict, dict->BaseFont().c_str());
        }
    }

public:
      virtual double drawOneChar(unsigned int ch, SkPaint* paint, PdfContext* pdfContext, SkCanvas* canvas) {
          return fBaseFont->drawOneChar(ToUnicode(ch), paint, pdfContext, canvas);
      }

      virtual void afterWord(SkPaint* paint, SkMatrix* matrix) {

      }
};

class SkPdfTrueTypeFont : public SkPdfType1Font {
public:
    SkPdfTrueTypeFont(SkPdfTrueTypeFontDictionary* dict) : SkPdfType1Font(dict) {
    }
};

class SkPdfMultiMasterFont : public SkPdfType1Font {
public:
    SkPdfMultiMasterFont(SkPdfMultiMasterFontDictionary* dict) : SkPdfType1Font(dict) {
    }
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
        SkPdfObject* fObj;
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
    SkPdfType3Font(SkPdfType3FontDictionary* dict) {
        fBaseFont = fontFromName(dict, dict->BaseFont().c_str());

        if (dict->has_Encoding()) {
            if (dict->isEncodingAName()) {
                 fEncoding = SkPdfEncoding::fromName(dict->getEncodingAsName().c_str());
            } else if (dict->isEncodingAEncodingdictionary()) {
                 // technically, there is no encoding.
                 fEncoding = SkPdfCIDToGIDMapIdentityEncoding::instance();
                 fEncodingDict = dict->getEncodingAsEncodingdictionary();
            }
        }

        // null?
        fCharProcs = dict->CharProcs();

        fToUnicode = NULL;
        if (dict->has_ToUnicode()) {
            fToUnicode = new SkPdfToUnicode(dict->ToUnicode());
        }

        fFirstChar = dict->FirstChar();
        fLastChar = dict->LastChar();
        fFonMatrix = dict->has_FontMatrix() ? *dict->FontMatrix() : SkMatrix::I();

        if (dict->FontBBox()) {
            fFontBBox = *dict->FontBBox();
        }

        fChars = new Type3FontChar[fLastChar - fFirstChar + 1];

        memset(fChars, 0, sizeof(fChars[0]) * (fLastChar - fFirstChar + 1));


        SkPdfArray* widths = dict->Widths();
        for (int i = 0 ; i < widths->size(); i++) {
            if ((fFirstChar + i) < fFirstChar || (fFirstChar + i) > fLastChar) {
                printf("break; error 1\n");
            }
            fChars[i].fWidth = (*widths)[i]->asNumber()->value();
        }

        SkPdfArray* diffs = fEncodingDict->Differences();
        int j = fFirstChar;
        for (int i = 0 ; i < diffs->size(); i++) {
            if ((*diffs)[i]->asInteger()) {
                j = (*diffs)[i]->asInteger()->value();
            } else if ((*diffs)[i]->asName()) {
                if (j < fFirstChar || j > fLastChar) {
                    printf("break; error 2\n");
                }
                fChars[j - fFirstChar].fObj = fCharProcs->get((*diffs)[i]->asName()->value().c_str());
                j++;
            } else {
                // err
            }
        }
    }

public:
    virtual double drawOneChar(unsigned int ch, SkPaint* paint, PdfContext* pdfContext, SkCanvas* canvas) {
        if (ch < fFirstChar || ch > fLastChar || !fChars[ch - fFirstChar].fObj) {
            return fBaseFont->drawOneChar(ToUnicode(ch), paint, pdfContext, canvas);
        }

#ifdef PDF_TRACE
        printf("Type 3 char to unicode: %c\n", ToUnicode(ch));
        if (ToUnicode(ch) == 'A') {
            printf("break;\n");
        }
#endif

        doType3Char(pdfContext, canvas, fChars[ch - fFirstChar].fObj, fFontBBox, fFonMatrix, pdfContext->fGraphicsState.fCurFontSize);

        // TODO(edisonn): verify/test translate code, not tested yet
        pdfContext->fGraphicsState.fMatrixTm.preTranslate(SkDoubleToScalar(pdfContext->fGraphicsState.fCurFontSize * fChars[ch - fFirstChar].fWidth),
                             SkDoubleToScalar(0.0));
    }

    virtual void afterWord(SkPaint* paint, SkMatrix* matrix) {

    }
};

#endif  // __DEFINED__SkPdfFont
