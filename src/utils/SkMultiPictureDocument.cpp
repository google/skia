/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMultiPictureDocument.h"
#include "SkMultiPictureDocumentPriv.h"
#include "SkNWayCanvas.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkStream.h"
#include "SkTArray.h"

#include <limits.h>

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
// The unique file signature for this file type.
static constexpr char kMagic[] = "Skia Multi-Picture Doc\n\n";

static constexpr char kEndPage[] = "SkMultiPictureEndPage";

const uint32_t kVersion = 2;

static SkSize join(const SkTArray<SkSize>& sizes) {
    SkSize joined = {0, 0};
    for (SkSize s : sizes) {
        joined = SkSize{SkTMax(joined.width(), s.width()), SkTMax(joined.height(), s.height())};
    }
    return joined;
}

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
        wStream->writeText(kMagic);
        wStream->write32(kVersion);
        wStream->write32(SkToU32(fPages.count()));
        for (SkSize s : fSizes) {
            wStream->write(&s, sizeof(s));
        }
        SkSize bigsize = join(fSizes);
        SkCanvas* c = fPictureRecorder.beginRecording(SkRect::MakeSize(bigsize));
        for (const sk_sp<SkPicture>& page : fPages) {
            c->drawPicture(page);
            c->drawAnnotation(SkRect::MakeEmpty(), kEndPage, nullptr);
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

////////////////////////////////////////////////////////////////////////////////

int SkMultiPictureDocumentReadPageCount(SkStreamSeekable* stream) {
    if (!stream) {
        return 0;
    }
    stream->seek(0);
    const size_t size = sizeof(kMagic) - 1;
    char buffer[size];
    if (size != stream->read(buffer, size) || 0 != memcmp(kMagic, buffer, size)) {
        stream = nullptr;
        return 0;
    }
    uint32_t versionNumber = stream->readU32();
    if (versionNumber != kVersion) {
        return 0;
    }
    uint32_t pageCount = stream->readU32();
    if (pageCount > INT_MAX) {
        return 0;
    }
    // leave stream position right here.
    return (int)pageCount;
}

bool SkMultiPictureDocumentReadPageSizes(SkStreamSeekable* stream,
                                         SkDocumentPage* dstArray,
                                         int dstArrayCount) {
    if (!dstArray || dstArrayCount < 1) {
        return false;
    }
    int pageCount = SkMultiPictureDocumentReadPageCount(stream);
    if (pageCount < 1 || pageCount != dstArrayCount) {
        return false;
    }
    for (int i = 0; i < pageCount; ++i) {
        SkSize& s = dstArray[i].fSize;
        if (sizeof(s) != stream->read(&s, sizeof(s))) {
            return false;
        }
    }
    // leave stream position right here.
    return true;
}

namespace {
struct PagerCanvas : public SkNWayCanvas {
    SkPictureRecorder fRecorder;
    SkDocumentPage* fDst;
    int fCount;
    int fIndex = 0;
    PagerCanvas(SkISize wh, SkDocumentPage* dst, int count)
            : SkNWayCanvas(wh.width(), wh.height()), fDst(dst), fCount(count) {
        this->nextCanvas();
    }
    void nextCanvas() {
        if (fIndex < fCount) {
            SkRect bounds = SkRect::MakeSize(fDst[fIndex].fSize);
            this->addCanvas(fRecorder.beginRecording(bounds));
        }
    }
    void onDrawAnnotation(const SkRect& r, const char* key, SkData* d) override {
        if (0 == strcmp(key, kEndPage)) {
            this->removeAll();
            if (fIndex < fCount) {
                fDst[fIndex].fPicture = fRecorder.finishRecordingAsPicture();
                ++fIndex;
            }
            this->nextCanvas();
        } else {
            this->SkNWayCanvas::onDrawAnnotation(r, key, d);
        }
    }
};
}  // namespace

bool SkMultiPictureDocumentRead(SkStreamSeekable* stream,
                                SkDocumentPage* dstArray,
                                int dstArrayCount) {
    if (!SkMultiPictureDocumentReadPageSizes(stream, dstArray, dstArrayCount)) {
        return false;
    }
    SkSize joined = {0.0f, 0.0f};
    for (int i = 0; i < dstArrayCount; ++i) {
        joined = SkSize{SkTMax(joined.width(), dstArray[i].fSize.width()),
                        SkTMax(joined.height(), dstArray[i].fSize.height())};
    }

    auto picture = SkPicture::MakeFromStream(stream);

    PagerCanvas canvas(joined.toCeil(), dstArray, dstArrayCount);
    // Must call playback(), not drawPicture() to reach
    // PagerCanvas::onDrawAnnotation().
    picture->playback(&canvas);
    if (canvas.fIndex != dstArrayCount) {
        SkDEBUGF(("Malformed SkMultiPictureDocument\n"));
    }
    return true;
}
