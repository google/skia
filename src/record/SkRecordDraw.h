/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecordDraw_DEFINED
#define SkRecordDraw_DEFINED

#include "SkRecord.h"
#include "SkCanvas.h"

// Draw an SkRecord into an SkCanvas.
void SkRecordDraw(const SkRecord&, SkCanvas*);

#endif//SkRecordDraw_DEFINED
