/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkExternalPicture.h"
#include "SkMatrix.h"

void SkExternalPicture::playback(SkCanvas* c, AbortCallback*) const {
  if (!fExternalPic)
    return;

  SkMatrix matrix;
  matrix.setRectToRect(fExternalPic->cullRect(), fCull, SkMatrix::kFill_ScaleToFit);
  c->drawPicture(fExternalPic.get(), &matrix, nullptr);
}

size_t SkExternalPicture::approximateBytesUsed() const {
  if (fExternalPic)
    return fExternalPic->approximateBytesUsed();
  return sizeof(*this);
}

int SkExternalPicture::approximateOpCount() const {
  if (fExternalPic)
    return fExternalPic->approximateOpCount();
  return 0;
}

bool SkExternalPicture::willPlayBackBitmaps() const {
  if (fExternalPic)
    return fExternalPic->willPlayBackBitmaps();
  return false;
}

int SkExternalPicture::numSlowPaths() const {
  if (fExternalPic)
    return fExternalPic->numSlowPaths();
  return 0;
}

sk_sp<SkPicture> CreateExternalPicture(const SkRect& rect) {
  return sk_make_sp<SkExternalPicture>(rect);
}
