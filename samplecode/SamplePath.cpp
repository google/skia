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
#include "SkTypeface.h"

class PathView : public SkView {
public:
    int fDStroke, fStroke, fMinStroke, fMaxStroke;
    SkPath fPath[6];
    bool fShowHairline;
    
	PathView()
    {
        fShowHairline = false;
        
        fDStroke = 1;
        fStroke = 10;
        fMinStroke = 10;
        fMaxStroke = 180;

        const int V = 85;
        
        fPath[0].moveTo(SkIntToScalar(40), SkIntToScalar(70));
        fPath[0].lineTo(SkIntToScalar(70), SkIntToScalar(70) + SK_Scalar1/1);
        fPath[0].lineTo(SkIntToScalar(110), SkIntToScalar(70));
        
        fPath[1].moveTo(SkIntToScalar(40), SkIntToScalar(70));
        fPath[1].lineTo(SkIntToScalar(70), SkIntToScalar(70) - SK_Scalar1/1);
        fPath[1].lineTo(SkIntToScalar(110), SkIntToScalar(70));
        
        fPath[2].moveTo(SkIntToScalar(V), SkIntToScalar(V));
        fPath[2].lineTo(SkIntToScalar(50), SkIntToScalar(V));
        fPath[2].lineTo(SkIntToScalar(50), SkIntToScalar(50));
        
        fPath[3].moveTo(SkIntToScalar(50), SkIntToScalar(50));
        fPath[3].lineTo(SkIntToScalar(50), SkIntToScalar(V));
        fPath[3].lineTo(SkIntToScalar(V), SkIntToScalar(V));
        
        fPath[4].moveTo(SkIntToScalar(50), SkIntToScalar(50));
        fPath[4].lineTo(SkIntToScalar(50), SkIntToScalar(V));
        fPath[4].lineTo(SkIntToScalar(52), SkIntToScalar(50));
        
        fPath[5].moveTo(SkIntToScalar(52), SkIntToScalar(50));
        fPath[5].lineTo(SkIntToScalar(50), SkIntToScalar(V));
        fPath[5].lineTo(SkIntToScalar(50), SkIntToScalar(50));
    }
    
    virtual ~PathView()
    {
    }
    
    void nextStroke()
    {
        fStroke += fDStroke;
        if (fStroke > fMaxStroke || fStroke < fMinStroke)
            fDStroke = -fDStroke;
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt)
    {
        if (SampleCode::TitleQ(*evt))
        {
            SampleCode::TitleR(evt, "Paths");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas)
    {
        canvas->drawColor(0xFFDDDDDD);
//        canvas->drawColor(SK_ColorWHITE);
    }
    
    void drawPath(SkCanvas* canvas, const SkPath& path, SkPaint::Join j)
    {
        SkPaint paint;
        
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeJoin(j);
        paint.setStrokeWidth(SkIntToScalar(fStroke));

        if (fShowHairline)
        {
            SkPath  fill;
            
            paint.getFillPath(path, &fill);            
            paint.setStrokeWidth(0);
            canvas->drawPath(fill, paint);
        }
        else
            canvas->drawPath(path, paint);
        
        paint.setColor(SK_ColorRED);
        paint.setStrokeWidth(0);
        canvas->drawPath(path, paint);
    }
    
    virtual void onDraw(SkCanvas* canvas)
    {
        this->drawBG(canvas);
        
        canvas->translate(SkIntToScalar(50), SkIntToScalar(50));

        static const SkPaint::Join gJoins[] = {
            SkPaint::kBevel_Join,
            SkPaint::kMiter_Join,
            SkPaint::kRound_Join
        };

        for (int i = 0; i < SK_ARRAY_COUNT(gJoins); i++)
        {
            canvas->save();
            for (int j = 0; j < SK_ARRAY_COUNT(fPath); j++)
            {
                this->drawPath(canvas, fPath[j], gJoins[i]);
                canvas->translate(SkIntToScalar(200), 0);
            }
            canvas->restore();
            
            canvas->translate(0, SkIntToScalar(200));
        }
        
        this->nextStroke();
        this->inval(NULL);
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) 
    {
        fShowHairline = !fShowHairline;
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

static SkView* MyFactory() { return new PathView; }
static SkViewRegister reg(MyFactory);

