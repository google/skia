/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMultiPictureDocument.h"
#include "SkMultiPictureDocumentPriv.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkStream.h"
#include "SkTArray.h"

/*
  File format:
      BEGINNING_OF_FILE:
        kMagic
        uint32_t version_number (==2)
        uint32_t page_count
        {
          float sizeX
          float sizeY
        } * page_count
        skp file
*/

namespace {
static SkCanvas* trim(SkCanvas* canvas,
                      SkScalar w, SkScalar h,
                      const SkRect& trimBox) {
    // Only trim if necessary.
    if (trimBox != SkRect::MakeWH(w, h)) {
        // All SkDocument implementations implement trimBox using a
        // clip+translate.
        canvas->clipRect(trimBox);
        canvas->translate(trimBox.x(), trimBox.y());
    }
    return canvas;
}

struct MultiPictureDocument final : public SkDocument {
    SkPictureRecorder fPictureRecorder;
    SkSize fCurrentPageSize;
    SkTArray<sk_sp<SkPicture>> fPages;
    SkTArray<SkSize> fSizes;
    MultiPictureDocument(SkWStream* s, void (*d)(SkWStream*, bool))
        : SkDocument(s, d) {}
    ~MultiPictureDocument() override { this->close(); }

    SkCanvas* onBeginPage(SkScalar w, SkScalar h, const SkRect& c) override {
        fCurrentPageSize.set(w, h);
        return trim(fPictureRecorder.beginRecording(w, h), w, h, c);
    }
    void onEndPage() override {
        fSizes.push_back(fCurrentPageSize);
        fPages.push_back(fPictureRecorder.finishRecordingAsPicture());
    }
    void onClose(SkWStream* wStream) override {
        SkASSERT(wStream);
        SkASSERT(wStream->bytesWritten() == 0);
        wStream->writeText(SkMultiPictureDocumentProtocol::kMagic);
        wStream->write32(SkMultiPictureDocumentProtocol::kVersion);
        wStream->write32(SkToU32(fPages.count()));
        for (SkSize s : fSizes) {
            wStream->write(&s, sizeof(s));
        }
        SkSize bigsize = SkMultiPictureDocumentProtocol::Join(fSizes);
        SkCanvas* c = fPictureRecorder.beginRecording(SkRect::MakeSize(bigsize));
        for (const sk_sp<SkPicture>& page : fPages) {
            c->drawPicture(page);
            c->drawAnnotation(SkRect::MakeEmpty(),
                              SkMultiPictureDocumentProtocol::kEndPage,
                              nullptr);
        }
        sk_sp<SkPicture> p = fPictureRecorder.finishRecordingAsPicture();
        p->serialize(wStream);
        fPages.reset();
        fSizes.reset();
        return;
    }
    void onAbort() override {
        fPages.reset();
        fSizes.reset();
    }
};
}

sk_sp<SkDocument> SkMakeMultiPictureDocument(SkWStream* wStream) {
    return sk_make_sp<MultiPictureDocument>(wStream, nullptr);
}
