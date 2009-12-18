#include "SkWindow.h"
#include "SkCanvas.h"
#include "SkOSMenu.h"
#include "SkSystemEventTypes.h"
#include "SkTime.h"

#define SK_EventDelayInval "\xd" "n" "\xa" "l"

#define TEST_BOUNDERx

#include "SkBounder.h"
class test_bounder : public SkBounder {
public:
	test_bounder(const SkBitmap& bm) : fCanvas(bm) {}
protected:
	virtual bool onIRect(const SkIRect& r)
	{
		SkRect	rr;

		rr.set(SkIntToScalar(r.fLeft), SkIntToScalar(r.fTop),
				SkIntToScalar(r.fRight), SkIntToScalar(r.fBottom));

		SkPaint	p;

		p.setStyle(SkPaint::kStroke_Style);
		p.setColor(SK_ColorYELLOW);

#if 0
		rr.inset(SK_ScalarHalf, SK_ScalarHalf);
#else
		rr.inset(-SK_ScalarHalf, -SK_ScalarHalf);
#endif

		fCanvas.drawRect(rr, p);
		return true;
	}
private:
	SkCanvas	fCanvas;
};

SkWindow::SkWindow() : fFocusView(NULL)
{
	fClick = NULL;
	fWaitingOnInval = false;

#ifdef SK_BUILD_FOR_WINCE
	fConfig = SkBitmap::kRGB_565_Config;
#else
	fConfig = SkBitmap::kARGB_8888_Config;
#endif
}

SkWindow::~SkWindow()
{
	delete fClick;

	fMenus.deleteAll();
}

void SkWindow::setConfig(SkBitmap::Config config)
{
	this->resize(fBitmap.width(), fBitmap.height(), config);
}

void SkWindow::resize(int width, int height, SkBitmap::Config config)
{
	if (config == SkBitmap::kNo_Config)
		config = fConfig;

	if (width != fBitmap.width() || height != fBitmap.height() || config != fConfig)
	{
		fConfig = config;
		fBitmap.setConfig(config, width, height);
		fBitmap.allocPixels();

		this->setSize(SkIntToScalar(width), SkIntToScalar(height));
		this->inval(NULL);
	}
}

void SkWindow::eraseARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b)
{
	fBitmap.eraseARGB(a, r, g, b);
}

void SkWindow::eraseRGB(U8CPU r, U8CPU g, U8CPU b)
{
	fBitmap.eraseRGB(r, g, b);
}

bool SkWindow::handleInval(const SkRect& r)
{
	SkIRect	ir;

	r.round(&ir);
	fDirtyRgn.op(ir, SkRegion::kUnion_Op);

#ifdef SK_BUILD_FOR_WIN32xxxx
	if (!fWaitingOnInval)
	{
		fWaitingOnInval = true;
		(new SkEvent(SK_EventDelayInval))->post(this->getSinkID(), 10);
	}
#else
	this->onHandleInval(ir);
#endif
	return true;
}

#if defined(SK_BUILD_FOR_WINCE) && defined(USE_GX_SCREEN)
	#include <windows.h>
	#include <gx.h>
	extern GXDisplayProperties gDisplayProps;
#endif

#ifdef SK_SIMULATE_FAILED_MALLOC
extern bool gEnableControlledThrow;
#endif

bool SkWindow::update(SkIRect* updateArea)
{
	if (!fDirtyRgn.isEmpty())
	{
		SkBitmap bm = this->getBitmap();

#if defined(SK_BUILD_FOR_WINCE) && defined(USE_GX_SCREEN)
		char* buffer = (char*)GXBeginDraw();
		SkASSERT(buffer);

		RECT	rect;
		GetWindowRect((HWND)((SkOSWindow*)this)->getHWND(), &rect);
		buffer += rect.top * gDisplayProps.cbyPitch + rect.left * gDisplayProps.cbxPitch;

		bm.setPixels(buffer);
#endif

		SkCanvas	canvas(bm);

		canvas.clipRegion(fDirtyRgn);
		if (updateArea)
			*updateArea = fDirtyRgn.getBounds();

		// empty this now, so we can correctly record any inval calls that
		// might be made during the draw call.
		fDirtyRgn.setEmpty();

#ifdef TEST_BOUNDER
		test_bounder	b(bm);
		canvas.setBounder(&b);
#endif
#ifdef SK_SIMULATE_FAILED_MALLOC
		gEnableControlledThrow = true;
#endif
#ifdef SK_BUILD_FOR_WIN32
		try {
			this->draw(&canvas);
		}
		catch (...) {
		}
#else
		this->draw(&canvas);
#endif
#ifdef SK_SIMULATE_FAILED_MALLOC
		gEnableControlledThrow = false;
#endif
#ifdef TEST_BOUNDER
		canvas.setBounder(NULL);
#endif

#if defined(SK_BUILD_FOR_WINCE) && defined(USE_GX_SCREEN)
		GXEndDraw();
#endif

		return true;
	}
	return false;
}

bool SkWindow::handleChar(SkUnichar uni)
{
	if (this->onHandleChar(uni))
		return true;

	SkView* focus = this->getFocusView();
	if (focus == NULL)
		focus = this;

	SkEvent evt(SK_EventType_Unichar);
	evt.setFast32(uni);
	return focus->doEvent(evt);
}

bool SkWindow::handleKey(SkKey key)
{
	if (key == kNONE_SkKey)
		return false;

	if (this->onHandleKey(key))
		return true;

	// send an event to the focus-view
	{
		SkView* focus = this->getFocusView();
		if (focus == NULL)
			focus = this;

		SkEvent evt(SK_EventType_Key);
		evt.setFast32(key);
		if (focus->doEvent(evt))
			return true;
	}

	if (key == kUp_SkKey || key == kDown_SkKey)
	{
		if (this->moveFocus(key == kUp_SkKey ? kPrev_FocusDirection : kNext_FocusDirection) == NULL)
			this->onSetFocusView(NULL);
		return true;
	}
	return false;
}

bool SkWindow::handleKeyUp(SkKey key)
{
    if (key == kNONE_SkKey)
        return false;
        
    if (this->onHandleKeyUp(key))
        return true;
    
    //send an event to the focus-view
    {
        SkView* focus = this->getFocusView();
        if (focus == NULL)
            focus = this;
            
        //should this one be the same?
        SkEvent evt(SK_EventType_KeyUp);
        evt.setFast32(key);
        if (focus->doEvent(evt))
            return true;
    }
    return false;
}

void SkWindow::addMenu(SkOSMenu* menu)
{
	*fMenus.append() = menu;
	this->onAddMenu(menu);
}

void SkWindow::setTitle(const char title[]) {
    if (NULL == title) {
        title = "";
    }
    fTitle.set(title);
    this->onSetTitle(title);
}

bool SkWindow::handleMenu(uint32_t cmd)
{
	for (int i = 0; i < fMenus.count(); i++)
	{
		SkEvent* evt = fMenus[i]->createEvent(cmd);
		if (evt)
		{
			evt->post(this->getSinkID());
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////

bool SkWindow::onEvent(const SkEvent& evt)
{
	if (evt.isType(SK_EventDelayInval))
	{
		SkRegion::Iterator	iter(fDirtyRgn);

		for (; !iter.done(); iter.next())
			this->onHandleInval(iter.rect());
		fWaitingOnInval = false;
		return true;
	}
	return this->INHERITED::onEvent(evt);
}

bool SkWindow::onGetFocusView(SkView** focus) const
{
	if (focus)
		*focus = fFocusView;
	return true;
}

bool SkWindow::onSetFocusView(SkView* focus)
{
	if (fFocusView != focus)
	{
		if (fFocusView)
			fFocusView->onFocusChange(false);
		fFocusView = focus;
		if (focus)
			focus->onFocusChange(true);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////

void SkWindow::onHandleInval(const SkIRect&)
{
}

bool SkWindow::onHandleChar(SkUnichar)
{
	return false;
}

bool SkWindow::onHandleKey(SkKey key)
{
	return false;
}

bool SkWindow::onHandleKeyUp(SkKey key)
{
    return false;
}

bool SkWindow::handleClick(int x, int y, Click::State state)
{
	bool handled = false;

	switch (state) {
	case Click::kDown_State:
		if (fClick)
			delete fClick;
		fClick = this->findClickHandler(SkIntToScalar(x), SkIntToScalar(y));
		if (fClick)
		{
			SkView::DoClickDown(fClick, x, y);
			handled = true;
		}
		break;
	case Click::kMoved_State:
		if (fClick)
		{
			SkView::DoClickMoved(fClick, x, y);
			handled = true;
		}
		break;
	case Click::kUp_State:
		if (fClick)
		{
			SkView::DoClickUp(fClick, x, y);
			delete fClick;
			fClick = NULL;
			handled = true;
		}
		break;
	}
	return handled;
}

