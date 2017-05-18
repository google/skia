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

#include "SkCanvas.h"
#include "SkDocument.h"
#include "SkShaper.h"
#include "SkStream.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"

// Options /////////////////////////////////////////////////////////////////////

struct BaseOption {
    std::string selector;
    std::string description;
    virtual void set(std::string _value) = 0;
    virtual std::string valueToString() = 0;

    BaseOption(std::string _selector, std::string _description)
        : selector(_selector), description(_description) {}

    virtual ~BaseOption() {}

    static void Init(const std::vector<BaseOption*> &, int argc, char **argv);
};

template <class T>
struct Option : BaseOption {
    T value;
    Option(std::string selector, std::string description, T defaultValue)
        : BaseOption(selector, description), value(defaultValue) {}
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
    virtual void set(std::string _value) { value = atof(_value.c_str()); }
    virtual std::string valueToString() {
        std::ostringstream stm;
        stm << value;
        return stm.str();
    }
    DoubleOption(std::string selector,
                 std::string description,
                 double defaultValue)
        : Option<double>(selector, description, defaultValue) {}
};

struct StringOption : Option<std::string> {
    virtual void set(std::string _value) { value = _value; }
    virtual std::string valueToString() { return value; }
    StringOption(std::string selector,
                 std::string description,
                 std::string defaultValue)
        : Option<std::string>(selector, description, defaultValue) {}
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
            DoubleOption("-h", "Line spacing ratio", 1.5f);
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
        glyph_paint.setFlags(SkPaint::kAntiAlias_Flag |
                             SkPaint::kSubpixelText_Flag);
        glyph_paint.setTextSize(SkDoubleToScalar(config->font_size.value));
    }

    void WriteLine(const SkShaper& shaper, const char *text, size_t textBytes) {
        if (!pageCanvas || current_y > config->page_height.value) {
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
        SkTextBlobBuilder textBlobBuilder;
        shaper.shape(&textBlobBuilder, glyph_paint, text, textBytes, SkPoint{0, 0});
        sk_sp<const SkTextBlob> blob = textBlobBuilder.make();
        pageCanvas->drawTextBlob(
                blob.get(), SkDoubleToScalar(current_x),
                SkDoubleToScalar(current_y), glyph_paint);
        // Advance to the next line.
        current_y += config->line_spacing_ratio.value * config->font_size.value;
    }

private:
    const Config* config;
    SkDocument *document;
    SkCanvas *pageCanvas;
    SkPaint white_paint;
    SkPaint glyph_paint;
    double current_x;
    double current_y;
};

////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkDocument> MakePDFDocument(const Config &config,
                                         SkWStream *wStream) {
    SkDocument::PDFMetadata pdf_info;
    pdf_info.fTitle = config.title.value.c_str();
    pdf_info.fAuthor = config.author.value.c_str();
    pdf_info.fSubject = config.subject.value.c_str();
    pdf_info.fKeywords = config.keywords.value.c_str();
    pdf_info.fCreator = config.creator.value.c_str();
    bool pdfa = false;
    #if 0
        SkTime::DateTime now;
        SkTime::GetDateTime(&now);
        pdf_info.fCreation.fEnabled = true;
        pdf_info.fCreation.fDateTime = now;
        pdf_info.fModified.fEnabled = true;
        pdf_info.fModified.fDateTime = now;
        pdfa = true;
    #endif
    return SkDocument::MakePDF(wStream, SK_ScalarDefaultRasterDPI, pdf_info,
                               nullptr, pdfa);
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
        typeface = SkTypeface::MakeFromFile(font_file.c_str(), 0 /* index */);
    }
    SkShaper shaper(typeface);
    assert(shaper.good());
    for (std::string line; std::getline(std::cin, line);) {
        placement.WriteLine(shaper, line.c_str(), line.size());
    }

    doc->close();
    return 0;
}
