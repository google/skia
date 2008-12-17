#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkMovie.h"
#include "SkTime.h"
#include <new>

class AnimGifView : public SkView {
    SkMovie*    fMovie;
public:
	AnimGifView() {
        fMovie = SkMovie::DecodeFile("/skimages/dollarblk.gif");
    }
    
    virtual ~AnimGifView() {
        fMovie->safeUnref();
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Animated Gif");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
        
        if (fMovie) {
            if (fMovie->duration()) {
                fMovie->setTime(SkTime::GetMSecs() % fMovie->duration());
            } else {
                fMovie->setTime(0);
            }
            canvas->drawBitmap(fMovie->bitmap(), SkIntToScalar(20),
                               SkIntToScalar(20));
            this->inval(NULL);
        }
    }
    
private:
    SkRect      fClip;
    SkIPoint*   fPoints;
    SkPath      fPath;
    int         fPtCount;

    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new AnimGifView; }
static SkViewRegister reg(MyFactory);

