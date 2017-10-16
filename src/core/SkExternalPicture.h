/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkPicture.h"
#include "SkRect.h"

class SkExternalPicture final : public SkPicture {
 public:
  explicit SkExternalPicture(const SkRect& cull, sk_sp<SkPicture> pic = nullptr)
      : fCull(cull), fExternalPic(pic) {}

  void playback(SkCanvas* c, AbortCallback*) const override;

  bool isExternal() const override { return true; }
  SkRect cullRect()             const override { return fCull; }
  size_t approximateBytesUsed() const override;
  int    approximateOpCount()   const override;
  bool   willPlayBackBitmaps()  const override;
  int    numSlowPaths()         const override;

 private:
  const SkRect fCull;
  sk_sp<SkPicture> fExternalPic;
};

