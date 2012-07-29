/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkSurface.h"
#include "SkCanvas.h"

static void drawContents(SkSurface* surface, SkColor fillC) {
    SkSize size = SkSize::Make(surface->width(), surface->height());
    SkAutoTUnref<SkCanvas> canvas(surface->newCanvas());

    SkScalar stroke = size.fWidth / 10;
    SkScalar radius = (size.fWidth - stroke) / 2;

    SkPaint paint;
    
    paint.setAntiAlias(true);
    paint.setColor(fillC);
    canvas->drawCircle(size.fWidth/2, size.fHeight/2, radius, paint);
    
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(stroke);
    paint.setColor(SK_ColorBLACK);
    canvas->drawCircle(size.fWidth/2, size.fHeight/2, radius, paint);
}

static void test_surface(SkCanvas* canvas, SkSurface* surf) {
    drawContents(surf, SK_ColorRED);
    SkImage* imgR = surf->newImageShapshot();

    drawContents(surf, SK_ColorGREEN);
    SkImage* imgG = surf->newImageShapshot();

    drawContents(surf, SK_ColorBLUE);

    imgR->draw(canvas, 0, 0, NULL);
    imgG->draw(canvas, 0, 80, NULL);
    surf->draw(canvas, 0, 160, NULL);

    imgG->unref();
    imgR->unref();
}

class ImageGM : public skiagm::GM {
    void*   fBuffer;
    SkSize  fSize;
    enum {
        W = 64,
        H = 64,
        RB = W * 4 + 8,
    };
public:
    ImageGM() {
        fBuffer = sk_malloc_throw(RB * H);
        fSize.set(SkIntToScalar(W), SkIntToScalar(H));
    }
    
    virtual ~ImageGM() {
        sk_free(fBuffer);
    }
        
    
protected:
    virtual SkString onShortName() {
        return SkString("image");
    }
    
    virtual SkISize onISize() {
        return SkISize::Make(640, 480);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        SkImage::Info info;

        info.fWidth = W;
        info.fHeight = H;
        info.fColorType = SkImage::kPMColor_ColorType;
        info.fAlphaType = SkImage::kPremul_AlphaType;
        SkAutoTUnref<SkSurface> surf0(SkSurface::NewRasterDirect(info, NULL, fBuffer, RB));
        SkAutoTUnref<SkSurface> surf1(SkSurface::NewRaster(info, NULL));
        SkAutoTUnref<SkSurface> surf2(SkSurface::NewPicture(info.fWidth, info.fHeight));

        test_surface(canvas, surf0);
        canvas->translate(80, 0);
        test_surface(canvas, surf1);
        canvas->translate(80, 0);
        test_surface(canvas, surf2);
    }
    
private:
    typedef skiagm::GM INHERITED;
};
    
//////////////////////////////////////////////////////////////////////////////

static skiagm::GM* MyFactory(void*) { return new ImageGM; }
static skiagm::GMRegistry reg(MyFactory);
    
