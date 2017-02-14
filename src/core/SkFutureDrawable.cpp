/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "third_party/skia/include/core/SkFutureDrawable.h"

#include "third_party/skia/include/core/SkDrawable.h"
#include "third_party/skia/include/core/SkScalar.h"
#include "third_party/skia/include/core/SkWriteBuffer.h"
#include "third_party/skia/src/core/SkReadBuffer.h"

std::unordered_map<int, std::unique_ptr<SkFutureDrawable>>
SkFutureDrawable::fDrawableRefMap = {};

void SkFutureDrawable::flatten(SkWriteBuffer& buffer) const {
  if (fDrawableRef) {
    // Directly flatten the drawable into the buffer.
    fDrawableRef->flatten(buffer);
  } else {
    // Write the bounds.
    buffer.writeRect(fBounds);
    // Write the unique id.
    buffer.writeInt(id);
  }
}

// static
sk_sp<SkFlattenable> SkFutureDrawable::CreateProc(SkReadBuffer& buffer) {
  // Read the bounds.
  SkRect bounds;
  buffer.readRect(&bounds);

  // Read the unique id.
  int id = buffer.readInt();
  sk_sp<SkFutureDrawable> drawable =
      sk_make_sp<SkFutureDrawable>(bounds, id);

  // Check whether the drawable reference is there
  const auto ptr = fDrawableRefMap.find(id);
  if (ptr != fDrawableRefMap.end())
    drawable->setDrawableRef(ptr->second.get());

  return drawable;
}

void SkFutureDrawable::onDraw(SkCanvas* canvas) {
  if (fDrawableRef)
    fDrawableRef->draw(canvas, nullptr);
}

SkPicture* SkFutureDrawable::onNewPictureSnapshot() {
  if (!fDrawableRef)
    return new SkEmptyPicture();
  return fDrawableRef->newPictureSnapshot();
}

