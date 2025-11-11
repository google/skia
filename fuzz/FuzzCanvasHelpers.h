/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"

class SkCanvas;
class SkPaint;
class SkImageFilter;
class Fuzz;

void FuzzCanvas(Fuzz* fuzz, SkCanvas* paint, int depth = 9);
void FuzzPaint(Fuzz* fuzz, SkPaint* paint, int depth);

sk_sp<SkImageFilter> MakeFuzzImageFilter(Fuzz* fuzz, int depth);
