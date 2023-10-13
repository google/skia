/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMultiPictureDocument_DEFINED
#define SkMultiPictureDocument_DEFINED

#include "include/core/SkPicture.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"

#include <functional>

class SkDocument;
class SkStreamSeekable;
class SkWStream;
struct SkDeserialProcs;
struct SkSerialProcs;

struct SkDocumentPage {
    sk_sp<SkPicture> fPicture;
    SkSize fSize;
};

namespace SkMultiPictureDocument {
/**
 *  Writes into a file format that is similar to SkPicture::serialize()
 *  Accepts a callback for endPage behavior
 */
SK_API sk_sp<SkDocument> Make(SkWStream* dst, const SkSerialProcs* = nullptr,
                              std::function<void(const SkPicture*)> onEndPage = nullptr);

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
                 SkDocumentPage* dstArray,
                 int dstArrayCount,
                 const SkDeserialProcs* = nullptr);
}  // namespace SkMultiPictureDocument

#endif  // SkMultiPictureDocument_DEFINED
