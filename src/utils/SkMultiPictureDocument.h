/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkMultiPictureDocument_DEFINED
#define SkMultiPictureDocument_DEFINED

#include <vector>
#include "SkDocument.h"
#include "SkPicture.h"

class SkStreamSeekable;
class SkStream;
class SkWStream;

#define PicIdType    uint32_t  // type of SkPicture::fUniqueID
#define OOPPicIdType uint64_t  // Out-of-process picture's unique id.
                               // process id (32bits) concats with PicIdType.

SK_API void SkSerializePictureWithOopContent(
    sk_sp<SkPicture> pic,
    SkWStream* wStream,
    uint32_t process_id,
    const std::vector<PicIdType>& pic_ids);

SK_API sk_sp<SkPicture> SkDeserializePictureWithOopContent(
    SkStream* stream, void* deserialize_context);

/**
 *  Writes into a file format that is similar to SkPicture::serialize()
 */
SK_API sk_sp<SkDocument> SkMakeMultiPictureDocument(SkWStream* dst);

SK_API sk_sp<SkDocument> SkMakeMultiPictureDocument(SkWStream* dst,
    uint32_t process_id,
    const std::vector<PicIdType>& pic_ids);

struct SkDocumentPage {
    sk_sp<SkPicture> fPicture;
    SkSize fSize;
};

/**
 *  Returns the number of pages in the SkMultiPictureDocument.
 */
SK_API int SkMultiPictureDocumentReadPageCount(SkStreamSeekable* src);

/**
 *  Read the SkMultiPictureDocument into the provided array of pages.
 *  dstArrayCount must equal SkMultiPictureDocumentReadPageCount().
 *  Return false on error.
 */
SK_API bool SkMultiPictureDocumentRead(SkStreamSeekable* src,
                                       SkDocumentPage* dstArray,
                                       int dstArrayCount,
                                       void* deserialize_context = nullptr);

#endif  // SkMultiPictureDocument_DEFINED
