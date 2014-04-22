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


// Turns logical no-op Save-[non-drawing command]*-Restore patterns into actual no-ops.
void SkRecordNoopSaveRestores(SkRecord*);  // TODO(mtklein): add unit tests

// Annotates PushCull commands with the relative offset of their paired PopCull.
void SkRecordAnnotateCullingPairs(SkRecord*);

// Convert DrawPosText to DrawPosTextH when all the Y coordinates are equal.
void SkRecordReduceDrawPosTextStrength(SkRecord*);  // TODO(mtklein): add unit tests

// Calculate min and max Y bounds for DrawPosTextH commands, for use with SkCanvas::quickRejectY.
void SkRecordBoundDrawPosTextH(SkRecord*);  // TODO(mtklein): add unit tests

#endif//SkRecordOpts_DEFINED
