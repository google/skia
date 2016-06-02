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

/*
  File format:
      BEGINNING_OF_FILE:
        kMagic
        uint32_t version_number
        uint32_t page_count
        {
          uint64_t offset
          float sizeX
          float sizeY
        } * page_count
      FIRST_OFFSET:
        skp file
      SECOND_OFFSET:
        skp file
      ...
      LAST_OFFSET:
        skp file
        "\nEndOfMultiPicture\n"
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

struct NullWStream : public SkWStream {
    NullWStream() : fN(0) {}
    bool write(const void*, size_t n) override {
        fN += n;
        return true;
    }
    size_t bytesWritten() const override { return fN; }
    size_t fN;
};

struct MultiPictureDocument final : public SkDocument {
    SkPictureRecorder fPictureRecorder;
    SkTArray<sk_sp<SkPicture>> fPages;
    MultiPictureDocument(SkWStream* s, void (*d)(SkWStream*, bool))
        : SkDocument(s, d) {}
    ~MultiPictureDocument() { this->close(); }

    SkCanvas* onBeginPage(SkScalar w, SkScalar h, const SkRect& c) override {
        return trim(fPictureRecorder.beginRecording(w, h), w, h, c);
    }
    void onEndPage() override {
        fPages.emplace_back(fPictureRecorder.finishRecordingAsPicture());
    }
    bool onClose(SkWStream* wStream) override {
        SkASSERT(wStream);
        SkASSERT(wStream->bytesWritten() == 0);
        bool good = true;
        good &= wStream->writeText(SkMultiPictureDocumentProtocol::kMagic);
        good &= wStream->write32(SkToU32(1));  // version
        good &= wStream->write32(SkToU32(fPages.count()));
        uint64_t offset = wStream->bytesWritten();
        offset += fPages.count() * sizeof(SkMultiPictureDocumentProtocol::Entry);
        for (const auto& page : fPages) {
            SkRect cullRect = page->cullRect();
            // We recorded a picture at the origin.
            SkASSERT(cullRect.x() == 0 && cullRect.y() == 0);
            SkMultiPictureDocumentProtocol::Entry entry{
                    offset, (float)cullRect.right(), (float)cullRect.bottom()};
            good &= wStream->write(&entry, sizeof(entry));
            NullWStream buffer;
            page->serialize(&buffer);
            offset += buffer.bytesWritten();
        }
        for (const auto& page : fPages) {
            page->serialize(wStream);
        }
        SkASSERT(wStream->bytesWritten() == offset);
        good &= wStream->writeText("\nEndOfMultiPicture\n");
        fPages.reset();
        return good;
    }
    void onAbort() override { fPages.reset(); }
};
}

sk_sp<SkDocument> SkMakeMultiPictureDocument(SkWStream* wStream) {
    return sk_make_sp<MultiPictureDocument>(wStream, nullptr);
}
