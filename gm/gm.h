/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skiagm_DEFINED
#define skiagm_DEFINED

#include "gm/verifiers/gmverifier.h"
#include "include/core/SkColor.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkMacros.h"
#include "tools/Registry.h"

#include <memory>

class GrDirectContext;
class GrRecordingContext;
class SkCanvas;
class SkMetaData;
struct GrContextOptions;

#define DEF_GM(CODE)                                         \
    static skiagm::GMRegistry SK_MACRO_APPEND_COUNTER(REG_)( \
            []() { return std::unique_ptr<skiagm::GM>([]() { CODE; }()); });

// A Simple GM is a rendering test that does not store state between rendering calls or make use of
// the onOnceBeforeDraw() virtual; it consists of:
//   *   A name.
//   *   Prefered width and height.
//   *   Optionally, a background color (default is white).
//   *   A standalone function pointer that implements its onDraw method.
#define DEF_SIMPLE_GM(NAME, CANVAS, W, H) \
    DEF_SIMPLE_GM_BG_NAME(NAME, CANVAS, W, H, SK_ColorWHITE, SkString(#NAME))
#define DEF_SIMPLE_GM_BG(NAME, CANVAS, W, H, BGCOLOR) \
    DEF_SIMPLE_GM_BG_NAME(NAME, CANVAS, W, H, BGCOLOR, SkString(#NAME))
#define DEF_SIMPLE_GM_BG_NAME(NAME, CANVAS, W, H, BGCOLOR, NAME_STR) \
    static void SK_MACRO_CONCAT(NAME,_GM_inner)(SkCanvas*); \
    DEF_SIMPLE_GM_BG_NAME_CAN_FAIL(NAME, CANVAS,, W, H, BGCOLOR, NAME_STR) { \
        SK_MACRO_CONCAT(NAME,_GM_inner)(CANVAS); \
        return skiagm::DrawResult::kOk; \
    } \
    void SK_MACRO_CONCAT(NAME,_GM_inner)(SkCanvas* CANVAS)

#define DEF_SIMPLE_GM_CAN_FAIL(NAME, CANVAS, ERR_MSG, W, H) \
    DEF_SIMPLE_GM_BG_NAME_CAN_FAIL(NAME, CANVAS, ERR_MSG, W, H, SK_ColorWHITE, SkString(#NAME))
#define DEF_SIMPLE_GM_BG_CAN_FAIL(NAME, CANVAS, ERR_MSG, W, H, BGCOLOR) \
    DEF_SIMPLE_GM_BG_NAME_CAN_FAIL(NAME, CANVAS, ERR_MSG, W, H, BGCOLOR, SkString(#NAME))
#define DEF_SIMPLE_GM_BG_NAME_CAN_FAIL(NAME, CANVAS, ERR_MSG, W, H, BGCOLOR, NAME_STR) \
    static skiagm::DrawResult SK_MACRO_CONCAT(NAME,_GM)(SkCanvas*, SkString*); \
    DEF_GM(return new skiagm::SimpleGM(BGCOLOR, NAME_STR, {W,H}, SK_MACRO_CONCAT(NAME,_GM));) \
    skiagm::DrawResult SK_MACRO_CONCAT(NAME,_GM)(SkCanvas* CANVAS, SkString* ERR_MSG)


// A Simple GpuGM makes direct GPU calls. Its onDraw hook that includes GPU objects as params, and
// is only invoked on GPU configs. Non-GPU configs automatically draw a GPU-only message and abort.
#define DEF_SIMPLE_GPU_GM(NAME, GR_CONTEXT, CANVAS, W, H)                                         \
    DEF_SIMPLE_GPU_GM_BG(NAME, GR_CONTEXT, CANVAS, W, H, SK_ColorWHITE)

#define DEF_SIMPLE_GPU_GM_BG(NAME, GR_CONTEXT, CANVAS, W, H, BGCOLOR)                             \
    static void SK_MACRO_CONCAT(NAME,_GM_inner)(GrRecordingContext*, SkCanvas*);                  \
    DEF_SIMPLE_GPU_GM_BG_CAN_FAIL(NAME, GR_CONTEXT, CANVAS, /* ERR_MSG */, W, H, BGCOLOR) {       \
        SK_MACRO_CONCAT(NAME,_GM_inner)(GR_CONTEXT, CANVAS);                                      \
        return skiagm::DrawResult::kOk;                                                           \
    }                                                                                             \
    void SK_MACRO_CONCAT(NAME,_GM_inner)(GrRecordingContext* GR_CONTEXT, SkCanvas* CANVAS)

#define DEF_SIMPLE_GPU_GM_CAN_FAIL(NAME, GR_CONTEXT, CANVAS, ERR_MSG, W, H)                       \
    DEF_SIMPLE_GPU_GM_BG_CAN_FAIL(NAME, GR_CONTEXT, CANVAS, ERR_MSG, W, H, SK_ColorWHITE)

#define DEF_SIMPLE_GPU_GM_BG_CAN_FAIL(NAME, GR_CONTEXT, CANVAS, ERR_MSG, W, H, BGCOLOR)           \
    static skiagm::DrawResult SK_MACRO_CONCAT(NAME,_GM)(                                          \
        GrRecordingContext*, SkCanvas*, SkString*);                                               \
    DEF_GM(return new skiagm::SimpleGpuGM(BGCOLOR, SkString(#NAME), {W,H},                        \
                                          SK_MACRO_CONCAT(NAME,_GM));)                            \
    skiagm::DrawResult SK_MACRO_CONCAT(NAME,_GM)(                                                 \
            GrRecordingContext* GR_CONTEXT, SkCanvas* CANVAS, SkString* ERR_MSG)

namespace skiagm {

    enum class DrawResult {
        kOk,   // Test drew successfully.
        kFail, // Test failed to draw.
        kSkip  // Test is not applicable in this context and should be skipped.
    };

    class GM {
    public:
        using DrawResult = skiagm::DrawResult;

        GM(SkColor backgroundColor = SK_ColorWHITE);
        virtual ~GM();

        enum Mode {
            kGM_Mode,
            kSample_Mode,
            kBench_Mode,
        };

        void setMode(Mode mode) { fMode = mode; }
        Mode getMode() const { return fMode; }

        inline static constexpr char kErrorMsg_DrawSkippedGpuOnly[] =
                "This test is for GPU configs only.";

        DrawResult gpuSetup(GrDirectContext* context, SkCanvas* canvas) {
            SkString errorMsg;
            return this->gpuSetup(context, canvas, &errorMsg);
        }
        DrawResult gpuSetup(GrDirectContext*, SkCanvas*, SkString* errorMsg);
        void gpuTeardown();

        void onceBeforeDraw() {
            if (!fHaveCalledOnceBeforeDraw) {
                fHaveCalledOnceBeforeDraw = true;
                this->onOnceBeforeDraw();
            }
        }

        DrawResult draw(SkCanvas* canvas) {
            SkString errorMsg;
            return this->draw(canvas, &errorMsg);
        }
        DrawResult draw(SkCanvas*, SkString* errorMsg);

        void drawBackground(SkCanvas*);
        DrawResult drawContent(SkCanvas* canvas) {
            SkString errorMsg;
            return this->drawContent(canvas, &errorMsg);
        }
        DrawResult drawContent(SkCanvas*, SkString* errorMsg);

        SkISize getISize() { return this->onISize(); }
        const char* getName();

        virtual bool runAsBench() const;

        SkScalar width() {
            return SkIntToScalar(this->getISize().width());
        }
        SkScalar height() {
            return SkIntToScalar(this->getISize().height());
        }

        SkColor getBGColor() const { return fBGColor; }
        void setBGColor(SkColor);

        // helper: fill a rect in the specified color based on the GM's getISize bounds.
        void drawSizeBounds(SkCanvas*, SkColor);

        bool animate(double /*nanos*/);
        virtual bool onChar(SkUnichar);

        bool getControls(SkMetaData* controls) { return this->onGetControls(controls); }
        void setControls(const SkMetaData& controls) { this->onSetControls(controls); }

        virtual void modifyGrContextOptions(GrContextOptions*);

        virtual std::unique_ptr<verifiers::VerifierList> getVerifiers() const;

    protected:
        // onGpuSetup is called once before any other processing with a direct context.
        virtual DrawResult onGpuSetup(GrDirectContext*, SkString*) { return DrawResult::kOk; }
        virtual void onGpuTeardown() {}
        virtual void onOnceBeforeDraw();
        virtual DrawResult onDraw(SkCanvas*, SkString* errorMsg);
        virtual void onDraw(SkCanvas*);

        virtual SkISize onISize() = 0;
        virtual SkString onShortName() = 0;

        virtual bool onAnimate(double /*nanos*/);
        virtual bool onGetControls(SkMetaData*);
        virtual void onSetControls(const SkMetaData&);

    private:
        Mode       fMode;
        SkString   fShortName;
        SkColor    fBGColor;
        bool       fHaveCalledOnceBeforeDraw = false;
        bool       fGpuSetup = false;
        DrawResult fGpuSetupResult = DrawResult::kOk;
    };

    using GMFactory = std::unique_ptr<skiagm::GM> (*)();
    using GMRegistry = sk_tools::Registry<GMFactory>;

    // A GpuGM replaces the onDraw method with one that also accepts GPU objects alongside the
    // SkCanvas. Its onDraw is only invoked on GPU configs; on non-GPU configs it will automatically
    // draw a GPU-only message and abort.
    class GpuGM : public GM {
    public:
        GpuGM(SkColor backgroundColor = SK_ColorWHITE) : GM(backgroundColor) {}

        // TODO(tdenniston): Currently GpuGMs don't have verifiers (because they do not render on
        //   CPU), but we may want to be able to verify the output images standalone, without
        //   requiring a gold image for comparison.
        std::unique_ptr<verifiers::VerifierList> getVerifiers() const override { return nullptr; }

    private:
        using GM::onDraw;
        DrawResult onDraw(SkCanvas*, SkString* errorMsg) final;

        virtual DrawResult onDraw(GrRecordingContext*, SkCanvas*, SkString* errorMsg);
        virtual void onDraw(GrRecordingContext*, SkCanvas*);
    };

    // SimpleGM is intended for basic GMs that can define their entire implementation inside a
    // single "draw" function pointer.
    class SimpleGM : public GM {
    public:
        using DrawProc = DrawResult(*)(SkCanvas*, SkString*);

        SimpleGM(SkColor bgColor, const SkString& name, const SkISize& size, DrawProc drawProc)
                : GM(bgColor), fName(name), fSize(size), fDrawProc(drawProc) {}

    private:
        SkISize onISize() override;
        SkString onShortName() override;
        DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override;

        const SkString fName;
        const SkISize fSize;
        const DrawProc fDrawProc;
    };

    class SimpleGpuGM : public GpuGM {
    public:
        using DrawProc = DrawResult (*)(GrRecordingContext*, SkCanvas*, SkString* errorMsg);

        SimpleGpuGM(SkColor bgColor, const SkString& name, const SkISize& size, DrawProc drawProc)
                : GpuGM(bgColor), fName(name), fSize(size), fDrawProc(drawProc) {}

    private:
        SkISize onISize() override;
        SkString onShortName() override;
        DrawResult onDraw(GrRecordingContext*, SkCanvas*, SkString* errorMsg) override;

        const SkString fName;
        const SkISize fSize;
        const DrawProc fDrawProc;
    };
}  // namespace skiagm

void MarkGMGood(SkCanvas*, SkScalar x, SkScalar y);
void MarkGMBad (SkCanvas*, SkScalar x, SkScalar y);

#endif
