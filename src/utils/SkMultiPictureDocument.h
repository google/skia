/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkMultiPictureDocument_DEFINED
#define SkMultiPictureDocument_DEFINED

/*
  This format is not intended to be used in production.

  For clients looking for a way to represent a document in memory,

    struct Doc {
        std::vector<sk_sp<SkPicture>> fPages;
        std::vector<SkSize> fPageSizes;
    };

  or

    struct Page {
        sk_sp<SkPicture> fPage;
        SkSize fPageSize;
    };
    std::vector<Page> pages;

  would work much better.

  Multi-SkPicture (MSKP) files are still useful for debugging and
  testing.

  The downsides of this format are currently:
  - no way to extract a single page; must read the entire file at once.
  - must use `dm` to convert to another format before passing into
    standard skp tools.
  - `dm` can extract the first page to skp, but no others.

  TODO(halcanary): replace with somthing that addresses these issues.
 */

#include "SkDocument.h"

/** Writes into an experimental, undocumented file format that is
    useful for debugging documents printed via Skia. */
SK_API sk_sp<SkDocument> SkMakeMultiPictureDocument(SkWStream* dst);

#endif  // SkMultiPictureDocument_DEFINED
