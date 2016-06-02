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
    if (versionNumber != 1) {
        return false;
    }
    uint32_t pageCount = stream->readU32();
    fSizes.reset(pageCount);
    fOffsets.reset(pageCount);
    for (uint32_t i = 0; i < pageCount; ++i) {
        SkMultiPictureDocumentProtocol::Entry entry;
        good &= sizeof(entry) == stream->read(&entry, sizeof(entry));
        fSizes[i] = SkSize::Make(entry.sizeX, entry.sizeY);
        good &= SkTFitsIn<size_t>(entry.offset);
        fOffsets[i] = static_cast<size_t>(entry.offset);
    }
    return good;
}

sk_sp<SkPicture> SkMultiPictureDocumentReader::readPage(SkStreamSeekable* stream,
                                                        int pageNumber) const {
    SkASSERT(pageNumber >= 0);
    SkASSERT(pageNumber < fOffsets.count());
    SkAssertResult(stream->seek(fOffsets[pageNumber]));
    return SkPicture::MakeFromStream(stream);
}
