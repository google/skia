/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "gm.h"
#include "SkRandom.h"

namespace skiagm {

#define W   400
#define H   400
#define N   100

static const SkScalar SW = SkIntToScalar(W);
static const SkScalar SH = SkIntToScalar(H);

class StrokeRectGM : public GM {
public:
    StrokeRectGM() {}
    
protected:
    virtual SkString onShortName() {
        return SkString("strokerects");
    }

    virtual SkISize onISize() {
        return make_isize(W*2, H*2);
    }

    static void rnd_rect(SkRect* r, SkRandom& rand) {
        SkScalar x = rand.nextUScalar1() * W;
        SkScalar y = rand.nextUScalar1() * H;
        SkScalar w = rand.nextUScalar1() * (W >> 2);
        SkScalar h = rand.nextUScalar1() * (H >> 2);
        
        r->set(x, y, x + w, y + h);
        r->offset(-w/2 + rand.nextSScalar1(), -h/2 +  + rand.nextSScalar1());
    }

    virtual void onDraw(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);

        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);

        for (int y = 0; y < 2; y++) {
            paint.setAntiAlias(!!y);
            for (int x = 0; x < 2; x++) {
                paint.setStrokeWidth(x * SkIntToScalar(3));

                SkAutoCanvasRestore acr(canvas, true);
                canvas->translate(SW * x, SH * y);
                canvas->clipRect(SkRect::MakeLTRB(
                        SkIntToScalar(2), SkIntToScalar(2)
                        , SW - SkIntToScalar(2), SH - SkIntToScalar(2)
                ));

                SkRandom rand;
                for (int i = 0; i < N; i++) {
                    SkRect r;
                    rnd_rect(&r, rand);
                    canvas->drawRect(r, paint);
                }
            }
        }
    }
    
private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new StrokeRectGM; }
static GMRegistry reg(MyFactory);

}

