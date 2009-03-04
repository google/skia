#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTypeface.h"
#include "SkAvoidXfermode.h"

static inline SkPMColor rgb2gray(SkPMColor c)
{
    unsigned r = SkGetPackedR32(c);
    unsigned g = SkGetPackedG32(c);
    unsigned b = SkGetPackedB32(c);
    
    unsigned x = r * 5 + g * 7 + b * 4 >> 4;
    
    return SkPackARGB32(0, x, x, x) | (c & (SK_A32_MASK << SK_A32_SHIFT));
}

class SkGrayScaleColorFilter : public SkColorFilter {
public:
    virtual void filterSpan(const SkPMColor src[], int count, SkPMColor result[])
    {
        for (int i = 0; i < count; i++)
            result[i] = rgb2gray(src[i]);
    }
};

class SkChannelMaskColorFilter : public SkColorFilter {
public:
    SkChannelMaskColorFilter(U8CPU redMask, U8CPU greenMask, U8CPU blueMask)
    {
        fMask = SkPackARGB32(0xFF, redMask, greenMask, blueMask);
    }

    virtual void filterSpan(const SkPMColor src[], int count, SkPMColor result[])
    {
        SkPMColor mask = fMask;
        for (int i = 0; i < count; i++)
            result[i] = src[i] & mask;
    }
    
private:
    SkPMColor   fMask;
};

///////////////////////////////////////////////////////////

#include "SkGradientShader.h"
#include "SkLayerRasterizer.h"
#include "SkBlurMaskFilter.h"

static void r0(SkLayerRasterizer* rast, SkPaint& p)
{
    p.setMaskFilter(SkBlurMaskFilter::Create(SkIntToScalar(3),
                                             SkBlurMaskFilter::kNormal_BlurStyle))->unref();
    rast->addLayer(p, SkIntToScalar(3), SkIntToScalar(3));

    p.setMaskFilter(NULL);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1);
    rast->addLayer(p);

    p.setAlpha(0x11);
    p.setStyle(SkPaint::kFill_Style);
    p.setPorterDuffXfermode(SkPorterDuff::kSrc_Mode);
    rast->addLayer(p);
}

static void r1(SkLayerRasterizer* rast, SkPaint& p)
{
    rast->addLayer(p);

    p.setAlpha(0x40);
    p.setPorterDuffXfermode(SkPorterDuff::kSrc_Mode);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1*2);
    rast->addLayer(p);
}
    
static void r2(SkLayerRasterizer* rast, SkPaint& p)
{
    p.setStyle(SkPaint::kStrokeAndFill_Style);
    p.setStrokeWidth(SK_Scalar1*4);
    rast->addLayer(p);

    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1*3/2);
    p.setPorterDuffXfermode(SkPorterDuff::kClear_Mode);
    rast->addLayer(p);
}

static void r3(SkLayerRasterizer* rast, SkPaint& p)
{
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1*3);
    rast->addLayer(p);

    p.setAlpha(0x20);
    p.setStyle(SkPaint::kFill_Style);
    p.setPorterDuffXfermode(SkPorterDuff::kSrc_Mode);
    rast->addLayer(p);
}

static void r4(SkLayerRasterizer* rast, SkPaint& p)
{
    p.setAlpha(0x60);
    rast->addLayer(p, SkIntToScalar(3), SkIntToScalar(3));

    p.setAlpha(0xFF);
    p.setPorterDuffXfermode(SkPorterDuff::kClear_Mode);
    rast->addLayer(p, SK_Scalar1*3/2, SK_Scalar1*3/2);

    p.setXfermode(NULL);
    rast->addLayer(p);
}

#include "SkDiscretePathEffect.h"

static void r5(SkLayerRasterizer* rast, SkPaint& p)
{
    rast->addLayer(p);

    p.setPathEffect(new SkDiscretePathEffect(SK_Scalar1*4, SK_Scalar1*3))->unref();
    p.setPorterDuffXfermode(SkPorterDuff::kSrcOut_Mode);
    rast->addLayer(p);
}

static void r6(SkLayerRasterizer* rast, SkPaint& p)
{
    rast->addLayer(p);
    
    p.setAntiAlias(false);
    SkLayerRasterizer* rast2 = new SkLayerRasterizer;
    r5(rast2, p);
    p.setRasterizer(rast2)->unref();
    p.setPorterDuffXfermode(SkPorterDuff::kClear_Mode);
    rast->addLayer(p);
}

#include "Sk2DPathEffect.h"

class Dot2DPathEffect : public Sk2DPathEffect {
public:
    Dot2DPathEffect(SkScalar radius, const SkMatrix& matrix)
        : Sk2DPathEffect(matrix), fRadius(radius) {}

    virtual void flatten(SkFlattenableWriteBuffer& buffer)
    {
        this->INHERITED::flatten(buffer);
        
        buffer.writeScalar(fRadius);
    }
    virtual Factory getFactory() { return CreateProc; }

protected:
	virtual void next(const SkPoint& loc, int u, int v, SkPath* dst)
    {
        dst->addCircle(loc.fX, loc.fY, fRadius);
    }
    
    Dot2DPathEffect(SkFlattenableReadBuffer& buffer) : Sk2DPathEffect(buffer)
    {
        fRadius = buffer.readScalar();
    }
private:
    SkScalar fRadius;

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer)
    {
        return new Dot2DPathEffect(buffer);
    }

    typedef Sk2DPathEffect INHERITED;
};

static void r7(SkLayerRasterizer* rast, SkPaint& p)
{
    SkMatrix    lattice;
    lattice.setScale(SK_Scalar1*6, SK_Scalar1*6, 0, 0);
    lattice.postSkew(SK_Scalar1/3, 0, 0, 0);
    p.setPathEffect(new Dot2DPathEffect(SK_Scalar1*4, lattice))->unref();
    rast->addLayer(p);
}

static void r8(SkLayerRasterizer* rast, SkPaint& p)
{
    rast->addLayer(p);
    
    SkMatrix    lattice;
    lattice.setScale(SK_Scalar1*6, SK_Scalar1*6, 0, 0);
    lattice.postSkew(SK_Scalar1/3, 0, 0, 0);
    p.setPathEffect(new Dot2DPathEffect(SK_Scalar1*2, lattice))->unref();
    p.setPorterDuffXfermode(SkPorterDuff::kClear_Mode);
    rast->addLayer(p);

    p.setPathEffect(NULL);
    p.setXfermode(NULL);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1);
    rast->addLayer(p);
}

class Line2DPathEffect : public Sk2DPathEffect {
public:
    Line2DPathEffect(SkScalar width, const SkMatrix& matrix)
        : Sk2DPathEffect(matrix), fWidth(width) {}

	virtual bool filterPath(SkPath* dst, const SkPath& src, SkScalar* width)
    {
        if (this->INHERITED::filterPath(dst, src, width))
        {
            *width = fWidth;
            return true;
        }
        return false;
    }
    
    virtual Factory getFactory() { return CreateProc; }
    virtual void flatten(SkFlattenableWriteBuffer& buffer)
    {
        this->INHERITED::flatten(buffer);
        buffer.writeScalar(fWidth);
    }
protected:
	virtual void nextSpan(int u, int v, int ucount, SkPath* dst)
    {
        if (ucount > 1)
        {
            SkPoint	src[2], dstP[2];

            src[0].set(SkIntToScalar(u) + SK_ScalarHalf,
                       SkIntToScalar(v) + SK_ScalarHalf);
            src[1].set(SkIntToScalar(u+ucount) + SK_ScalarHalf,
                       SkIntToScalar(v) + SK_ScalarHalf);
            this->getMatrix().mapPoints(dstP, src, 2);
            
            dst->moveTo(dstP[0]);
            dst->lineTo(dstP[1]);
        }
    }
    
    Line2DPathEffect::Line2DPathEffect(SkFlattenableReadBuffer& buffer) : Sk2DPathEffect(buffer)
    {
        fWidth = buffer.readScalar();
    }
    
private:
    SkScalar fWidth;

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer)
    {
        return new Line2DPathEffect(buffer);
    }

    typedef Sk2DPathEffect INHERITED;
};

static void r9(SkLayerRasterizer* rast, SkPaint& p)
{
    rast->addLayer(p);
    
    SkMatrix    lattice;
    lattice.setScale(SK_Scalar1, SK_Scalar1*6, 0, 0);
    lattice.postRotate(SkIntToScalar(30), 0, 0);
    p.setPathEffect(new Line2DPathEffect(SK_Scalar1*2, lattice))->unref();
    p.setPorterDuffXfermode(SkPorterDuff::kClear_Mode);
    rast->addLayer(p);

    p.setPathEffect(NULL);
    p.setXfermode(NULL);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SK_Scalar1);
    rast->addLayer(p);
}

typedef void (*raster_proc)(SkLayerRasterizer*, SkPaint&);

static const raster_proc gRastProcs[] = {
    r0, r1, r2, r3, r4, r5, r6, r7, r8, r9
};

static const struct {
    SkColor fMul, fAdd;
} gLightingColors[] = {
    { 0x808080, 0x800000 }, // general case
    { 0x707070, 0x707070 }, // no-pin case
    { 0xFFFFFF, 0x800000 }, // just-add case
    { 0x808080, 0x000000 }, // just-mul case
    { 0xFFFFFF, 0x000000 }  // identity case
};

#include "SkXfermode.h"

static unsigned color_dist16(uint16_t a, uint16_t b)
{
    unsigned dr = SkAbs32(SkPacked16ToR32(a) - SkPacked16ToR32(b));
    unsigned dg = SkAbs32(SkPacked16ToG32(a) - SkPacked16ToG32(b));
    unsigned db = SkAbs32(SkPacked16ToB32(a) - SkPacked16ToB32(b));
    
    return SkMax32(dr, SkMax32(dg, db));
}

static unsigned scale_dist(unsigned dist, unsigned scale)
{
    dist >>= 6;
    dist = (dist << 2) | dist;
    dist = (dist << 4) | dist;
    return dist;

//    return SkAlphaMul(dist, scale);
}

static void apply_shader(SkPaint* paint, int index)
{    
    raster_proc proc = gRastProcs[index];
    if (proc)
    {
        SkPaint p;
        SkLayerRasterizer*  rast = new SkLayerRasterizer;

        p.setAntiAlias(true);
        proc(rast, p);
        paint->setRasterizer(rast)->unref();
    }

#if 0
    SkScalar dir[] = { SK_Scalar1, SK_Scalar1, SK_Scalar1 };
    paint->setMaskFilter(SkBlurMaskFilter::CreateEmboss(dir, SK_Scalar1/4, SkIntToScalar(4), SkIntToScalar(3)))->unref();    
#endif
    paint->setColor(SK_ColorBLUE);
}

static int gRastIndex;

class TextEffectView : public SkView {
    SkTypeface* fFace;
public:
	TextEffectView()
    {
        fFace = SkTypeface::CreateFromFile("/Users/reed/Downloads/p052024l.pfb");
    }
    
    virtual ~TextEffectView()
    {
        fFace->safeUnref();
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt)
    {
        if (SampleCode::TitleQ(*evt))
        {
            SampleCode::TitleR(evt, "Text Effects");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas)
    {
//        canvas->drawColor(0xFFDDDDDD);
        canvas->drawColor(SK_ColorWHITE);
    }
    
    virtual void onDraw(SkCanvas* canvas)
    {
        this->drawBG(canvas);
        
        canvas->save();
//        canvas->scale(SK_Scalar1*2, SK_Scalar1*2, 0, 0);

        SkScalar    x = SkIntToScalar(20);
        SkScalar    y = SkIntToScalar(40);
        SkPaint     paint;
        
        paint.setAntiAlias(true);
        paint.setTextSize(SkIntToScalar(48));
        paint.setTypeface(SkTypeface::CreateFromName("sans-serif",
                                                     SkTypeface::kBold));

        SkString str("GOOGLE ");
        str.appendUnichar(0x5700);

        paint.setTypeface(fFace);
        
        for (int i = 0; i < SK_ARRAY_COUNT(gRastProcs); i++)
        {
            apply_shader(&paint, i);
            
          //  paint.setMaskFilter(NULL);
          //  paint.setColor(SK_ColorBLACK);

#if 0
            int index = i % SK_ARRAY_COUNT(gLightingColors);
            paint.setColorFilter(SkColorFilter::CreateLightingFilter(
                                    gLightingColors[index].fMul,
                                    gLightingColors[index].fAdd))->unref();
#endif
            
            canvas->drawText(str.c_str(), str.size(), x, y, paint);

            if (0)
            {
                SkPath path;
                paint.getTextPath(str.c_str(), str.size(), x + SkIntToScalar(260), y, &path);
                canvas->drawPath(path, paint);
            }

            y += paint.getFontSpacing();
        }

        canvas->restore();
        
        if (0)
        {
            SkPoint pts[] = { 0, 0, 0, SkIntToScalar(150) };
            SkColor colors[] = { 0xFFE6E6E6, 0xFFFFFFFF };
            SkShader* s = SkGradientShader::CreateLinear(pts, colors, NULL, 2, SkShader::kClamp_TileMode);

            paint.reset();
            paint.setShader(s);
            canvas->drawRectCoords(0, 0, SkIntToScalar(120), SkIntToScalar(150), paint);
        }
        
        if (1)
        {
            SkAvoidXfermode   mode(SK_ColorWHITE, 0xFF,
                                    SkAvoidXfermode::kTargetColor_Mode);
            SkPaint paint;
            x += SkIntToScalar(20);
            SkRect  r = { x, 0, x + SkIntToScalar(360), SkIntToScalar(700) };
            paint.setXfermode(&mode);
            paint.setColor(SK_ColorGREEN);
            paint.setAntiAlias(true);
            canvas->drawOval(r, paint);
        }
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) 
    {
        gRastIndex = (gRastIndex + 1) % SK_ARRAY_COUNT(gRastProcs);
        this->inval(NULL);

        return this->INHERITED::onFindClickHandler(x, y);
    }
    
    virtual bool onClick(Click* click) 
    {
        return this->INHERITED::onClick(click);
    }
    
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new TextEffectView; }
static SkViewRegister reg(MyFactory);

