#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkView.h"
#include "Sk1DPathEffect.h"
#include "Sk2DPathEffect.h"
#include "SkAvoidXfermode.h"
#include "SkBlurMaskFilter.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkCornerPathEffect.h"
#include "SkDashPathEffect.h"
#include "SkDiscretePathEffect.h"
#include "SkEmbossMaskFilter.h"
#include "SkGradientShader.h"
#include "SkImageDecoder.h"
#include "SkLayerRasterizer.h"
#include "SkMath.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkShaderExtras.h"
#include "SkCornerPathEffect.h"
#include "SkPathMeasure.h"
#include "SkPicture.h"
#include "SkRandom.h"
#include "SkTransparentShader.h"
#include "SkTypeface.h"
#include "SkUnitMappers.h"
#include "SkUtils.h"
#include "SkXfermode.h"

#include <math.h>

extern void Dump();
    
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

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) { return new Line2DPathEffect(buffer); }

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

#if 1
    SkScalar dir[] = { SK_Scalar1, SK_Scalar1, SK_Scalar1 };
    paint->setMaskFilter(SkBlurMaskFilter::CreateEmboss(dir, SK_Scalar1/4, SkIntToScalar(4), SkIntToScalar(3)))->unref();    
    paint->setColor(SK_ColorBLUE);
#endif
}

static void test_math()
{
    float x;
    const float PI = 3.141593;
    
    for (x = 0; x < 1; x += 0.05f)
        printf("atan(%g) = %g\n", x, atanf(x) * 180/PI);
    for (x = 1; x < 10000000; x *= 2)
        printf("atan(%g) = %g\n", x, atanf(x) * 180/PI);
}

class DemoView : public SkView {
public:
    DemoView()
    {
        test_math();
    }
	
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt)
    {
        if (SampleCode::TitleQ(*evt))
        {
            SampleCode::TitleR(evt, "Demo");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    virtual bool onClick(Click* click) 
    {
        return this->INHERITED::onClick(click);
    }
    
    void makePath(SkPath& path)
    {
        path.addCircle(SkIntToScalar(20), SkIntToScalar(20), SkIntToScalar(20),
            SkPath::kCCW_Direction);
        for (int index = 0; index < 10; index++) {
            SkScalar x = SkFloatToScalar(cos(index / 10.0f * 2 * 3.1415925358f));
            SkScalar y = SkFloatToScalar(sin(index / 10.0f * 2 * 3.1415925358f));
            x *= index & 1 ? 7 : 14;
            y *= index & 1 ? 7 : 14;
            x += SkIntToScalar(20);
            y += SkIntToScalar(20);
            if (index == 0)
                path.moveTo(x, y);
            else
                path.lineTo(x, y);
        }
        path.close();
    }
    
    virtual void onDraw(SkCanvas* canvas)
    {
        canvas->drawColor(SK_ColorWHITE);
        canvas->save();
        drawPicture(canvas, 0);
        canvas->restore();

        {
            SkPicture picture;
            SkCanvas* record = picture.beginRecording(320, 480);
            drawPicture(record, 120);
            canvas->translate(0, SkIntToScalar(120));

            SkRect clip;
            clip.set(0, 0, SkIntToScalar(160), SkIntToScalar(160));
            do {
                canvas->save();
                canvas->clipRect(clip);
                picture.draw(canvas);
                canvas->restore();
                if (clip.fRight < SkIntToScalar(320))
                    clip.offset(SkIntToScalar(160), 0);
                else if (clip.fBottom < SkIntToScalar(480))
                    clip.offset(-SkIntToScalar(320), SkIntToScalar(160));
                else
                    break;
            } while (true);
        }
        Dump();
    }
    
    void drawPicture(SkCanvas* canvas, int spriteOffset)
    {
	    SkMatrix matrix; matrix.reset();
		SkPaint paint;
		SkPath path;
        SkPoint start = {0, 0};
        SkPoint stop = { SkIntToScalar(40), SkIntToScalar(40) };
		SkRect rect = {0, 0, SkIntToScalar(40), SkIntToScalar(40) };
		SkRect rect2 = {0, 0, SkIntToScalar(65), SkIntToScalar(20) };
		SkScalar left = 0, top = 0, x = 0, y = 0;
		int index;
		
		char ascii[] = "ascii...";
		size_t asciiLength = sizeof(ascii) - 1;
		char utf8[] = "utf8" "\xe2\x80\xa6";
		short utf16[] = {'u', 't', 'f', '1', '6', 0x2026 };
		short utf16simple[] = {'u', 't', 'f', '1', '6', '!' };
		
        makePath(path);
        SkTDArray<SkPoint>(pos);
		pos.setCount(asciiLength);
		for (index = 0;  index < asciiLength; index++)
			pos[index].set(SkIntToScalar(index * 10), SkIntToScalar(index * 2));
        SkTDArray<SkPoint>(pos2);
		pos2.setCount(asciiLength);
		for (index = 0;  index < asciiLength; index++)
			pos2[index].set(SkIntToScalar(index * 10), SkIntToScalar(20));
		
        // shaders
        SkPoint linearPoints[] = { 0, 0, SkIntToScalar(40), SkIntToScalar(40) };
        SkColor linearColors[] = { SK_ColorRED, SK_ColorBLUE };
        SkScalar* linearPos = NULL;
        int linearCount = 2;
        SkShader::TileMode linearMode = SkShader::kMirror_TileMode;
        SkUnitMapper* linearMapper = new SkDiscreteMapper(3);
        SkAutoUnref unmapLinearMapper(linearMapper);
        SkShader* linear = SkGradientShader::CreateLinear(linearPoints,
            linearColors, linearPos, linearCount, linearMode, linearMapper);

        SkPoint radialCenter = { SkIntToScalar(25), SkIntToScalar(25) };
        SkScalar radialRadius = SkIntToScalar(25);
        SkColor radialColors[] = { SK_ColorGREEN, SK_ColorGRAY, SK_ColorRED };
        SkScalar radialPos[] = { 0, SkIntToScalar(3) / 5, SkIntToScalar(1)};
        int radialCount = 3;
        SkShader::TileMode radialMode = SkShader::kRepeat_TileMode;
        SkUnitMapper* radialMapper = new SkCosineMapper();
        SkAutoUnref unmapRadialMapper(radialMapper);
        SkShader* radial = SkGradientShader::CreateRadial(radialCenter, 
            radialRadius, radialColors, radialPos, radialCount,
            radialMode, radialMapper);
        
        SkTransparentShader* transparentShader = new SkTransparentShader();
        SkEmbossMaskFilter::Light light;
        light.fDirection[0] = SK_Scalar1/2;
        light.fDirection[1] = SK_Scalar1/2;
        light.fDirection[2] = SK_Scalar1/3;
        light.fAmbient		= 0x48;
        light.fSpecular		= 0x80;
        SkScalar radius = SkIntToScalar(12)/5;
        SkEmbossMaskFilter* embossFilter = new SkEmbossMaskFilter(light, 
            radius);
            
        SkXfermode* xfermode = SkPorterDuff::CreateXfermode(SkPorterDuff::kXor_Mode);
        SkColorFilter* lightingFilter = SkColorFilter::CreateLightingFilter(
            0xff89bc45, 0xff112233);
        
        canvas->save();
		canvas->translate(SkIntToScalar(0), SkIntToScalar(5));
		paint.setFlags(SkPaint::kAntiAlias_Flag | SkPaint::kFilterBitmap_Flag);
		// !!! draw through a clip
		paint.setColor(SK_ColorLTGRAY);
		paint.setStyle(SkPaint::kFill_Style);
        SkRect clip = {0, 0, SkIntToScalar(320), SkIntToScalar(120)};
        canvas->clipRect(clip);
        paint.setShader(SkShader::CreateBitmapShader(fTx, 
            SkShader::kMirror_TileMode, SkShader::kRepeat_TileMode))->unref();
		canvas->drawPaint(paint);
		canvas->save();
        
        // line (exercises xfermode, colorShader, colorFilter, filterShader)
		paint.setColor(SK_ColorGREEN);
		paint.setStrokeWidth(SkIntToScalar(10));
		paint.setStyle(SkPaint::kStroke_Style);
        paint.setXfermode(xfermode)->unref();
        paint.setColorFilter(lightingFilter)->unref();
		canvas->drawLine(start.fX, start.fY, stop.fX, stop.fY, paint); // should not be green
		paint.setXfermode(NULL);
        paint.setColorFilter(NULL);
        
        // rectangle
		paint.setStyle(SkPaint::kFill_Style);
		canvas->translate(SkIntToScalar(50), 0);
		paint.setColor(SK_ColorYELLOW);
        paint.setShader(linear)->unref();
        paint.setPathEffect(pathEffectTest())->unref();
		canvas->drawRect(rect, paint); 
        paint.setPathEffect(NULL);
        
        // circle w/ emboss & transparent (exercises 3dshader)
		canvas->translate(SkIntToScalar(50), 0);
        paint.setMaskFilter(embossFilter)->unref();
        canvas->drawOval(rect, paint);
		canvas->translate(SkIntToScalar(10), SkIntToScalar(10));
        paint.setShader(transparentShader)->unref();
        canvas->drawOval(rect, paint);
		canvas->translate(0, SkIntToScalar(-10));
        
        // path
		canvas->translate(SkIntToScalar(50), 0);
		paint.setColor(SK_ColorRED);
		paint.setStyle(SkPaint::kStroke_Style);
		paint.setStrokeWidth(SkIntToScalar(5));
        paint.setShader(radial)->unref();
        paint.setMaskFilter(NULL);
		canvas->drawPath(path, paint);
		
        paint.setShader(NULL);
        // bitmap, sprite
		canvas->translate(SkIntToScalar(50), 0);
		paint.setStyle(SkPaint::kFill_Style);
		canvas->drawBitmap(fBug, left, top, &paint);
		canvas->translate(SkIntToScalar(30), 0);
		canvas->drawSprite(fTb, 
			SkScalarRound(canvas->getTotalMatrix().getTranslateX()), 
            spriteOffset + 10, &paint);

		canvas->translate(-SkIntToScalar(30), SkIntToScalar(30));
        paint.setShader(shaderTest())->unref(); // test compose shader
		canvas->drawRect(rect2, paint); 
        paint.setShader(NULL);
		
        canvas->restore();
        // text
		canvas->translate(0, SkIntToScalar(60));
        canvas->save();
		paint.setColor(SK_ColorGRAY);
		canvas->drawPosText(ascii, asciiLength, pos.begin(), paint);
		canvas->drawPosText(ascii, asciiLength, pos2.begin(), paint);

		canvas->translate(SkIntToScalar(50), 0);
		paint.setColor(SK_ColorCYAN);
		canvas->drawText(utf8, sizeof(utf8) - 1, x, y, paint);
       
		canvas->translate(SkIntToScalar(30), 0);
		paint.setColor(SK_ColorMAGENTA);
		paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
        matrix.setTranslate(SkIntToScalar(10), SkIntToScalar(10));
		canvas->drawTextOnPath((void*) utf16, sizeof(utf16), path, &matrix, paint);
		canvas->translate(0, SkIntToScalar(20));
		canvas->drawTextOnPath((void*) utf16simple, sizeof(utf16simple), path, &matrix, paint);
        canvas->restore();
        
        canvas->translate(0, SkIntToScalar(60));
		paint.setTextEncoding(SkPaint::kUTF8_TextEncoding);
        canvas->restore();
    }
    
    /*
./SkColorFilter.h:25:class SkColorFilter : public SkFlattenable { -- abstract
    static SkColorFilter* CreatXfermodeFilter() *** untested ***
    static SkColorFilter* CreatePorterDuffFilter() *** untested ***
    static SkColorFilter* CreateLightingFilter() -- tested
./SkDrawLooper.h:9:class SkDrawLooper : public SkFlattenable { -- virtually abstract
    ./SkBlurDrawLooper.h:9:class SkBlurDrawLooper : public SkDrawLooper { *** untested ***
./SkMaskFilter.h:41:class SkMaskFilter : public SkFlattenable { -- abstract chmod +w .h
    ./SkEmbossMaskFilter.h:27:class SkEmbossMaskFilter : public SkMaskFilter { -- tested
./SkPathEffect.h:33:class SkPathEffect : public SkFlattenable { -- abstract
    ./Sk1DPathEffect.h:27:class Sk1DPathEffect : public SkPathEffect { -- abstract
        ./Sk1DPathEffect.h:48:class SkPath1DPathEffect : public Sk1DPathEffect { -- tested
    ./Sk2DPathEffect.h:25:class Sk2DPathEffect : public SkPathEffect { *** untested ***
    ./SkCornerPathEffect.h:28:class SkCornerPathEffect : public SkPathEffect { *** untested ***
    ./SkDashPathEffect.h:27:class SkDashPathEffect : public SkPathEffect {
    ./SkDiscretePathEffect.h:27:class SkDiscretePathEffect : public SkPathEffect {
    ./SkPaint.h:760:class SkStrokePathEffect : public SkPathEffect {
    ./SkPathEffect.h:58:class SkPairPathEffect : public SkPathEffect {
        ./SkPathEffect.h:78:class SkComposePathEffect : public SkPairPathEffect {
        ./SkPathEffect.h:114:class SkSumPathEffect : public SkPairPathEffect {
./SkRasterizer.h:29:class SkRasterizer : public SkFlattenable {
    ./SkLayerRasterizer.h:27:class SkLayerRasterizer : public SkRasterizer {
./SkShader.h:36:class SkShader : public SkFlattenable {
    ./SkColorFilter.h:59:class SkFilterShader : public SkShader {
    ./SkColorShader.h:26:class SkColorShader : public SkShader {
    ./SkShaderExtras.h:31:class SkComposeShader : public SkShader {
    ./SkTransparentShader.h:23:class SkTransparentShader : public SkShader {
./SkUnitMapper.h:24:class SkUnitMapper : public SkFlattenable {
    ./SkUnitMapper.h:33:class SkDiscreteMapper : public SkUnitMapper {
    ./SkUnitMapper.h:51:class SkFlipCosineMapper : public SkUnitMapper {
./SkXfermode.h:32:class SkXfermode : public SkFlattenable {
    ./SkAvoidXfermode.h:28:class SkAvoidXfermode : public SkXfermode { *** not done *** chmod +w .h .cpp
    ./SkXfermode.h:54:class SkProcXfermode : public SkXfermode {
    */
    
    /*
./SkBlurMaskFilter.h:25:class SkBlurMaskFilter {
    chmod +w SkBlurMaskFilter.cpp
./SkGradientShader.h:30:class SkGradientShader {
    */
        // save layer, bounder, looper
        // matrix
        // clip /path/region
        // bitmap proc shader ?

/* untested:
SkCornerPathEffect.h:28:class SkCornerPathEffect : public SkPathEffect {
*/
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) 
    {
        fClickPt.set(x, y);
        this->inval(NULL);
        return this->INHERITED::onFindClickHandler(x, y);
    }
    
    SkPathEffect* pathEffectTest()
    {
        static const int gXY[] = { 1, 0, 0, -1, 2, -1, 3, 0, 2, 1, 0, 1 };
        SkScalar gPhase = 0;
        SkPath path;
        path.moveTo(SkIntToScalar(gXY[0]), SkIntToScalar(gXY[1]));
        for (unsigned i = 2; i < SK_ARRAY_COUNT(gXY); i += 2)
            path.lineTo(SkIntToScalar(gXY[i]), SkIntToScalar(gXY[i+1]));
        path.close();
        path.offset(SkIntToScalar(-6), 0);
        SkPathEffect* outer = new SkPath1DPathEffect(path, SkIntToScalar(12), 
            gPhase, SkPath1DPathEffect::kRotate_Style);
        SkPathEffect* inner = new SkDiscretePathEffect(SkIntToScalar(2), 
            SkIntToScalar(1)/10); // SkCornerPathEffect(SkIntToScalar(2));
        SkPathEffect* result = new SkComposePathEffect(outer, inner);
        outer->unref();
        inner->unref();
        return result;
    }
    
    SkPathEffect* pathEffectTest2() // unsure this works (has no visible effect)
    {
        SkPathEffect* outer = new SkStrokePathEffect(SkIntToScalar(4), 
            SkPaint::kStroke_Style, SkPaint::kMiter_Join, SkPaint::kButt_Cap);
        static const SkScalar intervals[] = {SkIntToScalar(1), SkIntToScalar(2),
            SkIntToScalar(2), SkIntToScalar(1)};
        SkPathEffect* inner = new SkDashPathEffect(intervals, 
            sizeof(intervals) / sizeof(intervals[0]), 0);
        SkPathEffect* result = new SkSumPathEffect(outer, inner);
        outer->unref();
        inner->unref();
        return result;
    }
    
    SkShader* shaderTest()
    {
        SkPoint pts[] = {0, 0, SkIntToScalar(100), 0 };
        SkColor colors[] = { SK_ColorRED, SK_ColorBLUE };
        SkShader* shaderA = SkGradientShader::CreateLinear(pts, colors, NULL, 
            2, SkShader::kClamp_TileMode);
        pts[1].set(0, SkIntToScalar(100));
        SkColor colors2[] = {SK_ColorBLACK,  SkColorSetARGB(0x80, 0, 0, 0)};
        SkShader* shaderB = SkGradientShader::CreateLinear(pts, colors2, NULL, 
            2, SkShader::kClamp_TileMode);
        SkXfermode* mode = SkPorterDuff::CreateXfermode(SkPorterDuff::kDstIn_Mode);
        SkShader* result = new SkComposeShader(shaderA, shaderB, mode);
        shaderA->unref();
        shaderB->unref();
        mode->unref();
        return result;
    }

    virtual void startTest() {
		SkImageDecoder::DecodeFile("/Users/caryclark/Desktop/bugcirc.gif", &fBug);
		SkImageDecoder::DecodeFile("/Users/caryclark/Desktop/tbcirc.gif", &fTb);
		SkImageDecoder::DecodeFile("/Users/caryclark/Desktop/05psp04.gif", &fTx);
	}

    void drawRaster(SkCanvas* canvas) 
    {
        for (int index = 0; index < SK_ARRAY_COUNT(gRastProcs); index++)
            drawOneRaster(canvas);
    }
    
    void drawOneRaster(SkCanvas* canvas)
    {        
        canvas->save();
//        canvas->scale(SK_Scalar1*2, SK_Scalar1*2, 0, 0);

        SkScalar    x = SkIntToScalar(20);
        SkScalar    y = SkIntToScalar(40);
        SkPaint     paint;
        
        paint.setAntiAlias(true);
        paint.setTextSize(SkIntToScalar(48));
        paint.setTypeface(SkTypeface::Create("sans-serif", SkTypeface::kBold));

        SkString str("GOOGLE");

        for (int i = 0; i < SK_ARRAY_COUNT(gRastProcs); i++)
        {
            apply_shader(&paint, i);
            
          //  paint.setMaskFilter(NULL);
          //  paint.setColor(SK_ColorBLACK);

#if 01
            int index = i % SK_ARRAY_COUNT(gLightingColors);
            paint.setColorFilter(SkColorFilter::CreateLightingFilter(
                                    gLightingColors[index].fMul,
                                    gLightingColors[index].fAdd))->unref();
#endif
            
            canvas->drawText(str.c_str(), str.size(), x, y, paint);
            SkRect  oval = { x, y - SkIntToScalar(40), x + SkIntToScalar(40), y };
            paint.setStyle(SkPaint::kStroke_Style);
            canvas->drawOval(oval, paint);
            paint.setStyle(SkPaint::kFill_Style);
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
            paint.setShader(s)->unref();
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

private:
    SkPoint fClickPt;
    SkBitmap fBug, fTb, fTx;
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new DemoView; }
static SkViewRegister reg(MyFactory);

