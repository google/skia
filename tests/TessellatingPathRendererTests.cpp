/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPath.h"

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#include "GrTessellatingPathRenderer.h"
#include "GrTest.h"
#include "Test.h"

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

static void test_path(GrDrawTarget* dt, GrRenderTarget* rt, GrResourceProvider* rp,
                      const SkPath& path) {
    GrTessellatingPathRenderer tess;
    GrPipelineBuilder pipelineBuilder;
    pipelineBuilder.setRenderTarget(rt);
    GrStrokeInfo stroke(SkStrokeRec::kFill_InitStyle);
    GrPathRenderer::DrawPathArgs args;
    args.fTarget = dt;
    args.fPipelineBuilder = &pipelineBuilder;
    args.fResourceProvider = rp;
    args.fColor = GrColor_WHITE;
    args.fViewMatrix = &SkMatrix::I();
    args.fPath = &path;
    args.fStroke = &stroke;
    args.fAntiAlias = false;
    tess.drawPath(args);
}

DEF_GPUTEST(TessellatingPathRendererTests, reporter, factory) {
    GrContext* context = factory->get(static_cast<GrContextFactory::GLContextType>(0));
    if (NULL == context) {
        return;
    }
    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = 800;
    desc.fHeight = 800;
    desc.fConfig = kSkia8888_GrPixelConfig;
    desc.fOrigin = kTopLeft_GrSurfaceOrigin;
    SkAutoTUnref<GrTexture> texture(context->textureProvider()->createApproxTexture(desc));
    GrTestTarget tt;
    context->getTestTarget(&tt);
    GrRenderTarget* rt = texture->asRenderTarget();
    GrDrawTarget* dt = tt.target();
    GrResourceProvider* rp = tt.resourceProvider();

    test_path(dt, rt, rp, create_path_0());
    test_path(dt, rt, rp, create_path_1());
    test_path(dt, rt, rp, create_path_2());
    test_path(dt, rt, rp, create_path_3());
    test_path(dt, rt, rp, create_path_4());
    test_path(dt, rt, rp, create_path_5());
    test_path(dt, rt, rp, create_path_6());
    test_path(dt, rt, rp, create_path_7());
    test_path(dt, rt, rp, create_path_8());
    test_path(dt, rt, rp, create_path_9());
    test_path(dt, rt, rp, create_path_10());
    test_path(dt, rt, rp, create_path_11());
    test_path(dt, rt, rp, create_path_12());
    test_path(dt, rt, rp, create_path_13());
    test_path(dt, rt, rp, create_path_14());
    test_path(dt, rt, rp, create_path_15());
}
#endif
