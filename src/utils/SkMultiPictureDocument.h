/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkMultiPictureDocument_DEFINED
#define SkMultiPictureDocument_DEFINED

#include "SkDocument.h"

/** Writes into an experimental, undocumented file format that is
    useful for debugging documents printed via Skia. */
SK_API sk_sp<SkDocument> SkMakeMultiPictureDocument(SkWStream* dst);

#endif  // SkMultiPictureDocument_DEFINED
