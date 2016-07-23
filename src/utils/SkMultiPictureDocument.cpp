/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <vector>

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

struct Page {
  Page(SkSize s, sk_sp<SkPicture> c) : fSize(s), fContent(std::move(c)) {}
  Page(Page&& that) : fSize(that.fSize), fContent(std::move(that.fContent)) {}
  Page(const Page&) = default;
  Page& operator=(const Page&) = default;
  Page& operator=(Page&& that) {
    fSize = that.fSize;
    fContent = std::move(that.fContent);
    return *this;
  }
  SkSize fSize;
  sk_sp<SkPicture> fContent;
};

struct MultiPictureDocument final : public SkDocument {
    SkPictureRecorder fPictureRecorder;
    SkSize fCurrentPageSize;
    std::vector<Page> fPages;
    MultiPictureDocument(SkWStream* s, void (*d)(SkWStream*, bool))
        : SkDocument(s, d) {}
    ~MultiPictureDocument() { this->close(); }

    SkCanvas* onBeginPage(SkScalar w, SkScalar h, const SkRect& c) override {
        fCurrentPageSize.set(w, h);
        return trim(fPictureRecorder.beginRecording(w, h), w, h, c);
    }
    void onEndPage() override {
        fPages.emplace_back(fCurrentPageSize,
                            fPictureRecorder.finishRecordingAsPicture());
    }
    bool onClose(SkWStream* wStream) override {
        SkASSERT(wStream);
        SkASSERT(wStream->bytesWritten() == 0);
        bool good = true;
        good &= wStream->writeText(SkMultiPictureDocumentProtocol::kMagic);
        good &= wStream->write32(SkToU32(1));  // version
        good &= wStream->write32(SkToU32(fPages.size()));
        uint64_t offset = wStream->bytesWritten();
        offset += fPages.size() * sizeof(SkMultiPictureDocumentProtocol::Entry);
        for (const auto& page : fPages) {
            SkMultiPictureDocumentProtocol::Entry entry{
                    offset, page.fSize.width(), page.fSize.height()};
            good &= wStream->write(&entry, sizeof(entry));
            NullWStream buffer;
            page.fContent->serialize(&buffer);
            offset += buffer.bytesWritten();
        }
        for (const auto& page : fPages) {
            page.fContent->serialize(wStream);
        }
        SkASSERT(wStream->bytesWritten() == offset);
        good &= wStream->writeText("\nEndOfMultiPicture\n");
        fPages.clear();
        return good;
    }
    void onAbort() override { fPages.clear(); }
};
}

sk_sp<SkDocument> SkMakeMultiPictureDocument(SkWStream* wStream) {
    return sk_make_sp<MultiPictureDocument>(wStream, nullptr);
}
