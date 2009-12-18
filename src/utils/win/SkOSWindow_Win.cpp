#include "SkTypes.h"

#if defined(SK_BUILD_FOR_WIN)

#include "SkWindow.h"
#include "SkCanvas.h"
#include "SkOSMenu.h"
#include "SkTime.h"

#include "SkGraphics.h"

static SkOSWindow* gCurrOSWin;

SkOSWindow::SkOSWindow(void* hWnd) : fHWND(hWnd) {
}

static SkKey winToskKey(WPARAM vk) {
	static const struct {
		WPARAM	fVK;
		SkKey	fKey;
	} gPair[] = {
		{ VK_BACK,	kBack_SkKey },
		{ VK_CLEAR,	kBack_SkKey },
		{ VK_RETURN, kOK_SkKey },
		{ VK_UP,	 kUp_SkKey },
		{ VK_DOWN,	 kDown_SkKey },
		{ VK_LEFT,	 kLeft_SkKey },
		{ VK_RIGHT,	 kRight_SkKey }
	};
	for (size_t i = 0; i < SK_ARRAY_COUNT(gPair); i++) {
		if (gPair[i].fVK == vk) {
			return gPair[i].fKey;
		}
	}
	return kNONE_SkKey;
}

bool SkOSWindow::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_KEYDOWN: {
			SkKey key = winToskKey(wParam);
			if (kNONE_SkKey != key) {
				this->handleKey(key);
				return true;
			}
		} break;
		case WM_KEYUP: {
			SkKey key = winToskKey(wParam);
			if (kNONE_SkKey != key) {
				this->handleKeyUp(key);
				return true;
			}
		} break;
		case WM_UNICHAR:
			this->handleChar(lParam);
			return true;
		case WM_SIZE:
			this->resize(lParam & 0xFFFF, lParam >> 16);
			break;
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			this->doPaint(hdc);
			EndPaint(hWnd, &ps);
			return true;
		} break;
	}
	return false;
}

void SkOSWindow::doPaint(void* ctx) {
	this->update(NULL);

	HDC hdc = (HDC)ctx;
    const SkBitmap& bitmap = this->getBitmap();

	BITMAPINFO bmi;
	memset(&bmi, 0, sizeof(bmi));
	bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth       = bitmap.width();
	bmi.bmiHeader.biHeight      = -bitmap.height(); // top-down image 
	bmi.bmiHeader.biPlanes      = 1;
	bmi.bmiHeader.biBitCount    = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage   = 0;

	// 
	// Do the SetDIBitsToDevice. 
	// 
	bitmap.lockPixels();
	int iRet = SetDIBitsToDevice(hdc,
		0, 0,
		bitmap.width(), bitmap.height(),
		0, 0,
		0, bitmap.height(),
		bitmap.getPixels(),
		&bmi,
		DIB_RGB_COLORS);
	bitmap.unlockPixels();
}

#if 0
void SkOSWindow::updateSize()
{
	RECT	r;
	GetWindowRect((HWND)this->getHWND(), &r);
	this->resize(r.right - r.left, r.bottom - r.top);
}
#endif

void SkOSWindow::onHandleInval(const SkIRect& r) {
	RECT rect;
	rect.left = r.fLeft;
	rect.top = r.fTop;
	rect.right = r.fRight;
	rect.bottom = r.fBottom;
	InvalidateRect((HWND)this->getHWND(), &rect, false);
}

void SkOSWindow::onAddMenu(const SkOSMenu* sk_menu)
{
}


enum {
	SK_MacReturnKey		= 36,
	SK_MacDeleteKey		= 51,
	SK_MacEndKey		= 119,
	SK_MacLeftKey		= 123,
	SK_MacRightKey		= 124,
	SK_MacDownKey		= 125,
	SK_MacUpKey			= 126,
    
    SK_Mac0Key          = 0x52,
    SK_Mac1Key          = 0x53,
    SK_Mac2Key          = 0x54,
    SK_Mac3Key          = 0x55,
    SK_Mac4Key          = 0x56,
    SK_Mac5Key          = 0x57,
    SK_Mac6Key          = 0x58,
    SK_Mac7Key          = 0x59,
    SK_Mac8Key          = 0x5b,
    SK_Mac9Key          = 0x5c
};
	
static SkKey raw2key(uint32_t raw)
{
	static const struct {
		uint32_t  fRaw;
		SkKey   fKey;
	} gKeys[] = {
		{ SK_MacUpKey,		kUp_SkKey		},
		{ SK_MacDownKey,	kDown_SkKey		},
		{ SK_MacLeftKey,	kLeft_SkKey		},
		{ SK_MacRightKey,   kRight_SkKey	},
		{ SK_MacReturnKey,  kOK_SkKey		},
		{ SK_MacDeleteKey,  kBack_SkKey		},
		{ SK_MacEndKey,		kEnd_SkKey		},
        { SK_Mac0Key,       k0_SkKey        },
        { SK_Mac1Key,       k1_SkKey        },
        { SK_Mac2Key,       k2_SkKey        },
        { SK_Mac3Key,       k3_SkKey        },
        { SK_Mac4Key,       k4_SkKey        },
        { SK_Mac5Key,       k5_SkKey        },
        { SK_Mac6Key,       k6_SkKey        },
        { SK_Mac7Key,       k7_SkKey        },
        { SK_Mac8Key,       k8_SkKey        },
        { SK_Mac9Key,       k9_SkKey        }
	};
	
	for (unsigned i = 0; i < SK_ARRAY_COUNT(gKeys); i++)
		if (gKeys[i].fRaw == raw)
			return gKeys[i].fKey;
	return kNONE_SkKey;
}

///////////////////////////////////////////////////////////////////////////////////////

void SkEvent::SignalNonEmptyQueue()
{
//	post_skmacevent();
//	SkDebugf("signal nonempty\n");
}

//static void sk_timer_proc(TMTask* rec)
//{
//	SkEvent::ServiceQueueTimer();
//	SkDebugf("timer task fired\n");
//}

void SkEvent::SignalQueueTimer(SkMSec delay)
{
#if 0
	if (gTMTaskPtr)
	{
		RemoveTimeTask((QElem*)gTMTaskPtr);
		DisposeTimerUPP(gTMTaskPtr->tmAddr);
		gTMTaskPtr = nil;
	}
	if (delay)
	{
		gTMTaskPtr = &gTMTaskRec;
		memset(gTMTaskPtr, 0, sizeof(gTMTaskRec));
		gTMTaskPtr->tmAddr = NewTimerUPP(sk_timer_proc);
		OSErr err = InstallTimeTask((QElem*)gTMTaskPtr);
//		SkDebugf("installtimetask of %d returned %d\n", delay, err);
		PrimeTimeTask((QElem*)gTMTaskPtr, delay);
	}
#endif
}

#endif

