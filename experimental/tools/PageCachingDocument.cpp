/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "PageCachingDocument.h"
#include "SkCanvas.h"
#include "SkDocument.h"
#include "SkPictureRecorder.h"
#include "SkRect.h"
#include "SkTDArray.h"

namespace {

typedef void (*DoneProc)(SkWStream*, bool);
typedef SkData* (*Encoder)(size_t*, const SkBitmap&);

// This class allows us to compare the relative memory consumption of
// the PDF and SkPicture backends.
class PageCachingDocument : public SkDocument {
public:
    PageCachingDocument(SkWStream*, DoneProc, Encoder, SkScalar rasterDpi);
    virtual ~PageCachingDocument();
    virtual SkCanvas* onBeginPage(SkScalar width,
                                  SkScalar height,
                                  const SkRect& content) SK_OVERRIDE;
    void onEndPage() SK_OVERRIDE;
    bool onClose(SkWStream*) SK_OVERRIDE;
    void onAbort() SK_OVERRIDE;

private:
    struct Page {
        SkScalar fWidth;
        SkScalar fHeight;
        SkAutoTUnref<SkPicture> fPic;
    };
    SkPictureRecorder fRecorder;
    SkTDArray<Page> fPages;
    Encoder fEncoder;
    SkScalar fRasterDpi;
};

PageCachingDocument::PageCachingDocument(SkWStream* stream,
                                         DoneProc done,
                                         Encoder encoder,
                                         SkScalar rasterDpi)
    : SkDocument(stream, done), fEncoder(encoder), fRasterDpi(rasterDpi) {
}

PageCachingDocument::~PageCachingDocument() {
    for (Page* p = fPages.begin(); p != fPages.end(); ++p) {
        p->~Page();
    }
}

SkCanvas* PageCachingDocument::onBeginPage(SkScalar width,
                                           SkScalar height,
                                           const SkRect& content) {
    Page* page = fPages.push();
    sk_bzero(page, sizeof(*page));
    page->fWidth = width;
    page->fHeight = height;
    SkASSERT(!page->fPic.get());
    SkCanvas* canvas = fRecorder.beginRecording(content);
    return canvas;
}

void PageCachingDocument::onEndPage() {
    SkASSERT(fPages.count() > 0);
    SkASSERT(!fPages[fPages.count() - 1].fPic);
    fPages[fPages.count() - 1].fPic.reset(fRecorder.endRecording());
}

bool PageCachingDocument::onClose(SkWStream* stream) {
    SkAutoTUnref<SkDocument> doc(
        SkDocument::CreatePDF(stream, NULL, fEncoder, fRasterDpi));
    for (Page* page = fPages.begin(); page != fPages.end(); ++page) {
        SkRect cullRect = page->fPic->cullRect();
        SkCanvas* canvas =
            doc->beginPage(page->fWidth, page->fHeight, &cullRect);
        canvas->drawPicture(page->fPic);
        doc->endPage();
    }
    return doc->close();
}

void PageCachingDocument::onAbort() {
}
}  // namespace

SkDocument* CreatePageCachingDocument(SkWStream* stream,
                                      DoneProc done,
                                      Encoder encoder,
                                      SkScalar rasterDpi) {
    return SkNEW_ARGS(PageCachingDocument, (stream, done, encoder, rasterDpi));
}
