// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKIA_EXT_SKFUTUREDRAWABLE_H_
#define SKIA_EXT_SKFUTUREDRAWABLE_H_

#include <vector>
#include <unordered_map>

// This head has to be before SkDrawable.h
#include "third_party/skia/include/core/SkScalar.h"
#include "third_party/skia/include/core/SkDrawable.h"
#include "third_party/skia/include/core/SkPicture.h"

class SkEmptyPicture final : public SkPicture {
 public:
  void playback(SkCanvas*, AbortCallback*) const override { }

  size_t approximateBytesUsed() const override { return sizeof(*this); }
  int    approximateOpCount()   const override { return 0; }
  SkRect cullRect()             const override { return SkRect::MakeEmpty(); }
  int    numSlowPaths()         const override { return 0; }
  bool   willPlayBackBitmaps()  const override { return false; }
};


class SkFutureDrawable : public SkDrawable {
 public:
  SkFutureDrawable(const SkRect& bounds)
      : fBounds(bounds), fDrawableRef(nullptr) {}
  SkFutureDrawable(const SkRect& bounds, int id)
      : fBounds(bounds), id(id), fDrawableRef(nullptr) {}

  void flatten(SkWriteBuffer& buffer) const override;

  Factory getFactory() const override { return CreateProc; }

  int Id() const { return id; }

  void setDrawableRef(SkFutureDrawable* ref) { fDrawableRef = ref; }

  static sk_sp<SkFlattenable> CreateProc(SkReadBuffer& buffer);

  static void addDrawableRef(int id, std::unique_ptr<SkFutureDrawable> ref) {
    fDrawableRefMap[id] = std::move(ref);
  }

  static void resetDrawableRef() { fDrawableRefMap.clear(); }

 protected:
  SkRect onGetBounds() override { return fBounds; }

  void onDraw(SkCanvas* canvas) override;

  SkPicture* onNewPictureSnapshot() override;

  const char* getTypeName() const override { return "SkFutureDrawable"; }

 private:
  const SkRect  fBounds;
  int           id;
  SkFutureDrawable*   fDrawableRef;

  static std::unordered_map<int, std::unique_ptr<SkFutureDrawable>> fDrawableRefMap;
};

#endif  // SKIA_EXT_SKFUTUREDRAWABLE_H_

