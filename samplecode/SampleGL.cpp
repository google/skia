#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "Sk64.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkKernel33MaskFilter.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"
#include "SkXfermode.h"

// effects
#include "SkGradientShader.h"
#include "SkShaderExtras.h"
#include "SkUnitMappers.h"

#include "SkStream.h"
#include "SkXMLParser.h"

#include "SkGLCanvas.h"

#include <AGL/agl.h>
#include <OpenGL/gl.h>

extern void* gSampleWind;

static void makebm(SkBitmap* bm, SkBitmap::Config config, int w, int h)
{
    bm->setConfig(config, w, h);
    bm->allocPixels();
    bm->eraseColor(0);
    
    SkCanvas    canvas(*bm);
    SkPoint     pts[] = { 0, 0, SkIntToScalar(w), SkIntToScalar(h) };
    SkColor     colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
    SkScalar    pos[] = { 0, SK_Scalar1/2, SK_Scalar1 };
    SkPaint     paint;
    
    SkUnitMapper*   um = NULL;    
    
//    um = new SkCosineMapper;
    //    um = new SkDiscreteMapper(12);
    
    SkAutoUnref au(um);

    paint.setAntiAlias(true);
    paint.setShader(SkGradientShader::CreateLinear(pts, colors, pos,
            SK_ARRAY_COUNT(colors), SkShader::kClamp_TileMode, um))->unref();
    
    SkRect r;
    r.set(0, 0, SkIntToScalar(w), SkIntToScalar(h));
    canvas.drawOval(r, paint);
}

static void premulBitmap(const SkBitmap& bm) {
    for (int y = 0; y < bm.height(); y++) {
        SkPMColor* p = bm.getAddr32(0, y);
        for (int x = 0; x < bm.width(); x++) {
            SkPMColor c = *p;
            unsigned a = SkGetPackedA32(c);
            unsigned r = SkGetPackedR32(c);
            unsigned g = SkGetPackedG32(c);
            unsigned b = SkGetPackedB32(c);
            
            unsigned scale = SkAlpha255To256(a);
            r = SkAlphaMul(r, scale);
            g = SkAlphaMul(g, scale);
            b = SkAlphaMul(b, scale);
            *p++ = SkPackARGB32(a, r, g, b);
        }
    }
}

class GLView : public SkView {
public:
    AGLContext fCtx;
    SkBitmap    fOffscreen;
    SkBitmap    fTexture[3];

	GLView() {
        makebm(&fTexture[0], SkBitmap::kARGB_8888_Config, 64, 100);
        makebm(&fTexture[1], SkBitmap::kRGB_565_Config, 64, 100);
        makebm(&fTexture[2], SkBitmap::kARGB_4444_Config, 64, 100);

        GLint major, minor;
        
        aglGetVersion(&major, &minor);
        SkDebugf("---- version %d %d\n", major, minor);
        
        GLint attr[] = {
            AGL_RGBA,
            AGL_DEPTH_SIZE, 32,
            AGL_OFFSCREEN,
            AGL_NONE
        };

        SkDebugf("------ attr %p %d\n", attr, sizeof(attr));
        AGLPixelFormat format = aglCreatePixelFormat(attr);
        SkDebugf("----- format %p\n", format);
        fCtx = aglCreateContext(format, 0);
        SkDebugf("----- context %p\n", fCtx);
        GLboolean success;  //= aglSetWindowRef(fCtx, (WindowRef)gSampleWind);
//        SkDebugf("----- aglSetWindowRef %d\n", success);

        aglEnable(fCtx, GL_BLEND);
        aglEnable(fCtx, GL_LINE_SMOOTH);
        aglEnable(fCtx, GL_POINT_SMOOTH);
        aglEnable(fCtx, GL_POLYGON_SMOOTH);

        fOffscreen.setConfig(SkBitmap::kARGB_8888_Config, 300, 300);
        fOffscreen.allocPixels();
        
        success = aglSetOffScreen(fCtx,
                                  fOffscreen.width(),
                                  fOffscreen.height(),
                                  fOffscreen.rowBytes(),
                                  fOffscreen.getPixels());
        GLenum err = aglGetError();
        SkDebugf("---- setoffscreen %d %d %s\n", success, err, aglErrorString(err));
        
        aglSetCurrentContext(fCtx);
        glOrtho(0, fOffscreen.width(),
                fOffscreen.height(), 0,
                -1, 1);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);

        glEnable(GL_TEXTURE_2D);
}
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "GL");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
        
        SkGLCanvas c(fOffscreen.width(), fOffscreen.height());

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        SkPaint p;
        
        p.setAntiAlias(true);

        c.drawColor(SK_ColorWHITE);

        p.setColor(SK_ColorRED);
        c.drawCircle(SkIntToScalar(40), SkIntToScalar(40), SkIntToScalar(20), p);
        
        p.setColor(SK_ColorGREEN);
        p.setStrokeWidth(SkIntToScalar(6));
        p.setStrokeCap(SkPaint::kRound_Cap);
        c.drawLine(SkIntToScalar(10), SkIntToScalar(10), SkIntToScalar(40), SkIntToScalar(50), p);
        
      //  c.scale(SkIntToScalar(3)/2, SkIntToScalar(3)/2);
        p.setColor(0x880000FF);
        c.drawCircle(SkIntToScalar(40), SkIntToScalar(40), SkIntToScalar(20), p);

        for (int i = 0; i < SK_ARRAY_COUNT(fTexture); i++) {
            c.drawBitmap(fTexture[i], SkIntToScalar(10), SkIntToScalar(100), NULL);
            c.translate(SkIntToScalar(fTexture[i].width()), 0);
        }
        p.setColor(SK_ColorBLUE);
        c.drawRectCoords(SkIntToScalar(10), SkIntToScalar(100),
                         SkIntToScalar(10+fTexture[0].width()),
                         SkIntToScalar(100+fTexture[0].height()),
                         p);

        ////////
        glFlush();
        premulBitmap(fOffscreen);
        canvas->drawBitmap(fOffscreen, SkIntToScalar(10), SkIntToScalar(10), NULL);
    }
    
private:

    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new GLView; }
static SkViewRegister reg(MyFactory);

