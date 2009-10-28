#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"

#include "SkAnimator.h"
#include "SkStream.h"

#include "SkColorPriv.h"
static inline void Filter_32_opaque_portable(unsigned x, unsigned y,
                                             SkPMColor a00, SkPMColor a01,
                                             SkPMColor a10, SkPMColor a11,
                                             SkPMColor* dstColor) {
    SkASSERT((unsigned)x <= 0xF);
    SkASSERT((unsigned)y <= 0xF);
    
    int xy = x * y;
    uint32_t mask = gMask_00FF00FF; //0xFF00FF;
    
    int scale = 256 - 16*y - 16*x + xy;
    uint32_t lo = (a00 & mask) * scale;
    uint32_t hi = ((a00 >> 8) & mask) * scale;
    
    scale = 16*x - xy;
    lo += (a01 & mask) * scale;
    hi += ((a01 >> 8) & mask) * scale;
    
    scale = 16*y - xy;
    lo += (a10 & mask) * scale;
    hi += ((a10 >> 8) & mask) * scale;
    
    lo += (a11 & mask) * xy;
    hi += ((a11 >> 8) & mask) * xy;
    
    *dstColor = ((lo >> 8) & mask) | (hi & ~mask);
}

static void test_filter() {
    for (int r = 0; r <= 0xFF; r++) {
        SkPMColor c = SkPackARGB32(0xFF, r, r, r);
        for (int y = 0; y <= 0xF; y++) {
            for (int x = 0; x <= 0xF; x++) {
                SkPMColor dst;
                Filter_32_opaque_portable(x, y, c, c, c, c, &dst);
                SkASSERT(SkGetPackedA32(dst) == 255);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

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
    test_filter();
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

