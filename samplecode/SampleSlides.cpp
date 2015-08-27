
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkGradientShader.h"
#include "SkLayerRasterizer.h"
#include "SkPaint.h"
#include "SkView.h"

#define BG_COLOR    0xFFDDDDDD

typedef void (*SlideProc)(SkCanvas*);

///////////////////////////////////////////////////////////////////////////////

#include "Sk1DPathEffect.h"
#include "Sk2DPathEffect.h"
#include "SkCornerPathEffect.h"
#include "SkDashPathEffect.h"
#include "SkDiscretePathEffect.h"

static void compose_pe(SkPaint* paint) {
    SkPathEffect* pe = paint->getPathEffect();
    SkPathEffect* corner = SkCornerPathEffect::Create(25);
    SkPathEffect* compose;
    if (pe) {
        compose = SkComposePathEffect::Create(pe, corner);
        corner->unref();
    } else {
        compose = corner;
    }
    paint->setPathEffect(compose)->unref();
}

static void hair_pe(SkPaint* paint) {
    paint->setStrokeWidth(0);
}

static void hair2_pe(SkPaint* paint) {
    paint->setStrokeWidth(0);
    compose_pe(paint);
}

static void stroke_pe(SkPaint* paint) {
    paint->setStrokeWidth(12);
    compose_pe(paint);
}

static void dash_pe(SkPaint* paint) {
    SkScalar inter[] = { 20, 10, 10, 10 };
    paint->setStrokeWidth(12);
    paint->setPathEffect(SkDashPathEffect::Create(inter, SK_ARRAY_COUNT(inter),
                                                  0))->unref();
    compose_pe(paint);
}

static const int gXY[] = {
4, 0, 0, -4, 8, -4, 12, 0, 8, 4, 0, 4
};

static void scale(SkPath* path, SkScalar scale) {
    SkMatrix m;
    m.setScale(scale, scale);
    path->transform(m);
}

static void one_d_pe(SkPaint* paint) {
    SkPath  path;
    path.moveTo(SkIntToScalar(gXY[0]), SkIntToScalar(gXY[1]));
    for (unsigned i = 2; i < SK_ARRAY_COUNT(gXY); i += 2)
        path.lineTo(SkIntToScalar(gXY[i]), SkIntToScalar(gXY[i+1]));
    path.close();
    path.offset(SkIntToScalar(-6), 0);
    scale(&path, 1.5f);

    paint->setPathEffect(SkPath1DPathEffect::Create(path, SkIntToScalar(21), 0,
                                                    SkPath1DPathEffect::kRotate_Style))->unref();
    compose_pe(paint);
}

typedef void (*PE_Proc)(SkPaint*);
static const PE_Proc gPE[] = { hair_pe, hair2_pe, stroke_pe, dash_pe, one_d_pe };

static void fill_pe(SkPaint* paint) {
    paint->setStyle(SkPaint::kFill_Style);
    paint->setPathEffect(nullptr);
}

static void discrete_pe(SkPaint* paint) {
    paint->setPathEffect(SkDiscretePathEffect::Create(10, 4))->unref();
}

static SkPathEffect* MakeTileEffect() {
    SkMatrix m;
    m.setScale(SkIntToScalar(12), SkIntToScalar(12));

    SkPath path;
    path.addCircle(0, 0, SkIntToScalar(5));

    return SkPath2DPathEffect::Create(m, path);
}

static void tile_pe(SkPaint* paint) {
    paint->setPathEffect(MakeTileEffect())->unref();
}

static const PE_Proc gPE2[] = { fill_pe, discrete_pe, tile_pe };

static void patheffect_slide(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);

    SkPath path;
    path.moveTo(20, 20);
    path.lineTo(70, 120);
    path.lineTo(120, 30);
    path.lineTo(170, 80);
    path.lineTo(240, 50);

    size_t i;
    canvas->save();
    for (i = 0; i < SK_ARRAY_COUNT(gPE); i++) {
        gPE[i](&paint);
        canvas->drawPath(path, paint);
        canvas->translate(0, 75);
    }
    canvas->restore();

    path.reset();
    SkRect r = { 0, 0, 250, 120 };
    path.addOval(r, SkPath::kCW_Direction);
    r.inset(50, 50);
    path.addRect(r, SkPath::kCCW_Direction);

    canvas->translate(320, 20);
    for (i = 0; i < SK_ARRAY_COUNT(gPE2); i++) {
        gPE2[i](&paint);
        canvas->drawPath(path, paint);
        canvas->translate(0, 160);
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "SkGradientShader.h"

struct GradData {
    int             fCount;
    const SkColor*  fColors;
    const SkScalar* fPos;
};

static const SkColor gColors[] = {
SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK
};
static const SkScalar gPos0[] = { 0, SK_Scalar1 };
static const SkScalar gPos1[] = { SK_Scalar1/4, SK_Scalar1*3/4 };
static const SkScalar gPos2[] = {
0, SK_Scalar1/8, SK_Scalar1/2, SK_Scalar1*7/8, SK_Scalar1
};

static const GradData gGradData[] = {
{ 2, gColors, nullptr },
{ 2, gColors, gPos0 },
{ 2, gColors, gPos1 },
{ 5, gColors, nullptr },
{ 5, gColors, gPos2 }
};

static SkShader* MakeLinear(const SkPoint pts[2], const GradData& data, SkShader::TileMode tm) {
    return SkGradientShader::CreateLinear(pts, data.fColors, data.fPos, data.fCount, tm);
}

static SkShader* MakeRadial(const SkPoint pts[2], const GradData& data, SkShader::TileMode tm) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    return SkGradientShader::CreateRadial(center, center.fX, data.fColors,
                                          data.fPos, data.fCount, tm);
}

static SkShader* MakeSweep(const SkPoint pts[2], const GradData& data, SkShader::TileMode tm) {
    SkPoint center;
    center.set(SkScalarAve(pts[0].fX, pts[1].fX),
               SkScalarAve(pts[0].fY, pts[1].fY));
    return SkGradientShader::CreateSweep(center.fX, center.fY, data.fColors, data.fPos, data.fCount);
}

static SkShader* Make2Conical(const SkPoint pts[2], const GradData& data, SkShader::TileMode tm) {
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::CreateTwoPointConical(
                                                  center1, (pts[1].fX - pts[0].fX) / 7,
                                                  center0, (pts[1].fX - pts[0].fX) / 2,
                                                  data.fColors, data.fPos, data.fCount, tm);
}

typedef SkShader* (*GradMaker)(const SkPoint pts[2], const GradData& data, SkShader::TileMode tm);
static const GradMaker gGradMakers[] = {
    MakeLinear, MakeRadial, MakeSweep, Make2Conical
};

static void gradient_slide(SkCanvas* canvas) {
    SkPoint pts[2] = {
        { 0, 0 },
        { SkIntToScalar(100), SkIntToScalar(100) }
    };
    SkShader::TileMode tm = SkShader::kClamp_TileMode;
    SkRect r = { 0, 0, SkIntToScalar(100), SkIntToScalar(100) };
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setDither(true);

    canvas->translate(SkIntToScalar(20), SkIntToScalar(10));
    for (size_t i = 0; i < SK_ARRAY_COUNT(gGradData); i++) {
        canvas->save();
        for (size_t j = 0; j < SK_ARRAY_COUNT(gGradMakers); j++) {
            SkShader* shader = gGradMakers[j](pts, gGradData[i], tm);
            paint.setShader(shader);
            canvas->drawRect(r, paint);
            shader->unref();
            canvas->translate(0, SkIntToScalar(120));
        }
        canvas->restore();
        canvas->translate(SkIntToScalar(120), 0);
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "SkPathMeasure.h"

static SkScalar getpathlen(const SkPath& path) {
    SkPathMeasure   meas(path, false);
    return meas.getLength();
}

static void textonpath_slide(SkCanvas* canvas) {
    const char* text = "Displacement";
    size_t len =strlen(text);
    SkPath path;
    path.moveTo(100, 300);
    path.quadTo(300, 100, 500, 300);
    path.offset(0, -100);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(40);

    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(path, paint);
    paint.setStyle(SkPaint::kFill_Style);

    SkScalar x = 50;
    paint.setColor(0xFF008800);
    canvas->drawTextOnPathHV(text, len, path,
                             x, paint.getTextSize()*2/3, paint);
    paint.setColor(SK_ColorRED);
    canvas->drawTextOnPathHV(text, len, path,
                             x + 60, 0, paint);
    paint.setColor(SK_ColorBLUE);
    canvas->drawTextOnPathHV(text, len, path,
                             x + 120, -paint.getTextSize()*2/3, paint);

    path.offset(0, 200);
    paint.setTextAlign(SkPaint::kRight_Align);

    text = "Matrices";
    len = strlen(text);
    SkScalar pathLen = getpathlen(path);
    SkMatrix matrix;

    paint.setColor(SK_ColorBLACK);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(path, paint);
    paint.setStyle(SkPaint::kFill_Style);

    paint.setTextSize(50);
    canvas->drawTextOnPath(text, len, path, nullptr, paint);

    paint.setColor(SK_ColorRED);
    matrix.setScale(-SK_Scalar1, SK_Scalar1);
    matrix.postTranslate(pathLen, 0);
    canvas->drawTextOnPath(text, len, path, &matrix, paint);

    paint.setColor(SK_ColorBLUE);
    matrix.setScale(SK_Scalar1, -SK_Scalar1);
    canvas->drawTextOnPath(text, len, path, &matrix, paint);

    paint.setColor(0xFF008800);
    matrix.setScale(-SK_Scalar1, -SK_Scalar1);
    matrix.postTranslate(pathLen, 0);
    canvas->drawTextOnPath(text, len, path, &matrix, paint);
}

///////////////////////////////////////////////////////////////////////////////

#include "SkImageDecoder.h"
#include "SkOSFile.h"
#include "SkRandom.h"
#include "SkStream.h"

static SkShader* make_shader0(SkIPoint* size) {
    SkBitmap    bm;

    SkImageDecoder::DecodeFile("/skimages/logo.gif", &bm);
    size->set(bm.width(), bm.height());
    return SkShader::CreateBitmapShader(bm, SkShader::kClamp_TileMode,
                                        SkShader::kClamp_TileMode);
}

static SkShader* make_shader1(const SkIPoint& size) {
    SkPoint pts[] = { { 0, 0 },
                      { SkIntToScalar(size.fX), SkIntToScalar(size.fY) } };
    SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorRED };
    return SkGradientShader::CreateLinear(pts, colors, nullptr,
                                          SK_ARRAY_COUNT(colors), SkShader::kMirror_TileMode);
}

class Rec {
public:
    SkCanvas::VertexMode    fMode;
    int                     fCount;
    SkPoint*                fVerts;
    SkPoint*                fTexs;

    Rec() : fCount(0), fVerts(nullptr), fTexs(nullptr) {}
    ~Rec() { delete[] fVerts; delete[] fTexs; }
};

static void make_tris(Rec* rec) {
    int n = 10;
    SkRandom    rand;

    rec->fMode = SkCanvas::kTriangles_VertexMode;
    rec->fCount = n * 3;
    rec->fVerts = new SkPoint[rec->fCount];

    for (int i = 0; i < n; i++) {
        SkPoint* v = &rec->fVerts[i*3];
        for (int j = 0; j < 3; j++) {
            v[j].set(rand.nextUScalar1() * 250, rand.nextUScalar1() * 250);
        }
    }
}

static void make_fan(Rec* rec, int texWidth, int texHeight) {
    const SkScalar tx = SkIntToScalar(texWidth);
    const SkScalar ty = SkIntToScalar(texHeight);
    const int n = 24;

    rec->fMode = SkCanvas::kTriangleFan_VertexMode;
    rec->fCount = n + 2;
    rec->fVerts = new SkPoint[rec->fCount];
    rec->fTexs  = new SkPoint[rec->fCount];

    SkPoint* v = rec->fVerts;
    SkPoint* t = rec->fTexs;

    v[0].set(0, 0);
    t[0].set(0, 0);
    for (int i = 0; i < n; i++) {
        SkScalar cos;
        SkScalar sin = SkScalarSinCos(SK_ScalarPI * 2 * i / n, &cos);
        v[i+1].set(cos, sin);
        t[i+1].set(i*tx/n, ty);
    }
    v[n+1] = v[1];
    t[n+1].set(tx, ty);

    SkMatrix m;
    m.setScale(SkIntToScalar(100), SkIntToScalar(100));
    m.postTranslate(SkIntToScalar(110), SkIntToScalar(110));
    m.mapPoints(v, rec->fCount);
}

static void make_strip(Rec* rec, int texWidth, int texHeight) {
    const SkScalar tx = SkIntToScalar(texWidth);
    const SkScalar ty = SkIntToScalar(texHeight);
    const int n = 24;

    rec->fMode = SkCanvas::kTriangleStrip_VertexMode;
    rec->fCount = 2 * (n + 1);
    rec->fVerts = new SkPoint[rec->fCount];
    rec->fTexs  = new SkPoint[rec->fCount];

    SkPoint* v = rec->fVerts;
    SkPoint* t = rec->fTexs;

    for (int i = 0; i < n; i++) {
        SkScalar cos;
        SkScalar sin = SkScalarSinCos(SK_ScalarPI * 2 * i / n, &cos);
        v[i*2 + 0].set(cos/2, sin/2);
        v[i*2 + 1].set(cos, sin);

        t[i*2 + 0].set(tx * i / n, ty);
        t[i*2 + 1].set(tx * i / n, 0);
    }
    v[2*n + 0] = v[0];
    v[2*n + 1] = v[1];

    t[2*n + 0].set(tx, ty);
    t[2*n + 1].set(tx, 0);

    SkMatrix m;
    m.setScale(SkIntToScalar(100), SkIntToScalar(100));
    m.postTranslate(SkIntToScalar(110), SkIntToScalar(110));
    m.mapPoints(v, rec->fCount);
}

static void mesh_slide(SkCanvas* canvas) {
    Rec fRecs[3];
    SkIPoint    size;

    SkShader* fShader0 = make_shader0(&size);
    SkShader* fShader1 = make_shader1(size);

    SkAutoUnref aur0(fShader0);
    SkAutoUnref aur1(fShader1);

    make_strip(&fRecs[0], size.fX, size.fY);
    make_fan(&fRecs[1], size.fX, size.fY);
    make_tris(&fRecs[2]);

    SkPaint paint;
    paint.setDither(true);
    paint.setFilterQuality(kLow_SkFilterQuality);

    for (size_t i = 0; i < SK_ARRAY_COUNT(fRecs); i++) {
        canvas->save();

        paint.setShader(nullptr);
        canvas->drawVertices(fRecs[i].fMode, fRecs[i].fCount,
                             fRecs[i].fVerts, fRecs[i].fTexs,
                             nullptr, nullptr, nullptr, 0, paint);

        canvas->translate(SkIntToScalar(210), 0);

        paint.setShader(fShader0);
        canvas->drawVertices(fRecs[i].fMode, fRecs[i].fCount,
                             fRecs[i].fVerts, fRecs[i].fTexs,
                             nullptr, nullptr, nullptr, 0, paint);

        canvas->translate(SkIntToScalar(210), 0);

        paint.setShader(fShader1);
        canvas->drawVertices(fRecs[i].fMode, fRecs[i].fCount,
                             fRecs[i].fVerts, fRecs[i].fTexs,
                             nullptr, nullptr, nullptr, 0, paint);
        canvas->restore();

        canvas->translate(0, SkIntToScalar(250));
    }
}

///////////////////////////////////////////////////////////////////////////////

static void r0(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p)
{
    p.setMaskFilter(SkBlurMaskFilter::Create(kNormal_SkBlurStyle,
                                             SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(3))))->unref();
    rastBuilder->addLayer(p, SkIntToScalar(3), SkIntToScalar(3));

    p.setMaskFilter(nullptr);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1);
    rastBuilder->addLayer(p);

    p.setAlpha(0x11);
    p.setStyle(SkPaint::kFill_Style);
    p.setXfermodeMode(SkXfermode::kSrc_Mode);
    rastBuilder->addLayer(p);
}

static void r1(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p)
{
    rastBuilder->addLayer(p);

    p.setAlpha(0x40);
    p.setXfermodeMode(SkXfermode::kSrc_Mode);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1*2);
    rastBuilder->addLayer(p);
}

static void r2(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p)
{
    p.setStyle(SkPaint::kStrokeAndFill_Style);
    p.setStrokeWidth(SK_Scalar1*4);
    rastBuilder->addLayer(p);

    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1*3/2);
    p.setXfermodeMode(SkXfermode::kClear_Mode);
    rastBuilder->addLayer(p);
}

static void r3(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p)
{
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1*3);
    rastBuilder->addLayer(p);

    p.setAlpha(0x20);
    p.setStyle(SkPaint::kFill_Style);
    p.setXfermodeMode(SkXfermode::kSrc_Mode);
    rastBuilder->addLayer(p);
}

static void r4(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p)
{
    p.setAlpha(0x60);
    rastBuilder->addLayer(p, SkIntToScalar(3), SkIntToScalar(3));

    p.setAlpha(0xFF);
    p.setXfermodeMode(SkXfermode::kClear_Mode);
    rastBuilder->addLayer(p, SK_Scalar1*3/2, SK_Scalar1*3/2);

    p.setXfermode(nullptr);
    rastBuilder->addLayer(p);
}

#include "SkDiscretePathEffect.h"

static void r5(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p)
{
    rastBuilder->addLayer(p);

    p.setPathEffect(SkDiscretePathEffect::Create(SK_Scalar1*4, SK_Scalar1*3))->unref();
    p.setXfermodeMode(SkXfermode::kSrcOut_Mode);
    rastBuilder->addLayer(p);
}

static void r6(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p)
{
    rastBuilder->addLayer(p);

    p.setAntiAlias(false);
    SkLayerRasterizer::Builder rastBuilder2;
    r5(&rastBuilder2, p);
    p.setRasterizer(rastBuilder2.detachRasterizer())->unref();
    p.setXfermodeMode(SkXfermode::kClear_Mode);
    rastBuilder->addLayer(p);
}

#include "Sk2DPathEffect.h"

static SkPathEffect* MakeDotEffect(SkScalar radius, const SkMatrix& matrix) {
    SkPath path;
    path.addCircle(0, 0, radius);
    return SkPath2DPathEffect::Create(matrix, path);
}

static void r7(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p)
{
    SkMatrix    lattice;
    lattice.setScale(SK_Scalar1*6, SK_Scalar1*6, 0, 0);
    lattice.postSkew(SK_Scalar1/3, 0, 0, 0);
    p.setPathEffect(MakeDotEffect(SK_Scalar1*4, lattice))->unref();
    rastBuilder->addLayer(p);
}

static void r8(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p)
{
    rastBuilder->addLayer(p);

    SkMatrix    lattice;
    lattice.setScale(SK_Scalar1*6, SK_Scalar1*6, 0, 0);
    lattice.postSkew(SK_Scalar1/3, 0, 0, 0);
    p.setPathEffect(MakeDotEffect(SK_Scalar1*2, lattice))->unref();
    p.setXfermodeMode(SkXfermode::kClear_Mode);
    rastBuilder->addLayer(p);

    p.setPathEffect(nullptr);
    p.setXfermode(nullptr);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1);
    rastBuilder->addLayer(p);
}

static void r9(SkLayerRasterizer::Builder* rastBuilder, SkPaint& p)
{
    rastBuilder->addLayer(p);

    SkMatrix    lattice;
    lattice.setScale(SK_Scalar1, SK_Scalar1*6, 0, 0);
    lattice.postRotate(SkIntToScalar(30), 0, 0);
    p.setPathEffect(SkLine2DPathEffect::Create(SK_Scalar1*2, lattice))->unref();
    p.setXfermodeMode(SkXfermode::kClear_Mode);
    rastBuilder->addLayer(p);

    p.setPathEffect(nullptr);
    p.setXfermode(nullptr);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1);
    rastBuilder->addLayer(p);
}

typedef void (*raster_proc)(SkLayerRasterizer::Builder*, SkPaint&);

static const raster_proc gRastProcs[] = {
    r0, r1, r2, r3, r4, r5, r6, r7, r8, r9
};

static void apply_shader(SkPaint* paint, int index) {
    raster_proc proc = gRastProcs[index];
    SkPaint p;
    SkLayerRasterizer::Builder rastBuilder;

    p.setAntiAlias(true);
    proc(&rastBuilder, p);
    paint->setRasterizer(rastBuilder.detachRasterizer())->unref();
    paint->setColor(SK_ColorBLUE);
}

#include "SkTypeface.h"

static void texteffect_slide(SkCanvas* canvas) {
    const char* str = "Google";
    size_t len = strlen(str);
    SkScalar x = 20;
    SkScalar y = 80;
    SkPaint paint;
    paint.setTypeface(SkTypeface::CreateFromName("Georgia", SkTypeface::kItalic));
    paint.setTextSize(75);
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLUE);
    for (size_t i = 0; i < SK_ARRAY_COUNT(gRastProcs); i++) {
        apply_shader(&paint, (int)i);
        canvas->drawText(str, len, x, y, paint);
        y += 80;
        if (i == 4) {
            x += 320;
            y = 80;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "SkImageEncoder.h"

static const SlideProc gProc[] = {
    patheffect_slide,
    gradient_slide,
    textonpath_slide,
    mesh_slide,
    texteffect_slide
};

class SlideView : public SampleView {
    int fIndex;
    bool fOnce;
public:
    SlideView() {
        fOnce = false;
    }

    void init() {
        if (fOnce) {
            return;
        }
        fOnce = true;

        fIndex = 0;

        SkBitmap bm;
        bm.allocN32Pixels(1024, 768);
        SkCanvas canvas(bm);
        SkScalar s = SkIntToScalar(1024) / 640;
        canvas.scale(s, s);
        for (size_t i = 0; i < SK_ARRAY_COUNT(gProc); i++) {
            canvas.save();
            canvas.drawColor(BG_COLOR);
            gProc[i](&canvas);
            canvas.restore();
            SkString str;
            str.printf("/skimages/slide_" SK_SIZE_T_SPECIFIER ".png", i);
            SkImageEncoder::EncodeFile(str.c_str(), bm, SkImageEncoder::kPNG_Type, 100);
        }
        this->setBGColor(BG_COLOR);
    }

protected:
    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Slides");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        this->init();
        gProc[fIndex](canvas);
    }

    SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned) override {
        this->init();
        fIndex = (fIndex + 1) % SK_ARRAY_COUNT(gProc);
        this->inval(nullptr);
        return nullptr;
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new SlideView; }
static SkViewRegister reg(MyFactory);
