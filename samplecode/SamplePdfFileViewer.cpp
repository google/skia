/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#ifdef SAMPLE_PDF_FILE_VIEWER

#include "SampleCode.h"
#include "SkDumpCanvas.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkPath.h"
#include "SkPicture.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"
#include "SkXfermode.h"

#include "SkPdfRenderer.h"

class PdfFileViewer : public SampleView {
private:
    SkString    fFilename;
    SkPicture*  fPicture;  // TODO(edisonn): multiple pages, one page / picture, make it an array

    static SkPicture* LoadPdf(const char path[]) {
        SkAutoTDelete<SkPdfRenderer> renderer(SkPdfRenderer::CreateFromFile(path));
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
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SkString name("P:");
            const char* basename = strrchr(fFilename.c_str(), SkPATH_SEPARATOR);
            name.append(basename ? basename+1: fFilename.c_str());
            SampleCode::TitleR(evt, name.c_str());
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
    typedef SampleView INHERITED;
};

SampleView* CreateSamplePdfFileViewer(const char filename[]);
SampleView* CreateSamplePdfFileViewer(const char filename[]) {
    return new PdfFileViewer(filename);
}

//////////////////////////////////////////////////////////////////////////////

#if 0
static SkView* MyFactory() { return new PdfFileViewer; }
static SkViewRegister reg(MyFactory);
#endif

#endif  // SAMPLE_PDF_FILE_VIEWER
