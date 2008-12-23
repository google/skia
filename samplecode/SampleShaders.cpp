#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkPath.h"
#include "SkPorterDuff.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkXfermode.h"
#include "SkComposeShader.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTransparentShader.h"
#include "SkTypeface.h"

static SkShader* make_bitmapfade(const SkBitmap& bm)
{
    SkPoint pts[2];
    SkColor colors[2];

    pts[0].set(0, 0);
    pts[1].set(0, SkIntToScalar(bm.height()));
    colors[0] = SK_ColorBLACK;
    colors[1] = SkColorSetARGB(0, 0, 0, 0);
    SkShader* shaderA = SkGradientShader::CreateLinear(pts, colors, NULL, 2, SkShader::kClamp_TileMode);

    SkShader* shaderB = SkShader::CreateBitmapShader(bm,
                        SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);

    SkXfermode* mode = SkPorterDuff::CreateXfermode(SkPorterDuff::kDstIn_Mode);

    SkShader* shader = new SkComposeShader(shaderB, shaderA, mode);
    shaderA->unref();
    shaderB->unref();
    mode->unref();
    
    return shader;
}

class ShaderView : public SkView {
public:
    SkShader*   fShader;
    SkBitmap    fBitmap;

	ShaderView()
    {
        SkImageDecoder::DecodeFile("/cover.png", &fBitmap);

        SkPoint pts[2];
        SkColor colors[2];
        
        pts[0].set(0, 0);
        pts[1].set(SkIntToScalar(100), 0);
        colors[0] = SK_ColorRED;
        colors[1] = SK_ColorBLUE;
        SkShader* shaderA = SkGradientShader::CreateLinear(pts, colors, NULL, 2, SkShader::kClamp_TileMode);
        
        pts[0].set(0, 0);
        pts[1].set(0, SkIntToScalar(100));
        colors[0] = SK_ColorBLACK;
        colors[1] = SkColorSetARGB(0x80, 0, 0, 0);
        SkShader* shaderB = SkGradientShader::CreateLinear(pts, colors, NULL, 2, SkShader::kClamp_TileMode);
        
        SkXfermode* mode = SkPorterDuff::CreateXfermode(SkPorterDuff::kDstIn_Mode);

        fShader = new SkComposeShader(shaderA, shaderB, mode);
        shaderA->unref();
        shaderB->unref();
        mode->unref();
    }
    virtual ~ShaderView()
    {
        fShader->safeUnref();
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt)
    {
            if (SampleCode::TitleQ(*evt))
            {
                SampleCode::TitleR(evt, "Shaders");
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
        
        canvas->drawBitmap(fBitmap, 0, 0);
        
        {
            SkIRect src;
            SkRect  dst;
            
            src.set(20, 50, 120, 70);
            dst.set(src);
            dst.offset(SkIntToScalar(300), 0);

            canvas->drawBitmapRect(fBitmap, &src, dst);
        }

        canvas->translate(SkIntToScalar(80), SkIntToScalar(80));
        
        SkPaint paint;
        SkRect  r;

        paint.setColor(SK_ColorGREEN);
        canvas->drawRectCoords(0, 0, SkIntToScalar(100), SkIntToScalar(100), paint);
        paint.setShader(fShader);
        canvas->drawRectCoords(0, 0, SkIntToScalar(100), SkIntToScalar(100), paint);

        canvas->translate(SkIntToScalar(110), 0);

        r.set(0, 0, SkIntToScalar(fBitmap.width()), SkIntToScalar(fBitmap.height()));

        paint.setShader(NULL);
        canvas->drawRect(r, paint);
        paint.setShader(make_bitmapfade(fBitmap))->unref();
        canvas->drawRect(r, paint);
        
        paint.setShader(new SkTransparentShader)->unref();
        canvas->drawRect(r, paint);
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) 
    {
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

static SkView* MyFactory() { return new ShaderView; }
static SkViewRegister reg(MyFactory);

