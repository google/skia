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

// A Simple GM is a rendering test that does not store state between rendering calls or make use of
// the onOnceBeforeDraw() virtual; it consists of:
//   *   A single void(*)(SkCanvas*) function.
//   *   A name.
//   *   Prefered width and height.
//   *   Optionally, a background color (default is white).
#define DEF_SIMPLE_GM(NAME, CANVAS, W, H) \
    DEF_SIMPLE_GM_BG_NAME(NAME, CANVAS, W, H, SK_ColorWHITE, SkString(#NAME))
#define DEF_SIMPLE_GM_BG(NAME, CANVAS, W, H, BGCOLOR) \
    DEF_SIMPLE_GM_BG_NAME(NAME, CANVAS, W, H, BGCOLOR, SkString(#NAME))
#define DEF_SIMPLE_GM_BG_NAME(NAME, CANVAS, W, H, BGCOLOR, NAME_STR) \
    static void SK_MACRO_CONCAT(NAME, _GM)(SkCanvas * CANVAS); \
    DEF_GM(return skiagm::SimpleGM::Create(NAME_STR, SK_MACRO_CONCAT(NAME, _GM), \
                                           SkISize::Make(W, H), BGCOLOR);) \
    void SK_MACRO_CONCAT(NAME, _GM)(SkCanvas * CANVAS)

// A Skippable GM is a Simple GM whose draw function returns a string. The returned string will
// cause the entire test to be silently skipped when it is not null.
#define DEF_SKIPPABLE_GM(NAME, CANVAS, W, H) \
    DEF_SKIPPABLE_GM_BG(NAME, CANVAS, W, H, SK_ColorWHITE)
#define DEF_SKIPPABLE_GM_BG(NAME, CANVAS, W, H, BGCOLOR) \
    static const char* SK_MACRO_CONCAT(NAME, _GM)(SkCanvas * CANVAS); \
    DEF_GM(return skiagm::SimpleGM::CreateSkippable(SkString(#NAME), SK_MACRO_CONCAT(NAME, _GM), \
                                                    SkISize::Make(W, H), BGCOLOR);) \
    const char* SK_MACRO_CONCAT(NAME, _GM)(SkCanvas * CANVAS)

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

        // Common return values for draw().
        static constexpr char* kDrawComplete = nullptr;
        static constexpr char kDrawSkippedGPUOnly[] = "This test is for GPU configs only.";

        // Returns kDrawComplete (nullptr) if meaningful pixels have been drawn into the canvas
        // (whether successful, or unsuccessful with appropriate error information.)
        //
        // If this GM should be silently ignored (e.g., because it is trying to test a hardware
        // feature that isn't supported in the current context), this method returns a string
        // describing why. When a string is returned, the caller should not output any images or
        // upload any data to Gold.
        const char* draw(SkCanvas*);

        void drawBackground(SkCanvas*);
        const char* drawContent(SkCanvas*);

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

        static void DrawFailureMessage(SkCanvas*, const char[], ...) SK_PRINTF_LIKE(2, 3);

    protected:
        virtual void onOnceBeforeDraw() {}
        // Returns kDrawComplete if meaningful pixels have been drawn into the canvas, or a string
        // if the GM should be skipped. (See the comment for draw().)
        virtual const char* onDraw(SkCanvas*) = 0;
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
        static SimpleGM* Create(const SkString& name, std::function<void(SkCanvas*)> simpleDrawProc,
                                const SkISize&, SkColor backgroundColor);
        static SimpleGM* CreateSkippable(const SkString& name,
                                         std::function<const char*(SkCanvas*)> drawProc,
                                         const SkISize&, SkColor backgroundColor);

    protected:
        const char* onDraw(SkCanvas* canvas) override { return fDrawProc(canvas); }
        SkISize onISize() override { return fSize; }
        SkString onShortName() override { return fName; }

    private:
        SimpleGM(const SkString& name, std::function<const char*(SkCanvas*)> drawProc,
                 const SkISize& size, SkColor backgroundColor)
            : fName(name), fDrawProc(drawProc), fSize(size) {
            if (backgroundColor != SK_ColorWHITE) {
                this->setBGColor(backgroundColor);
            }
        }

        SkString fName;
        std::function<const char*(SkCanvas*)> fDrawProc;
        SkISize fSize;
    };
}

void MarkGMGood(SkCanvas*, SkScalar x, SkScalar y);
void MarkGMBad (SkCanvas*, SkScalar x, SkScalar y);

#endif
