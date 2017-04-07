/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkMultiPictureDocument_old_DEFINED
#define SkMultiPictureDocument_old_DEFINED

#include "../../include/utils/SkMultiPictureDocument.h"

// This header is deprecated in favor of the one in include/.

using SkDocumentPage = SkMultiPictureDocument::Page;

inline sk_sp<SkDocument> SkMakeMultiPictureDocument(SkWStream* dst,
                                                    const SkSerialProcs* procs = nullptr) {
    return SkMultiPictureDocument::Make(dst, procs);
}

inline int SkMultiPictureDocumentReadPageCount(SkStreamSeekable* src) {
    return SkMultiPictureDocument::ReadPageCount(src);
}

inline bool SkMultiPictureDocumentRead(SkStreamSeekable* src,
                                       SkMultiPictureDocument::Page* dstArray,
                                       int dstArrayCount,
                                       const SkDeserialProcs* procs = nullptr) {
    return SkMultiPictureDocument::Read(src, dstArray, dstArrayCount, procs);
}

#endif  // SkMultiPictureDocument_old_DEFINED
