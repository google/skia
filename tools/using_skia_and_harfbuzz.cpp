/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This sample progam demonstrates how to use Skia and HarfBuzz to
// produce a PDF file from UTF-8 text in stdin.

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/docs/SkPDFDocument.h"
#include "include/ports/SkFontMgr_empty.h"
#include "modules/skshaper/include/SkShaper_harfbuzz.h"
#include "modules/skshaper/include/SkShaper_skunicode.h"
#include "modules/skunicode/include/SkUnicode.h"

namespace {
sk_sp<SkUnicode> get_unicode() {
#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
    auto unicode = SkUnicodes::ICU::Make();
    if (unicode) {
        return unicode;
    }
#endif
    SkDEBUGFAIL("Only ICU implementation of SkUnicode is supported");
    return nullptr;
}

} // namespace

// Options /////////////////////////////////////////////////////////////////////

struct BaseOption {
    std::string selector;
    std::string description;
    virtual void set(const std::string& _value) = 0;
    virtual std::string valueToString() = 0;

    BaseOption(std::string _selector, std::string _description)
            : selector(std::move(_selector))
            , description(std::move(_description)) {}

    virtual ~BaseOption() {}

    static void Init(const std::vector<BaseOption*> &, int argc, char **argv);
};

template <class T>
struct Option : BaseOption {
    T value;
    Option(std::string _selector, std::string _description, T defaultValue)
            : BaseOption(std::move(_selector), std::move(_description))
            , value(defaultValue) {}
};

void BaseOption::Init(const std::vector<BaseOption*> &option_list,
                      int argc, char **argv) {
    std::map<std::string, BaseOption *> options;
    for (BaseOption *opt : option_list) {
        options[opt->selector] = opt;
    }
    for (int i = 1; i < argc; i++) {
        std::string option_selector(argv[i]);
        auto it = options.find(option_selector);
        if (it != options.end()) {
            if (i >= argc) {
                break;
            }
            const char *option_value = argv[i + 1];
            it->second->set(option_value);
            i++;
        } else {
            printf("Ignoring unrecognized option: %s.\n", argv[i]);
            printf("Usage: %s {option value}\n", argv[0]);
            printf("\tTakes text from stdin and produces pdf file.\n");
            printf("Supported options:\n");
            for (BaseOption *opt : option_list) {
                printf("\t%s\t%s (%s)\n", opt->selector.c_str(),
                       opt->description.c_str(), opt->valueToString().c_str());
            }
            exit(-1);
        }
    }
}

struct DoubleOption : Option<double> {
    void set(const std::string& _value) override { value = atof(_value.c_str()); }
    std::string valueToString() override {
        std::ostringstream stm;
        stm << value;
        return stm.str();
    }
    DoubleOption(std::string _selector, std::string _description, double defaultValue)
            : Option<double>(std::move(_selector),
                             std::move(_description),
                             std::move(defaultValue)) {}
};

struct StringOption : Option<std::string> {
    void set(const std::string& _value) override { value = _value; }
    std::string valueToString() override { return value; }
    StringOption(std::string _selector, std::string _description, std::string defaultValue)
            : Option<std::string>(std::move(_selector),
                                  std::move(_description),
                                  std::move(defaultValue)) {}
};

// Config //////////////////////////////////////////////////////////////////////

struct Config {
    DoubleOption page_width = DoubleOption("-w", "Page width", 600.0f);
    DoubleOption page_height = DoubleOption("-h", "Page height", 800.0f);
    StringOption title = StringOption("-t", "PDF title", "---");
    StringOption author = StringOption("-a", "PDF author", "---");
    StringOption subject = StringOption("-k", "PDF subject", "---");
    StringOption keywords = StringOption("-c", "PDF keywords", "---");
    StringOption creator = StringOption("-t", "PDF creator", "---");
    StringOption font_file = StringOption("-f", ".ttf font file", "");
    DoubleOption font_size = DoubleOption("-z", "Font size", 8.0f);
    DoubleOption left_margin = DoubleOption("-m", "Left margin", 20.0f);
    DoubleOption line_spacing_ratio =
            DoubleOption("-h", "Line spacing ratio", 0.25f);
    StringOption output_file_name =
            StringOption("-o", ".pdf output file name", "out-skiahf.pdf");

    Config(int argc, char **argv) {
        BaseOption::Init(std::vector<BaseOption*>{
                &page_width, &page_height, &title, &author, &subject,
                &keywords, &creator, &font_file, &font_size, &left_margin,
                &line_spacing_ratio, &output_file_name}, argc, argv);
    }
};

// Placement ///////////////////////////////////////////////////////////////////

class Placement {
public:
    Placement(const Config* conf, SkDocument *doc)
        : config(conf), document(doc), pageCanvas(nullptr) {
        white_paint.setColor(SK_ColorWHITE);
        glyph_paint.setColor(SK_ColorBLACK);
        glyph_paint.setAntiAlias(true);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        font.setSubpixel(true);
        font.setSize(SkDoubleToScalar(config->font_size.value));
    }

    void WriteLine(const SkShaper& shaper, const char* text, size_t textBytes) {
        SkTextBlobBuilderRunHandler textBlobBuilder(text, {0, 0});

        const SkBidiIterator::Level defaultLevel = SkBidiIterator::kLTR;
        std::unique_ptr<SkShaper::BiDiRunIterator> bidi =
                SkShapers::unicode::BidiRunIterator(get_unicode(), text, textBytes, defaultLevel);
        SkASSERT(bidi);

        std::unique_ptr<SkShaper::LanguageRunIterator> language =
                SkShaper::MakeStdLanguageRunIterator(text, textBytes);
        SkASSERT(language);

        std::unique_ptr<SkShaper::ScriptRunIterator> script =
                SkShapers::HB::ScriptRunIterator(text, textBytes);
        SkASSERT(script);

        std::unique_ptr<SkShaper::FontRunIterator> fontRuns =
                SkShaper::MakeFontMgrRunIterator(text, textBytes, font, SkFontMgr::RefEmpty());
        SkASSERT(fontRuns);

        const SkScalar width = config->page_width.value - 2 * config->left_margin.value;
        shaper.shape(text,
                     textBytes,
                     *fontRuns,
                     *bidi,
                     *script,
                     *language,
                     nullptr,
                     0,
                     width,
                     &textBlobBuilder);
        SkPoint endPoint = textBlobBuilder.endPoint();
        sk_sp<const SkTextBlob> blob = textBlobBuilder.makeBlob();
        // If we don't have a page, or if we're not at the start of the page and the blob won't fit
        if (!pageCanvas ||
              (current_y > config->line_spacing_ratio.value * config->font_size.value &&
               current_y + endPoint.y() > config->page_height.value)
        ) {
            if (pageCanvas) {
                document->endPage();
            }
            pageCanvas = document->beginPage(
                    SkDoubleToScalar(config->page_width.value),
                    SkDoubleToScalar(config->page_height.value));
            pageCanvas->drawPaint(white_paint);
            current_x = config->left_margin.value;
            current_y = config->line_spacing_ratio.value * config->font_size.value;
        }
        pageCanvas->drawTextBlob(
                blob.get(), SkDoubleToScalar(current_x),
                SkDoubleToScalar(current_y), glyph_paint);
        // Advance to the next line.
        current_y += endPoint.y() + config->line_spacing_ratio.value * config->font_size.value;
    }

private:
    const Config* config;
    SkDocument *document;
    SkCanvas *pageCanvas;
    SkPaint white_paint;
    SkPaint glyph_paint;
    SkFont font;
    double current_x;
    double current_y;
};

////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkDocument> MakePDFDocument(const Config &config, SkWStream *wStream) {
    SkPDF::Metadata pdf_info;
    pdf_info.fTitle = config.title.value.c_str();
    pdf_info.fAuthor = config.author.value.c_str();
    pdf_info.fSubject = config.subject.value.c_str();
    pdf_info.fKeywords = config.keywords.value.c_str();
    pdf_info.fCreator = config.creator.value.c_str();
    #if 0
        SkPDF::DateTime now;
        SkPDFUtils::GetDateTime(&now);
        pdf_info.fCreation = now;
        pdf_info.fModified = now;
        pdf_info.fPDFA = true;
    #endif
    return SkPDF::MakeDocument(wStream, pdf_info);
}

int main(int argc, char **argv) {
    Config config(argc, argv);
    SkFILEWStream wStream(config.output_file_name.value.c_str());
    sk_sp<SkDocument> doc = MakePDFDocument(config, &wStream);
    assert(doc);
    Placement placement(&config, doc.get());

    const std::string &font_file = config.font_file.value;
    sk_sp<SkTypeface> typeface;
    if (font_file.size() > 0) {
        // There are different font managers for different platforms. See include/ports
        sk_sp<SkFontMgr> mgr = SkFontMgr_New_Custom_Empty();
        assert(mgr);
        typeface = mgr->makeFromFile(font_file.c_str(), 0 /* index */);
    }
    std::unique_ptr<SkShaper> shaper =
            SkShapers::HB::ShaperDrivenWrapper(get_unicode(), nullptr);
    assert(shaper);
    //SkString line("This is هذا هو الخط a line.");
    //SkString line("⁧This is a line هذا هو الخط.⁩");
    for (std::string line; std::getline(std::cin, line);) {
        placement.WriteLine(*shaper, line.c_str(), line.size());
    }

    doc->close();
    wStream.flush();
    return 0;
}
