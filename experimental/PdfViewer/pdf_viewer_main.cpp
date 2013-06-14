/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkTypeface.h"
#include "SkTArray.h"
#include "picture_utils.h"

#include <iostream>
#include <cstdio>
#include <stack>

#include "podofo.h"
using namespace PoDoFo;

bool LongFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        long* data);

bool DoubleFromDictionary(const PdfMemDocument* pdfDoc,
                          const PdfDictionary& dict,
                          const char* key,
                          const char* abr,
                          double* data);

bool BoolFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        bool* data);

bool NameFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        std::string* data);

bool StringFromDictionary(const PdfMemDocument* pdfDoc,
                          const PdfDictionary& dict,
                          const char* key,
                          const char* abr,
                          std::string* data);

class SkPdfDictionary;
bool DictionaryFromDictionary(const PdfMemDocument* pdfDoc,
                              const PdfDictionary& dict,
                              const char* key,
                              const char* abr,
                              SkPdfDictionary** data);

class SkPdfObject;
bool ObjectFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkPdfObject** data);


#include "pdf_auto_gen.h"

/*
 * TODO(edisonn): ASAP so skp -> pdf -> png looks greap
 * - load gs/ especially smask and already known prop
 * - use transparency (I think ca and CA ops)
 * - load font for baidu.pdf
 * - load font for youtube.pdf
*/

//#define PDF_TRACE
//#define PDF_TRACE_DIFF_IN_PNG
//#define PDF_DEBUG_NO_CLIPING
//#define PDF_DEBUG_NO_PAGE_CLIPING
//#define PDF_DEBUG_3X

// TODO(edisonn): move in trace util.
#ifdef PDF_TRACE
static void SkTraceMatrix(const SkMatrix& matrix, const char* sz = "") {
    printf("SkMatrix %s ", sz);
    for (int i = 0 ; i < 9 ; i++) {
        printf("%f ", SkScalarToDouble(matrix.get(i)));
    }
    printf("\n");
}
#else
#define SkTraceMatrix(a,b)
#endif

using namespace std;
using namespace PoDoFo;

// Utilities
static void setup_bitmap(SkBitmap* bitmap, int width, int height, SkColor color = SK_ColorWHITE) {
    bitmap->setConfig(SkBitmap::kARGB_8888_Config, width, height);

    bitmap->allocPixels();
    bitmap->eraseColor(color);
}

// TODO(edisonn): synonyms? DeviceRGB and RGB ...
int GetColorSpaceComponents(const std::string& colorSpace) {
    if (colorSpace == "DeviceCMYK") {
        return 4;
    } else if (colorSpace == "DeviceGray" ||
            colorSpace == "CalGray" ||
            colorSpace == "Indexed") {
        return 1;
    } else if (colorSpace == "DeviceRGB" ||
            colorSpace == "CalRGB" ||
            colorSpace == "Lab") {
        return 3;
    } else {
        return 0;
    }
}

const PdfObject* resolveReferenceObject(const PdfMemDocument* pdfDoc,
                                  const PdfObject* obj,
                                  bool resolveOneElementArrays = false) {
    while (obj && (obj->IsReference() || (resolveOneElementArrays &&
                                          obj->IsArray() &&
                                          obj->GetArray().GetSize() == 1))) {
        if (obj->IsReference()) {
            // We need to force the non const, the only update we will do is for recurssion checks.
            PdfReference& ref = (PdfReference&)obj->GetReference();
            obj = pdfDoc->GetObjects().GetObject(ref);
        } else {
            obj = &obj->GetArray()[0];
        }
    }

    return obj;
}

static SkMatrix SkMatrixFromPdfMatrix(double array[6]) {
    SkMatrix matrix;
    matrix.setAll(SkDoubleToScalar(array[0]),
                  SkDoubleToScalar(array[2]),
                  SkDoubleToScalar(array[4]),
                  SkDoubleToScalar(array[1]),
                  SkDoubleToScalar(array[3]),
                  SkDoubleToScalar(array[5]),
                  SkDoubleToScalar(0),
                  SkDoubleToScalar(0),
                  SkDoubleToScalar(1));

    return matrix;
}

// TODO(edisonn): better class design.
struct PdfColorOperator {
    std::string fColorSpace;  // TODO(edisonn): use SkString
    SkColor fColor;
    // TODO(edisonn): add here other color space options.

    void setRGBColor(SkColor color) {
        // TODO(edisonn): ASSERT DeviceRGB is the color space.
        fColor = color;
    }
    // TODO(edisonn): double check the default values for all fields.
    PdfColorOperator() : fColor(SK_ColorBLACK) {}
};

// TODO(edisonn): better class design.
struct PdfGraphicsState {
    SkMatrix            fMatrix;
    SkMatrix            fMatrixTm;
    SkMatrix            fMatrixTlm;

    double              fCurPosX;
    double              fCurPosY;

    double              fCurFontSize;
    bool                fTextBlock;
    PdfFont*            fCurFont;
    SkPath              fPath;
    bool                fPathClosed;

    // Clip that is applied after the drawing is done!!!
    bool                fHasClipPathToApply;
    SkPath              fClipPath;

    PdfColorOperator    fStroking;
    PdfColorOperator    fNonStroking;

    double              fLineWidth;
    double              fTextLeading;
    double              fWordSpace;
    double              fCharSpace;

    const PdfObject*    fObjectWithResources;

    SkBitmap            fSMask;

    PdfGraphicsState() {
        fCurPosX      = 0.0;
        fCurPosY      = 0.0;
        fCurFontSize  = 0.0;
        fTextBlock    = false;
        fCurFont      = NULL;
        fMatrix       = SkMatrix::I();
        fMatrixTm     = SkMatrix::I();
        fMatrixTlm    = SkMatrix::I();
        fPathClosed   = true;
        fLineWidth    = 0;
        fTextLeading  = 0;
        fWordSpace    = 0;
        fCharSpace    = 0;
        fObjectWithResources = NULL;
        fHasClipPathToApply = false;
    }
};

// TODO(edisonn): better class design.
struct PdfInlineImage {
    std::map<std::string, std::string> fKeyValuePairs;
    std::string fImageData;

};

// TODO(edisonn): better class design.
struct PdfContext {
    std::stack<PdfVariant>          fVarStack;
    std::stack<PdfGraphicsState>    fStateStack;
    PdfGraphicsState                fGraphicsState;
    PoDoFo::PdfPage*                fPdfPage;
    PdfMemDocument*                 fPdfDoc;
    SkMatrix                        fOriginalMatrix;

    PdfInlineImage                  fInlineImage;

    PdfContext() :  fPdfPage(NULL),
                    fPdfDoc(NULL) {}

};

//  TODO(edisonn): temporary code, to report how much of the PDF we actually think we rendered.
enum PdfResult {
    kOK_PdfResult,
    kPartial_PdfResult,
    kNYI_PdfResult,
    kIgnoreError_PdfResult,
    kError_PdfResult,
    kUnsupported_PdfResult,

    kCount_PdfResult
};

struct PdfToken {
    const char*      pszToken;
    PdfVariant       var;
    EPdfContentsType eType;

    PdfToken() : pszToken(NULL) {}
};

PdfContext* gPdfContext = NULL;
SkBitmap* gDumpBitmap = NULL;
SkCanvas* gDumpCanvas = NULL;
char gLastKeyword[100] = "";
int gLastOpKeyword = -1;
char allOpWithVisualEffects[100] = ",S,s,f,F,f*,B,B*,b,b*,n,Tj,TJ,\',\",d0,d1,sh,EI,Do,EX";
int gReadOp = 0;



bool hasVisualEffect(const char* pdfOp) {
    return true;
    if (*pdfOp == '\0') return false;

    char markedPdfOp[100] = ",";
    strcat(markedPdfOp, pdfOp);
    strcat(markedPdfOp, ",");

    return (strstr(allOpWithVisualEffects, markedPdfOp) != NULL);
}

// TODO(edisonn): Pass PdfContext and SkCanvasd only with the define for instrumentation.
static bool readToken(PdfContentsTokenizer* fTokenizer, PdfToken* token) {
    bool ret = fTokenizer->ReadNext(token->eType, token->pszToken, token->var);

    gReadOp++;

#ifdef PDF_TRACE_DIFF_IN_PNG
    // TODO(edisonn): compare with old bitmap, and save only new bits are available, and save
    // the numbar and name of last operation, so the file name will reflect op that changed.
    if (hasVisualEffect(gLastKeyword)) {  // TODO(edisonn): and has dirty bits.
        gDumpCanvas->flush();

        SkBitmap bitmap;
        setup_bitmap(&bitmap, gDumpBitmap->width(), gDumpBitmap->height());

        memcpy(bitmap.getPixels(), gDumpBitmap->getPixels(), gDumpBitmap->getSize());

        SkAutoTUnref<SkDevice> device(SkNEW_ARGS(SkDevice, (bitmap)));
        SkCanvas canvas(device);

        // draw context stuff here
        SkPaint blueBorder;
        blueBorder.setColor(SK_ColorBLUE);
        blueBorder.setStyle(SkPaint::kStroke_Style);
        blueBorder.setTextSize(SkDoubleToScalar(20));

        SkString str;

        const SkClipStack* clipStack = gDumpCanvas->getClipStack();
        if (clipStack) {
            SkClipStack::Iter iter(*clipStack, SkClipStack::Iter::kBottom_IterStart);
            const SkClipStack::Element* elem;
            double y = 0;
            int total = 0;
            while (elem = iter.next()) {
                total++;
                y += 30;

                switch (elem->getType()) {
                    case SkClipStack::Element::kRect_Type:
                        canvas.drawRect(elem->getRect(), blueBorder);
                        canvas.drawText("Rect Clip", strlen("Rect Clip"), SkDoubleToScalar(10), SkDoubleToScalar(y), blueBorder);
                        break;
                    case SkClipStack::Element::kPath_Type:
                        canvas.drawPath(elem->getPath(), blueBorder);
                        canvas.drawText("Path Clip", strlen("Path Clip"), SkDoubleToScalar(10), SkDoubleToScalar(y), blueBorder);
                        break;
                    case SkClipStack::Element::kEmpty_Type:
                        canvas.drawText("Empty Clip!!!", strlen("Empty Clip!!!"), SkDoubleToScalar(10), SkDoubleToScalar(y), blueBorder);
                        break;
                    default:
                        canvas.drawText("Unkown Clip!!!", strlen("Unkown Clip!!!"), SkDoubleToScalar(10), SkDoubleToScalar(y), blueBorder);
                        break;
                }
            }

            y += 30;
            str.printf("Number of clips in stack: %i", total);
            canvas.drawText(str.c_str(), str.size(), SkDoubleToScalar(10), SkDoubleToScalar(y), blueBorder);
        }

        const SkRegion& clipRegion = gDumpCanvas->getTotalClip();
        SkPath clipPath;
        if (clipRegion.getBoundaryPath(&clipPath)) {
            SkPaint redBorder;
            redBorder.setColor(SK_ColorRED);
            redBorder.setStyle(SkPaint::kStroke_Style);
            canvas.drawPath(clipPath, redBorder);
        }

        canvas.flush();

        SkString out;

        // TODO(edisonn): get the image, and overlay on top of it, the clip , grafic state, teh stack,
        // ... and other properties, to be able to debug th code easily

        out.appendf("/usr/local/google/home/edisonn/log_view2/step-%i-%s.png", gLastOpKeyword, gLastKeyword);
        SkImageEncoder::EncodeFile(out.c_str(), bitmap, SkImageEncoder::kPNG_Type, 100);
    }

    if (token->eType == ePdfContentsType_Keyword) {
        strcpy(gLastKeyword, token->pszToken);
        gLastOpKeyword = gReadOp;
    } else {
        strcpy(gLastKeyword, "");
    }
#endif

    return ret;
}

// TODO(edisonn): Document PdfTokenLooper and subclasses.
class PdfTokenLooper {
protected:
    PdfTokenLooper* fParent;
    PdfContentsTokenizer* fTokenizer;
    PdfContext* fPdfContext;
    SkCanvas* fCanvas;

public:
    PdfTokenLooper(PdfTokenLooper* parent,
                   PdfContentsTokenizer* tokenizer,
                   PdfContext* pdfContext,
                   SkCanvas* canvas)
        : fParent(parent), fTokenizer(tokenizer), fPdfContext(pdfContext), fCanvas(canvas) {}

    virtual PdfResult consumeToken(PdfToken& token) = 0;
    virtual void loop() = 0;

    void setUp(PdfTokenLooper* parent) {
        fParent = parent;
        fTokenizer = parent->fTokenizer;
        fPdfContext = parent->fPdfContext;
        fCanvas = parent->fCanvas;
    }
};

class PdfMainLooper : public PdfTokenLooper {
public:
    PdfMainLooper(PdfTokenLooper* parent,
                  PdfContentsTokenizer* tokenizer,
                  PdfContext* pdfContext,
                  SkCanvas* canvas)
        : PdfTokenLooper(parent, tokenizer, pdfContext, canvas) {}

    virtual PdfResult consumeToken(PdfToken& token);
    virtual void loop();
};

class PdfInlineImageLooper : public PdfTokenLooper {
public:
    PdfInlineImageLooper()
        : PdfTokenLooper(NULL, NULL, NULL, NULL) {}

    virtual PdfResult consumeToken(PdfToken& token);
    virtual void loop();
    PdfResult done();
};

class PdfCompatibilitySectionLooper : public PdfTokenLooper {
public:
    PdfCompatibilitySectionLooper()
        : PdfTokenLooper(NULL, NULL, NULL, NULL) {}

    virtual PdfResult consumeToken(PdfToken& token);
    virtual void loop();
};

typedef PdfResult (*PdfOperatorRenderer)(PdfContext*, SkCanvas*, PdfTokenLooper**);

map<std::string, PdfOperatorRenderer> gPdfOps;

map<std::string, int> gRenderStats[kCount_PdfResult];

char* gRenderStatsNames[kCount_PdfResult] = {
    "Success",
    "Partially implemented",
    "Not yet implemented",
    "Ignore Error",
    "Error",
    "Unsupported/Unknown"
};

struct SkPdfStandardFont {
    const char* fName;
    bool fIsBold;
    bool fIsItalic;
};

static map<std::string, SkPdfStandardFont>& getStandardFonts() {
    static std::map<std::string, SkPdfStandardFont> gPdfStandardFonts;

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
        gPdfStandardFonts["Times-Bold"] = {"Times", true, false};
        gPdfStandardFonts["Times-BoldItalic"] = {"Times", true, true};
        gPdfStandardFonts["Times-Italic"] = {"Times", false, true};
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
    }

    return gPdfStandardFonts;
}

static SkTypeface* SkTypefaceFromPdfStandardFont(const char* fontName, bool bold, bool italic) {
    map<std::string, SkPdfStandardFont>& standardFontMap = getStandardFonts();

    if (standardFontMap.find(fontName) != standardFontMap.end()) {
        SkPdfStandardFont fontData = standardFontMap[fontName];

        // TODO(edisonn): How does the bold/italic specified in standard definition combines with
        // the one in /font key? use OR for now.
        bold = bold || fontData.fIsBold;
        italic = italic || fontData.fIsItalic;

        SkTypeface* typeface = SkTypeface::CreateFromName(
            fontData.fName,
            SkTypeface::Style((bold ? SkTypeface::kBold : 0) |
                              (italic ? SkTypeface::kItalic : 0)));
        if (typeface) {
            typeface->ref();
        }
        return typeface;
    }
    return NULL;
}

static SkTypeface* SkTypefaceFromPdfFont(PdfFont* font) {
    PdfObject* fontObject = font->GetObject();

    PdfObject* pBaseFont = NULL;
    // TODO(edisonn): warning, PoDoFo has a bug in PdfFont constructor, does not call InitVars()
    // for now fixed locally.
    pBaseFont = fontObject->GetIndirectKey( "BaseFont" );
    const char* pszBaseFontName = pBaseFont->GetName().GetName().c_str();

#ifdef PDF_TRACE
    std::string str;
    fontObject->ToString(str);
    printf("Base Font Name: %s\n", pszBaseFontName);
    printf("Font Object Data: %s\n", str.c_str());
#endif

    SkTypeface* typeface = SkTypefaceFromPdfStandardFont(pszBaseFontName, font->IsBold(), font->IsItalic());

    if (typeface != NULL) {
        return typeface;
    }

    char name[1000];
    // HACK
    strncpy(name, pszBaseFontName, 1000);
    char* comma = strstr(name, ",");
    char* dash = strstr(name, "-");
    if (comma) *comma = '\0';
    if (dash) *dash = '\0';

    typeface = SkTypeface::CreateFromName(
                name,
                SkTypeface::Style((font->IsBold() ? SkTypeface::kBold : 0) |
                                  (font->IsItalic() ? SkTypeface::kItalic : 0)));

    if (typeface != NULL) {
#ifdef PDF_TRACE
    printf("HACKED FONT found %s\n", name);
#endif
        return typeface;
    }

#ifdef PDF_TRACE
    printf("FONT_NOT_FOUND %s\n", pszBaseFontName);
#endif

    // TODO(edisonn): Report Warning, NYI
    return SkTypeface::CreateFromName(
                "Times New Roman",
                SkTypeface::Style((font->IsBold() ? SkTypeface::kBold : 0) |
                                  (font->IsItalic() ? SkTypeface::kItalic : 0)));
}

// TODO(edisonn): move this code in podofo, so we don't have to fix the font.
// This logic needs to be moved in PdfEncodingObjectFactory::CreateEncoding
std::map<PdfFont*, PdfCMapEncoding*> gFontsFixed;
PdfEncoding* FixPdfFont(PdfContext* pdfContext, PdfFont* fCurFont) {
    // TODO(edisonn): and is Identity-H
    if (gFontsFixed.find(fCurFont) == gFontsFixed.end()) {
        if (fCurFont->GetObject()->IsDictionary() && fCurFont->GetObject()->GetDictionary().HasKey(PdfName("ToUnicode"))) {
            PdfCMapEncoding* enc = new PdfCMapEncoding(
                    fCurFont->GetObject(),
                    (PdfObject*)resolveReferenceObject(pdfContext->fPdfDoc,
                                           fCurFont->GetObject()->GetDictionary().GetKey(PdfName("ToUnicode"))),
                    PdfCMapEncoding::eBaseEncoding_Identity);  // todo, read the base encoding
            gFontsFixed[fCurFont] = enc;
            return enc;
        }

        return NULL;
    }

    return gFontsFixed[fCurFont];
}

PdfResult DrawText(PdfContext* pdfContext,
                   PdfFont* fCurFont,
                   const PdfString& rString,
                   SkCanvas* canvas)
{
    if (!fCurFont)
    {
        // TODO(edisonn): ignore the error, use the default font?
        return kError_PdfResult;
    }

    const PdfEncoding* enc = FixPdfFont(pdfContext, fCurFont);
    bool cMapUnicodeFont = enc != NULL;
    if (!enc) enc = fCurFont->GetEncoding();
    if (!enc)
    {
        // TODO(edisonn): Can we recover from this error?
        return kError_PdfResult;
    }

    PdfString r2 = rString;
    PdfString unicode;

    if (cMapUnicodeFont) {
        r2 = PdfString((pdf_utf16be*)rString.GetString(), rString.GetLength() / 2);
    }

    unicode = enc->ConvertToUnicode( r2, fCurFont );

#ifdef PDF_TRACE
    printf("%i %i ? %c rString.len = %i\n", (int)rString.GetString()[0], (int)rString.GetString()[1], (int)rString.GetString()[1], rString.GetLength());
    printf("%i %i %i %i %c unicode.len = %i\n", (int)unicode.GetString()[0], (int)unicode.GetString()[1], (int)unicode.GetString()[2], (int)unicode.GetString()[3], (int)unicode.GetString()[0], unicode.GetLength());
#endif

    SkPaint paint;
    // TODO(edisonn): when should fCurFont->GetFontSize() used? When cur is fCurFontSize == 0?
    // Or maybe just not call setTextSize at all?
    if (pdfContext->fGraphicsState.fCurFontSize != 0) {
        paint.setTextSize(SkDoubleToScalar(pdfContext->fGraphicsState.fCurFontSize));
    }
    if (fCurFont->GetFontScale() != 0) {
        paint.setTextScaleX(SkFloatToScalar(fCurFont->GetFontScale() / 100.0));
    }
    paint.setColor(pdfContext->fGraphicsState.fNonStroking.fColor);

    paint.setTypeface(SkTypefaceFromPdfFont(fCurFont));

    paint.setAntiAlias(true);
    // TODO(edisonn): paint.setStyle(...);

    canvas->save();
    SkMatrix matrix = pdfContext->fGraphicsState.fMatrixTm;

#if 0
    // Reverse now the space, otherwise the text is upside down.
    SkScalar z = SkIntToScalar(0);
    SkScalar one = SkIntToScalar(1);

    SkPoint normalSpace1[4] = {SkPoint::Make(z, z), SkPoint::Make(one, z), SkPoint::Make(one, one), SkPoint::Make(z, one)};
    SkPoint mirrorSpace1[4];
    pdfContext->fGraphicsState.fMatrixTm.mapPoints(mirrorSpace1, normalSpace1, 4);

    SkPoint normalSpace2[4] = {SkPoint::Make(z, z), SkPoint::Make(one, z), SkPoint::Make(one, -one), SkPoint::Make(z, -one)};
    SkPoint mirrorSpace2[4];
    pdfContext->fGraphicsState.fMatrixTm.mapPoints(mirrorSpace2, normalSpace2, 4);

#ifdef PDF_TRACE
    printf("mirror1[0], x = %f y = %f\n", SkScalarToDouble(mirrorSpace1[0].x()), SkScalarToDouble(mirrorSpace1[0].y()));
    printf("mirror1[1], x = %f y = %f\n", SkScalarToDouble(mirrorSpace1[1].x()), SkScalarToDouble(mirrorSpace1[1].y()));
    printf("mirror1[2], x = %f y = %f\n", SkScalarToDouble(mirrorSpace1[2].x()), SkScalarToDouble(mirrorSpace1[2].y()));
    printf("mirror1[3], x = %f y = %f\n", SkScalarToDouble(mirrorSpace1[3].x()), SkScalarToDouble(mirrorSpace1[3].y()));
    printf("mirror2[0], x = %f y = %f\n", SkScalarToDouble(mirrorSpace2[0].x()), SkScalarToDouble(mirrorSpace2[0].y()));
    printf("mirror2[1], x = %f y = %f\n", SkScalarToDouble(mirrorSpace2[1].x()), SkScalarToDouble(mirrorSpace2[1].y()));
    printf("mirror2[2], x = %f y = %f\n", SkScalarToDouble(mirrorSpace2[2].x()), SkScalarToDouble(mirrorSpace2[2].y()));
    printf("mirror2[3], x = %f y = %f\n", SkScalarToDouble(mirrorSpace2[3].x()), SkScalarToDouble(mirrorSpace2[3].y()));
#endif

    SkMatrix mirror;
    SkASSERT(mirror.setPolyToPoly(mirrorSpace1, mirrorSpace2, 4));

    // TODO(edisonn): text positioning wrong right now. Need to get matrix operations right.
    matrix.preConcat(mirror);
    canvas->setMatrix(matrix);
#endif

    SkPoint point1;
    pdfContext->fGraphicsState.fMatrixTm.mapXY(SkIntToScalar(0), SkIntToScalar(0), &point1);

    SkMatrix mirror;
    mirror.setTranslate(0, -point1.y());
    // TODO(edisonn): fix rotated text, and skewed too
    mirror.postScale(SK_Scalar1, -SK_Scalar1);
    // TODO(edisonn): post rotate, skew
    mirror.postTranslate(0, point1.y());

    matrix.postConcat(mirror);

    canvas->setMatrix(matrix);

    SkTraceMatrix(matrix, "mirrored");

#ifdef PDF_TRACE
    SkPoint point;
    pdfContext->fGraphicsState.fMatrixTm.mapXY(SkDoubleToScalar(0), SkDoubleToScalar(0), &point);
    printf("Original SkCanvas resolved coordinates, x = %f y = %f\n", SkScalarToDouble(point.x()), SkScalarToDouble(point.y()));
    matrix.mapXY(SkDoubleToScalar(0), SkDoubleToScalar(0), &point);
    printf("Mirored SkCanvas resolved coordinates, x = %f y = %f\n", SkScalarToDouble(point.x()), SkScalarToDouble(point.y()));
#endif

    // TODO(edisonn): remove this call once we load the font properly
    // The extra * will show that we got at least the text positioning right
    // even if font failed to be loaded
//    canvas->drawText(".", 1, SkDoubleToScalar(-5.0), SkDoubleToScalar(0.0), paint);



    // TODO(edisonn): use character and word spacing .. add utility function
    if (cMapUnicodeFont) {
        paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
        SkScalar textWidth = paint.measureText(unicode.GetString(), unicode.GetLength());
        pdfContext->fGraphicsState.fMatrixTm.preTranslate(textWidth, SkDoubleToScalar(0.0));
        canvas->drawText(unicode.GetString(), unicode.GetLength(), SkDoubleToScalar(0.0), SkDoubleToScalar(0.0), paint);
    }
    else {
        paint.setTextEncoding(SkPaint::kUTF8_TextEncoding);
        SkScalar textWidth = paint.measureText(unicode.GetStringUtf8().c_str(), strlen(unicode.GetStringUtf8().c_str()));
        pdfContext->fGraphicsState.fMatrixTm.preTranslate(textWidth, SkDoubleToScalar(0.0));
        canvas->drawText(unicode.GetStringUtf8().c_str(), strlen(unicode.GetStringUtf8().c_str()), SkDoubleToScalar(0.0), SkDoubleToScalar(0.0), paint);
    }

//    paint.setTextEncoding(SkPaint::kUTF8_TextEncoding);
//    unsigned char ch = *(unicode.GetString() + 3);
//    if ((ch & 0xC0) != 0x80 && ch < 0x80) {
//        printf("x%i", ch);
//        SkScalar textWidth = paint.measureText(&ch, 1);
//        pdfContext->fGraphicsState.fMatrixTm.preTranslate(textWidth, SkDoubleToScalar(0.0));
//        canvas->drawText(&ch, 1, SkDoubleToScalar(0.0), SkDoubleToScalar(0.0), paint);
//    }

    canvas->restore();


    return kPartial_PdfResult;
}

// TODO(edisonn): create header files with declarations!
PdfResult PdfOp_q(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper);
PdfResult PdfOp_Q(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper);
PdfResult PdfOp_Tw(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper);
PdfResult PdfOp_Tc(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper);

// TODO(edisonn): deal with synonyms (/BPC == /BitsPerComponent), here or in GetKey?
// Always pass long form in key, and have a map of long -> short key
bool LongFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        long* data) {
    const PdfObject* value = resolveReferenceObject(pdfDoc,
                                              dict.GetKey(PdfName(key)));

    if (value == NULL || !value->IsNumber()) {
        return false;
    }

    *data = value->GetNumber();
    return true;
}

bool LongFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        long* data) {
    if (LongFromDictionary(pdfDoc, dict, key, data)) return true;
    if (abr == NULL || *abr == '\0') return false;
    return LongFromDictionary(pdfDoc, dict, abr, data);
}

bool DoubleFromDictionary(const PdfMemDocument* pdfDoc,
                          const PdfDictionary& dict,
                          const char* key,
                          double* data) {
    const PdfObject* value = resolveReferenceObject(pdfDoc,
                                              dict.GetKey(PdfName(key)));

    if (value == NULL || !value->IsReal()) {
        return false;
    }

    *data = value->GetReal();
    return true;
}

bool DoubleFromDictionary(const PdfMemDocument* pdfDoc,
                          const PdfDictionary& dict,
                          const char* key,
                          const char* abr,
                          double* data) {
    if (DoubleFromDictionary(pdfDoc, dict, key, data)) return true;
    if (abr == NULL || *abr == '\0') return false;
    return DoubleFromDictionary(pdfDoc, dict, abr, data);
}


bool BoolFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        bool* data) {
    const PdfObject* value = resolveReferenceObject(pdfDoc,
                                              dict.GetKey(PdfName(key)));

    if (value == NULL || !value->IsBool()) {
        return false;
    }

    *data = value->GetBool();
    return true;
}

bool BoolFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        bool* data) {
    if (BoolFromDictionary(pdfDoc, dict, key, data)) return true;
    if (abr == NULL || *abr == '\0') return false;
    return BoolFromDictionary(pdfDoc, dict, abr, data);
}

bool NameFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        std::string* data) {
    const PdfObject* value = resolveReferenceObject(pdfDoc,
                                              dict.GetKey(PdfName(key)),
                                              true);
    if (value == NULL || !value->IsName()) {
        return false;
    }

    *data = value->GetName().GetName();
    return true;
}

bool NameFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        std::string* data) {
    if (NameFromDictionary(pdfDoc, dict, key, data)) return true;
    if (abr == NULL || *abr == '\0') return false;
    return NameFromDictionary(pdfDoc, dict, abr, data);
}

bool StringFromDictionary(const PdfMemDocument* pdfDoc,
                          const PdfDictionary& dict,
                          const char* key,
                          std::string* data) {
    const PdfObject* value = resolveReferenceObject(pdfDoc,
                                              dict.GetKey(PdfName(key)),
                                              true);
    if (value == NULL || (!value->IsString() && !value->IsHexString())) {
        return false;
    }

    *data = value->GetString().GetString();
    return true;
}

bool StringFromDictionary(const PdfMemDocument* pdfDoc,
                          const PdfDictionary& dict,
                          const char* key,
                          const char* abr,
                          std::string* data) {
    if (StringFromDictionary(pdfDoc, dict, key, data)) return true;
    if (abr == NULL || *abr == '\0') return false;
    return StringFromDictionary(pdfDoc, dict, abr, data);
}

bool DictionaryFromDictionary(const PdfMemDocument* pdfDoc,
                              const PdfDictionary& dict,
                              const char* key,
                              SkPdfDictionary** data) {
    const PdfObject* value = resolveReferenceObject(pdfDoc,
                                              dict.GetKey(PdfName(key)),
                                              true);
    if (value == NULL || !value->IsDictionary()) {
        return false;
    }

    return PodofoMapper::mapDictionary(*pdfDoc, *value, (SkPdfObject**)data);
}

bool DictionaryFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkPdfDictionary** data) {
    if (DictionaryFromDictionary(pdfDoc, dict, key, data)) return true;
    if (abr == NULL || *abr == '\0') return false;
    return DictionaryFromDictionary(pdfDoc, dict, abr, data);
}

bool ObjectFromDictionary(const PdfMemDocument* pdfDoc,
                          const PdfDictionary& dict,
                          const char* key,
                          SkPdfObject** data) {
    const PdfObject* value = resolveReferenceObject(pdfDoc,
                                              dict.GetKey(PdfName(key)),
                                              true);
    if (value == NULL) {
        return false;
    }
    return PodofoMapper::mapObject(*pdfDoc, *value, data);
}

bool ObjectFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkPdfObject** data) {
    if (ObjectFromDictionary(pdfDoc, dict, key, data)) return true;
    if (abr == NULL || *abr == '\0') return false;
    return ObjectFromDictionary(pdfDoc, dict, abr, data);
}


// TODO(edisonn): perf!!!

static SkColorTable* getGrayColortable() {
    static SkColorTable* grayColortable = NULL;
    if (grayColortable == NULL) {
        SkPMColor* colors = new SkPMColor[256];
        for (int i = 0 ; i < 256; i++) {
            colors[i] = SkPreMultiplyARGB(255, i, i, i);
        }
        grayColortable = new SkColorTable(colors, 256);
    }
    return grayColortable;
}

SkBitmap transferImageStreamToBitmap(unsigned char* uncompressedStream, pdf_long uncompressedStreamLength,
                                     int width, int height, int bytesPerLine,
                                     int bpc, const std::string& colorSpace,
                                     bool transparencyMask) {
    SkBitmap bitmap;

    int components = GetColorSpaceComponents(colorSpace);
//#define MAX_COMPONENTS 10

    int bitsPerLine = width * components * bpc;
    // TODO(edisonn): assume start of lines are aligned at 32 bits?
    // Is there a faster way to load the uncompressed stream into a bitmap?

    // minimal support for now
    if ((colorSpace == "DeviceRGB" || colorSpace == "RGB") && bpc == 8) {
        SkColor* uncompressedStreamArgb = (SkColor*)malloc(width * height * sizeof(SkColor));

        for (int h = 0 ; h < height; h++) {
            long i = width * (height - 1 - h);
            for (int w = 0 ; w < width; w++) {
                uncompressedStreamArgb[i] = SkColorSetRGB(uncompressedStream[3 * w],
                                                          uncompressedStream[3 * w + 1],
                                                          uncompressedStream[3 * w + 2]);
                i++;
            }
            uncompressedStream += bytesPerLine;
        }

        bitmap.setConfig(SkBitmap::kARGB_8888_Config, width, height);
        bitmap.setPixels(uncompressedStreamArgb);
    }
    else if ((colorSpace == "DeviceGray" || colorSpace == "Gray") && bpc == 8) {
        unsigned char* uncompressedStreamA8 = (unsigned char*)malloc(width * height);

        for (int h = 0 ; h < height; h++) {
            long i = width * (height - 1 - h);
            for (int w = 0 ; w < width; w++) {
                uncompressedStreamA8[i] = transparencyMask ? 255 - uncompressedStream[w] :
                                                             uncompressedStream[w];
                i++;
            }
            uncompressedStream += bytesPerLine;
        }

        bitmap.setConfig(transparencyMask ? SkBitmap::kA8_Config : SkBitmap::kIndex8_Config,
                         width, height);
        bitmap.setPixels(uncompressedStreamA8, transparencyMask ? NULL : getGrayColortable());
    }

    // TODO(edisonn): Report Warning, NYI, or error
    return bitmap;
}

bool transferImageStreamToARGB(unsigned char* uncompressedStream, pdf_long uncompressedStreamLength,
                               int width, int bytesPerLine,
                               int bpc, const std::string& colorSpace,
                               SkColor** uncompressedStreamArgb,
                               pdf_long* uncompressedStreamLengthInBytesArgb) {
    int components = GetColorSpaceComponents(colorSpace);
//#define MAX_COMPONENTS 10

    int bitsPerLine = width * components * bpc;
    // TODO(edisonn): assume start of lines are aligned at 32 bits?
    int height = uncompressedStreamLength / bytesPerLine;

    // minimal support for now
    if ((colorSpace == "DeviceRGB" || colorSpace == "RGB") && bpc == 8) {
        *uncompressedStreamLengthInBytesArgb = width * height * 4;
        *uncompressedStreamArgb = (SkColor*)malloc(*uncompressedStreamLengthInBytesArgb);

        for (int h = 0 ; h < height; h++) {
            long i = width * (height - 1 - h);
            for (int w = 0 ; w < width; w++) {
                (*uncompressedStreamArgb)[i] = SkColorSetRGB(uncompressedStream[3 * w],
                                                             uncompressedStream[3 * w + 1],
                                                             uncompressedStream[3 * w + 2]);
                i++;
            }
            uncompressedStream += bytesPerLine;
        }
        return true;
    }

    if ((colorSpace == "DeviceGray" || colorSpace == "Gray") && bpc == 8) {
        *uncompressedStreamLengthInBytesArgb = width * height * 4;
        *uncompressedStreamArgb = (SkColor*)malloc(*uncompressedStreamLengthInBytesArgb);

        for (int h = 0 ; h < height; h++) {
            long i = width * (height - 1 - h);
            for (int w = 0 ; w < width; w++) {
                (*uncompressedStreamArgb)[i] = SkColorSetRGB(uncompressedStream[w],
                                                             uncompressedStream[w],
                                                             uncompressedStream[w]);
                i++;
            }
            uncompressedStream += bytesPerLine;
        }
        return true;
    }

    return false;
}

// utils

// TODO(edisonn): add cache, or put the bitmap property directly on the PdfObject
// TODO(edisonn): deal with colorSpaces, we could add them to SkBitmap::Config
// TODO(edisonn): preserve A1 format that skia knows, + fast convert from 111, 222, 444 to closest
// skia format, through a table

// this functions returns the image, it does not look at the smask.

SkBitmap getImageFromObject(PdfContext* pdfContext, const SkPdfImageDictionary* image, bool transparencyMask) {
    if (image == NULL || !image->valid()) {
        // TODO(edisonn): report warning to be used in testing.
        return SkBitmap();
    }

    // TODO (edisonn): Fast Jpeg(DCTDecode) draw, or fast PNG(FlateDecode) draw ...
//    PdfObject* value = resolveReferenceObject(pdfContext->fPdfDoc,
//                                              obj.GetDictionary().GetKey(PdfName("Filter")));
//    if (value && value->IsArray() && value->GetArray().GetSize() == 1) {
//        value = resolveReferenceObject(pdfContext->fPdfDoc,
//                                       &value->GetArray()[0]);
//    }
//    if (value && value->IsName() && value->GetName().GetName() == "DCTDecode") {
//        SkStream stream = SkStream::
//        SkImageDecoder::Factory()
//    }

    long bpc = image->BitsPerComponent();
    long width = image->Width();
    long height = image->Height();
    SkPdfObject* colorSpaceDict = image->ColorSpace();
    std::string colorSpace = "DeviceRGB";
    // TODO(edisonn): for multiple type fileds, generate code, like, isName(), isArray(), ...and fields like colorSpace_name(), colorSpace_array()
    // so we do nto go to podofo anywhere in our cpp file
    if (colorSpaceDict && colorSpaceDict->podofo() && colorSpaceDict->podofo()->IsName()) {
        colorSpace = colorSpaceDict->podofo()->GetName().GetName();
    }

/*
    bool imageMask = image->imageMask();

    if (imageMask) {
        if (bpc != 0 && bpc != 1) {
            // TODO(edisonn): report warning to be used in testing.
            return SkBitmap();
        }
        bpc = 1;
    }
*/

    const PdfObject* obj = image->podofo();

    char* uncompressedStream = NULL;
    pdf_long uncompressedStreamLength = 0;

    PdfResult ret = kPartial_PdfResult;
    // TODO(edisonn): get rid of try/catch exceptions! We should not throw on user data!
    try {
        obj->GetStream()->GetFilteredCopy(&uncompressedStream, &uncompressedStreamLength);
    } catch (PdfError& e) {
        // TODO(edisonn): report warning to be used in testing.
        return SkBitmap();
    }

    int bytesPerLine = uncompressedStreamLength / height;
#ifdef PDF_TRACE
    if (uncompressedStreamLength % height != 0) {
        printf("Warning uncompressedStreamLength % height != 0 !!!\n");
    }
#endif

    SkBitmap bitmap = transferImageStreamToBitmap(
            (unsigned char*)uncompressedStream, uncompressedStreamLength,
            width, height, bytesPerLine,
            bpc, colorSpace,
            transparencyMask);

    free(uncompressedStream);

    return bitmap;
}

SkBitmap getSmaskFromObject(PdfContext* pdfContext, const SkPdfImageDictionary* obj) {
    const PdfObject* sMask = resolveReferenceObject(pdfContext->fPdfDoc,
                                              obj->podofo()->GetDictionary().GetKey(PdfName("SMask")));

#ifdef PDF_TRACE
    std::string str;
    if (sMask) {
        sMask->ToString(str);
        printf("/SMask of /Subtype /Image: %s\n", str.c_str());
    }
#endif

    if (sMask) {
        SkPdfImageDictionary skxobjmask(pdfContext->fPdfDoc, sMask);
        return getImageFromObject(pdfContext, &skxobjmask, true);
    }

    // TODO(edisonn): implement GS SMask. Default to empty right now.
    return pdfContext->fGraphicsState.fSMask;
}

PdfResult doXObject_Image(PdfContext* pdfContext, SkCanvas* canvas, const SkPdfImageDictionary* skpdfimage) {
    if (skpdfimage == NULL || !skpdfimage->valid()) {
        return kIgnoreError_PdfResult;
    }

    SkBitmap image = getImageFromObject(pdfContext, skpdfimage, false);
    SkBitmap sMask = getSmaskFromObject(pdfContext, skpdfimage);

    canvas->save();
    canvas->setMatrix(pdfContext->fGraphicsState.fMatrix);
    SkRect dst = SkRect::MakeXYWH(SkDoubleToScalar(0.0), SkDoubleToScalar(0.0), SkDoubleToScalar(1.0), SkDoubleToScalar(1.0));

    if (sMask.empty()) {
        canvas->drawBitmapRect(image, dst, NULL);
    } else {
        canvas->saveLayer(&dst, NULL);
        canvas->drawBitmapRect(image, dst, NULL);
        SkPaint xfer;
        xfer.setXfermodeMode(SkXfermode::kSrcOut_Mode); // SkXfermode::kSdtOut_Mode
        canvas->drawBitmapRect(sMask, dst, &xfer);
        canvas->restore();
    }

    canvas->restore();

    return kPartial_PdfResult;
}

bool SkMatrixFromDictionary(PdfContext* pdfContext,
                            const PdfDictionary& dict,
                            const char* key,
                            SkMatrix* matrix) {
    const PdfObject* value = resolveReferenceObject(pdfContext->fPdfDoc,
                                                    dict.GetKey(PdfName(key)));

    if (value == NULL || !value->IsArray()) {
        return false;
    }

    if (value->GetArray().GetSize() != 6) {
        return false;
    }

    double array[6];
    for (int i = 0; i < 6; i++) {
        const PdfObject* elem = resolveReferenceObject(pdfContext->fPdfDoc, &value->GetArray()[i]);
        if (elem == NULL || (!elem->IsReal() && !elem->IsNumber())) {
            return false;
        }
        array[i] = elem->GetReal();
    }

    *matrix = SkMatrixFromPdfMatrix(array);
    return true;
}

bool SkRectFromDictionary(PdfContext* pdfContext,
                          const PdfDictionary& dict,
                          const char* key,
                          SkRect* rect) {
    const PdfObject* value = resolveReferenceObject(pdfContext->fPdfDoc,
                                                    dict.GetKey(PdfName(key)));

    if (value == NULL || !value->IsArray()) {
        return false;
    }

    if (value->GetArray().GetSize() != 4) {
        return false;
    }

    double array[4];
    for (int i = 0; i < 4; i++) {
        const PdfObject* elem = resolveReferenceObject(pdfContext->fPdfDoc, &value->GetArray()[i]);
        if (elem == NULL || (!elem->IsReal() && !elem->IsNumber())) {
            return false;
        }
        array[i] = elem->GetReal();
    }

    *rect = SkRect::MakeLTRB(SkDoubleToScalar(array[0]),
                             SkDoubleToScalar(array[1]),
                             SkDoubleToScalar(array[2]),
                             SkDoubleToScalar(array[3]));
    return true;
}

PdfResult doXObject_Form(PdfContext* pdfContext, SkCanvas* canvas, const PdfObject& obj) {
    if (!obj.HasStream() || obj.GetStream() == NULL || obj.GetStream()->GetLength() == 0) {
        return kOK_PdfResult;
    }

    PdfOp_q(pdfContext, canvas, NULL);
    canvas->save();

    pdfContext->fGraphicsState.fObjectWithResources = &obj;

    SkTraceMatrix(pdfContext->fGraphicsState.fMatrix, "Current matrix");

    SkMatrix matrix;
    if (SkMatrixFromDictionary(pdfContext, obj.GetDictionary(), "Matrix", &matrix)) {
        pdfContext->fGraphicsState.fMatrix.preConcat(matrix);
        pdfContext->fGraphicsState.fMatrixTm = pdfContext->fGraphicsState.fMatrix;
        pdfContext->fGraphicsState.fMatrixTlm = pdfContext->fGraphicsState.fMatrix;
        // TODO(edisonn) reset matrixTm and matricTlm also?
    }

    SkTraceMatrix(pdfContext->fGraphicsState.fMatrix, "Total matrix");

    canvas->setMatrix(pdfContext->fGraphicsState.fMatrix);

    SkRect bbox;
    if (SkRectFromDictionary(pdfContext, obj.GetDictionary(), "BBox", &bbox)) {
        canvas->clipRect(bbox, SkRegion::kIntersect_Op, true);  // TODO(edisonn): AA from settings.
    }

    // TODO(edisonn): iterate smart on the stream even if it is compressed, tokenize it as we go.
    // For this PdfContentsTokenizer needs to be extended.

    char* uncompressedStream = NULL;
    pdf_long uncompressedStreamLength = 0;

    PdfResult ret = kPartial_PdfResult;

    // TODO(edisonn): get rid of try/catch exceptions! We should not throw on user data!
    try {
        obj.GetStream()->GetFilteredCopy(&uncompressedStream, &uncompressedStreamLength);
        if (uncompressedStream != NULL && uncompressedStreamLength != 0) {
            PdfContentsTokenizer tokenizer(uncompressedStream, uncompressedStreamLength);
            PdfMainLooper looper(NULL, &tokenizer, pdfContext, canvas);
            looper.loop();
        }
        free(uncompressedStream);
    } catch (PdfError& e) {
        ret = kIgnoreError_PdfResult;
    }

    // TODO(edisonn): should we restore the variable stack at the same state?
    // There could be operands left, that could be consumed by a parent tokenizer when we pop.
    canvas->restore();
    PdfOp_Q(pdfContext, canvas, NULL);
    return ret;
}

PdfResult doXObject_PS(PdfContext* pdfContext, SkCanvas* canvas, const PdfObject& obj) {
    return kNYI_PdfResult;
}

// TODO(edisonn): faster, have the property on the PdfObject itself.
std::set<const PdfObject*> gInRendering;

class CheckRecursiveRendering {
    const PdfObject& fObj;
public:
    CheckRecursiveRendering(const PdfObject& obj) : fObj(obj) {
        gInRendering.insert(&obj);
    }

    ~CheckRecursiveRendering() {
        //SkASSERT(fObj.fInRendering);
        gInRendering.erase(&fObj);
    }

    static bool IsInRendering(const PdfObject& obj) {
        return gInRendering.find(&obj) != gInRendering.end();
    }
};

PdfResult doXObject(PdfContext* pdfContext, SkCanvas* canvas, const PdfObject& obj) {
    if (CheckRecursiveRendering::IsInRendering(obj)) {
        // Oops, corrupt PDF!
        return kIgnoreError_PdfResult;
    }

    CheckRecursiveRendering checkRecursion(obj);

    // TODO(edisonn): check type
    SkPdfObject* skobj = NULL;
    if (!PodofoMapper::mapXObjectDictionary(*pdfContext->fPdfDoc, obj, &skobj)) return kIgnoreError_PdfResult;

    if (!skobj || !skobj->valid()) return kIgnoreError_PdfResult;

    PdfResult ret = kIgnoreError_PdfResult;
    switch (skobj->getType())
    {
        case kObjectDictionaryXObjectDictionaryImageDictionary_SkPdfObjectType:
            ret = doXObject_Image(pdfContext, canvas, skobj->asImageDictionary());
            break;
        case kObjectDictionaryXObjectDictionaryType1FormDictionary_SkPdfObjectType:
            ret = doXObject_Form(pdfContext, canvas, obj);//skobj->asType1FormDictionary());
            break;
        //case kObjectDictionaryXObjectPS_SkPdfObjectType:
            //return doXObject_PS(skxobj.asPS());
    }

    delete skobj;
    return ret;
}

PdfResult PdfOp_q(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fStateStack.push(pdfContext->fGraphicsState);
    canvas->save();
    return kOK_PdfResult;
}

PdfResult PdfOp_Q(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fGraphicsState = pdfContext->fStateStack.top();
    pdfContext->fStateStack.pop();
    canvas->restore();
    return kOK_PdfResult;
}

PdfResult PdfOp_cm(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double array[6];
    for (int i = 0 ; i < 6 ; i++) {
        array[5 - i] = pdfContext->fVarStack.top().GetReal();
        pdfContext->fVarStack.pop();
    }

    // a b
    // c d
    // e f

    // 0 1
    // 2 3
    // 4 5

    // sx ky
    // kx sy
    // tx ty
    SkMatrix matrix = SkMatrixFromPdfMatrix(array);

    pdfContext->fGraphicsState.fMatrix.preConcat(matrix);

#ifdef PDF_TRACE
    printf("cm ");
    for (int i = 0 ; i < 6 ; i++) {
        printf("%f ", array[i]);
    }
    printf("\n");
    SkTraceMatrix(pdfContext->fGraphicsState.fMatrix);
#endif

    return kOK_PdfResult;
}

//leading TL Set the text leading, Tl
//, to leading, which is a number expressed in unscaled text
//space units. Text leading is used only by the T*, ', and " operators. Initial value: 0.
PdfResult PdfOp_TL(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double ty = pdfContext->fVarStack.top().GetReal(); pdfContext->fVarStack.pop();

    pdfContext->fGraphicsState.fTextLeading = ty;

    return kOK_PdfResult;
}

PdfResult PdfOp_Td(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double ty = pdfContext->fVarStack.top().GetReal(); pdfContext->fVarStack.pop();
    double tx = pdfContext->fVarStack.top().GetReal(); pdfContext->fVarStack.pop();

    double array[6] = {1, 0, 0, 1, tx, ty};
    SkMatrix matrix = SkMatrixFromPdfMatrix(array);

    pdfContext->fGraphicsState.fMatrixTm.preConcat(matrix);
    pdfContext->fGraphicsState.fMatrixTlm.preConcat(matrix);

    return kPartial_PdfResult;
}

PdfResult PdfOp_TD(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double ty = pdfContext->fVarStack.top().GetReal(); pdfContext->fVarStack.pop();
    double tx = pdfContext->fVarStack.top().GetReal(); pdfContext->fVarStack.pop();

    PdfVariant _ty(-ty);
    pdfContext->fVarStack.push(_ty);
    PdfOp_TL(pdfContext, canvas, looper);

    PdfVariant vtx(tx);
    PdfVariant vty(ty);
    pdfContext->fVarStack.push(vtx);
    pdfContext->fVarStack.push(vty);
    return PdfOp_Td(pdfContext, canvas, looper);
}

PdfResult PdfOp_Tm(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double f = pdfContext->fVarStack.top().GetReal(); pdfContext->fVarStack.pop();
    double e = pdfContext->fVarStack.top().GetReal(); pdfContext->fVarStack.pop();
    double d = pdfContext->fVarStack.top().GetReal(); pdfContext->fVarStack.pop();
    double c = pdfContext->fVarStack.top().GetReal(); pdfContext->fVarStack.pop();
    double b = pdfContext->fVarStack.top().GetReal(); pdfContext->fVarStack.pop();
    double a = pdfContext->fVarStack.top().GetReal(); pdfContext->fVarStack.pop();

    double array[6];
    array[0] = a;
    array[1] = b;
    array[2] = c;
    array[3] = d;
    array[4] = e;
    array[5] = f;

    SkMatrix matrix = SkMatrixFromPdfMatrix(array);
    matrix.postConcat(pdfContext->fGraphicsState.fMatrix);

    // TODO(edisonn): Text positioning.
    pdfContext->fGraphicsState.fMatrixTm = matrix;
    pdfContext->fGraphicsState.fMatrixTlm = matrix;;

    return kPartial_PdfResult;
}

//— T* Move to the start of the next line. This operator has the same effect as the code
//0 Tl Td
//where Tl is the current leading parameter in the text state
PdfResult PdfOp_T_star(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    PdfVariant zero(0.0);
    PdfVariant tl(pdfContext->fGraphicsState.fTextLeading);

    pdfContext->fVarStack.push(zero);
    pdfContext->fVarStack.push(tl);
    return PdfOp_Td(pdfContext, canvas, looper);
}

PdfResult PdfOp_m(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (pdfContext->fGraphicsState.fPathClosed) {
        pdfContext->fGraphicsState.fPath.reset();
        pdfContext->fGraphicsState.fPathClosed = false;
    }

    pdfContext->fGraphicsState.fCurPosY = pdfContext->fVarStack.top().GetReal();    pdfContext->fVarStack.pop();
    pdfContext->fGraphicsState.fCurPosX = pdfContext->fVarStack.top().GetReal();    pdfContext->fVarStack.pop();

    pdfContext->fGraphicsState.fPath.moveTo(SkDoubleToScalar(pdfContext->fGraphicsState.fCurPosX),
                                          SkDoubleToScalar(pdfContext->fGraphicsState.fCurPosY));

    return kOK_PdfResult;
}

PdfResult PdfOp_l(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (pdfContext->fGraphicsState.fPathClosed) {
        pdfContext->fGraphicsState.fPath.reset();
        pdfContext->fGraphicsState.fPathClosed = false;
    }

    pdfContext->fGraphicsState.fCurPosY = pdfContext->fVarStack.top().GetReal();    pdfContext->fVarStack.pop();
    pdfContext->fGraphicsState.fCurPosX = pdfContext->fVarStack.top().GetReal();    pdfContext->fVarStack.pop();

    pdfContext->fGraphicsState.fPath.lineTo(SkDoubleToScalar(pdfContext->fGraphicsState.fCurPosX),
                                          SkDoubleToScalar(pdfContext->fGraphicsState.fCurPosY));

    return kOK_PdfResult;
}

PdfResult PdfOp_c(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (pdfContext->fGraphicsState.fPathClosed) {
        pdfContext->fGraphicsState.fPath.reset();
        pdfContext->fGraphicsState.fPathClosed = false;
    }

    double y3 = pdfContext->fVarStack.top().GetReal();    pdfContext->fVarStack.pop();
    double x3 = pdfContext->fVarStack.top().GetReal();    pdfContext->fVarStack.pop();
    double y2 = pdfContext->fVarStack.top().GetReal();    pdfContext->fVarStack.pop();
    double x2 = pdfContext->fVarStack.top().GetReal();    pdfContext->fVarStack.pop();
    double y1 = pdfContext->fVarStack.top().GetReal();    pdfContext->fVarStack.pop();
    double x1 = pdfContext->fVarStack.top().GetReal();    pdfContext->fVarStack.pop();

    pdfContext->fGraphicsState.fPath.cubicTo(SkDoubleToScalar(x1), SkDoubleToScalar(y1),
                                            SkDoubleToScalar(x2), SkDoubleToScalar(y2),
                                            SkDoubleToScalar(x3), SkDoubleToScalar(y3));

    pdfContext->fGraphicsState.fCurPosX = x3;
    pdfContext->fGraphicsState.fCurPosY = y3;

    return kOK_PdfResult;
}

PdfResult PdfOp_v(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (pdfContext->fGraphicsState.fPathClosed) {
        pdfContext->fGraphicsState.fPath.reset();
        pdfContext->fGraphicsState.fPathClosed = false;
    }

    double y3 = pdfContext->fVarStack.top().GetReal();    pdfContext->fVarStack.pop();
    double x3 = pdfContext->fVarStack.top().GetReal();    pdfContext->fVarStack.pop();
    double y2 = pdfContext->fVarStack.top().GetReal();    pdfContext->fVarStack.pop();
    double x2 = pdfContext->fVarStack.top().GetReal();    pdfContext->fVarStack.pop();
    double y1 = pdfContext->fGraphicsState.fCurPosY;
    double x1 = pdfContext->fGraphicsState.fCurPosX;

    pdfContext->fGraphicsState.fPath.cubicTo(SkDoubleToScalar(x1), SkDoubleToScalar(y1),
                                            SkDoubleToScalar(x2), SkDoubleToScalar(y2),
                                            SkDoubleToScalar(x3), SkDoubleToScalar(y3));

    pdfContext->fGraphicsState.fCurPosX = x3;
    pdfContext->fGraphicsState.fCurPosY = y3;

    return kOK_PdfResult;
}

PdfResult PdfOp_y(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (pdfContext->fGraphicsState.fPathClosed) {
        pdfContext->fGraphicsState.fPath.reset();
        pdfContext->fGraphicsState.fPathClosed = false;
    }

    double y3 = pdfContext->fVarStack.top().GetReal();    pdfContext->fVarStack.pop();
    double x3 = pdfContext->fVarStack.top().GetReal();    pdfContext->fVarStack.pop();
    double y2 = pdfContext->fGraphicsState.fCurPosY;
    double x2 = pdfContext->fGraphicsState.fCurPosX;
    double y1 = pdfContext->fVarStack.top().GetReal();    pdfContext->fVarStack.pop();
    double x1 = pdfContext->fVarStack.top().GetReal();    pdfContext->fVarStack.pop();

    pdfContext->fGraphicsState.fPath.cubicTo(SkDoubleToScalar(x1), SkDoubleToScalar(y1),
                                            SkDoubleToScalar(x2), SkDoubleToScalar(y2),
                                            SkDoubleToScalar(x3), SkDoubleToScalar(y3));

    pdfContext->fGraphicsState.fCurPosX = x3;
    pdfContext->fGraphicsState.fCurPosY = y3;

    return kOK_PdfResult;
}

PdfResult PdfOp_re(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (pdfContext->fGraphicsState.fPathClosed) {
        pdfContext->fGraphicsState.fPath.reset();
        pdfContext->fGraphicsState.fPathClosed = false;
    }

    double height = pdfContext->fVarStack.top().GetReal();      pdfContext->fVarStack.pop();
    double width = pdfContext->fVarStack.top().GetReal();       pdfContext->fVarStack.pop();
    double y = pdfContext->fVarStack.top().GetReal();           pdfContext->fVarStack.pop();
    double x = pdfContext->fVarStack.top().GetReal();           pdfContext->fVarStack.pop();

    pdfContext->fGraphicsState.fPath.addRect(SkDoubleToScalar(x), SkDoubleToScalar(y),
                                           SkDoubleToScalar(x + width), SkDoubleToScalar(y + height));

    pdfContext->fGraphicsState.fCurPosX = x;
    pdfContext->fGraphicsState.fCurPosY = y + height;

    return kOK_PdfResult;
}

PdfResult PdfOp_h(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fGraphicsState.fPath.close();
    pdfContext->fGraphicsState.fPathClosed = true;
    return kOK_PdfResult;
}

PdfResult PdfOp_fillAndStroke(PdfContext* pdfContext, SkCanvas* canvas, bool fill, bool stroke, bool close, bool evenOdd) {
    SkPath path = pdfContext->fGraphicsState.fPath;

    if (close) {
        path.close();
    }

    canvas->setMatrix(pdfContext->fGraphicsState.fMatrix);

    SkPaint paint;

    // TODO(edisonn): get this from pdfContext->options,
    // or pdfContext->addPaintOptions(&paint);
    paint.setAntiAlias(true);

    // TODO(edisonn): dashing, miter, ...

//    path.transform(pdfContext->fGraphicsState.fMatrix);
//    path.transform(pdfContext->fOriginalMatrix);

    SkPoint line[2];
    if (fill && !stroke && path.isLine(line)) {
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(pdfContext->fGraphicsState.fNonStroking.fColor);
        paint.setStrokeWidth(SkDoubleToScalar(0));
        canvas->drawPath(path, paint);
    } else {
        if (fill) {
            paint.setStyle(SkPaint::kFill_Style);
            if (evenOdd) {
                path.setFillType(SkPath::kEvenOdd_FillType);
            }
            paint.setColor(pdfContext->fGraphicsState.fNonStroking.fColor);
            canvas->drawPath(path, paint);
        }

        if (stroke) {
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setColor(pdfContext->fGraphicsState.fStroking.fColor);
            paint.setStrokeWidth(SkDoubleToScalar(pdfContext->fGraphicsState.fLineWidth));
            path.setFillType(SkPath::kWinding_FillType);  // reset it, just in case it messes up the stroke
            canvas->drawPath(path, paint);
        }
    }

    pdfContext->fGraphicsState.fPath.reset();
    // todo zoom ... other stuff ?

    if (pdfContext->fGraphicsState.fHasClipPathToApply) {
#ifndef PDF_DEBUG_NO_CLIPING
        canvas->clipPath(pdfContext->fGraphicsState.fClipPath, SkRegion::kIntersect_Op, true);
#endif
    }

    //pdfContext->fGraphicsState.fClipPath.reset();
    pdfContext->fGraphicsState.fHasClipPathToApply = false;

    return kPartial_PdfResult;

}

PdfResult PdfOp_S(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_fillAndStroke(pdfContext, canvas, false, true, false, false);
}

PdfResult PdfOp_s(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_fillAndStroke(pdfContext, canvas, false, true, true, false);
}

PdfResult PdfOp_F(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_fillAndStroke(pdfContext, canvas, true, false, false, false);
}

PdfResult PdfOp_f(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_fillAndStroke(pdfContext, canvas, true, false, false, false);
}

PdfResult PdfOp_f_star(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_fillAndStroke(pdfContext, canvas, true, false, false, true);
}

PdfResult PdfOp_B(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_fillAndStroke(pdfContext, canvas, true, true, false, false);
}

PdfResult PdfOp_B_star(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_fillAndStroke(pdfContext, canvas, true, true, false, true);
}

PdfResult PdfOp_b(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_fillAndStroke(pdfContext, canvas, true, true, true, false);
}

PdfResult PdfOp_b_star(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_fillAndStroke(pdfContext, canvas, true, true, true, true);
}

PdfResult PdfOp_n(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    canvas->setMatrix(pdfContext->fGraphicsState.fMatrix);
    if (pdfContext->fGraphicsState.fHasClipPathToApply) {
#ifndef PDF_DEBUG_NO_CLIPING
        canvas->clipPath(pdfContext->fGraphicsState.fClipPath, SkRegion::kIntersect_Op, true);
#endif
    }

    //pdfContext->fGraphicsState.fClipPath.reset();
    pdfContext->fGraphicsState.fHasClipPathToApply = false;

    pdfContext->fGraphicsState.fPathClosed = true;

    return kOK_PdfResult;
}

PdfResult PdfOp_BT(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fGraphicsState.fTextBlock   = true;
    pdfContext->fGraphicsState.fMatrixTm = pdfContext->fGraphicsState.fMatrix;
    pdfContext->fGraphicsState.fMatrixTlm = pdfContext->fGraphicsState.fMatrix;

    return kPartial_PdfResult;
}

PdfResult PdfOp_ET(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (!pdfContext->fGraphicsState.fTextBlock) {
        return kIgnoreError_PdfResult;
    }
    // TODO(edisonn): anything else to be done once we are done with draw text? Like restore stack?
    return kPartial_PdfResult;
}

//font size Tf Set the text font, Tf
//, to font and the text font size, Tfs, to size. font is the name of a
//font resource in the Fontsubdictionary of the current resource dictionary; size is
//a number representing a scale factor. There is no initial value for either font or
//size; they must be speciﬁed explicitly using Tf before any text is shown.
PdfResult PdfOp_Tf(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fGraphicsState.fCurFontSize = pdfContext->fVarStack.top().GetReal();     pdfContext->fVarStack.pop();
    PdfName fontName = pdfContext->fVarStack.top().GetName();                           pdfContext->fVarStack.pop();

    // TODO(edisonn): Load font from pdfContext->fGraphicsState.fObjectWithResources ?
    PdfObject* pFont = pdfContext->fPdfPage->GetFromResources( PdfName("Font"), fontName );
    if( !pFont )
    {
        // TODO(edisonn): try to ignore the error, make sure we do not crash.
        return kIgnoreError_PdfResult;
    }

    pdfContext->fGraphicsState.fCurFont = pdfContext->fPdfDoc->GetFont( pFont );
    if( !pdfContext->fGraphicsState.fCurFont )
    {
        // TODO(edisonn): check ~/crasing, for one of the files PoDoFo throws exception
        // when calling pFont->Reference(), with Linked list corruption.
        return kIgnoreError_PdfResult;
    }

    return kPartial_PdfResult;
}

PdfResult PdfOp_Tj(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (!pdfContext->fGraphicsState.fTextBlock) {
        // TODO(edisonn): try to recover and draw it any way?
        return kIgnoreError_PdfResult;
    }

    PdfResult ret = DrawText(pdfContext,
                             pdfContext->fGraphicsState.fCurFont,
                             pdfContext->fVarStack.top().GetString(),
                             canvas);
    pdfContext->fVarStack.pop();

    return ret;
}

PdfResult PdfOp_quote(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (!pdfContext->fGraphicsState.fTextBlock) {
        // TODO(edisonn): try to recover and draw it any way?
        return kIgnoreError_PdfResult;
    }

    PdfOp_T_star(pdfContext, canvas, looper);
    // Do not pop, and push, just transfer the param to Tj
    return PdfOp_Tj(pdfContext, canvas, looper);
}

PdfResult PdfOp_doublequote(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (!pdfContext->fGraphicsState.fTextBlock) {
        // TODO(edisonn): try to recover and draw it any way?
        return kIgnoreError_PdfResult;
    }

    PdfVariant str = pdfContext->fVarStack.top();       pdfContext->fVarStack.pop();
    PdfVariant ac = pdfContext->fVarStack.top();        pdfContext->fVarStack.pop();
    PdfVariant aw = pdfContext->fVarStack.top();        pdfContext->fVarStack.pop();

    pdfContext->fVarStack.push(aw);
    PdfOp_Tw(pdfContext, canvas, looper);

    pdfContext->fVarStack.push(ac);
    PdfOp_Tc(pdfContext, canvas, looper);

    pdfContext->fVarStack.push(str);
    PdfOp_quote(pdfContext, canvas, looper);

    return kPartial_PdfResult;
}

PdfResult PdfOp_TJ(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (!pdfContext->fGraphicsState.fTextBlock) {
        // TODO(edisonn): try to recover and draw it any way?
        return kIgnoreError_PdfResult;
    }

    PdfArray array = pdfContext->fVarStack.top().GetArray();
    pdfContext->fVarStack.pop();

    for( int i=0; i<static_cast<int>(array.GetSize()); i++ )
    {
        if( array[i].IsString() || array[i].IsHexString() ) {
            DrawText(pdfContext,
                           pdfContext->fGraphicsState.fCurFont,
                           array[i].GetString(),
                           canvas);
        } else if (array[i].IsReal() || array[i].IsNumber()) {
            double dx = array[i].GetReal();
            SkMatrix matrix;
            matrix.setAll(SkDoubleToScalar(1),
                          SkDoubleToScalar(0),
                          // TODO(edisonn): use writing mode, vertical/horizontal.
                          SkDoubleToScalar(-dx),  // amount is substracted!!!
                          SkDoubleToScalar(0),
                          SkDoubleToScalar(1),
                          SkDoubleToScalar(0),
                          SkDoubleToScalar(0),
                          SkDoubleToScalar(0),
                          SkDoubleToScalar(1));

            pdfContext->fGraphicsState.fMatrixTm.preConcat(matrix);
        }
    }
    return kPartial_PdfResult;  // TODO(edisonn): Implement fully DrawText before returing OK.
}

PdfResult PdfOp_CS_cs(PdfContext* pdfContext, SkCanvas* canvas, PdfColorOperator* colorOperator) {
    colorOperator->fColorSpace = pdfContext->fVarStack.top().GetName().GetName();    pdfContext->fVarStack.pop();
    return kOK_PdfResult;
}

PdfResult PdfOp_CS(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_CS_cs(pdfContext, canvas, &pdfContext->fGraphicsState.fStroking);
}

PdfResult PdfOp_cs(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_CS_cs(pdfContext, canvas, &pdfContext->fGraphicsState.fNonStroking);
}

PdfResult PdfOp_SC_sc(PdfContext* pdfContext, SkCanvas* canvas, PdfColorOperator* colorOperator) {
    double c[4];
    pdf_int64 v[4];

    int n = GetColorSpaceComponents(colorOperator->fColorSpace);

    bool doubles = true;
    if (colorOperator->fColorSpace == "Indexed") {
        doubles = false;
    }

#ifdef PDF_TRACE
    printf("color space = %s, N = %i\n", colorOperator->fColorSpace.c_str(), n);
#endif

    for (int i = n - 1; i >= 0 ; i--) {
        if (doubles) {
            c[i] = pdfContext->fVarStack.top().GetReal();     pdfContext->fVarStack.pop();
        } else {
            v[i] = pdfContext->fVarStack.top().GetNumber();   pdfContext->fVarStack.pop();
        }
    }

    // TODO(edisonn): Now, set that color. Only DeviceRGB supported.
    if (colorOperator->fColorSpace == "DeviceRGB") {
        colorOperator->setRGBColor(SkColorSetRGB(255*c[0], 255*c[1], 255*c[2]));
    }
    return kPartial_PdfResult;
}

PdfResult PdfOp_SC(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_SC_sc(pdfContext, canvas, &pdfContext->fGraphicsState.fStroking);
}

PdfResult PdfOp_sc(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_SC_sc(pdfContext, canvas, &pdfContext->fGraphicsState.fNonStroking);
}

PdfResult PdfOp_SCN_scn(PdfContext* pdfContext, SkCanvas* canvas, PdfColorOperator* colorOperator) {
    PdfString name;

    if (pdfContext->fVarStack.top().IsName()) {
        pdfContext->fVarStack.pop();
    }

    // TODO(edisonn): SCN supports more color spaces than SCN. Read and implement spec.
    PdfOp_SC_sc(pdfContext, canvas, colorOperator);

    return kPartial_PdfResult;
}

PdfResult PdfOp_SCN(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_SCN_scn(pdfContext, canvas, &pdfContext->fGraphicsState.fStroking);
}

PdfResult PdfOp_scn(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_SCN_scn(pdfContext, canvas, &pdfContext->fGraphicsState.fNonStroking);
}

PdfResult PdfOp_G_g(PdfContext* pdfContext, SkCanvas* canvas, PdfColorOperator* colorOperator) {
    double gray = pdfContext->fVarStack.top().GetReal();     pdfContext->fVarStack.pop();
    return kNYI_PdfResult;
}

PdfResult PdfOp_G(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_G_g(pdfContext, canvas, &pdfContext->fGraphicsState.fStroking);
}

PdfResult PdfOp_g(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_G_g(pdfContext, canvas, &pdfContext->fGraphicsState.fNonStroking);
}

PdfResult PdfOp_RG_rg(PdfContext* pdfContext, SkCanvas* canvas, PdfColorOperator* colorOperator) {
    double b = pdfContext->fVarStack.top().GetReal();     pdfContext->fVarStack.pop();
    double g = pdfContext->fVarStack.top().GetReal();     pdfContext->fVarStack.pop();
    double r = pdfContext->fVarStack.top().GetReal();     pdfContext->fVarStack.pop();

    colorOperator->fColorSpace = "DeviceRGB";
    colorOperator->setRGBColor(SkColorSetRGB(255*r, 255*g, 255*b));
    return kOK_PdfResult;
}

PdfResult PdfOp_RG(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_RG_rg(pdfContext, canvas, &pdfContext->fGraphicsState.fStroking);
}

PdfResult PdfOp_rg(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_RG_rg(pdfContext, canvas, &pdfContext->fGraphicsState.fNonStroking);
}

PdfResult PdfOp_K_k(PdfContext* pdfContext, SkCanvas* canvas, PdfColorOperator* colorOperator) {
    // TODO(edisonn): spec has some rules about overprint, implement them.
    double k = pdfContext->fVarStack.top().GetReal();     pdfContext->fVarStack.pop();
    double y = pdfContext->fVarStack.top().GetReal();     pdfContext->fVarStack.pop();
    double m = pdfContext->fVarStack.top().GetReal();     pdfContext->fVarStack.pop();
    double c = pdfContext->fVarStack.top().GetReal();     pdfContext->fVarStack.pop();

    colorOperator->fColorSpace = "DeviceCMYK";
    // TODO(edisonn): Set color.
    return kNYI_PdfResult;
}

PdfResult PdfOp_K(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_K_k(pdfContext, canvas, &pdfContext->fGraphicsState.fStroking);
}

PdfResult PdfOp_k(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_K_k(pdfContext, canvas, &pdfContext->fGraphicsState.fNonStroking);
}

PdfResult PdfOp_W(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fGraphicsState.fClipPath = pdfContext->fGraphicsState.fPath;
    pdfContext->fGraphicsState.fHasClipPathToApply = true;

    return kOK_PdfResult;
}

PdfResult PdfOp_W_star(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fGraphicsState.fClipPath = pdfContext->fGraphicsState.fPath;

#ifdef PDF_TRACE
    if (pdfContext->fGraphicsState.fClipPath.isRect(NULL)) {
        printf("CLIP IS RECT\n");
    }
#endif

    // TODO(edisonn): there seem to be a bug with clipPath of a rect with even odd.
    pdfContext->fGraphicsState.fClipPath.setFillType(SkPath::kEvenOdd_FillType);
    pdfContext->fGraphicsState.fHasClipPathToApply = true;

    return kPartial_PdfResult;
}

PdfResult PdfOp_BX(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    *looper = new PdfCompatibilitySectionLooper();
    return kOK_PdfResult;
}

PdfResult PdfOp_EX(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
#ifdef ASSERT_BAD_PDF_OPS
    SkASSERT(false);  // EX must be consumed by PdfCompatibilitySectionLooper, but let's
                      // have the assert when testing good pdfs.
#endif
    return kIgnoreError_PdfResult;
}

PdfResult PdfOp_BI(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    *looper = new PdfInlineImageLooper();
    return kOK_PdfResult;
}

PdfResult PdfOp_ID(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
#ifdef ASSERT_BAD_PDF_OPS
    SkASSERT(false);  // must be processed in inline image looper, but let's
                      // have the assert when testing good pdfs.
#endif
    return kIgnoreError_PdfResult;
}

PdfResult PdfOp_EI(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
#ifdef ASSERT_BAD_PDF_OPS
    SkASSERT(false);  // must be processed in inline image looper, but let's
                      // have the assert when testing good pdfs.
#endif
    return kIgnoreError_PdfResult;
}

//lineWidth w Set the line width in the graphics state (see “Line Width” on page 152).
PdfResult PdfOp_w(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double lineWidth = pdfContext->fVarStack.top().GetReal();     pdfContext->fVarStack.pop();
    pdfContext->fGraphicsState.fLineWidth = lineWidth;

    return kOK_PdfResult;
}

//lineCap J Set the line cap style in the graphics state (see “Line Cap Style” on page 153).
PdfResult PdfOp_J(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fVarStack.pop();
    //double lineCap = pdfContext->fVarStack.top().GetReal();     pdfContext->fVarStack.pop();

    return kNYI_PdfResult;
}

//lineJoin j Set the line join style in the graphics state (see “Line Join Style” on page 153).
PdfResult PdfOp_j(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fVarStack.pop();
    //double lineJoin = pdfContext->fVarStack.top().GetReal();     pdfContext->fVarStack.pop();

    return kNYI_PdfResult;
}

//miterLimit M Set the miter limit in the graphics state (see “Miter Limit” on page 153).
PdfResult PdfOp_M(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fVarStack.pop();
    //double miterLimit = pdfContext->fVarStack.top().GetReal();     pdfContext->fVarStack.pop();

    return kNYI_PdfResult;
}

//dashArray dashPhase d Set the line dash pattern in the graphics state (see “Line Dash Pattern” on
//page 155).
PdfResult PdfOp_d(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fVarStack.pop();
    pdfContext->fVarStack.pop();

    return kNYI_PdfResult;
}

//intent ri (PDF 1.1) Set the color rendering intent in the graphics state (see “Rendering Intents” on page 197).
PdfResult PdfOp_ri(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fVarStack.pop();

    return kNYI_PdfResult;
}

//ﬂatness i Set the ﬂatness tolerance in the graphics state (see Section 6.5.1, “Flatness
//Tolerance”). ﬂatness is a number in the range 0 to 100; a value of 0 speci-
//ﬁes the output device’s default ﬂatness tolerance.
PdfResult PdfOp_i(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fVarStack.pop();

    return kNYI_PdfResult;
}

//dictName gs (PDF 1.2) Set the speciﬁed parameters in the graphics state. dictName is
//the name of a graphics state parameter dictionary in the ExtGState subdictionary of the current resource dictionary (see the next section).
PdfResult PdfOp_gs(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    PdfName name = pdfContext->fVarStack.top().GetName();    pdfContext->fVarStack.pop();

#ifdef PDF_TRACE
    std::string str;
#endif

    const PdfDictionary& pageDict = pdfContext->fGraphicsState.fObjectWithResources->GetDictionary();

#ifdef PDF_TRACE
    pdfContext->fGraphicsState.fObjectWithResources->ToString(str);
    printf("Print Object with resources: %s\n", str.c_str());
#endif

    const PdfObject* resources = resolveReferenceObject(pdfContext->fPdfDoc,
                                                        pageDict.GetKey("Resources"));

    if (resources == NULL) {
#ifdef PDF_TRACE
        printf("WARNING: No Resources for a page with 'gs' operator!\n");
#endif
        return kIgnoreError_PdfResult;
    }

#ifdef PDF_TRACE
    resources->ToString(str);
    printf("Print gs Page Resources: %s\n", str.c_str());
#endif

    if (!resources->IsDictionary()) {
#ifdef PDF_TRACE
        printf("Resources is not a dictionary!\n");
#endif
        return kIgnoreError_PdfResult;
    }

    const PdfDictionary& resourceDict = resources->GetDictionary();
    //Next, get the ExtGState Dictionary from the Resource Dictionary:
    const PdfObject* extGStateDictionary = resolveReferenceObject(pdfContext->fPdfDoc,
                                                                resourceDict.GetKey("ExtGState"));

    if (extGStateDictionary == NULL) {
#ifdef PDF_TRACE
        printf("ExtGState is NULL!\n");
#endif
        return kIgnoreError_PdfResult;
    }

    if (!extGStateDictionary->IsDictionary()) {
#ifdef PDF_TRACE
        printf("extGStateDictionary is not a dictionary!\n");
#endif
        return kIgnoreError_PdfResult;
    }

    const PdfObject* value =
            resolveReferenceObject(pdfContext->fPdfDoc,
                                   extGStateDictionary->GetDictionary().GetKey(name));

    if (value == NULL) {
#ifdef PDF_TRACE
        printf("Named object not found!\n");
#endif
        return kIgnoreError_PdfResult;
    }

#ifdef PDF_TRACE
    value->ToString(str);
    printf("gs object value: %s\n", str.c_str());
#endif

    // TODO(edisonn): now load all those properties in graphic state.

    return kNYI_PdfResult;
}

//charSpace Tc Set the character spacing, Tc
//, to charSpace, which is a number expressed in unscaled text space units. Character spacing is used by the Tj, TJ, and ' operators.
//Initial value: 0.
PdfResult PdfOp_Tc(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double charSpace = pdfContext->fVarStack.top().GetReal();     pdfContext->fVarStack.pop();
    pdfContext->fGraphicsState.fCharSpace = charSpace;

    return kOK_PdfResult;
}

//wordSpace Tw Set the word spacing, T
//w
//, to wordSpace, which is a number expressed in unscaled
//text space units. Word spacing is used by the Tj, TJ, and ' operators. Initial
//value: 0.
PdfResult PdfOp_Tw(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double wordSpace = pdfContext->fVarStack.top().GetReal();     pdfContext->fVarStack.pop();
    pdfContext->fGraphicsState.fWordSpace = wordSpace;

    return kOK_PdfResult;
}

//scale Tz Set the horizontal scaling, Th
//, to (scale ˜ 100). scale is a number specifying the
//percentage of the normal width. Initial value: 100 (normal width).
PdfResult PdfOp_Tz(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double scale = pdfContext->fVarStack.top().GetReal();     pdfContext->fVarStack.pop();

    return kNYI_PdfResult;
}

//render Tr Set the text rendering mode, T
//mode, to render, which is an integer. Initial value: 0.
PdfResult PdfOp_Tr(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double render = pdfContext->fVarStack.top().GetReal();     pdfContext->fVarStack.pop();

    return kNYI_PdfResult;
}

//rise Ts Set the text rise, Trise, to rise, which is a number expressed in unscaled text space
//units. Initial value: 0.
PdfResult PdfOp_Ts(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double rise = pdfContext->fVarStack.top().GetReal();     pdfContext->fVarStack.pop();

    return kNYI_PdfResult;
}

//wx wy d0
PdfResult PdfOp_d0(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fVarStack.pop();
    pdfContext->fVarStack.pop();

    return kNYI_PdfResult;
}

//wx wy llx lly urx ury d1
PdfResult PdfOp_d1(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fVarStack.pop();
    pdfContext->fVarStack.pop();
    pdfContext->fVarStack.pop();
    pdfContext->fVarStack.pop();
    pdfContext->fVarStack.pop();
    pdfContext->fVarStack.pop();

    return kNYI_PdfResult;
}

//name sh
PdfResult PdfOp_sh(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fVarStack.pop();

    return kNYI_PdfResult;
}

//name Do
PdfResult PdfOp_Do(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    PdfName name = pdfContext->fVarStack.top().GetName();    pdfContext->fVarStack.pop();

    const PdfDictionary& pageDict = pdfContext->fGraphicsState.fObjectWithResources->GetDictionary();
    const PdfObject* resources = resolveReferenceObject(pdfContext->fPdfDoc,
                                                        pageDict.GetKey("Resources"));

    if (resources == NULL) {
#ifdef PDF_TRACE
        printf("WARNING: No Resources for a page with 'Do' operator!s\n");
#endif
        return kIgnoreError_PdfResult;
    }

#ifdef PDF_TRACE
    std::string str;
    resources->ToString(str);
    printf("Print Do Page Resources: %s\n", str.c_str());
#endif

    if (!resources->IsDictionary()) {
#ifdef PDF_TRACE
        printf("Resources is not a dictionary!\n");
#endif
        return kIgnoreError_PdfResult;
    }

    const PdfDictionary& resourceDict = resources->GetDictionary();
    //Next, get the XObject Dictionary from the Resource Dictionary:
    const PdfObject* xObjectDictionary = resolveReferenceObject(pdfContext->fPdfDoc,
                                                                resourceDict.GetKey("XObject"));

    if (xObjectDictionary == NULL) {
#ifdef PDF_TRACE
        printf("XObject is NULL!\n");
#endif
        return kIgnoreError_PdfResult;
    }

    if (!xObjectDictionary->IsDictionary()) {
#ifdef PDF_TRACE
        printf("xObjectDictionary is not a dictionary!\n");
#endif
        return kIgnoreError_PdfResult;
    }

    const PdfObject* value =
            resolveReferenceObject(pdfContext->fPdfDoc,
                                   xObjectDictionary->GetDictionary().GetKey(name));

    if (value == NULL) {
#ifdef PDF_TRACE
        printf("Named object not found!\n");
#endif
        return kIgnoreError_PdfResult;
    }

#ifdef PDF_TRACE
    value->ToString(str);
    printf("Do object value: %s\n", str.c_str());
#endif

    return doXObject(pdfContext, canvas, *value);
}


//tag MP Designate a marked-content point. tag is a name object indicating the role or
//signiﬁcance of the point.
PdfResult PdfOp_MP(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fVarStack.pop();

    return kNYI_PdfResult;
}

//tag properties DP Designate a marked-content point with an associated property list. tag is a
//name object indicating the role or signiﬁcance of the point; properties is
//either an inline dictionary containing the property list or a name object
//associated with it in the Properties subdictionary of the current resource
//dictionary (see Section 9.5.1, “Property Lists”).
PdfResult PdfOp_DP(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fVarStack.pop();
    pdfContext->fVarStack.pop();

    return kNYI_PdfResult;
}

//tag BMC Begin a marked-content sequence terminated by a balancing EMC operator.
//tag is a name object indicating the role or signiﬁcance of the sequence.
PdfResult PdfOp_BMC(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fVarStack.pop();

    return kNYI_PdfResult;
}

//tag properties BDC Begin a marked-content sequence with an associated property list, terminated
//by a balancing EMCoperator. tag is a name object indicating the role or significance of the sequence; propertiesis either an inline dictionary containing the
//property list or a name object associated with it in the Properties subdictionary of the current resource dictionary (see Section 9.5.1, “Property Lists”).
PdfResult PdfOp_BDC(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fVarStack.pop();
    pdfContext->fVarStack.pop();

    return kNYI_PdfResult;
}

//— EMC End a marked-content sequence begun by a BMC or BDC operator.
PdfResult PdfOp_EMC(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return kNYI_PdfResult;
}

void initPdfOperatorRenderes() {
    static bool gInitialized = false;
    if (gInitialized) {
        return;
    }

    gPdfOps["q"] =      PdfOp_q;
    gPdfOps["Q"] =      PdfOp_Q;
    gPdfOps["cm"] =     PdfOp_cm;

    gPdfOps["TD"] =     PdfOp_TD;
    gPdfOps["Td"] =     PdfOp_Td;
    gPdfOps["Tm"] =     PdfOp_Tm;
    gPdfOps["T*"] =     PdfOp_T_star;

    gPdfOps["m"] =      PdfOp_m;
    gPdfOps["l"] =      PdfOp_l;
    gPdfOps["c"] =      PdfOp_c;
    gPdfOps["v"] =      PdfOp_v;
    gPdfOps["y"] =      PdfOp_y;
    gPdfOps["h"] =      PdfOp_h;
    gPdfOps["re"] =     PdfOp_re;

    gPdfOps["S"] =      PdfOp_S;
    gPdfOps["s"] =      PdfOp_s;
    gPdfOps["f"] =      PdfOp_f;
    gPdfOps["F"] =      PdfOp_F;
    gPdfOps["f*"] =     PdfOp_f_star;
    gPdfOps["B"] =      PdfOp_B;
    gPdfOps["B*"] =     PdfOp_B_star;
    gPdfOps["b"] =      PdfOp_b;
    gPdfOps["b*"] =     PdfOp_b_star;
    gPdfOps["n"] =      PdfOp_n;

    gPdfOps["BT"] =     PdfOp_BT;
    gPdfOps["ET"] =     PdfOp_ET;

    gPdfOps["Tj"] =     PdfOp_Tj;
    gPdfOps["'"] =      PdfOp_quote;
    gPdfOps["\""] =     PdfOp_doublequote;
    gPdfOps["TJ"] =     PdfOp_TJ;

    gPdfOps["CS"] =     PdfOp_CS;
    gPdfOps["cs"] =     PdfOp_cs;
    gPdfOps["SC"] =     PdfOp_SC;
    gPdfOps["SCN"] =    PdfOp_SCN;
    gPdfOps["sc"] =     PdfOp_sc;
    gPdfOps["scn"] =    PdfOp_scn;
    gPdfOps["G"] =      PdfOp_G;
    gPdfOps["g"] =      PdfOp_g;
    gPdfOps["RG"] =     PdfOp_RG;
    gPdfOps["rg"] =     PdfOp_rg;
    gPdfOps["K"] =      PdfOp_K;
    gPdfOps["k"] =      PdfOp_k;

    gPdfOps["W"] =      PdfOp_W;
    gPdfOps["W*"] =     PdfOp_W_star;

    gPdfOps["BX"] =     PdfOp_BX;
    gPdfOps["EX"] =     PdfOp_EX;

    gPdfOps["BI"] =     PdfOp_BI;
    gPdfOps["ID"] =     PdfOp_ID;
    gPdfOps["EI"] =     PdfOp_EI;

    gPdfOps["w"] =      PdfOp_w;
    gPdfOps["J"] =      PdfOp_J;
    gPdfOps["j"] =      PdfOp_j;
    gPdfOps["M"] =      PdfOp_M;
    gPdfOps["d"] =      PdfOp_d;
    gPdfOps["ri"] =     PdfOp_ri;
    gPdfOps["i"] =      PdfOp_i;
    gPdfOps["gs"] =     PdfOp_gs;

    gPdfOps["Tc"] =     PdfOp_Tc;
    gPdfOps["Tw"] =     PdfOp_Tw;
    gPdfOps["Tz"] =     PdfOp_Tz;
    gPdfOps["TL"] =     PdfOp_TL;
    gPdfOps["Tf"] =     PdfOp_Tf;
    gPdfOps["Tr"] =     PdfOp_Tr;
    gPdfOps["Ts"] =     PdfOp_Ts;

    gPdfOps["d0"] =     PdfOp_d0;
    gPdfOps["d1"] =     PdfOp_d1;

    gPdfOps["sh"] =     PdfOp_sh;

    gPdfOps["Do"] =     PdfOp_Do;

    gPdfOps["MP"] =     PdfOp_MP;
    gPdfOps["DP"] =     PdfOp_DP;
    gPdfOps["BMC"] =    PdfOp_BMC;
    gPdfOps["BDC"] =    PdfOp_BDC;
    gPdfOps["EMC"] =    PdfOp_EMC;

    gInitialized = true;
}

void reportPdfRenderStats() {
    std::map<std::string, int>::iterator iter;

    for (int i = 0 ; i < kCount_PdfResult; i++) {
        for (iter = gRenderStats[i].begin(); iter != gRenderStats[i].end(); ++iter) {
            printf("%s: %s -> count %i\n", gRenderStatsNames[i], iter->first.c_str(), iter->second);
        }
    }
}

PdfResult PdfMainLooper::consumeToken(PdfToken& token) {
    if( token.eType == ePdfContentsType_Keyword )
    {
        // TODO(edisonn): log trace flag (verbose, error, info, warning, ...)
#ifdef PDF_TRACE
        printf("KEYWORD: %s\n", token.pszToken);
#endif
        PdfOperatorRenderer pdfOperatorRenderer = gPdfOps[token.pszToken];
        if (pdfOperatorRenderer) {
            // caller, main work is done by pdfOperatorRenderer(...)
            PdfTokenLooper* childLooper = NULL;
            gRenderStats[pdfOperatorRenderer(fPdfContext, fCanvas, &childLooper)][token.pszToken]++;

            if (childLooper) {
                childLooper->setUp(this);
                childLooper->loop();
                delete childLooper;
            }
        } else {
            gRenderStats[kUnsupported_PdfResult][token.pszToken]++;
        }
    }
    else if ( token.eType == ePdfContentsType_Variant )
    {
#ifdef PDF_TRACE
        std::string _var;
        token.var.ToString(_var);
        printf("var: %s\n", _var.c_str());
#endif
        fPdfContext->fVarStack.push( token.var );
    }
    else if ( token.eType == ePdfContentsType_ImageData) {
        // TODO(edisonn): implement inline image.
    }
    else {
        return kIgnoreError_PdfResult;
    }
    return kOK_PdfResult;
}

void PdfMainLooper::loop() {
    PdfToken token;
    while (readToken(fTokenizer, &token)) {
        consumeToken(token);
    }
}

PdfResult PdfInlineImageLooper::consumeToken(PdfToken& token) {
    //pdfContext.fInlineImage.fKeyValuePairs[key] = value;
    return kNYI_PdfResult;
}

void PdfInlineImageLooper::loop() {
    PdfToken token;
    while (readToken(fTokenizer, &token)) {
        if (token.eType == ePdfContentsType_Keyword && strcmp(token.pszToken, "BX") == 0) {
            PdfTokenLooper* looper = new PdfCompatibilitySectionLooper();
            looper->setUp(this);
            looper->loop();
        } else {
            if (token.eType == ePdfContentsType_Keyword && strcmp(token.pszToken, "EI") == 0) {
                done();
                return;
            }

            consumeToken(token);
        }
    }
    // TODO(edisonn): report error/warning, EOF without EI.
}

PdfResult PdfInlineImageLooper::done() {

    // TODO(edisonn): long to short names
    // TODO(edisonn): set properties in a map
    // TODO(edisonn): extract bitmap stream, check if PoDoFo has public utilities to uncompress
    // the stream.

    SkBitmap bitmap;
    setup_bitmap(&bitmap, 50, 50, SK_ColorRED);

    // TODO(edisonn): matrix use.
    // Draw dummy red square, to show the prezence of the inline image.
    fCanvas->drawBitmap(bitmap,
                       SkDoubleToScalar(0),
                       SkDoubleToScalar(0),
                       NULL);
    return kNYI_PdfResult;
}

PdfResult PdfCompatibilitySectionLooper::consumeToken(PdfToken& token) {
    return fParent->consumeToken(token);
}

void PdfCompatibilitySectionLooper::loop() {
    // TODO(edisonn): save stacks position, or create a new stack?
    // TODO(edisonn): what happens if we pop out more variables then when we started?
    // restore them? fail? We could create a new operands stack for every new BX/EX section,
    // pop-ing too much will not affect outside the section.
    PdfToken token;
    while (readToken(fTokenizer, &token)) {
        if (token.eType == ePdfContentsType_Keyword && strcmp(token.pszToken, "BX") == 0) {
            PdfTokenLooper* looper = new PdfCompatibilitySectionLooper();
            looper->setUp(this);
            looper->loop();
            delete looper;
        } else {
            if (token.eType == ePdfContentsType_Keyword && strcmp(token.pszToken, "EX") == 0) break;
            fParent->consumeToken(token);
        }
    }
    // TODO(edisonn): restore stack.
}

// TODO(edisonn): fix PoDoFo load ~/crashing/Shading.pdf
// TODO(edisonn): Add API for Forms viewing and editing
// e.g. SkBitmap getPage(int page);
//      int formsCount();
//      SkForm getForm(int formID); // SkForm(SkRect, .. other data)
// TODO (edisonn): Add intend when loading pdf, for example: for viewing, parsing all content, ...
// if we load the first page, and we zoom to fit to screen horizontally, then load only those
// resources needed, so the preview is fast.
// TODO (edisonn): hide parser/tokenizer behind and interface and a query language, and resolve
// references automatically.
class SkPdfViewer : public SkRefCnt {
public:

  bool load(const SkString inputFileName, SkPicture* out) {

    initPdfOperatorRenderes();

    try
    {
        std::cout << "Init: " << inputFileName.c_str() << std::endl;

        PdfMemDocument doc(inputFileName.c_str());
        if( !doc.GetPageCount() )
        {
            std::cout << "ERROR: Empty Document" << inputFileName.c_str() << std::endl;
            return false;
        } else {

            for (int pn = 0; pn < doc.GetPageCount(); ++pn) {
                PoDoFo::PdfPage* page = doc.GetPage(pn);
                PdfRect rect = page->GetMediaBox();
#ifdef PDF_TRACE
                printf("Page Width: %f, Page Height: %f\n", rect.GetWidth(), rect.GetHeight());
#endif

                // TODO(edisonn): page->GetCropBox(), page->GetTrimBox() ... how to use?

                SkBitmap bitmap;
#ifdef PDF_DEBUG_3X
                setup_bitmap(&bitmap, 3*rect.GetWidth(), 3*rect.GetHeight());
#else
                setup_bitmap(&bitmap, rect.GetWidth(), rect.GetHeight());
#endif
                SkAutoTUnref<SkDevice> device(SkNEW_ARGS(SkDevice, (bitmap)));
                SkCanvas canvas(device);


                const char*      pszToken = NULL;
                PdfVariant       var;
                EPdfContentsType eType;

                PdfContentsTokenizer tokenizer( page );

                PdfContext pdfContext;
                pdfContext.fPdfPage = page;
                pdfContext.fPdfDoc = &doc;
                pdfContext.fOriginalMatrix = SkMatrix::I();
                pdfContext.fGraphicsState.fObjectWithResources = pdfContext.fPdfPage->GetObject();

                gPdfContext = &pdfContext;
                gDumpBitmap = &bitmap;
                gDumpCanvas = &canvas;


                // TODO(edisonn): get matrix stuff right.
                // TODO(edisonn): add DPI/scale/zoom.
                SkScalar z = SkIntToScalar(0);
                SkScalar w = SkDoubleToScalar(rect.GetWidth());
                SkScalar h = SkDoubleToScalar(rect.GetHeight());

                SkPoint pdfSpace[4] = {SkPoint::Make(z, z), SkPoint::Make(w, z), SkPoint::Make(w, h), SkPoint::Make(z, h)};
//                SkPoint skiaSpace[4] = {SkPoint::Make(z, h), SkPoint::Make(w, h), SkPoint::Make(w, z), SkPoint::Make(z, z)};

                // TODO(edisonn): add flag for this app to create sourunding buffer zone
                // TODO(edisonn): add flagg for no clipping.
                // Use larger image to make sure we do not draw anything outside of page
                // could be used in tests.

#ifdef PDF_DEBUG_3X
                SkPoint skiaSpace[4] = {SkPoint::Make(w+z, h+h), SkPoint::Make(w+w, h+h), SkPoint::Make(w+w, h+z), SkPoint::Make(w+z, h+z)};
#else
                SkPoint skiaSpace[4] = {SkPoint::Make(z, h), SkPoint::Make(w, h), SkPoint::Make(w, z), SkPoint::Make(z, z)};
#endif
                //SkPoint pdfSpace[2] = {SkPoint::Make(z, z), SkPoint::Make(w, h)};
                //SkPoint skiaSpace[2] = {SkPoint::Make(w, z), SkPoint::Make(z, h)};

                //SkPoint pdfSpace[2] = {SkPoint::Make(z, z), SkPoint::Make(z, h)};
                //SkPoint skiaSpace[2] = {SkPoint::Make(z, h), SkPoint::Make(z, z)};

                //SkPoint pdfSpace[3] = {SkPoint::Make(z, z), SkPoint::Make(z, h), SkPoint::Make(w, h)};
                //SkPoint skiaSpace[3] = {SkPoint::Make(z, h), SkPoint::Make(z, z), SkPoint::Make(w, 0)};

                SkAssertResult(pdfContext.fOriginalMatrix.setPolyToPoly(pdfSpace, skiaSpace, 4));
                SkTraceMatrix(pdfContext.fOriginalMatrix, "Original matrix");


                pdfContext.fGraphicsState.fMatrix = pdfContext.fOriginalMatrix;
                pdfContext.fGraphicsState.fMatrixTm = pdfContext.fGraphicsState.fMatrix;
                pdfContext.fGraphicsState.fMatrixTlm = pdfContext.fGraphicsState.fMatrix;

                canvas.setMatrix(pdfContext.fOriginalMatrix);

#ifndef PDF_DEBUG_NO_PAGE_CLIPING
                canvas.clipRect(SkRect::MakeXYWH(z, z, w, h), SkRegion::kIntersect_Op, true);
#endif

                PdfMainLooper looper(NULL, &tokenizer, &pdfContext, &canvas);
                looper.loop();

                canvas.flush();

                SkString out;
                out.appendf("%s-%i.png", inputFileName.c_str(), pn);
                SkImageEncoder::EncodeFile(out.c_str(), bitmap, SkImageEncoder::kPNG_Type, 100);
            }
            return true;
        }
    }
    catch( PdfError & e )
    {
        std::cout << "ERROR: PDF can't be parsed!" << inputFileName.c_str() << std::endl;
        return false;
    }

    return true;
  }
  bool write(void*) const { return false; }
};



/**
 * Given list of directories and files to use as input, expects to find .pdf
 * files and it will convert them to .png files writing them in the same directory
 * one file for each page.
 *
 * Returns zero exit code if all .pdf files were converted successfully,
 * otherwise returns error code 1.
 */

static const char PDF_FILE_EXTENSION[] = "pdf";
static const char PNG_FILE_EXTENSION[] = "png";

// TODO(edisonn): add ability to write to a new directory.
static void usage(const char* argv0) {
    SkDebugf("PDF to PNG rendering tool\n");
    SkDebugf("\n"
"Usage: \n"
"     %s <input>... -w <outputDir> \n"
, argv0);
    SkDebugf("\n\n");
    SkDebugf(
"     input:     A list of directories and files to use as input. Files are\n"
"                expected to have the .skp extension.\n\n");
    SkDebugf(
"     outputDir: directory to write the rendered pdfs.\n\n");
    SkDebugf("\n");
}

/** Replaces the extension of a file.
 * @param path File name whose extension will be changed.
 * @param old_extension The old extension.
 * @param new_extension The new extension.
 * @returns false if the file did not has the expected extension.
 *  if false is returned, contents of path are undefined.
 */
static bool replace_filename_extension(SkString* path,
                                       const char old_extension[],
                                       const char new_extension[]) {
    if (path->endsWith(old_extension)) {
        path->remove(path->size() - strlen(old_extension),
                     strlen(old_extension));
        if (!path->endsWith(".")) {
            return false;
        }
        path->append(new_extension);
        return true;
    }
    return false;
}

/** Builds the output filename. path = dir/name, and it replaces expected
 * .skp extension with .pdf extention.
 * @param path Output filename.
 * @param name The name of the file.
 * @returns false if the file did not has the expected extension.
 *  if false is returned, contents of path are undefined.
 */
static bool make_output_filepath(SkString* path, const SkString& dir,
                                 const SkString& name) {
    sk_tools::make_filepath(path, dir, name);
    return replace_filename_extension(path,
                                      PDF_FILE_EXTENSION,
                                      PNG_FILE_EXTENSION);
}

/** Write the output of pdf renderer to a file.
 * @param outputDir Output dir.
 * @param inputFilename The skp file that was read.
 * @param renderer The object responsible to write the pdf file.
 */
static bool write_output(const SkString& outputDir,
                         const SkString& inputFilename,
                         const SkPdfViewer& renderer) {
    if (outputDir.isEmpty()) {
        SkDynamicMemoryWStream stream;
        renderer.write(&stream);
        return true;
    }

    SkString outputPath;
    if (!make_output_filepath(&outputPath, outputDir, inputFilename)) {
        return false;
    }

    SkFILEWStream stream(outputPath.c_str());
    if (!stream.isValid()) {
        SkDebugf("Could not write to file %s\n", outputPath.c_str());
        return false;
    }
    renderer.write(&stream);

    return true;
}

/** Reads an skp file, renders it to pdf and writes the output to a pdf file
 * @param inputPath The skp file to be read.
 * @param outputDir Output dir.
 * @param renderer The object responsible to render the skp object into pdf.
 */
static bool parse_pdf(const SkString& inputPath, const SkString& outputDir,
                       SkPdfViewer& renderer) {
    SkString inputFilename;
    sk_tools::get_basename(&inputFilename, inputPath);

    SkFILEStream inputStream;
    inputStream.setPath(inputPath.c_str());
    if (!inputStream.isValid()) {
        SkDebugf("Could not open file %s\n", inputPath.c_str());
        return false;
    }

    bool success = false;

    success = renderer.load(inputPath, NULL);


//    success = write_output(outputDir, inputFilename, renderer);

    //renderer.end();
    return success;
}

/** For each file in the directory or for the file passed in input, call
 * parse_pdf.
 * @param input A directory or an pdf file.
 * @param outputDir Output dir.
 * @param renderer The object responsible to render the skp object into pdf.
 */
static int process_input(const SkString& input, const SkString& outputDir,
                         SkPdfViewer& renderer) {
    int failures = 0;
    if (sk_isdir(input.c_str())) {
        SkOSFile::Iter iter(input.c_str(), PDF_FILE_EXTENSION);
        SkString inputFilename;
        while (iter.next(&inputFilename)) {
            SkString inputPath;
            sk_tools::make_filepath(&inputPath, input, inputFilename);
            if (!parse_pdf(inputPath, outputDir, renderer)) {
                ++failures;
            }
        }
    } else {
        SkString inputPath(input);
        if (!parse_pdf(inputPath, outputDir, renderer)) {
            ++failures;
        }
    }
    return failures;
}

static void parse_commandline(int argc, char* const argv[],
                              SkTArray<SkString>* inputs,
                              SkString* outputDir) {
    const char* argv0 = argv[0];
    char* const* stop = argv + argc;

    for (++argv; argv < stop; ++argv) {
        if ((0 == strcmp(*argv, "-h")) || (0 == strcmp(*argv, "--help"))) {
            usage(argv0);
            exit(-1);
        } else if (0 == strcmp(*argv, "-w")) {
            ++argv;
            if (argv >= stop) {
                SkDebugf("Missing outputDir for -w\n");
                usage(argv0);
                exit(-1);
            }
            *outputDir = SkString(*argv);
        } else {
            inputs->push_back(SkString(*argv));
        }
    }

    if (inputs->count() < 1) {
        usage(argv0);
        exit(-1);
    }
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkAutoGraphics ag;
    SkTArray<SkString> inputs;

    SkAutoTUnref<SkPdfViewer>
        renderer(SkNEW(SkPdfViewer));
    SkASSERT(renderer.get());

    SkString outputDir;
    parse_commandline(argc, argv, &inputs, &outputDir);

    int failures = 0;
    for (int i = 0; i < inputs.count(); i ++) {
        failures += process_input(inputs[i], outputDir, *renderer);
    }

    reportPdfRenderStats();

    if (failures != 0) {
        SkDebugf("Failed to render %i PDFs.\n", failures);
        return 1;
    }

    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
