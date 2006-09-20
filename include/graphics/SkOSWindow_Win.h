#ifndef SkOSWindow_Win_DEFINED
#define SkOSWindow_Win_DEFINED

#include "SkWindow.h"

class SkOSWindow : public SkWindow {
public:
	SkOSWindow(void* hwnd);

	void*	getHWND() const { return fHWND; }
	void	setSize(int width, int height);
	void	updateSize();

	static bool PostEvent(SkEvent* evt, SkEventSinkID, SkMSec delay);

	static bool WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static bool SkOSWindow::QuitOnDeactivate(HWND hWnd);

	enum {
		SK_WM_SkEvent = WM_APP + 1000,
		SK_WM_SkTimerID = 0xFFFF	// just need a non-zero value
	};

protected:
	virtual bool quitOnDeactivate() { return true; }

	// overrides from SkWindow
	virtual void onHandleInval(const SkRect16&);
	// overrides from SkView
	virtual void onAddMenu(const SkOSMenu*);

private:
	void*	fHWND;

	void	doPaint(void* ctx);

	HMENU	fMBar;

	typedef SkWindow INHERITED;
};

#endif

