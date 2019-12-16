/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkModeColorFilter_DEFINED
#define SkModeColorFilter_DEFINED

#include "include/core/SkFlattenable.h"

// exported for SkColorFilter::RegisterFlattenables
extern sk_sp<SkFlattenable> SkModeColorFilter_CreateProc(SkReadBuffer&);
extern sk_sp<SkFlattenable> SkLerpColorFilter_CreateProc(SkReadBuffer&);
extern sk_sp<SkFlattenable> SkBlendColorFilter_CreateProc(SkReadBuffer&);

#endif
