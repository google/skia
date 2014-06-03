/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DMPDFTask.h"
#include "DMExpectationsTask.h"
#include "DMPDFRasterizeTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"
#include "SkCommandLineFlags.h"
#include "SkDocument.h"

DEFINE_bool(pdf, true, "PDF backend master switch.");

namespace DM {

PDFTask::PDFTask(const char* suffix,
                 Reporter* reporter,
                 TaskRunner* taskRunner,
                 const Expectations& expectations,
                 skiagm::GMRegistry::Factory factory,
                 RasterizePdfProc rasterizePdfProc)
    : CpuTask(reporter, taskRunner)
    , fGM(factory(NULL))
    , fName(UnderJoin(fGM->getName(), suffix))
    , fExpectations(expectations)
    , fRasterize(rasterizePdfProc) {}

namespace {

class SinglePagePDF {
public:
    SinglePagePDF(SkScalar width, SkScalar height)
        : fDocument(SkDocument::CreatePDF(&fWriteStream))
        , fCanvas(fDocument->beginPage(width, height)) {}

    SkCanvas* canvas() { return fCanvas; }

    SkData* end() {
        fCanvas->flush();
        fDocument->endPage();
        fDocument->close();
        return fWriteStream.copyToData();
    }

private:
    SkDynamicMemoryWStream fWriteStream;
    SkAutoTUnref<SkDocument> fDocument;
    SkCanvas* fCanvas;
};

}  // namespace

void PDFTask::draw() {
    SinglePagePDF pdf(fGM->width(), fGM->height());
    //TODO(mtklein): GM doesn't do this.  Why not?
    //pdf.canvas()->concat(fGM->getInitialTransform());
    fGM->draw(pdf.canvas());

    SkAutoTUnref<SkData> pdfData(pdf.end());
    SkASSERT(pdfData.get());

    if (!(fGM->getFlags() & skiagm::GM::kSkipPDFRasterization_Flag)) {
        this->spawnChild(SkNEW_ARGS(PDFRasterizeTask,
                                    (*this, pdfData.get(), fExpectations, fRasterize)));
    }
    this->spawnChild(SkNEW_ARGS(WriteTask, (*this, pdfData.get(), ".pdf")));
}

bool PDFTask::shouldSkip() const {
    if (!FLAGS_pdf) {
        return true;
    }
    if (fGM->getFlags() & skiagm::GM::kSkipPDF_Flag) {
        return true;
    }
    return false;
}

}  // namespace DM
