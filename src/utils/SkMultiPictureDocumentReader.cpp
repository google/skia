/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMultiPictureDocumentPriv.h"
#include "SkMultiPictureDocumentReader.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkPictureRecorder.h"
#include "SkNWayCanvas.h"

bool SkMultiPictureDocumentReader::init(SkStreamSeekable* stream) {
    if (!stream) {
        return false;
    }
    stream->seek(0);
    const size_t size = sizeof(SkMultiPictureDocumentProtocol::kMagic) - 1;
    char buffer[size];
    if (size != stream->read(buffer, size) ||
        0 != memcmp(SkMultiPictureDocumentProtocol::kMagic, buffer, size)) {
        stream = nullptr;
        return false;
    }
    bool good = true;
    uint32_t versionNumber = stream->readU32();
    if (versionNumber != SkMultiPictureDocumentProtocol::kVersion) {
        return false;
    }
    uint32_t pageCount = stream->readU32();
    fSizes.reset(pageCount);
    for (uint32_t i = 0; i < pageCount; ++i) {
        SkSize size;
        good &= sizeof(size) == stream->read(&size, sizeof(size));
        fSizes[i] = size;
    }
    fOffset = stream->getPosition();
    return good;
}

namespace {
struct PagerCanvas : public SkNWayCanvas {
    SkPictureRecorder fRecorder;
    const SkTArray<SkSize>* fSizes;
    SkTArray<sk_sp<SkPicture>>* fDest;
    PagerCanvas(SkISize  wh,
                const SkTArray<SkSize>* s,
                SkTArray<sk_sp<SkPicture>>* d)
        : SkNWayCanvas(wh.width(), wh.height()), fSizes(s), fDest(d) {
        this->nextCanvas();
    }
    void nextCanvas() {
        int i = fDest->count();
        if (i < fSizes->count()) {
            SkRect bounds = SkRect::MakeSize((*fSizes)[i]);
            this->addCanvas(fRecorder.beginRecording(bounds));
        }
    }
    void onDrawAnnotation(const SkRect& r, const char* key, SkData* d) override {
        if (0 == strcmp(key, SkMultiPictureDocumentProtocol::kEndPage)) {
            this->removeAll();
            if (fRecorder.getRecordingCanvas()) {
                fDest->emplace_back(fRecorder.finishRecordingAsPicture());
            }
            this->nextCanvas();
        } else {
            this->SkNWayCanvas::onDrawAnnotation(r, key, d);
        }
    }
};
}  // namespace

sk_sp<SkPicture> SkMultiPictureDocumentReader::readPage(SkStreamSeekable* stream,
                                                        int pageNumber) const {
    SkASSERT(pageNumber >= 0);
    SkASSERT(pageNumber < fSizes.count());
    if (0 == fPages.count()) {
        stream->seek(fOffset); // jump to beginning of skp
        auto picture = SkPicture::MakeFromStream(stream);
        SkISize size = SkMultiPictureDocumentProtocol::Join(fSizes).toCeil();
        PagerCanvas canvas(size, &fSizes, &this->fPages);
        // Must call playback(), not drawPicture() to reach
        // PagerCanvas::onDrawAnnotation().
        picture->playback(&canvas);
        if (fPages.count() != fSizes.count()) {
            SkDEBUGF(("Malformed SkMultiPictureDocument\n"));
        }
    }
    // Allow for malformed document.
    return pageNumber < fPages.count() ? fPages[pageNumber] : nullptr;
}
