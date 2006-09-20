#ifndef SkOSWindow_Unix_DEFINED
#define SkOSWindow_Unix_DEFINED

#include "SkWindow.h"
#include <X11/Xlib.h>

struct SkUnixWindow {
  Display* fDisplay;
  Window fWin;
  size_t fOSWin;
};

class SkOSWindow : public SkWindow {
public:
	SkOSWindow(Display* display, Window win);

	void*	getHWND() const { return (void*)fUnixWindow.fWin; }
  void* getDisplay() const { return (void*)fUnixWindow.fDisplay; }
  void* getUnixWindow() const { return (void*)&fUnixWindow; }
  void	setSize(int width, int height);
	void	updateSize();

	static bool PostEvent(SkEvent* evt, SkEventSinkID, SkMSec delay);

	static bool WndProc(SkUnixWindow* w,  XEvent &e);

protected:
	// overrides from SkWindow
	virtual void onHandleInval(const SkRect16&);
	// overrides from SkView
	virtual void onAddMenu(const SkOSMenu*);

private:
	SkUnixWindow  fUnixWindow;

	void	doPaint();

	void*	fMBar;

	typedef SkWindow INHERITED;
};

#endif

