/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#ifdef SAMPLE_PDF_FILE_VIEWER

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkTime.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkGradientShader.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkUTF.h"
#include "SkPdfRenderer.h"

class PdfFileViewer : public Sample {
private:
    SkString    fFilename;
    SkPicture*  fPicture;  // TODO(edisonn): multiple pages, one page / picture, make it an array

    static SkPicture* LoadPdf(const char path[]) {
        std::unique_ptr<SkPdfRenderer> renderer(SkPdfRenderer::CreateFromFile(path));
        if (nullptr == renderer.get()) {
            return nullptr;
        }

        SkPicture* pic = new SkPicture;
        SkCanvas* canvas = pic->beginRecording((int) renderer->MediaBox(0).width(),
                                               (int) renderer->MediaBox(0).height());
        renderer->renderPage(0, canvas, renderer->MediaBox(0));
        pic->endRecording();
        return pic;
    }

public:
    PdfFileViewer(const char name[] = nullptr) : fFilename(name) {
        fPicture = nullptr;
    }

    virtual ~PdfFileViewer() {
        SkSafeUnref(fPicture);
    }

protected:
    virtual bool onQuery(Sample::Event* evt) {
        if (Sample::TitleQ(*evt)) {
            SkString name("P:");
            const char* basename = strrchr(fFilename.c_str(), SkPATH_SEPARATOR);
            name.append(basename ? basename+1: fFilename.c_str());
            Sample::TitleR(evt, name.c_str());
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual bool onEvent(const SkEvent& evt) {
        // TODO(edisonn): add here event handlers to disable clipping, or to show helpful info
        // like pdf object from click, ...
        // TODO(edisonn): first, next, prev, last page navigation + slideshow
        return this->INHERITED::onEvent(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        if (!fPicture) {
            fPicture = LoadPdf(fFilename.c_str());
        }
        if (fPicture) {
            canvas->drawPicture(*fPicture);
        }
    }

private:
    typedef Sample INHERITED;
};

Sample* CreateSamplePdfFileViewer(const char filename[]);
Sample* CreateSamplePdfFileViewer(const char filename[]) {
    return new PdfFileViewer(filename);
}

//////////////////////////////////////////////////////////////////////////////

#if 0
static Sample* MyFactory() { return new PdfFileViewer; }
static SampleRegister reg(MyFactory);
#endif

#endif  // SAMPLE_PDF_FILE_VIEWER
