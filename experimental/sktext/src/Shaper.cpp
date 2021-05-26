// Copyright 2021 Google LLC.
#include "experimental/sktext/src/Shaper.h"
#include "experimental/sktext/include/Layout.h"
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/TextLine.h"
#include "modules/skparagraph/src/ParagraphImpl.h"

namespace skia {
namespace text {

// TODO: SkShaper operates in UTF8 indexes
// TODO: calculate intrinsic sizes
// Shape the text in one line
bool Shaper::process() {

    SkString text8 = fProcessor->fUnicode->convertUtf16ToUtf8(fProcessor->fText);
    for (auto& block : this->fProcessor->fFontBlocks) {

        SkFont font(this->createFont(block));

        SkShaper::TrivialFontRunIterator fontIter(font, text8.size());
        SkShaper::TrivialLanguageRunIterator langIter(text8.c_str(), text8.size());
        std::unique_ptr<SkShaper::BiDiRunIterator> bidiIter(
            SkShaper::MakeSkUnicodeBidiRunIterator(
                fProcessor->fUnicode.get(), text8.c_str(), text8.size(), fProcessor->fDefaultTextDirection == TextDirection::kLtr ? 0 : 1));
        std::unique_ptr<SkShaper::ScriptRunIterator> scriptIter(
            SkShaper::MakeSkUnicodeHbScriptRunIterator(fProcessor->fUnicode.get(), text8.c_str(), text8.size()));
        auto shaper = SkShaper::MakeShapeDontWrapOrReorder();
        if (shaper == nullptr) {
            // For instance, loadICU does not work. We have to stop the process
            return false;
        }

        shaper->shape(text8.c_str(), text8.size(),
                fontIter, *bidiIter, *scriptIter, langIter,
                std::numeric_limits<SkScalar>::max(), this);
    }

    fProcessor->markGlyphs();

    return true;
}

bool Shaper::processBySkParagraph() {

    SkString text8 = fProcessor->fUnicode->convertUtf16ToUtf8(fProcessor->fText);

    // 1. Font collection
    sk_sp<skia::textlayout::TypefaceFontProvider> fontProvider = sk_make_sp<skia::textlayout::TypefaceFontProvider>();
    sk_sp<skia::textlayout::FontCollection> fontCollection = sk_make_sp<skia::textlayout::FontCollection>();
    for (auto& block : this->fProcessor->fFontBlocks) {
        for (size_t i = 0; i < block.fFontChain->count(); ++i) {
            auto typeface =  sk_sp<SkTypeface>(block.fFontChain->operator[](i));
            fontProvider->registerTypeface(typeface);
        }
    }
    fontCollection->setAssetFontManager(std::move(fontProvider));

    // 2. Paragraph builder
    skia::textlayout::ParagraphStyle paragraphStyle;
    skia::textlayout::ParagraphBuilderImpl builder(paragraphStyle, fontCollection);
    size_t start16 = 0;
    for (auto& block : this->fProcessor->fFontBlocks) {
        skia::textlayout::TextStyle textStyle;
        textStyle.setFontSize(block.fSize);
        std::vector<SkString> ffs;
        for (size_t i = 0; i < block.fFontChain->count(); ++i) {
            SkString ff;
            block.fFontChain->operator[](i)->getFamilyName(&ff);
            ffs.push_back(ff);
        }
        textStyle.setFontFamilies(ffs);
        builder.pushStyle(textStyle);
        auto text16 = fProcessor->fText.substr(start16, block.fLength);
        auto text8 = fProcessor->fUnicode->convertUtf16ToUtf8(text16);
        builder.addText(text8.c_str(), text8.size());
        builder.pop();
        start16 += block.fLength;
    }

    // 3. Paragraph
    auto paragraph = builder.Build();
    auto impl = static_cast<skia::textlayout::ParagraphImpl*>(paragraph.get());
    impl->layout(SK_FloatInfinity);

    // 4. Convert back
    for (auto& run : impl->runs()) {
        // "Shape" the placeholder
        const SkShaper::RunHandler::RunInfo runInfo = {
            run.font(),
            run.bidiLevel(),
            run.advance(),
            run.size(),
            SkShaper::RunHandler::Range(run.textRange().start - run.clusterStart(), run.textRange().width())
        };
        auto textRun = &fProcessor->fRuns.emplace_back(runInfo);
        for (size_t i = 0; i <= run.glyphs().size(); ++i) {
            if (i < run.glyphs().size()) {
                textRun->fGlyphs[i] = run.glyphs()[i];
                textRun->fBounds[i] = run.getBounds(i);
            }
            textRun->fClusters[i] = run.clusterIndex(i);
            textRun->fPositions[i] = run.positions()[i];
        }
    }

    fProcessor->markGlyphs();

    return true;
}

void Shaper::commitRunBuffer(const RunInfo&) {
    fCurrentRun->commit();

    // Convert all utf8 into utf16
    for (size_t i = 0; i < fCurrentRun->fClusters.size(); ++i) {
        auto& element = fCurrentRun->fClusters[i];
        element = fProcessor->fUTF16FromUTF8[element];
    }
    fProcessor->fRuns.emplace_back(std::move(*fCurrentRun));
}

SkFont Shaper::createFont(const FontBlock& block) {

    SkFont font((block.fFontChain->operator[])(0), block.fSize);
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setHinting(SkFontHinting::kSlight);
    font.setSubpixel(true);

    return font;
}

} // namespace text
} // namespace skia
