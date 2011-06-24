#ifndef SkOSWindow_iOS_DEFINED
#define SkOSWindow_iOS_DEFINED

#include "SkWindow.h"
#include "SkMatrix.h"
class SkOSWindow : public SkWindow {
public:
    SkOSWindow(void* hwnd);
    ~SkOSWindow();
    void*   getHWND() const { return fHWND; }

    static bool PostEvent(SkEvent* evt, SkEventSinkID, SkMSec delay);
    
    void    detachGL();
    bool    attachGL();
    void    presentGL();

protected:
    // overrides from SkEventSink
    virtual bool onEvent(const SkEvent& evt);
    // overrides from SkWindow
    virtual void onHandleInval(const SkIRect&);
    // overrides from SkView
    virtual void onAddMenu(const SkOSMenu*);
    virtual void onSetTitle(const char[]);
    
private:
    void*   fHWND;
    bool    fInvalEventIsPending;
    void*   fNotifier;
    typedef SkWindow INHERITED;
};

#endif

