/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMultiPictureDocumentPriv_DEFINED
#define SkMultiPictureDocumentPriv_DEFINED

#include "include/docs/SkMultiPictureDocument.h"

namespace SkMultiPictureDocument {
/**
 *  Additional API allows one to read the array of page-sizes without parsing
 *  the entire file.  Used by DM.
 */
bool ReadPageSizes(SkStreamSeekable* src,
                   SkDocumentPage* dstArray,
                   int dstArrayCount);

}  // namespace SkMultiPictureDocument

#endif  // SkMultiPictureDocumentPriv_DEFINED
