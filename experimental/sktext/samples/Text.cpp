// Copyright 2021 Google LLC.

#include "experimental/sktext/include/Processor.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkStream.h"
#include "include/core/SkTime.h"
#include "samplecode/Sample.h"
#include "src/core/SkOSFile.h"
#include "src/shaders/SkColorShader.h"
#include "src/utils/SkOSPath.h"
#include "src/utils/SkUTF.h"
#include "tools/Resources.h"
#include "tools/flags/CommandLineFlags.h"

using namespace skia::text;

namespace {
class TextSample1 : public Sample {
protected:
    SkString name() override { return SkString("TextSample1"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        Processor::drawText("Hello word", canvas, 0, 0);
    }

private:
    using INHERITED = Sample;
};
}
DEF_SAMPLE(return new TextSample1();)
