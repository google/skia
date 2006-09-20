#ifndef SkOSWindow_Mac_DEFINED
#define SkOSWindow_Mac_DEFINED

#include "SkWindow.h"

class SkOSWindow : public SkWindow {
public:
	SkOSWindow(void* hwnd);

	void*	getHWND() const { return fHWND; }
	void	updateSize();

	static bool PostEvent(SkEvent* evt, SkEventSinkID, SkMSec delay);

	static pascal OSStatus SkOSWindow::EventHandler( EventHandlerCallRef inHandler, EventRef inEvent, void* userData );

protected:
	// overrides from SkWindow
	virtual void onHandleInval(const SkRect16&);
	// overrides from SkView
	virtual void onAddMenu(const SkOSMenu*);

private:
	void*	fHWND;

	void	doPaint(void* ctx);

	typedef SkWindow INHERITED;
};

#endif

