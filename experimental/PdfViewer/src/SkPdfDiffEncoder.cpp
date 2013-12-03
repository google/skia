/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfDiffEncoder.h"
#include "SkPdfNativeTokenizer.h"

#ifdef PDF_TRACE_DIFF_IN_PNG
#include "SkBitmap.h"
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

extern "C" SkBitmap* gDumpBitmap;
extern "C" SkCanvas* gDumpCanvas;
SkBitmap* gDumpBitmap = NULL;
SkCanvas* gDumpCanvas = NULL;
static int gReadOp;
static int gOpCounter;
static SkString gLastKeyword;
#endif  // PDF_TRACE_DIFF_IN_PNG

void SkPdfDiffEncoder::WriteToFile(PdfToken* token) {
#ifdef PDF_TRACE_DIFF_IN_PNG
    gReadOp++;
    gOpCounter++;

    // Only attempt to write if the dump bitmap and canvas are non NULL. They are set by
    // pdf_viewer_main.cpp
    if (NULL == gDumpBitmap || NULL == gDumpCanvas) {
        return;
    }

    // TODO(edisonn): this code is used to make a step by step history of all the draw operations
    // so we could find the step where something is wrong.
    if (!gLastKeyword.isEmpty()) {
        gDumpCanvas->flush();

        // Copy the existing drawing. Then we will draw the difference caused by this command,
        // highlighted with a blue border.
        SkBitmap bitmap;
        if (gDumpBitmap->copyTo(&bitmap, SkBitmap::kARGB_8888_Config)) {

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
                            canvas.drawText("Unknown Clip!!!", strlen("Unknown Clip!!!"),
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

            out.appendf("/tmp/log_step_by_step/step-%i-%s.png", gOpCounter, gLastKeyword.c_str());

            SkImageEncoder::EncodeFile(out.c_str(), bitmap, SkImageEncoder::kPNG_Type, 100);
        }
    }

    if (token->fType == kKeyword_TokenType && token->fKeyword && token->fKeywordLength > 0) {
        gLastKeyword.set(token->fKeyword, token->fKeywordLength);
    } else {
        gLastKeyword.reset();
    }
#endif
}
