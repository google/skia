/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfRenderer.h"

#include "SkBitmapDevice.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkDevice.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkPdfFont.h"
#include "SkPdfGraphicsState.h"
#include "SkPdfHeaders_autogen.h"
#include "SkPdfMapper_autogen.h"
#include "SkPdfNativeTokenizer.h"
#include "SkPdfRenderer.h"
#include "SkPdfReporter.h"
#include "SkPdfTokenLooper.h"
#include "SkPdfUtils.h"
#include "SkStream.h"
#include "SkTypeface.h"
#include "SkTArray.h"
#include "SkTDict.h"

// TODO(edisonn): #ifdef these ones, as they are used only for debugging.
extern "C" SkPdfContext* gPdfContext;

__SK_FORCE_IMAGE_DECODER_LINKING;

// TODO(edisonn): tool, show what objects were read during rendering - will help to identify
//                features with incomplete implementation
// TODO(edisonn): security - validate all the user input, all pdf!
// TODO(edisonn): testability -add option to render without text, or only render text

// Helper macros to load variables from stack, and automatically check their type.
#define EXPECT_OPERANDS(name,pdfContext,n) \
    bool __failed = pdfContext->fObjectStack.count() < n; \
    SkPdfREPORTCODE(const char* __operator_name = name); \
    SkPdfREPORTCODE((void)__operator_name); \
    SkPdfReportIf(pdfContext->fObjectStack.count() < n, \
                  kIgnoreError_SkPdfIssueSeverity, \
                  kStackOverflow_SkPdfIssue, \
                  "Not enought parameters.", NULL, pdfContext); \
    SkDEBUGCODE(int __cnt = n);

#define POP_OBJ(pdfContext,name) \
    SkDEBUGCODE(__cnt--); \
    SkASSERT(__cnt >= 0); \
    SkPdfNativeObject* name = NULL; \
    __failed = __failed || pdfContext->fObjectStack.count() == 0; \
    if (pdfContext->fObjectStack.count() > 0) { \
        name = pdfContext->fObjectStack.top(); \
        pdfContext->fObjectStack.pop(); \
    }

// TODO(edisonn): make all pop function to use name##_obj
#define POP_NUMBER(pdfContext,name) \
    SkDEBUGCODE(__cnt--); \
    SkASSERT(__cnt >= 0); \
    double name = 0; \
    SkPdfNativeObject* name##_obj = NULL; \
    __failed = __failed || pdfContext->fObjectStack.count() == 0; \
    if (pdfContext->fObjectStack.count() > 0) { \
        name##_obj = pdfContext->fObjectStack.top(); \
        pdfContext->fObjectStack.pop(); \
        if (!name##_obj || !name##_obj->isNumber()) { \
            SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, \
                                      __operator_name, \
                                      name##_obj, \
                                      SkPdfNativeObject::_kNumber_PdfObjectType, \
                                      NULL);\
            __failed = true;\
        } else { \
            name = name##_obj->numberValue(); \
        } \
    }

#define POP_INTEGER(pdfContext,name) \
    SkDEBUGCODE(__cnt--); \
    SkASSERT(__cnt >= 0); \
    int64_t name = 0; \
    __failed = __failed || pdfContext->fObjectStack.count() == 0; \
    SkPdfNativeObject* name##_obj = NULL; \
    if (pdfContext->fObjectStack.count() > 0) { \
        name##_obj = pdfContext->fObjectStack.top(); \
        pdfContext->fObjectStack.pop(); \
        if (!name##_obj || !name##_obj->isInteger()) { \
            SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, \
                                      __operator_name, \
                                      name##_obj, \
                                      SkPdfNativeObject::kInteger_PdfObjectType, \
                                      NULL);\
            __failed = true;\
        } else { \
            name = name##_obj->intValue(); \
        } \
    }

#define POP_NUMBER_INTO(pdfContext,var) \
    SkDEBUGCODE(__cnt--); \
    SkASSERT(__cnt >= 0); \
    __failed = __failed || pdfContext->fObjectStack.count() == 0; \
    if (pdfContext->fObjectStack.count() > 0) { \
        SkPdfNativeObject* tmp = pdfContext->fObjectStack.top(); \
        pdfContext->fObjectStack.pop(); \
        if (!tmp || !tmp->isNumber()) { \
            SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, \
                                      __operator_name, \
                                      tmp, \
                                      SkPdfNativeObject::kInteger_PdfObjectType | \
                                          SkPdfNativeObject::kReal_PdfObjectType, \
                                      NULL);\
            __failed = true;\
        } else { \
            var = tmp->numberValue(); \
        } \
    }


#define POP_NAME(pdfContext,name) \
    SkDEBUGCODE(__cnt--); \
    SkASSERT(__cnt >= 0); \
    SkPdfNativeObject* name = NULL; \
    __failed = __failed || pdfContext->fObjectStack.count() == 0; \
    if (pdfContext->fObjectStack.count() > 0) { \
        SkPdfNativeObject* tmp = pdfContext->fObjectStack.top(); \
        pdfContext->fObjectStack.pop(); \
        if (!tmp || !tmp->isName()) { \
            SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, \
                                      __operator_name, \
                                      tmp, \
                                      SkPdfNativeObject::kName_PdfObjectType, \
                                      NULL);\
            __failed = true;\
        } else { \
            name = tmp; \
        } \
    }

#define POP_STRING(pdfContext,name) \
    SkDEBUGCODE(__cnt--); \
    SkASSERT(__cnt >= 0); \
    SkPdfNativeObject* name = NULL; \
    __failed = __failed || pdfContext->fObjectStack.count() == 0; \
    if (pdfContext->fObjectStack.count() > 0) { \
        SkPdfNativeObject* tmp = pdfContext->fObjectStack.top(); \
        pdfContext->fObjectStack.pop(); \
        if (!tmp || !tmp->isAnyString()) { \
            SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, \
                                      __operator_name, \
                                      tmp, \
                                      SkPdfNativeObject::kString_PdfObjectType | \
                                          SkPdfNativeObject::kHexString_PdfObjectType, \
                                      NULL);\
            __failed = true;\
        } else { \
            name = tmp; \
        } \
    }

#define POP_ARRAY(pdfContext,name) \
    SkDEBUGCODE(__cnt--); \
    SkASSERT(__cnt >= 0); \
    SkPdfArray* name = NULL; \
    __failed = __failed || pdfContext->fObjectStack.count() == 0; \
    if (pdfContext->fObjectStack.count() > 0) { \
        SkPdfNativeObject* tmp = pdfContext->fObjectStack.top(); \
        pdfContext->fObjectStack.pop(); \
        if (!tmp || !tmp->isArray()) { \
            SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, \
                                      __operator_name, \
                                      tmp, \
                                      SkPdfNativeObject::kArray_PdfObjectType, \
                                      NULL);\
            __failed = true;\
        } else { \
            name = (SkPdfArray*)tmp; \
        } \
    }

#define CHECK_PARAMETERS() \
    SkASSERT(__cnt == 0); \
    if (__failed) return kIgnoreError_SkPdfResult;


NotOwnedString strings_DeviceRGB;
NotOwnedString strings_DeviceCMYK;

class StringsInit {
public:
    StringsInit() {
        NotOwnedString::init(&strings_DeviceRGB, "DeviceRGB");
        NotOwnedString::init(&strings_DeviceCMYK, "DeviceCMYK");
    }
};

// TODO(edisonn): this will not work in chrome! Find another solution!
StringsInit gStringsInit;

// TODO(edisonn): Document SkPdfTokenLooper and subclasses.
class PdfInlineImageLooper : public SkPdfTokenLooper {
public:
    explicit PdfInlineImageLooper(SkPdfTokenLooper* parent)
        : INHERITED(parent) {}

    SkPdfResult consumeToken(PdfToken& token) override;
    void loop() override;

private:
    typedef SkPdfTokenLooper INHERITED;
};

class PdfCompatibilitySectionLooper : public SkPdfTokenLooper {
public:
    explicit PdfCompatibilitySectionLooper(SkPdfTokenLooper* parent)
        : INHERITED (parent) {}

    SkPdfResult consumeToken(PdfToken& token) override;
    void loop() override;

private:
    typedef SkPdfTokenLooper INHERITED;
};

// Utilities
static void setup_bitmap(SkBitmap* bitmap, int width, int height, SkColor color = SK_ColorWHITE) {
    bitmap->allocN32Pixels(width, height);
    bitmap->eraseColor(color);
}

// TODO(edisonn): synonyms? /DeviceRGB and /RGB mean the same thing. Context dependent.
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
    if (pdfArray == NULL) {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kNullObject_SkPdfIssue,
                    "null array passed to build matrix", NULL, NULL);
        return SkMatrix::I();
    }

    if (pdfArray->size() != 6) {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kUnexpectedArraySize_SkPdfIssue,
                    "null array passed to build matrix", pdfArray, NULL);
        return SkMatrix::I();
    }

    for (int i = 0; i < 6; i++) {
        const SkPdfNativeObject* elem = pdfArray->operator [](i);
        if (elem == NULL || !elem->isNumber()) {
            SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, NULL, elem,
                                      SkPdfNativeObject::_kNumber_PdfObjectType, NULL);
            return SkMatrix::I();
        }
        array[i] = elem->numberValue();
    }

    return SkMatrixFromPdfMatrix(array);
}

// TODO(edisonn): debug code, used to analyze rendering when we find bugs.
extern "C" SkPdfNativeDoc* gDoc;

static SkPdfResult DrawText(SkPdfContext* pdfContext,
                   const SkPdfNativeObject* _str,
                   SkCanvas* canvas)
{
    SkPdfFont* skfont = pdfContext->fGraphicsState.fSkFont;
    if (skfont == NULL) {
        skfont = SkPdfFont::Default();
    }

    if (_str == NULL || !_str->isAnyString()) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity,
                                  "DrawText",
                                  _str,
                                  SkPdfNativeObject::_kAnyString_PdfObjectType,
                                  pdfContext);
        return kIgnoreError_SkPdfResult;
    }
    const SkPdfString* str = (const SkPdfString*)_str;

    SkUnencodedText binary(str);

    SkDecodedText decoded;

    if (skfont->encoding() == NULL) {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kMissingEncoding_SkPdfIssue,
                    "draw text", _str, pdfContext);
        return kNYI_SkPdfResult;
    }

    skfont->encoding()->decodeText(binary, &decoded);

    SkPaint paint;
    // TODO(edisonn): does size 0 mean anything special?
    if (pdfContext->fGraphicsState.fCurFontSize != 0) {
        paint.setTextSize(SkDoubleToScalar(pdfContext->fGraphicsState.fCurFontSize));
    }

    // TODO(edisonn): implement font scaler
//    if (fCurFont && fCurFont->GetFontScale() != 0) {
//        paint.setTextScaleX(fCurFont->GetFontScale() / 100.0);
//    }

    pdfContext->fGraphicsState.applyGraphicsState(&paint, false);

    skfont->drawText(decoded, &paint, pdfContext, canvas);

    return kOK_SkPdfResult;
}

// TODO(edisonn): create header files with declarations!
SkPdfResult PdfOp_q(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper* parentLooper);
SkPdfResult PdfOp_Q(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper* parentLooper);
SkPdfResult PdfOp_Tw(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper* parentLooper);
SkPdfResult PdfOp_Tc(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper* parentLooper);

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

static SkBitmap* transferImageStreamToBitmap(const unsigned char* uncompressedStream,
                                             size_t uncompressedStreamLength,
                                             int width, int height, int bytesPerLine,
                                             int bpc, const SkString& colorSpace,
                                             bool transparencyMask) {
    SkBitmap* bitmap = new SkBitmap();

    //int components = GetColorSpaceComponents(colorSpace);
//#define MAX_COMPONENTS 10

    // TODO(edisonn): assume start of lines are aligned at 32 bits?
    // Is there a faster way to load the uncompressed stream into a bitmap?

    // minimal support for now
    if ((colorSpace.equals("DeviceRGB") || colorSpace.equals("RGB")) && bpc == 8) {
        uint32_t* uncompressedStreamArgb = (SkColor*)malloc(width * height * sizeof(uint32_t));

        for (int h = 0 ; h < height; h++) {
            long i = width * (h);
            for (int w = 0 ; w < width; w++) {
                uncompressedStreamArgb[i] = SkPackARGB32(0xFF,
                                                         uncompressedStream[3 * w],
                                                         uncompressedStream[3 * w + 1],
                                                         uncompressedStream[3 * w + 2]);
                i++;
            }
            uncompressedStream += bytesPerLine;
        }

        const SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
        bitmap->installPixels(info, uncompressedStreamArgb, info.minRowBytes());
    }
    else if ((colorSpace.equals("DeviceGray") || colorSpace.equals("Gray")) && bpc == 8) {
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

        const SkColorType ct = transparencyMask ? kAlpha_8_SkColorType : kIndex_8_SkColorType;
        const SkImageInfo info = SkImageInfo::Make(width, height, ct, kPremul_SkAlphaType);
        bitmap->installPixels(info, uncompressedStreamA8, info.minRowBytes(),
                              transparencyMask ? NULL : getGrayColortable(), NULL, NULL);
    }

    // TODO(edisonn): pass color space and context here?
    SkPdfReport(kCodeWarning_SkPdfIssueSeverity, kNYI_SkPdfIssue, "Color space NYI", NULL, NULL);
    return bitmap;
}
// TODO(edisonn): preserve A1 format that skia knows, + fast convert from 111, 222, 444 to closest
// skia format.

// This functions returns the image, it does not look at the smask.
static SkBitmap* getImageFromObjectCore(SkPdfContext* pdfContext,
                                        SkPdfImageDictionary* image, bool transparencyMask) {
    if (image == NULL || !image->hasStream()) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, "Missing stream", image,
                                  SkPdfNativeObject::_kStream_PdfObjectType, pdfContext);
        return NULL;
    }

    int bpc = (int)image->BitsPerComponent(pdfContext->fPdfDoc);
    int width = (int)image->Width(pdfContext->fPdfDoc);
    int height = (int)image->Height(pdfContext->fPdfDoc);
    SkString colorSpace("DeviceRGB");

    bool indexed = false;
    SkPMColor colors[256];
    int cnt = 0;

    if (image->isColorSpaceAName(pdfContext->fPdfDoc)) {
        colorSpace = image->getColorSpaceAsName(pdfContext->fPdfDoc);
    } else if (image->isColorSpaceAArray(pdfContext->fPdfDoc)) {
        SkPdfArray* array = image->getColorSpaceAsArray(pdfContext->fPdfDoc);
        if (array && array->size() == 4 && array->objAtAIndex(0)->isName("Indexed") &&
                                           (array->objAtAIndex(1)->isName("DeviceRGB") ||
                                                   array->objAtAIndex(1)->isName("RGB")) &&
                                           array->objAtAIndex(2)->isInteger() &&
                                           array->objAtAIndex(3)->isHexString()
                                           ) {
            SkPdfReport(kCodeWarning_SkPdfIssueSeverity, kNYI_SkPdfIssue, "Color space NYI",
                        image, pdfContext);
            indexed = true;
            cnt = (int)array->objAtAIndex(2)->intValue() + 1;
            if (cnt > 256) {
                SkPdfReport(kCodeWarning_SkPdfIssueSeverity, kNYI_SkPdfIssue,
                            "Color space feature NYI, cnt > 256", image, pdfContext);
                return NULL;
            }
            NotOwnedString data = array->objAtAIndex(3)->strRef();
            if (data.fBytes != (unsigned int)cnt * 3) {
                SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kIncostistentSizes_SkPdfIssue,
                            "Image color table mismatch color space specs", array, pdfContext);
                return NULL;
            }
            for (int i = 0 ; i < cnt; i++) {
                colors[i] = SkPreMultiplyARGB(0xff,
                                              data.fBuffer[3 * i],
                                              data.fBuffer[3 * i + 1],
                                              data.fBuffer[3 * i + 2]);
            }
        }
    }

    //  TODO(edisonn): implement image masks.
/*  bool imageMask = image->imageMask();
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
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, "Missing stream", stream,
                                  SkPdfNativeObject::_kStream_PdfObjectType, pdfContext);
        return NULL;
    }

    SkPdfStreamCommonDictionary* streamDict = (SkPdfStreamCommonDictionary*)stream;

    if (streamDict->has_Filter() &&
            ((streamDict->isFilterAName(NULL) &&
                  streamDict->getFilterAsName(NULL).equals("DCTDecode")) ||
             (streamDict->isFilterAArray(NULL) &&
                  streamDict->getFilterAsArray(NULL)->size() > 0 &&
                  streamDict->getFilterAsArray(NULL)->objAtAIndex(0)->isName() &&
                  streamDict->getFilterAsArray(NULL)->objAtAIndex(0)->nameValue2()
                                                                    .equals("DCTDecode")))) {
        SkBitmap* bitmap = new SkBitmap();
        SkImageDecoder::DecodeMemory(uncompressedStream, uncompressedStreamLength, bitmap);
        return bitmap;
    }

    // TODO(edisonn): assumes RGB for now, since it is the only one implemented
    if (indexed) {
        SkBitmap* bitmap = new SkBitmap();
        const SkImageInfo info = SkImageInfo::Make(width, height, kIndex_8_SkColorType,
                                                   kPremul_SkAlphaType);
        SkAutoTUnref<SkColorTable> colorTable(new SkColorTable(colors, cnt));
        bitmap->installPixels(info, (void*)uncompressedStream, info.minRowBytes(), colorTable,
                              NULL, NULL);
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

static SkBitmap* getImageFromObject(SkPdfContext* pdfContext, SkPdfImageDictionary* image,
                                    bool transparencyMask) {
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
    SkPdfReport(kCodeWarning_SkPdfIssueSeverity, kNYI_SkPdfIssue,
                "implement GS SMask. Default to empty right now.", obj, pdfContext);

    return pdfContext->fGraphicsState.fSMask;
}

static SkPdfResult doXObject_Image(SkPdfContext* pdfContext, SkCanvas* canvas,
                                   SkPdfImageDictionary* skpdfimage) {
    if (skpdfimage == NULL) {
        return kIgnoreError_SkPdfResult;
    }

    SkBitmap* image = getImageFromObject(pdfContext, skpdfimage, false);
    SkBitmap* sMask = getSmaskFromObject(pdfContext, skpdfimage);

    canvas->save();
    canvas->setMatrix(pdfContext->fGraphicsState.fCTM);

    SkScalar z = SkIntToScalar(0);
    SkScalar one = SkIntToScalar(1);

    SkPoint from[4] = {SkPoint::Make(z, z), SkPoint::Make(one, z),
                       SkPoint::Make(one, one), SkPoint::Make(z, one)};
    SkPoint to[4] = {SkPoint::Make(z, one), SkPoint::Make(one, one),
                     SkPoint::Make(one, z), SkPoint::Make(z, z)};
    SkMatrix flip;
    SkAssertResult(flip.setPolyToPoly(from, to, 4));
    SkMatrix solveImageFlip = pdfContext->fGraphicsState.fCTM;
    solveImageFlip.preConcat(flip);
    canvas->setMatrix(solveImageFlip);

#ifdef PDF_TRACE
    SkPoint final[4] = {SkPoint::Make(z, z), SkPoint::Make(one, z),
                        SkPoint::Make(one, one), SkPoint::Make(z, one)};
    solveImageFlip.mapPoints(final, 4);
    printf("IMAGE rect = ");
    for (int i = 0; i < 4; i++) {
        printf("(%f %f) ", SkScalarToDouble(final[i].x()), SkScalarToDouble(final[i].y()));
    }
    printf("\n");
#endif  // PDF_TRACE

    SkRect dst = SkRect::MakeXYWH(SkDoubleToScalar(0.0), SkDoubleToScalar(0.0),
                                  SkDoubleToScalar(1.0), SkDoubleToScalar(1.0));

    // TODO(edisonn): soft mask type? alpha/luminosity.
    SkPdfReport(kCodeWarning_SkPdfIssueSeverity, kNYI_SkPdfIssue,
                "implement soft mask type", skpdfimage, pdfContext);

    SkPaint paint;
    pdfContext->fGraphicsState.applyGraphicsState(&paint, false);

    if (!sMask || sMask->empty()) {
        canvas->drawBitmapRect(*image, dst, &paint);
    } else {
        canvas->saveLayer(&dst, &paint);
        canvas->drawBitmapRect(*image, dst, NULL);
        SkPaint xfer;
        xfer.setXfermodeMode(SkXfermode::kSrcOut_Mode);
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
//     CON: multiple drawings (but on smaller areas), pay a price at loading pdf to
//          compute a pdf draw plan
//          on average, a load with empty draw is 100ms on all the skps we have, for complete sites
// 3) support them natively in SkCanvas
//     PRO: simple
//     CON: we would still need to use a form of readPixels anyway, so perf might be the same as 1)
// 4) compile a plan using pathops, and render once without any fancy rules with backdrop
//     PRO: simple, fast
//     CON: pathops must be bug free first + time to compute new paths
//          pay a price at loading pdf to compute a pdf draw plan
//          on average, a load with empty draw is 100ms on all the skps we have, for complete sites
// 5) for knockout, render the objects in reverse order, and add every object to the clip, and any
//          new draw will be cliped

static void doGroup_before(SkPdfContext* pdfContext, SkCanvas* canvas, SkRect bbox,
                           SkPdfTransparencyGroupDictionary* tgroup, bool page) {
    SkRect bboxOrig = bbox;
    SkBitmap backdrop;
    bool isolatedGroup = tgroup->I(pdfContext->fPdfDoc);
//  bool knockoutGroup = tgroup->K(pdfContext->fPdfDoc);
    SkPaint paint;
    pdfContext->fGraphicsState.applyGraphicsState(&paint, false);
    canvas->saveLayer(&bboxOrig, isolatedGroup ? &paint : NULL);
}

// TODO(edisonn): non isolation should probably be implemented in skia
//static void doGroup_after(SkPdfContext* pdfContext, SkCanvas* canvas, SkRect bbox,
//                          SkPdfTransparencyGroupDictionary* tgroup) {
//    if not isolated
//        canvas->drawBitmapRect(backdrop, bboxOrig, NULL);
//}

static SkPdfResult doXObject_Form(SkPdfContext* pdfContext, SkCanvas* canvas,
                                  SkPdfType1FormDictionary* skobj) {
    if (!skobj || !skobj->hasStream()) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, "Missing stream", skobj,
                                  SkPdfNativeObject::_kStream_PdfObjectType, pdfContext);
        return kIgnoreError_SkPdfResult;
    }

    if (!skobj->has_BBox()) {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kMissingRequiredKey_SkPdfIssue, "BBox",
                    skobj, pdfContext);
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
        // TODO(edisonn): text matrixes mosltly NYI
    }

    SkTraceMatrix(pdfContext->fGraphicsState.fCTM, "Total matrix");
    pdfContext->fGraphicsState.fContentStreamMatrix = pdfContext->fGraphicsState.fCTM;

    canvas->setMatrix(pdfContext->fGraphicsState.fCTM);

    SkRect bbox = skobj->BBox(pdfContext->fPdfDoc);
    // TODO(edisonn): constants (AA) from settings.
    canvas->clipRect(bbox, SkRegion::kIntersect_Op, false);

    // This is a group?
    if (skobj->has_Group()) {
        SkPdfTransparencyGroupDictionary* tgroup = skobj->Group(pdfContext->fPdfDoc);
        doGroup_before(pdfContext, canvas, bbox, tgroup, false);
    }

    SkPdfStream* stream = (SkPdfStream*)skobj;

    pdfContext->parseStream(stream, canvas);

    if (skobj->has_Group()) {
        canvas->restore();
    }

    PdfOp_Q(pdfContext, canvas, NULL);
    return kPartial_SkPdfResult;
}

static SkPdfResult doXObject_Pattern(SkPdfContext* pdfContext, SkCanvas* canvas,
                                     SkPdfType1PatternDictionary* skobj) {
    if (!skobj || !skobj->hasStream()) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, "Missing stream",
                                  skobj, SkPdfNativeObject::_kStream_PdfObjectType, pdfContext);
        return kIgnoreError_SkPdfResult;
    }

    if (!skobj->has_BBox()) {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kMissingRequiredKey_SkPdfIssue, "BBox",
                    skobj, pdfContext);
        return kIgnoreError_SkPdfResult;
    }

    PdfOp_q(pdfContext, canvas, NULL);


    if (skobj->Resources(pdfContext->fPdfDoc)) {
        pdfContext->fGraphicsState.fResources = skobj->Resources(pdfContext->fPdfDoc);
    }

    SkTraceMatrix(pdfContext->fGraphicsState.fContentStreamMatrix, "Current Content stream matrix");

    if (skobj->has_Matrix()) {
        pdfContext->fGraphicsState.fContentStreamMatrix.preConcat(
                skobj->Matrix(pdfContext->fPdfDoc));
    }

    SkTraceMatrix(pdfContext->fGraphicsState.fContentStreamMatrix, "Total Content stream matrix");

    canvas->setMatrix(pdfContext->fGraphicsState.fContentStreamMatrix);
    pdfContext->fGraphicsState.fCTM = pdfContext->fGraphicsState.fContentStreamMatrix;

    SkRect bbox = skobj->BBox(pdfContext->fPdfDoc);
    // TODO(edisonn): constants (AA) from settings.
    canvas->clipRect(bbox, SkRegion::kIntersect_Op, false);

    SkPdfStream* stream = (SkPdfStream*)skobj;

    pdfContext->parseStream(stream, canvas);

    PdfOp_Q(pdfContext, canvas, NULL);
    return kPartial_SkPdfResult;
}

// TODO(edisonn): PS NYI
//static SkPdfResult doXObject_PS(SkPdfContext* pdfContext, SkCanvas* canvas,
//                                const SkPdfNativeObject* obj) {
//    return kNYI_SkPdfResult;
//}

SkPdfResult doType3Char(SkPdfContext* pdfContext, SkCanvas* canvas, const SkPdfNativeObject* skobj,
                        SkRect bBox, SkMatrix matrix, double textSize) {
    if (!skobj || !skobj->hasStream()) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, "Missing stream", skobj,
                                  SkPdfNativeObject::_kStream_PdfObjectType, pdfContext);
        return kIgnoreError_SkPdfResult;
    }

    PdfOp_q(pdfContext, canvas, NULL);

    pdfContext->fGraphicsState.fMatrixTm.preConcat(matrix);
    pdfContext->fGraphicsState.fMatrixTm.preScale(SkDoubleToScalar(textSize),
                                                  SkDoubleToScalar(textSize));
    pdfContext->fGraphicsState.fMatrixTlm = pdfContext->fGraphicsState.fMatrixTm;

    pdfContext->fGraphicsState.fCTM = pdfContext->fGraphicsState.fMatrixTm;
    pdfContext->fGraphicsState.fCTM.preScale(SkDoubleToScalar(1), SkDoubleToScalar(-1));

    SkTraceMatrix(pdfContext->fGraphicsState.fCTM, "Total matrix");

    canvas->setMatrix(pdfContext->fGraphicsState.fCTM);

    SkRect rm = bBox;
    pdfContext->fGraphicsState.fCTM.mapRect(&rm);

    SkTraceRect(rm, "bbox mapped");

    // TODO(edisonn): constants (AA) from settings.
    canvas->clipRect(bBox, SkRegion::kIntersect_Op, false);

    SkPdfStream* stream = (SkPdfStream*)skobj;

    pdfContext->parseStream(stream, canvas);

    PdfOp_Q(pdfContext, canvas, NULL);

    return kPartial_SkPdfResult;
}

// The PDF could be corrupted so a dict refers recursively to the same dict, if this happens
// we end up with a stack overflow and crash.
class CheckRecursiveRendering {
    SkPdfNativeObject* fObj;
public:
    CheckRecursiveRendering(SkPdfNativeObject* obj) : fObj(obj) {
        SkASSERT(!obj->inRendering());
        obj->startRendering();
    }

    ~CheckRecursiveRendering() {
        SkASSERT(fObj->inRendering());
        fObj->doneRendering();
    }

    static bool IsInRendering(const SkPdfNativeObject* obj) {
        return obj->inRendering();
    }
};

static SkPdfResult doXObject(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfNativeObject* obj) {
    if (CheckRecursiveRendering::IsInRendering(obj)) {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kRecursiveReferencing_SkPdfIssue,
                    "Recursive reverencing is invalid in draw objects", obj, pdfContext);
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
            if (pdfContext->fPdfDoc->mapper()->mapType1PatternDictionary(obj) !=
                    kNone_SkPdfNativeObjectType) {
                SkPdfType1PatternDictionary* pattern = (SkPdfType1PatternDictionary*)obj;
                return doXObject_Pattern(pdfContext, canvas, pattern);
            }
            SkPdfReport(kCodeWarning_SkPdfIssueSeverity, kNYI_SkPdfIssue, "doXObject",
                        obj, pdfContext);
        }
    }
    return kIgnoreError_SkPdfResult;
}

static SkPdfResult doPage(SkPdfContext* pdfContext, SkCanvas* canvas,
                          SkPdfPageObjectDictionary* skobj) {
    if (!skobj || !skobj->isContentsAStream(pdfContext->fPdfDoc)) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, "Missing stream", skobj,
                                  SkPdfNativeObject::_kStream_PdfObjectType, pdfContext);
        return kNYI_SkPdfResult;
    }

    SkPdfStream* stream = skobj->getContentsAsStream(pdfContext->fPdfDoc);

    if (!stream) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, "Missing stream",
                                  skobj, SkPdfNativeObject::_kStream_PdfObjectType, pdfContext);
        return kIgnoreError_SkPdfResult;
    }

    // FIXME (scroggo): renderPage also sets fResources. Are these redundant?
    pdfContext->fGraphicsState.fResources = skobj->Resources(pdfContext->fPdfDoc);

    if (!pdfContext->fGraphicsState.fResources) {
        // It might be null because we have not implemented yet inheritance.
        return kIgnoreError_SkPdfResult;
    }

    if (CheckRecursiveRendering::IsInRendering(skobj)) {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kRecursiveReferencing_SkPdfIssue,
                    "Recursive reverencing is invalid in draw objects", skobj, pdfContext);
        return kIgnoreError_SkPdfResult;
    }
    CheckRecursiveRendering checkRecursion(skobj);


    // FIXME (scroggo): Is this save necessary? May be needed for rendering a nested PDF.
    PdfOp_q(pdfContext, canvas, NULL);

    // TODO(edisonn): MediaBox can be inherited!!!!
    SkPdfReport(kCodeWarning_SkPdfIssueSeverity, kNYI_SkPdfIssue, "MediaBox inheritance NYI",
                NULL, pdfContext);
    SkRect bbox = skobj->MediaBox(pdfContext->fPdfDoc);
    if (skobj->has_Group()) {
        SkPdfTransparencyGroupDictionary* tgroup = skobj->Group(pdfContext->fPdfDoc);
        doGroup_before(pdfContext, canvas, bbox, tgroup, true);
    } else {
        canvas->save();
    }

    pdfContext->parseStream(stream, canvas);

    canvas->restore();
    PdfOp_Q(pdfContext, canvas, NULL);
    return kPartial_SkPdfResult;
}

SkPdfResult PdfOp_q(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    pdfContext->fStateStack.push(pdfContext->fGraphicsState);
    canvas->save();
    pdfContext->fObjectStack.nest();
    return kOK_SkPdfResult;
}

SkPdfResult PdfOp_Q(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    if (pdfContext->fStateStack.count() > 0) {
        pdfContext->fGraphicsState = pdfContext->fStateStack.top();
        pdfContext->fStateStack.pop();
        canvas->restore();

        if (pdfContext->fObjectStack.nestingLevel() == 0) {
            SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kStackNestingOverflow_SkPdfIssue,
                        "stack nesting overflow (q/Q)", NULL, pdfContext);
            return kIgnoreError_SkPdfResult;
        } else {
            pdfContext->fObjectStack.unnest();
        }
    } else {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kStackOverflow_SkPdfIssue,
                    "stack overflow (q/Q)", NULL, pdfContext);
        return kIgnoreError_SkPdfResult;
    }

    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_cm(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("cm", pdfContext, 6);
    POP_NUMBER(pdfContext, f);
    POP_NUMBER(pdfContext, e);
    POP_NUMBER(pdfContext, d);
    POP_NUMBER(pdfContext, c);
    POP_NUMBER(pdfContext, b);
    POP_NUMBER(pdfContext, a);
    CHECK_PARAMETERS();
    double array[6] = {a, b, c, d, e, f};

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
static SkPdfResult PdfOp_TL(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("TL", pdfContext, 1);
    POP_NUMBER(pdfContext, ty);
    CHECK_PARAMETERS();

    pdfContext->fGraphicsState.fTextLeading = ty;

    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_Td(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("Td", pdfContext, 2);
    POP_NUMBER(pdfContext, ty);
    POP_NUMBER(pdfContext, tx);
    CHECK_PARAMETERS();

    double array[6] = {1, 0, 0, 1, tx, -ty};
    SkMatrix matrix = SkMatrixFromPdfMatrix(array);

    pdfContext->fGraphicsState.fMatrixTm.preConcat(matrix);
    pdfContext->fGraphicsState.fMatrixTlm.preConcat(matrix);

    return kPartial_SkPdfResult;
}

static SkPdfResult PdfOp_TD(SkPdfContext* pdfContext, SkCanvas* canvas,
                            SkPdfTokenLooper* parentLooper) {
    EXPECT_OPERANDS("TD", pdfContext, 2)
    POP_NUMBER(pdfContext, ty);
    POP_NUMBER(pdfContext, tx);
    CHECK_PARAMETERS();

    // TODO(edisonn): Create factory methods or constructors so native is hidden
    SkPdfReal* _ty = pdfContext->fPdfDoc->createReal(-ty);
    pdfContext->fObjectStack.push(_ty);

    PdfOp_TL(pdfContext, canvas, parentLooper);

    SkPdfReal* vtx = pdfContext->fPdfDoc->createReal(tx);
    pdfContext->fObjectStack.push(vtx);

    SkPdfReal* vty = pdfContext->fPdfDoc->createReal(ty);
    pdfContext->fObjectStack.push(vty);

    SkPdfResult ret = PdfOp_Td(pdfContext, canvas, parentLooper);

    return ret;
}

static SkPdfResult PdfOp_Tm(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("Tm", pdfContext, 6);
    POP_NUMBER(pdfContext, f);
    POP_NUMBER(pdfContext, e);
    POP_NUMBER(pdfContext, d);
    POP_NUMBER(pdfContext, c);
    POP_NUMBER(pdfContext, b);
    POP_NUMBER(pdfContext, a);
    CHECK_PARAMETERS();

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

    // TODO(edisonn): NYI - Text positioning.
    SkPdfReport(kCodeWarning_SkPdfIssueSeverity, kNYI_SkPdfIssue,
                "Text positioning not implemented for 2+ chars", NULL, pdfContext);

    pdfContext->fGraphicsState.fMatrixTm = matrix;
    pdfContext->fGraphicsState.fMatrixTlm = matrix;

    return kPartial_SkPdfResult;
}

//— T* Move to the start of the next line. This operator has the same effect as the code
//0 Tl Td
//where Tl is the current leading parameter in the text state
static SkPdfResult PdfOp_T_star(SkPdfContext* pdfContext, SkCanvas* canvas,
                                SkPdfTokenLooper* parentLooper) {
    SkPdfReal* zero = pdfContext->fPdfDoc->createReal(0.0);
    SkPdfReal* tl = pdfContext->fPdfDoc->createReal(pdfContext->fGraphicsState.fTextLeading);

    pdfContext->fObjectStack.push(zero);
    pdfContext->fObjectStack.push(tl);

    SkPdfResult ret = PdfOp_Td(pdfContext, canvas, parentLooper);

    return ret;
}

static SkPdfResult PdfOp_m(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    if (pdfContext->fGraphicsState.fPathClosed) {
        pdfContext->fGraphicsState.fPath.reset();
        pdfContext->fGraphicsState.fPathClosed = false;
    }

    EXPECT_OPERANDS("m", pdfContext, 2);
    POP_NUMBER(pdfContext, y);
    POP_NUMBER(pdfContext, x);
    CHECK_PARAMETERS();

    pdfContext->fGraphicsState.fCurPosY = y;
    pdfContext->fGraphicsState.fCurPosX = x;

    pdfContext->fGraphicsState.fPath.moveTo(SkDoubleToScalar(pdfContext->fGraphicsState.fCurPosX),
                                            SkDoubleToScalar(pdfContext->fGraphicsState.fCurPosY));

    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_l(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    if (pdfContext->fGraphicsState.fPathClosed) {
        pdfContext->fGraphicsState.fPath.reset();
        pdfContext->fGraphicsState.fPathClosed = false;
    }

    EXPECT_OPERANDS("l", pdfContext, 2);
    POP_NUMBER(pdfContext, y);
    POP_NUMBER(pdfContext, x);
    CHECK_PARAMETERS();

    pdfContext->fGraphicsState.fCurPosY = y;
    pdfContext->fGraphicsState.fCurPosX = x;

    pdfContext->fGraphicsState.fPath.lineTo(SkDoubleToScalar(pdfContext->fGraphicsState.fCurPosX),
                                            SkDoubleToScalar(pdfContext->fGraphicsState.fCurPosY));

    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_c(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    if (pdfContext->fGraphicsState.fPathClosed) {
        pdfContext->fGraphicsState.fPath.reset();
        pdfContext->fGraphicsState.fPathClosed = false;
    }

    EXPECT_OPERANDS("c", pdfContext, 6);
    POP_NUMBER(pdfContext, y3);
    POP_NUMBER(pdfContext, x3);
    POP_NUMBER(pdfContext, y2);
    POP_NUMBER(pdfContext, x2);
    POP_NUMBER(pdfContext, y1);
    POP_NUMBER(pdfContext, x1);
    CHECK_PARAMETERS();

    pdfContext->fGraphicsState.fPath.cubicTo(SkDoubleToScalar(x1), SkDoubleToScalar(y1),
                                             SkDoubleToScalar(x2), SkDoubleToScalar(y2),
                                             SkDoubleToScalar(x3), SkDoubleToScalar(y3));

    pdfContext->fGraphicsState.fCurPosX = x3;
    pdfContext->fGraphicsState.fCurPosY = y3;

    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_v(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    if (pdfContext->fGraphicsState.fPathClosed) {
        pdfContext->fGraphicsState.fPath.reset();
        pdfContext->fGraphicsState.fPathClosed = false;
    }

    EXPECT_OPERANDS("v", pdfContext, 4);
    POP_NUMBER(pdfContext, y3);
    POP_NUMBER(pdfContext, x3);
    POP_NUMBER(pdfContext, y2);
    POP_NUMBER(pdfContext, x2);
    CHECK_PARAMETERS();

    double y1 = pdfContext->fGraphicsState.fCurPosY;
    double x1 = pdfContext->fGraphicsState.fCurPosX;

    pdfContext->fGraphicsState.fPath.cubicTo(SkDoubleToScalar(x1), SkDoubleToScalar(y1),
                                             SkDoubleToScalar(x2), SkDoubleToScalar(y2),
                                             SkDoubleToScalar(x3), SkDoubleToScalar(y3));

    pdfContext->fGraphicsState.fCurPosX = x3;
    pdfContext->fGraphicsState.fCurPosY = y3;

    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_y(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    if (pdfContext->fGraphicsState.fPathClosed) {
        pdfContext->fGraphicsState.fPath.reset();
        pdfContext->fGraphicsState.fPathClosed = false;
    }

    EXPECT_OPERANDS("y", pdfContext, 4);
    POP_NUMBER(pdfContext, y3);
    POP_NUMBER(pdfContext, x3);
    POP_NUMBER(pdfContext, y1);
    POP_NUMBER(pdfContext, x1);
    CHECK_PARAMETERS();

    double y2 = pdfContext->fGraphicsState.fCurPosY;
    double x2 = pdfContext->fGraphicsState.fCurPosX;

    pdfContext->fGraphicsState.fPath.cubicTo(SkDoubleToScalar(x1), SkDoubleToScalar(y1),
                                             SkDoubleToScalar(x2), SkDoubleToScalar(y2),
                                             SkDoubleToScalar(x3), SkDoubleToScalar(y3));

    pdfContext->fGraphicsState.fCurPosX = x3;
    pdfContext->fGraphicsState.fCurPosY = y3;

    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_re(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    if (pdfContext->fGraphicsState.fPathClosed) {
        pdfContext->fGraphicsState.fPath.reset();
        pdfContext->fGraphicsState.fPathClosed = false;
    }

    EXPECT_OPERANDS("re", pdfContext, 4);
    POP_NUMBER(pdfContext, height);
    POP_NUMBER(pdfContext, width);
    POP_NUMBER(pdfContext, y);
    POP_NUMBER(pdfContext, x);
    CHECK_PARAMETERS();

    pdfContext->fGraphicsState.fPath.addRect(SkDoubleToScalar(x),
                                             SkDoubleToScalar(y),
                                             SkDoubleToScalar(x + width),
                                             SkDoubleToScalar(y + height));

    pdfContext->fGraphicsState.fCurPosX = x;
    pdfContext->fGraphicsState.fCurPosY = y + height;

    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_h(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    pdfContext->fGraphicsState.fPath.close();
    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_fillAndStroke(SkPdfContext* pdfContext, SkCanvas* canvas,
                                       bool fill, bool stroke, bool close, bool evenOdd) {
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
            if (strncmp((char*)pdfContext->fGraphicsState.fNonStroking.fColorSpace.fBuffer,
                        "Pattern", strlen("Pattern")) == 0 &&
                pdfContext->fGraphicsState.fNonStroking.fPattern != NULL) {

                // TODO(edisonn): we can use a shader here, like imageshader to draw fast.

                PdfOp_q(pdfContext, canvas, NULL);

                if (evenOdd) {
                    path.setFillType(SkPath::kEvenOdd_FillType);
                }
                canvas->clipPath(path);

                if (pdfContext->fPdfDoc
                              ->mapper()
                              ->mapType1PatternDictionary(pdfContext->fGraphicsState
                                                                    .fNonStroking
                                                                    .fPattern)
                                                         != kNone_SkPdfNativeObjectType) {
                    SkPdfType1PatternDictionary* pattern
                            = (SkPdfType1PatternDictionary*)pdfContext->fGraphicsState
                                                                      .fNonStroking
                                                                      .fPattern;

                    // TODO(edisonn): make PaintType constants
                    if (pattern->PaintType(pdfContext->fPdfDoc) == 1) {
                        // TODO(edisonn): don't use abs, iterate as asked, if the cells intersect
                        // it will change the result iterating in reverse
                        // remove then the following  bounds.sort();
                        int xStep = abs((int)pattern->XStep(pdfContext->fPdfDoc));
                        int yStep = abs((int)pattern->YStep(pdfContext->fPdfDoc));

                        SkPdfReport(kCodeWarning_SkPdfIssueSeverity, kNYI_SkPdfIssue,
                                    "paterns x/y step is forced to positive number",
                                    pattern, pdfContext);

                        SkRect bounds = path.getBounds();
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

                                pdfContext->fGraphicsState.fContentStreamMatrix.preTranslate(
                                        SkIntToScalar(xStep), SkIntToScalar(0));
                                totalx += xStep;
                                x += SkIntToScalar(xStep);
                            }
                            pdfContext->fGraphicsState.fContentStreamMatrix.preTranslate(
                                    SkIntToScalar(-totalx), SkIntToScalar(0));

                            pdfContext->fGraphicsState.fContentStreamMatrix.preTranslate(
                                    SkIntToScalar(0), SkIntToScalar(-yStep));
                            totaly += yStep;
                            y += SkIntToScalar(yStep);
                        }
                        pdfContext->fGraphicsState.fContentStreamMatrix.preTranslate(
                                SkIntToScalar(0), SkIntToScalar(totaly));
                    }
                }

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
            if (false && strncmp((char*)pdfContext->fGraphicsState.fNonStroking.fColorSpace.fBuffer,
                                 "Pattern", strlen("Pattern")) == 0) {
                // TODO(edisonn): implement Pattern for strokes
                paint.setStyle(SkPaint::kStroke_Style);

                paint.setColor(SK_ColorGREEN);

                // reset it, just in case it messes up the stroke
                path.setFillType(SkPath::kWinding_FillType);
                canvas->drawPath(path, paint);
            } else {
                paint.setStyle(SkPaint::kStroke_Style);

                pdfContext->fGraphicsState.applyGraphicsState(&paint, true);

                // reset it, just in case it messes up the stroke
                path.setFillType(SkPath::kWinding_FillType);
                canvas->drawPath(path, paint);
            }
        }
    }

    pdfContext->fGraphicsState.fPath.reset();
    // TODO(edisonn): implement scale/zoom

    if (pdfContext->fGraphicsState.fHasClipPathToApply) {
#ifndef PDF_DEBUG_NO_CLIPING
        canvas->clipPath(pdfContext->fGraphicsState.fClipPath, SkRegion::kIntersect_Op, true);
#endif
    }

    //pdfContext->fGraphicsState.fClipPath.reset();
    pdfContext->fGraphicsState.fHasClipPathToApply = false;

    return kOK_SkPdfResult;

}

static SkPdfResult PdfOp_S(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    return PdfOp_fillAndStroke(pdfContext, canvas, false, true, false, false);
}

static SkPdfResult PdfOp_s(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    return PdfOp_fillAndStroke(pdfContext, canvas, false, true, true, false);
}

static SkPdfResult PdfOp_F(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    return PdfOp_fillAndStroke(pdfContext, canvas, true, false, false, false);
}

static SkPdfResult PdfOp_f(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    return PdfOp_fillAndStroke(pdfContext, canvas, true, false, false, false);
}

static SkPdfResult PdfOp_f_star(SkPdfContext* pdfContext, SkCanvas* canvas,
                                SkPdfTokenLooper*) {
    return PdfOp_fillAndStroke(pdfContext, canvas, true, false, false, true);
}

static SkPdfResult PdfOp_B(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    return PdfOp_fillAndStroke(pdfContext, canvas, true, true, false, false);
}

static SkPdfResult PdfOp_B_star(SkPdfContext* pdfContext, SkCanvas* canvas,
                                SkPdfTokenLooper*) {
    return PdfOp_fillAndStroke(pdfContext, canvas, true, true, false, true);
}

static SkPdfResult PdfOp_b(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    return PdfOp_fillAndStroke(pdfContext, canvas, true, true, true, false);
}

static SkPdfResult PdfOp_b_star(SkPdfContext* pdfContext, SkCanvas* canvas,
                                SkPdfTokenLooper*) {
    return PdfOp_fillAndStroke(pdfContext, canvas, true, true, true, true);
}

static SkPdfResult PdfOp_n(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    canvas->setMatrix(pdfContext->fGraphicsState.fCTM);
    if (pdfContext->fGraphicsState.fHasClipPathToApply) {
#ifndef PDF_DEBUG_NO_CLIPING
        canvas->clipPath(pdfContext->fGraphicsState.fClipPath, SkRegion::kIntersect_Op, true);
#endif
    }

    pdfContext->fGraphicsState.fHasClipPathToApply = false;

    pdfContext->fGraphicsState.fPathClosed = true;

    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_BT(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    pdfContext->fGraphicsState.fTextBlock   = true;
    SkMatrix matrix = pdfContext->fGraphicsState.fCTM;
    matrix.preScale(SkDoubleToScalar(1), SkDoubleToScalar(-1));
    pdfContext->fGraphicsState.fMatrixTm = matrix;
    pdfContext->fGraphicsState.fMatrixTlm = matrix;

    return kPartial_SkPdfResult;
}

static SkPdfResult PdfOp_ET(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    if (!pdfContext->fGraphicsState.fTextBlock) {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kMissingBT_SkPdfIssue, "ET without BT", NULL,
                    pdfContext);

        return kIgnoreError_SkPdfResult;
    }

    pdfContext->fGraphicsState.fTextBlock = false;

    // TODO(edisonn): anything else to be done once we are done with draw text? Like restore stack?
    return kOK_SkPdfResult;
}

static SkPdfResult skpdfGraphicsStateApplyFontCore(SkPdfContext* pdfContext,
                                                   const SkPdfNativeObject* fontName, double fontSize) {
#ifdef PDF_TRACE
    printf("font name: %s\n", fontName->nameValue2().c_str());
#endif

    if (!pdfContext->fGraphicsState.fResources->Font(pdfContext->fPdfDoc)) {
        // TODO(edisonn): try to recover and draw it any way?
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kMissingFont_SkPdfIssue,
                    "No font", fontName, pdfContext);
        return kIgnoreError_SkPdfResult;
    }

    SkPdfNativeObject* objFont
            = pdfContext->fGraphicsState.fResources->Font(pdfContext->fPdfDoc)->get(fontName);
    objFont = pdfContext->fPdfDoc->resolveReference(objFont);
    if (kNone_SkPdfNativeObjectType == pdfContext->fPdfDoc->mapper()->mapFontDictionary(objFont)) {
        // TODO(edisonn): try to recover and draw it any way?
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kInvalidFont_SkPdfIssue,
                    "Invalid font", objFont, pdfContext);
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
static SkPdfResult PdfOp_Tf(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("Tf", pdfContext, 2);
    POP_NUMBER(pdfContext, fontSize);
    POP_NAME(pdfContext, fontName);
    CHECK_PARAMETERS();

    return skpdfGraphicsStateApplyFontCore(pdfContext, fontName, fontSize);
}

static SkPdfResult PdfOp_Tj(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("Tj", pdfContext, 1);
    POP_STRING(pdfContext, str);
    CHECK_PARAMETERS();

    if (!pdfContext->fGraphicsState.fTextBlock) {
        // TODO(edisonn): try to recover and draw it any way?
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kMissingBT_SkPdfIssue, "Tj without BT", NULL,
                    pdfContext);
        return kIgnoreError_SkPdfResult;
    }

    SkPdfResult ret = DrawText(pdfContext, str, canvas);

    return ret;
}

static SkPdfResult PdfOp_quote(SkPdfContext* pdfContext, SkCanvas* canvas,
                               SkPdfTokenLooper* parentLooper) {
    if (!pdfContext->fGraphicsState.fTextBlock) {
        // TODO(edisonn): try to recover and draw it any way?
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kMissingBT_SkPdfIssue,
                    "' without BT", NULL, pdfContext);
        return kIgnoreError_SkPdfResult;
    }

    PdfOp_T_star(pdfContext, canvas, parentLooper);
    // Do not pop, and push, just transfer the param to Tj
    return PdfOp_Tj(pdfContext, canvas, parentLooper);
}

static SkPdfResult PdfOp_doublequote(SkPdfContext* pdfContext, SkCanvas* canvas,
                                     SkPdfTokenLooper* parentLooper) {
    if (!pdfContext->fGraphicsState.fTextBlock) {
        // TODO(edisonn): try to recover and draw it any way?
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kMissingBT_SkPdfIssue,
                    "\" without BT", NULL, pdfContext);
        return kIgnoreError_SkPdfResult;
    }

    EXPECT_OPERANDS("\"", pdfContext, 3);
    POP_OBJ(pdfContext, str);
    POP_OBJ(pdfContext, ac);
    POP_OBJ(pdfContext, aw);
    CHECK_PARAMETERS();

    pdfContext->fObjectStack.push(aw);
    PdfOp_Tw(pdfContext, canvas, parentLooper);

    pdfContext->fObjectStack.push(ac);
    PdfOp_Tc(pdfContext, canvas, parentLooper);

    pdfContext->fObjectStack.push(str);
    PdfOp_quote(pdfContext, canvas, parentLooper);

    return kPartial_SkPdfResult;
}

static SkPdfResult PdfOp_TJ(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("Tf", pdfContext, 1);
    POP_ARRAY(pdfContext, array);
    CHECK_PARAMETERS();

    if (!pdfContext->fGraphicsState.fTextBlock) {
        // TODO(edisonn): try to recover and draw it any way?
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kMissingBT_SkPdfIssue, "TJ without BT", NULL,
                    pdfContext);
        return kIgnoreError_SkPdfResult;
    }

    if (!array->isArray()) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, NULL, array,
                                  SkPdfNativeObject::kArray_PdfObjectType, pdfContext);
        return kIgnoreError_SkPdfResult;
    }

    for( int i=0; i<static_cast<int>(array->size()); i++ )
    {
        if (!(*array)[i]) {
            SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity,
                                      "element [i] is null, no element should be null",
                                      array,
                                      SkPdfNativeObject::_kAnyString_PdfObjectType |
                                              SkPdfNativeObject::_kNumber_PdfObjectType,
                                      pdfContext);
        } else if( (*array)[i]->isAnyString()) {
            SkPdfNativeObject* obj = (*array)[i];
            DrawText(pdfContext, obj, canvas);
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
        } else {
            SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, "wrong type", (*array)[i],
                                      SkPdfNativeObject::kArray_PdfObjectType |
                                              SkPdfNativeObject::_kNumber_PdfObjectType,
                                      pdfContext);
        }
    }
    return kPartial_SkPdfResult;  // TODO(edisonn): Implement fully DrawText before returing OK.
}

static SkPdfResult PdfOp_CS_cs(SkPdfContext* pdfContext, SkCanvas* canvas,
                               SkPdfColorOperator* colorOperator) {
    EXPECT_OPERANDS("CS/cs", pdfContext, 1);
    POP_NAME(pdfContext, name);
    CHECK_PARAMETERS();

    //Next, get the ColorSpace Dictionary from the Resource Dictionary:
    SkPdfDictionary* colorSpaceResource
            = pdfContext->fGraphicsState.fResources->ColorSpace(pdfContext->fPdfDoc);

    SkPdfNativeObject* colorSpace
            = colorSpaceResource ? pdfContext->fPdfDoc
                                             ->resolveReference(colorSpaceResource->get(name)) :
                                   name;

    if (colorSpace == NULL) {
        colorOperator->fColorSpace = name->strRef();
    } else {
#ifdef PDF_TRACE
        printf("CS = %s\n", colorSpace->toString(0, 0).c_str());
#endif   // PDF_TRACE
        if (colorSpace->isName()) {
            colorOperator->fColorSpace = colorSpace->strRef();
        } else if (colorSpace->isArray()) {
            size_t cnt = colorSpace->size();
            if (cnt == 0) {
                SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kIncostistentSizes_SkPdfIssue,
                            "color space has length 0", colorSpace, pdfContext);
                return kIgnoreError_SkPdfResult;
            }
            SkPdfNativeObject* type = colorSpace->objAtAIndex(0);
            type = pdfContext->fPdfDoc->resolveReference(type);

            if (type->isName("ICCBased")) {
                if (cnt != 2) {
                    SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kIncostistentSizes_SkPdfIssue,
                                "ICCBased color space must have an array with 2 elements",
                                colorSpace, pdfContext);
                    return kIgnoreError_SkPdfResult;
                }
                SkPdfNativeObject* prop = colorSpace->objAtAIndex(1);
                prop = pdfContext->fPdfDoc->resolveReference(prop);
#ifdef PDF_TRACE
                printf("ICCBased prop = %s\n", prop->toString(0, 0).c_str());
#endif   // PDF_TRACE
                // TODO(edisonn): hack
                if (prop && prop->isDictionary() && prop->get("N") &&
                        prop->get("N")->isInteger() && prop->get("N")->intValue() == 3) {
                    colorOperator->setColorSpace(&strings_DeviceRGB);
                    return kPartial_SkPdfResult;
                }
                return kNYI_SkPdfResult;
            }
        }
    }

    return kPartial_SkPdfResult;
}

static SkPdfResult PdfOp_CS(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    return PdfOp_CS_cs(pdfContext, canvas, &pdfContext->fGraphicsState.fStroking);
}

static SkPdfResult PdfOp_cs(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    return PdfOp_CS_cs(pdfContext, canvas, &pdfContext->fGraphicsState.fNonStroking);
}

static SkPdfResult PdfOp_SC_sc(SkPdfContext* pdfContext, SkCanvas* canvas,
                               SkPdfColorOperator* colorOperator) {
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

    EXPECT_OPERANDS("SC/sc", pdfContext, n);

    for (int i = n - 1; i >= 0 ; i--) {
        if (doubles) {
            POP_NUMBER_INTO(pdfContext, c[i]);
//        } else {
//            v[i] = pdfContext->fObjectStack.top()->intValue();        pdfContext->fObjectStack.pop();
        }
    }
    CHECK_PARAMETERS();

    // TODO(edisonn): Now, set that color. Only DeviceRGB supported.
    // TODO(edisonn): do possible field values to enum at parsing time!
    // TODO(edisonn): support also abbreviations /DeviceRGB == /RGB
    if (colorOperator->fColorSpace.equals("DeviceRGB") ||
            colorOperator->fColorSpace.equals("RGB")) {
        colorOperator->setRGBColor(SkColorSetRGB((U8CPU)(255*c[0]),
                                                 (U8CPU)(255*c[1]),
                                                 (U8CPU)(255*c[2])));
    }
    return kPartial_SkPdfResult;
}

static SkPdfResult PdfOp_SC(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    return PdfOp_SC_sc(pdfContext, canvas, &pdfContext->fGraphicsState.fStroking);
}

static SkPdfResult PdfOp_sc(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    return PdfOp_SC_sc(pdfContext, canvas, &pdfContext->fGraphicsState.fNonStroking);
}

static SkPdfResult PdfOp_SCN_scn(SkPdfContext* pdfContext, SkCanvas* canvas,
                                 SkPdfColorOperator* colorOperator) {
    if (pdfContext->fObjectStack.count() > 0 && pdfContext->fObjectStack.top()->isName()) {
        SkPdfNativeObject* name = pdfContext->fObjectStack.top();    pdfContext->fObjectStack.pop();

        SkPdfDictionary* patternResources
                = pdfContext->fGraphicsState.fResources->Pattern(pdfContext->fPdfDoc);

        if (patternResources == NULL) {
#ifdef PDF_TRACE
            printf("ExtGState is NULL!\n");
#endif
            return kIgnoreError_SkPdfResult;
        }

        colorOperator->setPatternColorSpace(
                pdfContext->fPdfDoc->resolveReference(patternResources->get(name)));
    }

    // TODO(edisonn): SCN supports more color spaces than SCN. Read and implement spec.
    PdfOp_SC_sc(pdfContext, canvas, colorOperator);

    return kPartial_SkPdfResult;
}

static SkPdfResult PdfOp_SCN(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    return PdfOp_SCN_scn(pdfContext, canvas, &pdfContext->fGraphicsState.fStroking);
}

static SkPdfResult PdfOp_scn(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    return PdfOp_SCN_scn(pdfContext, canvas, &pdfContext->fGraphicsState.fNonStroking);
}

static SkPdfResult PdfOp_G_g(SkPdfContext* pdfContext, SkCanvas* canvas,
                             SkPdfColorOperator* colorOperator) {
    EXPECT_OPERANDS("G/g", pdfContext, 1);
    POP_NUMBER(pdfContext, gray);
    CHECK_PARAMETERS();

    // TODO(edisonn): limit gray in [0, 1]

    // TODO(edisonn): HACK - it should be device gray, but not suported right now
    colorOperator->fColorSpace = strings_DeviceRGB;
    colorOperator->setRGBColor(SkColorSetRGB((U8CPU)(255 * gray),
                                             (U8CPU)(255 * gray),
                                             (U8CPU)(255 * gray)));

    return kPartial_SkPdfResult;
}

static SkPdfResult PdfOp_G(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    return PdfOp_G_g(pdfContext, canvas, &pdfContext->fGraphicsState.fStroking);
}

static SkPdfResult PdfOp_g(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    return PdfOp_G_g(pdfContext, canvas, &pdfContext->fGraphicsState.fNonStroking);
}

static SkPdfResult PdfOp_RG_rg(SkPdfContext* pdfContext, SkCanvas* canvas,
                               SkPdfColorOperator* colorOperator) {
    EXPECT_OPERANDS("RG/rg", pdfContext, 3);
    POP_NUMBER(pdfContext, b);
    POP_NUMBER(pdfContext, g);
    POP_NUMBER(pdfContext, r);
    CHECK_PARAMETERS();

    colorOperator->fColorSpace = strings_DeviceRGB;
    colorOperator->setRGBColor(SkColorSetRGB((U8CPU)(255*r), (U8CPU)(255*g), (U8CPU)(255*b)));
    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_RG(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    return PdfOp_RG_rg(pdfContext, canvas, &pdfContext->fGraphicsState.fStroking);
}

static SkPdfResult PdfOp_rg(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    return PdfOp_RG_rg(pdfContext, canvas, &pdfContext->fGraphicsState.fNonStroking);
}

static SkPdfResult PdfOp_K_k(SkPdfContext* pdfContext, SkCanvas* canvas,
                             SkPdfColorOperator* colorOperator) {
    // TODO(edisonn): spec has some rules about overprint, implement them.
    EXPECT_OPERANDS("K/k", pdfContext, 4);
    POP_NUMBER(pdfContext, k);
    POP_NUMBER(pdfContext, y);
    POP_NUMBER(pdfContext, m);
    POP_NUMBER(pdfContext, c);
    CHECK_PARAMETERS();

    // TODO(edisonn): really silly quick way to remove compiler warning
    if (k + y + m + c == 0) {
        return kNYI_SkPdfResult;
    }

    //colorOperator->fColorSpace = strings_DeviceCMYK;
    // TODO(edisonn): Set color.
    return kNYI_SkPdfResult;
}

static SkPdfResult PdfOp_K(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    return PdfOp_K_k(pdfContext, canvas, &pdfContext->fGraphicsState.fStroking);
}

static SkPdfResult PdfOp_k(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    return PdfOp_K_k(pdfContext, canvas, &pdfContext->fGraphicsState.fNonStroking);
}

static SkPdfResult PdfOp_W(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    pdfContext->fGraphicsState.fClipPath = pdfContext->fGraphicsState.fPath;
    pdfContext->fGraphicsState.fHasClipPathToApply = true;

    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_W_star(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    pdfContext->fGraphicsState.fClipPath = pdfContext->fGraphicsState.fPath;

    pdfContext->fGraphicsState.fClipPath.setFillType(SkPath::kEvenOdd_FillType);
    pdfContext->fGraphicsState.fHasClipPathToApply = true;

    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_BX(SkPdfContext* pdfContext, SkCanvas* canvas,
                            SkPdfTokenLooper* parentLooper) {
    PdfCompatibilitySectionLooper looper(parentLooper);
    looper.loop();
    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_EX(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kNullObject_SkPdfIssue,
                "EX operator should not be called, it is handled in a looper, "
                        "unless the file is corrupted, we should assert",
                NULL, pdfContext);

    return kIgnoreError_SkPdfResult;
}

static SkPdfResult PdfOp_BI(SkPdfContext* pdfContext, SkCanvas* canvas,
                            SkPdfTokenLooper* parentLooper) {
    PdfInlineImageLooper looper(parentLooper);
    looper.loop();
    return kOK_SkPdfResult;
}

static SkPdfResult PdfOp_ID(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kNullObject_SkPdfIssue,
                "ID operator should not be called, it is habdled in a looper, "
                        "unless the file is corrupted, we should assert",
                NULL, pdfContext);
    return kIgnoreError_SkPdfResult;
}

static SkPdfResult PdfOp_EI(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kNullObject_SkPdfIssue,
                "EI operator should not be called, it is habdled in a looper, "
                        "unless the file is corrupted, we should assert",
                NULL, pdfContext);
    return kIgnoreError_SkPdfResult;
}

static SkPdfResult skpdfGraphicsStateApply_ca(SkPdfContext* pdfContext, double ca) {
    pdfContext->fGraphicsState.fNonStroking.fOpacity = ca;
    return kOK_SkPdfResult;
}

static SkPdfResult skpdfGraphicsStateApply_CA(SkPdfContext* pdfContext, double CA) {
    pdfContext->fGraphicsState.fStroking.fOpacity = CA;
    return kOK_SkPdfResult;
}

static SkPdfResult skpdfGraphicsStateApplyLW(SkPdfContext* pdfContext, double lineWidth) {
    pdfContext->fGraphicsState.fLineWidth = lineWidth;
    return kOK_SkPdfResult;
}

static SkPdfResult skpdfGraphicsStateApplyLC(SkPdfContext* pdfContext, int64_t lineCap) {
    pdfContext->fGraphicsState.fLineCap = (int)lineCap;
    return kOK_SkPdfResult;
}

static SkPdfResult skpdfGraphicsStateApplyLJ(SkPdfContext* pdfContext, int64_t lineJoin) {
    pdfContext->fGraphicsState.fLineJoin = (int)lineJoin;
    return kOK_SkPdfResult;
}

static SkPdfResult skpdfGraphicsStateApplyML(SkPdfContext* pdfContext, double miterLimit) {
    pdfContext->fGraphicsState.fMiterLimit = miterLimit;
    return kOK_SkPdfResult;
}

// TODO(edisonn): test all dashing rules, not sure if they work as in skia.
/*
1) [ ] 0 No dash; solid, unbroken lines
2) [3] 0 3 units on, 3 units off, …
3) [2] 1 1 on, 2 off, 2 on, 2 off, …
4) [2 1] 0 2 on, 1 off, 2 on, 1 off, …
5) [3 5] 6 2 off, 3 on, 5 off, 3 on, 5 off, …
6) [2 3] 11 1 on, 3 off, 2 on, 3 off, 2 on, …
 */

static SkPdfResult skpdfGraphicsStateApplyD(SkPdfContext* pdfContext, SkPdfArray* intervals,
                                            SkPdfNativeObject* phase) {
    if (intervals == NULL) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, NULL, intervals,
                                  SkPdfNativeObject::_kNumber_PdfObjectType, pdfContext);
        return kIgnoreError_SkPdfResult;
    }

    if (phase == NULL) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, NULL, phase,
                                  SkPdfNativeObject::_kNumber_PdfObjectType, pdfContext);
        return kIgnoreError_SkPdfResult;
    }

    int cnt = (int) intervals->size();
    if (cnt >= 256) {
        // TODO(edisonn): alloc memory
        SkPdfReport(kCodeWarning_SkPdfIssueSeverity, kNYI_SkPdfIssue,
                    "dash array size unssuported, cnt > 256", intervals, pdfContext);
        return kIgnoreError_SkPdfResult;
    }
    for (int i = 0; i < cnt; i++) {
        if (!intervals->objAtAIndex(i) || !intervals->objAtAIndex(i)->isNumber()) {
            SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, NULL,
                                      intervals->objAtAIndex(i),
                                      SkPdfNativeObject::_kNumber_PdfObjectType, NULL);
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
        pdfContext->fGraphicsState.fDashPhase = SkDoubleToScalar(total);
    }

    return kOK_SkPdfResult;
}

static SkPdfResult skpdfGraphicsStateApplyD(SkPdfContext* pdfContext, SkPdfArray* dash) {
    if (!dash || dash->isArray()) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, NULL, dash,
                                  SkPdfNativeObject::kArray_PdfObjectType, pdfContext);
        return kIgnoreError_SkPdfResult;
    }

    if (dash->size() != 2) {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kIncostistentSizes_SkPdfIssue,
                    "hash array must have 2 elements", dash, pdfContext);
        return kIgnoreError_SkPdfResult;
    }

    if (!dash->objAtAIndex(0) || !dash->objAtAIndex(0)->isArray()) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, NULL, dash->objAtAIndex(0),
                                  SkPdfNativeObject::kArray_PdfObjectType, pdfContext);
        return kIgnoreError_SkPdfResult;
    }

    if (!dash->objAtAIndex(1) || !dash->objAtAIndex(1)->isNumber()) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, NULL, dash->objAtAIndex(1),
                                  SkPdfNativeObject::_kNumber_PdfObjectType, pdfContext);
        return kIgnoreError_SkPdfResult;
    }

    return skpdfGraphicsStateApplyD(pdfContext, (SkPdfArray*)dash->objAtAIndex(0),
                                    dash->objAtAIndex(1));
}

static void skpdfGraphicsStateApplyFont(SkPdfContext* pdfContext, SkPdfArray* fontAndSize) {
    if (!fontAndSize || !fontAndSize->isArray()) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, NULL, fontAndSize,
                                  SkPdfNativeObject::kArray_PdfObjectType, pdfContext);
        return;
    }

    if (fontAndSize->size() != 2) {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kIncostistentSizes_SkPdfIssue,
                    "font array must have 2 elements", fontAndSize, pdfContext);
        return;
    }

    if (!fontAndSize->objAtAIndex(0) || !fontAndSize->objAtAIndex(0)->isName()) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, NULL,
                                  fontAndSize->objAtAIndex(0),
                                  SkPdfNativeObject::kName_PdfObjectType, pdfContext);
        return;
    }


    if (!fontAndSize->objAtAIndex(1) || !fontAndSize->objAtAIndex(1)->isNumber()) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, NULL,
                                  fontAndSize->objAtAIndex(0),
                                  SkPdfNativeObject::_kNumber_PdfObjectType, pdfContext);
        return;
    }

    skpdfGraphicsStateApplyFontCore(pdfContext, fontAndSize->objAtAIndex(0),
                                    fontAndSize->objAtAIndex(1)->numberValue());
}


//lineWidth w Set the line width in the graphics state (see “Line Width” on page 152).
static SkPdfResult PdfOp_w(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("w", pdfContext, 1);
    POP_NUMBER(pdfContext, lw);
    CHECK_PARAMETERS();

    return skpdfGraphicsStateApplyLW(pdfContext, lw);
}

//lineCap J Set the line cap style in the graphics state (see “Line Cap Style” on page 153).
static SkPdfResult PdfOp_J(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    // TODO(edisonn): round/ceil to int?
    EXPECT_OPERANDS("J", pdfContext, 1);
    POP_NUMBER(pdfContext, lc);
    CHECK_PARAMETERS();

    return skpdfGraphicsStateApplyLC(pdfContext, (int)lc);
}

//lineJoin j Set the line join style in the graphics state (see “Line Join Style” on page 153).
static SkPdfResult PdfOp_j(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    // TODO(edisonn): round/ceil to int?
    EXPECT_OPERANDS("j", pdfContext, 1);
    POP_NUMBER(pdfContext, lj);
    CHECK_PARAMETERS();

    return skpdfGraphicsStateApplyLJ(pdfContext, (int)lj);
}

//miterLimit M Set the miter limit in the graphics state (see “Miter Limit” on page 153).
static SkPdfResult PdfOp_M(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("M", pdfContext, 1);
    POP_NUMBER(pdfContext, ml);
    CHECK_PARAMETERS();
    return skpdfGraphicsStateApplyML(pdfContext, ml);
}

//dashArray dashPhase d Set the line dash pattern in the graphics state (see “Line Dash Pattern” on
//page 155).
static SkPdfResult PdfOp_d(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("d", pdfContext, 2);
    POP_OBJ(pdfContext, phase);
    POP_ARRAY(pdfContext, array);
    CHECK_PARAMETERS();

    return skpdfGraphicsStateApplyD(pdfContext, array, phase);
}

//intent ri (PDF 1.1) Set the color rendering intent in the graphics state (see “Rendering Intents”
// on page 197).
static SkPdfResult PdfOp_ri(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    pdfContext->fObjectStack.pop();

    SkPdfReport(kCodeWarning_SkPdfIssueSeverity, kNYI_SkPdfIssue, "render intent NYI", NULL,
                pdfContext);

    return kNYI_SkPdfResult;
}

//ﬂatness i Set the ﬂatness tolerance in the graphics state (see Section 6.5.1, “Flatness
//Tolerance”). ﬂatness is a number in the range 0 to 100; a value of 0 speci-
//ﬁes the output device’s default ﬂatness tolerance.
static SkPdfResult PdfOp_i(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("i", pdfContext, 1);
    POP_NUMBER(pdfContext, flatness);
    CHECK_PARAMETERS();

    if (flatness < 0 || flatness > 100) {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kOutOfRange_SkPdfIssue,
                    "flatness must be a real in [0, 100] range", flatness_obj, pdfContext);
        return kIgnoreError_SkPdfResult;
    }

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

static SkXfermode::Mode xferModeFromBlendMode(const char* blendMode, size_t len) {
    SkXfermode::Mode mode = (SkXfermode::Mode)(SkXfermode::kLastMode + 1);
    if (gPdfBlendModes.find(blendMode, len, &mode)) {
        return mode;
    }

    return (SkXfermode::Mode)(SkXfermode::kLastMode + 1);
}

static void skpdfGraphicsStateApplyBM_name(SkPdfContext* pdfContext, const SkString& blendMode) {
    SkXfermode::Mode mode = xferModeFromBlendMode(blendMode.c_str(), blendMode.size());
    if (mode <= SkXfermode::kLastMode) {
        pdfContext->fGraphicsState.fBlendModesLength = 1;
        pdfContext->fGraphicsState.fBlendModes[0] = mode;
    } else {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kUnknownBlendMode_SkPdfIssue,
                    blendMode.c_str(), NULL, pdfContext);
    }
}

static void skpdfGraphicsStateApplyBM_array(SkPdfContext* pdfContext, SkPdfArray* blendModes) {
    if (!blendModes || !blendModes->isArray()) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, NULL, blendModes,
                                  SkPdfNativeObject::kArray_PdfObjectType, pdfContext);
        return;
    }

    if (blendModes->size() == 0 || blendModes->size() > 256) {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kIncostistentSizes_SkPdfIssue,
                    "length of blendmodes, 0, is an erro, 256+, is NYI", blendModes, pdfContext);
        return;
    }

    SkXfermode::Mode modes[256];
    int cnt = (int) blendModes->size();
    for (int i = 0; i < cnt; i++) {
        SkPdfNativeObject* name = blendModes->objAtAIndex(i);
        if (!name || !name->isName()) {
            SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, NULL, name,
                                      SkPdfNativeObject::kName_PdfObjectType, pdfContext);
            return;
        }
        SkXfermode::Mode mode = xferModeFromBlendMode(name->c_str(), name->lenstr());
        if (mode > SkXfermode::kLastMode) {
            SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kUnknownBlendMode_SkPdfIssue, NULL, name,
                        pdfContext);
            return;
        }
    }

    pdfContext->fGraphicsState.fBlendModesLength = cnt;
    for (int i = 0; i < cnt; i++) {
        pdfContext->fGraphicsState.fBlendModes[i] = modes[i];
    }
}

static void skpdfGraphicsStateApplySMask_dict(SkPdfContext* pdfContext, SkPdfDictionary* sMask) {
    if (!sMask || !sMask->isName()) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, NULL, sMask,
                                  SkPdfNativeObject::kArray_PdfObjectType, pdfContext);
        return;
    }

    if (pdfContext->fPdfDoc->mapper()->mapSoftMaskDictionary(sMask)) {
        pdfContext->fGraphicsState.fSoftMaskDictionary = (SkPdfSoftMaskDictionary*)sMask;
    } else if (pdfContext->fPdfDoc->mapper()->mapSoftMaskImageDictionary(sMask)) {
        SkPdfSoftMaskImageDictionary* smid = (SkPdfSoftMaskImageDictionary*)sMask;
        pdfContext->fGraphicsState.fSMask = getImageFromObject(pdfContext, smid, true);
    } else {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity,
                                  "Dictionary must be SoftMask, or SoftMaskImage",
                                  sMask, SkPdfNativeObject::kDictionary_PdfObjectType, pdfContext);
    }
}

static void skpdfGraphicsStateApplySMask_name(SkPdfContext* pdfContext, const SkString& sMask) {
    if (sMask.equals("None")) {
        pdfContext->fGraphicsState.fSoftMaskDictionary = NULL;
        pdfContext->fGraphicsState.fSMask = NULL;
        return;
    }

    SkPdfDictionary* extGStateDictionary
            = pdfContext->fGraphicsState.fResources->ExtGState(pdfContext->fPdfDoc);

    if (extGStateDictionary == NULL) {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kMissingExtGState_SkPdfIssue, NULL,
                    pdfContext->fGraphicsState.fResources, pdfContext);
        return;
    }

    SkPdfNativeObject* obj
            = pdfContext->fPdfDoc->resolveReference(extGStateDictionary->get(sMask.c_str()));
    if (!obj || !obj->isDictionary()) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, NULL, obj,
                                  SkPdfNativeObject::kDictionary_PdfObjectType, pdfContext);
        return;
    }

    pdfContext->fGraphicsState.fSoftMaskDictionary = NULL;
    pdfContext->fGraphicsState.fSMask = NULL;

    skpdfGraphicsStateApplySMask_dict(pdfContext, obj->asDictionary());
}

static void skpdfGraphicsStateApplyAIS(SkPdfContext* pdfContext, bool alphaSource) {
    pdfContext->fGraphicsState.fAlphaSource = alphaSource;
}


//dictName gs (PDF 1.2) Set the speciﬁed parameters in the graphics state. dictName is
//the name of a graphics state parameter dictionary in the ExtGState subdictionary of the current
//resource dictionary (see the next section).
static SkPdfResult PdfOp_gs(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("gs", pdfContext, 1);
    POP_NAME(pdfContext, name);
    CHECK_PARAMETERS();

    SkPdfDictionary* extGStateDictionary
            = pdfContext->fGraphicsState.fResources->ExtGState(pdfContext->fPdfDoc);

    if (extGStateDictionary == NULL) {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kMissingExtGState_SkPdfIssue, NULL,
                    pdfContext->fGraphicsState.fResources, pdfContext);
        return kIgnoreError_SkPdfResult;
    }

    SkPdfNativeObject* value
            = pdfContext->fPdfDoc->resolveReference(extGStateDictionary->get(name));

    if (kNone_SkPdfNativeObjectType ==
                pdfContext->fPdfDoc->mapper()->mapGraphicsStateDictionary(value)) {
        return kIgnoreError_SkPdfResult;
    }
    SkPdfGraphicsStateDictionary* gs = (SkPdfGraphicsStateDictionary*)value;

    if (gs == NULL) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, NULL,
                                  gs, SkPdfNativeObject::kDictionary_PdfObjectType, pdfContext);
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
            SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, "wrong type", gs->get("BM"),
                                      SkPdfNativeObject::kArray_PdfObjectType |
                                              SkPdfNativeObject::kName_PdfObjectType, pdfContext);
        }
    }

    if (gs->has_SMask()) {
        if (gs->isSMaskAName(pdfContext->fPdfDoc)) {
            skpdfGraphicsStateApplySMask_name(pdfContext, gs->getSMaskAsName(pdfContext->fPdfDoc));
        } else if (gs->isSMaskADictionary(pdfContext->fPdfDoc)) {
            skpdfGraphicsStateApplySMask_dict(pdfContext,
                                              gs->getSMaskAsDictionary(pdfContext->fPdfDoc));
        } else {
            SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity,
                                      "wrong type",
                                      gs->get("BM"),
                                      SkPdfNativeObject::kDictionary_PdfObjectType |
                                              SkPdfNativeObject::kName_PdfObjectType,
                                              pdfContext);
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

    // TODO(edisonn): make sure we loaded all those properties in graphic state.

    return kOK_SkPdfResult;
}

//charSpace Tc Set the character spacing, Tc
//, to charSpace, which is a number expressed in unscaled text space units.
//  Character spacing is used by the Tj, TJ, and ' operators.
//Initial value: 0.
SkPdfResult PdfOp_Tc(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("Tc", pdfContext, 1);
    POP_NUMBER(pdfContext, charSpace);
    CHECK_PARAMETERS();

    pdfContext->fGraphicsState.fCharSpace = charSpace;

    return kOK_SkPdfResult;
}

//wordSpace Tw Set the word spacing, T
//w
//, to wordSpace, which is a number expressed in unscaled
//text space units. Word spacing is used by the Tj, TJ, and ' operators. Initial
//value: 0.
SkPdfResult PdfOp_Tw(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("Tw", pdfContext, 1);
    POP_NUMBER(pdfContext, wordSpace);
    CHECK_PARAMETERS();

    pdfContext->fGraphicsState.fWordSpace = wordSpace;

    return kOK_SkPdfResult;
}

//scale Tz Set the horizontal scaling, Th
//, to (scale ˜ 100). scale is a number specifying the
//percentage of the normal width. Initial value: 100 (normal width).
static SkPdfResult PdfOp_Tz(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("Tz", pdfContext, 1);
    POP_NUMBER(pdfContext, scale);
    CHECK_PARAMETERS();

    if (scale < 0) {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kOutOfRange_SkPdfIssue,
                    "scale must a positive real number", scale_obj, pdfContext);
        return kError_SkPdfResult;
    }

    return kNYI_SkPdfResult;
}

//render Tr Set the text rendering mode, T
//mode, to render, which is an integer. Initial value: 0.
static SkPdfResult PdfOp_Tr(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("Tr", pdfContext, 1);
    POP_INTEGER(pdfContext, mode);
    CHECK_PARAMETERS();

    if (mode < 0) {  // TODO(edisonn): function/enums with supported modes
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kOutOfRange_SkPdfIssue,
                    "mode must a positive integer or 0", mode_obj, pdfContext);
        return kError_SkPdfResult;
    }

    return kNYI_SkPdfResult;
}
//rise Ts Set the text rise, Trise, to rise, which is a number expressed in unscaled text space
//units. Initial value: 0.
static SkPdfResult PdfOp_Ts(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("Ts", pdfContext, 1);
    POP_NUMBER(pdfContext, rise);
    CHECK_PARAMETERS();

    if (rise < 0) {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kOutOfRange_SkPdfIssue,
                    "rise must a positive real number", rise_obj, pdfContext);
        return kNYI_SkPdfResult;
    }

    return kNYI_SkPdfResult;
}

//wx wy d0
static SkPdfResult PdfOp_d0(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("d0", pdfContext, 2);
    POP_NUMBER(pdfContext, wy);
    POP_NUMBER(pdfContext, wx);
    CHECK_PARAMETERS();

    if (wx < 0) {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kOutOfRange_SkPdfIssue,
                    "wx must a positive real number", wx_obj, pdfContext);
        return kError_SkPdfResult;
    }

    if (wy < 0) {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kOutOfRange_SkPdfIssue,
                    "wy must a positive real number", wy_obj, pdfContext);
        return kError_SkPdfResult;
    }

    return kNYI_SkPdfResult;
}

//wx wy llx lly urx ury d1
static SkPdfResult PdfOp_d1(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("d1", pdfContext, 6);
    POP_NUMBER(pdfContext, ury);
    POP_NUMBER(pdfContext, urx);
    POP_NUMBER(pdfContext, lly);
    POP_NUMBER(pdfContext, llx);
    POP_NUMBER(pdfContext, wy);
    POP_NUMBER(pdfContext, wx);
    CHECK_PARAMETERS();

    // TODO(edisonn): really silly quick way to remove warning
    if (wx + wy + llx + lly + urx + ury) {
        return kNYI_SkPdfResult;
    }

    return kNYI_SkPdfResult;
}

//name sh
static SkPdfResult PdfOp_sh(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("sh", pdfContext, 1);
    POP_NAME(pdfContext, name);
    CHECK_PARAMETERS();

    if (name == NULL) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, NULL, name,
                                  SkPdfNativeObject::kName_PdfObjectType, pdfContext);
        return kError_SkPdfResult;
    }

    return kNYI_SkPdfResult;
}

//name Do
static SkPdfResult PdfOp_Do(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("Do", pdfContext, 1);
    POP_NAME(pdfContext, name);
    CHECK_PARAMETERS();

    SkPdfDictionary* xObject =  pdfContext->fGraphicsState.fResources->XObject(pdfContext->fPdfDoc);

    if (xObject == NULL) {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kMissingXObject_SkPdfIssue, NULL,
                    pdfContext->fGraphicsState.fResources, pdfContext);
        return kIgnoreError_SkPdfResult;
    }

    SkPdfNativeObject* value = xObject->get(name);
    value = pdfContext->fPdfDoc->resolveReference(value);

    return doXObject(pdfContext, canvas, value);
}

//tag MP Designate a marked-content point. tag is a name object indicating the role or
//signiﬁcance of the point.
static SkPdfResult PdfOp_MP(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("MP", pdfContext, 1);
    POP_OBJ(pdfContext, tag);
    CHECK_PARAMETERS();

    if (tag == NULL) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, NULL, tag,
                                  SkPdfNativeObject::_kObject_PdfObjectType, pdfContext);
        return kNYI_SkPdfResult;
    }

    SkPdfReport(kCodeWarning_SkPdfIssueSeverity, kNYI_SkPdfIssue, "MP NYI", NULL, NULL);
    return kNYI_SkPdfResult;
}

//tag properties DP Designate a marked-content point with an associated property list. tag is a
//name object indicating the role or signiﬁcance of the point; properties is
//either an inline dictionary containing the property list or a name object
//associated with it in the Properties subdictionary of the current resource
//dictionary (see Section 9.5.1, “Property Lists”).
static SkPdfResult PdfOp_DP(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("DP", pdfContext, 2);
    POP_OBJ(pdfContext, properties);
    POP_OBJ(pdfContext, tag);
    CHECK_PARAMETERS();

    if (tag == NULL) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, NULL, tag,
                                  SkPdfNativeObject::_kObject_PdfObjectType, pdfContext);
        return kNYI_SkPdfResult;
    }

    if (properties == NULL) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, NULL, properties,
                                  SkPdfNativeObject::_kObject_PdfObjectType, pdfContext);
        return kNYI_SkPdfResult;
    }

    SkPdfReport(kCodeWarning_SkPdfIssueSeverity, kNYI_SkPdfIssue, "DP NYI", NULL, NULL);
    return kNYI_SkPdfResult;
}

//tag BMC Begin a marked-content sequence terminated by a balancing EMC operator.
//tag is a name object indicating the role or signiﬁcance of the sequence.
static SkPdfResult PdfOp_BMC(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("BMC", pdfContext, 1);
    POP_OBJ(pdfContext, tag);
    CHECK_PARAMETERS();

    if (tag == NULL) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, NULL, tag,
                                  SkPdfNativeObject::_kObject_PdfObjectType, pdfContext);
        return kNYI_SkPdfResult;
    }

    SkPdfReport(kCodeWarning_SkPdfIssueSeverity, kNYI_SkPdfIssue, "BMC NYI", NULL, NULL);
    return kNYI_SkPdfResult;
}

//tag properties BDC Begin a marked-content sequence with an associated property list, terminated
//by a balancing EMCoperator. tag is a name object indicating the role or significance of the
// sequence; propertiesis either an inline dictionary containing the
//property list or a name object associated with it in the Properties subdictionary of the current
//resource dictionary (see Section 9.5.1, “Property Lists”).
static SkPdfResult PdfOp_BDC(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    EXPECT_OPERANDS("BDC", pdfContext, 2);
    POP_OBJ(pdfContext, properties);
    POP_OBJ(pdfContext, tag);
    CHECK_PARAMETERS();

    if (tag == NULL) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, NULL, tag,
                                  SkPdfNativeObject::_kObject_PdfObjectType, pdfContext);
        return kNYI_SkPdfResult;
    }

    if (properties == NULL) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, NULL, properties,
                                  SkPdfNativeObject::_kObject_PdfObjectType, pdfContext);
        return kNYI_SkPdfResult;
    }

    SkPdfReport(kCodeWarning_SkPdfIssueSeverity, kNYI_SkPdfIssue, "BDC NYI", NULL, NULL);
    return kNYI_SkPdfResult;
}

//— EMC End a marked-content sequence begun by a BMC or BDC operator.
static SkPdfResult PdfOp_EMC(SkPdfContext* pdfContext, SkCanvas* canvas, SkPdfTokenLooper*) {
    SkPdfReport(kCodeWarning_SkPdfIssueSeverity, kNYI_SkPdfIssue, "EMC NYI", NULL, NULL);
    return kNYI_SkPdfResult;
}

#include "SkPdfOps.h"

SkTDict<PdfOperatorRenderer> gPdfOps(100);

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

SkPdfResult PdfInlineImageLooper::consumeToken(PdfToken& token) {
    SkASSERT(false);
    return kIgnoreError_SkPdfResult;
}

void PdfInlineImageLooper::loop() {
    // FIXME (scroggo): Does this need to be looper? It does not consumeTokens,
    // nor does it loop. The one thing it does is provide access to the
    // protected members of SkPdfTokenLooper.
    doXObject_Image(fPdfContext, fCanvas, fTokenizer->readInlineImage());
}

SkPdfResult PdfCompatibilitySectionLooper::consumeToken(PdfToken& token) {
    return fParent->consumeToken(token);
}

void PdfCompatibilitySectionLooper::loop() {
    PdfOp_q(fPdfContext, fCanvas, NULL);

    PdfToken token;
    while (fTokenizer->readToken(&token)) {
        if (token.fType == kKeyword_TokenType && strcmp(token.fKeyword, "BX") == 0) {
            PdfCompatibilitySectionLooper looper(this);
            looper.loop();
        } else {
            if (token.fType == kKeyword_TokenType && strcmp(token.fKeyword, "EX") == 0) {
                break;
            }
            fParent->consumeToken(token);
        }
    }

    PdfOp_Q(fPdfContext, fCanvas, NULL);
}

// TODO(edisonn): for debugging - remove or put it in a #ifdef
SkPdfContext* gPdfContext = NULL;

bool SkPdfRenderer::renderPage(int page, SkCanvas* canvas, const SkRect& dst) const {
    if (!fPdfDoc) {
        return false;
    }

    if (page < 0 || page >= pages()) {
        return false;
    }

    SkPdfContext pdfContext(fPdfDoc);

    // FIXME (scroggo): Is this matrix needed?
    pdfContext.fOriginalMatrix = SkMatrix::I();
    pdfContext.fGraphicsState.fResources = fPdfDoc->pageResources(page);

    gPdfContext = &pdfContext;

    SkScalar z = SkIntToScalar(0);
    SkScalar w = dst.width();
    SkScalar h = dst.height();

    if (SkScalarTruncToInt(w) <= 0 || SkScalarTruncToInt(h) <= 0) {
        return true;
    }

    // FIXME (scroggo): The media box may not be anchored at 0,0. Is this okay?
    SkScalar wp = fPdfDoc->MediaBox(page).width();
    SkScalar hp = fPdfDoc->MediaBox(page).height();

    SkPoint pdfSpace[4] = {SkPoint::Make(z, z),
                           SkPoint::Make(wp, z),
                           SkPoint::Make(wp, hp),
                           SkPoint::Make(z, hp)};

#ifdef PDF_DEBUG_3X
    // Use larger image to make sure we do not draw anything outside of page
    // could be used in tests.
    SkPoint skiaSpace[4] = {SkPoint::Make(w+z, h+h),
                            SkPoint::Make(w+w, h+h),
                            SkPoint::Make(w+w, h+z),
                            SkPoint::Make(w+z, h+z)};
#else
    SkPoint skiaSpace[4] = {SkPoint::Make(z, h),
                            SkPoint::Make(w, h),
                            SkPoint::Make(w, z),
                            SkPoint::Make(z, z)};
#endif

    SkAssertResult(pdfContext.fOriginalMatrix.setPolyToPoly(pdfSpace, skiaSpace, 4));
    SkTraceMatrix(pdfContext.fOriginalMatrix, "Original matrix");

    // FIXME (scroggo): Do we need to translate to account for the fact that
    // the media box (or the destination rect) may not be anchored at 0,0?
    pdfContext.fOriginalMatrix.postConcat(canvas->getTotalMatrix());

    pdfContext.fGraphicsState.fCTM = pdfContext.fOriginalMatrix;
    pdfContext.fGraphicsState.fContentStreamMatrix = pdfContext.fOriginalMatrix;
    pdfContext.fGraphicsState.fMatrixTm = pdfContext.fGraphicsState.fCTM;
    pdfContext.fGraphicsState.fMatrixTlm = pdfContext.fGraphicsState.fCTM;

#ifndef PDF_DEBUG_NO_PAGE_CLIPING
    canvas->clipRect(dst, SkRegion::kIntersect_Op, true);
#endif

    // FIXME (scroggo): This concat may not be necessary, since we generally
    // call SkCanvas::setMatrix() before using the canvas.
    canvas->concat(pdfContext.fOriginalMatrix);

    doPage(&pdfContext, canvas, fPdfDoc->page(page));

          // TODO(edisonn:) erase with white before draw? Right now the caller is responsible.
//        SkPaint paint;
//        paint.setColor(SK_ColorWHITE);
//        canvas->drawRect(rect, paint);


    canvas->flush();
    return true;
}

SkPdfRenderer* SkPdfRenderer::CreateFromFile(const char* inputFileName) {
    // FIXME: SkPdfNativeDoc should have a similar Create function.
    SkPdfNativeDoc* pdfDoc = SkNEW_ARGS(SkPdfNativeDoc, (inputFileName));
    if (pdfDoc->pages() == 0) {
        SkDELETE(pdfDoc);
        return NULL;
    }

    return SkNEW_ARGS(SkPdfRenderer, (pdfDoc));
}

SkPdfRenderer* SkPdfRenderer::CreateFromStream(SkStream* stream) {
    // TODO(edisonn): create static function that could return NULL if there are errors
    SkPdfNativeDoc* pdfDoc = SkNEW_ARGS(SkPdfNativeDoc, (stream));
    if (pdfDoc->pages() == 0) {
        SkDELETE(pdfDoc);
        return NULL;
    }

    return SkNEW_ARGS(SkPdfRenderer, (pdfDoc));
}

SkPdfRenderer::SkPdfRenderer(SkPdfNativeDoc* doc)
    :fPdfDoc(doc) {
}

SkPdfRenderer::~SkPdfRenderer() {
    SkDELETE(fPdfDoc);
}

int SkPdfRenderer::pages() const {
    SkASSERT(fPdfDoc != NULL);
    return fPdfDoc->pages();
}

SkRect SkPdfRenderer::MediaBox(int page) const {
    SkASSERT(fPdfDoc != NULL);
    return fPdfDoc->MediaBox(page);
}

size_t SkPdfRenderer::bytesUsed() const {
    SkASSERT(fPdfDoc != NULL);
    return fPdfDoc->bytesUsed();
}

bool SkPDFNativeRenderToBitmap(SkStream* stream,
                               SkBitmap* output,
                               int page,
                               SkPdfContent unused,
                               double dpi) {
    SkASSERT(page >= 0);
    SkPdfRenderer* renderer = SkPdfRenderer::CreateFromStream(stream);
    if (NULL == renderer) {
        return false;
    }

    SkRect rect = renderer->MediaBox(page < 0 ? 0 :page);

    SkScalar width = SkScalarMul(rect.width(),  SkDoubleToScalar(dpi / 72.0));
    SkScalar height = SkScalarMul(rect.height(),  SkDoubleToScalar(dpi / 72.0));

    rect = SkRect::MakeWH(width, height);

    setup_bitmap(output, SkScalarCeilToInt(width), SkScalarCeilToInt(height));

    SkAutoTUnref<SkBaseDevice> device(SkNEW_ARGS(SkBitmapDevice, (*output)));
    SkCanvas canvas(device);

    return renderer->renderPage(page, &canvas, rect);
}
