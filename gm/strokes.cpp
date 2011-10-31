
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "gm.h"
#include "SkRandom.h"

namespace skiagm {

#define W   400
#define H   400
#define N   50

static const SkScalar SW = SkIntToScalar(W);
static const SkScalar SH = SkIntToScalar(H);

static void rnd_rect(SkRect* r, SkPaint* paint, SkRandom& rand) {
    SkScalar x = rand.nextUScalar1() * W;
    SkScalar y = rand.nextUScalar1() * H;
    SkScalar w = rand.nextUScalar1() * (W >> 2);
    SkScalar h = rand.nextUScalar1() * (H >> 2);
    SkScalar hoffset = rand.nextSScalar1();
    SkScalar woffset = rand.nextSScalar1();
    
    r->set(x, y, x + w, y + h);
    r->offset(-w/2 + woffset, -h/2 + hoffset);
    
    paint->setColor(rand.nextU());
    paint->setAlpha(0xFF);
}

    
class StrokesGM : public GM {
public:
    StrokesGM() {}
    
protected:
    virtual SkString onShortName() {
        return SkString("strokes_round");
    }
    
    virtual SkISize onISize() {
        return make_isize(W, H*2);
    }
    
    virtual void onDraw(SkCanvas* canvas) {        
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkIntToScalar(9)/2);
        
        for (int y = 0; y < 2; y++) {
            paint.setAntiAlias(!!y);
            SkAutoCanvasRestore acr(canvas, true);
            canvas->translate(0, SH * y);
            canvas->clipRect(SkRect::MakeLTRB(
                                              SkIntToScalar(2), SkIntToScalar(2)
                                              , SW - SkIntToScalar(2), SH - SkIntToScalar(2)
                                              ));
            
            SkRandom rand;
            for (int i = 0; i < N; i++) {
                SkRect r;
                rnd_rect(&r, &paint, rand);
                canvas->drawOval(r, paint);
                rnd_rect(&r, &paint, rand);
                canvas->drawRoundRect(r, r.width()/4, r.height()/4, paint);
                rnd_rect(&r, &paint, rand);
            }
        }
    }
    
private:
    typedef GM INHERITED;
};

class Strokes2GM : public GM {
    SkPath fPath;
public:
    Strokes2GM() {
        SkRandom rand;
        fPath.moveTo(0, 0);
        for (int i = 0; i < 13; i++) {
            SkScalar x = rand.nextUScalar1() * (W >> 1);
            SkScalar y = rand.nextUScalar1() * (H >> 1);
            fPath.lineTo(x, y);
        }
    }
    
protected:
    virtual SkString onShortName() {
        return SkString("strokes_poly");
    }
    
    virtual SkISize onISize() {
        return make_isize(W, H*2);
    }
    
    static void rotate(SkScalar angle, SkScalar px, SkScalar py, SkCanvas* canvas) {
        SkMatrix matrix;
        matrix.setRotate(angle, px, py);
        canvas->concat(matrix);
    }

    virtual void onDraw(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
        
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkIntToScalar(9)/2);
        
        for (int y = 0; y < 2; y++) {
            paint.setAntiAlias(!!y);
            SkAutoCanvasRestore acr(canvas, true);
            canvas->translate(0, SH * y);
            canvas->clipRect(SkRect::MakeLTRB(SkIntToScalar(2),
                                              SkIntToScalar(2),
                                              SW - SkIntToScalar(2),
                                              SH - SkIntToScalar(2)));
            
            SkRandom rand;
            for (int i = 0; i < N/2; i++) {
                SkRect r;
                rnd_rect(&r, &paint, rand);
                rotate(SkIntToScalar(15), SW/2, SH/2, canvas);
                canvas->drawPath(fPath, paint);
            }
        }
    }
    
private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new StrokesGM; }
static GMRegistry reg(MyFactory);

static GM* MyFactory2(void*) { return new Strokes2GM; }
static GMRegistry reg2(MyFactory2);

}

