#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"

#include "gm.h"

using namespace skiagm;

// need to explicitly declare this, or we get some weird infinite loop llist
template GMRegistry* GMRegistry::gHead;

class Iter {
public:
    Iter() {
        fReg = GMRegistry::Head();
    }
	
    void reset() {
        fReg = GMRegistry::Head();
    }
        
    GM* next() {
        if (fReg) {
            GMRegistry::Factory fact = fReg->factory();
            fReg = fReg->next();
            return fact(0);
        }
        return NULL;
    }
	
    static int Count() {
        const GMRegistry* reg = GMRegistry::Head();
        int count = 0;
        while (reg) {
            count += 1;
            reg = reg->next();
        }
        return count;
    }
	
private:
    const GMRegistry* fReg;
};

///////////////////////////////////////////////////////////////////////////////

class GMView : public SampleView {
    Iter fIter;
    GM*  fGM;
public:
	GMView() {
        fGM = fIter.next();
        this->postNextGM();
        
        this->setBGColor(0xFFDDDDDD);
    }

    virtual ~GMView() {
        delete fGM;
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "GM");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual bool onEvent(const SkEvent& evt) {
        if (evt.isType("next-gm")) {
            delete fGM;
            if (!(fGM = fIter.next())) {
                fIter.reset();
                fGM = fIter.next();
            }
            this->inval(NULL);
            this->postNextGM();
            return true;
        }
        return this->INHERITED::onEvent(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        fGM->draw(canvas);
    }
    
private:
    void postNextGM() {
        (new SkEvent("next-gm"))->post(this->getSinkID(), 1500);
    }

    typedef SampleView INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new GMView; }
static SkViewRegister reg(MyFactory);

///////////////////////////////////////////////////////////////////////////////

using namespace skiagm;

GM::GM() {}
GM::~GM() {}

void GM::draw(SkCanvas* canvas) {
	this->onDraw(canvas);
}


