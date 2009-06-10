#ifndef skiagm_DEFINED
#define skiagm_DEFINED

#include "SkRefCnt.h"
#include "SkString.h"
#include "SkTRegistry.h"

namespace skiagm {

    class GM {
    public:
        GM();
        virtual ~GM();
		
		void draw(SkCanvas*);

	protected:
		virtual void onDraw(SkCanvas*) {}
    };

    typedef SkTRegistry<GM*, void*> GMRegistry;
}

#endif
