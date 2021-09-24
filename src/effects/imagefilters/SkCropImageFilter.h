/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCropImageFilter_DEFINED
#define SkCropImageFilter_DEFINED

#include "include/core/SkRefCnt.h"

class SkImageFilter;
struct SkRect;

// TODO (michaelludwig): Move to SkImageFilters::Crop when ready to expose to the public
SK_API sk_sp<SkImageFilter> SkMakeCropImageFilter(const SkRect& rect, sk_sp<SkImageFilter> input);

#endif
