
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skiagm_DEFINED
#define skiagm_DEFINED

#include "SkBitmap.h"
#include "SkBitmapDevice.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkSize.h"
#include "SkString.h"
#include "SkTRegistry.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif

#define DEF_GM(code) \
    static skiagm::GM*          SK_MACRO_APPEND_LINE(F_)(void*) { code; } \
    static skiagm::GMRegistry   SK_MACRO_APPEND_LINE(R_)(SK_MACRO_APPEND_LINE(F_));

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
            kSkipPDF_Flag               = 1 << 0,
            kSkipPicture_Flag           = 1 << 1,
            kSkipPipe_Flag              = 1 << 2,
            kSkipPipeCrossProcess_Flag  = 1 << 3,
            kSkipTiled_Flag             = 1 << 4,
            kSkip565_Flag               = 1 << 5,
            kSkipScaledReplay_Flag      = 1 << 6,
            kSkipGPU_Flag               = 1 << 7,
            kSkipPDFRasterization_Flag  = 1 << 8,

            kGPUOnly_Flag               = 1 << 9,
        };

        void draw(SkCanvas*);
        void drawBackground(SkCanvas*);
        void drawContent(SkCanvas*);

        SkISize getISize() { return this->onISize(); }
        const char* shortName();

        uint32_t getFlags() const {
            return this->onGetFlags();
        }

        SkScalar width() {
            return SkIntToScalar(this->getISize().width());
        }
        SkScalar height() {
            return SkIntToScalar(this->getISize().height());
        }

        // TODO(vandebo) Instead of exposing this, we should run all the GMs
        // with and without an initial transform.
        // Most GMs will return the identity matrix, but some PDFs tests
        // require setting the initial transform.
        SkMatrix getInitialTransform() const {
            SkMatrix matrix = fStarterMatrix;
            matrix.preConcat(this->onGetInitialTransform());
            return matrix;
        }

        SkColor getBGColor() const { return fBGColor; }
        void setBGColor(SkColor);

        // helper: fill a rect in the specified color based on the
        // GM's getISize bounds.
        void drawSizeBounds(SkCanvas*, SkColor);

        static void SetResourcePath(const char* resourcePath) {
            gResourcePath = resourcePath;
        }

        static SkString& GetResourcePath() {
            return gResourcePath;
        }

        bool isCanvasDeferred() const { return fCanvasIsDeferred; }
        void setCanvasIsDeferred(bool isDeferred) {
            fCanvasIsDeferred = isDeferred;
        }

    const SkMatrix& getStarterMatrix() { return fStarterMatrix; }
    void setStarterMatrix(const SkMatrix& matrix) {
        fStarterMatrix = matrix;
    }

    protected:
        static SkString gResourcePath;

        virtual void onOnceBeforeDraw() {}
        virtual void onDraw(SkCanvas*) = 0;
        virtual void onDrawBackground(SkCanvas*);
        virtual SkISize onISize() = 0;
        virtual SkString onShortName() = 0;
        virtual uint32_t onGetFlags() const { return 0; }
        virtual SkMatrix onGetInitialTransform() const { return SkMatrix::I(); }

    private:
        SkString fShortName;
        SkColor  fBGColor;
        bool     fCanvasIsDeferred; // work-around problem in srcmode.cpp
        bool     fHaveCalledOnceBeforeDraw;
        SkMatrix fStarterMatrix;
    };

    typedef SkTRegistry<GM*(*)(void*)> GMRegistry;
}

#endif
