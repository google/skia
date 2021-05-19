// Copyright 2021 Google LLC.
#ifndef Shaper_DEFINED
#define Shaper_DEFINED

#include "experimental/sktext/src/TextRun.h"
#include "modules/skshaper/include/SkShaper.h"

namespace skia {
namespace text {

class Processor;
class Shaper : public SkShaper::RunHandler{
public:
    Shaper(Processor* processor, TextDirection textDirection, SkSpan<FontBlock> fontBlocks)
            : fProcessor(processor)
            , fFontBlocks(fontBlocks)
            , fDefaultTextDirection(textDirection)
            , fCurrentRun(nullptr) { }

    bool process();

private:
    SkFont createFont(const FontBlock& block);
    sk_sp<SkTypeface> matchTypeface(const SkString& fontFamily, SkFontStyle fontStyle);

    void beginLine() override {}
    void runInfo(const RunInfo&) override {}
    void commitRunInfo() override {}
    void commitLine() override {}

    Buffer runBuffer(const RunInfo& info) override {
        fCurrentRun = std::make_unique<TextRun>(info);
        return fCurrentRun->newRunBuffer();
    }

    void commitRunBuffer(const RunInfo&) override;

    Processor* fProcessor;
    sk_sp<SkFontMgr> fFontManager;
    TextDirection fDefaultTextDirection;
    SkSpan<FontBlock> fFontBlocks;
    std::unique_ptr<TextRun> fCurrentRun;
};

} // namespace text
} // namespace skia
#endif
