#include "SkPdfFont.h"

#include "SkStream.h"
#include "SkTypeface.h"
#include "SkPdfNativeTokenizer.h"

std::map<std::string, SkPdfStandardFontEntry>& getStandardFonts() {
    static std::map<std::string, SkPdfStandardFontEntry> gPdfStandardFonts;

    // TODO (edisonn): , vs - ? what does it mean?
    // TODO (edisonn): MT, PS, Oblique=italic?, ... what does it mean?
    if (gPdfStandardFonts.empty()) {
        gPdfStandardFonts["Arial"] = SkPdfStandardFontEntry("Arial", false, false);
        gPdfStandardFonts["Arial,Bold"] = SkPdfStandardFontEntry("Arial", true, false);
        gPdfStandardFonts["Arial,BoldItalic"] = SkPdfStandardFontEntry("Arial", true, true);
        gPdfStandardFonts["Arial,Italic"] = SkPdfStandardFontEntry("Arial", false, true);
        gPdfStandardFonts["Arial-Bold"] = SkPdfStandardFontEntry("Arial", true, false);
        gPdfStandardFonts["Arial-BoldItalic"] = SkPdfStandardFontEntry("Arial", true, true);
        gPdfStandardFonts["Arial-BoldItalicMT"] = SkPdfStandardFontEntry("Arial", true, true);
        gPdfStandardFonts["Arial-BoldMT"] = SkPdfStandardFontEntry("Arial", true, false);
        gPdfStandardFonts["Arial-Italic"] = SkPdfStandardFontEntry("Arial", false, true);
        gPdfStandardFonts["Arial-ItalicMT"] = SkPdfStandardFontEntry("Arial", false, true);
        gPdfStandardFonts["ArialMT"] = SkPdfStandardFontEntry("Arial", false, false);
        gPdfStandardFonts["Courier"] = SkPdfStandardFontEntry("Courier New", false, false);
        gPdfStandardFonts["Courier,Bold"] = SkPdfStandardFontEntry("Courier New", true, false);
        gPdfStandardFonts["Courier,BoldItalic"] = SkPdfStandardFontEntry("Courier New", true, true);
        gPdfStandardFonts["Courier,Italic"] = SkPdfStandardFontEntry("Courier New", false, true);
        gPdfStandardFonts["Courier-Bold"] = SkPdfStandardFontEntry("Courier New", true, false);
        gPdfStandardFonts["Courier-BoldOblique"] = SkPdfStandardFontEntry("Courier New", true, true);
        gPdfStandardFonts["Courier-Oblique"] = SkPdfStandardFontEntry("Courier New", false, true);
        gPdfStandardFonts["CourierNew"] = SkPdfStandardFontEntry("Courier New", false, false);
        gPdfStandardFonts["CourierNew,Bold"] = SkPdfStandardFontEntry("Courier New", true, false);
        gPdfStandardFonts["CourierNew,BoldItalic"] = SkPdfStandardFontEntry("Courier New", true, true);
        gPdfStandardFonts["CourierNew,Italic"] = SkPdfStandardFontEntry("Courier New", false, true);
        gPdfStandardFonts["CourierNew-Bold"] = SkPdfStandardFontEntry("Courier New", true, false);
        gPdfStandardFonts["CourierNew-BoldItalic"] = SkPdfStandardFontEntry("Courier New", true, true);
        gPdfStandardFonts["CourierNew-Italic"] = SkPdfStandardFontEntry("Courier New", false, true);
        gPdfStandardFonts["CourierNewPS-BoldItalicMT"] = SkPdfStandardFontEntry("Courier New", true, true);
        gPdfStandardFonts["CourierNewPS-BoldMT"] = SkPdfStandardFontEntry("Courier New", true, false);
        gPdfStandardFonts["CourierNewPS-ItalicMT"] = SkPdfStandardFontEntry("Courier New", false, true);
        gPdfStandardFonts["CourierNewPSMT"] = SkPdfStandardFontEntry("Courier New", false, false);
        gPdfStandardFonts["Helvetica"] = SkPdfStandardFontEntry("Helvetica", false, false);
        gPdfStandardFonts["Helvetica,Bold"] = SkPdfStandardFontEntry("Helvetica", true, false);
        gPdfStandardFonts["Helvetica,BoldItalic"] = SkPdfStandardFontEntry("Helvetica", true, true);
        gPdfStandardFonts["Helvetica,Italic"] = SkPdfStandardFontEntry("Helvetica", false, true);
        gPdfStandardFonts["Helvetica-Bold"] = SkPdfStandardFontEntry("Helvetica", true, false);
        gPdfStandardFonts["Helvetica-BoldItalic"] = SkPdfStandardFontEntry("Helvetica", true, true);
        gPdfStandardFonts["Helvetica-BoldOblique"] = SkPdfStandardFontEntry("Helvetica", true, true);
        gPdfStandardFonts["Helvetica-Italic"] = SkPdfStandardFontEntry("Helvetica", false, true);
        gPdfStandardFonts["Helvetica-Oblique"] = SkPdfStandardFontEntry("Helvetica", false, true);
        gPdfStandardFonts["Times-Bold"] = SkPdfStandardFontEntry("Times New Roman", true, false);
        gPdfStandardFonts["Times-BoldItalic"] = SkPdfStandardFontEntry("Times New Roman", true, true);
        gPdfStandardFonts["Times-Italic"] = SkPdfStandardFontEntry("Times New Roman", false, true);
        gPdfStandardFonts["Times-Roman"] = SkPdfStandardFontEntry("Times New Roman", false, false);
        gPdfStandardFonts["TimesNewRoman"] = SkPdfStandardFontEntry("Times New Roman", false, false);
        gPdfStandardFonts["TimesNewRoman,Bold"] = SkPdfStandardFontEntry("Times New Roman", true, false);
        gPdfStandardFonts["TimesNewRoman,BoldItalic"] = SkPdfStandardFontEntry("Times New Roman", true, true);
        gPdfStandardFonts["TimesNewRoman,Italic"] = SkPdfStandardFontEntry("Times New Roman", false, true);
        gPdfStandardFonts["TimesNewRoman-Bold"] = SkPdfStandardFontEntry("Times New Roman", true, false);
        gPdfStandardFonts["TimesNewRoman-BoldItalic"] = SkPdfStandardFontEntry("Times New Roman", true, true);
        gPdfStandardFonts["TimesNewRoman-Italic"] = SkPdfStandardFontEntry("Times New Roman", false, true);
        gPdfStandardFonts["TimesNewRomanPS"] = SkPdfStandardFontEntry("Times New Roman", false, false);
        gPdfStandardFonts["TimesNewRomanPS-Bold"] = SkPdfStandardFontEntry("Times New Roman", true, false);
        gPdfStandardFonts["TimesNewRomanPS-BoldItalic"] = SkPdfStandardFontEntry("Times New Roman", true, true);
        gPdfStandardFonts["TimesNewRomanPS-BoldItalicMT"] = SkPdfStandardFontEntry("Times New Roman", true, true);
        gPdfStandardFonts["TimesNewRomanPS-BoldMT"] = SkPdfStandardFontEntry("Times New Roman", true, false);
        gPdfStandardFonts["TimesNewRomanPS-Italic"] = SkPdfStandardFontEntry("Times New Roman", false, true);
        gPdfStandardFonts["TimesNewRomanPS-ItalicMT"] = SkPdfStandardFontEntry("Times New Roman", false, true);
        gPdfStandardFonts["TimesNewRomanPSMT"] = SkPdfStandardFontEntry("Times New Roman", false, false);
        gPdfStandardFonts["Symbol"] = SkPdfStandardFontEntry("Symbol", false, false);
        gPdfStandardFonts["ZapfDingbats"] = SkPdfStandardFontEntry("ZapfDingbats", false, false);

        // TODO(edisonn): these are hacks. Load Post Script font name.
        // see FT_Get_Postscript_Name
        // Font config is not using it, yet.
        //https://bugs.freedesktop.org/show_bug.cgi?id=18095

        gPdfStandardFonts["Arial-Black"] = SkPdfStandardFontEntry("Arial", true, false);
        gPdfStandardFonts["DejaVuSans"] = SkPdfStandardFontEntry("DejaVu Sans", false, false);
        gPdfStandardFonts["DejaVuSansMono"] = SkPdfStandardFontEntry("DejaVuSans Mono", false, false);
        gPdfStandardFonts["DejaVuSansMono-Bold"] = SkPdfStandardFontEntry("DejaVuSans Mono", true, false);
        gPdfStandardFonts["DejaVuSansMono-Oblique"] = SkPdfStandardFontEntry("DejaVuSans Mono", false, true);
        gPdfStandardFonts["Georgia-Bold"] = SkPdfStandardFontEntry("Georgia", true, false);
        gPdfStandardFonts["Georgia-BoldItalic"] = SkPdfStandardFontEntry("Georgia", true, true);
        gPdfStandardFonts["Georgia-Italic"] = SkPdfStandardFontEntry("Georgia", false, true);
        gPdfStandardFonts["TrebuchetMS"] = SkPdfStandardFontEntry("Trebuchet MS", false, false);
        gPdfStandardFonts["TrebuchetMS-Bold"] = SkPdfStandardFontEntry("Trebuchet MS", true, false);
        gPdfStandardFonts["Verdana-Bold"] = SkPdfStandardFontEntry("Verdana", true, false);
        gPdfStandardFonts["WenQuanYiMicroHei"] = SkPdfStandardFontEntry("WenQuanYi Micro Hei", false, false);

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

SkPdfFont* SkPdfFont::fontFromFontDescriptor(SkNativeParsedPDF* doc, SkPdfFontDescriptorDictionary* fd, bool loadFromName) {
    // TODO(edisonn): partial implementation ... also const handling ...
    // Only one, at most be available
    SkPdfStream* pdfStream = NULL;
    if (fd->has_FontFile()) {
        pdfStream = fd->FontFile(doc);
    } else if (fd->has_FontFile2()) {
        pdfStream = fd->FontFile2(doc);
    } if (fd->has_FontFile3()) {
        pdfStream = fd->FontFile3(doc);
    } else {
        if (loadFromName) {
            return fontFromName(doc, fd, fd->FontName(doc).c_str());
        }
    }

    const unsigned char* uncompressedStream = NULL;
    size_t uncompressedStreamLength = 0;

    // TODO(edisonn): report warning to be used in testing.
    if (!pdfStream ||
            !pdfStream->GetFilteredStreamRef(&uncompressedStream, &uncompressedStreamLength) ||
            !uncompressedStream ||
            !uncompressedStreamLength) {
        return NULL;
    }

    SkMemoryStream* skStream = new SkMemoryStream(uncompressedStream, uncompressedStreamLength);
    SkTypeface* face = SkTypeface::CreateFromStream(skStream);

    if (face == NULL) {
        // TODO(edisonn): report warning to be used in testing.
        return NULL;
    }

    face->ref();

    return new SkPdfStandardFont(face);
}

SkPdfFont* fontFromName(SkNativeParsedPDF* doc, SkPdfObject* obj, const char* fontName) {
    SkTypeface* typeface = SkTypefaceFromPdfStandardFont(fontName, false, false);
    if (typeface != NULL) {
        return new SkPdfStandardFont(typeface);
    }

    // TODO(edisonn): perf - make a map
    for (unsigned int i = 0 ; i < doc->objects(); i++) {
        SkPdfObject* obj = doc->object(i);
        if (!obj || !obj->isDictionary()) {
            continue;
        }

        SkPdfFontDescriptorDictionary* fd = obj->asDictionary()->asFontDescriptorDictionary();

        if (!fd->valid()) {
            continue;
        }

        if (fd->has_FontName() && fd->FontName(doc) == fontName) {
            SkPdfFont* font = SkPdfFont::fontFromFontDescriptor(doc, fd, false);
            if (font) {
                return font;
            } else {
                // failed to load font descriptor
                break;
            }
        }
    }

    // TODO(edisonn): warning/report issue
    return SkPdfFont::Default();
}

SkPdfFont* SkPdfFont::fontFromPdfDictionaryOnce(SkNativeParsedPDF* doc, SkPdfFontDictionary* dict) {
    // TODO(edisonn): keep the type in a smart way in the SkPdfObject
    // 1) flag, isResolved (1bit): reset at reset, add/remove/update (array) and set(dict)
    // in a tree like structure, 3-4 bits for all the datatypes inheriting from obj (int, real, ...)
    // if is a dict, reserveve a few bytes to encode type of dict, and so on like in a tree
    // issue: type can be determined from context! atribute night be missing/wrong
    switch (doc->mapper()->mapFontDictionary(dict)) {
        case kType0FontDictionary_SkPdfObjectType:
            return fontFromType0FontDictionary(doc, dict->asType0FontDictionary());

        case kTrueTypeFontDictionary_SkPdfObjectType:
            return fontFromTrueTypeFontDictionary(doc, dict->asTrueTypeFontDictionary());

        case kType1FontDictionary_SkPdfObjectType:
            return fontFromType1FontDictionary(doc, dict->asType1FontDictionary());

        case kMultiMasterFontDictionary_SkPdfObjectType:
            return fontFromMultiMasterFontDictionary(doc, dict->asMultiMasterFontDictionary());

        case kType3FontDictionary_SkPdfObjectType:
            return fontFromType3FontDictionary(doc, dict->asType3FontDictionary());

        default:
            // TODO(edisonn): report error?
            return NULL;
    }
}

SkPdfFont* SkPdfFont::fontFromPdfDictionary(SkNativeParsedPDF* doc, SkPdfFontDictionary* dict) {
    if (dict == NULL) {
        return NULL;  // TODO(edisonn): report default one?
    }

    if (!dict->hasData(SkPdfObject::kFont_Data)) {
        dict->setData(fontFromPdfDictionaryOnce(doc, dict), SkPdfObject::kFont_Data);
    }
    return (SkPdfFont*)dict->data(SkPdfObject::kFont_Data);
}



SkPdfType0Font* SkPdfFont::fontFromType0FontDictionary(SkNativeParsedPDF* doc, SkPdfType0FontDictionary* dict) {
    if (dict == NULL) {
        return NULL;  // default one?
    }

    return new SkPdfType0Font(doc, dict);
}

SkPdfType1Font* SkPdfFont:: fontFromType1FontDictionary(SkNativeParsedPDF* doc, SkPdfType1FontDictionary* dict) {
    if (dict == NULL) {
        return NULL;  // default one?
    }

    return new SkPdfType1Font(doc, dict);
}

SkPdfType3Font* SkPdfFont::fontFromType3FontDictionary(SkNativeParsedPDF* doc, SkPdfType3FontDictionary* dict) {
    if (dict == NULL) {
        return NULL;  // default one?
    }



    return new SkPdfType3Font(doc, dict);
}

SkPdfTrueTypeFont* SkPdfFont::fontFromTrueTypeFontDictionary(SkNativeParsedPDF* doc, SkPdfTrueTypeFontDictionary* dict) {
    if (dict == NULL) {
        return NULL;  // default one?
    }

    return new SkPdfTrueTypeFont(doc, dict);
}

SkPdfMultiMasterFont* SkPdfFont::fontFromMultiMasterFontDictionary(SkNativeParsedPDF* doc, SkPdfMultiMasterFontDictionary* dict) {
    if (dict == NULL) {
        return NULL;  // default one?
    }

    return new SkPdfMultiMasterFont(doc, dict);
}

static int skstoi(const SkPdfObject* str) {
    // TODO(edisonn): report err of it is not a (hex) string
    int ret = 0;
    for (unsigned int i = 0 ; i < str->lenstr(); i++) {
        ret = (ret << 8) + ((unsigned char*)str->c_str())[i];
    }
    // TODO(edisonn): character larger than 0x0000ffff not supported right now.
    return ret & 0x0000ffff;
}

#define tokenIsKeyword(token,keyword) (token.fType == kKeyword_TokenType && token.fKeywordLength==sizeof(keyword)-1 && strncmp(token.fKeyword, keyword, sizeof(keyword)-1) == 0)

SkPdfToUnicode::SkPdfToUnicode(SkNativeParsedPDF* parsed, SkPdfStream* stream) : fParsed(parsed) {
    fCMapEncoding = NULL;
    fCMapEncodingFlag = NULL;

    if (stream) {
        // Since font will be cached, the font has to sit in the per doc allocator, not to be
        // freed after the page is done drawing.
        SkPdfNativeTokenizer* tokenizer = fParsed->tokenizerOfStream(stream, parsed->allocator());
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

        while (tokenizer->readToken(&token)) {

            // TODO(edisonn): perf, macro that would make equal first for token.fKeywordLength with sizeof(keyword), instead od strlen, make sure it is keyword, not a char*
            if (tokenIsKeyword(token, "begincodespacerange")) {
                while (tokenizer->readToken(&token) && !tokenIsKeyword(token, "endcodespacerange")) {
//                    tokenizer->PutBack(token);
//                    tokenizer->readToken(&token);
                    // TODO(edisonn): check token type! ignore/report errors.
                    int start = skstoi(token.fObject);
                    tokenizer->readToken(&token);
                    int end = skstoi(token.fObject);
                    for (int i = start; i <= end; i++) {
                        fCMapEncodingFlag[i] |= 1;
                    }
                }
            }

            if (tokenIsKeyword(token, "beginbfchar")) {
                while (tokenizer->readToken(&token) && !tokenIsKeyword(token, "endbfchar")) {
//                    tokenizer->PutBack(token);
//                    tokenizer->readToken(&token);
                    int from = skstoi(token.fObject);
                    tokenizer->readToken(&token);
                    int to = skstoi(token.fObject);

                    fCMapEncodingFlag[from] |= 2;
                    fCMapEncoding[from] = to;
                }
            }

            if (tokenIsKeyword(token, "beginbfrange")) {
                while (tokenizer->readToken(&token) && !tokenIsKeyword(token, "endbfrange")) {
//                    tokenizer->PutBack(token);
//                    tokenizer->readToken(&token);
                    int start = skstoi(token.fObject);
                    tokenizer->readToken(&token);
                    int end = skstoi(token.fObject);


                    tokenizer->readToken(&token); // [ or just an array directly?
//                    tokenizer->PutBack(token);

                    // TODO(edisonn): read spec: any string or only hex string?
                    if (token.fType == kObject_TokenType && token.fObject->isAnyString()) {
//                        tokenizer->readToken(&token);
                        int value = skstoi(token.fObject);

                        for (int i = start; i <= end; i++) {
                            fCMapEncodingFlag[i] |= 2;
                            fCMapEncoding[i] = value;
                            value++;
                            // if i != end, verify last byte id not if, ignore/report error
                        }

                        // read one string
                    } else if (token.fType == kObject_TokenType && token.fObject->isArray()) {
//                        tokenizer->readToken(&token);
                        for (unsigned int i = 0; i < token.fObject->size(); i++) {
                            fCMapEncodingFlag[start + i] |= 2;
                            fCMapEncoding[start + i] = skstoi((*token.fObject)[i]);
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


SkPdfType0Font::SkPdfType0Font(SkNativeParsedPDF* doc, SkPdfType0FontDictionary* dict) {
    fBaseFont = fontFromName(doc, dict, dict->BaseFont(doc).c_str());
    fEncoding = NULL;

    if (dict->has_Encoding()) {
        if (dict->isEncodingAName(doc)) {
            fEncoding = SkPdfEncoding::fromName(dict->getEncodingAsName(doc).c_str());
        } else if (dict->isEncodingAStream(doc)) {
            //fEncoding = loadEncodingFromStream(dict->getEncodingAsStream());
        } else {
            // TODO(edisonn): error ... warning .. assert?
        }
    }

    if (dict->has_ToUnicode()) {
        fToUnicode = new SkPdfToUnicode(doc, dict->ToUnicode(doc));
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
