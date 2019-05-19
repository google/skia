/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/c/sk_types.h"

class SkCanvas;

extern "C" void sk_test_c_api(sk_canvas_t*);

DEF_SIMPLE_GM(c_gms, canvas, 640, 480) {
    sk_test_c_api((sk_canvas_t*)canvas);
}
