/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkMultiPictureDocumentReader_DEFINED
#define SkMultiPictureDocumentReader_DEFINED

#include "../private/SkTArray.h"
#include "SkPicture.h"
#include "SkSize.h"
#include "SkStream.h"

/** A lightweight helper class for reading a Skia MultiPictureDocument. */
class SkMultiPictureDocumentReader {
public:
    /** Initialize the MultiPictureDocument.  Does not take ownership
        of the SkStreamSeekable. */
    bool init(SkStreamSeekable*);

    /** Return to factory settings. */
    void reset() {
        fSizes.reset();
        fOffsets.reset();
    }

    /** Call this after calling init() */
    int pageCount() const { return fSizes.count(); }

    /** Deserialize a page from the stream.  Call init() first.  The
        SkStreamSeekable doesn't need to be the same object, but
        should point to the same information as before. */
    sk_sp<SkPicture> readPage(SkStreamSeekable*, int) const;

    /** Fetch the size of the given page, without deserializing the
        entire page. */
    SkSize pageSize(int i) const { return fSizes[i]; }

private:
    SkTArray<SkSize> fSizes;
    SkTArray<size_t> fOffsets;
};

#endif  // SkMultiPictureDocumentReader_DEFINED
