/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRecordReplaceDraw_DEFINED
#define GrRecordReplaceDraw_DEFINED

#include "SkPicture.h"

class GrLayerCache;
class SkCanvas;
class SkMatrix;

// Draw an SkPicture into an SkCanvas replacing saveLayer/restore blocks with
// drawBitmap calls.  A convenience wrapper around SkRecords::Draw.
// It returns the number of saveLayer/restore blocks replaced with drawBitmap calls.
int GrRecordReplaceDraw(const SkPicture*,
                        SkCanvas*,
                        GrLayerCache* layerCache,
                        const SkMatrix& initialMatrix,
                        SkPicture::AbortCallback*);

#endif // GrRecordReplaceDraw_DEFINED
