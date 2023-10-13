/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * TODO(kjlubick) delete this file in favor of the one in include/docs after
 * clients have been updated.
 */

#ifndef SkMultiPictureDocumentOLD_DEFINED
#define SkMultiPictureDocumentOLD_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/docs/SkMultiPictureDocument.h"  // IWYU pragma: keep

#include <functional>

class SkDocument;
class SkPicture;
class SkStreamSeekable;
class SkWStream;
struct SkDeserialProcs;
struct SkSerialProcs;

/**
 *  Writes into a file format that is similar to SkPicture::serialize()
 *  Accepts a callback for endPage behavior
 */
SK_SPI sk_sp<SkDocument> SkMakeMultiPictureDocument(SkWStream* dst, const SkSerialProcs* = nullptr,
  std::function<void(const SkPicture*)> onEndPage = nullptr);

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

#endif  // SkMultiPictureDocumentOLD_DEFINED
