/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SamplePipeControllers.h"

#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkGPipe.h"
#include "SkMatrix.h"

PipeController::PipeController(SkCanvas* target)
:fReader(target) {
    fBlock = NULL;
    fBlockSize = fBytesWritten = 0;
}

PipeController::~PipeController() {
    sk_free(fBlock);
}

void* PipeController::requestBlock(size_t minRequest, size_t *actual) {
    sk_free(fBlock);
    fBlockSize = minRequest * 4;
    fBlock = sk_malloc_throw(fBlockSize);
    fBytesWritten = 0;
    *actual = fBlockSize;
    return fBlock;
}

void PipeController::notifyWritten(size_t bytes) {
    fStatus = fReader.playback(this->getData(), bytes);
    SkASSERT(SkGPipeReader::kError_Status != fStatus);
    fBytesWritten += bytes;
}

////////////////////////////////////////////////////////////////////////////////

TiledPipeController::TiledPipeController(const SkBitmap& bitmap,
                                         const SkMatrix* initial)
: INHERITED(NULL) {
    int32_t top = 0;
    int32_t bottom;
    int32_t height = bitmap.height() / NumberOfTiles;
    SkIRect rect;
    for (int i = 0; i < NumberOfTiles; i++) {
        bottom = i + 1 == NumberOfTiles ? bitmap.height() : top + height;
        rect.setLTRB(0, top, bitmap.width(), bottom);
        top = bottom;

        SkDEBUGCODE(bool extracted = )bitmap.extractSubset(&fBitmaps[i], rect);
        SkASSERT(extracted);
        SkDevice* device = new SkDevice(fBitmaps[i]);
        SkCanvas* canvas = new SkCanvas(device);
        device->unref();
        if (initial != NULL) {
            canvas->setMatrix(*initial);
        }
        canvas->translate(SkIntToScalar(-rect.left()),
                          SkIntToScalar(-rect.top()));
        if (0 == i) {
            fReader.setCanvas(canvas);
        } else {
            fReaders[i - 1].setCanvas(canvas);
        }
        canvas->unref();
    }
}

void TiledPipeController::notifyWritten(size_t bytes) {
    for (int i = 0; i < NumberOfTiles - 1; i++) {
        fReaders[i].playback(this->getData(), bytes);
    }
    this->INHERITED::notifyWritten(bytes);
}

////////////////////////////////////////////////////////////////////////////////

ThreadSafePipeController::ThreadSafePipeController(int numberOfReaders)
: fAllocator(kMinBlockSize)
, fNumberOfReaders(numberOfReaders) {
    fBlock = NULL;
    fBytesWritten = 0;
}

void* ThreadSafePipeController::requestBlock(size_t minRequest, size_t *actual) {
    if (fBlock) {
        // Save the previous block for later
        PipeBlock previousBloc(fBlock, fBytesWritten);
        fBlockList.push(previousBloc);
    }
    int32_t blockSize = SkMax32(SkToS32(minRequest), kMinBlockSize);
    fBlock = fAllocator.allocThrow(blockSize);
    fBytesWritten = 0;
    *actual = blockSize;
    return fBlock;
}

void ThreadSafePipeController::notifyWritten(size_t bytes) {
    fBytesWritten += bytes;
}

void ThreadSafePipeController::draw(SkCanvas* target) {
    SkGPipeReader reader(target);
    for (int currentBlock = 0; currentBlock < fBlockList.count(); currentBlock++ ) {
        reader.playback(fBlockList[currentBlock].fBlock, fBlockList[currentBlock].fBytes);
    }

    if (fBlock) {
        reader.playback(fBlock, fBytesWritten);
    }
}
