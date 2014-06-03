/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DMPDFRasterizeTask_DEFINED
#define DMPDFRasterizeTask_DEFINED

#include "DMExpectations.h"
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
    PDFRasterizeTask(const Task& parent,
                     SkData* pdf,
                     const Expectations&,
                     RasterizePdfProc);

    virtual void draw() SK_OVERRIDE;
    virtual bool shouldSkip() const SK_OVERRIDE { return NULL == fRasterize; }
    virtual SkString name()   const SK_OVERRIDE { return fName; }

private:
    const SkString fName;
    SkAutoTUnref<SkData> fPdf;
    const Expectations& fExpectations;
    RasterizePdfProc fRasterize;
};

}  // namespace DM

#endif  // DMPDFRasterizeTask_DEFINED
