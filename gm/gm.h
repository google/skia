
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skiagm_DEFINED
#define skiagm_DEFINED

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkPaint.h"
#include "SkSize.h"
#include "SkString.h"
#include "SkTRegistry.h"

namespace skiagm {
        
        static inline SkISize make_isize(int w, int h) {
                SkISize sz;
                sz.set(w, h);
                return sz;
        }

    class GM {
    public:
        GM();
        virtual ~GM();
                
        enum Flags {
            kSkipPDF_Flag       = 1 << 0,
            kSkipPicture_Flag   = 1 << 1,
            kSkipPipe_Flag      = 1 << 2,
            kSkipTiled_Flag     = 1 << 3,
        };

        void draw(SkCanvas*);
        void drawBackground(SkCanvas*);
        void drawContent(SkCanvas*);
        
        SkISize getISize() { return this->onISize(); }
        const char* shortName();

        uint32_t getFlags() const {
            return this->onGetFlags();
        }

        // TODO(vandebo) Instead of exposing this, we should run all the GMs
        // with and without an initial transform.
        // Most GMs will return the identity matrix, but some PDFs tests
        // require setting the initial transform.
        SkMatrix getInitialTransform() const {
            return this->onGetInitialTransform();
        }

        SkColor getBGColor() const { return fBGColor; }
        void setBGColor(SkColor);

        // helper: fill a rect in the specified color based on the
        // GM's getISize bounds.
        void drawSizeBounds(SkCanvas*, SkColor);

        static void SetResourcePath(const char* resourcePath) { 
            gResourcePath = resourcePath; 
        }

    protected:
        static SkString gResourcePath;

        virtual void onDraw(SkCanvas*) = 0;
        virtual void onDrawBackground(SkCanvas*);
        virtual SkISize onISize() = 0;
        virtual SkString onShortName() = 0;
        virtual uint32_t onGetFlags() const { return 0; }
        virtual SkMatrix onGetInitialTransform() const { return SkMatrix::I(); }

    private:
        SkString fShortName;
        SkColor  fBGColor;
    };

    typedef SkTRegistry<GM*, void*> GMRegistry;
}

#endif
