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
    static void SK_MACRO_CONCAT(NAME, _GM)(SkCanvas* CANVAS);                \
    DEF_GM(return new skiagm::SimpleGM(NAME_STR, SK_MACRO_CONCAT(NAME, _GM), \
                                       SkISize::Make(W, H), BGCOLOR);)       \
    void SK_MACRO_CONCAT(NAME, _GM)(SkCanvas* CANVAS)


// A SimpleGpuGM is a SimpleGM that makes direct GPU calls. It uses a new onDraw hook that includes
// GPU objects as params, and is only invoked on GPU configs. Non-GPU configs automatically draw
// a GPU-only message and abort.
#define DEF_SIMPLE_GPU_GM(NAME, GR_CONTEXT, RENDER_TARGET_CONTEXT, CANVAS, W, H) \
    DEF_SIMPLE_GPU_GM_BG(NAME, GR_CONTEXT, RENDER_TARGET_CONTEXT, CANVAS, W, H, SK_ColorWHITE)

#define DEF_SIMPLE_GPU_GM_BG(NAME, GR_CONTEXT, RENDER_TARGET_CONTEXT, CANVAS, W, H, BGCOLOR)       \
    static void SK_MACRO_CONCAT(NAME, _GM)(GrContext* GR_CONTEXT,                                  \
                                           GrRenderTargetContext* RENDER_TARGET_CONTEXT,           \
                                           SkCanvas* CANVAS);                                      \
    DEF_GM(return new skiagm::SimpleGpuGM(SkString(#NAME), SK_MACRO_CONCAT(NAME, _GM),             \
                                          SkISize::Make(W, H), BGCOLOR);)                          \
    static void SK_MACRO_CONCAT(NAME, _GM)(GrContext* GR_CONTEXT,                                  \
                                           GrRenderTargetContext* RENDER_TARGET_CONTEXT,           \
                                           SkCanvas* CANVAS)

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

    class SimpleGM : public skiagm::GM {
    public:
        SimpleGM(const SkString& name,
                 void (*drawProc)(SkCanvas*),
                 const SkISize& size,
                 SkColor backgroundColor)
            : fName(name), fDrawProc(drawProc), fSize(size)
        {
            if (backgroundColor != SK_ColorWHITE) {
                this->setBGColor(backgroundColor);
            }
        }

    private:
        void onDraw(SkCanvas*) override;
        SkISize onISize() override { return fSize; }
        SkString onShortName() override { return fName; }

        SkString fName;
        void (*fDrawProc)(SkCanvas*);
        SkISize fSize;
    };

    class SimpleGpuGM : public skiagm::GM {
    public:
        SimpleGpuGM(const SkString& name,
                    void (*drawProc)(GrContext*, GrRenderTargetContext*, SkCanvas*),
                    const SkISize& size,
                    SkColor backgroundColor)
            : fName(name), fDrawProc(drawProc), fSize(size)
        {
            if (backgroundColor != SK_ColorWHITE) {
                this->setBGColor(backgroundColor);
            }
        }

    private:
        void onDraw(SkCanvas*) final;
        SkISize onISize() override { return fSize; }
        SkString onShortName() override { return fName; }

        SkString fName;
        void (*fDrawProc)(GrContext*, GrRenderTargetContext*, SkCanvas*);
        SkISize fSize;
    };
}

void MarkGMGood(SkCanvas*, SkScalar x, SkScalar y);
void MarkGMBad (SkCanvas*, SkScalar x, SkScalar y);

#endif
