// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkShaperPriv_DEFINED
#define SkShaperPriv_DEFINED

#include "SkShaper.h"

SkPoint ShapePrimitive(SkShaper::RunHandler* handler,
                       const SkFont& font,
                       const char* utf8text,
                       size_t textBytes,
                       bool leftToRight,
                       SkPoint point,
                       SkScalar width);

#endif  // SkShaperPriv_DEFINED
