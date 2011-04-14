#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkCamera.h"
#include "SkEmbossMaskFilter.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkRandom.h"
#include "SkImageDecoder.h"

class CameraView : public SkView {
    SkTDArray<SkShader*> fShaders;
    int     fShaderIndex;
    bool    fFrontFace;
public:
	CameraView() {
        fRX = fRY = fRZ = 0;
        fShaderIndex = 0;
        fFrontFace = false;

        for (int i = 0;; i++) {
            SkString str;
            str.printf("/skimages/elephant%d.jpeg", i);
            SkBitmap bm;
            if (SkImageDecoder::DecodeFile(str.c_str(), &bm)) {
                SkShader* s = SkShader::CreateBitmapShader(bm,
                                                           SkShader::kClamp_TileMode,
                                                           SkShader::kClamp_TileMode);
                
                SkRect src = { 0, 0, SkIntToScalar(bm.width()), SkIntToScalar(bm.height()) };
                SkRect dst = { -150, -150, 150, 150 };
                SkMatrix matrix;
                matrix.setRectToRect(src, dst, SkMatrix::kFill_ScaleToFit);
                s->setLocalMatrix(matrix);
                *fShaders.append() = s;
            } else {
                break;
            }
        }
    }
    
    virtual ~CameraView() {
        fShaders.unrefAll();
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt))
        {
            SampleCode::TitleR(evt, "Camera");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

        canvas->translate(this->width()/2, this->height()/2);

        Sk3DView    view;
        view.rotateX(fRX);
        view.rotateY(fRY);
        view.applyToCanvas(canvas);
        
        SkPaint paint;
        SkScalar rad = SkIntToScalar(50);
        SkScalar dim = rad*2;
        if (fShaders.count() > 0) {
            bool frontFace = view.dotWithNormal(0, 0, SK_Scalar1) < 0;
            if (frontFace != fFrontFace) {
                fFrontFace = frontFace;
                fShaderIndex = (fShaderIndex + 1) % fShaders.count();
            }
        
            paint.setAntiAlias(true);
            paint.setShader(fShaders[fShaderIndex]);
#if 0
            canvas->drawCircle(0, 0, rad, paint);
            canvas->drawCircle(-dim, -dim, rad, paint);
            canvas->drawCircle(-dim,  dim, rad, paint);
            canvas->drawCircle( dim, -dim, rad, paint);
            canvas->drawCircle( dim,  dim, rad, paint);
#else
            SkRect r = { -150, -150, 150, 150 };
            canvas->drawRoundRect(r, 30, 30, paint);
#endif
        }
        
        fRY += SampleCode::GetAnimSecondsDelta() * 90;
        if (fRY >= SkIntToScalar(360)) {
            fRY = 0;
        }
        this->inval(NULL);
    }

private:
    SkScalar fRX, fRY, fRZ;
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new CameraView; }
static SkViewRegister reg(MyFactory);

