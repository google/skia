
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/SkGLCanvas.h"
#include "SkBitmap.h"
#include "SkPaint.h"
#include "gl/SkGpuGLShaders.h"

extern "C" {
    void gr_hello_world();
}

void gr_hello_world() {
    static GrGpu* gGpu;
    if (NULL == gGpu) {
        gGpu = new SkGpuGLShaders;
    }

    SkGLCanvas canvas(gGpu);
    SkBitmap bm;

    bm.setConfig(SkBitmap::kARGB_8888_Config, WIDTH, HEIGHT);
    canvas.setBitmapDevice(bm);

    canvas.drawColor(SK_ColorWHITE);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(30);
    canvas.drawText("Hello Kno", 9, 40, 40, paint);
}


