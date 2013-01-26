/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "EdgeWalker_Test.h"
#include "Intersection_Tests.h"
#include "SkBitmap.h"

static SkBitmap bitmap;

static void testSimplifyTriangle() {
    SkPath path, out;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(10,10); // triangle      |\      .
    path.lineTo(10,30); //               |_\     .
    path.lineTo(20,30);
    path.close();
    path.moveTo(20,10); // triangle        /|
    path.lineTo(10,30); //                /_|
    path.lineTo(20,30);
    path.close();
    testSimplify(path, true, out, bitmap); // expect |\/|
                                   //        |__|
}

static void testSimplifyTriangle3() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(3, 1);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyTriangle4() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(2, 1);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyTriangle5() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 1);
    path.lineTo(2, 1);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyTriangle6() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.lineTo(3, 1);
    path.lineTo(0, 0);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyTriangle7() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 1);
    path.lineTo(0, 2);
    path.lineTo(0, 0);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyTriangle8() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 1);
    path.lineTo(1, 2);
    path.lineTo(1, 3);
    path.lineTo(0, 1);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyTriangle9() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(1, 1);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 1);
    path.lineTo(2, 1);
    path.lineTo(0, 0);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyTriangle10() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(1, 1);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(2, 0);
    path.lineTo(0, 1);
    path.lineTo(0, 0);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyTriangle11() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 2);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(2, 1);
    path.lineTo(2, 2);
    path.lineTo(0, 0);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyTriangle12() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(1, 2);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(0, 3);
    path.lineTo(1, 1);
    path.lineTo(2, 0);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyTriangle13() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 3);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(3, 0);
    path.lineTo(0, 3);
    path.lineTo(1, 1);
    path.lineTo(3, 0);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyTriangle14() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.lineTo(0, 0);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyTriangle15() {
    SkPath path, out;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.lineTo(2, 2);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyTriangle16() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.lineTo(1, 3);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyTriangle17() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 3);
    path.lineTo(0, 1);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyTriangle18() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(0, 1);
    path.lineTo(0, 3);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyTriangle19() {
    SkPath path, out;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.lineTo(3, 2);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 1);
    path.lineTo(2, 1);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyTriangle20() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(2, 1);
    path.lineTo(1, 3);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(3, 2);
    path.lineTo(0, 3);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyTriangle21() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(1, 2);
    path.close();
    path.moveTo(2, 0);
    path.lineTo(2, 1);
    path.lineTo(0, 3);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyDegenerateTriangle1() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(0, 0);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyDegenerateTriangle2() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 1);
    path.lineTo(2, 2);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(2, 2);
    path.lineTo(3, 3);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyWindingParallelogram() {
    SkPath path, out;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(20,10); // parallelogram  _
    path.lineTo(30,30); //               \ \      .
    path.lineTo(40,30); //                \_\     .
    path.lineTo(30,10);
    path.close();
    path.moveTo(20,10); // parallelogram   _
    path.lineTo(10,30); //                / /
    path.lineTo(20,30); //               /_/
    path.lineTo(30,10);
    path.close();
    testSimplify(path, true, out, bitmap); // expect   _
                                   //         / \     .
}                                  //        /___\    .

static void testSimplifyXorParallelogram() {
    SkPath path, out;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(20,10); // parallelogram  _
    path.lineTo(30,30); //               \ \      .
    path.lineTo(40,30); //                \_\     .
    path.lineTo(30,10);
    path.close();
    path.moveTo(20,10); // parallelogram   _
    path.lineTo(10,30); //                / /
    path.lineTo(20,30); //               /_/
    path.lineTo(30,10);
    path.close();
    testSimplify(path, true, out, bitmap); // expect   _
}                                  //         \ /

static void testSimplifyTriangle2() {
    SkPath path, out;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(10,10); // triangle      |\      .
    path.lineTo(10,30); //               |_\     .
    path.lineTo(20,30);
    path.close();
    path.moveTo(10,10); // triangle       _
    path.lineTo(20,10); //               \ |
    path.lineTo(20,30); //                \|
    path.close();                  //         _
    testSimplify(path, true, out, bitmap); // expect | |
}                                  //        |_|

#if 0
static void testPathTriangleRendering() {
    SkPath one, two;
    one.moveTo(0, 0);
    one.lineTo(3, 3);
    one.lineTo(0, 3);
    one.lineTo(1, 2);
    one.close();
    for (float x = .1f; x <= 2.9ff; x += .1f) {
        SkDebugf("%s x=%g\n", __FUNCTION__, x);
        two.moveTo(0, 0);
        two.lineTo(x, x);
        two.lineTo(3, 3);
        two.lineTo(0, 3);
        two.lineTo(1, 2);
        two.close();
        comparePaths(one, two);
        two.reset();
    }
}
#endif

static void simplify(const char* functionName, const SkPath& path,
        bool fill, SkPath& out) {
    if (false) SkDebugf("%s\n", functionName);
    simplify(path, fill, out);
}

static void testSimplifySkinnyTriangle1() {
    for (int x = 1; x < 255; ++x) {
        SkPath path, out;
        path.moveTo((x * 101) % 10, 0);
        path.lineTo((x * 91) % 10, 1000);
        path.lineTo((x * 71) % 10, 2000);
        path.lineTo((x * 51) % 10, 3000);
        path.close();
        path.moveTo((x * 101) % 20, 0);
        path.lineTo((x * 91) % 20, 1000);
        path.lineTo((x * 71) % 20, 2000);
        path.lineTo((x * 51) % 20, 3000);
        path.close();
        path.moveTo((x * 101) % 30, 0);
        path.lineTo((x * 91) % 30, 1000);
        path.lineTo((x * 71) % 30, 2000);
        path.lineTo((x * 51) % 30, 3000);
        path.close();
        simplify(path, true, out);
    }
}

static void testSimplifySkinnyTriangle2() {
        SkPath path, out;
#if 01
path.moveTo(591.091064f, 627.534851f);
path.lineTo(541.088135f, 560.707642f);
path.lineTo(491.085175f, 493.880310f);
path.lineTo(441.082214f, 427.053101f);
//path.lineTo(591.091064f, 627.534851f);
path.close();
#endif
path.moveTo(317.093445f, 592.013306f);
path.lineTo(366.316162f, 542.986572f);
path.lineTo(416.051514f, 486.978577f);
path.lineTo(465.786865f, 430.970581f);
//path.lineTo(317.093445f, 592.013306f);
path.close();
#if 0
path.moveTo(289.392517f, 517.138489f);
path.lineTo(249.886078f, 508.598022f);
path.lineTo(217.110916f, 450.916443f);
path.lineTo(196.621033f, 394.917633f);
//path.lineTo(289.392517f, 517.138489f);
path.close();
#endif
    simplify(__FUNCTION__, path, true, out);
}

static void testSimplifySkinnyTriangle3() {
        SkPath path, out;
        path.moveTo(591, 627.534851f);
        path.lineTo(541, 560.707642f);
        path.lineTo(491, 493.880310f);
        path.lineTo(441, 427.053101f);
        path.close();
        path.moveTo(317, 592.013306f);
        path.lineTo(366, 542.986572f);
        path.lineTo(416, 486.978577f);
        path.lineTo(465, 430.970581f);
        path.close();
    simplify(__FUNCTION__, path, true, out);
}

static void testSimplifySkinnyTriangle4() {
        SkPath path, out;
path.moveTo(572.655212f, 614.959961f);
path.lineTo(524.618896f, 549.339600f);
path.lineTo(476.582581f, 483.719269f);
path.lineTo(428.546265f, 418.098938f);
path.lineTo(572.655212f, 614.959961f);
path.close();
path.moveTo(312.166382f, 583.723083f);
path.lineTo(361.047791f, 529.824219f);
path.lineTo(409.929230f, 475.925354f);
path.lineTo(458.810669f, 422.026520f);
path.lineTo(312.166382f, 583.723083f);
path.close();
path.moveTo(278.742737f, 508.065643f);
path.lineTo(241.475800f, 493.465118f);
path.lineTo(210.344177f, 437.315125f);
path.lineTo(197.019455f, 383.794556f);
path.lineTo(278.742737f, 508.065643f);
path.close();
    simplify(__FUNCTION__, path, true, out);
}

static void testSimplifySkinnyTriangle5() {
        SkPath path, out;
path.moveTo(554.690613f, 602.286072f);
path.lineTo(508.590057f, 537.906250f);
path.lineTo(462.489441f, 473.526520f);
path.lineTo(416.388855f, 409.146729f);
path.lineTo(554.690613f, 602.286072f);
path.close();
path.moveTo(307.216949f, 575.189270f);
path.lineTo(355.826965f, 516.804688f);
path.lineTo(403.815918f, 464.990753f);
path.lineTo(451.804871f, 413.176819f);
path.lineTo(307.216949f, 575.189270f);
path.close();
path.moveTo(271.998901f, 521.301025f);
path.lineTo(234.619705f, 499.687683f);
path.lineTo(203.059692f, 441.332336f);
path.lineTo(195.994370f, 386.856506f);
path.lineTo(271.998901f, 521.301025f);
path.close();
    simplify(__FUNCTION__, path, true, out);
}

static void testSimplifySkinnyTriangle6() {
        SkPath path, out;
path.moveTo(591.091064f, 627.534851f);
path.lineTo(541.088135f, 560.707642f);
path.lineTo(491.085175f, 493.880310f);
path.lineTo(441.082214f, 427.053101f);
path.lineTo(591.091064f, 627.534851f);
path.close();
path.moveTo(317.093445f, 592.013306f);
path.lineTo(366.316162f, 542.986572f);
path.lineTo(416.051514f, 486.978577f);
path.lineTo(465.786865f, 430.970581f);
path.lineTo(317.093445f, 592.013306f);
path.close();
path.moveTo(289.392517f, 517.138489f);
path.lineTo(249.886078f, 508.598022f);
path.lineTo(217.110916f, 450.916443f);
path.lineTo(196.621033f, 394.917633f);
path.lineTo(289.392517f, 517.138489f);
path.close();
    simplify(__FUNCTION__, path, true, out);
}

static void testSimplifyTriangle22() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 2);
    path.close();
    path.moveTo(1, 0);
    path.lineTo(0, 2);
    path.lineTo(0, 1);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyTriangle23() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(0, 0);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(0, 1);
    path.lineTo(1, 2);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifyTriangle24() {
    SkPath path, out;
    path.moveTo(0, 0);
    path.lineTo(0, 0);
    path.lineTo(0, 1);
    path.close();
    path.moveTo(0, 0);
    path.lineTo(1, 0);
    path.lineTo(0, 1);
    path.close();
    testSimplify(path, true, out, bitmap);
}

static void testSimplifySkinnyTriangle7() {
        SkPath path, out;
path.moveTo(487.502319f, 550.811279f);
path.lineTo(448.826050f, 491.720123f);
path.lineTo(410.149780f, 432.628967f);
path.lineTo(371.473572f, 373.537781f);
path.lineTo(487.502319f, 550.811279f);
path.close();
path.moveTo(295.817108f, 532.655579f);
path.lineTo(342.896271f, 485.912292f);
path.lineTo(389.975433f, 439.169006f);
path.lineTo(437.054596f, 392.425781f);
path.lineTo(295.817108f, 532.655579f);
path.close();
path.moveTo(239.726822f, 575.025269f);
path.lineTo(204.117569f, 521.429688f);
path.lineTo(171.275452f, 454.110382f);
path.lineTo(193.328583f, 397.859497f);
path.lineTo(239.726822f, 575.025269f);
path.close();
    simplify(__FUNCTION__, path, true, out);
}

static void testSimplifySkinnyTriangle8() {
        SkPath path, out;
path.moveTo(441.943115f, 511.678040f);
path.lineTo(408.487549f, 456.880920f);
path.lineTo(375.031952f, 402.083801f);
path.lineTo(341.576385f, 347.286682f);
path.lineTo(441.943115f, 511.678040f);
path.close();
path.moveTo(297.548492f, 557.246704f);
path.lineTo(350.768494f, 507.627014f);
path.lineTo(403.988525f, 458.007385f);
path.lineTo(457.208527f, 408.387695f);
path.lineTo(297.548492f, 557.246704f);
path.close();
path.moveTo(209.857895f, 615.802979f);
path.lineTo(178.249481f, 534.230347f);
path.lineTo(144.905640f, 460.056824f);
path.lineTo(192.953125f, 404.972900f);
path.lineTo(209.857895f, 615.802979f);
path.close();
    simplify(__FUNCTION__, path, true, out);
}

static void testSimplifySkinnyTriangle9() {
        SkPath path, out;
path.moveTo(439.867065f, 528.291931f);
path.lineTo(405.413025f, 469.107178f);
path.lineTo(370.958954f, 409.922363f);
path.lineTo(336.504883f, 350.737610f);
path.lineTo(439.867065f, 528.291931f);
path.close();
path.moveTo(298.922455f, 573.251953f);
path.lineTo(356.360962f, 521.905090f);
path.lineTo(413.799438f, 470.558228f);
path.lineTo(471.237915f, 419.211365f);
path.lineTo(298.922455f, 573.251953f);
path.close();
path.moveTo(187.200775f, 643.035156f);
path.lineTo(159.713165f, 540.993774f);
path.lineTo(126.257164f, 462.198517f);
path.lineTo(193.534012f, 409.266235f);
path.lineTo(187.200775f, 643.035156f);
path.close();
path.close();
    simplify(__FUNCTION__, path, true, out);
}

static void testSimplifySkinnyTriangle10() {
        SkPath path, out;
#if 0
path.moveTo(99.270325f, 239.365234f);
path.lineTo(105.967056f, 173.361206f);
path.lineTo(148.821381f, 141.309891f);
path.lineTo(159.101013f, 189.235138f);
path.lineTo(99.270325f, 239.365234f);
path.close();
#endif
path.moveTo(213.673737f, 413.292938f);
path.lineTo(225.200134f, 343.616821f);
path.lineTo(236.726532f, 273.940704f);
path.lineTo(219.386414f, 231.373322f);
path.lineTo(213.673737f, 413.292938f);
path.close();
path.moveTo(43.485352f, 308.984497f);
path.lineTo(122.610657f, 305.950134f);
path.lineTo(201.735962f, 302.915802f);
path.lineTo(280.861267f, 299.881470f);
path.lineTo(43.485352f, 308.984497f);
path.close();
    simplify(__FUNCTION__, path, true, out);
}

static void testSimplifySkinnyTriangle11() {
        SkPath path, out;
path.moveTo(-177.878387f, 265.368988f);
path.lineTo(-254.415771f, 303.709961f);
path.lineTo(-317.465363f, 271.325562f);
path.lineTo(-374.520386f, 207.507660f);
path.lineTo(-177.878387f, 265.368988f);
path.close();
path.moveTo(-63.582489f, -3.679123f);
path.lineTo(-134.496841f, 26.434566f);
path.lineTo(-205.411209f, 56.548256f);
path.lineTo(-276.325562f, 86.661942f);
path.lineTo(-63.582489f, -3.679123f);
path.close();
path.moveTo(-57.078423f, 162.633453f);
path.lineTo(-95.963928f, 106.261139f);
path.lineTo(-134.849457f, 49.888824f);
path.lineTo(-173.734955f, -6.483480f);
path.lineTo(-57.078423f, 162.633453f);
path.close();
    simplify(__FUNCTION__, path, true, out);
}

static void testSimplifySkinnyTriangle12() {
        SkPath path, out;
path.moveTo(98.666489f, -94.295059f);
path.lineTo(156.584320f, -61.939133f);
path.lineTo(174.672974f, -12.343765f);
path.lineTo(158.622345f, 52.028267f);
path.lineTo(98.666489f, -94.295059f);
path.close();
path.moveTo(-133.225616f, -48.622055f);
path.lineTo(-73.855499f, -10.375397f);
path.lineTo(-14.485367f, 27.871277f);
path.lineTo(44.884750f, 66.117935f);
path.lineTo(-133.225616f, -48.622055f);
path.close();
path.moveTo( 9.030045f, -163.413132f);
path.lineTo(-19.605331f, -89.588760f);
path.lineTo(-48.240707f, -15.764404f);
path.lineTo(-76.876053f, 58.059944f);
path.lineTo( 9.030045f, -163.413132f);
path.close();
    simplify(__FUNCTION__, path, true, out);
}

static void testSimplifySkinnyTriangle13() {
        SkPath path, out;
path.moveTo(340.41568f, -170.97171f);
path.lineTo(418.846893f, -142.428329f);
path.lineTo(497.278107f, -113.884933f);
path.lineTo(449.18222f, -45.6723022f);
path.lineTo(340.41568f, -170.97171f);
path.close();
path.moveTo(326.610535f, 34.0393639f);
path.lineTo(371.334595f, -14.9620667f);
path.lineTo(416.058624f, -63.9634857f);
path.lineTo(460.782654f, -112.96492f);
path.lineTo(326.610535f, 34.0393639f);
path.close();
    simplify(__FUNCTION__, path, true, out);
}

static void (*simplifyTests[])() = {
    testSimplifySkinnyTriangle13,
    testSimplifySkinnyTriangle12,
    testSimplifySkinnyTriangle11,
    testSimplifySkinnyTriangle10,
    testSimplifySkinnyTriangle9,
    testSimplifySkinnyTriangle8,
    testSimplifySkinnyTriangle7,
    testSimplifySkinnyTriangle6,
    testSimplifySkinnyTriangle5,
    testSimplifySkinnyTriangle4,
    testSimplifySkinnyTriangle3,
    testSimplifySkinnyTriangle2,
    testSimplifySkinnyTriangle1,
    testSimplifyTriangle24,
    testSimplifyTriangle23,
    testSimplifyTriangle22,
    testSimplifyDegenerateTriangle2,
    testSimplifyDegenerateTriangle1,
    testSimplifyTriangle21,
    testSimplifyTriangle20,
    testSimplifyTriangle19,
    testSimplifyTriangle18,
    testSimplifyTriangle17,
    testSimplifyTriangle16,
    testSimplifyTriangle15,
    testSimplifyTriangle14,
    testSimplifyTriangle13,
    testSimplifyTriangle12,
    testSimplifyTriangle11,
    testSimplifyTriangle10,
    testSimplifyTriangle7,
    testSimplifyTriangle9,
    testSimplifyTriangle8,
    testSimplifyTriangle6,
    testSimplifyTriangle5,
    testSimplifyTriangle4,
    testSimplifyTriangle3,
    testSimplifyTriangle,
    testSimplifyTriangle2,
    testSimplifyWindingParallelogram,
    testSimplifyXorParallelogram,
//    testPathTriangleRendering,
};

static size_t simplifyTestsCount = sizeof(simplifyTests) / sizeof(simplifyTests[0]);

static void (*firstTest)() = testSimplifySkinnyTriangle12;

void SimplifyPolygonPaths_Test() {
    size_t index = 0;
    if (firstTest) {
        while (index < simplifyTestsCount && simplifyTests[index] != firstTest) {
            ++index;
        }
    }
    bool firstTestComplete = false;
    for ( ; index < simplifyTestsCount; ++index) {
        (*simplifyTests[index])();
        if (simplifyTests[index] == testSimplifySkinnyTriangle2) {
            if (false) SkDebugf("%s last fast skinny test\n", __FUNCTION__);
        }
        firstTestComplete = true;
    }
}
