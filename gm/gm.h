/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skiagm_DEFINED
#define skiagm_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkMacros.h"
#include "tools/Registry.h"

#include <functional>
#include <map>
#include <memory>

class GrRecordingContext;
class SkCanvas;
class SkMetaData;
struct GrContextOptions;

namespace skiagm::verifiers {
class VerifierList;
}

namespace skgpu::graphite {
struct ContextOptions;
}

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

// Declares a function that dynamically registers GMs (e.g. based on some command-line flag). See
// the GMRegistererFnRegistry definition below for additional context.
#define DEF_GM_REGISTERER_FN(FN) \
    static skiagm::GMRegistererFnRegistry SK_MACRO_APPEND_COUNTER(REG_)(FN)

#if defined(SK_GANESH)
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
#endif

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

        DrawResult gpuSetup(SkCanvas* canvas) {
            SkString errorMsg;
            return this->gpuSetup(canvas, &errorMsg);
        }
        DrawResult gpuSetup(SkCanvas*, SkString* errorMsg);
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

        virtual SkISize getISize() = 0;

        virtual SkString getName() const = 0;

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

        virtual void modifyGrContextOptions(GrContextOptions*) {}
        virtual void modifyGraphiteContextOptions(skgpu::graphite::ContextOptions*) const {}

        // Convenience method to skip Bazel-only GMs from DM.
        //
        // As of Q3 2023, lovisolo@ is experimenting with reimplementing some DM behaviors as
        // smaller, independent Bazel targets. For example, file
        // //tools/testrunners/gm/BazelGMTestRunner.cpp provides a main function that can run GMs.
        // With this file, one can define multiple small Bazel tests to run groups of related GMs
        // with Bazel. However, GMs are only one kind of "source" supported by DM (see class
        // GMSrc). DM supports other kinds of sources as well, such as codecs (CodecSrc class) and
        // image generators (ImageGenSrc class). One possible strategy to support these sources in
        // our Bazel build is to turn them into GMs. For example, instead of using the CodecSrc
        // class from Bazel, we could have a GM subclass that takes an image as an input, decodes
        // it using a codec, and draws in on a canvas. Given that this overlaps with existing DM
        // functionality, we would mark such GMs as Bazel-only.
        //
        // Another possibility is to slowly replace all existing DM source types with just GMs.
        // This would lead to a simpler DM architecture where there is only one source type and
        // multiple sinks, as opposed to the current design with multiple sources and sinks.
        // Furthermore, it would simplify the migration to Bazel because it would allow us to
        // leverage existing work to run GMs with Bazel.
        //
        // TODO(lovisolo): Delete once it's no longer needed.
        virtual bool isBazelOnly() const { return false; }

        // Ignored by DM. Returns the set of Gold key/value pairs specific to this GM, such as the
        // GM name and corpus. GMs may define additional keys. For example, codec GMs define keys
        // for the parameters utilized to initialize the codec.
        virtual std::map<std::string, std::string> getGoldKeys() const {
            return std::map<std::string, std::string>{
                    {"name", getName().c_str()},
                    {"source_type", "gm"},
            };
        }

    protected:
        // onGpuSetup is called once before any other processing with a direct context.
        virtual DrawResult onGpuSetup(SkCanvas*, SkString*) { return DrawResult::kOk; }
        virtual void onGpuTeardown() {}
        virtual void onOnceBeforeDraw();
        virtual DrawResult onDraw(SkCanvas*, SkString* errorMsg);
        virtual void onDraw(SkCanvas*);

        virtual bool onAnimate(double /*nanos*/);
        virtual bool onGetControls(SkMetaData*);
        virtual void onSetControls(const SkMetaData&);

    private:
        Mode fMode;
        SkColor    fBGColor;
        bool       fHaveCalledOnceBeforeDraw = false;
        bool       fGpuSetup = false;
        DrawResult fGpuSetupResult = DrawResult::kOk;
    };

    using GMFactory = std::function<std::unique_ptr<skiagm::GM>()>;
    using GMRegistry = sk_tools::Registry<GMFactory>;

    // Adds a GM to the GMRegistry.
    void Register(skiagm::GM* gm);

    // Registry of functions that dynamically register GMs. Useful for GMs that are unknown at
    // compile time, such as those that are created from images in a directory (see e.g.
    // //gm/png_codec.cpp).
    //
    // A GMRegistererFn may call skiagm::Register() zero or more times to register GMs as needed.
    // It should return the empty string on success, or a human-friendly message in the case of
    // errors.
    //
    // Only used by //tools/testrunners/gm/BazelGMTestRunner.cpp for now.
    using GMRegistererFn = std::function<std::string()>;
    using GMRegistererFnRegistry = sk_tools::Registry<GMRegistererFn>;

#if defined(SK_GANESH)
    // A GpuGM replaces the onDraw method with one that also accepts GPU objects alongside the
    // SkCanvas. Its onDraw is only invoked on GPU configs; on non-GPU configs it will automatically
    // draw a GPU-only message and abort.
    class GpuGM : public GM {
    public:
        GpuGM(SkColor backgroundColor = SK_ColorWHITE) : GM(backgroundColor) {}

    private:
        using GM::onDraw;
        DrawResult onDraw(SkCanvas*, SkString* errorMsg) final;

        virtual DrawResult onDraw(GrRecordingContext*, SkCanvas*, SkString* errorMsg);
        virtual void onDraw(GrRecordingContext*, SkCanvas*);
    };
#endif

    // SimpleGM is intended for basic GMs that can define their entire implementation inside a
    // single "draw" function pointer.
    class SimpleGM : public GM {
    public:
        using DrawProc = DrawResult(*)(SkCanvas*, SkString*);

        SimpleGM(SkColor bgColor, const SkString& name, const SkISize& size, DrawProc drawProc)
                : GM(bgColor), fName(name), fSize(size), fDrawProc(drawProc) {}

        SkString getName() const override;
        SkISize getISize() override;

    private:
        DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override;

        const SkString fName;
        const SkISize fSize;
        const DrawProc fDrawProc;
    };

#if defined(SK_GANESH)
    class SimpleGpuGM : public GpuGM {
    public:
        using DrawProc = DrawResult (*)(GrRecordingContext*, SkCanvas*, SkString* errorMsg);

        SimpleGpuGM(SkColor bgColor, const SkString& name, const SkISize& size, DrawProc drawProc)
                : GpuGM(bgColor), fName(name), fSize(size), fDrawProc(drawProc) {}

        SkString getName() const override;
        SkISize getISize() override;

    private:
        DrawResult onDraw(GrRecordingContext*, SkCanvas*, SkString* errorMsg) override;

        const SkString fName;
        const SkISize fSize;
        const DrawProc fDrawProc;
    };
#endif
}  // namespace skiagm

void MarkGMGood(SkCanvas*, SkScalar x, SkScalar y);
void MarkGMBad (SkCanvas*, SkScalar x, SkScalar y);

#endif
