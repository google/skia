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
#include "SkTDict.h"

#include "SkPdfGraphicsState.h"
#include "SkPdfNativeTokenizer.h"
#include <cstdio>
#include <stack>
#include <set>

extern "C" SkPdfContext* gPdfContext;
extern "C" SkBitmap* gDumpBitmap;
extern "C" SkCanvas* gDumpCanvas;

__SK_FORCE_IMAGE_DECODER_LINKING;

// TODO(edisonn): tool, show what objects were read at least, show the ones not even read
// keep for each object pos in file
// plug in for VS? syntax coloring, show selected object ... from the text, or from rendered x,y

// TODO(edisonn): security - validate all the user input, all pdf!

// TODO(edisonn): put drawtext in #ifdefs, so comparations will ignore minor changes in text positioning and font
// this way, we look more at other features and layout in diffs

// TODO(edisonn): move trace dump in the get functions, and mapper ones too so it ghappens automatically
/*
#ifdef PDF_TRACE
    std::string str;
    pdfContext->fGraphicsState.fResources->native()->ToString(str);
    printf("Print Tf Resources: %s\n", str.c_str());
#endif
 */

#include "SkPdfHeaders_autogen.h"
#include "SkPdfMapper_autogen.h"
#include "SkPdfRenderer.h"

#include "SkPdfUtils.h"

#include "SkPdfFont.h"

/*
 * TODO(edisonn):
 * - all font types and all ppdf font features
 *      - word spacing
 *      - load font for baidu.pdf
 *      - load font for youtube.pdf
 *      - parser for pdf from the definition already available in pdfspec_autogen.py
 *      - all docs from ~/work
 * - encapsulate native in the pdf api so the skpdf does not know anything about native ... in progress
 * - load gs/ especially smask and already known prop (skp) ... in progress
 * - wrapper on classes for customizations? e.g.
 * SkPdfPageObjectVanila - has only the basic loaders/getters
 * SkPdfPageObject : public SkPdfPageObjectVanila, extends, and I can add customizations here
 * need to find a nice object model for all this with constructors and factories
 * - deal with inheritable automatically ?
 * - deal with specific type in spec directly, add all dictionary types to known types
*/

using namespace std;

NotOwnedString strings_DeviceRGB;
NotOwnedString strings_DeviceCMYK;

class StringsInit {
public:
    StringsInit() {
        NotOwnedString::init(&strings_DeviceRGB, "DeviceRGB");
        NotOwnedString::init(&strings_DeviceCMYK, "DeviceCMYK");
    }
};

StringsInit gStringsInit;

// TODO(edisonn): Document PdfTokenLooper and subclasses.
class PdfTokenLooper {
protected:
    PdfTokenLooper* fParent;
    SkPdfNativeTokenizer* fTokenizer;
    SkPdfContext* fPdfContext;
    SkCanvas* fCanvas;

public:
    PdfTokenLooper(PdfTokenLooper* parent,
                   SkPdfNativeTokenizer* tokenizer,
                   SkPdfContext* pdfContext,
                   SkCanvas* canvas)
        : fParent(parent), fTokenizer(tokenizer), fPdfContext(pdfContext), fCanvas(canvas) {}

    virtual ~PdfTokenLooper() {}

    virtual SkPdfResult consumeToken(PdfToken& token) = 0;
    virtual void loop() = 0;

    void setUp(PdfTokenLooper* parent) {
        fParent = parent;
        fTokenizer = parent->fTokenizer;
        fPdfContext = parent->fPdfContext;
        fCanvas = parent->fCanvas;
    }

    SkPdfNativeTokenizer* tokenizer() { return fTokenizer; }
};

class PdfMainLooper : public PdfTokenLooper {
public:
    PdfMainLooper(PdfTokenLooper* parent,
                  SkPdfNativeTokenizer* tokenizer,
                  SkPdfContext* pdfContext,
                  SkCanvas* canvas)
        : PdfTokenLooper(parent, tokenizer, pdfContext, canvas) {}

    virtual SkPdfResult consumeToken(PdfToken& token);
    virtual void loop();
};

class PdfInlineImageLooper : public PdfTokenLooper {
public:
    PdfInlineImageLooper()
        : PdfTokenLooper(NULL, NULL, NULL, NULL) {}

    virtual SkPdfResult consumeToken(PdfToken& token);
    virtual void loop();
    SkPdfResult done();
};

class PdfCompatibilitySectionLooper : public PdfTokenLooper {
public:
    PdfCompatibilitySectionLooper()
        : PdfTokenLooper(NULL, NULL, NULL, NULL) {}

    virtual SkPdfResult consumeToken(PdfToken& token);
    virtual void loop();
};

// Utilities
static void setup_bitmap(SkBitmap* bitmap, int width, int height, SkColor color = SK_ColorWHITE) {
    bitmap->setConfig(SkBitmap::kARGB_8888_Config, width, height);

    bitmap->allocPixels();
    bitmap->eraseColor(color);
}

// TODO(edisonn): synonyms? DeviceRGB and RGB ...
static int GetColorSpaceComponents(NotOwnedString& colorSpace) {
    if (colorSpace.equals("DeviceCMYK")) {
        return 4;
    } else if (colorSpace.equals("DeviceGray") ||
            colorSpace.equals("CalGray") ||
            colorSpace.equals("Indexed")) {
        return 1;
    } else if (colorSpace.equals("DeviceRGB") ||
            colorSpace.equals("CalRGB") ||
            colorSpace.equals("Lab")) {
        return 3;
    } else {
        return 0;
    }
}

SkMatrix SkMatrixFromPdfMatrix(double array[6]) {
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
        const SkPdfNativeObject* elem = pdfArray->operator [](i);
        if (elem == NULL || !elem->isNumber()) {
            return SkMatrix::I();  // TODO(edisonn): report issue
        }
        array[i] = elem->numberValue();
    }

    return SkMatrixFromPdfMatrix(array);
}


extern "C" SkPdfNativeDoc* gDoc;
SkBitmap* gDumpBitmap = NULL;
SkCanvas* gDumpCanvas = NULL;
char gLastKeyword[100] = "";
int gLastOpKeyword = -1;
char allOpWithVisualEffects[100] = ",S,s,f,F,f*,B,B*,b,b*,n,Tj,TJ,\',\",d0,d1,sh,EI,Do,EX,";
int gReadOp = 0;


#ifdef PDF_TRACE_DIFF_IN_PNG
static bool hasVisualEffect(const char* pdfOp) {
    return true;
    if (*pdfOp == '\0') return false;

    char markedPdfOp[100] = ",";
    strcat(markedPdfOp, pdfOp);
    strcat(markedPdfOp, ",");

    return (strstr(allOpWithVisualEffects, markedPdfOp) != NULL);
}
#endif  // PDF_TRACE_DIFF_IN_PNG



// TODO(edisonn): Pass SkPdfContext and SkCanvasd only with the define for instrumentation.
static bool readToken(SkPdfNativeTokenizer* fTokenizer, PdfToken* token) {
    bool ret = fTokenizer->readToken(token);

    gReadOp++;
    gLastOpKeyword++;
#ifdef PDF_TRACE_DIFF_IN_PNG
    // TODO(edisonn): compare with old bitmap, and save only new bits are available, and save
    // the numbar and name of last operation, so the file name will reflect op that changed.
    if (gLastKeyword[0] && hasVisualEffect(gLastKeyword)) {  // TODO(edisonn): and has dirty bits.
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
            while ((elem = iter.next()) != NULL) {
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

    if (ret && token->fType == kKeyword_TokenType && token->fKeyword && token->fKeywordLength > 0 && token->fKeywordLength < 100) {
        strncpy(gLastKeyword, token->fKeyword, token->fKeywordLength);
        gLastKeyword[token->fKeywordLength] = '\0';
    } else {
        gLastKeyword[0] = '\0';
    }

#endif

    return ret;
}



typedef SkPdfResult (*PdfOperatorRenderer)(SkPdfContext*, SkCanvas*, PdfTokenLooper**);

SkTDict<PdfOperatorRenderer> gPdfOps(100);


template <typename T> class SkTDictWithDefaultConstructor : public SkTDict<T> {
public:
    SkTDictWithDefaultConstructor() : SkTDict<T>(10) {}
};

SkTDictWithDefaultConstructor<int> gRenderStats[kCount_SkPdfResult];

const char* gRenderStatsNames[kCount_SkPdfResult] = {
    "Success",
    "Partially implemented",
    "Not yet implemented",
    "Ignore Error",
    "Error",
    "Unsupported/Unknown"
};

static SkPdfResult DrawText(SkPdfContext* pdfContext,
                   const SkPdfNativeObject* _str,
                   SkCanvas* canvas)
{

    SkPdfFont* skfont = pdfContext->fGraphicsState.fSkFont;
    if (skfont == NULL) {
        skfont = SkPdfFont::Default();
    }


    if (_str == NULL || !_str->isAnyString()) {
        // TODO(edisonn): report warning
        return kIgnoreError_SkPdfResult;
    }
    const SkPdfString* str = (const SkPdfString*)_str;

    SkUnencodedText binary(str);

    SkDecodedText decoded;

    if (skfont->encoding() == NULL) {
        // TODO(edisonn): report warning
        return kNYI_SkPdfResult;
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

    skfont->drawText(decoded, &paint, pdfContext, canvas);

    return kOK_SkPdfResult;
}

// TODO(edisonn): create header files with declarations!
SkPdfResult PdfOp_q(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper);
SkPdfResult PdfOp_Q(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper);
SkPdfResult PdfOp_Tw(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper);
SkPdfResult PdfOp_Tc(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper);

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

static SkBitmap* transferImageStreamToBitmap(const unsigned char* uncompressedStream, size_t uncompressedStreamLength,
                                     int width, int height, int bytesPerLine,
                                     int bpc, const std::string& colorSpace,
                                     bool transparencyMask) {
    SkBitmap* bitmap = new SkBitmap();

    //int components = GetColorSpaceComponents(colorSpace);
//#define MAX_COMPONENTS 10

    // TODO(edisonn): assume start of lines are aligned at 32 bits?
    // Is there a faster way to load the uncompressed stream into a bitmap?

    // minimal support for now
    if ((colorSpace == "DeviceRGB" || colorSpace == "RGB") && bpc == 8) {
        SkColor* uncompressedStreamArgb = (SkColor*)malloc(width * height * sizeof(SkColor));

        for (int h = 0 ; h < height; h++) {
            long i = width * (h);
            for (int w = 0 ; w < width; w++) {
                uncompressedStreamArgb[i] = SkColorSetRGB(uncompressedStream[3 * w],
                                                          uncompressedStream[3 * w + 1],
                                                          uncompressedStream[3 * w + 2]);
                i++;
            }
            uncompressedStream += bytesPerLine;
        }

        bitmap->setConfig(SkBitmap::kARGB_8888_Config, width, height);
        bitmap->setPixels(uncompressedStreamArgb);
    }
    else if ((colorSpace == "DeviceGray" || colorSpace == "Gray") && bpc == 8) {
        unsigned char* uncompressedStreamA8 = (unsigned char*)malloc(width * height);

        for (int h = 0 ; h < height; h++) {
            long i = width * (h);
            for (int w = 0 ; w < width; w++) {
                uncompressedStreamA8[i] = transparencyMask ? 255 - uncompressedStream[w] :
                                                             uncompressedStream[w];
                i++;
            }
            uncompressedStream += bytesPerLine;
        }

        bitmap->setConfig(transparencyMask ? SkBitmap::kA8_Config : SkBitmap::kIndex8_Config,
                         width, height);
        bitmap->setPixels(uncompressedStreamA8, transparencyMask ? NULL : getGrayColortable());
    }

    // TODO(edisonn): Report Warning, NYI, or error
    return bitmap;
}

// utils

// TODO(edisonn): add cache, or put the bitmap property directly on the PdfObject
// TODO(edisonn): deal with colorSpaces, we could add them to SkBitmap::Config
// TODO(edisonn): preserve A1 format that skia knows, + fast convert from 111, 222, 444 to closest
// skia format, through a table

// this functions returns the image, it does not look at the smask.

static SkBitmap* getImageFromObjectCore(SkPdfContext* pdfContext, SkPdfImageDictionary* image, bool transparencyMask) {
    if (image == NULL || !image->hasStream()) {
        // TODO(edisonn): report warning to be used in testing.
        return NULL;
    }

    int64_t bpc = image->BitsPerComponent(pdfContext->fPdfDoc);
    int64_t width = image->Width(pdfContext->fPdfDoc);
    int64_t height = image->Height(pdfContext->fPdfDoc);
    std::string colorSpace = "DeviceRGB";

    bool indexed = false;
    SkPMColor colors[256];
    int cnt = 0;

    // TODO(edisonn): color space can be an array too!
    if (image->isColorSpaceAName(pdfContext->fPdfDoc)) {
        colorSpace = image->getColorSpaceAsName(pdfContext->fPdfDoc);
    } else if (image->isColorSpaceAArray(pdfContext->fPdfDoc)) {
        SkPdfArray* array = image->getColorSpaceAsArray(pdfContext->fPdfDoc);
        if (array && array->size() == 4 && array->objAtAIndex(0)->isName("Indexed") &&
                                           (array->objAtAIndex(1)->isName("DeviceRGB") || array->objAtAIndex(1)->isName("RGB")) &&
                                           array->objAtAIndex(2)->isInteger() &&
                                           array->objAtAIndex(3)->isHexString()
                                           ) {
            // TODO(edisonn): suport only DeviceRGB for now.
            indexed = true;
            cnt = array->objAtAIndex(2)->intValue() + 1;
            if (cnt > 256) {
                // TODO(edionn): report NYIs
                return NULL;
            }
            SkColorTable colorTable(cnt);
            NotOwnedString data = array->objAtAIndex(3)->strRef();
            if (data.fBytes != (unsigned int)cnt * 3) {
                // TODO(edionn): report error/warning
                return NULL;
            }
            for (int i = 0 ; i < cnt; i++) {
                colors[i] = SkPreMultiplyARGB(0xff, data.fBuffer[3 * i], data.fBuffer[3 * i + 1], data.fBuffer[3 * i + 2]);
            }
        }
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

    const unsigned char* uncompressedStream = NULL;
    size_t uncompressedStreamLength = 0;

    SkPdfStream* stream = (SkPdfStream*)image;

    if (!stream || !stream->GetFilteredStreamRef(&uncompressedStream, &uncompressedStreamLength) ||
            uncompressedStream == NULL || uncompressedStreamLength == 0) {
        // TODO(edisonn): report warning to be used in testing.
        return NULL;
    }

    SkPdfStreamCommonDictionary* streamDict = (SkPdfStreamCommonDictionary*)stream;

    if (streamDict->has_Filter() && ((streamDict->isFilterAName(NULL) &&
                                          streamDict->getFilterAsName(NULL) == "DCTDecode") ||
                                     (streamDict->isFilterAArray(NULL) &&
                                          streamDict->getFilterAsArray(NULL)->size() > 0 &&
                                          streamDict->getFilterAsArray(NULL)->objAtAIndex(0)->isName() &&
                                          streamDict->getFilterAsArray(NULL)->objAtAIndex(0)->nameValue2() == "DCTDecode"))) {
        SkBitmap* bitmap = new SkBitmap();
        SkImageDecoder::DecodeMemory(uncompressedStream, uncompressedStreamLength, bitmap);
        return bitmap;
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

    // TODO(edisonn): assumes RGB for now, since it is the only onwe implemented
    if (indexed) {
        SkBitmap* bitmap = new SkBitmap();
        bitmap->setConfig(SkBitmap::kIndex8_Config, width, height);
        SkColorTable* colorTable = new SkColorTable(colors, cnt);
        bitmap->setPixels((void*)uncompressedStream, colorTable);
        return bitmap;
    }

    int bytesPerLine = (int)(uncompressedStreamLength / height);
#ifdef PDF_TRACE
    if (uncompressedStreamLength % height != 0) {
        printf("Warning uncompressedStreamLength modulo height != 0 !!!\n");
    }
#endif

    SkBitmap* bitmap = transferImageStreamToBitmap(
            (unsigned char*)uncompressedStream, uncompressedStreamLength,
            (int)width, (int)height, bytesPerLine,
            (int)bpc, colorSpace,
            transparencyMask);

    return bitmap;
}

static SkBitmap* getImageFromObject(SkPdfContext* pdfContext, SkPdfImageDictionary* image, bool transparencyMask) {
    if (!transparencyMask) {
        if (!image->hasData(SkPdfNativeObject::kBitmap_Data)) {
            SkBitmap* bitmap = getImageFromObjectCore(pdfContext, image, transparencyMask);
            image->setData(bitmap, SkPdfNativeObject::kBitmap_Data);
        }
        return (SkBitmap*) image->data(SkPdfNativeObject::kBitmap_Data);
    } else {
        return getImageFromObjectCore(pdfContext, image, transparencyMask);
    }
}

static SkBitmap* getSmaskFromObject(SkPdfContext* pdfContext, SkPdfImageDictionary* obj) {
    SkPdfImageDictionary* sMask = obj->SMask(pdfContext->fPdfDoc);

    if (sMask) {
        return getImageFromObject(pdfContext, sMask, true);
    }

    // TODO(edisonn): implement GS SMask. Default to empty right now.
    return pdfContext->fGraphicsState.fSMask;
}

static SkPdfResult doXObject_Image(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfImageDictionary* skpdfimage) {
    if (skpdfimage == NULL) {
        return kIgnoreError_SkPdfResult;
    }

    SkBitmap* image = getImageFromObject(pdfContext, skpdfimage, false);
    SkBitmap* sMask = getSmaskFromObject(pdfContext, skpdfimage);

    canvas->save();
    canvas->setMatrix(pdfContext->fGraphicsState.fCTM);

    SkScalar z = SkIntToScalar(0);
    SkScalar one = SkIntToScalar(1);

    SkPoint from[4] = {SkPoint::Make(z, z), SkPoint::Make(one, z), SkPoint::Make(one, one), SkPoint::Make(z, one)};
    SkPoint to[4] = {SkPoint::Make(z, one), SkPoint::Make(one, one), SkPoint::Make(one, z), SkPoint::Make(z, z)};
    SkMatrix flip;
    SkAssertResult(flip.setPolyToPoly(from, to, 4));
    SkMatrix solveImageFlip = pdfContext->fGraphicsState.fCTM;
    solveImageFlip.preConcat(flip);
    canvas->setMatrix(solveImageFlip);

#ifdef PDF_TRACE
    SkPoint final[4] = {SkPoint::Make(z, z), SkPoint::Make(one, z), SkPoint::Make(one, one), SkPoint::Make(z, one)};
    solveImageFlip.mapPoints(final, 4);
    printf("IMAGE rect = ");
    for (int i = 0; i < 4; i++) {
        printf("(%f %f) ", SkScalarToDouble(final[i].x()), SkScalarToDouble(final[i].y()));
    }
    printf("\n");
#endif  // PDF_TRACE

    SkRect dst = SkRect::MakeXYWH(SkDoubleToScalar(0.0), SkDoubleToScalar(0.0), SkDoubleToScalar(1.0), SkDoubleToScalar(1.0));

    // TODO(edisonn): soft mask type? alpha/luminosity.
    SkPaint paint;
    pdfContext->fGraphicsState.applyGraphicsState(&paint, false);

    if (!sMask || sMask->empty()) {
        canvas->drawBitmapRect(*image, dst, &paint);
    } else {
        canvas->saveLayer(&dst, &paint);
        canvas->drawBitmapRect(*image, dst, NULL);
        SkPaint xfer;
        // TODO(edisonn): is the blend mode specified already implicitly/explicitly in pdf?
        xfer.setXfermodeMode(SkXfermode::kSrcOut_Mode); // SkXfermode::kSdtOut_Mode
        canvas->drawBitmapRect(*sMask, dst, &xfer);
        canvas->restore();
    }

    canvas->restore();

    return kPartial_SkPdfResult;
}

//TODO(edisonn): options for implementing isolation and knockout
// 1) emulate them (current solution)
//     PRO: simple
//     CON: will need to use readPixels, which means serious perf issues
// 2) Compile a plan for an array of matrixes, compose the result at the end
//     PRO: might be faster then 1, no need to readPixels
//     CON: multiple drawings (but on smaller areas), pay a price at loading pdf to compute a pdf draw plan
//          on average, a load with empty draw is 100ms on all the skps we have, for complete sites
// 3) support them natively in SkCanvas
//     PRO: simple
//     CON: we would still need to use a form of readPixels anyway, so perf might be the same as 1)
// 4) compile a plan using pathops, and render once without any fancy rules with backdrop
//     PRO: simple, fast
//     CON: pathops must be bug free first + time to compute new paths
//          pay a price at loading pdf to compute a pdf draw plan
//          on average, a load with empty draw is 100ms on all the skps we have, for complete sites
// 5) for knockout, render the objects in reverse order, and add every object to the clip, and any new draw will be cliped


// TODO(edisonn): draw plan from point! - list of draw ops of a point, like a tree!
// TODO(edisonn): Minimal PDF to draw some points - remove everything that it is not needed, save pdf uncompressed



static void doGroup_before(SkPdfContext* pdfContext, SkCanvas* canvas, SkRect bbox, SkPdfTransparencyGroupDictionary* tgroup, bool page) {
    SkRect bboxOrig = bbox;
    SkBitmap backdrop;
    bool isolatedGroup = tgroup->I(pdfContext->fPdfDoc);
//  bool knockoutGroup = tgroup->K(pdfContext->fPdfDoc);
    SkPaint paint;
    pdfContext->fGraphicsState.applyGraphicsState(&paint, false);
    canvas->saveLayer(&bboxOrig, isolatedGroup ? &paint : NULL);
}

// TODO(edisonn): non isolation implemented in skia
//static void doGroup_after(SkPdfContext* pdfContext, SkCanvas* canvas, SkRect bbox, SkPdfTransparencyGroupDictionary* tgroup) {
//    if not isolated
//        canvas->drawBitmapRect(backdrop, bboxOrig, NULL);
//}

static SkPdfResult doXObject_Form(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfType1FormDictionary* skobj) {
    if (!skobj || !skobj->hasStream()) {
        return kIgnoreError_SkPdfResult;
    }

    if (!skobj->has_BBox()) {
        return kIgnoreError_SkPdfResult;
    }

    PdfOp_q(pdfContext, canvas, NULL);


    if (skobj->Resources(pdfContext->fPdfDoc)) {
        pdfContext->fGraphicsState.fResources = skobj->Resources(pdfContext->fPdfDoc);
    }

    SkTraceMatrix(pdfContext->fGraphicsState.fCTM, "Current matrix");

    if (skobj->has_Matrix()) {
        pdfContext->fGraphicsState.fCTM.preConcat(skobj->Matrix(pdfContext->fPdfDoc));
        SkMatrix matrix = pdfContext->fGraphicsState.fCTM;
        matrix.preScale(SkDoubleToScalar(1), SkDoubleToScalar(-1));
        pdfContext->fGraphicsState.fMatrixTm = matrix;
        pdfContext->fGraphicsState.fMatrixTlm = matrix;
        // TODO(edisonn) reset matrixTm and matricTlm also?
    }

    SkTraceMatrix(pdfContext->fGraphicsState.fCTM, "Total matrix");
    pdfContext->fGraphicsState.fContentStreamMatrix = pdfContext->fGraphicsState.fCTM;

    canvas->setMatrix(pdfContext->fGraphicsState.fCTM);

    SkRect bbox = skobj->BBox(pdfContext->fPdfDoc);
    canvas->clipRect(bbox, SkRegion::kIntersect_Op, true);  // TODO(edisonn): AA from settings.

    // TODO(edisonn): iterate smart on the stream even if it is compressed, tokenize it as we go.
    // For this PdfContentsTokenizer needs to be extended.

    // This is a group?
    if (skobj->has_Group()) {
        SkPdfTransparencyGroupDictionary* tgroup = skobj->Group(pdfContext->fPdfDoc);
        doGroup_before(pdfContext, canvas, bbox, tgroup, false);
    }

    SkPdfStream* stream = (SkPdfStream*)skobj;

    SkPdfNativeTokenizer* tokenizer =
            pdfContext->fPdfDoc->tokenizerOfStream(stream, pdfContext->fTmpPageAllocator);
    if (tokenizer != NULL) {
        PdfMainLooper looper(NULL, tokenizer, pdfContext, canvas);
        looper.loop();
        delete tokenizer;
    }

    // TODO(edisonn): should we restore the variable stack at the same state?
    // There could be operands left, that could be consumed by a parent tokenizer when we pop.

    if (skobj->has_Group()) {
        canvas->restore();
    }

    PdfOp_Q(pdfContext, canvas, NULL);
    return kPartial_SkPdfResult;
}


// TODO(edisonn): Extract a class like ObjWithStream
static SkPdfResult doXObject_Pattern(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfType1PatternDictionary* skobj) {
    if (!skobj || !skobj->hasStream()) {
        return kIgnoreError_SkPdfResult;
    }

    if (!skobj->has_BBox()) {
        return kIgnoreError_SkPdfResult;
    }

    PdfOp_q(pdfContext, canvas, NULL);


    if (skobj->Resources(pdfContext->fPdfDoc)) {
        pdfContext->fGraphicsState.fResources = skobj->Resources(pdfContext->fPdfDoc);
    }

    SkTraceMatrix(pdfContext->fGraphicsState.fContentStreamMatrix, "Current Content stream matrix");

    if (skobj->has_Matrix()) {
        pdfContext->fGraphicsState.fContentStreamMatrix.preConcat(skobj->Matrix(pdfContext->fPdfDoc));
    }

    SkTraceMatrix(pdfContext->fGraphicsState.fContentStreamMatrix, "Total Content stream matrix");

    canvas->setMatrix(pdfContext->fGraphicsState.fContentStreamMatrix);
    pdfContext->fGraphicsState.fCTM = pdfContext->fGraphicsState.fContentStreamMatrix;

    SkRect bbox = skobj->BBox(pdfContext->fPdfDoc);
    canvas->clipRect(bbox, SkRegion::kIntersect_Op, true);  // TODO(edisonn): AA from settings.

    // TODO(edisonn): iterate smart on the stream even if it is compressed, tokenize it as we go.
    // For this PdfContentsTokenizer needs to be extended.

    SkPdfStream* stream = (SkPdfStream*)skobj;

    SkPdfNativeTokenizer* tokenizer =
            pdfContext->fPdfDoc->tokenizerOfStream(stream, pdfContext->fTmpPageAllocator);
    if (tokenizer != NULL) {
        PdfMainLooper looper(NULL, tokenizer, pdfContext, canvas);
        looper.loop();
        delete tokenizer;
    }

    // TODO(edisonn): should we restore the variable stack at the same state?
    // There could be operands left, that could be consumed by a parent tokenizer when we pop.

    PdfOp_Q(pdfContext, canvas, NULL);
    return kPartial_SkPdfResult;
}


//static SkPdfResult doXObject_PS(SkPdfContext* pdfContext, SkCanvas* canvas, const SkPdfNativeObject* obj) {
//    return kNYI_SkPdfResult;
//}

SkPdfResult doType3Char(SkPdfContext* pdfContext, SkCanvas* canvas, const SkPdfNativeObject* skobj, SkRect bBox, SkMatrix matrix, double textSize) {
    if (!skobj || !skobj->hasStream()) {
        return kIgnoreError_SkPdfResult;
    }

    PdfOp_q(pdfContext, canvas, NULL);

    pdfContext->fGraphicsState.fMatrixTm.preConcat(matrix);
    pdfContext->fGraphicsState.fMatrixTm.preScale(SkDoubleToScalar(textSize), SkDoubleToScalar(textSize));
    pdfContext->fGraphicsState.fMatrixTlm = pdfContext->fGraphicsState.fMatrixTm;

    pdfContext->fGraphicsState.fCTM = pdfContext->fGraphicsState.fMatrixTm;
    pdfContext->fGraphicsState.fCTM.preScale(SkDoubleToScalar(1), SkDoubleToScalar(-1));

    SkTraceMatrix(pdfContext->fGraphicsState.fCTM, "Total matrix");

    canvas->setMatrix(pdfContext->fGraphicsState.fCTM);

    SkRect rm = bBox;
    pdfContext->fGraphicsState.fCTM.mapRect(&rm);

    SkTraceRect(rm, "bbox mapped");

    canvas->clipRect(bBox, SkRegion::kIntersect_Op, true);  // TODO(edisonn): AA from settings.

    // TODO(edisonn): iterate smart on the stream even if it is compressed, tokenize it as we go.
    // For this PdfContentsTokenizer needs to be extended.

    SkPdfStream* stream = (SkPdfStream*)skobj;

    SkPdfNativeTokenizer* tokenizer =
            pdfContext->fPdfDoc->tokenizerOfStream(stream, pdfContext->fTmpPageAllocator);
    if (tokenizer != NULL) {
        PdfMainLooper looper(NULL, tokenizer, pdfContext, canvas);
        looper.loop();
        delete tokenizer;
    }

    // TODO(edisonn): should we restore the variable stack at the same state?
    // There could be operands left, that could be consumed by a parent tokenizer when we pop.
    PdfOp_Q(pdfContext, canvas, NULL);

    return kPartial_SkPdfResult;
}


// TODO(edisonn): make sure the pointer is unique
std::set<const SkPdfNativeObject*> gInRendering;

class CheckRecursiveRendering {
    const SkPdfNativeObject* fUniqueData;
public:
    CheckRecursiveRendering(const SkPdfNativeObject* obj) : fUniqueData(obj) {
        gInRendering.insert(obj);
    }

    ~CheckRecursiveRendering() {
        //SkASSERT(fObj.fInRendering);
        gInRendering.erase(fUniqueData);
    }

    static bool IsInRendering(const SkPdfNativeObject* obj) {
        return gInRendering.find(obj) != gInRendering.end();
    }
};

static SkPdfResult doXObject(SkPdfContext* pdfContext, SkCanvas* canvas, const SkPdfNativeObject* obj) {
    if (CheckRecursiveRendering::IsInRendering(obj)) {
        // Oops, corrupt PDF!
        return kIgnoreError_SkPdfResult;
    }

    CheckRecursiveRendering checkRecursion(obj);

    switch (pdfContext->fPdfDoc->mapper()->mapXObjectDictionary(obj))
    {
        case kImageDictionary_SkPdfNativeObjectType:
            return doXObject_Image(pdfContext, canvas, (SkPdfImageDictionary*)obj);
        case kType1FormDictionary_SkPdfNativeObjectType:
            return doXObject_Form(pdfContext, canvas, (SkPdfType1FormDictionary*)obj);
        //case kObjectDictionaryXObjectPS_SkPdfNativeObjectType:
            //return doXObject_PS(skxobj.asPS());
        default: {
            if (pdfContext->fPdfDoc->mapper()->mapType1PatternDictionary(obj) != kNone_SkPdfNativeObjectType) {
                SkPdfType1PatternDictionary* pattern = (SkPdfType1PatternDictionary*)obj;
                return doXObject_Pattern(pdfContext, canvas, pattern);
            }
        }
    }
    return kIgnoreError_SkPdfResult;
}

static SkPdfResult doPage(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfPageObjectDictionary* skobj) {
    if (!skobj) {
        return kIgnoreError_SkPdfResult;
    }

    if (!skobj->isContentsAStream(pdfContext->fPdfDoc)) {
        return kNYI_SkPdfResult;
    }

    SkPdfStream* stream = skobj->getContentsAsStream(pdfContext->fPdfDoc);

    if (!stream) {
        return kIgnoreError_SkPdfResult;
    }

    if (CheckRecursiveRendering::IsInRendering(skobj)) {
        // Oops, corrupt PDF!
        return kIgnoreError_SkPdfResult;
    }
    CheckRecursiveRendering checkRecursion(skobj);


    PdfOp_q(pdfContext, canvas, NULL);


    if (skobj->Resources(pdfContext->fPdfDoc)) {
        pdfContext->fGraphicsState.fResources = skobj->Resources(pdfContext->fPdfDoc);
    }

    // TODO(edisonn): MediaBox can be inherited!!!!
    SkRect bbox = skobj->MediaBox(pdfContext->fPdfDoc);
    if (skobj->has_Group()) {
        SkPdfTransparencyGroupDictionary* tgroup = skobj->Group(pdfContext->fPdfDoc);
        doGroup_before(pdfContext, canvas, bbox, tgroup, true);
    } else {
        canvas->save();
    }


    SkPdfNativeTokenizer* tokenizer =
            pdfContext->fPdfDoc->tokenizerOfStream(stream, pdfContext->fTmpPageAllocator);
    if (tokenizer != NULL) {
        PdfMainLooper looper(NULL, tokenizer, pdfContext, canvas);
        looper.loop();
        delete tokenizer;
    }

    // TODO(edisonn): should we restore the variable stack at the same state?
    // There could be operands left, that could be consumed by a parent tokenizer when we pop.
    canvas->restore();
    PdfOp_Q(pdfContext, canvas, NULL);
    return kPartial_SkPdfResult;
}

SkPdfResult PdfOp_q(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fStateStack.push(pdfContext->fGraphicsState);
    canvas->save();
    return kOK_SkPdfResult;
}

SkPdfResult PdfOp_Q(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fGraphicsState = pdfContext->fStateStack.top();
    pdfContext->fStateStack.pop();
    canvas->restore();
    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_cm(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double array[6];
    for (int i = 0 ; i < 6 ; i++) {
        array[5 - i] = pdfContext->fObjectStack.top()->numberValue();
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

    pdfContext->fGraphicsState.fCTM.preConcat(matrix);

#ifdef PDF_TRACE
    printf("cm ");
    for (int i = 0 ; i < 6 ; i++) {
        printf("%f ", array[i]);
    }
    printf("\n");
    SkTraceMatrix(pdfContext->fGraphicsState.fCTM, "cm");
#endif

    return kOK_SkPdfResult;
}

//leading TL Set the text leading, Tl
//, to leading, which is a number expressed in unscaled text
//space units. Text leading is used only by the T*, ', and " operators. Initial value: 0.
static SkPdfResult PdfOp_TL(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double ty = pdfContext->fObjectStack.top()->numberValue();   pdfContext->fObjectStack.pop();

    pdfContext->fGraphicsState.fTextLeading = ty;

    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_Td(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
#ifdef PDF_TRACE
    printf("stack size = %i\n", (int)pdfContext->fObjectStack.size());
#endif
    double ty = pdfContext->fObjectStack.top()->numberValue(); pdfContext->fObjectStack.pop();
    SkPdfNativeObject* obj = pdfContext->fObjectStack.top();
    obj = obj;
    double tx = pdfContext->fObjectStack.top()->numberValue(); pdfContext->fObjectStack.pop();

    double array[6] = {1, 0, 0, 1, tx, -ty};
    SkMatrix matrix = SkMatrixFromPdfMatrix(array);

    pdfContext->fGraphicsState.fMatrixTm.preConcat(matrix);
    pdfContext->fGraphicsState.fMatrixTlm.preConcat(matrix);

    return kPartial_SkPdfResult;
}

static SkPdfResult PdfOp_TD(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double ty = pdfContext->fObjectStack.top()->numberValue(); pdfContext->fObjectStack.pop();
    double tx = pdfContext->fObjectStack.top()->numberValue(); pdfContext->fObjectStack.pop();

    // TODO(edisonn): Create factory methods or constructors so native is hidden
    SkPdfReal* _ty = pdfContext->fPdfDoc->createReal(-ty);
    pdfContext->fObjectStack.push(_ty);

    PdfOp_TL(pdfContext, canvas, looper);

    SkPdfReal* vtx = pdfContext->fPdfDoc->createReal(tx);
    pdfContext->fObjectStack.push(vtx);

    SkPdfReal* vty = pdfContext->fPdfDoc->createReal(ty);
    pdfContext->fObjectStack.push(vty);

    SkPdfResult ret = PdfOp_Td(pdfContext, canvas, looper);

    // TODO(edisonn): delete all the objects after rendering was complete, in this way pdf is rendered faster
    // and the cleanup can happen while the user looks at the image

    return ret;
}

static SkPdfResult PdfOp_Tm(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double f = pdfContext->fObjectStack.top()->numberValue(); pdfContext->fObjectStack.pop();
    double e = pdfContext->fObjectStack.top()->numberValue(); pdfContext->fObjectStack.pop();
    double d = pdfContext->fObjectStack.top()->numberValue(); pdfContext->fObjectStack.pop();
    double c = pdfContext->fObjectStack.top()->numberValue(); pdfContext->fObjectStack.pop();
    double b = pdfContext->fObjectStack.top()->numberValue(); pdfContext->fObjectStack.pop();
    double a = pdfContext->fObjectStack.top()->numberValue(); pdfContext->fObjectStack.pop();

    double array[6];
    array[0] = a;
    array[1] = b;
    array[2] = c;
    array[3] = d;
    array[4] = e;
    array[5] = f;

    SkMatrix matrix = SkMatrixFromPdfMatrix(array);
    matrix.postConcat(pdfContext->fGraphicsState.fCTM);
    matrix.preScale(SkDoubleToScalar(1), SkDoubleToScalar(-1));

    // TODO(edisonn): Text positioning.
    pdfContext->fGraphicsState.fMatrixTm = matrix;
    pdfContext->fGraphicsState.fMatrixTlm = matrix;;

    return kPartial_SkPdfResult;
}

//— T* Move to the start of the next line. This operator has the same effect as the code
//0 Tl Td
//where Tl is the current leading parameter in the text state
static SkPdfResult PdfOp_T_star(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    SkPdfReal* zero = pdfContext->fPdfDoc->createReal(0.0);
    SkPdfReal* tl = pdfContext->fPdfDoc->createReal(pdfContext->fGraphicsState.fTextLeading);

    pdfContext->fObjectStack.push(zero);
    pdfContext->fObjectStack.push(tl);

    SkPdfResult ret = PdfOp_Td(pdfContext, canvas, looper);

    return ret;
}

static SkPdfResult PdfOp_m(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (pdfContext->fGraphicsState.fPathClosed) {
        pdfContext->fGraphicsState.fPath.reset();
        pdfContext->fGraphicsState.fPathClosed = false;
    }

    pdfContext->fGraphicsState.fCurPosY = pdfContext->fObjectStack.top()->numberValue();    pdfContext->fObjectStack.pop();
    pdfContext->fGraphicsState.fCurPosX = pdfContext->fObjectStack.top()->numberValue();    pdfContext->fObjectStack.pop();

    pdfContext->fGraphicsState.fPath.moveTo(SkDoubleToScalar(pdfContext->fGraphicsState.fCurPosX),
                                          SkDoubleToScalar(pdfContext->fGraphicsState.fCurPosY));

    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_l(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (pdfContext->fGraphicsState.fPathClosed) {
        pdfContext->fGraphicsState.fPath.reset();
        pdfContext->fGraphicsState.fPathClosed = false;
    }

    pdfContext->fGraphicsState.fCurPosY = pdfContext->fObjectStack.top()->numberValue();    pdfContext->fObjectStack.pop();
    pdfContext->fGraphicsState.fCurPosX = pdfContext->fObjectStack.top()->numberValue();    pdfContext->fObjectStack.pop();

    pdfContext->fGraphicsState.fPath.lineTo(SkDoubleToScalar(pdfContext->fGraphicsState.fCurPosX),
                                          SkDoubleToScalar(pdfContext->fGraphicsState.fCurPosY));

    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_c(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (pdfContext->fGraphicsState.fPathClosed) {
        pdfContext->fGraphicsState.fPath.reset();
        pdfContext->fGraphicsState.fPathClosed = false;
    }

    double y3 = pdfContext->fObjectStack.top()->numberValue();    pdfContext->fObjectStack.pop();
    double x3 = pdfContext->fObjectStack.top()->numberValue();    pdfContext->fObjectStack.pop();
    double y2 = pdfContext->fObjectStack.top()->numberValue();    pdfContext->fObjectStack.pop();
    double x2 = pdfContext->fObjectStack.top()->numberValue();    pdfContext->fObjectStack.pop();
    double y1 = pdfContext->fObjectStack.top()->numberValue();    pdfContext->fObjectStack.pop();
    double x1 = pdfContext->fObjectStack.top()->numberValue();    pdfContext->fObjectStack.pop();

    pdfContext->fGraphicsState.fPath.cubicTo(SkDoubleToScalar(x1), SkDoubleToScalar(y1),
                                            SkDoubleToScalar(x2), SkDoubleToScalar(y2),
                                            SkDoubleToScalar(x3), SkDoubleToScalar(y3));

    pdfContext->fGraphicsState.fCurPosX = x3;
    pdfContext->fGraphicsState.fCurPosY = y3;

    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_v(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (pdfContext->fGraphicsState.fPathClosed) {
        pdfContext->fGraphicsState.fPath.reset();
        pdfContext->fGraphicsState.fPathClosed = false;
    }

    double y3 = pdfContext->fObjectStack.top()->numberValue();    pdfContext->fObjectStack.pop();
    double x3 = pdfContext->fObjectStack.top()->numberValue();    pdfContext->fObjectStack.pop();
    double y2 = pdfContext->fObjectStack.top()->numberValue();    pdfContext->fObjectStack.pop();
    double x2 = pdfContext->fObjectStack.top()->numberValue();    pdfContext->fObjectStack.pop();
    double y1 = pdfContext->fGraphicsState.fCurPosY;
    double x1 = pdfContext->fGraphicsState.fCurPosX;

    pdfContext->fGraphicsState.fPath.cubicTo(SkDoubleToScalar(x1), SkDoubleToScalar(y1),
                                            SkDoubleToScalar(x2), SkDoubleToScalar(y2),
                                            SkDoubleToScalar(x3), SkDoubleToScalar(y3));

    pdfContext->fGraphicsState.fCurPosX = x3;
    pdfContext->fGraphicsState.fCurPosY = y3;

    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_y(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (pdfContext->fGraphicsState.fPathClosed) {
        pdfContext->fGraphicsState.fPath.reset();
        pdfContext->fGraphicsState.fPathClosed = false;
    }

    double y3 = pdfContext->fObjectStack.top()->numberValue();    pdfContext->fObjectStack.pop();
    double x3 = pdfContext->fObjectStack.top()->numberValue();    pdfContext->fObjectStack.pop();
    double y2 = pdfContext->fGraphicsState.fCurPosY;
    double x2 = pdfContext->fGraphicsState.fCurPosX;
    double y1 = pdfContext->fObjectStack.top()->numberValue();    pdfContext->fObjectStack.pop();
    double x1 = pdfContext->fObjectStack.top()->numberValue();    pdfContext->fObjectStack.pop();

    pdfContext->fGraphicsState.fPath.cubicTo(SkDoubleToScalar(x1), SkDoubleToScalar(y1),
                                            SkDoubleToScalar(x2), SkDoubleToScalar(y2),
                                            SkDoubleToScalar(x3), SkDoubleToScalar(y3));

    pdfContext->fGraphicsState.fCurPosX = x3;
    pdfContext->fGraphicsState.fCurPosY = y3;

    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_re(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (pdfContext->fGraphicsState.fPathClosed) {
        pdfContext->fGraphicsState.fPath.reset();
        pdfContext->fGraphicsState.fPathClosed = false;
    }

    double height = pdfContext->fObjectStack.top()->numberValue();      pdfContext->fObjectStack.pop();
    double width = pdfContext->fObjectStack.top()->numberValue();       pdfContext->fObjectStack.pop();
    double y = pdfContext->fObjectStack.top()->numberValue();           pdfContext->fObjectStack.pop();
    double x = pdfContext->fObjectStack.top()->numberValue();           pdfContext->fObjectStack.pop();

    pdfContext->fGraphicsState.fPath.addRect(SkDoubleToScalar(x), SkDoubleToScalar(y),
                                           SkDoubleToScalar(x + width), SkDoubleToScalar(y + height));

    pdfContext->fGraphicsState.fCurPosX = x;
    pdfContext->fGraphicsState.fCurPosY = y + height;

    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_h(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fGraphicsState.fPath.close();
    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_fillAndStroke(SkPdfContext* pdfContext, SkCanvas* canvas, bool fill, bool stroke, bool close, bool evenOdd) {
    SkPath path = pdfContext->fGraphicsState.fPath;

    if (close) {
        path.close();
    }

    canvas->setMatrix(pdfContext->fGraphicsState.fCTM);

    SkPaint paint;

    SkPoint line[2];
    if (fill && !stroke && path.isLine(line)) {
        paint.setStyle(SkPaint::kStroke_Style);

        // TODO(edisonn): implement this with patterns
        pdfContext->fGraphicsState.applyGraphicsState(&paint, false);
        paint.setStrokeWidth(SkDoubleToScalar(0));

        canvas->drawPath(path, paint);
    } else {
        if (fill) {
            if (strncmp((char*)pdfContext->fGraphicsState.fNonStroking.fColorSpace.fBuffer, "Pattern", strlen("Pattern")) == 0 &&
                pdfContext->fGraphicsState.fNonStroking.fPattern != NULL) {

                // TODO(edisonn): we can use a shader here, like imageshader to draw fast. ultimately,
                // if this is not possible, and we are in rasper mode, and the cells don't intersect, we could even have multiple cpus.

                PdfOp_q(pdfContext, canvas, NULL);

                if (evenOdd) {
                    path.setFillType(SkPath::kEvenOdd_FillType);
                }
                canvas->clipPath(path);

                if (pdfContext->fPdfDoc->mapper()->mapType1PatternDictionary(pdfContext->fGraphicsState.fNonStroking.fPattern) != kNone_SkPdfNativeObjectType) {
                    SkPdfType1PatternDictionary* pattern = (SkPdfType1PatternDictionary*)pdfContext->fGraphicsState.fNonStroking.fPattern;

                    // TODO(edisonn): constants
                    // TODO(edisonn): colored
                    if (pattern->PaintType(pdfContext->fPdfDoc) == 1) {
                        // TODO(edisonn): don't use abs, iterate as asked, if the cells intersect
                        // it will change the result iterating in reverse
                        int xStep = abs((int)pattern->XStep(pdfContext->fPdfDoc));
                        int yStep = abs((int)pattern->YStep(pdfContext->fPdfDoc));

                        SkRect bounds = path.getBounds();

                        // TODO(edisonn): xstep and ystep can be negative, and we need to iterate in reverse
                        // TODO(edisonn): don't do that!
                        bounds.sort();

                        SkScalar x;
                        SkScalar y;

                        y = bounds.top();
                        int totalx = 0;
                        int totaly = 0;
                        while (y < bounds.bottom()) {
                            x = bounds.left();
                            totalx = 0;

                            while (x < bounds.right()) {
                                doXObject(pdfContext, canvas, pattern);

                                pdfContext->fGraphicsState.fContentStreamMatrix.preTranslate(SkIntToScalar(xStep), SkIntToScalar(0));
                                totalx += xStep;
                                x += SkIntToScalar(xStep);
                            }
                            pdfContext->fGraphicsState.fContentStreamMatrix.preTranslate(SkIntToScalar(-totalx), SkIntToScalar(0));

                            pdfContext->fGraphicsState.fContentStreamMatrix.preTranslate(SkIntToScalar(0), SkIntToScalar(-yStep));
                            totaly += yStep;
                            y += SkIntToScalar(yStep);
                        }
                        pdfContext->fGraphicsState.fContentStreamMatrix.preTranslate(SkIntToScalar(0), SkIntToScalar(totaly));
                    }
                }

                // apply matrix
                // get xstep, y step, bbox ... for cliping, and bos of the path

                PdfOp_Q(pdfContext, canvas, NULL);
            } else {
                paint.setStyle(SkPaint::kFill_Style);
                if (evenOdd) {
                    path.setFillType(SkPath::kEvenOdd_FillType);
                }

                pdfContext->fGraphicsState.applyGraphicsState(&paint, false);

                canvas->drawPath(path, paint);
            }
        }

        if (stroke) {
            if (false && strncmp((char*)pdfContext->fGraphicsState.fNonStroking.fColorSpace.fBuffer, "Pattern", strlen("Pattern")) == 0) {
                // TODO(edisonn): implement Pattern for strokes
                paint.setStyle(SkPaint::kStroke_Style);

                paint.setColor(SK_ColorGREEN);

                path.setFillType(SkPath::kWinding_FillType);  // reset it, just in case it messes up the stroke
                canvas->drawPath(path, paint);
            } else {
                paint.setStyle(SkPaint::kStroke_Style);

                pdfContext->fGraphicsState.applyGraphicsState(&paint, true);

                path.setFillType(SkPath::kWinding_FillType);  // reset it, just in case it messes up the stroke
                canvas->drawPath(path, paint);
            }
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

    return kOK_SkPdfResult;

}

static SkPdfResult PdfOp_S(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_fillAndStroke(pdfContext, canvas, false, true, false, false);
}

static SkPdfResult PdfOp_s(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_fillAndStroke(pdfContext, canvas, false, true, true, false);
}

static SkPdfResult PdfOp_F(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_fillAndStroke(pdfContext, canvas, true, false, false, false);
}

static SkPdfResult PdfOp_f(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_fillAndStroke(pdfContext, canvas, true, false, false, false);
}

static SkPdfResult PdfOp_f_star(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_fillAndStroke(pdfContext, canvas, true, false, false, true);
}

static SkPdfResult PdfOp_B(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_fillAndStroke(pdfContext, canvas, true, true, false, false);
}

static SkPdfResult PdfOp_B_star(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_fillAndStroke(pdfContext, canvas, true, true, false, true);
}

static SkPdfResult PdfOp_b(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_fillAndStroke(pdfContext, canvas, true, true, true, false);
}

static SkPdfResult PdfOp_b_star(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_fillAndStroke(pdfContext, canvas, true, true, true, true);
}

static SkPdfResult PdfOp_n(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    canvas->setMatrix(pdfContext->fGraphicsState.fCTM);
    if (pdfContext->fGraphicsState.fHasClipPathToApply) {
#ifndef PDF_DEBUG_NO_CLIPING
        canvas->clipPath(pdfContext->fGraphicsState.fClipPath, SkRegion::kIntersect_Op, true);
#endif
    }

    //pdfContext->fGraphicsState.fClipPath.reset();
    pdfContext->fGraphicsState.fHasClipPathToApply = false;

    pdfContext->fGraphicsState.fPathClosed = true;

    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_BT(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fGraphicsState.fTextBlock   = true;
    SkMatrix matrix = pdfContext->fGraphicsState.fCTM;
    matrix.preScale(SkDoubleToScalar(1), SkDoubleToScalar(-1));
    pdfContext->fGraphicsState.fMatrixTm = matrix;
    pdfContext->fGraphicsState.fMatrixTlm = matrix;

    return kPartial_SkPdfResult;
}

static SkPdfResult PdfOp_ET(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (!pdfContext->fGraphicsState.fTextBlock) {
        return kIgnoreError_SkPdfResult;
    }
    // TODO(edisonn): anything else to be done once we are done with draw text? Like restore stack?
    return kOK_SkPdfResult;
}

SkPdfResult skpdfGraphicsStateApplyFontCore(SkPdfContext* pdfContext, const SkPdfNativeObject* fontName, double fontSize) {
#ifdef PDF_TRACE
    printf("font name: %s\n", fontName->nameValue2().c_str());
#endif

    if (!pdfContext->fGraphicsState.fResources->Font(pdfContext->fPdfDoc)) {
        // TODO(edisonn): try to recover and draw it any way?
        return kIgnoreError_SkPdfResult;
    }

    SkPdfNativeObject* objFont = pdfContext->fGraphicsState.fResources->Font(pdfContext->fPdfDoc)->get(fontName);
    objFont = pdfContext->fPdfDoc->resolveReference(objFont);
    if (kNone_SkPdfNativeObjectType == pdfContext->fPdfDoc->mapper()->mapFontDictionary(objFont)) {
        // TODO(edisonn): try to recover and draw it any way?
        return kIgnoreError_SkPdfResult;
    }

    SkPdfFontDictionary* fd = (SkPdfFontDictionary*)objFont;

    SkPdfFont* skfont = SkPdfFont::fontFromPdfDictionary(pdfContext->fPdfDoc, fd);

    if (skfont) {
        pdfContext->fGraphicsState.fSkFont = skfont;
    }
    pdfContext->fGraphicsState.fCurFontSize = fontSize;
    return kOK_SkPdfResult;
}

//font size Tf Set the text font, Tf
//, to font and the text font size, Tfs, to size. font is the name of a
//font resource in the Fontsubdictionary of the current resource dictionary; size is
//a number representing a scale factor. There is no initial value for either font or
//size; they must be speciﬁed explicitly using Tf before any text is shown.
static SkPdfResult PdfOp_Tf(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double fontSize = pdfContext->fObjectStack.top()->numberValue();     pdfContext->fObjectStack.pop();
    SkPdfNativeObject* fontName = pdfContext->fObjectStack.top();                           pdfContext->fObjectStack.pop();
    return skpdfGraphicsStateApplyFontCore(pdfContext, fontName, fontSize);
}

static SkPdfResult PdfOp_Tj(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (!pdfContext->fGraphicsState.fTextBlock) {
        // TODO(edisonn): try to recover and draw it any way?
        return kIgnoreError_SkPdfResult;
    }

    SkPdfResult ret = DrawText(pdfContext,
                             pdfContext->fObjectStack.top(),
                             canvas);
    pdfContext->fObjectStack.pop();

    return ret;
}

static SkPdfResult PdfOp_quote(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (!pdfContext->fGraphicsState.fTextBlock) {
        // TODO(edisonn): try to recover and draw it any way?
        return kIgnoreError_SkPdfResult;
    }

    PdfOp_T_star(pdfContext, canvas, looper);
    // Do not pop, and push, just transfer the param to Tj
    return PdfOp_Tj(pdfContext, canvas, looper);
}

static SkPdfResult PdfOp_doublequote(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (!pdfContext->fGraphicsState.fTextBlock) {
        // TODO(edisonn): try to recover and draw it any way?
        return kIgnoreError_SkPdfResult;
    }

    SkPdfNativeObject* str = pdfContext->fObjectStack.top();       pdfContext->fObjectStack.pop();
    SkPdfNativeObject* ac = pdfContext->fObjectStack.top();        pdfContext->fObjectStack.pop();
    SkPdfNativeObject* aw = pdfContext->fObjectStack.top();        pdfContext->fObjectStack.pop();

    pdfContext->fObjectStack.push(aw);
    PdfOp_Tw(pdfContext, canvas, looper);

    pdfContext->fObjectStack.push(ac);
    PdfOp_Tc(pdfContext, canvas, looper);

    pdfContext->fObjectStack.push(str);
    PdfOp_quote(pdfContext, canvas, looper);

    return kPartial_SkPdfResult;
}

static SkPdfResult PdfOp_TJ(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    if (!pdfContext->fGraphicsState.fTextBlock) {
        // TODO(edisonn): try to recover and draw it any way?
        return kIgnoreError_SkPdfResult;
    }

    SkPdfArray* array = (SkPdfArray*)pdfContext->fObjectStack.top();
    pdfContext->fObjectStack.pop();

    if (!array->isArray()) {
        return kIgnoreError_SkPdfResult;
    }

    for( int i=0; i<static_cast<int>(array->size()); i++ )
    {
        if( (*array)[i]->isAnyString()) {
            SkPdfNativeObject* obj = (*array)[i];
            DrawText(pdfContext,
                     obj,
                     canvas);
        } else if ((*array)[i]->isNumber()) {
            double dx = (*array)[i]->numberValue();
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
    return kPartial_SkPdfResult;  // TODO(edisonn): Implement fully DrawText before returing OK.
}

static SkPdfResult PdfOp_CS_cs(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfColorOperator* colorOperator) {
    SkPdfNativeObject* name = pdfContext->fObjectStack.top();    pdfContext->fObjectStack.pop();

    //Next, get the ColorSpace Dictionary from the Resource Dictionary:
    SkPdfDictionary* colorSpaceResource = pdfContext->fGraphicsState.fResources->ColorSpace(pdfContext->fPdfDoc);

    SkPdfNativeObject* colorSpace = colorSpaceResource ? pdfContext->fPdfDoc->resolveReference(colorSpaceResource->get(name)) : name;

    if (colorSpace == NULL) {
        colorOperator->fColorSpace = name->strRef();
    } else {
#ifdef PDF_TRACE
        printf("CS = %s\n", colorSpace->toString(0, 0).c_str());
#endif   // PDF_TRACE
        if (colorSpace->isName()) {
            colorOperator->fColorSpace = colorSpace->strRef();
        } else if (colorSpace->isArray()) {
            int cnt = colorSpace->size();
            if (cnt == 0) {
                return kIgnoreError_SkPdfResult;
            }
            SkPdfNativeObject* type = colorSpace->objAtAIndex(0);
            type = pdfContext->fPdfDoc->resolveReference(type);

            if (type->isName("ICCBased")) {
                if (cnt != 2) {
                    return kIgnoreError_SkPdfResult;
                }
                SkPdfNativeObject* prop = colorSpace->objAtAIndex(1);
                prop = pdfContext->fPdfDoc->resolveReference(prop);
#ifdef PDF_TRACE
                printf("ICCBased prop = %s\n", prop->toString(0, 0).c_str());
#endif   // PDF_TRACE
                // TODO(edisonn): hack
                if (prop && prop->isDictionary() && prop->get("N") &&  prop->get("N")->isInteger() && prop->get("N")->intValue() == 3) {
                    colorOperator->setColorSpace(&strings_DeviceRGB);
                    return kPartial_SkPdfResult;
                }
                return kNYI_SkPdfResult;
            }
        }
    }

    return kPartial_SkPdfResult;
}

static SkPdfResult PdfOp_CS(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_CS_cs(pdfContext, canvas, &pdfContext->fGraphicsState.fStroking);
}

static SkPdfResult PdfOp_cs(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_CS_cs(pdfContext, canvas, &pdfContext->fGraphicsState.fNonStroking);
}

static SkPdfResult PdfOp_SC_sc(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfColorOperator* colorOperator) {
    double c[4];
//    int64_t v[4];

    int n = GetColorSpaceComponents(colorOperator->fColorSpace);

    bool doubles = true;
    if (colorOperator->fColorSpace.equals("Indexed")) {
        doubles = false;
    }

#ifdef PDF_TRACE
    printf("color space = %s, N = %i\n", colorOperator->fColorSpace.fBuffer, n);
#endif

    for (int i = n - 1; i >= 0 ; i--) {
        if (doubles) {
            c[i] = pdfContext->fObjectStack.top()->numberValue();         pdfContext->fObjectStack.pop();
//        } else {
//            v[i] = pdfContext->fObjectStack.top()->intValue();        pdfContext->fObjectStack.pop();
        }
    }

    // TODO(edisonn): Now, set that color. Only DeviceRGB supported.
    // TODO(edisonn): do possible field values to enum at parsing time!
    // TODO(edisonn): support also abreviations /DeviceRGB == /RGB
    if (colorOperator->fColorSpace.equals("DeviceRGB") || colorOperator->fColorSpace.equals("RGB")) {
        colorOperator->setRGBColor(SkColorSetRGB((U8CPU)(255*c[0]), (U8CPU)(255*c[1]), (U8CPU)(255*c[2])));
    }
    return kPartial_SkPdfResult;
}

static SkPdfResult PdfOp_SC(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_SC_sc(pdfContext, canvas, &pdfContext->fGraphicsState.fStroking);
}

static SkPdfResult PdfOp_sc(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_SC_sc(pdfContext, canvas, &pdfContext->fGraphicsState.fNonStroking);
}

static SkPdfResult PdfOp_SCN_scn(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfColorOperator* colorOperator) {
    if (pdfContext->fObjectStack.top()->isName()) {
        SkPdfNativeObject* name = pdfContext->fObjectStack.top();    pdfContext->fObjectStack.pop();

        //Next, get the ExtGState Dictionary from the Resource Dictionary:
        SkPdfDictionary* patternResources = pdfContext->fGraphicsState.fResources->Pattern(pdfContext->fPdfDoc);

        if (patternResources == NULL) {
#ifdef PDF_TRACE
            printf("ExtGState is NULL!\n");
#endif
            return kIgnoreError_SkPdfResult;
        }

        colorOperator->setPatternColorSpace(pdfContext->fPdfDoc->resolveReference(patternResources->get(name)));
    }

    // TODO(edisonn): SCN supports more color spaces than SCN. Read and implement spec.
    PdfOp_SC_sc(pdfContext, canvas, colorOperator);

    return kPartial_SkPdfResult;
}

static SkPdfResult PdfOp_SCN(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_SCN_scn(pdfContext, canvas, &pdfContext->fGraphicsState.fStroking);
}

static SkPdfResult PdfOp_scn(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_SCN_scn(pdfContext, canvas, &pdfContext->fGraphicsState.fNonStroking);
}

static SkPdfResult PdfOp_G_g(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfColorOperator* colorOperator) {
    /*double gray = */pdfContext->fObjectStack.top()->numberValue();     pdfContext->fObjectStack.pop();
    return kNYI_SkPdfResult;
}

static SkPdfResult PdfOp_G(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_G_g(pdfContext, canvas, &pdfContext->fGraphicsState.fStroking);
}

static SkPdfResult PdfOp_g(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_G_g(pdfContext, canvas, &pdfContext->fGraphicsState.fNonStroking);
}

static SkPdfResult PdfOp_RG_rg(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfColorOperator* colorOperator) {
    double b = pdfContext->fObjectStack.top()->numberValue();     pdfContext->fObjectStack.pop();
    double g = pdfContext->fObjectStack.top()->numberValue();     pdfContext->fObjectStack.pop();
    double r = pdfContext->fObjectStack.top()->numberValue();     pdfContext->fObjectStack.pop();

    colorOperator->fColorSpace = strings_DeviceRGB;
    colorOperator->setRGBColor(SkColorSetRGB((U8CPU)(255*r), (U8CPU)(255*g), (U8CPU)(255*b)));
    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_RG(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_RG_rg(pdfContext, canvas, &pdfContext->fGraphicsState.fStroking);
}

static SkPdfResult PdfOp_rg(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_RG_rg(pdfContext, canvas, &pdfContext->fGraphicsState.fNonStroking);
}

static SkPdfResult PdfOp_K_k(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfColorOperator* colorOperator) {
    // TODO(edisonn): spec has some rules about overprint, implement them.
    /*double k = */pdfContext->fObjectStack.top()->numberValue();     pdfContext->fObjectStack.pop();
    /*double y = */pdfContext->fObjectStack.top()->numberValue();     pdfContext->fObjectStack.pop();
    /*double m = */pdfContext->fObjectStack.top()->numberValue();     pdfContext->fObjectStack.pop();
    /*double c = */pdfContext->fObjectStack.top()->numberValue();     pdfContext->fObjectStack.pop();

    colorOperator->fColorSpace = strings_DeviceCMYK;
    // TODO(edisonn): Set color.
    return kNYI_SkPdfResult;
}

static SkPdfResult PdfOp_K(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_K_k(pdfContext, canvas, &pdfContext->fGraphicsState.fStroking);
}

static SkPdfResult PdfOp_k(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return PdfOp_K_k(pdfContext, canvas, &pdfContext->fGraphicsState.fNonStroking);
}

static SkPdfResult PdfOp_W(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fGraphicsState.fClipPath = pdfContext->fGraphicsState.fPath;
    pdfContext->fGraphicsState.fHasClipPathToApply = true;

    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_W_star(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fGraphicsState.fClipPath = pdfContext->fGraphicsState.fPath;

    pdfContext->fGraphicsState.fClipPath.setFillType(SkPath::kEvenOdd_FillType);
    pdfContext->fGraphicsState.fHasClipPathToApply = true;

    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_BX(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    *looper = new PdfCompatibilitySectionLooper();
    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_EX(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
#ifdef ASSERT_BAD_PDF_OPS
    SkASSERT(false);  // EX must be consumed by PdfCompatibilitySectionLooper, but let's
                      // have the assert when testing good pdfs.
#endif
    return kIgnoreError_SkPdfResult;
}

static SkPdfResult PdfOp_BI(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    *looper = new PdfInlineImageLooper();
    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_ID(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
#ifdef ASSERT_BAD_PDF_OPS
    SkASSERT(false);  // must be processed in inline image looper, but let's
                      // have the assert when testing good pdfs.
#endif
    return kIgnoreError_SkPdfResult;
}

static SkPdfResult PdfOp_EI(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
#ifdef ASSERT_BAD_PDF_OPS
    SkASSERT(false);  // must be processed in inline image looper, but let's
                      // have the assert when testing good pdfs.
#endif
    return kIgnoreError_SkPdfResult;
}


// TODO(edisonn): security review here, make sure all parameters are valid, and safe.
SkPdfResult skpdfGraphicsStateApply_ca(SkPdfContext* pdfContext, double ca) {
    pdfContext->fGraphicsState.fNonStroking.fOpacity = ca;
    return kOK_SkPdfResult;
}

SkPdfResult skpdfGraphicsStateApply_CA(SkPdfContext* pdfContext, double CA) {
    pdfContext->fGraphicsState.fStroking.fOpacity = CA;
    return kOK_SkPdfResult;
}

SkPdfResult skpdfGraphicsStateApplyLW(SkPdfContext* pdfContext, double lineWidth) {
    pdfContext->fGraphicsState.fLineWidth = lineWidth;
    return kOK_SkPdfResult;
}

SkPdfResult skpdfGraphicsStateApplyLC(SkPdfContext* pdfContext, int64_t lineCap) {
    pdfContext->fGraphicsState.fLineCap = (int)lineCap;
    return kOK_SkPdfResult;
}

SkPdfResult skpdfGraphicsStateApplyLJ(SkPdfContext* pdfContext, int64_t lineJoin) {
    pdfContext->fGraphicsState.fLineJoin = (int)lineJoin;
    return kOK_SkPdfResult;
}

SkPdfResult skpdfGraphicsStateApplyML(SkPdfContext* pdfContext, double miterLimit) {
    pdfContext->fGraphicsState.fMiterLimit = miterLimit;
    return kOK_SkPdfResult;
}

// TODO(edisonn): implement all rules, as of now 3) and 5) and 6) do not seem suported by skia, but I am not sure
/*
1) [ ] 0 No dash; solid, unbroken lines
2) [3] 0 3 units on, 3 units off, …
3) [2] 1 1 on, 2 off, 2 on, 2 off, …
4) [2 1] 0 2 on, 1 off, 2 on, 1 off, …
5) [3 5] 6 2 off, 3 on, 5 off, 3 on, 5 off, …
6) [2 3] 11 1 on, 3 off, 2 on, 3 off, 2 on, …
 */

SkPdfResult skpdfGraphicsStateApplyD(SkPdfContext* pdfContext, SkPdfArray* intervals, SkPdfNativeObject* phase) {
    int cnt = intervals->size();
    if (cnt >= 256) {
        // TODO(edisonn): report error/warning, unsuported;
        // TODO(edisonn): alloc memory
        return kIgnoreError_SkPdfResult;
    }
    for (int i = 0; i < cnt; i++) {
        if (!intervals->objAtAIndex(i)->isNumber()) {
            // TODO(edisonn): report error/warning
            return kIgnoreError_SkPdfResult;
        }
    }

    double total = 0;
    for (int i = 0 ; i < cnt; i++) {
        pdfContext->fGraphicsState.fDashArray[i] = intervals->objAtAIndex(i)->scalarValue();
        total += pdfContext->fGraphicsState.fDashArray[i];
    }
    if (cnt & 1) {
        if (cnt == 1) {
            pdfContext->fGraphicsState.fDashArray[1] = pdfContext->fGraphicsState.fDashArray[0];
            cnt++;
        } else {
            // TODO(edisonn): report error/warning
            return kNYI_SkPdfResult;
        }
    }
    pdfContext->fGraphicsState.fDashArrayLength = cnt;
    pdfContext->fGraphicsState.fDashPhase = phase->scalarValue();
    if (pdfContext->fGraphicsState.fDashPhase == 0) {
        // other rules, changes?
        pdfContext->fGraphicsState.fDashPhase = total;
    }

    return kOK_SkPdfResult;
}

SkPdfResult skpdfGraphicsStateApplyD(SkPdfContext* pdfContext, SkPdfArray* dash) {
    // TODO(edisonn): verify input
    if (!dash || dash->isArray() || dash->size() != 2 || !dash->objAtAIndex(0)->isArray() || !dash->objAtAIndex(1)->isNumber()) {
        // TODO(edisonn): report error/warning
        return kIgnoreError_SkPdfResult;
    }
    return skpdfGraphicsStateApplyD(pdfContext, (SkPdfArray*)dash->objAtAIndex(0), dash->objAtAIndex(1));
}

void skpdfGraphicsStateApplyFont(SkPdfContext* pdfContext, SkPdfArray* fontAndSize) {
    if (!fontAndSize || fontAndSize->isArray() || fontAndSize->size() != 2 || !fontAndSize->objAtAIndex(0)->isName() || !fontAndSize->objAtAIndex(1)->isNumber()) {
        // TODO(edisonn): report error/warning
        return;
    }
    skpdfGraphicsStateApplyFontCore(pdfContext, fontAndSize->objAtAIndex(0), fontAndSize->objAtAIndex(1)->numberValue());
}


//lineWidth w Set the line width in the graphics state (see “Line Width” on page 152).
static SkPdfResult PdfOp_w(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double lw = pdfContext->fObjectStack.top()->numberValue();    pdfContext->fObjectStack.pop();
    return skpdfGraphicsStateApplyLW(pdfContext, lw);
}

//lineCap J Set the line cap style in the graphics state (see “Line Cap Style” on page 153).
static SkPdfResult PdfOp_J(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    int64_t lc = pdfContext->fObjectStack.top()->numberValue();    pdfContext->fObjectStack.pop();
    return skpdfGraphicsStateApplyLC(pdfContext, lc);
}

//lineJoin j Set the line join style in the graphics state (see “Line Join Style” on page 153).
static SkPdfResult PdfOp_j(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double lj = pdfContext->fObjectStack.top()->numberValue();    pdfContext->fObjectStack.pop();
    return skpdfGraphicsStateApplyLJ(pdfContext, lj);
}

//miterLimit M Set the miter limit in the graphics state (see “Miter Limit” on page 153).
static SkPdfResult PdfOp_M(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double ml = pdfContext->fObjectStack.top()->numberValue();    pdfContext->fObjectStack.pop();
    return skpdfGraphicsStateApplyML(pdfContext, ml);
}

//dashArray dashPhase d Set the line dash pattern in the graphics state (see “Line Dash Pattern” on
//page 155).
static SkPdfResult PdfOp_d(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    SkPdfNativeObject* phase = pdfContext->fObjectStack.top();          pdfContext->fObjectStack.pop();
    SkPdfNativeObject* array = pdfContext->fObjectStack.top();          pdfContext->fObjectStack.pop();

    if (!array->isArray()) {
        return kIgnoreError_SkPdfResult;
    }

    return skpdfGraphicsStateApplyD(pdfContext, (SkPdfArray*)array, phase);
}

//intent ri (PDF 1.1) Set the color rendering intent in the graphics state (see “Rendering Intents” on page 197).
static SkPdfResult PdfOp_ri(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fObjectStack.pop();

    return kNYI_SkPdfResult;
}

//ﬂatness i Set the ﬂatness tolerance in the graphics state (see Section 6.5.1, “Flatness
//Tolerance”). ﬂatness is a number in the range 0 to 100; a value of 0 speci-
//ﬁes the output device’s default ﬂatness tolerance.
static SkPdfResult PdfOp_i(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fObjectStack.pop();

    return kNYI_SkPdfResult;
}

SkTDict<SkXfermode::Mode> gPdfBlendModes(20);

class InitBlendModes {
public:
    InitBlendModes() {
        // TODO(edisonn): use the python code generator?
        // TABLE 7.2 Standard separable blend modes
        gPdfBlendModes.set("Normal", SkXfermode::kSrc_Mode);
        gPdfBlendModes.set("Multiply", SkXfermode::kMultiply_Mode);
        gPdfBlendModes.set("Screen", SkXfermode::kScreen_Mode);
        gPdfBlendModes.set("Overlay", SkXfermode::kOverlay_Mode);
        gPdfBlendModes.set("Darken", SkXfermode::kDarken_Mode);
        gPdfBlendModes.set("Lighten", SkXfermode::kLighten_Mode);
        gPdfBlendModes.set("ColorDodge", SkXfermode::kColorDodge_Mode);
        gPdfBlendModes.set("ColorBurn", SkXfermode::kColorBurn_Mode);
        gPdfBlendModes.set("HardLight", SkXfermode::kHardLight_Mode);
        gPdfBlendModes.set("SoftLight", SkXfermode::kSoftLight_Mode);
        gPdfBlendModes.set("Difference", SkXfermode::kDifference_Mode);
        gPdfBlendModes.set("Exclusion", SkXfermode::kExclusion_Mode);

        // TABLE 7.3 Standard nonseparable blend modes
        gPdfBlendModes.set("Hue", SkXfermode::kHue_Mode);
        gPdfBlendModes.set("Saturation", SkXfermode::kSaturation_Mode);
        gPdfBlendModes.set("Color", SkXfermode::kColor_Mode);
        gPdfBlendModes.set("Luminosity", SkXfermode::kLuminosity_Mode);
    }
};

InitBlendModes _gDummyInniter;

SkXfermode::Mode xferModeFromBlendMode(const char* blendMode, size_t len) {
    SkXfermode::Mode mode = (SkXfermode::Mode)(SkXfermode::kLastMode + 1);
    if (gPdfBlendModes.find(blendMode, len, &mode)) {
        return mode;
    }

    return (SkXfermode::Mode)(SkXfermode::kLastMode + 1);
}

void skpdfGraphicsStateApplyBM_name(SkPdfContext* pdfContext, const std::string& blendMode) {
    SkXfermode::Mode mode = xferModeFromBlendMode(blendMode.c_str(), blendMode.length());
    if (mode <= SkXfermode::kLastMode) {
        pdfContext->fGraphicsState.fBlendModesLength = 1;
        pdfContext->fGraphicsState.fBlendModes[0] = mode;
    } else {
        // TODO(edisonn): report unknown blend mode
    }
}

void skpdfGraphicsStateApplyBM_array(SkPdfContext* pdfContext, SkPdfArray* blendModes) {
    if (!blendModes || blendModes->isArray() || blendModes->size() == 0 || blendModes->size() > 256) {
        // TODO(edisonn): report error/warning
        return;
    }
    SkXfermode::Mode modes[256];
    int cnt = blendModes->size();
    for (int i = 0; i < cnt; i++) {
        SkPdfNativeObject* name = blendModes->objAtAIndex(i);
        if (!name->isName()) {
            // TODO(edisonn): report error/warning
            return;
        }
        SkXfermode::Mode mode = xferModeFromBlendMode(name->c_str(), name->lenstr());
        if (mode > SkXfermode::kLastMode) {
            // TODO(edisonn): report error/warning
            return;
        }
    }

    pdfContext->fGraphicsState.fBlendModesLength = cnt;
    for (int i = 0; i < cnt; i++) {
        pdfContext->fGraphicsState.fBlendModes[i] = modes[i];
    }
}

void skpdfGraphicsStateApplySMask_dict(SkPdfContext* pdfContext, SkPdfDictionary* sMask) {
    // TODO(edisonn): verify input
    if (pdfContext->fPdfDoc->mapper()->mapSoftMaskDictionary(sMask)) {
        pdfContext->fGraphicsState.fSoftMaskDictionary = (SkPdfSoftMaskDictionary*)sMask;
    } else if (pdfContext->fPdfDoc->mapper()->mapSoftMaskImageDictionary(sMask)) {
        SkPdfSoftMaskImageDictionary* smid = (SkPdfSoftMaskImageDictionary*)sMask;
        pdfContext->fGraphicsState.fSMask = getImageFromObject(pdfContext, smid, true);
    } else {
        // TODO (edisonn): report error/warning
    }
}

void skpdfGraphicsStateApplySMask_name(SkPdfContext* pdfContext, const std::string& sMask) {
    if (sMask == "None") {
        pdfContext->fGraphicsState.fSoftMaskDictionary = NULL;
        pdfContext->fGraphicsState.fSMask = NULL;
        return;
    }

    //Next, get the ExtGState Dictionary from the Resource Dictionary:
    SkPdfDictionary* extGStateDictionary = pdfContext->fGraphicsState.fResources->ExtGState(pdfContext->fPdfDoc);

    if (extGStateDictionary == NULL) {
#ifdef PDF_TRACE
        printf("ExtGState is NULL!\n");
#endif
        // TODO (edisonn): report error/warning
        return;
    }

    SkPdfNativeObject* obj = pdfContext->fPdfDoc->resolveReference(extGStateDictionary->get(sMask.c_str()));
    if (!obj || !obj->isDictionary()) {
        // TODO (edisonn): report error/warning
        return;
    }

    pdfContext->fGraphicsState.fSoftMaskDictionary = NULL;
    pdfContext->fGraphicsState.fSMask = NULL;

    skpdfGraphicsStateApplySMask_dict(pdfContext, obj->asDictionary());
}

void skpdfGraphicsStateApplyAIS(SkPdfContext* pdfContext, bool alphaSource) {
    pdfContext->fGraphicsState.fAlphaSource = alphaSource;
}


//dictName gs (PDF 1.2) Set the speciﬁed parameters in the graphics state. dictName is
//the name of a graphics state parameter dictionary in the ExtGState subdictionary of the current resource dictionary (see the next section).
static SkPdfResult PdfOp_gs(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    SkPdfNativeObject* name = pdfContext->fObjectStack.top();    pdfContext->fObjectStack.pop();

#ifdef PDF_TRACE
    std::string str;
#endif

    //Next, get the ExtGState Dictionary from the Resource Dictionary:
    SkPdfDictionary* extGStateDictionary = pdfContext->fGraphicsState.fResources->ExtGState(pdfContext->fPdfDoc);

    if (extGStateDictionary == NULL) {
#ifdef PDF_TRACE
        printf("ExtGState is NULL!\n");
#endif
        return kIgnoreError_SkPdfResult;
    }

    SkPdfNativeObject* value = pdfContext->fPdfDoc->resolveReference(extGStateDictionary->get(name));

    if (kNone_SkPdfNativeObjectType == pdfContext->fPdfDoc->mapper()->mapGraphicsStateDictionary(value)) {
        return kIgnoreError_SkPdfResult;
    }
    SkPdfGraphicsStateDictionary* gs = (SkPdfGraphicsStateDictionary*)value;

    // TODO(edisonn): now load all those properties in graphic state.
    if (gs == NULL) {
        return kIgnoreError_SkPdfResult;
    }

    if (gs->has_LW()) {
        skpdfGraphicsStateApplyLW(pdfContext, gs->LW(pdfContext->fPdfDoc));
    }

    if (gs->has_LC()) {
        skpdfGraphicsStateApplyLC(pdfContext, gs->LC(pdfContext->fPdfDoc));
    }

    if (gs->has_LJ()) {
        skpdfGraphicsStateApplyLJ(pdfContext, gs->LJ(pdfContext->fPdfDoc));
    }

    if (gs->has_ML()) {
        skpdfGraphicsStateApplyML(pdfContext, gs->ML(pdfContext->fPdfDoc));
    }

    if (gs->has_D()) {
        skpdfGraphicsStateApplyD(pdfContext, gs->D(pdfContext->fPdfDoc));
    }

    if (gs->has_Font()) {
        skpdfGraphicsStateApplyFont(pdfContext, gs->Font(pdfContext->fPdfDoc));
    }

    if (gs->has_BM()) {
        if (gs->isBMAName(pdfContext->fPdfDoc)) {
            skpdfGraphicsStateApplyBM_name(pdfContext, gs->getBMAsName(pdfContext->fPdfDoc));
        } else if (gs->isBMAArray(pdfContext->fPdfDoc)) {
            skpdfGraphicsStateApplyBM_array(pdfContext, gs->getBMAsArray(pdfContext->fPdfDoc));
        } else {
            // TODO(edisonn): report/warn
        }
    }

    if (gs->has_SMask()) {
        if (gs->isSMaskAName(pdfContext->fPdfDoc)) {
            skpdfGraphicsStateApplySMask_name(pdfContext, gs->getSMaskAsName(pdfContext->fPdfDoc));
        } else if (gs->isSMaskADictionary(pdfContext->fPdfDoc)) {
            skpdfGraphicsStateApplySMask_dict(pdfContext, gs->getSMaskAsDictionary(pdfContext->fPdfDoc));
        } else {
            // TODO(edisonn): report/warn
        }
    }

    if (gs->has_ca()) {
        skpdfGraphicsStateApply_ca(pdfContext, gs->ca(pdfContext->fPdfDoc));
    }

    if (gs->has_CA()) {
        skpdfGraphicsStateApply_CA(pdfContext, gs->CA(pdfContext->fPdfDoc));
    }

    if (gs->has_AIS()) {
        skpdfGraphicsStateApplyAIS(pdfContext, gs->AIS(pdfContext->fPdfDoc));
    }

    return kOK_SkPdfResult;
}

//charSpace Tc Set the character spacing, Tc
//, to charSpace, which is a number expressed in unscaled text space units. Character spacing is used by the Tj, TJ, and ' operators.
//Initial value: 0.
SkPdfResult PdfOp_Tc(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double charSpace = pdfContext->fObjectStack.top()->numberValue();     pdfContext->fObjectStack.pop();
    pdfContext->fGraphicsState.fCharSpace = charSpace;

    return kOK_SkPdfResult;
}

//wordSpace Tw Set the word spacing, T
//w
//, to wordSpace, which is a number expressed in unscaled
//text space units. Word spacing is used by the Tj, TJ, and ' operators. Initial
//value: 0.
SkPdfResult PdfOp_Tw(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    double wordSpace = pdfContext->fObjectStack.top()->numberValue();     pdfContext->fObjectStack.pop();
    pdfContext->fGraphicsState.fWordSpace = wordSpace;

    return kOK_SkPdfResult;
}

//scale Tz Set the horizontal scaling, Th
//, to (scale ˜ 100). scale is a number specifying the
//percentage of the normal width. Initial value: 100 (normal width).
static SkPdfResult PdfOp_Tz(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    /*double scale = */pdfContext->fObjectStack.top()->numberValue();     pdfContext->fObjectStack.pop();

    return kNYI_SkPdfResult;
}

//render Tr Set the text rendering mode, T
//mode, to render, which is an integer. Initial value: 0.
static SkPdfResult PdfOp_Tr(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    /*double render = */pdfContext->fObjectStack.top()->numberValue();     pdfContext->fObjectStack.pop();

    return kNYI_SkPdfResult;
}
//rise Ts Set the text rise, Trise, to rise, which is a number expressed in unscaled text space
//units. Initial value: 0.
static SkPdfResult PdfOp_Ts(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    /*double rise = */pdfContext->fObjectStack.top()->numberValue();     pdfContext->fObjectStack.pop();

    return kNYI_SkPdfResult;
}

//wx wy d0
static SkPdfResult PdfOp_d0(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fObjectStack.pop();
    pdfContext->fObjectStack.pop();

    return kNYI_SkPdfResult;
}

//wx wy llx lly urx ury d1
static SkPdfResult PdfOp_d1(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fObjectStack.pop();
    pdfContext->fObjectStack.pop();
    pdfContext->fObjectStack.pop();
    pdfContext->fObjectStack.pop();
    pdfContext->fObjectStack.pop();
    pdfContext->fObjectStack.pop();

    return kNYI_SkPdfResult;
}

//name sh
static SkPdfResult PdfOp_sh(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fObjectStack.pop();

    return kNYI_SkPdfResult;
}

//name Do
static SkPdfResult PdfOp_Do(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    SkPdfNativeObject* name = pdfContext->fObjectStack.top();    pdfContext->fObjectStack.pop();

    SkPdfDictionary* xObject =  pdfContext->fGraphicsState.fResources->XObject(pdfContext->fPdfDoc);

    if (xObject == NULL) {
#ifdef PDF_TRACE
        printf("XObject is NULL!\n");
#endif
        return kIgnoreError_SkPdfResult;
    }

    SkPdfNativeObject* value = xObject->get(name);
    value = pdfContext->fPdfDoc->resolveReference(value);

#ifdef PDF_TRACE
//    value->ToString(str);
//    printf("Do object value: %s\n", str);
#endif

    return doXObject(pdfContext, canvas, value);
}

//tag MP Designate a marked-content point. tag is a name object indicating the role or
//signiﬁcance of the point.
static SkPdfResult PdfOp_MP(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fObjectStack.pop();

    return kNYI_SkPdfResult;
}

//tag properties DP Designate a marked-content point with an associated property list. tag is a
//name object indicating the role or signiﬁcance of the point; properties is
//either an inline dictionary containing the property list or a name object
//associated with it in the Properties subdictionary of the current resource
//dictionary (see Section 9.5.1, “Property Lists”).
static SkPdfResult PdfOp_DP(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fObjectStack.pop();
    pdfContext->fObjectStack.pop();

    return kNYI_SkPdfResult;
}

//tag BMC Begin a marked-content sequence terminated by a balancing EMC operator.
//tag is a name object indicating the role or signiﬁcance of the sequence.
static SkPdfResult PdfOp_BMC(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fObjectStack.pop();

    return kNYI_SkPdfResult;
}

//tag properties BDC Begin a marked-content sequence with an associated property list, terminated
//by a balancing EMCoperator. tag is a name object indicating the role or significance of the sequence; propertiesis either an inline dictionary containing the
//property list or a name object associated with it in the Properties subdictionary of the current resource dictionary (see Section 9.5.1, “Property Lists”).
static SkPdfResult PdfOp_BDC(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    pdfContext->fObjectStack.pop();
    pdfContext->fObjectStack.pop();

    return kNYI_SkPdfResult;
}

//— EMC End a marked-content sequence begun by a BMC or BDC operator.
static SkPdfResult PdfOp_EMC(SkPdfContext* pdfContext, SkCanvas* canvas, PdfTokenLooper** looper) {
    return kNYI_SkPdfResult;
}

static void initPdfOperatorRenderes() {
    static bool gInitialized = false;
    if (gInitialized) {
        return;
    }

    gPdfOps.set("q", PdfOp_q);
    gPdfOps.set("Q", PdfOp_Q);
    gPdfOps.set("cm", PdfOp_cm);

    gPdfOps.set("TD", PdfOp_TD);
    gPdfOps.set("Td", PdfOp_Td);
    gPdfOps.set("Tm", PdfOp_Tm);
    gPdfOps.set("T*", PdfOp_T_star);

    gPdfOps.set("m", PdfOp_m);
    gPdfOps.set("l", PdfOp_l);
    gPdfOps.set("c", PdfOp_c);
    gPdfOps.set("v", PdfOp_v);
    gPdfOps.set("y", PdfOp_y);
    gPdfOps.set("h", PdfOp_h);
    gPdfOps.set("re", PdfOp_re);

    gPdfOps.set("S", PdfOp_S);
    gPdfOps.set("s", PdfOp_s);
    gPdfOps.set("f", PdfOp_f);
    gPdfOps.set("F", PdfOp_F);
    gPdfOps.set("f*", PdfOp_f_star);
    gPdfOps.set("B", PdfOp_B);
    gPdfOps.set("B*", PdfOp_B_star);
    gPdfOps.set("b", PdfOp_b);
    gPdfOps.set("b*", PdfOp_b_star);
    gPdfOps.set("n", PdfOp_n);

    gPdfOps.set("BT", PdfOp_BT);
    gPdfOps.set("ET", PdfOp_ET);

    gPdfOps.set("Tj", PdfOp_Tj);
    gPdfOps.set("'", PdfOp_quote);
    gPdfOps.set("\"", PdfOp_doublequote);
    gPdfOps.set("TJ", PdfOp_TJ);

    gPdfOps.set("CS", PdfOp_CS);
    gPdfOps.set("cs", PdfOp_cs);
    gPdfOps.set("SC", PdfOp_SC);
    gPdfOps.set("SCN", PdfOp_SCN);
    gPdfOps.set("sc", PdfOp_sc);
    gPdfOps.set("scn", PdfOp_scn);
    gPdfOps.set("G", PdfOp_G);
    gPdfOps.set("g", PdfOp_g);
    gPdfOps.set("RG", PdfOp_RG);
    gPdfOps.set("rg", PdfOp_rg);
    gPdfOps.set("K", PdfOp_K);
    gPdfOps.set("k", PdfOp_k);

    gPdfOps.set("W", PdfOp_W);
    gPdfOps.set("W*", PdfOp_W_star);

    gPdfOps.set("BX", PdfOp_BX);
    gPdfOps.set("EX", PdfOp_EX);

    gPdfOps.set("BI", PdfOp_BI);
    gPdfOps.set("ID", PdfOp_ID);
    gPdfOps.set("EI", PdfOp_EI);

    gPdfOps.set("w", PdfOp_w);
    gPdfOps.set("J", PdfOp_J);
    gPdfOps.set("j", PdfOp_j);
    gPdfOps.set("M", PdfOp_M);
    gPdfOps.set("d", PdfOp_d);
    gPdfOps.set("ri", PdfOp_ri);
    gPdfOps.set("i", PdfOp_i);
    gPdfOps.set("gs", PdfOp_gs);

    gPdfOps.set("Tc", PdfOp_Tc);
    gPdfOps.set("Tw", PdfOp_Tw);
    gPdfOps.set("Tz", PdfOp_Tz);
    gPdfOps.set("TL", PdfOp_TL);
    gPdfOps.set("Tf", PdfOp_Tf);
    gPdfOps.set("Tr", PdfOp_Tr);
    gPdfOps.set("Ts", PdfOp_Ts);

    gPdfOps.set("d0", PdfOp_d0);
    gPdfOps.set("d1", PdfOp_d1);

    gPdfOps.set("sh", PdfOp_sh);

    gPdfOps.set("Do", PdfOp_Do);

    gPdfOps.set("MP", PdfOp_MP);
    gPdfOps.set("DP", PdfOp_DP);
    gPdfOps.set("BMC", PdfOp_BMC);
    gPdfOps.set("BDC", PdfOp_BDC);
    gPdfOps.set("EMC", PdfOp_EMC);

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

    for (int i = 0 ; i < kCount_SkPdfResult; i++) {
        SkTDict<int>::Iter iter(gRenderStats[i]);
        const char* key;
        int value = 0;
        while ((key = iter.next(&value)) != NULL) {
            printf("%s: %s -> count %i\n", gRenderStatsNames[i], key, value);
        }
    }
}

SkPdfResult PdfMainLooper::consumeToken(PdfToken& token) {
    if (token.fType == kKeyword_TokenType && token.fKeywordLength < 256)
    {
        // TODO(edisonn): log trace flag (verbose, error, info, warning, ...)
        PdfOperatorRenderer pdfOperatorRenderer = NULL;
        if (gPdfOps.find(token.fKeyword, token.fKeywordLength, &pdfOperatorRenderer) && pdfOperatorRenderer) {
            // caller, main work is done by pdfOperatorRenderer(...)
            PdfTokenLooper* childLooper = NULL;
            SkPdfResult result = pdfOperatorRenderer(fPdfContext, fCanvas, &childLooper);

            int cnt = 0;
            gRenderStats[result].find(token.fKeyword, token.fKeywordLength, &cnt);
            gRenderStats[result].set(token.fKeyword, token.fKeywordLength, cnt + 1);

            if (childLooper) {
                childLooper->setUp(this);
                childLooper->loop();
                delete childLooper;
            }
        } else {
            int cnt = 0;
            gRenderStats[kUnsupported_SkPdfResult].find(token.fKeyword, token.fKeywordLength, &cnt);
            gRenderStats[kUnsupported_SkPdfResult].set(token.fKeyword, token.fKeywordLength, cnt + 1);
        }
    }
    else if (token.fType == kObject_TokenType)
    {
        fPdfContext->fObjectStack.push( token.fObject );
    }
    else {
        // TODO(edisonn): deine or use assert not reached
        return kIgnoreError_SkPdfResult;
    }
    return kOK_SkPdfResult;
}

void PdfMainLooper::loop() {
    PdfToken token;
    while (readToken(fTokenizer, &token)) {
        consumeToken(token);
    }
}

SkPdfResult PdfInlineImageLooper::consumeToken(PdfToken& token) {
    SkASSERT(false);
    return kIgnoreError_SkPdfResult;
}

void PdfInlineImageLooper::loop() {
    doXObject_Image(fPdfContext, fCanvas, fTokenizer->readInlineImage());
}

SkPdfResult PdfInlineImageLooper::done() {
    return kNYI_SkPdfResult;
}

SkPdfResult PdfCompatibilitySectionLooper::consumeToken(PdfToken& token) {
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

SkPdfContext* gPdfContext = NULL;

bool SkPdfRenderer::renderPage(int page, SkCanvas* canvas, const SkRect& dst) const {
    if (!fPdfDoc) {
        return false;
    }

    if (page < 0 || page >= pages()) {
        return false;
    }

    SkPdfContext pdfContext(fPdfDoc);

    pdfContext.fOriginalMatrix = SkMatrix::I();
    pdfContext.fGraphicsState.fResources = fPdfDoc->pageResources(page);

    gPdfContext = &pdfContext;

    // TODO(edisonn): get matrix stuff right.
    SkScalar z = SkIntToScalar(0);
    SkScalar w = dst.width();
    SkScalar h = dst.height();

    SkScalar wp = fPdfDoc->MediaBox(page).width();
    SkScalar hp = fPdfDoc->MediaBox(page).height();

    SkPoint pdfSpace[4] = {SkPoint::Make(z, z), SkPoint::Make(wp, z), SkPoint::Make(wp, hp), SkPoint::Make(z, hp)};
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

    pdfContext.fGraphicsState.fCTM = pdfContext.fOriginalMatrix;
    pdfContext.fGraphicsState.fContentStreamMatrix = pdfContext.fOriginalMatrix;
    pdfContext.fGraphicsState.fMatrixTm = pdfContext.fGraphicsState.fCTM;
    pdfContext.fGraphicsState.fMatrixTlm = pdfContext.fGraphicsState.fCTM;

#ifndef PDF_DEBUG_NO_PAGE_CLIPING
    canvas->clipRect(dst, SkRegion::kIntersect_Op, true);
#endif

    canvas->setMatrix(pdfContext.fOriginalMatrix);

    doPage(&pdfContext, canvas, fPdfDoc->page(page));

          // TODO(edisonn:) erase with white before draw?
//        SkPaint paint;
//        paint.setColor(SK_ColorWHITE);
//        canvas->drawRect(rect, paint);


    canvas->flush();
    return true;
}

bool SkPdfRenderer::load(const SkString inputFileName) {
    unload();

    // TODO(edisonn): create static function that could return NULL if there are errors
    fPdfDoc = new SkPdfNativeDoc(inputFileName.c_str());
    if (fPdfDoc->pages() == 0) {
        delete fPdfDoc;
        fPdfDoc = NULL;
    }

    return fPdfDoc != NULL;
}

bool SkPdfRenderer::load(SkStream* stream) {
    unload();

    // TODO(edisonn): create static function that could return NULL if there are errors
    fPdfDoc = new SkPdfNativeDoc(stream);
    if (fPdfDoc->pages() == 0) {
        delete fPdfDoc;
        fPdfDoc = NULL;
    }

    return fPdfDoc != NULL;
}


int SkPdfRenderer::pages() const {
    return fPdfDoc != NULL ? fPdfDoc->pages() : 0;
}

void SkPdfRenderer::unload() {
    delete fPdfDoc;
    fPdfDoc = NULL;
}

SkRect SkPdfRenderer::MediaBox(int page) const {
    SkASSERT(fPdfDoc);
    return fPdfDoc->MediaBox(page);
}

size_t SkPdfRenderer::bytesUsed() const {
    return fPdfDoc ? fPdfDoc->bytesUsed() : 0;
}

bool SkPDFNativeRenderToBitmap(SkStream* stream,
                               SkBitmap* output,
                               int page,
                               SkPdfContent content,
                               double dpi) {
    SkASSERT(page >= 0);
    SkPdfRenderer renderer;
    renderer.load(stream);
    if (!renderer.loaded() || page >= renderer.pages() || page < 0) {
        return false;
    }

    SkRect rect = renderer.MediaBox(page < 0 ? 0 :page);

    SkScalar width = SkScalarMul(rect.width(),  SkDoubleToScalar(sqrt(dpi / 72.0)));
    SkScalar height = SkScalarMul(rect.height(),  SkDoubleToScalar(sqrt(dpi / 72.0)));

    rect = SkRect::MakeWH(width, height);

    setup_bitmap(output, (int)SkScalarToDouble(width), (int)SkScalarToDouble(height));

    SkAutoTUnref<SkDevice> device(SkNEW_ARGS(SkDevice, (*output)));
    SkCanvas canvas(device);

    return renderer.renderPage(page, &canvas, rect);
}
