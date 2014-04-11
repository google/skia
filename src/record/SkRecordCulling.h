/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecordCulling_DEFINED
#define SkRecordCulling_DEFINED

#include "SkRecord.h"

// Annotates PushCull records in record with the relative offset of their paired PopCull.
void SkRecordAnnotateCullingPairs(SkRecord* record);

#endif//SkRecordCulling_DEFINED
