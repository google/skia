
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "ReaderView.h"
#include "SkGPipe.h"
#include "SkCanvas.h"

#include <stdio.h>

#define FILE_PATH   "/Users/yangsu/Code/test/test.a"
ReaderView::ReaderView() {
    fBGColor = 0xFFDDDDDD;
    fFilePos = 0;
    fBufferBitmaps[0].setConfig(SkBitmap::kARGB_8888_Config, 640, 480);
    fBufferBitmaps[0].allocPixels(NULL);
    fBufferBitmaps[1].setConfig(SkBitmap::kARGB_8888_Config, 640, 480);
    fBufferBitmaps[1].allocPixels(NULL);
    fFront  = 0;
    fBack   = 1;
}

void ReaderView::draw(SkCanvas* canvas) {
    canvas->drawColor(fBGColor);

    SkAutoCanvasRestore acr(canvas, true);

    //Create a temporary canvas and reader object that draws into the back
    //bitmap so that the incremental changes or incomplete reads are not shown
    //on screen
    SkCanvas bufferCanvas(fBufferBitmaps[fBack]);
    SkGPipeReader reader(&bufferCanvas);

    //The file specified by FILE_PATH MUST exist
    FILE* f = fopen(FILE_PATH, "rb");
    SkASSERT(f != NULL);

    fseek(f, 0, SEEK_END);
    int size = ftell(f) * sizeof(char);
    if (size <= fFilePos) {
        fFilePos = 0;
    }

    //Resume from the last read location
    fseek(f, fFilePos, SEEK_SET);
    int toBeRead = size - fFilePos;
    if (size > 0 && toBeRead > 0) {
        void* block = sk_malloc_throw(toBeRead);
        fread(block, 1, toBeRead, f);

        size_t bytesRead;
        SkGPipeReader::Status fStatus = reader.playback(block, toBeRead, &bytesRead);
        SkASSERT(SkGPipeReader::kError_Status != fStatus);
        SkASSERT(toBeRead >= bytesRead);

        //if the reader reaches a done verb, a frame is complete.
        //Update the file location and swap the front and back bitmaps to show
        //the new frame
        if (SkGPipeReader::kDone_Status == fStatus) {
            fFilePos += bytesRead;
            fFront = fFront ^ 0x1;
            fBack = fBack ^ 0x1;
        }
        sk_free(block);
    }

    fclose(f);

    //the front bitmap is always drawn
    canvas->drawBitmap(fBufferBitmaps[fFront], 0, 0, NULL);
    this->inval(NULL);
}
