/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrEagerVertexAllocator.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/effects/GrPorterDuffXferProcessor.h"
#include "src/gpu/geometry/GrAATriangulator.h"
#include "src/gpu/geometry/GrInnerFanTriangulator.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/shaders/SkShaderBase.h"
#include "tools/ToolUtils.h"
#include <map>

/*
 * These tests pass by not crashing, hanging or asserting in Debug.
 */

using CreatePathFn = SkPath(*)();

CreatePathFn kNonEdgeAAPaths[] = {
    // Tests active edges made inactive by splitting.
    // Also tests active edge list forced into an invalid ordering by
    // splitting (mopped up in cleanup_active_edges()).
    []() -> SkPath {
        SkPath path;
        path.moveTo(229.127044677734375f,  67.34100341796875f);
        path.lineTo(187.8097381591796875f, -6.7729740142822265625f);
        path.lineTo(171.411407470703125f,  50.94266510009765625f);
        path.lineTo(245.5253753662109375f,  9.6253643035888671875f);
        path.moveTo(208.4683990478515625f, 30.284009933471679688f);
        path.lineTo(171.411407470703125f,  50.94266510009765625f);
        path.lineTo(187.8097381591796875f, -6.7729740142822265625f);
        return path;
    },

    // Intersections which fall exactly on the current vertex, and require
    // a restart of the intersection checking.
    []() -> SkPath {
        SkPath path;
        path.moveTo(314.483551025390625f, 486.246002197265625f);
        path.lineTo(385.41949462890625f,  532.8087158203125f);
        path.lineTo(373.232879638671875f, 474.05938720703125f);
        path.lineTo(326.670166015625f,    544.995361328125f);
        path.moveTo(349.951507568359375f, 509.52734375f);
        path.lineTo(373.232879638671875f, 474.05938720703125f);
        path.lineTo(385.41949462890625f,  532.8087158203125f);
        return path;
    },

    // Tests active edges which are removed by splitting.
    []() -> SkPath {
        SkPath path;
        path.moveTo(343.107391357421875f, 613.62176513671875f);
        path.lineTo(426.632415771484375f, 628.5740966796875f);
        path.lineTo(392.3460693359375f,   579.33544921875f);
        path.lineTo(377.39373779296875f,  662.86041259765625f);
        path.moveTo(384.869873046875f,    621.097900390625f);
        path.lineTo(392.3460693359375f,   579.33544921875f);
        path.lineTo(426.632415771484375f, 628.5740966796875f);
        return path;
    },

    // Collinear edges merged in set_top().
    // Also, an intersection between left and right enclosing edges which
    // falls above the current vertex.
    []() -> SkPath {
        SkPath path;
        path.moveTo(545.95751953125f,    791.69854736328125f);
        path.lineTo(612.05816650390625f, 738.494140625f);
        path.lineTo(552.4056396484375f,  732.0460205078125f);
        path.lineTo(605.61004638671875f, 798.14666748046875f);
        path.moveTo(579.00787353515625f, 765.0963134765625f);
        path.lineTo(552.4056396484375f,  732.0460205078125f);
        path.lineTo(612.05816650390625f, 738.494140625f);
        return path;
    },

    // Tests active edges which are made inactive by set_top().
    []() -> SkPath {
        SkPath path;
        path.moveTo(819.2725830078125f,  751.77447509765625f);
        path.lineTo(820.70904541015625f, 666.933837890625f);
        path.lineTo(777.57049560546875f, 708.63592529296875f);
        path.lineTo(862.4111328125f,     710.0723876953125f);
        path.moveTo(819.99078369140625f, 709.3541259765625f);
        path.lineTo(777.57049560546875f, 708.63592529296875f);
        path.lineTo(820.70904541015625f, 666.933837890625f);
        return path;
    },

    []() -> SkPath {
        SkPath path;
        path.moveTo(823.33209228515625f, 749.052734375f);
        path.lineTo(823.494873046875f,   664.20013427734375f);
        path.lineTo(780.9871826171875f,  706.5450439453125f);
        path.lineTo(865.8397216796875f,  706.70782470703125f);
        path.moveTo(823.4134521484375f,  706.6263427734375f);
        path.lineTo(780.9871826171875f,  706.5450439453125f);
        path.lineTo(823.494873046875f,   664.20013427734375f);
        return path;
    },

    []() -> SkPath {
        SkPath path;
        path.moveTo(954.862548828125f,   562.8349609375f);
        path.lineTo(899.32818603515625f, 498.679443359375f);
        path.lineTo(895.017578125f,      558.52435302734375f);
        path.lineTo(959.17315673828125f, 502.990081787109375f);
        path.moveTo(927.0953369140625f,  530.7572021484375f);
        path.lineTo(895.017578125f,      558.52435302734375f);
        path.lineTo(899.32818603515625f, 498.679443359375f);
        return path;
    },

    []() -> SkPath {
        SkPath path;
        path.moveTo(958.5330810546875f,  547.35516357421875f);
        path.lineTo(899.93109130859375f, 485.989013671875f);
        path.lineTo(898.54901123046875f, 545.97308349609375f);
        path.lineTo(959.9151611328125f,  487.37109375f);
        path.moveTo(929.2320556640625f,  516.67205810546875f);
        path.lineTo(898.54901123046875f, 545.97308349609375f);
        path.lineTo(899.93109130859375f, 485.989013671875f);
        return path;
    },

    []() -> SkPath {
        SkPath path;
        path.moveTo(389.8609619140625f,   369.326873779296875f);
        path.lineTo(470.6290283203125f,   395.33697509765625f);
        path.lineTo(443.250030517578125f, 341.9478759765625f);
        path.lineTo(417.239959716796875f, 422.7159423828125f);
        path.moveTo(430.244964599609375f, 382.3319091796875f);
        path.lineTo(443.250030517578125f, 341.9478759765625f);
        path.lineTo(470.6290283203125f,   395.33697509765625f);
        return path;
    },

    []() -> SkPath {
        SkPath path;
        path.moveTo(20, 20);
        path.lineTo(50, 80);
        path.lineTo(20, 80);
        path.moveTo(80, 50);
        path.lineTo(50, 50);
        path.lineTo(20, 50);
        return path;
    },

    []() -> SkPath {
        SkPath path;
        path.moveTo(257.19439697265625f, 320.876617431640625f);
        path.lineTo(190.113037109375f,   320.58978271484375f);
        path.lineTo(203.64404296875f,    293.8145751953125f);
        path.moveTo(203.357177734375f,   360.896026611328125f);
        path.lineTo(216.88824462890625f, 334.120819091796875f);
        path.lineTo(230.41925048828125f, 307.345611572265625f);
        return path;
    },

    // A degenerate segments case, where both upper and lower segments of
    // a split edge must remain active.
    []() -> SkPath {
        SkPath path;
        path.moveTo(231.9331207275390625f, 306.2012939453125f);
        path.lineTo(191.4859161376953125f, 306.04547119140625f);
        path.lineTo(231.0659332275390625f, 300.2642822265625f);
        path.moveTo(189.946807861328125f,  302.072265625f);
        path.lineTo(179.79705810546875f,   294.859771728515625f);
        path.lineTo(191.0016021728515625f, 296.165679931640625f);
        path.moveTo(150.8942108154296875f, 304.900146484375f);
        path.lineTo(179.708892822265625f,  297.849029541015625f);
        path.lineTo(190.4742279052734375f, 299.11895751953125f);
        return path;
    },

    // Handle the case where edge.dist(edge.fTop) != 0.0.
    []() -> SkPath {
        SkPath path;
        path.moveTo(                  0.0f,  400.0f);
        path.lineTo(                138.0f,  202.0f);
        path.lineTo(                  0.0f,  202.0f);
        path.moveTo( 12.62693023681640625f,  250.57464599609375f);
        path.lineTo(  8.13896942138671875f,  254.556884765625f);
        path.lineTo(-18.15641021728515625f,  220.40203857421875f);
        path.lineTo(-15.986493110656738281f, 219.6513519287109375f);
        path.moveTo( 36.931194305419921875f, 282.485504150390625f);
        path.lineTo( 15.617521286010742188f, 261.2901611328125f);
        path.lineTo( 10.3829498291015625f,   252.565765380859375f);
        path.lineTo(-16.165292739868164062f, 222.646026611328125f);
        return path;
    },

    // A degenerate segments case which exercises inactive edges being
    // made active by splitting.
    []() -> SkPath {
        SkPath path;
        path.moveTo(690.62127685546875f, 509.25555419921875f);
        path.lineTo(99.336181640625f,    511.71405029296875f);
        path.lineTo(708.362548828125f,   512.4349365234375f);
        path.lineTo(729.9940185546875f,  516.3114013671875f);
        path.lineTo(738.708984375f,      518.76995849609375f);
        path.lineTo(678.3463134765625f,  510.0819091796875f);
        path.lineTo(681.21795654296875f, 504.81378173828125f);
        path.moveTo(758.52764892578125f, 521.55963134765625f);
        path.lineTo(719.1549072265625f,  514.50372314453125f);
        path.lineTo(689.59063720703125f, 512.0628662109375f);
        path.lineTo(679.78216552734375f, 507.447845458984375f);
        return path;
    },

    // Tests vertices which become "orphaned" (ie., no connected edges)
    // after simplification.
    []() -> SkPath {
        SkPath path;
        path.moveTo(217.326019287109375f, 166.4752960205078125f);
        path.lineTo(226.279266357421875f, 170.929473876953125f);
        path.lineTo(234.3973388671875f,   177.0623626708984375f);
        path.lineTo(262.0921630859375f,   188.746124267578125f);
        path.moveTo(196.23638916015625f,  174.0722198486328125f);
        path.lineTo(416.15277099609375f,  180.138214111328125f);
        path.lineTo(192.651947021484375f, 304.0228271484375f);
        return path;
    },

    []() -> SkPath {
        SkPath path;
        path.moveTo(    0.0f,   0.0f);
        path.lineTo(10000.0f,   0.0f);
        path.lineTo(    0.0f,  -1.0f);
        path.lineTo(10000.0f,   0.000001f);
        path.lineTo(    0.0f, -30.0f);
        return path;
    },

    // Reduction of Nebraska-StateSeal.svg. Floating point error causes the
    // same edge to be added to more than one poly on the same side.
    []() -> SkPath {
        SkPath path;
        path.moveTo(170.8199920654296875,   491.86700439453125);
        path.lineTo(173.7649993896484375,    489.7340087890625);
        path.lineTo(174.1450958251953125,  498.545989990234375);
        path.lineTo( 171.998992919921875,   500.88201904296875);
        path.moveTo(168.2922515869140625,   498.66265869140625);
        path.lineTo(169.8589935302734375,   497.94500732421875);
        path.lineTo(                 172,   500.88299560546875);
        path.moveTo( 169.555267333984375,   490.70111083984375);
        path.lineTo(173.7649993896484375,    489.7340087890625);
        path.lineTo(  170.82000732421875,   491.86700439453125);
        return path;
    },

    // A shape with a vertex collinear to the right hand edge.
    // This messes up find_enclosing_edges.
    []() -> SkPath {
        SkPath path;
        path.moveTo(80, 20);
        path.lineTo(80, 60);
        path.lineTo(20, 60);
        path.moveTo(80, 50);
        path.lineTo(80, 80);
        path.lineTo(20, 80);
        return path;
    },

    // Exercises the case where an edge becomes collinear with *two* of its
    // adjacent neighbour edges after splitting.
    // This is a reduction from
    // http://mooooo.ooo/chebyshev-sine-approximation/horner_ulp.svg
    []() -> SkPath {
        SkPath path;
        path.moveTo(  351.99298095703125,         348.23046875);
        path.lineTo(  351.91876220703125,         347.33984375);
        path.lineTo(  351.91876220703125,          346.1953125);
        path.lineTo(  351.90313720703125,           347.734375);
        path.lineTo(  351.90313720703125,          346.1328125);
        path.lineTo(  351.87579345703125,         347.93359375);
        path.lineTo(  351.87579345703125,           345.484375);
        path.lineTo(  351.86407470703125,          347.7890625);
        path.lineTo(  351.86407470703125,          346.2109375);
        path.lineTo(  351.84844970703125,   347.63763427734375);
        path.lineTo(  351.84454345703125,   344.19232177734375);
        path.lineTo(  351.78204345703125,    346.9483642578125);
        path.lineTo( 351.758636474609375,      347.18310546875);
        path.lineTo(  351.75469970703125,               346.75);
        path.lineTo(  351.75469970703125,            345.46875);
        path.lineTo(         352.5546875,            345.46875);
        path.lineTo(        352.55078125,         347.01953125);
        path.lineTo(  351.75079345703125,   347.02313232421875);
        path.lineTo(  351.74688720703125,   346.15203857421875);
        path.lineTo(  351.74688720703125,  347.646148681640625);
        path.lineTo(         352.5390625,         346.94140625);
        path.lineTo(  351.73907470703125,   346.94268798828125);
        path.lineTo(  351.73516845703125,   344.48565673828125);
        path.lineTo(          352.484375,         346.73828125);
        path.lineTo(  351.68438720703125,    346.7401123046875);
        path.lineTo(         352.4765625,           346.546875);
        path.lineTo(  351.67657470703125,   346.54937744140625);
        path.lineTo(        352.47265625,         346.75390625);
        path.lineTo(  351.67266845703125,  346.756622314453125);
        path.lineTo(  351.66876220703125,  345.612091064453125);
        return path;
    },

    // A path which contains out-of-range colinear intersections.
    []() -> SkPath {
        SkPath path;
        path.moveTo(                   0, 63.39080047607421875);
        path.lineTo(-0.70804601907730102539, 63.14350128173828125);
        path.lineTo(-7.8608899287380243391e-17, 64.14080047607421875);
        path.moveTo(                   0, 64.14080047607421875);
        path.lineTo(44.285900115966796875, 64.14080047607421875);
        path.lineTo(                   0, 62.64080047607421875);
        path.moveTo(21.434900283813476562, -0.24732701480388641357);
        path.lineTo(-0.70804601907730102539, 63.14350128173828125);
        path.lineTo(0.70804601907730102539,  63.6381988525390625);
        return path;
    },

    // A path which results in infs and nans when conics are converted to quads.
    []() -> SkPath {
         SkPath path;
         path.moveTo(-2.20883e+37f, -1.02892e+37f);
         path.conicTo(-2.00958e+38f, -9.36107e+37f, -1.7887e+38f, -8.33215e+37f, 0.707107f);
         path.conicTo(-1.56782e+38f, -7.30323e+37f, 2.20883e+37f, 1.02892e+37f, 0.707107f);
         path.conicTo(2.00958e+38f, 9.36107e+37f, 1.7887e+38f, 8.33215e+37f, 0.707107f);
         path.conicTo(1.56782e+38f, 7.30323e+37f, -2.20883e+37f, -1.02892e+37f, 0.707107f);
         return path;
    },

    // A quad which generates a huge number of points (>2B) when uniformly
    // linearized. This should not hang or OOM.
    []() -> SkPath {
        SkPath path;
        path.moveTo(10, 0);
        path.lineTo(0, 0);
        path.quadTo(10, 0, 0, 8315084722602508288);
        return path;
    },

    // A path which hangs during simplification. It produces an edge which is
    // to the left of its own endpoints, which causes an infinite loop in the
    // right-enclosing-edge splitting.
    []() -> SkPath {
        SkPath path;
        path.moveTo(0.75001740455627441406,     23.051967620849609375);
        path.lineTo(5.8471612930297851562,      22.731662750244140625);
        path.lineTo(10.749670028686523438,      22.253145217895507812);
        path.lineTo(13.115868568420410156,      22.180681228637695312);
        path.lineTo(15.418928146362304688,      22.340015411376953125);
        path.lineTo(  17.654022216796875,       22.82159423828125);
        path.lineTo(19.81632232666015625,       23.715869903564453125);
        path.lineTo(40,                         0);
        path.lineTo(5.5635203441547955577e-15,  0);
        path.lineTo(5.5635203441547955577e-15,  47);
        path.lineTo(-1.4210854715202003717e-14, 21.713298797607421875);
        path.lineTo(0.75001740455627441406,     21.694292068481445312);
        path.lineTo(0.75001740455627441406,     23.051967620849609375);
        return path;
    },

    // Reduction from skbug.com/7911 that causes a crash due to splitting a
    // zombie edge.
    []() -> SkPath {
        SkPath path;
        path.moveTo(                   0, 1.0927740941146660348e+24);
        path.lineTo(2.9333931225865729333e+32,             16476101);
        path.lineTo(1.0927731573659435417e+24, 1.0927740941146660348e+24);
        path.lineTo(1.0927740941146660348e+24, 3.7616281094287041715e-37);
        path.lineTo(1.0927740941146660348e+24, 1.0927740941146660348e+24);
        path.lineTo(1.3061803026169399536e-33, 1.0927740941146660348e+24);
        path.lineTo(4.7195362919941370727e-16, -8.4247545146051822591e+32);
        return path;
    },

    // From crbug.com/844873. Crashes trying to merge a zombie edge.
    []() -> SkPath {
        SkPath path;
        path.moveTo( 316.000579833984375, -4338355948977389568);
        path.lineTo(1.5069369808623501312e+20, 75180972320904708096.0);
        path.lineTo(1.5069369808623501312e+20, 75180972320904708096.0);
        path.lineTo(  771.21014404296875, -4338355948977389568.0);
        path.lineTo( 316.000579833984375, -4338355948977389568.0);
        path.moveTo(       354.208984375, -4338355948977389568.0);
        path.lineTo(  773.00177001953125, -4338355948977389568.0);
        path.lineTo(1.5069369808623501312e+20, 75180972320904708096.0);
        path.lineTo(1.5069369808623501312e+20, 75180972320904708096.0);
        path.lineTo(       354.208984375, -4338355948977389568.0);
        return path;
    },

    // From crbug.com/844873. Hangs repeatedly splitting alternate vertices.
    []() -> SkPath {
        SkPath path;
        path.moveTo(10, -1e+20f);
        path.lineTo(11, 25000);
        path.lineTo(10, 25000);
        path.lineTo(11, 25010);
        return path;
    },

    // Reduction from circular_arcs_stroke_and_fill_round GM which
    // repeatedly splits on the opposite edge from case 34 above.
    []() -> SkPath {
        SkPath path;
        path.moveTo(               16.25, 26.495191574096679688);
        path.lineTo(32.420825958251953125, 37.377376556396484375);
        path.lineTo(25.176382064819335938, 39.31851959228515625);
        path.moveTo(                  20,                   20);
        path.lineTo(28.847436904907226562, 37.940830230712890625);
        path.lineTo(25.17638397216796875, 39.31851959228515625);
        return path;
    },

    // Reduction from crbug.com/843135 where an intersection is found
    // below the bottom of both intersected edges.
    []() -> SkPath {
        SkPath path;
        path.moveTo(-2791476679359332352,  2608107002026524672);
        path.lineTo(                   0, 11.95427703857421875);
        path.lineTo(-2781824066779086848,  2599088532777598976);
        path.lineTo(          -7772.6875,                 7274);
        return path;
    },

    // Reduction from crbug.com/843135. Exercises a case where an intersection is missed.
    // This causes bad ordering in the active edge list.
    []() -> SkPath {
        SkPath path;
        path.moveTo(-1.0662557646016024569e+23, 9.9621425197286319718e+22);
        path.lineTo(                -121806400,                 113805032);
        path.lineTo(                -120098872,                 112209680);
        path.lineTo( 6.2832999862817380468e-36,     2.9885697364807128906);
        return path;
    },

    // Reduction from crbug.com/851409. Exercises collinear last vertex.
    []() -> SkPath {
        SkPath path;
        path.moveTo(2072553216, 0);
        path.lineTo(2072553216, 1);
        path.lineTo(2072553472, -13.5);
        path.lineTo(2072553216, 0);
        path.lineTo(2072553472, -6.5);
        return path;
    },

    // Another reduction from crbug.com/851409. Exercises two sequential collinear edges.
    []() -> SkPath {
        SkPath path;
        path.moveTo(2072553216, 0);
        path.lineTo(2072553216, 1);
        path.lineTo(2072553472, -13);
        path.lineTo(2072553216, 0);
        path.lineTo(2072553472, -6);
        path.lineTo(2072553472, -13);
        return path;
    },

    // Reduction from crbug.com/860655. Cause is three collinear edges discovered during
    // sanitize_contours pass, before the vertices have been found coincident.
    []() -> SkPath {
        SkPath path;
        path.moveTo(   32572426382475264,    -3053391034974208);
        path.lineTo(           521289856,            -48865776);
        path.lineTo(           130322464,            -12215873);
        path.moveTo(   32572426382475264,    -3053391034974208);
        path.lineTo(           521289856,            -48865776);
        path.lineTo(           130322464,            -12215873);
        path.moveTo(   32572426382475264,    -3053391034974208);
        path.lineTo(   32114477642022912,    -3010462031544320);
        path.lineTo(   32111784697528320,    -3010209702215680);
        return path;
    },
};

#if SK_GPU_V1
#include "src/gpu/ops/TriangulatingPathRenderer.h"
#include "src/gpu/v1/SurfaceDrawContext_v1.h"

// A simple concave path. Test this with a non-invertible matrix.
static SkPath create_path_17() {
    SkPath path;
    path.moveTo(20, 20);
    path.lineTo(80, 20);
    path.lineTo(30, 30);
    path.lineTo(20, 80);
    return path;
}

// An intersection above the first vertex in the mesh.
// Reduction from http://crbug.com/730687
static SkPath create_path_20() {
    SkPath path;
    path.moveTo(           2822128.5,  235.026336669921875);
    path.lineTo(          2819349.25, 235.3623504638671875);
    path.lineTo(          -340558688, 23.83478546142578125);
    path.lineTo(          -340558752, 25.510419845581054688);
    path.lineTo(          -340558720, 27.18605804443359375);
    return path;
}

// An intersection whose result is NaN (due to rounded-to-inf endpoint).
static SkPath create_path_21() {
    SkPath path;
    path.moveTo(1.7889142061167663539e+38, 39338463358011572224.0);
    path.lineTo(  1647.4193115234375,       -522.603515625);
    path.lineTo(    1677.74560546875,   -529.0028076171875);
    path.lineTo(    1678.29541015625,   -528.7847900390625);
    path.lineTo(  1637.5167236328125,  -519.79266357421875);
    path.lineTo(  1647.4193115234375,       -522.603515625);
    return path;
}

// An edge collapse event which also collapses a neighbour, requiring
// its event to be removed.
static SkPath create_path_25() {
    SkPath path;
    path.moveTo( 43.44110107421875,  148.15106201171875);
    path.lineTo( 44.64471435546875,  148.16748046875);
    path.lineTo( 46.35009765625,     147.403076171875);
    path.lineTo( 46.45404052734375,  148.34906005859375);
    path.lineTo( 45.0400390625,      148.54205322265625);
    path.lineTo( 44.624053955078125, 148.9810791015625);
    path.lineTo( 44.59405517578125,  149.16107177734375);
    path.lineTo( 44.877044677734375, 149.62005615234375);
    path.lineTo(144.373016357421875,  68.8070068359375);
    return path;
}

// An edge collapse event causes an edge to become collinear, requiring
// its event to be removed.
static SkPath create_path_26() {
    SkPath path;
    path.moveTo( 43.44110107421875,  148.15106201171875);
    path.lineTo( 44.64471435546875,  148.16748046875);
    path.lineTo( 46.35009765625,     147.403076171875);
    path.lineTo( 46.45404052734375,  148.34906005859375);
    path.lineTo( 45.0400390625,      148.54205322265625);
    path.lineTo( 44.624053955078125, 148.9810791015625);
    path.lineTo( 44.59405517578125,  149.16107177734375);
    path.lineTo( 44.877044677734375, 149.62005615234375);
    path.lineTo(144.373016357421875,  68.8070068359375);
    return path;
}

// A path which results in non-finite points when stroked and bevelled for AA.
static SkPath create_path_27() {
     SkPath path;
     path.moveTo(8.5027233009104409507e+37, 1.7503381025241130639e+37);
     path.lineTo(7.0923661737711584874e+37, 1.4600074517285415699e+37);
     path.lineTo(7.0848733446033294691e+37, 1.4584649744781838604e+37);
     path.lineTo(-2.0473916115129349496e+37, -4.2146796450364162012e+36);
     path.lineTo(2.0473912312177548811e+37, 4.2146815465123165435e+36);
     return path;
}

// AA stroking this path produces intersection failures on bevelling.
// This should skip the point, but not assert.
static SkPath create_path_28() {
    SkPath path;
    path.moveTo(-7.5952312625177475154e+21, -2.6819185100266674911e+24);
    path.lineTo(  1260.3787841796875,   1727.7947998046875);
    path.lineTo(  1260.5567626953125,   1728.0386962890625);
    path.lineTo(1.1482511310557754163e+21, 4.054538502765980051e+23);
    path.lineTo(-7.5952312625177475154e+21, -2.6819185100266674911e+24);
    return path;
}

// A path with vertices which become infinite on AA stroking. Should not crash or assert.
static SkPath create_path_31() {
    SkPath path;
    path.moveTo(2.0257809259190991347e+36,  -1244080640);
    path.conicTo(2.0257809259190991347e+36, -1244080640,
                 2.0257809259190991347e+36, 0.10976474732160568237, 0.70710676908493041992);
    path.lineTo(-10036566016, -1954718402215936);
    path.conicTo(-1.1375507718551896064e+20, -1954721086570496,
                 10036566016, -1954721086570496, 0.70710676908493041992);
    return path;
}

// Reduction from crbug.com/851914.
static SkPath create_path_38() {
    SkPath path;
    path.moveTo(14.400531768798828125, 17.711114883422851562);
    path.lineTo(14.621990203857421875,   171563104293879808);
    path.lineTo(14.027951240539550781,   872585759381520384);
    path.lineTo( 14.0216827392578125,   872665817571917824);
    path.lineTo(7.699314117431640625,    -3417320793833472);
    path.moveTo(11.606547355651855469,       17.40966796875);
    path.lineTo( 7642114886926860288, 21.08358001708984375);
    path.lineTo(11.606547355651855469, 21.08358001708984375);
    return path;
}

// Reduction from crbug.com/860453. Tests a case where a "missing" intersection
// requires the active edge list to go out-of-order.
static SkPath create_path_41() {
    SkPath path;
    path.moveTo(72154931603311689728.0,   330.95965576171875);
    path.lineTo(24053266013925408768.0,       78.11376953125);
    path.lineTo(1.2031099003292404941e+20,  387.168731689453125);
    path.lineTo(68859835992355373056.0,   346.55047607421875);
    path.lineTo(76451708695451009024.0,     337.780029296875);
    path.moveTo(-20815817797613387776.0, 18065700622522384384.0);
    path.lineTo(-72144121204987396096.0,  142.855804443359375);
    path.lineTo(72144121204987396096.0,  325.184783935546875);
    path.lineTo(1.2347242901040791552e+20, 18065700622522384384.0);
    return path;
}

// Reduction from crbug.com/866319. Cause is edges that are collinear when tested from
// one side, but non-collinear when tested from the other.
static SkPath create_path_43() {
    SkPath path;
    path.moveTo(     307316821852160,      -28808363114496);
    path.lineTo(     307165222928384,      -28794154909696);
    path.lineTo(     307013691113472,      -28779948802048);
    path.lineTo(     306862159298560,      -28765744791552);
    path.lineTo(     306870313025536,      -28766508154880);
    path.lineTo(     307049695019008,      -28783327313920);
    path.lineTo(     307408660332544,      -28816974020608);
    return path;
}

// Reduction from crbug.com/966696
static SkPath create_path_44() {
    SkPath path;
    path.moveTo(114.4606170654296875,       186.443878173828125);
    path.lineTo( 91.5394744873046875,       185.4189453125);
    path.lineTo(306.45538330078125,        3203.986083984375);
    path.moveTo(16276206965409972224.0,     815.59393310546875);
    path.lineTo(-3.541605062372533207e+20,  487.7236328125);
    path.lineTo(-3.541605062372533207e+20,  168.204071044921875);
    path.lineTo(16276206965409972224.0,     496.07427978515625);
    path.moveTo(-3.541605062372533207e+20,  167.00958251953125);
    path.lineTo(-3.541605062372533207e+20,  488.32086181640625);
    path.lineTo(16276206965409972224.0,     816.78839111328125);
    path.lineTo(16276206965409972224.0,     495.47705078125);
    return path;
}

// Reduction from crbug.com/966274.
static SkPath create_path_45() {
    SkPath path;
    path.moveTo(        706471854080,         379003666432);
    path.lineTo(        706503180288,         379020443648);
    path.lineTo(        706595717120,         379070087168);
    path.lineTo(        706626060288,         379086372864);
    path.lineTo(        706656141312,         379102527488);
    path.lineTo(        706774171648,         379165835264);
    path.lineTo(        706803073024,         379181334528);
    path.lineTo(        706831712256,         379196702720);
    path.lineTo(        706860154880,         379211939840);
    path.lineTo(        706888335360,         379227078656);
    path.lineTo(        706916253696,         379242053632);
    path.lineTo(        706956820480,         379263811584);
    path.lineTo(        706929098752,         379248934912);
    path.lineTo(        706901114880,         379233927168);
    path.lineTo(        706872934400,         379218821120);
    path.lineTo(        706844491776,         379203551232);
    path.lineTo(        706815787008,         379188183040);
    path.lineTo(        706786885632,         379172651008);
    path.lineTo(        706757722112,         379156987904);
    path.lineTo(        706728296448,         379141226496);
    path.lineTo(        706698608640,         379125301248);
    path.lineTo(        706668724224,         379109244928);
    path.lineTo(        706638577664,         379093090304);
    path.lineTo(        706608168960,         379076771840);
    path.lineTo(        706484174848,         379010252800);
    return path;
}

// Reduction from crbug.com/969359. Inf generated by intersections
// causes NaN in subsequent intersections, leading to assert or hang.

static SkPath create_path_46() {
    SkPath path;
    path.moveTo(1.0321827899075254821e+37, -5.1199920965387697886e+37);
    path.lineTo(-1.0321827899075254821e+37, 5.1199920965387697886e+37);
    path.lineTo(-1.0425214946728668754e+37, 4.5731834042267216669e+37);
    path.moveTo(-9.5077331762291841872e+36, 8.1304868292377430302e+37);
    path.lineTo(9.5077331762291841872e+36, -8.1304868292377430302e+37);
    path.lineTo(1.0795449417808426232e+37, 1.2246856113744539311e+37);
    path.moveTo(-165.8018341064453125,           -44.859375);
    path.lineTo(-9.558702871563160835e+36, -7.9814405281448285475e+37);
    path.lineTo(-9.4147814283168490381e+36, -8.3935116522790983488e+37);
    return path;
}

// Reduction from crbug.com/1245359
static SkPath create_path_47() {
    SkPath path;
    path.setFillType(SkPathFillType::kWinding);
    path.moveTo(SkBits2Float(0xdfb80000), SkBits2Float(0x4cb9b4a5)); // -2.65172e+19f,  9.73632e+07f
    path.lineTo(SkBits2Float(0xdfb80000), SkBits2Float(0xe396b530)); // -2.65172e+19f, -5.56014e+21f
    path.lineTo(SkBits2Float(0x5fb80000), SkBits2Float(0xe396b530)); //  2.65172e+19f, -5.56014e+21f
    path.lineTo(SkBits2Float(0x5fb80000), SkBits2Float(0x6396b530)); //  2.65172e+19f,  5.56014e+21f
    path.lineTo(SkBits2Float(0x4cc07742), SkBits2Float(0x6396b530)); //  1.00908e+08f,  5.56014e+21f
    path.lineTo(SkBits2Float(0xdfb80000), SkBits2Float(0x6396b530)); // -2.65172e+19f,  5.56014e+21f
    path.lineTo(SkBits2Float(0xdfb80000), SkBits2Float(0xe396b530)); // -2.65172e+19f, -5.56014e+21f
    path.lineTo(SkBits2Float(0x4cc07742), SkBits2Float(0xe396b530)); //  1.00908e+08f, -5.56014e+21f
    path.lineTo(SkBits2Float(0x4cc079c8), SkBits2Float(0xe396b530)); //  1.00913e+08f, -5.56014e+21f
    path.lineTo(SkBits2Float(0x4cc079c8), SkBits2Float(0x4cb9b4a5)); //  1.00913e+08f,  9.73632e+07f
    path.lineTo(SkBits2Float(0xdfb80000), SkBits2Float(0x4cb9b4a5)); // -2.65172e+19f,  9.73632e+07f
    path.lineTo(SkBits2Float(0xdfb80000), SkBits2Float(0x4cb74d74)); // -2.65172e+19f,  9.61033e+07f
    path.lineTo(SkBits2Float(0x4cc079c8), SkBits2Float(0x4cb74d74)); //  1.00913e+08f,  9.61033e+07f
    path.lineTo(SkBits2Float(0x4cc079c8), SkBits2Float(0x6396b530)); //  1.00913e+08f,  5.56014e+21f
    path.lineTo(SkBits2Float(0x4cc07742), SkBits2Float(0x6396b530)); //  1.00908e+08f,  5.56014e+21f
    path.lineTo(SkBits2Float(0x4cc07742), SkBits2Float(0x4cb74d74)); //  1.00908e+08f,  9.61033e+07f
    path.lineTo(SkBits2Float(0x5fb80000), SkBits2Float(0x4cb74d74)); //  2.65172e+19f,  9.61033e+07f
    path.lineTo(SkBits2Float(0x5fb80000), SkBits2Float(0x6396b530)); //  2.65172e+19f,  5.56014e+21f
    path.lineTo(SkBits2Float(0xdfb80000), SkBits2Float(0x6396b530)); // -2.65172e+19f,  5.56014e+21f
    path.lineTo(SkBits2Float(0xdfb80000), SkBits2Float(0x4cb9b4a5)); // -2.65172e+19f,  9.73632e+07f
    path.close();

    path.moveTo(SkBits2Float(0xdfb39e51), SkBits2Float(0xe282c5bd)); // -2.58857e+19f, -1.20616e+21f
    path.lineTo(SkBits2Float(0xdf8a47ec), SkBits2Float(0xe3b90de5)); // -1.99284e+19f, -6.8273e+21f
    path.lineTo(SkBits2Float(0x5eb8b548), SkBits2Float(0xe391e278)); //  6.65481e+18f, -5.38219e+21f
    path.quadTo(SkBits2Float(0x5eaa9855), SkBits2Float(0xe392a246),  //  6.14633e+18f, -5.40984e+21f
                SkBits2Float(0x5e9c5925), SkBits2Float(0xe39344a0)); //  5.63304e+18f, -5.43323e+21f
    path.quadTo(SkBits2Float(0x5e89eefd), SkBits2Float(0xe3941678),  //  4.96958e+18f, -5.46347e+21f
                SkBits2Float(0x5e6ead5a), SkBits2Float(0xe394b6a4)); //  4.29963e+18f, -5.48656e+21f
    path.quadTo(SkBits2Float(0x5e6c0307), SkBits2Float(0xe394c21f),  //  4.25161e+18f, -5.48821e+21f
                SkBits2Float(0x5e694ef2), SkBits2Float(0xe394cd7f)); //  4.20291e+18f, -5.48985e+21f
    path.quadTo(SkBits2Float(0x5e67eeaa), SkBits2Float(0xe394d349),  //  4.17812e+18f, -5.49069e+21f
                SkBits2Float(0x5e669614), SkBits2Float(0xe394d8e2)); //  4.15387e+18f, -5.49149e+21f
    path.quadTo(SkBits2Float(0x5e6534d4), SkBits2Float(0xe394de9e),  //  4.12901e+18f, -5.49232e+21f
                SkBits2Float(0x5e63d6a7), SkBits2Float(0xe394e43c)); //  4.10437e+18f, -5.49313e+21f
    path.quadTo(SkBits2Float(0x5e610d59), SkBits2Float(0xe394efad),  //  4.05418e+18f, -5.49478e+21f
                SkBits2Float(0x5e5e43cb), SkBits2Float(0xe394fad6)); //  4.00397e+18f, -5.49639e+21f
    path.quadTo(SkBits2Float(0x5e5b6ac0), SkBits2Float(0xe395063d),  //  3.95267e+18f, -5.49803e+21f
                SkBits2Float(0x5e5895ab), SkBits2Float(0xe3951148)); //  3.90164e+18f, -5.49962e+21f
    path.quadTo(SkBits2Float(0x5e55b52e), SkBits2Float(0xe3951c7f),  //  3.84982e+18f, -5.50124e+21f
                SkBits2Float(0x5e52cb8e), SkBits2Float(0xe395278b)); //  3.79735e+18f, -5.50283e+21f
    path.quadTo(SkBits2Float(0x5e514f61), SkBits2Float(0xe3952d2d),  //  3.7706e+18f,  -5.50364e+21f
                SkBits2Float(0x5e4fdbc5), SkBits2Float(0xe395329a)); //  3.74445e+18f, -5.50442e+21f
    path.lineTo(SkBits2Float(0x5e4fdbc5), SkBits2Float(0xe395329a)); //  3.74445e+18f, -5.50442e+21f
    path.lineTo(SkBits2Float(0x5e4fdbc5), SkBits2Float(0xe395329a)); //  3.74445e+18f, -5.50442e+21f
    path.lineTo(SkBits2Float(0x5e4fdbc5), SkBits2Float(0xe395329a)); //  3.74445e+18f, -5.50442e+21f
    path.lineTo(SkBits2Float(0x5e4fdbc5), SkBits2Float(0xe395329a)); //  3.74445e+18f, -5.50442e+21f
    path.lineTo(SkBits2Float(0x5e4fdbc5), SkBits2Float(0xe395329a)); //  3.74445e+18f, -5.50442e+21f
    path.lineTo(SkBits2Float(0x5e4fdbc5), SkBits2Float(0xe395329a)); //  3.74445e+18f, -5.50442e+21f
    path.lineTo(SkBits2Float(0x5e4fdbc5), SkBits2Float(0xe395329a)); //  3.74445e+18f, -5.50442e+21f
    path.lineTo(SkBits2Float(0x5e4fdbc5), SkBits2Float(0xe395329a)); //  3.74445e+18f, -5.50442e+21f
    path.lineTo(SkBits2Float(0x5e4fdbc5), SkBits2Float(0xe395329a)); //  3.74445e+18f, -5.50442e+21f
    path.lineTo(SkBits2Float(0xdfb80000), SkBits2Float(0xe396b530)); // -2.65172e+19f, -5.56014e+21f
    path.lineTo(SkBits2Float(0x5fb80000), SkBits2Float(0xe396b530)); //  2.65172e+19f, -5.56014e+21f
    path.lineTo(SkBits2Float(0x5fb80000), SkBits2Float(0x4cc8d35d)); //  2.65172e+19f,  1.0529e+08f
    path.lineTo(SkBits2Float(0xdfe2ba48), SkBits2Float(0x63512f2f)); // -3.26749e+19f,  3.85877e+21f
    path.lineTo(SkBits2Float(0xdf7f64f6), SkBits2Float(0xe3b9b457)); // -1.84031e+19f, -6.85129e+21f
    path.lineTo(SkBits2Float(0xdfb80000), SkBits2Float(0x4cc8d35d)); // -2.65172e+19f,  1.0529e+08f
    path.lineTo(SkBits2Float(0xdfb80000), SkBits2Float(0x4cbbf2a2)); // -2.65172e+19f,  9.85388e+07f
    path.lineTo(SkBits2Float(0x4cc079c8), SkBits2Float(0x4cbbf2a2)); //  1.00913e+08f,  9.85388e+07f
    path.lineTo(SkBits2Float(0x4cc079c8), SkBits2Float(0x6396b530)); //  1.00913e+08f,  5.56014e+21f
    path.lineTo(SkBits2Float(0x4cc07742), SkBits2Float(0x6396b530)); //  1.00908e+08f,  5.56014e+21f
    path.lineTo(SkBits2Float(0x4cc07742), SkBits2Float(0x4cbbf2a2)); //  1.00908e+08f,  9.85388e+07f
    path.lineTo(SkBits2Float(0x5fb80000), SkBits2Float(0x4cbbf2a2)); //  2.65172e+19f,  9.85388e+07f
    path.lineTo(SkBits2Float(0xdeb8b548), SkBits2Float(0x6391e278)); // -6.65481e+18f,  5.38219e+21f
    path.lineTo(SkBits2Float(0x4cc07488), SkBits2Float(0x4ccb2302)); //  1.00902e+08f,  1.06502e+08f
    path.lineTo(SkBits2Float(0x5fb39e51), SkBits2Float(0x6282c5bd)); //  2.58857e+19f,  1.20616e+21f
    path.lineTo(SkBits2Float(0x5fb39e51), SkBits2Float(0x6282c5bd)); //  2.58857e+19f,  1.20616e+21f
    path.lineTo(SkBits2Float(0x5f8bb406), SkBits2Float(0x63b3cfe4)); //  2.01334e+19f,  6.63389e+21f
    path.lineTo(SkBits2Float(0xdfdb889b), SkBits2Float(0x6364da0b)); // -3.16381e+19f,  4.22157e+21f
    path.lineTo(SkBits2Float(0xdfb39e51), SkBits2Float(0xe282c5bd)); // -2.58857e+19f, -1.20616e+21f
    path.close();
    return path;
}

static std::unique_ptr<GrFragmentProcessor> create_linear_gradient_processor(
            GrRecordingContext* rContext) {

    SkPoint pts[2] = { {0, 0}, {1, 1} };
    SkColor colors[2] = { SK_ColorGREEN, SK_ColorBLUE };
    sk_sp<SkShader> shader = SkGradientShader::MakeLinear(
        pts, colors, nullptr, SK_ARRAY_COUNT(colors), SkTileMode::kClamp);
    GrColorInfo colorInfo(GrColorType::kRGBA_8888, kPremul_SkAlphaType, nullptr);
    SkMatrixProvider matrixProvider(SkMatrix::I());
    return as_SB(shader)->asFragmentProcessor({rContext, matrixProvider, &colorInfo});
}

static void test_path(GrRecordingContext* rContext,
                      skgpu::v1::SurfaceDrawContext* sdc,
                      const SkPath& path,
                      const SkMatrix& matrix = SkMatrix::I(),
                      GrAAType aaType = GrAAType::kNone,
                      std::unique_ptr<GrFragmentProcessor> fp = nullptr) {
    skgpu::v1::TriangulatingPathRenderer pr;
    pr.setMaxVerbCount(100);

    GrPaint paint;
    paint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));
    if (fp) {
        paint.setColorFragmentProcessor(std::move(fp));
    }

    SkIRect clipConservativeBounds = SkIRect::MakeWH(sdc->width(), sdc->height());
    GrStyle style(SkStrokeRec::kFill_InitStyle);
    GrStyledShape shape(path, style);
    skgpu::v1::PathRenderer::DrawPathArgs args{rContext,
                                               std::move(paint),
                                               &GrUserStencilSettings::kUnused,
                                               sdc,
                                               nullptr,
                                               &clipConservativeBounds,
                                               &matrix,
                                               &shape,
                                               aaType,
                                               false};
    pr.drawPath(args);
}

DEF_GPUTEST_FOR_ALL_CONTEXTS(TriangulatingPathRendererTests, reporter, ctxInfo) {
    auto ctx = ctxInfo.directContext();
    auto sdc = skgpu::v1::SurfaceDrawContext::Make(
            ctx, GrColorType::kRGBA_8888, nullptr, SkBackingFit::kApprox, {800, 800},
            SkSurfaceProps(), 1, GrMipmapped::kNo, GrProtected::kNo, kTopLeft_GrSurfaceOrigin);
    if (!sdc) {
        return;
    }

    ctx->flushAndSubmit();
    // Adding discard to appease vulkan validation warning about loading uninitialized data on draw
    sdc->discard();

    for (CreatePathFn createPath : kNonEdgeAAPaths) {
        test_path(ctx, sdc.get(), createPath());
    }
    SkMatrix nonInvertibleMatrix = SkMatrix::Scale(0, 0);
    std::unique_ptr<GrFragmentProcessor> fp(create_linear_gradient_processor(ctx));
    test_path(ctx, sdc.get(), create_path_17(), nonInvertibleMatrix, GrAAType::kCoverage,
              std::move(fp));
    test_path(ctx, sdc.get(), create_path_20(), SkMatrix(), GrAAType::kCoverage);
    test_path(ctx, sdc.get(), create_path_21(), SkMatrix(), GrAAType::kCoverage);
    test_path(ctx, sdc.get(), create_path_25(), SkMatrix(), GrAAType::kCoverage);
    test_path(ctx, sdc.get(), create_path_26(), SkMatrix(), GrAAType::kCoverage);
    test_path(ctx, sdc.get(), create_path_27(), SkMatrix(), GrAAType::kCoverage);
    test_path(ctx, sdc.get(), create_path_28(), SkMatrix(), GrAAType::kCoverage);
    test_path(ctx, sdc.get(), create_path_31(), SkMatrix(), GrAAType::kCoverage);
    test_path(ctx, sdc.get(), create_path_38(), SkMatrix(), GrAAType::kCoverage);
    test_path(ctx, sdc.get(), create_path_41(), SkMatrix(), GrAAType::kCoverage);
    test_path(ctx, sdc.get(), create_path_43(), SkMatrix(), GrAAType::kCoverage);
    test_path(ctx, sdc.get(), create_path_44(), SkMatrix(), GrAAType::kCoverage);
    test_path(ctx, sdc.get(), create_path_45(), SkMatrix(), GrAAType::kCoverage);
    test_path(ctx, sdc.get(), create_path_46(), SkMatrix(), GrAAType::kCoverage);
    test_path(ctx, sdc.get(), create_path_47(), SkMatrix(), GrAAType::kCoverage);
}

#endif // SK_GPU_V1

namespace {

class SimpleVertexAllocator : public GrEagerVertexAllocator {
public:
    void* lock(size_t stride, int eagerCount) override {
        SkASSERT(!fPoints);
        SkASSERT(stride == sizeof(SkPoint));
        fPoints.reset(eagerCount);
        return fPoints;
    }
    void unlock(int actualCount) override {}
    SkPoint operator[](int idx) const { return fPoints[idx]; }
    SkAutoTMalloc<SkPoint> fPoints;
};

class SimplerVertexAllocator : public GrEagerVertexAllocator {
public:
    void* lock(size_t stride, int eagerCount) override {
        size_t allocSize = eagerCount * stride;
        if (allocSize > fVertexAllocSize) {
            fVertexData.reset(allocSize);
        }
        return fVertexData;
    }

    void unlock(int) override {}

    SkAutoTMalloc<char> fVertexData;
    size_t fVertexAllocSize = 0;
};

}  // namespace

struct Edge {
    Edge reverse() const { return {fP1, fP0}; }
    SkPoint fP0, fP1;
};

static bool operator<(const Edge& a, const Edge& b) {
    if (a.fP0.fX != b.fP0.fX) {
        return a.fP0.fX < b.fP0.fX;
    }
    if (a.fP0.fY != b.fP0.fY) {
        return a.fP0.fY < b.fP0.fY;
    }
    if (a.fP1.fX != b.fP1.fX) {
        return a.fP1.fX < b.fP1.fX;
    }
    if (a.fP1.fY != b.fP1.fY) {
        return a.fP1.fY < b.fP1.fY;
    }
    return false;
}

using EdgeMap = std::map<Edge, int>;

static void add_edge(EdgeMap& edgeMap, SkPoint p0, SkPoint p1) {
    Edge edge{p0, p1};
    // First check if this edge already exists in reverse.
    auto reverseIter = edgeMap.find(edge.reverse());
    if (reverseIter != edgeMap.end()) {
        --reverseIter->second;
    } else {
        ++edgeMap[edge];
    }
}

static void add_tri_edges(skiatest::Reporter* r, EdgeMap& edgeMap, const SkPoint pts[3]) {
    for (int i = 0; i < 3; ++i) {
        SkPoint p0=pts[i], p1=pts[(i+1)%3];
        // The triangulator shouldn't output degenerate triangles.
        REPORTER_ASSERT(r, p0 != p1);
        add_edge(edgeMap, p0, p1);
    }
}

static EdgeMap simplify(const EdgeMap& edges, SkPathFillType fillType) {
    // Prune out the edges whose count went to zero, and reverse the edges whose count is negative.
    EdgeMap simplifiedEdges;
    for (auto [edge, count] : edges) {
        // We should only have one ordering of any given edge.
        SkASSERT(edges.find(edge.reverse()) == edges.end());
        if (fillType == SkPathFillType::kEvenOdd) {
            count = abs(count) & 1;
        }
        if (count > 0) {
            simplifiedEdges[edge] = count;
        } else if (count < 0) {
            simplifiedEdges[edge.reverse()] = -count;
        }
    }
    return simplifiedEdges;
}

static void verify_simple_inner_polygons(skiatest::Reporter* r, const char* shapeName,
                                         SkPath path) {
    for (auto fillType : {SkPathFillType::kWinding}) {
        path.setFillType(fillType);
        SkArenaAlloc arena(GrTriangulator::kArenaDefaultChunkSize);
        GrInnerFanTriangulator::BreadcrumbTriangleList breadcrumbs;
        SimpleVertexAllocator vertexAlloc;
        int vertexCount;
        {
            bool isLinear;
            GrInnerFanTriangulator triangulator(path, &arena);
            vertexCount = triangulator.pathToTriangles(&vertexAlloc, &breadcrumbs, &isLinear);
        }

        // Count up all the triangulated edges.
        EdgeMap trianglePlusBreadcrumbEdges;
        for (int i = 0; i < vertexCount; i += 3) {
            add_tri_edges(r, trianglePlusBreadcrumbEdges, vertexAlloc.fPoints.data() + i);
        }
        // Count up all the breadcrumb edges.
        int breadcrumbCount = 0;
        for (const auto* node = breadcrumbs.head(); node; node = node->fNext) {
            add_tri_edges(r, trianglePlusBreadcrumbEdges, node->fPts);
            ++breadcrumbCount;
        }
        REPORTER_ASSERT(r, breadcrumbCount == breadcrumbs.count());
        // The triangulated + breadcrumb edges should cancel out to the inner polygon edges.
        trianglePlusBreadcrumbEdges = simplify(trianglePlusBreadcrumbEdges, path.getFillType());

        // Build the inner polygon edges.
        EdgeMap innerFanEdges;
        SkPoint startPoint{}, lastPoint{};
        for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
            switch (verb) {
                case SkPathVerb::kMove:
                    if (lastPoint != startPoint) {
                        add_edge(innerFanEdges, lastPoint, startPoint);
                    }
                    lastPoint = startPoint = pts[0];
                    continue;
                case SkPathVerb::kClose:
                    lastPoint = startPoint;
                    break;
                case SkPathVerb::kLine:
                    lastPoint = pts[1];
                    break;
                case SkPathVerb::kQuad:
                case SkPathVerb::kConic:
                    lastPoint = pts[2];
                    break;
                case SkPathVerb::kCubic:
                    lastPoint = pts[3];
                    break;
            }
            if (pts[0] != lastPoint) {
                add_edge(innerFanEdges, pts[0], lastPoint);
            }
        }
        if (lastPoint != startPoint) {
            add_edge(innerFanEdges, lastPoint, startPoint);
        }
        innerFanEdges = simplify(innerFanEdges, path.getFillType());

        // The triangulated + breadcrumb edges should cancel out to the inner polygon edges. First
        // verify that every inner polygon edge can be found in the triangulation.
        for (auto [edge, count] : innerFanEdges) {
            auto it = trianglePlusBreadcrumbEdges.find(edge);
            if (it != trianglePlusBreadcrumbEdges.end()) {
                it->second -= count;
                if (it->second == 0) {
                    trianglePlusBreadcrumbEdges.erase(it);
                }
                continue;
            }
            it = trianglePlusBreadcrumbEdges.find(edge.reverse());
            if (it != trianglePlusBreadcrumbEdges.end()) {
                it->second += count;
                if (it->second == 0) {
                    trianglePlusBreadcrumbEdges.erase(it);
                }
                continue;
            }
            ERRORF(r, "error: %s: edge [%g,%g]:[%g,%g] not found in triangulation.",
                   shapeName, edge.fP0.fX, edge.fP0.fY, edge.fP1.fX, edge.fP1.fY);
            return;
        }
        // Now verify that there are no spurious edges in the triangulation.
        //
        // NOTE: The triangulator's definition of wind isn't always correct for edges that run
        // exactly parallel to the sweep (either vertical or horizontal edges). This doesn't
        // actually matter though because T-junction artifacts don't happen on axis-aligned edges.
        // Tolerate spurious edges that (1) come in pairs of 2, and (2) are either exactly
        // horizontal or exactly vertical exclusively.
        bool hasSpuriousHorz=false, hasSpuriousVert=false;
        for (auto [edge, count] : trianglePlusBreadcrumbEdges) {
            if (count % 2 == 0) {
                if (edge.fP0.fX == edge.fP1.fX && !hasSpuriousVert) {
                    hasSpuriousHorz = true;
                    continue;
                }
                if (edge.fP0.fY == edge.fP1.fY && !hasSpuriousHorz) {
                    hasSpuriousVert = true;
                    continue;
                }
            }
            ERRORF(r, "error: %s: spurious edge [%g,%g]:[%g,%g] found in triangulation.",
                   shapeName, edge.fP0.fX, edge.fP0.fY, edge.fP1.fX, edge.fP1.fY);
            return;
        }
    }
}

DEF_TEST(GrInnerFanTriangulator, r) {
    verify_simple_inner_polygons(r, "simple triangle", SkPath().lineTo(1,0).lineTo(0,1));
    verify_simple_inner_polygons(r, "simple square", SkPath().lineTo(1,0).lineTo(1,1).lineTo(0,1));
    verify_simple_inner_polygons(r,  "concave polygon", SkPath()
            .lineTo(1,0).lineTo(.5f,.5f).lineTo(1,1).lineTo(0,1));
    verify_simple_inner_polygons(r, "double wound triangle", SkPath()
            .lineTo(1,0).lineTo(0,1).lineTo(0,0).lineTo(1,0).lineTo(0,1));
    verify_simple_inner_polygons(r, "self-intersecting bowtie", SkPath()
            .lineTo(1,0).lineTo(0,1).lineTo(1,1));
    verify_simple_inner_polygons(r, "asymmetrical bowtie", SkPath()
            .lineTo(1,0).lineTo(0,1).lineTo(.1f,-.1f));
    verify_simple_inner_polygons(r, "bowtie with extremely small section", SkPath()
            .lineTo(1,0).lineTo(0,1).lineTo(1e-6f,-1e-6f));
    verify_simple_inner_polygons(r, "intersecting squares", SkPath()
            .lineTo(1,0).lineTo(1,1).lineTo(0,1)
            .moveTo(.5f,.5f).lineTo(1.5f,.5f).lineTo(1.5f,1.5f).lineTo(.5f,1.5f).close());
    verify_simple_inner_polygons(r, "6-point \"Star of David\"", SkPath()
            .moveTo(cosf(-SK_ScalarPI/3), sinf(-SK_ScalarPI/3))
            .lineTo(cosf(SK_ScalarPI/3), sinf(SK_ScalarPI/3))
            .lineTo(cosf(SK_ScalarPI), sinf(SK_ScalarPI))
            .moveTo(cosf(0), sinf(0))
            .lineTo(cosf(2*SK_ScalarPI/3), sinf(2*SK_ScalarPI/3))
            .lineTo(cosf(-2*SK_ScalarPI/3), sinf(-2*SK_ScalarPI/3)));
    verify_simple_inner_polygons(r, "double wound \"Star of David\"", SkPath()
            .moveTo(cosf(-SK_ScalarPI/3), sinf(-SK_ScalarPI/3))
            .lineTo(cosf(SK_ScalarPI/3), sinf(SK_ScalarPI/3))
            .lineTo(cosf(SK_ScalarPI), sinf(SK_ScalarPI))
            .lineTo(cosf(-SK_ScalarPI/3), sinf(-SK_ScalarPI/3))
            .lineTo(cosf(SK_ScalarPI/3), sinf(SK_ScalarPI/3))
            .lineTo(cosf(SK_ScalarPI), sinf(SK_ScalarPI))
            .moveTo(cosf(0), sinf(0))
            .lineTo(cosf(2*SK_ScalarPI/3), sinf(2*SK_ScalarPI/3))
            .lineTo(cosf(-2*SK_ScalarPI/3), sinf(-2*SK_ScalarPI/3)));
    verify_simple_inner_polygons(r, "5-point star", ToolUtils::make_star(SkRect::MakeWH(100, 200)));
    verify_simple_inner_polygons(r, "\"pointy\" intersecting triangles", SkPath()
            .moveTo(0,-100).lineTo(-1e-6f,100).lineTo(1e-6f,100)
            .moveTo(-100,0).lineTo(100,1e-6f).lineTo(100,-1e-6f));
    verify_simple_inner_polygons(r, "overlapping rects with vertical collinear edges", SkPath()
            .moveTo(0,0).lineTo(0,2).lineTo(1,2).lineTo(1,0)
            .moveTo(0,1).lineTo(0,3).lineTo(1,3).lineTo(1,1));
    verify_simple_inner_polygons(r, "overlapping rects with horizontal collinear edges", SkPath()
            .lineTo(2,0).lineTo(2,1).lineTo(0,1)
            .moveTo(1,0).lineTo(3,0).lineTo(3,1).lineTo(1,1).close());
    for (int i = 0; i < (int)SK_ARRAY_COUNT(kNonEdgeAAPaths); ++i) {
        verify_simple_inner_polygons(r, SkStringPrintf("kNonEdgeAAPaths[%i]", i).c_str(),
                                     kNonEdgeAAPaths[i]());
    }
    SkRandom rand;
    for (int i = 0; i < 50; ++i) {
        auto randomPath = SkPath().moveTo(rand.nextF(), rand.nextF());
        for (int j = 0; j < i; ++j) {
            randomPath.lineTo(rand.nextF(), rand.nextF());
        }
        verify_simple_inner_polygons(r, SkStringPrintf("random_path_%i", i).c_str(), randomPath);
    }
}

static void test_crbug_1262444(skiatest::Reporter* r) {
    SkPath path;

    path.setFillType(SkPathFillType::kWinding);
    path.moveTo(SkBits2Float(0x3fe0633f), SkBits2Float(0x3d04a60d));  // 1.75303f, 0.0323849f
    path.cubicTo(SkBits2Float(0x3fe27540), SkBits2Float(0x3dff593f), SkBits2Float(0x3fe45241),
                 SkBits2Float(0x3e5e2fbb), SkBits2Float(0x3fe55b41), SkBits2Float(
                    0x3e9e596d));  // 1.7692f, 0.124682f, 1.78376f, 0.216979f, 1.79185f, 0.309276f
    path.cubicTo(SkBits2Float(0x3fe5fa41), SkBits2Float(0x3eb3e79c), SkBits2Float(0x3fe62f41),
                 SkBits2Float(0x3ec975cb), SkBits2Float(0x3fe69941), SkBits2Float(
                    0x3edfd837));  // 1.7967f, 0.351376f, 1.79832f, 0.393477f, 1.80155f, 0.437196f
    path.cubicTo(SkBits2Float(0x3fe70341), SkBits2Float(0x3f064e87), SkBits2Float(0x3fe6ce41),
                 SkBits2Float(0x3f1cb0f2), SkBits2Float(0x3fe59041), SkBits2Float(
                    0x3f33135e));  // 1.80479f, 0.524636f, 1.80317f, 0.612075f, 1.79346f, 0.699514f
    path.cubicTo(SkBits2Float(0x3fe48740), SkBits2Float(0x3f468ef5), SkBits2Float(0x3fe2df40),
                 SkBits2Float(0x3f59a06d), SkBits2Float(0x3fe02e3f), SkBits2Float(
                    0x3f6cb1e6));  // 1.78538f, 0.775619f, 1.77244f, 0.850104f, 1.75141f, 0.92459f
    path.cubicTo(SkBits2Float(0x3fde863f), SkBits2Float(0x3f78b759), SkBits2Float(0x3fdc743e),
                 SkBits2Float(0x3f822957), SkBits2Float(0x3fd9c33e), SkBits2Float(
                    0x3f87f701));  // 1.73847f, 0.971548f, 1.7223f, 1.01689f, 1.70127f, 1.06223f
    path.cubicTo(SkBits2Float(0x3fd98e3e), SkBits2Float(0x3f88611f), SkBits2Float(0x3fd9593e),
                 SkBits2Float(0x3f88cb3e), SkBits2Float(0x3fd9243d), SkBits2Float(
                    0x3f896a6b));  // 1.69965f, 1.06546f, 1.69804f, 1.0687f, 1.69642f, 1.07356f
    path.cubicTo(SkBits2Float(0x3fd63e3c), SkBits2Float(0x3f8fa234), SkBits2Float(0x3fd2ee3b),
                 SkBits2Float(0x3f95d9fd), SkBits2Float(0x3fd2ee3b), SkBits2Float(
                    0x3f9ce602));  // 1.67377f, 1.12214f, 1.6479f, 1.17071f, 1.6479f, 1.22577f
    path.cubicTo(SkBits2Float(0x3fd3233b), SkBits2Float(0x3f9cb0f3), SkBits2Float(0x3fd3583b),
                 SkBits2Float(0x3f9cb0f3), SkBits2Float(0x3fd3c23c), SkBits2Float(
                    0x3f9c7be4));  // 1.64951f, 1.22415f, 1.65113f, 1.22415f, 1.65437f, 1.22253f
    path.cubicTo(SkBits2Float(0x3fd3c23c), SkBits2Float(0x3f9cb0f3), SkBits2Float(0x3fd3c23c),
                 SkBits2Float(0x3f9cb0f3), SkBits2Float(0x3fd3c23c), SkBits2Float(
                    0x3f9ce602));  // 1.65437f, 1.22415f, 1.65437f, 1.22415f, 1.65437f, 1.22577f
    path.cubicTo(SkBits2Float(0x3fd5353c), SkBits2Float(0x3f9c46d4), SkBits2Float(0x3fd6dd3d),
                 SkBits2Float(0x3f9bdcb6), SkBits2Float(0x3fd7b13d), SkBits2Float(
                    0x3f9ad36a));  // 1.66569f, 1.22091f, 1.67863f, 1.21767f, 1.6851f, 1.20958f
    path.cubicTo(SkBits2Float(0x3fda623e), SkBits2Float(0x3f96ae3a), SkBits2Float(0x3fdca93f),
                 SkBits2Float(0x3f921eeb), SkBits2Float(0x3fdf253f), SkBits2Float(
                    0x3f8dc4ab));  // 1.70612f, 1.17719f, 1.72391f, 1.14157f, 1.74332f, 1.10756f
    path.cubicTo(SkBits2Float(0x3fe0983f), SkBits2Float(0x3f8b12e5), SkBits2Float(0x3fe1d640),
                 SkBits2Float(0x3f87f700), SkBits2Float(0x3fe3b340), SkBits2Float(
                    0x3f857a4a));  // 1.75465f, 1.08651f, 1.76435f, 1.06223f, 1.77891f, 1.04279f
    path.cubicTo(SkBits2Float(0x3fe48740), SkBits2Float(0x3f8470fe), SkBits2Float(0x3fe62f40),
                 SkBits2Float(0x3f8470fe), SkBits2Float(0x3fe7d741), SkBits2Float(
                    0x3f843bef));  // 1.78538f, 1.0347f, 1.79832f, 1.0347f, 1.81126f, 1.03308f
    path.cubicTo(SkBits2Float(0x3fe2aa40), SkBits2Float(0x3f943182), SkBits2Float(0x3fda623d),
                 SkBits2Float(0x3fa2498e), SkBits2Float(0x3fceff3a), SkBits2Float(
                    0x3fae4f01));  // 1.77082f, 1.15776f, 1.70612f, 1.26787f, 1.61716f, 1.36179f
    path.cubicTo(SkBits2Float(0x3fce6039), SkBits2Float(0x3faf233e), SkBits2Float(0x3fcd2239),
                 SkBits2Float(0x3faf584d), SkBits2Float(0x3fcc1939), SkBits2Float(
                    0x3fafc26b));  // 1.61231f, 1.36826f, 1.60261f, 1.36988f, 1.59452f, 1.37312f
    path.cubicTo(SkBits2Float(0x3fcc1939), SkBits2Float(0x3faff77a), SkBits2Float(0x3fcc1939),
                 SkBits2Float(0x3faff77a), SkBits2Float(0x3fcc4e39), SkBits2Float(
                    0x3fb02c89));  // 1.59452f, 1.37474f, 1.59452f, 1.37474f, 1.59614f, 1.37636f
    path.cubicTo(SkBits2Float(0x3fcc1939), SkBits2Float(0x3fb02c89), SkBits2Float(0x3fcc1939),
                 SkBits2Float(0x3fb02c89), SkBits2Float(0x3fcbe439), SkBits2Float(
                    0x3fb02c89));  // 1.59452f, 1.37636f, 1.59452f, 1.37636f, 1.5929f, 1.37636f
    path.cubicTo(SkBits2Float(0x3fcbe439), SkBits2Float(0x3fb20a12), SkBits2Float(0x3fcb4539),
                 SkBits2Float(0x3fb37d7d), SkBits2Float(0x3fc99d39), SkBits2Float(
                    0x3fb3b28c));  // 1.5929f, 1.39093f, 1.58805f, 1.40227f, 1.57511f, 1.40389f
    path.cubicTo(SkBits2Float(0x3fc93339), SkBits2Float(0x3fb3e79b), SkBits2Float(0x3fc8c938),
                 SkBits2Float(0x3fb41caa), SkBits2Float(0x3fc7f538), SkBits2Float(
                    0x3fb41caa));  // 1.57188f, 1.40551f, 1.56864f, 1.40712f, 1.56217f, 1.40712f
    path.cubicTo(SkBits2Float(0x3fc7f538), SkBits2Float(0x3fb3e79b), SkBits2Float(0x3fc7f538),
                 SkBits2Float(0x3fb3e79b), SkBits2Float(0x3fc7f538), SkBits2Float(
                    0x3fb3b28c));  // 1.56217f, 1.40551f, 1.56217f, 1.40551f, 1.56217f, 1.40389f
    path.lineTo(SkBits2Float(0x3fc7c038), SkBits2Float(0x3fb3b28c));  // 1.56055f, 1.40389f
    path.cubicTo(SkBits2Float(0x3fc7c038), SkBits2Float(0x3fb4f0e7), SkBits2Float(0x3fc7f538),
                 SkBits2Float(0x3fb66452), SkBits2Float(0x3fc78b38), SkBits2Float(
                    0x3fb76d9e));  // 1.56055f, 1.4136f, 1.56217f, 1.42494f, 1.55894f, 1.43303f
    path.cubicTo(SkBits2Float(0x3fc3d137), SkBits2Float(0x3fbe4495), SkBits2Float(0x3fbf4336),
                 SkBits2Float(0x3fc4123e), SkBits2Float(0x3fb80434), SkBits2Float(
                    0x3fc76331));  // 1.52982f, 1.48647f, 1.49424f, 1.53181f, 1.43763f, 1.55771f
    path.cubicTo(SkBits2Float(0x3fb47f33), SkBits2Float(0x3fc90bac), SkBits2Float(0x3fb19932),
                 SkBits2Float(0x3fcb5353), SkBits2Float(0x3faf1d31), SkBits2Float(
                    0x3fce6f37));  // 1.41013f, 1.57067f, 1.38749f, 1.58848f, 1.36808f, 1.61277f
    path.cubicTo(SkBits2Float(0x3fa4592e), SkBits2Float(0x3fdb13d7), SkBits2Float(0x3f974e2a),
                 SkBits2Float(0x3fe53bc1), SkBits2Float(0x3f896f25), SkBits2Float(
                    0x3fee5a5f));  // 1.28397f, 1.71154f, 1.18207f, 1.79089f, 1.0737f, 1.86213f
    path.cubicTo(SkBits2Float(0x3f6b883f), SkBits2Float(0x3ffb691f), SkBits2Float(0x3f42f434),
                 SkBits2Float(0x400367b2), SkBits2Float(0x3f184e28), SkBits2Float(
                    0x4008611f));  // 0.920048f, 1.96415f, 0.761539f, 2.0532f, 0.594943f, 2.13093f
    path.cubicTo(SkBits2Float(0x3f184e28), SkBits2Float(0x4008611f), SkBits2Float(0x3f17e428),
                 SkBits2Float(0x4008611f), SkBits2Float(0x3f17e428), SkBits2Float(
                    0x40087ba7));  // 0.594943f, 2.13093f, 0.593325f, 2.13093f, 0.593325f, 2.13255f
    path.cubicTo(SkBits2Float(0x3effc044), SkBits2Float(0x400b47f5), SkBits2Float(0x3ed08c36),
                 SkBits2Float(0x400e2eca), SkBits2Float(0x3e9edc28), SkBits2Float(
                    0x401090f9));  // 0.499514f, 2.17627f, 0.40732f, 2.22161f, 0.310273f, 2.25885f
    path.cubicTo(SkBits2Float(0x3e5a5832), SkBits2Float(0x4012f328), SkBits2Float(0x3de40030),
                 SkBits2Float(0x4014811a), SkBits2Float(0x3c1a7f9e), SkBits2Float(
                    0x40158a66));  // 0.213227f, 2.29609f, 0.111328f, 2.32038f, 0.00942984f, 2.33657f
    path.lineTo(SkBits2Float(0x3c1a7f9e), SkBits2Float(0x401bf73d));  // 0.00942984f, 2.43697f
    path.cubicTo(SkBits2Float(0x3dc98028), SkBits2Float(0x401b580f), SkBits2Float(0x3e3fd82e),
                 SkBits2Float(0x401a694b), SkBits2Float(0x3e8ca424), SkBits2Float(
                    0x40191068));  // 0.098389f, 2.42725f, 0.187348f, 2.41268f, 0.27469f, 2.39163f
    path.cubicTo(SkBits2Float(0x3e94ec27), SkBits2Float(0x4018db59), SkBits2Float(0x3e9d3429),
                 SkBits2Float(0x40188bc2), SkBits2Float(0x3ea4a82b), SkBits2Float(
                    0x401856b3));  // 0.290864f, 2.38839f, 0.307039f, 2.38353f, 0.321596f, 2.38029f
    path.cubicTo(SkBits2Float(0x3eae982e), SkBits2Float(0x4018071c), SkBits2Float(0x3eb95c31),
                 SkBits2Float(0x40179cfe), SkBits2Float(0x3ec34c34), SkBits2Float(
                    0x40174d67));  // 0.341005f, 2.37543f, 0.362031f, 2.36896f, 0.381441f, 2.3641f
    path.cubicTo(SkBits2Float(0x3ec9ec36), SkBits2Float(0x40171858), SkBits2Float(0x3ed08c38),
                 SkBits2Float(0x4016c8c1), SkBits2Float(0x3ed8003a), SkBits2Float(
                    0x401693b2));  // 0.39438f, 2.36086f, 0.40732f, 2.356f, 0.421877f, 2.35276f
    path.cubicTo(SkBits2Float(0x3eda7c3a), SkBits2Float(0x4016792a), SkBits2Float(0x3eddcc3c),
                 SkBits2Float(0x40165ea3), SkBits2Float(0x3ee0483c), SkBits2Float(
                    0x4016441b));  // 0.426729f, 2.35115f, 0.433199f, 2.34953f, 0.438051f, 2.34791f
    path.cubicTo(SkBits2Float(0x3ee2c43d), SkBits2Float(0x40162993), SkBits2Float(0x3ee5403e),
                 SkBits2Float(0x40160f0c), SkBits2Float(0x3ee8903f), SkBits2Float(
                    0x4015f484));  // 0.442903f, 2.34629f, 0.447756f, 2.34467f, 0.454226f, 2.34305f
    path.cubicTo(SkBits2Float(0x3f1c082a), SkBits2Float(0x4012be17), SkBits2Float(0x3f422036),
                 SkBits2Float(0x400e63d8), SkBits2Float(0x3f66fa40), SkBits2Float(
                    0x40096a6a));  // 0.6095f, 2.29285f, 0.758304f, 2.22484f, 0.902256f, 2.14712f
    path.cubicTo(SkBits2Float(0x3f6a4a41), SkBits2Float(0x4009004c), SkBits2Float(0x3f6d3042),
                 SkBits2Float(0x4008962d), SkBits2Float(0x3f708043), SkBits2Float(
                    0x40081187));  // 0.915196f, 2.14064f, 0.926518f, 2.13417f, 0.939457f, 2.12607f
    path.cubicTo(SkBits2Float(0x3f7efe47), SkBits2Float(0x4005feef), SkBits2Float(0x3f868925),
                 SkBits2Float(0x4003b748), SkBits2Float(0x3f8d5e28), SkBits2Float(
                    0x40015519));  // 0.996067f, 2.09368f, 1.05106f, 2.05806f, 1.10444f, 2.02082f
    path.cubicTo(SkBits2Float(0x3f97b82b), SkBits2Float(0x3ffb691d), SkBits2Float(0x3fa1a82e),
                 SkBits2Float(0x3ff388da), SkBits2Float(0x3fab9830), SkBits2Float(
                    0x3feb7389));  // 1.18531f, 1.96415f, 1.26294f, 1.90261f, 1.34058f, 1.83946f
    path.cubicTo(SkBits2Float(0x3fb20332), SkBits2Float(0x3fe6450c), SkBits2Float(0x3fb80434),
                 SkBits2Float(0x3fe0e181), SkBits2Float(0x3fbd6635), SkBits2Float(
                    0x3fda3f99));  // 1.39072f, 1.79898f, 1.43763f, 1.75688f, 1.47968f, 1.70507f
    path.cubicTo(SkBits2Float(0x3fbf4336), SkBits2Float(0x3fd7f7f2), SkBits2Float(0x3fc12037),
                 SkBits2Float(0x3fd5b04b), SkBits2Float(0x3fc2fd36), SkBits2Float(
                    0x3fd33394));  // 1.49424f, 1.68725f, 1.5088f, 1.66944f, 1.52335f, 1.65001f
    path.cubicTo(SkBits2Float(0x3fc5e337), SkBits2Float(0x3fcf7881), SkBits2Float(0x3fc8c938),
                 SkBits2Float(0x3fcbbd70), SkBits2Float(0x3fcbaf38), SkBits2Float(
                    0x3fc8025d));  // 1.546f, 1.62086f, 1.56864f, 1.59172f, 1.59128f, 1.56257f
    path.cubicTo(SkBits2Float(0x3fceff39), SkBits2Float(0x3fc3a81e), SkBits2Float(0x3fd2843b),
                 SkBits2Float(0x3fbf18cf), SkBits2Float(0x3fd5d43b), SkBits2Float(
                    0x3fbabe8f));  // 1.61716f, 1.52857f, 1.64466f, 1.49294f, 1.67054f, 1.45894f
    path.cubicTo(SkBits2Float(0x3fd8503c), SkBits2Float(0x3fb7a2ab), SkBits2Float(0x3fda973d),
                 SkBits2Float(0x3fb486c7), SkBits2Float(0x3fdca93e), SkBits2Float(
                    0x3fb135d3));  // 1.68995f, 1.43465f, 1.70774f, 1.41036f, 1.72391f, 1.38446f
    path.cubicTo(SkBits2Float(0x3fe5c541), SkBits2Float(0x3fa2b3aa), SkBits2Float(0x3feb5c42),
                 SkBits2Float(0x3f92be16), SkBits2Float(0x3ff15d44), SkBits2Float(
                    0x3f82c882));  // 1.79508f, 1.27111f, 1.83875f, 1.14643f, 1.88566f, 1.02174f
    path.cubicTo(SkBits2Float(0x3ff1fc44), SkBits2Float(0x3f812008), SkBits2Float(0x3ff23144),
                 SkBits2Float(0x3f7e1adf), SkBits2Float(0x3ff29b44), SkBits2Float(
                    0x3f7a5fcc));  // 1.89051f, 1.00879f, 1.89213f, 0.992598f, 1.89536f, 0.978024f
    path.cubicTo(SkBits2Float(0x3ff47845), SkBits2Float(0x3f5fd830), SkBits2Float(0x3ff65545),
                 SkBits2Float(0x3f455094), SkBits2Float(0x3ff6bf45), SkBits2Float(
                    0x3f2a5ed9));  // 1.90992f, 0.874393f, 1.92448f, 0.770761f, 1.92771f, 0.66551f
    path.cubicTo(SkBits2Float(0x3ff33a44), SkBits2Float(0x3f0d5a87), SkBits2Float(0x3ff08943),
                 SkBits2Float(0x3edf03ee), SkBits2Float(0x3fee7743), SkBits2Float(
                    0x3ea352cf));  // 1.90022f, 0.552163f, 1.87919f, 0.435577f, 1.86301f, 0.318991f
    path.cubicTo(SkBits2Float(0x3feccf42), SkBits2Float(0x3e5c872d), SkBits2Float(0x3feb9142),
                 SkBits2Float(0x3de4d179), SkBits2Float(0x3feaf242), SkBits2Float(
                    0x3c04a4ae));  // 1.85008f, 0.215359f, 1.84037f, 0.111728f, 1.83552f, 0.0080959f
    path.lineTo(SkBits2Float(0x3fe02e3f), SkBits2Float(0x3c04a4ae));  // 1.75141f, 0.0080959f
    path.cubicTo(SkBits2Float(0x3fdff93f), SkBits2Float(0x3c6ec47e), SkBits2Float(0x3fe02e3f),
                 SkBits2Float(0x3cb9b545), SkBits2Float(0x3fe0633f), SkBits2Float(
                    0x3d04a60d));  // 1.74979f, 0.0145732f, 1.75141f, 0.0226694f, 1.75303f, 0.0323849f
    path.close();
    path.moveTo(SkBits2Float(0x3fe97f42), SkBits2Float(0x3f7b9e2e));  // 1.8242f, 0.982882f
    path.cubicTo(SkBits2Float(0x3fe91542), SkBits2Float(0x3f7eef21), SkBits2Float(0x3fe87642),
                 SkBits2Float(0x3f81551a), SkBits2Float(0x3fe7d741), SkBits2Float(
                    0x3f82fd94));  // 1.82096f, 0.995836f, 1.81611f, 1.01041f, 1.81126f, 1.02336f
    path.cubicTo(SkBits2Float(0x3fe6ce41), SkBits2Float(0x3f81bf39), SkBits2Float(0x3fe66441),
                 SkBits2Float(0x3f8080dd), SkBits2Float(0x3fe66441), SkBits2Float(
                    0x3f7e1ae4));  // 1.80317f, 1.01365f, 1.79993f, 1.00393f, 1.79993f, 0.992598f
    path.cubicTo(SkBits2Float(0x3fe66441), SkBits2Float(0x3f7c726a), SkBits2Float(0x3fe69941),
                 SkBits2Float(0x3f7b340e), SkBits2Float(0x3fe6ce41), SkBits2Float(
                    0x3f798b95));  // 1.79993f, 0.986121f, 1.80155f, 0.981263f, 1.80317f, 0.974786f
    path.cubicTo(SkBits2Float(0x3fe70341), SkBits2Float(0x3f78b758), SkBits2Float(0x3fe76d41),
                 SkBits2Float(0x3f770edf), SkBits2Float(0x3fe7d741), SkBits2Float(
                    0x3f770edf));  // 1.80479f, 0.971548f, 1.80802f, 0.965071f, 1.81126f, 0.965071f
    path.cubicTo(SkBits2Float(0x3fe84141), SkBits2Float(0x3f770edf), SkBits2Float(0x3fe8ab42),
                 SkBits2Float(0x3f770edf), SkBits2Float(0x3fe8e041), SkBits2Float(
                    0x3f7778fd));  // 1.81449f, 0.965071f, 1.81773f, 0.965071f, 1.81934f, 0.96669f
    path.cubicTo(SkBits2Float(0x3fe97f42), SkBits2Float(0x3f77e31b), SkBits2Float(0x3fe9e942),
                 SkBits2Float(0x3f798b95), SkBits2Float(0x3fe97f42), SkBits2Float(
                    0x3f7b9e2e));  // 1.8242f, 0.968309f, 1.82743f, 0.974786f, 1.8242f, 0.982882f
    path.close();

    float kTol = 0.25f;
    SkRect clipBounds = SkRect::MakeLTRB(0, 0, 14, 14);
    SimplerVertexAllocator alloc;

    int vertexCount = GrAATriangulator::PathToAATriangles(path, kTol, clipBounds, &alloc);
    REPORTER_ASSERT(r, vertexCount == 0);
}

DEF_TEST(TriangulatorBugs, r) {
    test_crbug_1262444(r);
}
