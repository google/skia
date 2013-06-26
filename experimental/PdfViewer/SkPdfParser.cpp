/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkForceLinking.h"
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


__SK_FORCE_IMAGE_DECODER_LINKING;

// TODO(edisonn): tool, show what objects were read at least, show the ones not even read
// keep for each object pos in file
// plug in for VS? syntax coloring, show selected object ... from the text, or from rendered x,y

// TODO(edisonn): security - validate all the user input, all pdf!


#include "SkPdfHeaders_autogen.h"
#include "SkPdfPodofoMapper_autogen.h"
#include "SkPdfParser.h"

#include "SkPdfBasics.h"
#include "SkPdfUtils.h"

#include "SkPdfFont.h"

bool skpdfmap(const PdfMemDocument& podofoDoc, const PdfObject& podofoObj, SkPdfObject** out) {
    return mapObject(podofoDoc, podofoObj, out);
}


/*
 * TODO(edisonn):
 * - all font types and all ppdf font features
 *      - word spacing
 *      - load font for baidu.pdf
 *      - load font for youtube.pdf
 *      - parser for pdf from the definition already available in pdfspec_autogen.py
 *      - all docs from ~/work
 * - encapsulate podofo in the pdf api so the skpdf does not know anything about podofo ... in progress
 * - load gs/ especially smask and already known prop (skp) ... in progress
 * - wrapper on classes for customizations? e.g.
 * SkPdfPageObjectVanila - has only the basic loaders/getters
 * SkPdfPageObject : public SkPdfPageObjectVanila, extends, and I can add customizations here
 * need to find a nice object model for all this with constructors and factories
 * - deal with inheritable automatically ?
 * - deal with specific type in spec directly, add all dictionary types to known types
*/

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
                                  bool resolveOneElementArrays) {
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

SkMatrix SkMatrixFromPdfArray(SkPdfArray* pdfArray) {
    double array[6];

    // TODO(edisonn): security issue, ret if size() != 6
    for (int i = 0; i < 6; i++) {
        const PdfObject* elem = resolveReferenceObject(pdfArray->doc(), (*pdfArray)[i]->podofo());
        if (elem == NULL || (!elem->IsReal() && !elem->IsNumber())) {
            return SkMatrix::I();  // TODO(edisonn): report issue
        }
        array[i] = elem->GetReal();
    }

    return SkMatrixFromPdfMatrix(array);
}

PdfContext* gPdfContext = NULL;
SkBitmap* gDumpBitmap = NULL;
SkCanvas* gDumpCanvas = NULL;
char gLastKeyword[100] = "";
int gLastOpKeyword = -1;
char allOpWithVisualEffects[100] = ",S,s,f,F,f*,B,B*,b,b*,n,Tj,TJ,\',\",d0,d1,sh,EI,Do,EX,";
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
static bool readToken(SkPdfTokenizer* fTokenizer, PdfToken* token) {
    bool ret = fTokenizer->readToken(token);

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

    if (token->fType == kKeyword_TokenType) {
        strcpy(gLastKeyword, token->fKeyword);
        gLastOpKeyword = gReadOp;
    } else {
        strcpy(gLastKeyword, "");
    }
#endif

    return ret;
}



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

static SkTypeface* SkTypefaceFromPdfFont(PdfFont* font) {
    if (font == NULL) {
        return SkTypeface::CreateFromName("Times New Roman", SkTypeface::kNormal);
    }

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

PdfResult DrawText(PdfContext* pdfContext,
                   const SkPdfObject* str,
                   SkCanvas* canvas)
{

    SkPdfFont* skfont = pdfContext->fGraphicsState.fSkFont;
    if (skfont == NULL) {
        skfont = SkPdfFont::Default();
    }

    SkUnencodedText binary(str);

    SkDecodedText decoded;

    if (skfont->encoding() == NULL) {
        // TODO(edisonn): report warning
        return kNYI_PdfResult;
    }

    skfont->encoding()->decodeText(binary, &decoded);

    SkPaint paint;
    // TODO(edisonn): when should fCurFont->GetFontSize() used? When cur is fCurFontSize == 0?
    // Or maybe just not call setTextSize at all?
    if (pdfContext->fGraphicsState.fCurFontSize != 0) {
        paint.setTextSize(SkDoubleToScalar(pdfContext->fGraphicsState.fCurFontSize));
    }

//    if (fCurFont && fCurFont->GetFontScale() != 0) {
//        paint.setTextScaleX(SkFloatToScalar(fCurFont->GetFontScale() / 100.0));
//    }

    pdfContext->fGraphicsState.applyGraphicsState(&paint, false);

    canvas->save();

#if 1
    SkMatrix matrix = pdfContext->fGraphicsState.fMatrixTm;

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
#endif

    skfont->drawText(decoded, &paint, pdfContext, canvas, &pdfContext->fGraphicsState.fMatrixTm);
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
    if (data == NULL) {
        return true;
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

    if (value == NULL || (!value->IsReal() && !value->IsNumber())) {
        return false;
    }
    if (data == NULL) {
        return true;
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
    if (data == NULL) {
        return true;
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
    if (data == NULL) {
        return true;
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
    if (data == NULL) {
        return true;
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

/*
bool ArrayFromDictionary(const PdfMemDocument* pdfDoc,
                              const PdfDictionary& dict,
                              const char* key,
                              SkPdfArray** data) {
    const PdfObject* value = resolveReferenceObject(pdfDoc,
                                              dict.GetKey(PdfName(key)),
                                              true);
    if (value == NULL || !value->IsArray()) {
        return false;
    }
    if (data == NULL) {
        return true;
    }

    return mapArray(*pdfDoc, *value, data);
}


bool ArrayFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkPdfArray** data) {
    if (ArrayFromDictionary(pdfDoc, dict, key, data)) return true;
    if (abr == NULL || *abr == '\0') return false;
    return ArrayFromDictionary(pdfDoc, dict, abr, data);
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
    if (data == NULL) {
        return true;
    }

    return mapDictionary(*pdfDoc, *value, data);
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
    if (data == NULL) {
        return true;
    }
    return mapObject(*pdfDoc, *value, data);
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

bool StreamFromDictionary(const PdfMemDocument* pdfDoc,
                          const PdfDictionary& dict,
                          const char* key,
                          SkPdfStream** data) {
    const PdfObject* value = resolveReferenceObject(pdfDoc,
                                              dict.GetKey(PdfName(key)),
                                              true);
    if (value == NULL) {
        return false;
    }
    if (data == NULL) {
        return true;
    }
    return mapStream(*pdfDoc, *value, data);
}

bool StreamFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkPdfStream** data) {
    if (StreamFromDictionary(pdfDoc, dict, key, data)) return true;
    if (abr == NULL || *abr == '\0') return false;
    return StreamFromDictionary(pdfDoc, dict, abr, data);
}
*/

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
    std::string colorSpace = "DeviceRGB";

    // TODO(edisonn): color space can be an array too!
    if (image->isColorSpaceAName()) {
        colorSpace = image->getColorSpaceAsName();
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
    const PdfObject* sMask = resolveReferenceObject(&pdfContext->fPdfDoc->podofo(),
                                              obj->podofo()->GetDictionary().GetKey(PdfName("SMask")));

#ifdef PDF_TRACE
    std::string str;
    if (sMask) {
        sMask->ToString(str);
        printf("/SMask of /Subtype /Image: %s\n", str.c_str());
    }
#endif

    if (sMask) {
        SkPdfImageDictionary skxobjmask(&pdfContext->fPdfDoc->podofo(), sMask);
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
        pdfContext->fGraphicsState.applyGraphicsState(&xfer, false);
        xfer.setXfermodeMode(SkXfermode::kSrcOut_Mode); // SkXfermode::kSdtOut_Mode
        canvas->drawBitmapRect(sMask, dst, &xfer);
        canvas->restore();
    }

    canvas->restore();

    return kPartial_PdfResult;
}

bool SkMatrixFromDictionary(const PdfMemDocument* pdfDoc,
                            const PdfDictionary& dict,
                            const char* key,
                            SkMatrix** matrix) {
    const PdfObject* value = resolveReferenceObject(pdfDoc,
                                                    dict.GetKey(PdfName(key)));

    if (value == NULL || !value->IsArray()) {
        return false;
    }

    if (value->GetArray().GetSize() != 6) {
        return false;
    }

    double array[6];
    for (int i = 0; i < 6; i++) {
        const PdfObject* elem = resolveReferenceObject(pdfDoc, &value->GetArray()[i]);
        if (elem == NULL || (!elem->IsReal() && !elem->IsNumber())) {
            return false;
        }
        array[i] = elem->GetReal();
    }

    *matrix = new SkMatrix();
    **matrix = SkMatrixFromPdfMatrix(array);
    return true;
}

bool SkMatrixFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkMatrix** data) {
    if (SkMatrixFromDictionary(pdfDoc, dict, key, data)) return true;
    if (abr == NULL || *abr == '\0') return false;
    return SkMatrixFromDictionary(pdfDoc, dict, abr, data);

}

bool SkRectFromDictionary(const PdfMemDocument* pdfDoc,
                          const PdfDictionary& dict,
                          const char* key,
                          SkRect** rect) {
    const PdfObject* value = resolveReferenceObject(pdfDoc,
                                                    dict.GetKey(PdfName(key)));

    if (value == NULL || !value->IsArray()) {
        return false;
    }

    if (value->GetArray().GetSize() != 4) {
        return false;
    }

    double array[4];
    for (int i = 0; i < 4; i++) {
        const PdfObject* elem = resolveReferenceObject(pdfDoc, &value->GetArray()[i]);
        if (elem == NULL || (!elem->IsReal() && !elem->IsNumber())) {
            return false;
        }
        array[i] = elem->GetReal();
    }

    *rect = new SkRect();
    **rect = SkRect::MakeLTRB(SkDoubleToScalar(array[0]),
                              SkDoubleToScalar(array[1]),
                              SkDoubleToScalar(array[2]),
                              SkDoubleToScalar(array[3]));
    return true;
}

bool SkRectFromDictionary(const PdfMemDocument* pdfDoc,
                        const PdfDictionary& dict,
                        const char* key,
                        const char* abr,
                        SkRect** data) {
    if (SkRectFromDictionary(pdfDoc, dict, key, data)) return true;
    if (abr == NULL || *abr == '\0') return false;
    return SkRectFromDictionary(pdfDoc, dict, abr, data);

}


SkPdfObject* get(const SkPdfObject* obj, const char* key, const char* abr = "") {
    SkPdfObject* ret = NULL;
    if (obj == NULL) return NULL;
    const SkPdfDictionary* dict = obj->asDictionary();
    if (dict == NULL) return NULL;
    if (!dict->podofo()->IsDictionary()) return NULL;
    ObjectFromDictionary(dict->doc(), dict->podofo()->GetDictionary(), key, abr, &ret);
    return ret;
}

PdfResult doXObject_Form(PdfContext* pdfContext, SkCanvas* canvas, SkPdfType1FormDictionary* skobj) {
    if (!skobj || !skobj->podofo() || !skobj->podofo()->HasStream() || skobj->podofo()->GetStream() == NULL || skobj->podofo()->GetStream()->GetLength() == 0) {
        return kOK_PdfResult;
    }

    PdfOp_q(pdfContext, canvas, NULL);
    canvas->save();


    if (skobj->Resources()) {
        pdfContext->fGraphicsState.fResources = skobj->Resources();
    }

    SkTraceMatrix(pdfContext->fGraphicsState.fMatrix, "Current matrix");

    if (skobj->Matrix()) {
        pdfContext->fGraphicsState.fMatrix.preConcat(*skobj->Matrix());
        pdfContext->fGraphicsState.fMatrixTm = pdfContext->fGraphicsState.fMatrix;
        pdfContext->fGraphicsState.fMatrixTlm = pdfContext->fGraphicsState.fMatrix;
        // TODO(edisonn) reset matrixTm and matricTlm also?
    }

    SkTraceMatrix(pdfContext->fGraphicsState.fMatrix, "Total matrix");

    canvas->setMatrix(pdfContext->fGraphicsState.fMatrix);

    if (skobj->BBox()) {
        canvas->clipRect(*skobj->BBox(), SkRegion::kIntersect_Op, true);  // TODO(edisonn): AA from settings.
    }

    // TODO(edisonn): iterate smart on the stream even if it is compressed, tokenize it as we go.
    // For this PdfContentsTokenizer needs to be extended.

    PdfResult ret = kPartial_PdfResult;
    SkPdfTokenizer tokenizer(skobj);
    PdfMainLooper looper(NULL, &tokenizer, pdfContext, canvas);
    looper.loop();

    // TODO(edisonn): should we restore the variable stack at the same state?
    // There could be operands left, that could be consumed by a parent tokenizer when we pop.
    canvas->restore();
    PdfOp_Q(pdfContext, canvas, NULL);
    return ret;
}

PdfResult doXObject_PS(PdfContext* pdfContext, SkCanvas* canvas, const PdfObject& obj) {
    return kNYI_PdfResult;
}

PdfResult doType3Char(PdfContext* pdfContext, SkCanvas* canvas, SkPdfObject* skobj, SkRect bBox, SkMatrix matrix, double textSize) {
    if (!skobj || !skobj->podofo() || !skobj->podofo()->HasStream() || skobj->podofo()->GetStream() == NULL || skobj->podofo()->GetStream()->GetLength() == 0) {
        return kOK_PdfResult;
    }

    PdfOp_q(pdfContext, canvas, NULL);
    canvas->save();

    pdfContext->fGraphicsState.fMatrixTm.preConcat(matrix);
    pdfContext->fGraphicsState.fMatrixTm.preScale(SkDoubleToScalar(textSize), SkDoubleToScalar(textSize));

    pdfContext->fGraphicsState.fMatrix = pdfContext->fGraphicsState.fMatrixTm;
    pdfContext->fGraphicsState.fMatrixTlm = pdfContext->fGraphicsState.fMatrix;

    SkTraceMatrix(pdfContext->fGraphicsState.fMatrix, "Total matrix");

    canvas->setMatrix(pdfContext->fGraphicsState.fMatrix);

    SkRect rm = bBox;
    pdfContext->fGraphicsState.fMatrix.mapRect(&rm);

    SkTraceRect(rm, "bbox mapped");

    canvas->clipRect(bBox, SkRegion::kIntersect_Op, true);  // TODO(edisonn): AA from settings.

    // TODO(edisonn): iterate smart on the stream even if it is compressed, tokenize it as we go.
    // For this PdfContentsTokenizer needs to be extended.

    PdfResult ret = kPartial_PdfResult;
    SkPdfTokenizer tokenizer(skobj);
    PdfMainLooper looper(NULL, &tokenizer, pdfContext, canvas);
    looper.loop();

    // TODO(edisonn): should we restore the variable stack at the same state?
    // There could be operands left, that could be consumed by a parent tokenizer when we pop.
    canvas->restore();
    PdfOp_Q(pdfContext, canvas, NULL);
    return ret;
}


// TODO(edisonn): faster, have the property on the SkPdfObject itself?
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

PdfResult doXObject(PdfContext* pdfContext, SkCanvas* canvas, const SkPdfObject& obj) {
    if (CheckRecursiveRendering::IsInRendering(*obj.podofo())) {
        // Oops, corrupt PDF!
        return kIgnoreError_PdfResult;
    }

    CheckRecursiveRendering checkRecursion(*obj.podofo());

    // TODO(edisonn): check type
    SkPdfXObjectDictionary* skobj = NULL;
    if (!mapXObjectDictionary(obj, &skobj)) return kIgnoreError_PdfResult;

    if (!skobj || !skobj->valid()) return kIgnoreError_PdfResult;

    PdfResult ret = kIgnoreError_PdfResult;
    switch (skobj->getType())
    {
        case kImageDictionary_SkPdfObjectType:
            ret = doXObject_Image(pdfContext, canvas, skobj->asImageDictionary());
            break;
        case kType1FormDictionary_SkPdfObjectType:
            ret = doXObject_Form(pdfContext, canvas, skobj->asType1FormDictionary());
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
        array[5 - i] = pdfContext->fObjectStack.top()->asNumber()->value();
        pdfContext->fObjectStack.pop();
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
    double ty = pdfContext->fObjectStack.top()->asNumber()->value(); pdfContext->fObjectStack.pop();

    pdfContext->fGraphicsState.fTextLeading = ty;

    return kOK_PdfResult;
}

PdfResult PdfOp_Td(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double ty = pdfContext->fObjectStack.top()->asNumber()->value(); pdfContext->fObjectStack.pop();
    double tx = pdfContext->fObjectStack.top()->asNumber()->value(); pdfContext->fObjectStack.pop();

    double array[6] = {1, 0, 0, 1, tx, ty};
    SkMatrix matrix = SkMatrixFromPdfMatrix(array);

    pdfContext->fGraphicsState.fMatrixTm.preConcat(matrix);
    pdfContext->fGraphicsState.fMatrixTlm.preConcat(matrix);

    return kPartial_PdfResult;
}

PdfResult PdfOp_TD(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double ty = pdfContext->fObjectStack.top()->asNumber()->value(); pdfContext->fObjectStack.pop();
    double tx = pdfContext->fObjectStack.top()->asNumber()->value(); pdfContext->fObjectStack.pop();

    // TODO(edisonn): Create factory methods or constructors so podofo is hidden
    PdfObject _ty(PdfVariant(-ty));
    pdfContext->fObjectStack.push(new SkPdfNumber(&pdfContext->fPdfDoc->podofo(), &_ty));

    PdfOp_TL(pdfContext, canvas, looper);

    PdfObject vtx(PdfVariant(-(-tx)));  // TODO(edisonn): Hmm, the compiler thinks I have here a function pointer if we use (tx), but not -(-tx)
    pdfContext->fObjectStack.push(new SkPdfNumber(&pdfContext->fPdfDoc->podofo(), &vtx));

    PdfObject vty(PdfVariant(-(-ty)));
    pdfContext->fObjectStack.push(new SkPdfNumber(&pdfContext->fPdfDoc->podofo(), &vty));

    return PdfOp_Td(pdfContext, canvas, looper);
}

PdfResult PdfOp_Tm(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double f = pdfContext->fObjectStack.top()->asNumber()->value(); pdfContext->fObjectStack.pop();
    double e = pdfContext->fObjectStack.top()->asNumber()->value(); pdfContext->fObjectStack.pop();
    double d = pdfContext->fObjectStack.top()->asNumber()->value(); pdfContext->fObjectStack.pop();
    double c = pdfContext->fObjectStack.top()->asNumber()->value(); pdfContext->fObjectStack.pop();
    double b = pdfContext->fObjectStack.top()->asNumber()->value(); pdfContext->fObjectStack.pop();
    double a = pdfContext->fObjectStack.top()->asNumber()->value(); pdfContext->fObjectStack.pop();

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
    PdfObject zero(PdfVariant(0.0));
    PdfObject tl(PdfVariant(-(-pdfContext->fGraphicsState.fTextLeading)));

    pdfContext->fObjectStack.push(new SkPdfNumber(&pdfContext->fPdfDoc->podofo(), &zero));
    pdfContext->fObjectStack.push(new SkPdfNumber(&pdfContext->fPdfDoc->podofo(), &tl));
    return PdfOp_Td(pdfContext, canvas, looper);
}

PdfResult PdfOp_m(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (pdfContext->fGraphicsState.fPathClosed) {
        pdfContext->fGraphicsState.fPath.reset();
        pdfContext->fGraphicsState.fPathClosed = false;
    }

    pdfContext->fGraphicsState.fCurPosY = pdfContext->fObjectStack.top()->asNumber()->value();    pdfContext->fObjectStack.pop();
    pdfContext->fGraphicsState.fCurPosX = pdfContext->fObjectStack.top()->asNumber()->value();    pdfContext->fObjectStack.pop();

    pdfContext->fGraphicsState.fPath.moveTo(SkDoubleToScalar(pdfContext->fGraphicsState.fCurPosX),
                                          SkDoubleToScalar(pdfContext->fGraphicsState.fCurPosY));

    return kOK_PdfResult;
}

PdfResult PdfOp_l(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (pdfContext->fGraphicsState.fPathClosed) {
        pdfContext->fGraphicsState.fPath.reset();
        pdfContext->fGraphicsState.fPathClosed = false;
    }

    pdfContext->fGraphicsState.fCurPosY = pdfContext->fObjectStack.top()->asNumber()->value();    pdfContext->fObjectStack.pop();
    pdfContext->fGraphicsState.fCurPosX = pdfContext->fObjectStack.top()->asNumber()->value();    pdfContext->fObjectStack.pop();

    pdfContext->fGraphicsState.fPath.lineTo(SkDoubleToScalar(pdfContext->fGraphicsState.fCurPosX),
                                          SkDoubleToScalar(pdfContext->fGraphicsState.fCurPosY));

    return kOK_PdfResult;
}

PdfResult PdfOp_c(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (pdfContext->fGraphicsState.fPathClosed) {
        pdfContext->fGraphicsState.fPath.reset();
        pdfContext->fGraphicsState.fPathClosed = false;
    }

    double y3 = pdfContext->fObjectStack.top()->asNumber()->value();    pdfContext->fObjectStack.pop();
    double x3 = pdfContext->fObjectStack.top()->asNumber()->value();    pdfContext->fObjectStack.pop();
    double y2 = pdfContext->fObjectStack.top()->asNumber()->value();    pdfContext->fObjectStack.pop();
    double x2 = pdfContext->fObjectStack.top()->asNumber()->value();    pdfContext->fObjectStack.pop();
    double y1 = pdfContext->fObjectStack.top()->asNumber()->value();    pdfContext->fObjectStack.pop();
    double x1 = pdfContext->fObjectStack.top()->asNumber()->value();    pdfContext->fObjectStack.pop();

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

    double y3 = pdfContext->fObjectStack.top()->asNumber()->value();    pdfContext->fObjectStack.pop();
    double x3 = pdfContext->fObjectStack.top()->asNumber()->value();    pdfContext->fObjectStack.pop();
    double y2 = pdfContext->fObjectStack.top()->asNumber()->value();    pdfContext->fObjectStack.pop();
    double x2 = pdfContext->fObjectStack.top()->asNumber()->value();    pdfContext->fObjectStack.pop();
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

    double y3 = pdfContext->fObjectStack.top()->asNumber()->value();    pdfContext->fObjectStack.pop();
    double x3 = pdfContext->fObjectStack.top()->asNumber()->value();    pdfContext->fObjectStack.pop();
    double y2 = pdfContext->fGraphicsState.fCurPosY;
    double x2 = pdfContext->fGraphicsState.fCurPosX;
    double y1 = pdfContext->fObjectStack.top()->asNumber()->value();    pdfContext->fObjectStack.pop();
    double x1 = pdfContext->fObjectStack.top()->asNumber()->value();    pdfContext->fObjectStack.pop();

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

    double height = pdfContext->fObjectStack.top()->asNumber()->value();      pdfContext->fObjectStack.pop();
    double width = pdfContext->fObjectStack.top()->asNumber()->value();       pdfContext->fObjectStack.pop();
    double y = pdfContext->fObjectStack.top()->asNumber()->value();           pdfContext->fObjectStack.pop();
    double x = pdfContext->fObjectStack.top()->asNumber()->value();           pdfContext->fObjectStack.pop();

    pdfContext->fGraphicsState.fPath.addRect(SkDoubleToScalar(x), SkDoubleToScalar(y),
                                           SkDoubleToScalar(x + width), SkDoubleToScalar(y + height));

    pdfContext->fGraphicsState.fCurPosX = x;
    pdfContext->fGraphicsState.fCurPosY = y + height;

    return kOK_PdfResult;
}

PdfResult PdfOp_h(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fGraphicsState.fPath.close();
    return kOK_PdfResult;
}

PdfResult PdfOp_fillAndStroke(PdfContext* pdfContext, SkCanvas* canvas, bool fill, bool stroke, bool close, bool evenOdd) {
    SkPath path = pdfContext->fGraphicsState.fPath;

    if (close) {
        path.close();
    }

    canvas->setMatrix(pdfContext->fGraphicsState.fMatrix);

    SkPaint paint;

    SkPoint line[2];
    if (fill && !stroke && path.isLine(line)) {
        paint.setStyle(SkPaint::kStroke_Style);

        pdfContext->fGraphicsState.applyGraphicsState(&paint, false);
        paint.setStrokeWidth(SkDoubleToScalar(0));

        canvas->drawPath(path, paint);
    } else {
        if (fill) {
            paint.setStyle(SkPaint::kFill_Style);
            if (evenOdd) {
                path.setFillType(SkPath::kEvenOdd_FillType);
            }

            pdfContext->fGraphicsState.applyGraphicsState(&paint, false);

            canvas->drawPath(path, paint);
        }

        if (stroke) {
            paint.setStyle(SkPaint::kStroke_Style);

            pdfContext->fGraphicsState.applyGraphicsState(&paint, true);

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
    pdfContext->fGraphicsState.fCurFontSize = pdfContext->fObjectStack.top()->asNumber()->value();     pdfContext->fObjectStack.pop();
    std::string fontName = pdfContext->fObjectStack.top()->asName()->value();                           pdfContext->fObjectStack.pop();

#ifdef PDF_TRACE
    printf("font name: %s\n", fontName.c_str());
    std::string str;
    pdfContext->fGraphicsState.fResources->podofo()->ToString(str);
    printf("Print Tf Resources: %s\n", str.c_str());
    pdfContext->fGraphicsState.fResources->Font()->podofo()->ToString(str);
    printf("Print Tf Resources/Font: %s\n", str.c_str());
#endif

    SkPdfFontDictionary* fd = NULL;
    if (pdfContext->fGraphicsState.fResources->Font()) {
        SkPdfObject* objFont = pdfContext->fGraphicsState.fResources->Font()->get(fontName.c_str());
        mapFontDictionary(*objFont, &fd);

#ifdef PDF_TRACE
        objFont->podofo()->ToString(str);
        printf("Print Font loaded: %s\n", str.c_str());
        fd->podofo()->ToString(str);
        printf("Print Font loaded and resolved and upgraded: %s\n", str.c_str());
#endif

    }

    SkPdfFont* skfont = SkPdfFont::fontFromPdfDictionary(fd);

    if (skfont) {
        pdfContext->fGraphicsState.fSkFont = skfont;
    }

    return kPartial_PdfResult;
}

PdfResult PdfOp_Tj(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (!pdfContext->fGraphicsState.fTextBlock) {
        // TODO(edisonn): try to recover and draw it any way?
        return kIgnoreError_PdfResult;
    }

    PdfResult ret = DrawText(pdfContext,
                             pdfContext->fObjectStack.top(),
                             canvas);
    pdfContext->fObjectStack.pop();

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

    SkPdfObject* str = pdfContext->fObjectStack.top();       pdfContext->fObjectStack.pop();
    SkPdfObject* ac = pdfContext->fObjectStack.top();        pdfContext->fObjectStack.pop();
    SkPdfObject* aw = pdfContext->fObjectStack.top();        pdfContext->fObjectStack.pop();

    pdfContext->fObjectStack.push(aw);
    PdfOp_Tw(pdfContext, canvas, looper);

    pdfContext->fObjectStack.push(ac);
    PdfOp_Tc(pdfContext, canvas, looper);

    pdfContext->fObjectStack.push(str);
    PdfOp_quote(pdfContext, canvas, looper);

    return kPartial_PdfResult;
}

PdfResult PdfOp_TJ(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (!pdfContext->fGraphicsState.fTextBlock) {
        // TODO(edisonn): try to recover and draw it any way?
        return kIgnoreError_PdfResult;
    }

    SkPdfArray* array = pdfContext->fObjectStack.top()->asArray();
    pdfContext->fObjectStack.pop();

    for( int i=0; i<static_cast<int>(array->size()); i++ )
    {
        if( (*array)[i]->asString()) {
            SkPdfObject* obj = (*array)[i];
            DrawText(pdfContext,
                     obj,
                     canvas);
        } else if ((*array)[i]->asInteger() || (*array)[i]->asNumber()) {
            double dx = (*array)[i]->asNumber()->value();
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
    colorOperator->fColorSpace = pdfContext->fObjectStack.top()->asName()->value();    pdfContext->fObjectStack.pop();
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
            c[i] = pdfContext->fObjectStack.top()->asNumber()->value();     pdfContext->fObjectStack.pop();
        } else {
            v[i] = pdfContext->fObjectStack.top()->asInteger()->value();   pdfContext->fObjectStack.pop();
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

    if (pdfContext->fObjectStack.top()->asName()) {
        pdfContext->fObjectStack.pop();
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
    double gray = pdfContext->fObjectStack.top()->asNumber()->value();     pdfContext->fObjectStack.pop();
    return kNYI_PdfResult;
}

PdfResult PdfOp_G(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_G_g(pdfContext, canvas, &pdfContext->fGraphicsState.fStroking);
}

PdfResult PdfOp_g(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_G_g(pdfContext, canvas, &pdfContext->fGraphicsState.fNonStroking);
}

PdfResult PdfOp_RG_rg(PdfContext* pdfContext, SkCanvas* canvas, PdfColorOperator* colorOperator) {
    double b = pdfContext->fObjectStack.top()->asNumber()->value();     pdfContext->fObjectStack.pop();
    double g = pdfContext->fObjectStack.top()->asNumber()->value();     pdfContext->fObjectStack.pop();
    double r = pdfContext->fObjectStack.top()->asNumber()->value();     pdfContext->fObjectStack.pop();

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
    double k = pdfContext->fObjectStack.top()->asNumber()->value();     pdfContext->fObjectStack.pop();
    double y = pdfContext->fObjectStack.top()->asNumber()->value();     pdfContext->fObjectStack.pop();
    double m = pdfContext->fObjectStack.top()->asNumber()->value();     pdfContext->fObjectStack.pop();
    double c = pdfContext->fObjectStack.top()->asNumber()->value();     pdfContext->fObjectStack.pop();

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
    double lineWidth = pdfContext->fObjectStack.top()->asNumber()->value();     pdfContext->fObjectStack.pop();
    pdfContext->fGraphicsState.fLineWidth = lineWidth;

    return kOK_PdfResult;
}

//lineCap J Set the line cap style in the graphics state (see “Line Cap Style” on page 153).
PdfResult PdfOp_J(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fObjectStack.pop();
    //double lineCap = pdfContext->fObjectStack.top()->asNumber()->value();     pdfContext->fObjectStack.pop();

    return kNYI_PdfResult;
}

//lineJoin j Set the line join style in the graphics state (see “Line Join Style” on page 153).
PdfResult PdfOp_j(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fObjectStack.pop();
    //double lineJoin = pdfContext->fObjectStack.top()->asNumber()->value();     pdfContext->fObjectStack.pop();

    return kNYI_PdfResult;
}

//miterLimit M Set the miter limit in the graphics state (see “Miter Limit” on page 153).
PdfResult PdfOp_M(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fObjectStack.pop();
    //double miterLimit = pdfContext->fObjectStack.top()->asNumber()->value();     pdfContext->fObjectStack.pop();

    return kNYI_PdfResult;
}

//dashArray dashPhase d Set the line dash pattern in the graphics state (see “Line Dash Pattern” on
//page 155).
PdfResult PdfOp_d(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fObjectStack.pop();
    pdfContext->fObjectStack.pop();

    return kNYI_PdfResult;
}

//intent ri (PDF 1.1) Set the color rendering intent in the graphics state (see “Rendering Intents” on page 197).
PdfResult PdfOp_ri(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fObjectStack.pop();

    return kNYI_PdfResult;
}

//ﬂatness i Set the ﬂatness tolerance in the graphics state (see Section 6.5.1, “Flatness
//Tolerance”). ﬂatness is a number in the range 0 to 100; a value of 0 speci-
//ﬁes the output device’s default ﬂatness tolerance.
PdfResult PdfOp_i(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fObjectStack.pop();

    return kNYI_PdfResult;
}

//dictName gs (PDF 1.2) Set the speciﬁed parameters in the graphics state. dictName is
//the name of a graphics state parameter dictionary in the ExtGState subdictionary of the current resource dictionary (see the next section).
PdfResult PdfOp_gs(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    std::string name = pdfContext->fObjectStack.top()->asName()->value();    pdfContext->fObjectStack.pop();

#ifdef PDF_TRACE
    std::string str;
#endif

    //Next, get the ExtGState Dictionary from the Resource Dictionary:
    const SkPdfDictionary* extGStateDictionary = pdfContext->fGraphicsState.fResources->ExtGState();

    if (extGStateDictionary == NULL) {
#ifdef PDF_TRACE
        printf("ExtGState is NULL!\n");
#endif
        return kIgnoreError_PdfResult;
    }

    SkPdfObject* value = extGStateDictionary->get(name.c_str());

#ifdef PDF_TRACE
//    value->ToString(str);
//    printf("gs object value: %s\n", str.c_str());
#endif

    SkPdfGraphicsStateDictionary* gs = NULL;
    mapGraphicsStateDictionary(*value, &gs);

    // TODO(edisonn): now load all those properties in graphic state.
    if (gs == NULL) {
        return kIgnoreError_PdfResult;
    }

    if (gs->has_CA()) {
        pdfContext->fGraphicsState.fStroking.fOpacity = gs->CA();
    }

    if (gs->has_ca()) {
        pdfContext->fGraphicsState.fNonStroking.fOpacity = gs->ca();
    }

    if (gs->has_LW()) {
        pdfContext->fGraphicsState.fLineWidth = gs->LW();
    }



    return kNYI_PdfResult;
}

//charSpace Tc Set the character spacing, Tc
//, to charSpace, which is a number expressed in unscaled text space units. Character spacing is used by the Tj, TJ, and ' operators.
//Initial value: 0.
PdfResult PdfOp_Tc(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double charSpace = pdfContext->fObjectStack.top()->asNumber()->value();     pdfContext->fObjectStack.pop();
    pdfContext->fGraphicsState.fCharSpace = charSpace;

    return kOK_PdfResult;
}

//wordSpace Tw Set the word spacing, T
//w
//, to wordSpace, which is a number expressed in unscaled
//text space units. Word spacing is used by the Tj, TJ, and ' operators. Initial
//value: 0.
PdfResult PdfOp_Tw(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double wordSpace = pdfContext->fObjectStack.top()->asNumber()->value();     pdfContext->fObjectStack.pop();
    pdfContext->fGraphicsState.fWordSpace = wordSpace;

    return kOK_PdfResult;
}

//scale Tz Set the horizontal scaling, Th
//, to (scale ˜ 100). scale is a number specifying the
//percentage of the normal width. Initial value: 100 (normal width).
PdfResult PdfOp_Tz(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double scale = pdfContext->fObjectStack.top()->asNumber()->value();     pdfContext->fObjectStack.pop();

    return kNYI_PdfResult;
}

//render Tr Set the text rendering mode, T
//mode, to render, which is an integer. Initial value: 0.
PdfResult PdfOp_Tr(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double render = pdfContext->fObjectStack.top()->asNumber()->value();     pdfContext->fObjectStack.pop();

    return kNYI_PdfResult;
}
//rise Ts Set the text rise, Trise, to rise, which is a number expressed in unscaled text space
//units. Initial value: 0.
PdfResult PdfOp_Ts(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double rise = pdfContext->fObjectStack.top()->asNumber()->value();     pdfContext->fObjectStack.pop();

    return kNYI_PdfResult;
}

//wx wy d0
PdfResult PdfOp_d0(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fObjectStack.pop();
    pdfContext->fObjectStack.pop();

    return kNYI_PdfResult;
}

//wx wy llx lly urx ury d1
PdfResult PdfOp_d1(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fObjectStack.pop();
    pdfContext->fObjectStack.pop();
    pdfContext->fObjectStack.pop();
    pdfContext->fObjectStack.pop();
    pdfContext->fObjectStack.pop();
    pdfContext->fObjectStack.pop();

    return kNYI_PdfResult;
}

//name sh
PdfResult PdfOp_sh(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fObjectStack.pop();

    return kNYI_PdfResult;
}

//name Do
PdfResult PdfOp_Do(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    std::string name = pdfContext->fObjectStack.top()->asName()->value();    pdfContext->fObjectStack.pop();

    SkPdfDictionary* xObject =  pdfContext->fGraphicsState.fResources->XObject();

    if (xObject == NULL) {
#ifdef PDF_TRACE
        printf("XObject is NULL!\n");
#endif
        return kIgnoreError_PdfResult;
    }

    SkPdfObject* value = xObject->get(name.c_str());

#ifdef PDF_TRACE
//    value->ToString(str);
//    printf("Do object value: %s\n", str.c_str());
#endif

    return doXObject(pdfContext, canvas, *value);
}

//tag MP Designate a marked-content point. tag is a name object indicating the role or
//signiﬁcance of the point.
PdfResult PdfOp_MP(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fObjectStack.pop();

    return kNYI_PdfResult;
}

//tag properties DP Designate a marked-content point with an associated property list. tag is a
//name object indicating the role or signiﬁcance of the point; properties is
//either an inline dictionary containing the property list or a name object
//associated with it in the Properties subdictionary of the current resource
//dictionary (see Section 9.5.1, “Property Lists”).
PdfResult PdfOp_DP(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fObjectStack.pop();
    pdfContext->fObjectStack.pop();

    return kNYI_PdfResult;
}

//tag BMC Begin a marked-content sequence terminated by a balancing EMC operator.
//tag is a name object indicating the role or signiﬁcance of the sequence.
PdfResult PdfOp_BMC(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fObjectStack.pop();

    return kNYI_PdfResult;
}

//tag properties BDC Begin a marked-content sequence with an associated property list, terminated
//by a balancing EMCoperator. tag is a name object indicating the role or significance of the sequence; propertiesis either an inline dictionary containing the
//property list or a name object associated with it in the Properties subdictionary of the current resource dictionary (see Section 9.5.1, “Property Lists”).
PdfResult PdfOp_BDC(PdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fObjectStack.pop();
    pdfContext->fObjectStack.pop();

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

class InitPdfOps {
public:
    InitPdfOps() {
        initPdfOperatorRenderes();
    }
};

InitPdfOps gInitPdfOps;

void reportPdfRenderStats() {
    std::map<std::string, int>::iterator iter;

    for (int i = 0 ; i < kCount_PdfResult; i++) {
        for (iter = gRenderStats[i].begin(); iter != gRenderStats[i].end(); ++iter) {
            printf("%s: %s -> count %i\n", gRenderStatsNames[i], iter->first.c_str(), iter->second);
        }
    }
}

PdfResult PdfMainLooper::consumeToken(PdfToken& token) {
    if (token.fType == kKeyword_TokenType)
    {
        // TODO(edisonn): log trace flag (verbose, error, info, warning, ...)
        PdfOperatorRenderer pdfOperatorRenderer = gPdfOps[token.fKeyword];
        if (pdfOperatorRenderer) {
            // caller, main work is done by pdfOperatorRenderer(...)
            PdfTokenLooper* childLooper = NULL;
            gRenderStats[pdfOperatorRenderer(fPdfContext, fCanvas, &childLooper)][token.fKeyword]++;

            if (childLooper) {
                childLooper->setUp(this);
                childLooper->loop();
                delete childLooper;
            }
        } else {
            gRenderStats[kUnsupported_PdfResult][token.fKeyword]++;
        }
    }
    else if (token.fType == kObject_TokenType)
    {
        fPdfContext->fObjectStack.push( token.fObject );
    }
    else if ( token.fType == kImageData_TokenType) {
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
        if (token.fType == kKeyword_TokenType && strcmp(token.fKeyword, "BX") == 0) {
            PdfTokenLooper* looper = new PdfCompatibilitySectionLooper();
            looper->setUp(this);
            looper->loop();
        } else {
            if (token.fType == kKeyword_TokenType && strcmp(token.fKeyword, "EI") == 0) {
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
        if (token.fType == kKeyword_TokenType && strcmp(token.fKeyword, "BX") == 0) {
            PdfTokenLooper* looper = new PdfCompatibilitySectionLooper();
            looper->setUp(this);
            looper->loop();
            delete looper;
        } else {
            if (token.fType == kKeyword_TokenType && strcmp(token.fKeyword, "EX") == 0) break;
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

bool SkPdfViewer::load(const SkString inputFileName, SkPicture* out) {
    try
    {
        std::cout << "Init: " << inputFileName.c_str() << std::endl;

        SkPdfDoc doc(inputFileName.c_str());
        if (!doc.pages())
        {
            std::cout << "ERROR: Empty Document" << inputFileName.c_str() << std::endl;
            return false;
        } else {

            for (int pn = 0; pn < doc.pages(); ++pn) {
                // TODO(edisonn): implement inheritance properties as per PDF spec
                //SkRect rect = page->MediaBox();
                SkRect rect = doc.MediaBox(pn);

    #ifdef PDF_TRACE
                printf("Page Width: %f, Page Height: %f\n", SkScalarToDouble(rect.width()), SkScalarToDouble(rect.height()));
    #endif

                // TODO(edisonn): page->GetCropBox(), page->GetTrimBox() ... how to use?

                SkBitmap bitmap;
    #ifdef PDF_DEBUG_3X
                setup_bitmap(&bitmap, 3 * (int)SkScalarToDouble(rect.width()), 3 * (int)SkScalarToDouble(rect.height()));
    #else
                setup_bitmap(&bitmap, (int)SkScalarToDouble(rect.width()), (int)SkScalarToDouble(rect.height()));
    #endif
                SkAutoTUnref<SkDevice> device(SkNEW_ARGS(SkDevice, (bitmap)));
                SkCanvas canvas(device);

                gDumpBitmap = &bitmap;

                doc.drawPage(pn, &canvas);

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


