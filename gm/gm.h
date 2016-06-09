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
#include "SkPaint.h"
#include "SkSize.h"
#include "SkString.h"
#include "SkTRegistry.h"
#include "sk_tool_utils.h"

class SkAnimTimer;
struct GrContextOptions;

#define DEF_GM(code) \
    static skiagm::GM*          SK_MACRO_APPEND_LINE(F_)(void*) { code; } \
    static skiagm::GMRegistry   SK_MACRO_APPEND_LINE(R_)(SK_MACRO_APPEND_LINE(F_));

// a Simple GM is a rendering test that does not store state between
// rendering calls or make use of the onOnceBeforeDraw() virtual; it
// consists of:
//   *   A single void(*)(SkCanvas*) function.
//   *   A name.
//   *   Prefered width and height.
//   *   Optionally, a background color (default is white).
#define DEF_SIMPLE_GM(NAME, CANVAS, W, H) \
    DEF_SIMPLE_GM_BG_NAME(NAME, CANVAS, W, H, SK_ColorWHITE, SkString(#NAME))
#define DEF_SIMPLE_GM_BG(NAME, CANVAS, W, H, BGCOLOR)\
    DEF_SIMPLE_GM_BG_NAME(NAME, CANVAS, W, H, BGCOLOR, SkString(#NAME))
#define DEF_SIMPLE_GM_BG_NAME(NAME, CANVAS, W, H, BGCOLOR, NAME_STR)         \
    static void SK_MACRO_CONCAT(NAME, _GM)(SkCanvas * CANVAS);               \
    DEF_GM(return new skiagm::SimpleGM(NAME_STR, SK_MACRO_CONCAT(NAME, _GM), \
                                       SkISize::Make(W, H), BGCOLOR);)       \
    void SK_MACRO_CONCAT(NAME, _GM)(SkCanvas * CANVAS)

namespace skiagm {

    class GM {
    public:
        GM();
        virtual ~GM();

        enum Mode {
            kGM_Mode,
            kSample_Mode,
            kBench_Mode,
        };

        void setMode(Mode mode) { fMode = mode; }
        Mode getMode() const { return fMode; }

        void draw(SkCanvas*);
        void drawBackground(SkCanvas*);
        void drawContent(SkCanvas*);

        SkISize getISize() { return this->onISize(); }
        const char* getName();

        virtual bool runAsBench() const { return false; }

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

        bool isCanvasDeferred() const { return fCanvasIsDeferred; }
        void setCanvasIsDeferred(bool isDeferred) {
            fCanvasIsDeferred = isDeferred;
        }

        const SkMatrix& getStarterMatrix() { return fStarterMatrix; }
        void setStarterMatrix(const SkMatrix& matrix) {
            fStarterMatrix = matrix;
        }

        bool animate(const SkAnimTimer&);

        virtual void modifyGrContextOptions(GrContextOptions* options) {}

        /** draws a standard message that the GM is only intended to be used with the GPU.*/
        static void DrawGpuOnlyMessage(SkCanvas*);

    protected:
        virtual void onOnceBeforeDraw() {}
        virtual void onDraw(SkCanvas*) = 0;
        virtual void onDrawBackground(SkCanvas*);
        virtual SkISize onISize() = 0;
        virtual SkString onShortName() = 0;

        virtual bool onAnimate(const SkAnimTimer&) { return false; }
        virtual SkMatrix onGetInitialTransform() const { return SkMatrix::I(); }

    private:
        Mode     fMode;
        SkString fShortName;
        SkColor  fBGColor;
        bool     fCanvasIsDeferred; // work-around problem in srcmode.cpp
        bool     fHaveCalledOnceBeforeDraw;
        SkMatrix fStarterMatrix;
    };

    typedef SkTRegistry<GM*(*)(void*)> GMRegistry;

    class SimpleGM : public skiagm::GM {
    public:
        SimpleGM(const SkString& name,
                 void (*drawProc)(SkCanvas*),
                 const SkISize& size,
                 SkColor backgroundColor)
            : fName(name), fDrawProc(drawProc), fSize(size) {
            if (backgroundColor != SK_ColorWHITE) {
                this->setBGColor(backgroundColor);
            }
        }
    protected:
        void onDraw(SkCanvas* canvas) override;
        SkISize onISize() override;
        SkString onShortName() override;
    private:
        SkString fName;
        void (*fDrawProc)(SkCanvas*);
        SkISize fSize;
    };
}

#endif
