/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMultiPictureDocumentPriv_DEFINED
#define SkMultiPictureDocumentPriv_DEFINED

#include "SkMultiPictureDocument.h"

bool SkMultiPictureDocumentReadPageSizes(SkStreamSeekable* src,
                                         SkDocumentPage* dstArray,
                                         int dstArrayCount);

#endif  // SkMultiPictureDocumentPriv_DEFINED
