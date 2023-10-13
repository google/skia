/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/docs/SkMultiPictureDocument.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkDocument.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkStream.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "include/utils/SkNWayCanvas.h"
#include "src/utils/SkMultiPictureDocument.h"
#include "src/utils/SkMultiPictureDocumentPriv.h"

#include <algorithm>
#include <climits>
#include <cstdint>
#include <cstring>
#include <functional>

using namespace skia_private;

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

static SkSize join(const TArray<SkSize>& sizes) {
    SkSize joined = {0, 0};
    for (SkSize s : sizes) {
        joined = SkSize{std::max(joined.width(), s.width()), std::max(joined.height(), s.height())};
    }
    return joined;
}

struct MultiPictureDocument final : public SkDocument {
    const SkSerialProcs fProcs;
    SkPictureRecorder fPictureRecorder;
    SkSize fCurrentPageSize;
    TArray<sk_sp<SkPicture>> fPages;
    TArray<SkSize> fSizes;
    std::function<void(const SkPicture*)> fOnEndPage;
    MultiPictureDocument(SkWStream* s, const SkSerialProcs* procs,
        std::function<void(const SkPicture*)> onEndPage)
        : SkDocument(s)
        , fProcs(procs ? *procs : SkSerialProcs())
        , fOnEndPage(onEndPage)
    {}
    ~MultiPictureDocument() override { this->close(); }

    SkCanvas* onBeginPage(SkScalar w, SkScalar h) override {
        fCurrentPageSize.set(w, h);
        return fPictureRecorder.beginRecording(w, h);
    }
    void onEndPage() override {
        fSizes.push_back(fCurrentPageSize);
        sk_sp<SkPicture> lastPage = fPictureRecorder.finishRecordingAsPicture();
        fPages.push_back(lastPage);
        if (fOnEndPage) {
            fOnEndPage(lastPage.get());
        }
    }
    void onClose(SkWStream* wStream) override {
        SkASSERT(wStream);
        SkASSERT(wStream->bytesWritten() == 0);
        wStream->writeText(kMagic);
        wStream->write32(kVersion);
        wStream->write32(SkToU32(fPages.size()));
        for (SkSize s : fSizes) {
            wStream->write(&s, sizeof(s));
        }
        SkSize bigsize = join(fSizes);
        SkCanvas* c = fPictureRecorder.beginRecording(SkRect::MakeSize(bigsize));
        for (const sk_sp<SkPicture>& page : fPages) {
            c->drawPicture(page);
            // Annotations must include some data.
            c->drawAnnotation(SkRect::MakeEmpty(), kEndPage, SkData::MakeWithCString("X"));
        }
        sk_sp<SkPicture> p = fPictureRecorder.finishRecordingAsPicture();
        p->serialize(wStream, &fProcs);
        fPages.clear();
        fSizes.clear();
        return;
    }
    void onAbort() override {
        fPages.clear();
        fSizes.clear();
    }
};

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

namespace SkMultiPictureDocument {
sk_sp<SkDocument> Make(SkWStream* dst, const SkSerialProcs* procs,
                       std::function<void(const SkPicture*)> onEndPage) {
    return sk_make_sp<MultiPictureDocument>(dst, procs, onEndPage);
}

int ReadPageCount(SkStreamSeekable* src) {
    if (!src) {
        return 0;
    }
    src->seek(0);
    const size_t size = sizeof(kMagic) - 1;
    char buffer[size];
    if (size != src->read(buffer, size) || 0 != memcmp(kMagic, buffer, size)) {
        src = nullptr;
        return 0;
    }
    uint32_t versionNumber;
    if (!src->readU32(&versionNumber) || versionNumber != kVersion) {
        return 0;
    }
    uint32_t pageCount;
    if (!src->readU32(&pageCount) || pageCount > INT_MAX) {
        return 0;
    }
    // leave stream position right here.
    return SkTo<int>(pageCount);
}

bool ReadPageSizes(SkStreamSeekable* stream,
                   SkDocumentPage* dstArray,
                   int dstArrayCount) {
    if (!dstArray || dstArrayCount < 1) {
        return false;
    }
    int pageCount = ReadPageCount(stream);
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

bool Read(SkStreamSeekable* src,
          SkDocumentPage* dstArray,
          int dstArrayCount,
          const SkDeserialProcs* procs) {
    if (!ReadPageSizes(src, dstArray, dstArrayCount)) {
        return false;
    }
    SkSize joined = {0.0f, 0.0f};
    for (int i = 0; i < dstArrayCount; ++i) {
        joined = SkSize{std::max(joined.width(), dstArray[i].fSize.width()),
                        std::max(joined.height(), dstArray[i].fSize.height())};
    }

    auto picture = SkPicture::MakeFromStream(src, procs);
    if (!picture) {
        return false;
    }

    PagerCanvas canvas(joined.toCeil(), dstArray, dstArrayCount);
    // Must call playback(), not drawPicture() to reach
    // PagerCanvas::onDrawAnnotation().
    picture->playback(&canvas);
    if (canvas.fIndex != dstArrayCount) {
        SkDEBUGF("Malformed SkMultiPictureDocument: canvas.fIndex=%d dstArrayCount=%d\n",
            canvas.fIndex, dstArrayCount);
    }
    return true;
}
}  // namespace SkMultiPictureDocument

sk_sp<SkDocument> SkMakeMultiPictureDocument(SkWStream* wStream, const SkSerialProcs* procs,
    std::function<void(const SkPicture*)> onEndPage) {
    return SkMultiPictureDocument::Make(wStream, procs, onEndPage);
}

int SkMultiPictureDocumentReadPageCount(SkStreamSeekable* src) {
    return SkMultiPictureDocument::ReadPageCount(src);
}


bool SkMultiPictureDocumentRead(SkStreamSeekable* src,
                                SkDocumentPage* dstArray,
                                int dstArrayCount,
                                const SkDeserialProcs* procs) {
    return SkMultiPictureDocument::Read(src, dstArray, dstArrayCount, procs);
}
