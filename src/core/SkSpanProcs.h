/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSpanProcs_DEFINED
#define SkSpanProcs_DEFINED

#include "SkPM4f.h"

struct SkImageInfo;
class SkPaint;
class SkPixmap;
struct SkPM4f;

typedef void (*SkLoadSpanProc)(const SkPixmap&, int x, int y, SkPM4f span[], int count);
typedef void (*SkFilterSpanProc)(const SkPaint& paint, SkPM4f span[], int count);

SkLoadSpanProc SkLoadSpanProc_Choose(const SkImageInfo&);
SkFilterSpanProc SkFilterSpanProc_Choose(const SkPaint&);

#endif
