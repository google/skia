utils#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"

#include "test.h"

namespace skiatest {
    
class MyReporter : public Reporter {
protected:
    virtual void onStart(Test* test) {}
    virtual void onReport(const char desc[], Reporter::Result result) {
        SkASSERT(Reporter::kPassed == result);
    }
    virtual void onEnd(Test* test) {}
};

class Iter {
public:
    Iter(Reporter* r) : fReporter(r) {
        r->ref();
        fReg = TestRegistry::Head();
    }
    
    ~Iter() {
        fReporter->unref();
    }
    
    Test* next() {
        if (fReg) {
            TestRegistry::Factory fact = fReg->factory();
            fReg = fReg->next();
            Test* test = fact(NULL);
            test->setReporter(fReporter);
            return test;
        }
        return NULL;
    }
    
    static int Count() {
        const TestRegistry* reg = TestRegistry::Head();
        int count = 0;
        while (reg) {
            count += 1;
            reg = reg->next();
        }
        return count;
    }
    
private:
    Reporter* fReporter;
    const TestRegistry* fReg;
};
}

class TestsView : public SkView {
public:
	TestsView() {}

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Tests");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

        skiatest::MyReporter reporter;
        skiatest::Iter iter(&reporter);
        skiatest::Test* test;
        
        while ((test = iter.next()) != NULL) {
            test->run();
            SkDELETE(test);
        }
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        this->inval(NULL);
        
        return this->INHERITED::onFindClickHandler(x, y);
    }
    
    virtual bool onClick(Click* click) {
        this->inval(NULL);
        return this->INHERITED::onClick(click);
    }

	virtual bool handleKey(SkKey key) {
        this->inval(NULL);
        return true;
    }

private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new TestsView; }
static SkViewRegister reg(MyFactory);

