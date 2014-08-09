/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCanvasPriv_DEFINED
#define SkCanvasPriv_DEFINED

#include "SkCanvas.h"

class SkAutoCanvasMatrixPaint : SkNoncopyable {
public:
    SkAutoCanvasMatrixPaint(SkCanvas*, const SkMatrix*, const SkPaint*, int width, int height);
    ~SkAutoCanvasMatrixPaint();

private:
    SkCanvas*   fCanvas;
    int         fSaveCount;
};

#endif
