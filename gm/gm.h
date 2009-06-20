#ifndef skiagm_DEFINED
#define skiagm_DEFINED

#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRefCnt.h"
#include "SkSize.h"
#include "SkString.h"
#include "SkTRegistry.h"

namespace skiagm {
	
	static SkISize make_isize(int w, int h) {
		SkISize sz;
		sz.set(w, h);
		return sz;
	}

    class GM {
    public:
        GM();
        virtual ~GM();
		
		void draw(SkCanvas*);
		SkISize getISize() { return this->onISize(); }

	protected:
		virtual void onDraw(SkCanvas*) {}
		virtual SkISize onISize() { return make_isize(0, 0); }
    };

    typedef SkTRegistry<GM*, void*> GMRegistry;
}

#endif
