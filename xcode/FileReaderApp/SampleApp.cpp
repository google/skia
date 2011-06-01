#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkWindow.h"

#include "ReaderView.h"
#include "SkGradientShader.h"

class SampleWindow : public SkOSWindow {
    ReaderView fTest;
public:
    SampleWindow(void* hwnd);
    
    virtual void draw(SkCanvas* canvas);
protected:
    virtual void onSizeChange();
    
private:
    void loadView(SkView*);
    void updateTitle();
    
    typedef SkOSWindow INHERITED;
};

bool gNeverSetToTrueJustNeedToFoolLinker;
static void init_effects() {
    if (gNeverSetToTrueJustNeedToFoolLinker) {
        SkPoint p = SkPoint::Make(0,0);
        SkPoint q = SkPoint::Make(100,100);
        SkPoint pts[] = {p, q};
        SkColor colors[] = { SK_ColorRED, SK_ColorGREEN };
        SkScalar pos[] = { 0, 1.0};
        SkGradientShader::CreateLinear(pts, colors, pos, 2, 
                                       SkShader::kMirror_TileMode);
    }
}

SampleWindow::SampleWindow(void* hwnd) : INHERITED(hwnd) {
    init_effects();

    this->setConfig(SkBitmap::kARGB_8888_Config);
    this->setVisibleP(true);
    this->setClipToBounds(false);
    
    this->loadView(&fTest);
    this->setTitle("Reader App");
}

void SampleWindow::draw(SkCanvas* canvas) {
    this->INHERITED::draw(canvas);
}

///////////////////////////////////////////////////////////////////////////////

void SampleWindow::loadView(SkView* view) {
    view->setVisibleP(true);
    view->setClipToBounds(false);
    this->attachChildToFront(view)->unref();
    view->setSize(this->width(), this->height());
}

void SampleWindow::onSizeChange() {
    this->INHERITED::onSizeChange();
    
    SkView::F2BIter iter(this);
    SkView* view = iter.next();
    view->setSize(this->width(), this->height());
}

///////////////////////////////////////////////////////////////////////////////
SkOSWindow* create_sk_window(void* hwnd) {
    return new SampleWindow(hwnd);
}

void application_init() {
    //    setenv("ANDROID_ROOT", "../../../data", 0);
#ifdef SK_BUILD_FOR_MAC
    setenv("ANDROID_ROOT", "/android/device/data", 0);
#endif
    SkGraphics::Init();
    SkEvent::Init();
}

void application_term() {
    SkEvent::Term();
    SkGraphics::Term();
}