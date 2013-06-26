#include "SkPdfFont.h"
#include "SkPdfParser.h"

std::map<std::string, SkPdfStandardFontEntry>& getStandardFonts() {
    static std::map<std::string, SkPdfStandardFontEntry> gPdfStandardFonts;

    // TODO (edisonn): , vs - ? what does it mean?
    // TODO (edisonn): MT, PS, Oblique=italic?, ... what does it mean?
    if (gPdfStandardFonts.empty()) {
        gPdfStandardFonts["Arial"] = {"Arial", false, false};
        gPdfStandardFonts["Arial,Bold"] = {"Arial", true, false};
        gPdfStandardFonts["Arial,BoldItalic"] = {"Arial", true, true};
        gPdfStandardFonts["Arial,Italic"] = {"Arial", false, true};
        gPdfStandardFonts["Arial-Bold"] = {"Arial", true, false};
        gPdfStandardFonts["Arial-BoldItalic"] = {"Arial", true, true};
        gPdfStandardFonts["Arial-BoldItalicMT"] = {"Arial", true, true};
        gPdfStandardFonts["Arial-BoldMT"] = {"Arial", true, false};
        gPdfStandardFonts["Arial-Italic"] = {"Arial", false, true};
        gPdfStandardFonts["Arial-ItalicMT"] = {"Arial", false, true};
        gPdfStandardFonts["ArialMT"] = {"Arial", false, false};
        gPdfStandardFonts["Courier"] = {"Courier New", false, false};
        gPdfStandardFonts["Courier,Bold"] = {"Courier New", true, false};
        gPdfStandardFonts["Courier,BoldItalic"] = {"Courier New", true, true};
        gPdfStandardFonts["Courier,Italic"] = {"Courier New", false, true};
        gPdfStandardFonts["Courier-Bold"] = {"Courier New", true, false};
        gPdfStandardFonts["Courier-BoldOblique"] = {"Courier New", true, true};
        gPdfStandardFonts["Courier-Oblique"] = {"Courier New", false, true};
        gPdfStandardFonts["CourierNew"] = {"Courier New", false, false};
        gPdfStandardFonts["CourierNew,Bold"] = {"Courier New", true, false};
        gPdfStandardFonts["CourierNew,BoldItalic"] = {"Courier New", true, true};
        gPdfStandardFonts["CourierNew,Italic"] = {"Courier New", false, true};
        gPdfStandardFonts["CourierNew-Bold"] = {"Courier New", true, false};
        gPdfStandardFonts["CourierNew-BoldItalic"] = {"Courier New", true, true};
        gPdfStandardFonts["CourierNew-Italic"] = {"Courier New", false, true};
        gPdfStandardFonts["CourierNewPS-BoldItalicMT"] = {"Courier New", true, true};
        gPdfStandardFonts["CourierNewPS-BoldMT"] = {"Courier New", true, false};
        gPdfStandardFonts["CourierNewPS-ItalicMT"] = {"Courier New", false, true};
        gPdfStandardFonts["CourierNewPSMT"] = {"Courier New", false, false};
        gPdfStandardFonts["Helvetica"] = {"Helvetica", false, false};
        gPdfStandardFonts["Helvetica,Bold"] = {"Helvetica", true, false};
        gPdfStandardFonts["Helvetica,BoldItalic"] = {"Helvetica", true, true};
        gPdfStandardFonts["Helvetica,Italic"] = {"Helvetica", false, true};
        gPdfStandardFonts["Helvetica-Bold"] = {"Helvetica", true, false};
        gPdfStandardFonts["Helvetica-BoldItalic"] = {"Helvetica", true, true};
        gPdfStandardFonts["Helvetica-BoldOblique"] = {"Helvetica", true, true};
        gPdfStandardFonts["Helvetica-Italic"] = {"Helvetica", false, true};
        gPdfStandardFonts["Helvetica-Oblique"] = {"Helvetica", false, true};
        gPdfStandardFonts["Times-Bold"] = {"Times New Roman", true, false};
        gPdfStandardFonts["Times-BoldItalic"] = {"Times New Roman", true, true};
        gPdfStandardFonts["Times-Italic"] = {"Times New Roman", false, true};
        gPdfStandardFonts["Times-Roman"] = {"Times New Roman", false, false};
        gPdfStandardFonts["TimesNewRoman"] = {"Times New Roman", false, false};
        gPdfStandardFonts["TimesNewRoman,Bold"] = {"Times New Roman", true, false};
        gPdfStandardFonts["TimesNewRoman,BoldItalic"] = {"Times New Roman", true, true};
        gPdfStandardFonts["TimesNewRoman,Italic"] = {"Times New Roman", false, true};
        gPdfStandardFonts["TimesNewRoman-Bold"] = {"Times New Roman", true, false};
        gPdfStandardFonts["TimesNewRoman-BoldItalic"] = {"Times New Roman", true, true};
        gPdfStandardFonts["TimesNewRoman-Italic"] = {"Times New Roman", false, true};
        gPdfStandardFonts["TimesNewRomanPS"] = {"Times New Roman", false, false};
        gPdfStandardFonts["TimesNewRomanPS-Bold"] = {"Times New Roman", true, false};
        gPdfStandardFonts["TimesNewRomanPS-BoldItalic"] = {"Times New Roman", true, true};
        gPdfStandardFonts["TimesNewRomanPS-BoldItalicMT"] = {"Times New Roman", true, true};
        gPdfStandardFonts["TimesNewRomanPS-BoldMT"] = {"Times New Roman", true, false};
        gPdfStandardFonts["TimesNewRomanPS-Italic"] = {"Times New Roman", false, true};
        gPdfStandardFonts["TimesNewRomanPS-ItalicMT"] = {"Times New Roman", false, true};
        gPdfStandardFonts["TimesNewRomanPSMT"] = {"Times New Roman", false, false};
        gPdfStandardFonts["Symbol"] = {"Symbol", false, false};
        gPdfStandardFonts["ZapfDingbats"] = {"ZapfDingbats", false, false};

        // TODO(edisonn): these are hacks. Load Post Script font name.
        // see FT_Get_Postscript_Name
        // Font config is not using it, yet.
        //https://bugs.freedesktop.org/show_bug.cgi?id=18095

        gPdfStandardFonts["Arial-Black"] = {"Arial", true, false};
        gPdfStandardFonts["DejaVuSans"] = {"DejaVu Sans", false, false};
        gPdfStandardFonts["DejaVuSansMono"] = {"DejaVuSans Mono", false, false};
        gPdfStandardFonts["DejaVuSansMono-Bold"] = {"DejaVuSans Mono", true, false};
        gPdfStandardFonts["DejaVuSansMono-Oblique"] = {"DejaVuSans Mono", false, true};
        gPdfStandardFonts["Georgia-Bold"] = {"Georgia", true, false};
        gPdfStandardFonts["Georgia-BoldItalic"] = {"Georgia", true, true};
        gPdfStandardFonts["Georgia-Italic"] = {"Georgia", false, true};
        gPdfStandardFonts["TrebuchetMS"] = {"Trebuchet MS", false, false};
        gPdfStandardFonts["TrebuchetMS-Bold"] = {"Trebuchet MS", true, false};
        gPdfStandardFonts["Verdana-Bold"] = {"Verdana", true, false};
        gPdfStandardFonts["WenQuanYiMicroHei"] = {"WenQuanYi Micro Hei", false, false};

        // TODO(edisonn): list all phonts available, builf post script name as in pdf spec
        /*
         * The PostScript name for the value of BaseFontis determined in one of two ways:
• Use the PostScript name that is an optional entry in the “name” table of the
TrueType font itself.
• In the absence of such an entry in the “name” table, derive a PostScript name
from the name by which the font is known in the host operating system: on a
Windows system, it is based on the lfFaceName ﬁeld in a LOGFONT structure; in
the Mac OS, it is based on the name of the FONDresource. If the name contains
any spaces, the spaces are removed.
If the font in a source document uses a bold or italic style, but there is no font
data for that style, the host operating system will synthesize the style. In this case,
a comma and the style name (one of Bold, Italic, or BoldItalic) are appended to the
font name. For example, for a TrueType font that is a bold variant of the New
         */

        /*
         * If the value of Subtype is MMType1.
• If the PostScript name of the instance contains spaces, the spaces are replaced
by underscores in the value of BaseFont. For instance, as illustrated in Example
5.7, the name “MinionMM 366 465 11 ” (which ends with a space character)
becomes /MinionMM_366_465_11_.
         */

        // might not work on all oses ?
        //

    }

    return gPdfStandardFonts;
}

SkTypeface* SkTypefaceFromPdfStandardFont(const char* fontName, bool bold, bool italic) {
    std::map<std::string, SkPdfStandardFontEntry>& standardFontMap = getStandardFonts();

    SkTypeface* typeface = NULL;
    if (standardFontMap.find(fontName) != standardFontMap.end()) {
        SkPdfStandardFontEntry fontData = standardFontMap[fontName];

        // TODO(edisonn): How does the bold/italic specified in standard definition combines with
        // the one in /font key? use OR for now.
        bold = bold || fontData.fIsBold;
        italic = italic || fontData.fIsItalic;

        typeface = SkTypeface::CreateFromName(
            fontData.fName,
            SkTypeface::Style((bold ? SkTypeface::kBold : 0) |
                              (italic ? SkTypeface::kItalic : 0)));
    } else {
        typeface = SkTypeface::CreateFromName(
            fontName,
            SkTypeface::kNormal);
    }

    if (typeface) {
        typeface->ref();
    }
    return typeface;
}

SkPdfFont* SkPdfFontFromName(SkPdfObject* obj, const char* fontName) {
    SkTypeface* typeface = SkTypefaceFromPdfStandardFont(fontName, false, false);
    if (typeface != NULL) {
        return new SkPdfStandardFont(typeface);
    }
//        SkPdfObject* font = obtainFont(pdfContext, fontName);
//        if (!font->asFontDictionary()) {
//            return NULL;
//        }
//        SkPdfFont::fontFromPdfDictionary(font->asDictionary());
//    }
    // TODO(edisonn): deal with inherited fonts, load from parent objects
    return SkPdfFont::Default();
}

SkPdfFont* SkPdfFont::fontFromPdfDictionary(SkPdfFontDictionary* dict) {
    if (dict == NULL) {
        return NULL;  // TODO(edisonn): report default one?
    }

    switch (dict->getType()) {
        case kType0FontDictionary_SkPdfObjectType:
            return fontFromType0FontDictionary(dict->asType0FontDictionary());

        case kTrueTypeFontDictionary_SkPdfObjectType:
            return fontFromTrueTypeFontDictionary(dict->asTrueTypeFontDictionary());

        case kType1FontDictionary_SkPdfObjectType:
            return fontFromType1FontDictionary(dict->asType1FontDictionary());

        case kCIDFontDictionary_SkPdfObjectType:
            return fontFromCIDFontDictionary(dict->asCIDFontDictionary());

        case kMultiMasterFontDictionary_SkPdfObjectType:
            return fontFromMultiMasterFontDictionary(dict->asMultiMasterFontDictionary());

        case kType3FontDictionary_SkPdfObjectType:
            return fontFromType3FontDictionary(dict->asType3FontDictionary());
    }
    return NULL;  // TODO(edisonn): report error?
}

SkPdfType0Font* SkPdfFont::fontFromType0FontDictionary(SkPdfType0FontDictionary* dict) {
    if (dict == NULL) {
        return NULL;  // default one?
    }

    return new SkPdfType0Font(dict);
}

SkPdfType1Font* SkPdfFont:: fontFromType1FontDictionary(SkPdfType1FontDictionary* dict) {
    if (dict == NULL) {
        return NULL;  // default one?
    }

    return new SkPdfType1Font(dict);
}

SkPdfType3Font* SkPdfFont::fontFromType3FontDictionary(SkPdfType3FontDictionary* dict) {
    if (dict == NULL) {
        return NULL;  // default one?
    }



    return new SkPdfType3Font(dict);
}

SkPdfTrueTypeFont* SkPdfFont::fontFromTrueTypeFontDictionary(SkPdfTrueTypeFontDictionary* dict) {
    if (dict == NULL) {
        return NULL;  // default one?
    }

    return new SkPdfTrueTypeFont(dict);
}

SkPdfCIDFont* SkPdfFont::fontFromCIDFontDictionary(SkPdfCIDFontDictionary* dict) {
    if (dict == NULL) {
        return NULL;  // default one?
    }

    return new SkPdfCIDFont(dict);
}

SkPdfMultiMasterFont* SkPdfFont::fontFromMultiMasterFontDictionary(SkPdfMultiMasterFontDictionary* dict) {
    if (dict == NULL) {
        return NULL;  // default one?
    }

    return new SkPdfMultiMasterFont(dict);
}

static int skstoi(const SkPdfString* str) {
    int ret = 0;
    for (int i = 0 ; i < str->podofo()->GetString().GetLength(); i++) {
        ret = (ret << 8) + ((unsigned char*)str->podofo()->GetString().GetString())[i];
    }
    return ret;
}

SkPdfToUnicode::SkPdfToUnicode(const SkPdfStream* stream) {
    fCMapEncoding = NULL;
    fCMapEncodingFlag = NULL;

    if (stream) {
        SkPdfTokenizer tokenizer(stream);
        PdfToken token;

        fCMapEncoding = new unsigned short[256 * 256];
        fCMapEncodingFlag = new unsigned char[256 * 256];
        for (int i = 0 ; i < 256 * 256; i++) {
            fCMapEncoding[i] = i;
            fCMapEncodingFlag[i] = 0;
        }

        // TODO(edisonn): deal with multibyte character, or longer strings.
        // Ritght now we deal with up 2 characters, e.g. <0020> but not longer like <00660066006C>
        //2 beginbfrange
        //<0000> <005E> <0020>
        //<005F> <0061> [<00660066> <00660069> <00660066006C>]

        while (tokenizer.readToken(&token)) {

            if (token.fType == kKeyword_TokenType && strcmp(token.fKeyword, "begincodespacerange") == 0) {
                while (tokenizer.readToken(&token) && !(token.fType == kKeyword_TokenType && strcmp(token.fKeyword, "endcodespacerange") == 0)) {
//                    tokenizer.PutBack(token);
//                    tokenizer.readToken(&token);
                    // TODO(edisonn): check token type! ignore/report errors.
                    int start = skstoi(token.fObject->asString());
                    tokenizer.readToken(&token);
                    int end = skstoi(token.fObject->asString());
                    for (int i = start; i <= end; i++) {
                        fCMapEncodingFlag[i] |= 1;
                    }
                }
            }

            if (token.fType == kKeyword_TokenType && strcmp(token.fKeyword, "beginbfchar") == 0) {
                while (tokenizer.readToken(&token) && !(token.fType == kKeyword_TokenType && strcmp(token.fKeyword, "endbfchar") == 0)) {
//                    tokenizer.PutBack(token);
//                    tokenizer.readToken(&token);
                    int from = skstoi(token.fObject->asString());
                    tokenizer.readToken(&token);
                    int to = skstoi(token.fObject->asString());

                    fCMapEncodingFlag[from] |= 2;
                    fCMapEncoding[from] = to;
                }
            }

            if (token.fType == kKeyword_TokenType && strcmp(token.fKeyword, "beginbfrange") == 0) {
                while (tokenizer.readToken(&token) && !(token.fType == kKeyword_TokenType && strcmp(token.fKeyword, "endbfrange") == 0)) {
//                    tokenizer.PutBack(token);
//                    tokenizer.readToken(&token);
                    int start = skstoi(token.fObject->asString());
                    tokenizer.readToken(&token);
                    int end = skstoi(token.fObject->asString());


                    tokenizer.readToken(&token); // [ or just an array directly?
//                    tokenizer.PutBack(token);

                    if (token.fType == kObject_TokenType && token.fObject->asString()) {
//                        tokenizer.readToken(&token);
                        int value = skstoi(token.fObject->asString());

                        for (int i = start; i <= end; i++) {
                            fCMapEncodingFlag[i] |= 2;
                            fCMapEncoding[i] = value;
                            value++;
                            // if i != end, verify last byte id not if, ignore/report error
                        }

                        // read one string
                    } else if (token.fType == kObject_TokenType && token.fObject->asArray()) {
//                        tokenizer.readToken(&token);
                        for (int i = 0; i < token.fObject->asArray()->size(); i++) {
                            fCMapEncodingFlag[start + i] |= 2;
                            fCMapEncoding[start + i] = skstoi((*token.fObject->asArray())[i]->asString());
                        }
                        // read array
                    }

                    //fCMapEncodingFlag[from] = 1;
                    //fCMapEncoding[from] = to;
                }
            }
        }
    }
}


SkPdfType0Font::SkPdfType0Font(SkPdfType0FontDictionary* dict) {
    fBaseFont = SkPdfFontFromName(dict, dict->BaseFont().c_str());
    fEncoding = NULL;

    if (dict->has_Encoding()) {
        if (dict->isEncodingAName()) {
            fEncoding = SkPdfEncoding::fromName(dict->getEncodingAsName().c_str());
        } else if (dict->isEncodingAStream()) {
            //fEncoding = loadEncodingFromStream(dict->getEncodingAsStream());
        } else {
            // TODO(edisonn): error ... warning .. assert?
        }
    }

    if (dict->has_ToUnicode()) {
        fToUnicode = new SkPdfToUnicode(dict->ToUnicode());
    }
}

std::map<std::string, SkPdfEncoding*>& getStandardEncodings() {
    static std::map<std::string, SkPdfEncoding*> encodings;
    if (encodings.empty()) {
        encodings["Identity-H"] = SkPdfIdentityHEncoding::instance();
    }

    return encodings;
}


SkPdfEncoding* SkPdfEncoding::fromName(const char* name) {
    SkPdfEncoding* encoding = getStandardEncodings()[name];

#ifdef PDF_TRACE
    if (encoding == NULL) {
        printf("Encoding not found: %s\n", name);
    }
#endif
    return encoding;
}
