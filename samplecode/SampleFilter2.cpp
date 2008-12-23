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
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"

static const char* gNames[] = {
    "/skimages/background_01.png"
};

class Filter2View : public SkView {
public:
    SkBitmap*   fBitmaps;
    int         fBitmapCount;
    int         fCurrIndex;

	Filter2View() {
        fBitmapCount = SK_ARRAY_COUNT(gNames)*2;
        fBitmaps = new SkBitmap[fBitmapCount];
        
        for (int i = 0; i < fBitmapCount/2; i++) {
            SkImageDecoder::DecodeFile(gNames[i], &fBitmaps[i],
                                       SkBitmap::kARGB_8888_Config,
                                       SkImageDecoder::kDecodePixels_Mode);
        }
        for (int i = fBitmapCount/2; i < fBitmapCount; i++) {
            SkImageDecoder::DecodeFile(gNames[i-fBitmapCount/2], &fBitmaps[i],
                                       SkBitmap::kRGB_565_Config,
                                       SkImageDecoder::kDecodePixels_Mode);
        }
        fCurrIndex = 0;
    }
    
    virtual ~Filter2View() {
        delete[] fBitmaps;
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SkString str("Filter/Dither ");
            str.append(gNames[fCurrIndex]);
            SampleCode::TitleR(evt, str.c_str());
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
//        canvas->drawColor(0xFFDDDDDD);
        canvas->drawColor(SK_ColorGRAY);
//        canvas->drawColor(SK_ColorWHITE);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
        
        canvas->translate(SkIntToScalar(10), SkIntToScalar(50));
        
        const SkScalar W = SkIntToScalar(fBitmaps[0].width() + 1);
        const SkScalar H = SkIntToScalar(fBitmaps[0].height() + 1);
        SkPaint paint;
        
        const SkScalar scale = SkFloatToScalar(0.897917f);
        canvas->scale(SK_Scalar1, scale);

        for (int k = 0; k < 2; k++) {
            paint.setFilterBitmap(k == 1);
            for (int j = 0; j < 2; j++) {
                paint.setDither(j == 1);
                for (int i = 0; i < fBitmapCount; i++) {
                    SkScalar x = (k * fBitmapCount + j) * W;
                    SkScalar y = i * H;
                    x = SkIntToScalar(SkScalarRound(x));
                    y = SkIntToScalar(SkScalarRound(y));
                    canvas->drawBitmap(fBitmaps[i], x, y, &paint);
                    if (i == 0) {
                        SkPaint p;
                        p.setAntiAlias(true);
                        p.setTextAlign(SkPaint::kCenter_Align);
                        p.setTextSize(SkIntToScalar(18));
                        SkString s("dither=");
                        s.appendS32(paint.isDither());
                        s.append(" filter=");
                        s.appendS32(paint.isFilterBitmap());
                        canvas->drawText(s.c_str(), s.size(), x + W/2,
                                         y - p.getTextSize(), p);
                    }
                    if (k+j == 2) {
                        SkPaint p;
                        p.setAntiAlias(true);
                        p.setTextSize(SkIntToScalar(18));
                        SkString s;
                        s.append(" depth=");
                        s.appendS32(fBitmaps[i].config() == SkBitmap::kRGB_565_Config ? 16 : 32);
                        canvas->drawText(s.c_str(), s.size(), x + W + SkIntToScalar(4),
                                         y + H/2, p);
                    }
                }
            }
        }
    }
    
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new Filter2View; }
static SkViewRegister reg(MyFactory);

