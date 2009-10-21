#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"

#include "SkAnimator.h"
#include "SkStream.h"

class SkAnimatorView : public SkView {
public:
    SkAnimatorView();
    virtual ~SkAnimatorView();

    void setURIBase(const char dir[]);

    SkAnimator* getAnimator() const { return fAnimator; }
    
    bool    decodeFile(const char path[]);
    bool    decodeMemory(const void* buffer, size_t size);
    bool    decodeStream(SkStream* stream);
    
protected:
    // overrides
    virtual void onDraw(SkCanvas*);
    
private:
    SkString fBaseURI;
    SkAnimator* fAnimator;
    
    typedef SkView INHERITED;
};

SkAnimatorView::SkAnimatorView() : fAnimator(NULL) {
}

SkAnimatorView::~SkAnimatorView() {
    delete fAnimator;
}

void SkAnimatorView::setURIBase(const char dir[]) {
    fBaseURI.set(dir);
}

bool SkAnimatorView::decodeFile(const char path[]) {
    SkFILEStream* is = new SkFILEStream(path);
    SkAutoUnref aur(is);
    return is->isValid() && this->decodeStream(is);
}

bool SkAnimatorView::decodeMemory(const void* buffer, size_t size) {
    SkMemoryStream* is = new SkMemoryStream(buffer, size);
    SkAutoUnref aur(is);
    return this->decodeStream(is);
}

bool SkAnimatorView::decodeStream(SkStream* stream) {
    delete fAnimator;
    fAnimator = new SkAnimator;
    fAnimator->setURIBase(fBaseURI.c_str());
    if (!fAnimator->decodeStream(stream)) {
        delete fAnimator;
        fAnimator = NULL;
        return false;
    }
    return true;
}

#include "SkTime.h"

void SkAnimatorView::onDraw(SkCanvas* canvas) {
    if (fAnimator) {
        canvas->drawColor(SK_ColorWHITE);
        fAnimator->draw(canvas, 0);
        
        canvas->save();
        canvas->translate(120, 30);
        canvas->scale(0.5, 0.5);
        fAnimator->draw(canvas, 0);
        canvas->restore();
        
        canvas->save();
        canvas->translate(190, 40);
        canvas->scale(0.25, 0.25);
        fAnimator->draw(canvas, 0);
        canvas->restore();
        
        this->inval(NULL);
    }
}

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() {
    SkAnimatorView* av = new SkAnimatorView;
//    av->decodeFile("/skimages/test.xml");
    av->setURIBase("/skia/trunk/animations/");
    av->decodeFile("/skia/trunk/animations/checkbox.xml");
    return av;
}

static SkViewRegister reg(MyFactory);

