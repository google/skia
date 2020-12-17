/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMultiPictureDocument_DEFINED
#define SkMultiPictureDocument_DEFINED

#include "include/core/SkDocument.h"
#include "include/core/SkPicture.h"
#include "include/core/SkSize.h"

#include <functional>

struct SkDeserialProcs;
struct SkSerialProcs;
class SkStreamSeekable;
/**
 *  Writes into a file format that is similar to SkPicture::serialize()
 *  Accepts a callback for endPage behavior
 */
SK_SPI sk_sp<SkDocument> SkMakeMultiPictureDocument(SkWStream* dst, const SkSerialProcs* = nullptr,
  std::function<void(const SkPicture*)> onEndPage = nullptr);

struct SkDocumentPage {
    sk_sp<SkPicture> fPicture;
    SkSize fSize;
};

/**
 *  Returns the number of pages in the SkMultiPictureDocument.
 */
SK_SPI int SkMultiPictureDocumentReadPageCount(SkStreamSeekable* src);

/**
 *  Read the SkMultiPictureDocument into the provided array of pages.
 *  dstArrayCount must equal SkMultiPictureDocumentReadPageCount().
 *  Return false on error.
 */
SK_SPI bool SkMultiPictureDocumentRead(SkStreamSeekable* src,
                                       SkDocumentPage* dstArray,
                                       int dstArrayCount,
                                       const SkDeserialProcs* = nullptr);

#endif  // SkMultiPictureDocument_DEFINED
