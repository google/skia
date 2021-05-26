/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRuntimeImageFilter_DEFINED
#define SkRuntimeImageFilter_DEFINED

#include "include/core/SkRefCnt.h"

class SkData;
class SkImageFilter;
struct SkRect;
class SkRuntimeEffect;

SK_API sk_sp<SkImageFilter> SkMakeRuntimeImageFilter(sk_sp<SkRuntimeEffect> effect,
                                                     sk_sp<SkData> uniforms,
                                                     sk_sp<SkImageFilter> input);

#endif
