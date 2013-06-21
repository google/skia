#ifndef __DEFINED__SkPdfFont
#define __DEFINED__SkPdfFont

#include "SkPdfHeaders_autogen.h"
#include "SkPdfPodofoMapper_autogen.h"

#include <map>
#include <string>

#include "SkUtils.h"

class SkPdfType0Font;
class SkPdfType1Font;
class SkPdfType3Font;
class SkPdfTrueTypeFont;
class SkPdfCIDFont;
class SkPdfMultiMasterFont;
class SkPdfFont;


struct SkPdfStandardFontEntry {
    const char* fName;
    bool fIsBold;
    bool fIsItalic;
};

std::map<std::string, SkPdfStandardFontEntry>& getStandardFonts();
SkTypeface* SkTypefaceFromPdfStandardFont(const char* fontName, bool bold, bool italic);
SkPdfFont* SkPdfFontFromName(SkPdfObject* obj, const char* fontName);

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

class SkPdfFont {
public:
    SkPdfFont* fBaseFont;
    SkPdfEncoding* fEncoding;

public:
    SkPdfFont() : fBaseFont(NULL), fEncoding(SkPdfIdentityHEncoding::instance()) {}

    const SkPdfEncoding* encoding() const {return fEncoding;}

    void drawText(const SkUnicodeText& text, SkPaint* paint, SkCanvas* canvas, SkMatrix* matrix) {
        for (int i = 0 ; i < text.size(); i++) {
            drawOneChar(text[i], paint, canvas, matrix);
        }
    }

    virtual void ToUnicode(const SkDecodedText& textIn, SkUnicodeText* textOut) const {
        textOut->text = textIn.text;
        textOut->len = textIn.len;
    };

    static SkPdfFont* fontFromPdfDictionary(SkPdfFontDictionary* dict);
    static SkPdfFont* Default() {return SkPdfFontFromName(NULL, "TimesNewRoman");}

    static SkPdfType0Font* fontFromType0FontDictionary(SkPdfType0FontDictionary* dict);
    static SkPdfType1Font* fontFromType1FontDictionary(SkPdfType1FontDictionary* dict);
    static SkPdfType3Font* fontFromType3FontDictionary(SkPdfType3FontDictionary* dict);
    static SkPdfTrueTypeFont* fontFromTrueTypeFontDictionary(SkPdfTrueTypeFontDictionary* dict);
    static SkPdfCIDFont* fontFromCIDFontDictionary(SkPdfCIDFontDictionary* dict);
    static SkPdfMultiMasterFont* fontFromMultiMasterFontDictionary(SkPdfMultiMasterFontDictionary* dict);

public:
    virtual void drawOneChar(unsigned int ch, SkPaint* paint, SkCanvas* canvas, SkMatrix* matrix) = 0;
    virtual void afterChar(SkPaint* paint, SkMatrix* matrix) = 0;
    virtual void afterWord(SkPaint* paint, SkMatrix* matrix) = 0;
};

class SkPdfStandardFont : public SkPdfFont {
    SkTypeface* fTypeface;

public:
    SkPdfStandardFont(SkTypeface* typeface) : fTypeface(typeface) {}

public:
    virtual void drawOneChar(unsigned int ch, SkPaint* paint, SkCanvas* canvas, SkMatrix* matrix) {
        paint->setTypeface(fTypeface);
        paint->setTextEncoding(SkPaint::kUTF8_TextEncoding);

        unsigned long ch4 = ch;
        char utf8[10];
        int len = SkUTF8_FromUnichar(ch4, utf8);

        canvas->drawText(utf8, len, SkDoubleToScalar(0), SkDoubleToScalar(0), *paint);

        SkScalar textWidth = paint->measureText(utf8, len);
        matrix->preTranslate(textWidth, SkDoubleToScalar(0.0));
    }

    virtual void afterChar(SkPaint* paint, SkMatrix* matrix) {}
    virtual void afterWord(SkPaint* paint, SkMatrix* matrix) {}
};


class SkPdfType0Font : public SkPdfFont {
    unsigned short* fCMapEncoding;
    unsigned char* fCMapEncodingFlag;
public:
    SkPdfType0Font(SkPdfType0FontDictionary* dict);

public:
    virtual void ToUnicode(const SkDecodedText& textIn, SkUnicodeText* textOut) const {
        textOut->text = new uint16_t[textIn.len];
        textOut->len = textIn.len;
        for (int i = 0; i < textIn.len; i++) {
            textOut->text[i] = fCMapEncoding[textIn.text[i]];
        }
    };

    virtual void drawOneChar(unsigned int ch, SkPaint* paint, SkCanvas* canvas, SkMatrix* matrix) {
        fBaseFont->drawOneChar(ch, paint, canvas, matrix);
    }

    virtual void afterChar(SkPaint* paint, SkMatrix* matrix) {

    }

    virtual void afterWord(SkPaint* paint, SkMatrix* matrix) {

    }
};

class SkPdfTrueTypeFont : public SkPdfFont {
public:
    SkPdfTrueTypeFont(SkPdfTrueTypeFontDictionary* dict) {
        fBaseFont = SkPdfFontFromName(dict, dict->BaseFont().c_str());
    }

public:
    virtual void drawOneChar(unsigned int ch, SkPaint* paint, SkCanvas* canvas, SkMatrix* matrix) {

    }

    virtual void afterChar(SkPaint* paint, SkMatrix* matrix) {

    }

    virtual void afterWord(SkPaint* paint, SkMatrix* matrix) {

    }
};


class SkPdfType1Font : public SkPdfFont {
public:
    SkPdfType1Font(SkPdfType1FontDictionary* dict) {
        fBaseFont = SkPdfFontFromName(dict, dict->BaseFont().c_str());
    }


public:
      virtual void drawOneChar(unsigned int ch, SkPaint* paint, SkCanvas* canvas, SkMatrix* matrix) {

      }

      virtual void afterChar(SkPaint* paint, SkMatrix* matrix) {

      }

      virtual void afterWord(SkPaint* paint, SkMatrix* matrix) {

      }
 };


class SkPdfCIDFont : public SkPdfFont {
public:
    SkPdfCIDFont(SkPdfCIDFontDictionary* dict) {
        fBaseFont = SkPdfFontFromName(dict, dict->BaseFont().c_str());
    }

public:
    virtual void drawOneChar(unsigned int ch, SkPaint* paint, SkCanvas* canvas, SkMatrix* matrix) {

    }

    virtual void afterChar(SkPaint* paint, SkMatrix* matrix) {

    }

    virtual void afterWord(SkPaint* paint, SkMatrix* matrix) {

    }
};

class SkPdfMultiMasterFont : public SkPdfFont {
public:
    SkPdfMultiMasterFont(SkPdfMultiMasterFontDictionary* dict) {
        fBaseFont = SkPdfFontFromName(dict, dict->BaseFont().c_str());
    }

public:
    virtual void drawOneChar(unsigned int ch, SkPaint* paint, SkCanvas* canvas, SkMatrix* matrix) {

    }

    virtual void afterChar(SkPaint* paint, SkMatrix* matrix) {

    }

    virtual void afterWord(SkPaint* paint, SkMatrix* matrix) {

    }
};

class SkPdfType3Font : public SkPdfFont {
public:
    SkPdfType3Font(SkPdfType3FontDictionary* dict) {
        fBaseFont = SkPdfFontFromName(dict, dict->BaseFont().c_str());
    }

public:
    virtual void drawOneChar(unsigned int ch, SkPaint* paint, SkCanvas* canvas, SkMatrix* matrix) {

    }

    virtual void afterChar(SkPaint* paint, SkMatrix* matrix) {

    }

    virtual void afterWord(SkPaint* paint, SkMatrix* matrix) {

    }
};

#endif  // __DEFINED__SkPdfFont
