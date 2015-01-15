/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DMPDFRasterizeTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkStream.h"

namespace DM {

PDFRasterizeTask::PDFRasterizeTask(const Task& parent,
                                   SkStreamAsset* pdf,
                                   RasterizePdfProc proc)
    : CpuTask(parent)
    , fName(UnderJoin(parent.name().c_str(), "rasterize"))
    , fPdf(pdf)
    , fRasterize(proc) {
    SkASSERT(fPdf.get());
    SkASSERT(fPdf->unique());
}

void PDFRasterizeTask::draw() {
    SkBitmap bitmap;

    if (fRasterize(fPdf.get(), &bitmap)) {
        this->spawnChild(SkNEW_ARGS(WriteTask, (*this, "PDF", bitmap)));
    } else {
        this->fail();
    }
}

}  // namespace DM
