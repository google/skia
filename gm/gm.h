/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skiagm_DEFINED
#define skiagm_DEFINED

#include "../tools/Registry.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkClipOpPriv.h"
#include "SkMacros.h"
#include "SkMetaData.h"
#include "SkPaint.h"
#include "SkSize.h"
#include "SkString.h"

class SkAnimTimer;
struct GrContextOptions;

#define DEF_GM(code) \
    static skiagm::GM*          SK_MACRO_APPEND_LINE(F_)(void*) { code; } \
    static skiagm::GMRegistry   SK_MACRO_APPEND_LINE(R_)(SK_MACRO_APPEND_LINE(F_));

// A SimpleGM is a rendering test that does not store state between rendering calls or make use of
// the onOnceBeforeDraw() virtual; it consists of:
//   *   A name.
//   *   Prefered width and height.
//   *   Optionally, a background color (default is white).
//   *   A standalone onDraw implementation.
#define DEF_SIMPLE_GM(NAME, CANVAS, W, H) \
    DEF_SIMPLE_GM_BG_NAME(NAME, CANVAS, W, H, SK_ColorWHITE, SkString(#NAME))
#define DEF_SIMPLE_GM_BG(NAME, CANVAS, W, H, BGCOLOR) \
    DEF_SIMPLE_GM_BG_NAME(NAME, CANVAS, W, H, BGCOLOR, SkString(#NAME))
#define DEF_SIMPLE_GM_BG_NAME(NAME, CANVAS, W, H, BGCOLOR, NAME_STR) \
    class SK_MACRO_CONCAT(NAME,_GM) : public skiagm::SimpleGM { \
    public: \
        SK_MACRO_CONCAT(NAME,_GM)() : SimpleGM(SkString(NAME_STR), SkISize::Make(W,H), BGCOLOR) {} \
    private: \
        void onDraw(SkCanvas*) override; \
    }; \
    DEF_GM(return skiagm::SimpleGM::Create<SK_MACRO_CONCAT(NAME,_GM)>();) \
    void SK_MACRO_CONCAT(NAME,_GM)::onDraw(SkCanvas* CANVAS)

// A SimpleGpuGM is a SimpleGM that makes direct GPU calls. It uses a new onDraw hook that includes
// GPU objects as params, and is only invoked on GPU configs. Non-GPU configs automatically draw
// a GPU-only message and abort.
#define DEF_SIMPLE_GPU_GM(NAME, GR_CONTEXT, RENDER_TARGET_CONTEXT, CANVAS, W, H) \
    DEF_SIMPLE_GPU_GM_BG(NAME, GR_CONTEXT, RENDER_TARGET_CONTEXT, CANVAS, W, H, SK_ColorWHITE)
#define DEF_SIMPLE_GPU_GM_BG(NAME, GR_CONTEXT, RENDER_TARGET_CONTEXT, CANVAS, W, H, BGCOLOR)\
    class SK_MACRO_CONCAT(NAME,_GM) : public skiagm::SimpleGpuGM { \
    public: \
        SK_MACRO_CONCAT(NAME,_GM)() : SimpleGpuGM(SkString(#NAME), SkISize::Make(W,H), BGCOLOR) {} \
    private: \
        void onDraw(GrContext*, GrRenderTargetContext*, SkCanvas*) override; \
    }; \
    DEF_GM(return skiagm::SimpleGM::Create<SK_MACRO_CONCAT(NAME,_GM)>();) \
    void SK_MACRO_CONCAT(NAME,_GM)::onDraw( \
            GrContext* GR_CONTEXT, GrRenderTargetContext* RENDER_TARGET_CONTEXT, SkCanvas* CANVAS)

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

        SkColor getBGColor() const { return fBGColor; }
        void setBGColor(SkColor);

        // helper: fill a rect in the specified color based on the
        // GM's getISize bounds.
        void drawSizeBounds(SkCanvas*, SkColor);

        bool isCanvasDeferred() const { return fCanvasIsDeferred; }
        void setCanvasIsDeferred(bool isDeferred) {
            fCanvasIsDeferred = isDeferred;
        }

        bool animate(const SkAnimTimer&);
        bool handleKey(SkUnichar uni) {
            return this->onHandleKey(uni);
        }

        bool getControls(SkMetaData* controls) { return this->onGetControls(controls); }
        void setControls(const SkMetaData& controls) { this->onSetControls(controls); }

        virtual void modifyGrContextOptions(GrContextOptions* options) {}

        /** draws a standard message that the GM is only intended to be used with the GPU.*/
        static void DrawGpuOnlyMessage(SkCanvas*);

        static void DrawFailureMessage(SkCanvas*, const char[], ...) SK_PRINTF_LIKE(2, 3);

    protected:
        virtual void onOnceBeforeDraw() {}
        virtual void onDraw(SkCanvas*) = 0;
        virtual SkISize onISize() = 0;
        virtual SkString onShortName() = 0;

        virtual bool onAnimate(const SkAnimTimer&) { return false; }
        virtual bool onHandleKey(SkUnichar uni) { return false; }
        virtual bool onGetControls(SkMetaData*) { return false; }
        virtual void onSetControls(const SkMetaData&) {}

    private:
        Mode     fMode;
        SkString fShortName;
        SkColor  fBGColor;
        bool     fCanvasIsDeferred; // work-around problem in srcmode.cpp
        bool     fHaveCalledOnceBeforeDraw;
    };

    typedef GM*(*GMFactory)(void*) ;
    typedef sk_tools::Registry<GMFactory> GMRegistry;

    class SimpleGM : private skiagm::GM {
    public:
        // Performs the private conversion from SimpleGM to GM. We keep GM private so
        // implementations don't accidentally have their local methods shadowed by GM members.
        template<typename TSubclass> static GM* Create() { return new TSubclass(); }

    protected:
        SimpleGM(const SkString& name, const SkISize& size, SkColor backgroundColor)
                : fName(name), fSize(size) {
            if (backgroundColor != SK_ColorWHITE) {
                this->setBGColor(backgroundColor);
            }
        }

    private:
        SkISize onISize() override { return fSize; }
        SkString onShortName() override { return fName; }
        using GM::draw;  // Don't let GM::draw potentially shadow local methods for tests.

        const SkString fName;
        const SkISize fSize;
    };


    class SimpleGpuGM : public SimpleGM {
    protected:
        SimpleGpuGM(const SkString& name, const SkISize& size, SkColor backgroundColor)
                : SimpleGM(name, size, backgroundColor) {}

    private:
        void onDraw(SkCanvas*) final;
        virtual void onDraw(GrContext*, GrRenderTargetContext*, SkCanvas*) = 0;
    };
}

void MarkGMGood(SkCanvas*, SkScalar x, SkScalar y);
void MarkGMBad (SkCanvas*, SkScalar x, SkScalar y);

#endif
