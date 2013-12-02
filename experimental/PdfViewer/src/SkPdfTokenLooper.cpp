/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfTokenLooper.h"
#include "SkPdfNativeTokenizer.h"
#include "SkBitmap.h"

#ifdef PDF_TRACE_DIFF_IN_PNG
#include "SkBitmapDevice.h"
#include "SkCanvas.h"
#include "SkClipStack.h"
#include "SkColor.h"
#include "SkImageEncoder.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkScalar.h"
#include "SkString.h"
#endif  // PDF_TRACE_DIFF_IN_PNG

// FIXME (scroggo): Put behind build flags.
extern "C" SkBitmap* gDumpBitmap;
extern "C" SkCanvas* gDumpCanvas;
SkBitmap* gDumpBitmap = NULL;
SkCanvas* gDumpCanvas = NULL;
int gReadOp;
int gLastOpKeyword;
char gLastKeyword[100] = "";

#ifdef PDF_TRACE_DIFF_IN_PNG
// FIXME (scroggo): allOpWithVisualEffects can be local to hasVisualEffect.
char allOpWithVisualEffects[100] = ",S,s,f,F,f*,B,B*,b,b*,n,Tj,TJ,\',\",d0,d1,sh,EI,Do,EX,";
// FIXME (scroggo): has_visual_effect
static bool hasVisualEffect(const char* pdfOp) {
    return true;
    if (*pdfOp == '\0') return false;

    char markedPdfOp[100] = ",";
    strcat(markedPdfOp, pdfOp);
    strcat(markedPdfOp, ",");

    return (strstr(allOpWithVisualEffects, markedPdfOp) != NULL);
}

static void setup_bitmap(SkBitmap* bitmap, int width, int height) {
    bitmap->setConfig(SkBitmap::kARGB_8888_Config, width, height);

    bitmap->allocPixels();
    bitmap->eraseColor(SK_ColorWHITE);
}

#endif  // PDF_TRACE_DIFF_IN_PNG

// FIXME (scroggo): fTokenizer -> tokenizer.
bool readToken(SkPdfNativeTokenizer* fTokenizer, PdfToken* token) {
    bool ret = fTokenizer->readToken(token);

    gReadOp++;
    gLastOpKeyword++;
#ifdef PDF_TRACE_DIFF_IN_PNG
    // TODO(edisonn): this code is used to make a step by step history of all the draw operations
    // so we could find the step where something is wrong.
    if (gLastKeyword[0] && hasVisualEffect(gLastKeyword)) {
        gDumpCanvas->flush();

        // FIXME (scroggo): Could use SkSurface/SkImage?
        SkBitmap bitmap;
        setup_bitmap(&bitmap, gDumpBitmap->width(), gDumpBitmap->height());

        memcpy(bitmap.getPixels(), gDumpBitmap->getPixels(), gDumpBitmap->getSize());

        SkAutoTUnref<SkBaseDevice> device(SkNEW_ARGS(SkBitmapDevice, (bitmap)));
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
                        canvas.drawText("Rect Clip", strlen("Rect Clip"),
                                        SkDoubleToScalar(10), SkDoubleToScalar(y), blueBorder);
                        break;
                    case SkClipStack::Element::kPath_Type:
                        canvas.drawPath(elem->getPath(), blueBorder);
                        canvas.drawText("Path Clip", strlen("Path Clip"),
                                        SkDoubleToScalar(10), SkDoubleToScalar(y), blueBorder);
                        break;
                    case SkClipStack::Element::kEmpty_Type:
                        canvas.drawText("Empty Clip!!!", strlen("Empty Clip!!!"),
                                        SkDoubleToScalar(10), SkDoubleToScalar(y), blueBorder);
                        break;
                    default:
                        canvas.drawText("Unkown Clip!!!", strlen("Unkown Clip!!!"),
                                        SkDoubleToScalar(10), SkDoubleToScalar(y), blueBorder);
                        break;
                }
            }

            y += 30;
            str.printf("Number of clips in stack: %i", total);
            canvas.drawText(str.c_str(), str.size(),
                            SkDoubleToScalar(10), SkDoubleToScalar(y), blueBorder);
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

        // TODO(edisonn): overlay on top of image inf about the clip , grafic state, the stack

        out.appendf("/tmp/log_step_by_step/step-%i-%s.png",
                    gLastOpKeyword, gLastKeyword);
        SkImageEncoder::EncodeFile(out.c_str(), bitmap, SkImageEncoder::kPNG_Type, 100);
    }

    if (ret && token->fType == kKeyword_TokenType &&
            token->fKeyword && token->fKeywordLength > 0 && token->fKeywordLength < 100) {
        strncpy(gLastKeyword, token->fKeyword, token->fKeywordLength);
        gLastKeyword[token->fKeywordLength] = '\0';
    } else {
        gLastKeyword[0] = '\0';
    }
#endif

    return ret;
}

