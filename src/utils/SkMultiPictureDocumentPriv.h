/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMultiPictureDocumentPriv_DEFINED
#define SkMultiPictureDocumentPriv_DEFINED

#include "src/utils/SkMultiPictureDocument.h"

/**
 *  Additional API allows one to read the array of page-sizes without parsing
 *  the entire file.  Used by DM.
 */
bool SkMultiPictureDocumentReadPageSizes(SkStreamSeekable* src,
                                         SkDocumentPage* dstArray,
                                         int dstArrayCount);

#endif  // SkMultiPictureDocumentPriv_DEFINED
