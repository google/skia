/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecordOpts_DEFINED
#define SkRecordOpts_DEFINED

#include "SkRecord.h"

// Run all optimizations in recommended order.
void SkRecordOptimize(SkRecord*);


// Annotates PushCull records in record with the relative offset of their paired PopCull.
void SkRecordAnnotateCullingPairs(SkRecord*);

#endif//SkRecordOpts_DEFINED
