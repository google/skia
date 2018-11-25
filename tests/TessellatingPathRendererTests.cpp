/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkPath.h"

#if SK_SUPPORT_GPU
#include "GrClip.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "SkGradientShader.h"
#include "SkShaderBase.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "ops/GrTessellatingPathRenderer.h"

/*
 * These tests pass by not crashing, hanging or asserting in Debug.
 */

// Tests active edges made inactive by splitting.
// Also tests active edge list forced into an invalid ordering by
// splitting (mopped up in cleanup_active_edges()).
static SkPath create_path_0() {
    SkPath path;
    path.moveTo(229.127044677734375f,  67.34100341796875f);
    path.lineTo(187.8097381591796875f, -6.7729740142822265625f);
    path.lineTo(171.411407470703125f,  50.94266510009765625f);
    path.lineTo(245.5253753662109375f,  9.6253643035888671875f);
    path.moveTo(208.4683990478515625f, 30.284009933471679688f);
    path.lineTo(171.411407470703125f,  50.94266510009765625f);
    path.lineTo(187.8097381591796875f, -6.7729740142822265625f);
    return path;
}

// Intersections which fall exactly on the current vertex, and require
// a restart of the intersection checking.
static SkPath create_path_1() {
    SkPath path;
    path.moveTo(314.483551025390625f, 486.246002197265625f);
    path.lineTo(385.41949462890625f,  532.8087158203125f);
    path.lineTo(373.232879638671875f, 474.05938720703125f);
    path.lineTo(326.670166015625f,    544.995361328125f);
    path.moveTo(349.951507568359375f, 509.52734375f);
    path.lineTo(373.232879638671875f, 474.05938720703125f);
    path.lineTo(385.41949462890625f,  532.8087158203125f);
    return path;
}

// Tests active edges which are removed by splitting.
static SkPath create_path_2() {
    SkPath path;
    path.moveTo(343.107391357421875f, 613.62176513671875f);
    path.lineTo(426.632415771484375f, 628.5740966796875f);
    path.lineTo(392.3460693359375f,   579.33544921875f);
    path.lineTo(377.39373779296875f,  662.86041259765625f);
    path.moveTo(384.869873046875f,    621.097900390625f);
    path.lineTo(392.3460693359375f,   579.33544921875f);
    path.lineTo(426.632415771484375f, 628.5740966796875f);
    return path;
}

// Collinear edges merged in set_top().
// Also, an intersection between left and right enclosing edges which
// falls above the current vertex.
static SkPath create_path_3() {
    SkPath path;
    path.moveTo(545.95751953125f,    791.69854736328125f);
    path.lineTo(612.05816650390625f, 738.494140625f);
    path.lineTo(552.4056396484375f,  732.0460205078125f);
    path.lineTo(605.61004638671875f, 798.14666748046875f);
    path.moveTo(579.00787353515625f, 765.0963134765625f);
    path.lineTo(552.4056396484375f,  732.0460205078125f);
    path.lineTo(612.05816650390625f, 738.494140625f);
    return path;
}

// Tests active edges which are made inactive by set_top().
static SkPath create_path_4() {
    SkPath path;
    path.moveTo(819.2725830078125f,  751.77447509765625f);
    path.lineTo(820.70904541015625f, 666.933837890625f);
    path.lineTo(777.57049560546875f, 708.63592529296875f);
    path.lineTo(862.4111328125f,     710.0723876953125f);
    path.moveTo(819.99078369140625f, 709.3541259765625f);
    path.lineTo(777.57049560546875f, 708.63592529296875f);
    path.lineTo(820.70904541015625f, 666.933837890625f);
    return path;
}

static SkPath create_path_5() {
    SkPath path;
    path.moveTo(823.33209228515625f, 749.052734375f);
    path.lineTo(823.494873046875f,   664.20013427734375f);
    path.lineTo(780.9871826171875f,  706.5450439453125f);
    path.lineTo(865.8397216796875f,  706.70782470703125f);
    path.moveTo(823.4134521484375f,  706.6263427734375f);
    path.lineTo(780.9871826171875f,  706.5450439453125f);
    path.lineTo(823.494873046875f,   664.20013427734375f);
    return path;
}

static SkPath create_path_6() {
    SkPath path;
    path.moveTo(954.862548828125f,   562.8349609375f);
    path.lineTo(899.32818603515625f, 498.679443359375f);
    path.lineTo(895.017578125f,      558.52435302734375f);
    path.lineTo(959.17315673828125f, 502.990081787109375f);
    path.moveTo(927.0953369140625f,  530.7572021484375f);
    path.lineTo(895.017578125f,      558.52435302734375f);
    path.lineTo(899.32818603515625f, 498.679443359375f);
    return path;
}

static SkPath create_path_7() {
    SkPath path;
    path.moveTo(958.5330810546875f,  547.35516357421875f);
    path.lineTo(899.93109130859375f, 485.989013671875f);
    path.lineTo(898.54901123046875f, 545.97308349609375f);
    path.lineTo(959.9151611328125f,  487.37109375f);
    path.moveTo(929.2320556640625f,  516.67205810546875f);
    path.lineTo(898.54901123046875f, 545.97308349609375f);
    path.lineTo(899.93109130859375f, 485.989013671875f);
    return path;
}

static SkPath create_path_8() {
    SkPath path;
    path.moveTo(389.8609619140625f,   369.326873779296875f);
    path.lineTo(470.6290283203125f,   395.33697509765625f);
    path.lineTo(443.250030517578125f, 341.9478759765625f);
    path.lineTo(417.239959716796875f, 422.7159423828125f);
    path.moveTo(430.244964599609375f, 382.3319091796875f);
    path.lineTo(443.250030517578125f, 341.9478759765625f);
    path.lineTo(470.6290283203125f,   395.33697509765625f);
    return path;
}

static SkPath create_path_9() {
    SkPath path;
    path.moveTo(20, 20);
    path.lineTo(50, 80);
    path.lineTo(20, 80);
    path.moveTo(80, 50);
    path.lineTo(50, 50);
    path.lineTo(20, 50);
    return path;
}

static SkPath create_path_10() {
    SkPath path;
    path.moveTo(257.19439697265625f, 320.876617431640625f);
    path.lineTo(190.113037109375f,   320.58978271484375f);
    path.lineTo(203.64404296875f,    293.8145751953125f);
    path.moveTo(203.357177734375f,   360.896026611328125f);
    path.lineTo(216.88824462890625f, 334.120819091796875f);
    path.lineTo(230.41925048828125f, 307.345611572265625f);
    return path;
}

// A degenerate segments case, where both upper and lower segments of
// a split edge must remain active.
static SkPath create_path_11() {
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
}

// Handle the case where edge.dist(edge.fTop) != 0.0.
static SkPath create_path_12() {
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
}

// A degenerate segments case which exercises inactive edges being
// made active by splitting.
static SkPath create_path_13() {
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
}

// Tests vertices which become "orphaned" (ie., no connected edges)
// after simplification.
static SkPath create_path_14() {
    SkPath path;
    path.moveTo(217.326019287109375f, 166.4752960205078125f);
    path.lineTo(226.279266357421875f, 170.929473876953125f);
    path.lineTo(234.3973388671875f,   177.0623626708984375f);
    path.lineTo(262.0921630859375f,   188.746124267578125f);
    path.moveTo(196.23638916015625f,  174.0722198486328125f);
    path.lineTo(416.15277099609375f,  180.138214111328125f);
    path.lineTo(192.651947021484375f, 304.0228271484375f);
    return path;
}

static SkPath create_path_15() {
    SkPath path;
    path.moveTo(    0.0f,   0.0f);
    path.lineTo(10000.0f,   0.0f);
    path.lineTo(    0.0f,  -1.0f);
    path.lineTo(10000.0f,   0.000001f);
    path.lineTo(    0.0f, -30.0f);
    return path;
}

// Reduction of Nebraska-StateSeal.svg. Floating point error causes the
// same edge to be added to more than one poly on the same side.
static SkPath create_path_16() {
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
}

// A simple concave path. Test this with a non-invertible matrix.
static SkPath create_path_17() {
    SkPath path;
    path.moveTo(20, 20);
    path.lineTo(80, 20);
    path.lineTo(30, 30);
    path.lineTo(20, 80);
    return path;
}

// A shape with a vertex collinear to the right hand edge.
// This messes up find_enclosing_edges.
static SkPath create_path_18() {
    SkPath path;
    path.moveTo(80, 20);
    path.lineTo(80, 60);
    path.lineTo(20, 60);
    path.moveTo(80, 50);
    path.lineTo(80, 80);
    path.lineTo(20, 80);
    return path;
}

// Exercises the case where an edge becomes collinear with *two* of its
// adjacent neighbour edges after splitting.
// This is a reduction from
// http://mooooo.ooo/chebyshev-sine-approximation/horner_ulp.svg
static SkPath create_path_19() {
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

// A quad which becomes NaN when interpolated.
static SkPath create_path_22() {
    SkPath path;
    path.moveTo(-5.71889e+13f, 1.36759e+09f);
    path.quadTo(2.45472e+19f, -3.12406e+15f, -2.19589e+18f, 2.79462e+14f);
    return path;
}

// A path which contains out-of-range colinear intersections.
static SkPath create_path_23() {
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
}

// A path which results in infs and nans when conics are converted to quads.
static SkPath create_path_24() {
     SkPath path;
     path.moveTo(-2.20883e+37f, -1.02892e+37f);
     path.conicTo(-2.00958e+38f, -9.36107e+37f, -1.7887e+38f, -8.33215e+37f, 0.707107f);
     path.conicTo(-1.56782e+38f, -7.30323e+37f, 2.20883e+37f, 1.02892e+37f, 0.707107f);
     path.conicTo(2.00958e+38f, 9.36107e+37f, 1.7887e+38f, 8.33215e+37f, 0.707107f);
     path.conicTo(1.56782e+38f, 7.30323e+37f, -2.20883e+37f, -1.02892e+37f, 0.707107f);
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

// A quad which generates a huge number of points (>2B) when uniformly
// linearized. This should not hang or OOM.
static SkPath create_path_29() {
    SkPath path;
    path.moveTo(10, 0);
    path.lineTo(0, 0);
    path.quadTo(10, 0, 0, 8315084722602508288);
    return path;
}

static std::unique_ptr<GrFragmentProcessor> create_linear_gradient_processor(GrContext* ctx) {

    SkPoint pts[2] = { {0, 0}, {1, 1} };
    SkColor colors[2] = { SK_ColorGREEN, SK_ColorBLUE };
    sk_sp<SkShader> shader = SkGradientShader::MakeLinear(
        pts, colors, nullptr, SK_ARRAY_COUNT(colors), SkShader::kClamp_TileMode);
    GrColorSpaceInfo colorSpaceInfo(nullptr, kRGBA_8888_GrPixelConfig);
    GrFPArgs args(ctx, &SkMatrix::I(), &SkMatrix::I(), SkFilterQuality::kLow_SkFilterQuality,
                  &colorSpaceInfo);
    return as_SB(shader)->asFragmentProcessor(args);
}

static void test_path(GrContext* ctx,
                      GrRenderTargetContext* renderTargetContext,
                      const SkPath& path,
                      const SkMatrix& matrix = SkMatrix::I(),
                      GrAAType aaType = GrAAType::kNone,
                      std::unique_ptr<GrFragmentProcessor> fp = nullptr) {
    GrTessellatingPathRenderer tess;

    GrPaint paint;
    paint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));
    if (fp) {
        paint.addColorFragmentProcessor(std::move(fp));
    }

    GrNoClip noClip;
    SkIRect clipConservativeBounds = SkIRect::MakeWH(renderTargetContext->width(),
                                                     renderTargetContext->height());
    GrStyle style(SkStrokeRec::kFill_InitStyle);
    GrShape shape(path, style);
    GrPathRenderer::DrawPathArgs args{ctx,
                                      std::move(paint),
                                      &GrUserStencilSettings::kUnused,
                                      renderTargetContext,
                                      &noClip,
                                      &clipConservativeBounds,
                                      &matrix,
                                      &shape,
                                      aaType,
                                      false};
    tess.drawPath(args);
}

DEF_GPUTEST_FOR_ALL_CONTEXTS(TessellatingPathRendererTests, reporter, ctxInfo) {
    GrContext* ctx = ctxInfo.grContext();
    sk_sp<GrRenderTargetContext> rtc(ctx->makeDeferredRenderTargetContext(
            SkBackingFit::kApprox, 800, 800, kRGBA_8888_GrPixelConfig, nullptr, 1, GrMipMapped::kNo,
            kTopLeft_GrSurfaceOrigin));
    if (!rtc) {
        return;
    }

    ctx->flush();
    // Adding discard to appease vulkan validation warning about loading uninitialized data on draw
    rtc->discard();

    test_path(ctx, rtc.get(), create_path_0());
    test_path(ctx, rtc.get(), create_path_1());
    test_path(ctx, rtc.get(), create_path_2());
    test_path(ctx, rtc.get(), create_path_3());
    test_path(ctx, rtc.get(), create_path_4());
    test_path(ctx, rtc.get(), create_path_5());
    test_path(ctx, rtc.get(), create_path_6());
    test_path(ctx, rtc.get(), create_path_7());
    test_path(ctx, rtc.get(), create_path_8());
    test_path(ctx, rtc.get(), create_path_9());
    test_path(ctx, rtc.get(), create_path_10());
    test_path(ctx, rtc.get(), create_path_11());
    test_path(ctx, rtc.get(), create_path_12());
    test_path(ctx, rtc.get(), create_path_13());
    test_path(ctx, rtc.get(), create_path_14());
    test_path(ctx, rtc.get(), create_path_15());
    test_path(ctx, rtc.get(), create_path_16());
    SkMatrix nonInvertibleMatrix = SkMatrix::MakeScale(0, 0);
    std::unique_ptr<GrFragmentProcessor> fp(create_linear_gradient_processor(ctx));
    test_path(ctx, rtc.get(), create_path_17(), nonInvertibleMatrix, GrAAType::kCoverage,
              std::move(fp));
    test_path(ctx, rtc.get(), create_path_18());
    test_path(ctx, rtc.get(), create_path_19());
    test_path(ctx, rtc.get(), create_path_20(), SkMatrix(), GrAAType::kCoverage);
    test_path(ctx, rtc.get(), create_path_21(), SkMatrix(), GrAAType::kCoverage);
    test_path(ctx, rtc.get(), create_path_22());
    test_path(ctx, rtc.get(), create_path_23());
    test_path(ctx, rtc.get(), create_path_24());
    test_path(ctx, rtc.get(), create_path_25(), SkMatrix(), GrAAType::kCoverage);
    test_path(ctx, rtc.get(), create_path_26(), SkMatrix(), GrAAType::kCoverage);
    test_path(ctx, rtc.get(), create_path_27(), SkMatrix(), GrAAType::kCoverage);
    test_path(ctx, rtc.get(), create_path_28(), SkMatrix(), GrAAType::kCoverage);
    test_path(ctx, rtc.get(), create_path_29());
}
#endif
