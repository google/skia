/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMultiPictureDocument_DEFINED
#define SkMultiPictureDocument_DEFINED

#include "SkDocument.h"

struct SkDeserialProcs;
struct SkSerialProcs;
class SkStreamSeekable;

namespace SkMultiPictureDocument {

/**
 *  Writes into a file format that is similar to SkPicture::serialize()
 */
SK_API sk_sp<SkDocument> Make(SkWStream* dst, const SkSerialProcs* = nullptr);

struct Page {
    sk_sp<SkPicture> fPicture;
    SkSize fSize;
};

/**
 *  Returns the number of pages in the SkMultiPictureDocument.
 */
SK_API int ReadPageCount(SkStreamSeekable* src);

/**
 *  Read the SkMultiPictureDocument into the provided array of pages.
 *  dstArrayCount must equal SkMultiPictureDocumentReadPageCount().
 *  Return false on error.
 */
SK_API bool Read(SkStreamSeekable* src,
                 Page* dstArray,
                 int dstArrayCount,
                 const SkDeserialProcs* = nullptr);

}  // namespace SkMultiPictureDocument

#endif  // SkMultiPictureDocument_DEFINED
