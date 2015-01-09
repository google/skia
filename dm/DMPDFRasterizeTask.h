/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DMPDFRasterizeTask_DEFINED
#define DMPDFRasterizeTask_DEFINED

#include "DMTask.h"
#include "SkBitmap.h"
#include "SkData.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTemplates.h"

namespace DM {

typedef bool (*RasterizePdfProc)(SkStream* pdf, SkBitmap* output);

class PDFRasterizeTask : public CpuTask {
public:
    // takes ownership of SkStreamAsset.
    PDFRasterizeTask(const Task& parent,
                     SkStreamAsset* pdf,
                     RasterizePdfProc);

    void draw() SK_OVERRIDE;
    bool shouldSkip() const SK_OVERRIDE { return NULL == fRasterize; }
    SkString name()   const SK_OVERRIDE { return fName; }

private:
    const SkString fName;
    SkAutoTDelete<SkStreamAsset> fPdf;
    RasterizePdfProc fRasterize;
};

}  // namespace DM

#endif  // DMPDFRasterizeTask_DEFINED
