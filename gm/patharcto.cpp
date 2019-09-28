/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"

// crbug.com/982968
// Intended to draw a curvy triangle.
// With the bug, we extend the left side of the triangle with lines, a bit like a flag pole.
// The bug/fix is related to precision of the calculation. The original impl used all floats,
// but the very shallow angle causes our sin/cos and other calcs to lose too many bits.
// The fix was to use doubles for this part of the calc in SkPath::arcTo().
//
DEF_SIMPLE_GM(shallow_angle_path_arcto, canvas, 300, 300) {
    SkPath path;
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);

    path.moveTo(313.44189096331155f, 106.6009423589212f);
    path.arcTo(284.3113082008462f, 207.1407719157063f,
               255.15053777129728f, 307.6718505416374f,
               697212.0011054524f);
    path.lineTo(255.15053777129728f, 307.6718505416374f);
    path.arcTo(340.4737465981018f, 252.6907319346971f,
               433.54333477716153f, 212.18116363345337f,
               1251.2484277907251f);
    path.lineTo(433.54333477716153f, 212.18116363345337f);
    path.arcTo(350.19513833839466f, 185.89280014838369f,
               313.44189096331155f, 106.6009423589212f,
               198.03116885327813f);

    canvas->translate(-200, -50);
    canvas->drawPath(path, paint);
};

#include "include/utils/SkParsePath.h"

DEF_SIMPLE_GM(arcto_skbug_9272, canvas, 150, 150) {
    const char* str = "M66.652,65.509c0.663,-2 -0.166,-4.117 -2.117,-5.212 -0.673,-0.378 -1.36,-0.733 -2.04,-1.1a1647300864,1647300864 0,0 1,-31.287 -16.86c-5.39,-2.903 -10.78,-5.808 -16.171,-8.713 -1.626,-0.876 -3.253,-1.752 -4.88,-2.63 -1.224,-0.659 -2.4,-1.413 -3.851,-1.413 -1.135,0 -2.242,0.425 -3.049,1.197 0.08,-0.083 0.164,-0.164 0.248,-0.246l5.309,-5.13 9.37,-9.054 9.525,-9.204 5.903,-5.704C34.237,0.836 34.847,0.297 35.75,0.13c0.982,-0.182 1.862,0.127 2.703,0.592l6.23,3.452L55.76,10.31l11.951,6.62 9.02,4.996c1.74,0.963 4.168,1.854 4.205,4.21 0.011,0.678 -0.246,1.28 -0.474,1.9l-1.005,2.733 -5.665,15.42 -7.106,19.338 -0.034,-0.018z";
    SkPath path;
    SkParsePath::FromSVGString(str, &path);

    const char* str2 = "M10.156,30.995l4.881,2.63 16.17,8.713a1647300736,1647300736 0,0 0,31.287 16.86c0.68,0.366 1.368,0.721 2.041,1.1 2.242,1.257 3.002,3.864 1.72,6.094 -0.659,1.147 -1.296,2.31 -1.978,3.442 -1.276,2.117 -3.973,2.632 -6.102,1.536 -0.244,-0.125 -0.485,-0.259 -0.727,-0.388l-4.102,-2.19 -15.401,-8.225 -18.536,-9.9 -13.893,-7.419c-0.939,-0.501 -1.88,-0.998 -2.816,-1.504C1.2,40.935 0.087,39.5 0.004,37.75c-0.08,-1.672 1.078,-3.277 1.826,-4.702 0.248,-0.471 0.479,-0.958 0.75,-1.416 0.772,-1.306 2.224,-2.05 3.726,-2.05 1.45,0 2.627,0.754 3.85,1.414z";
    SkPath path2;
    SkParsePath::FromSVGString(str2, &path2);

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->translate(30, 30);
    canvas->drawPath(path, paint);
    canvas->drawPath(path2, paint);
}
