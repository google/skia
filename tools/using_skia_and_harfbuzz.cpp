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
#include <string>
#include <sstream>

#include <hb-ot.h>

#include "SkCanvas.h"
#include "SkDocument.h"
#include "SkStream.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"

struct BaseOption {
  std::string selector;
  std::string description;
  virtual void set(std::string _value) = 0;
  virtual std::string valueToString() = 0;

  BaseOption(std::string _selector, std::string _description) :
    selector(_selector),
    description(_description) {}

  virtual ~BaseOption() {}
};

template <class T> struct Option : BaseOption {
  T value;
  Option(std::string selector, std::string description, T defaultValue) :
    BaseOption(selector, description),
    value(defaultValue) {}
};

struct DoubleOption : Option<double> {
  virtual void set(std::string _value) {
    value = atof(_value.c_str());
  }
  virtual std::string valueToString() {
      std::ostringstream stm;
      stm << value;
      return stm.str();
  }
  DoubleOption(std::string selector, std::string description, double defaultValue) :
    Option<double>(selector, description, defaultValue) {}
};

struct SkStringOption : Option<SkString> {
  virtual void set(std::string _value) {
    value = _value.c_str();
  }
  virtual std::string valueToString() {
    return value.c_str();
  }
  SkStringOption(std::string selector, std::string description, SkString defaultValue) :
    Option<SkString>(selector, description, defaultValue) {}
};

struct StdStringOption : Option<std::string> {
  virtual void set(std::string _value) {
    value = _value;
  }
  virtual std::string valueToString() {
    return value;
  }
  StdStringOption(std::string selector, std::string description, std::string defaultValue) :
    Option<std::string>(selector, description, defaultValue) {}
};

struct Config {
  DoubleOption *page_width = new DoubleOption("-w", "Page width", 600.0f);
  DoubleOption *page_height = new DoubleOption("-h", "Page height", 800.0f);
  SkStringOption *title = new SkStringOption("-t", "PDF title", SkString("---"));
  SkStringOption *author = new SkStringOption("-a", "PDF author", SkString("---"));
  SkStringOption *subject = new SkStringOption("-k", "PDF subject", SkString("---"));
  SkStringOption *keywords = new SkStringOption("-c", "PDF keywords", SkString("---"));
  SkStringOption *creator = new SkStringOption("-t", "PDF creator", SkString("---"));
  StdStringOption *font_file = new StdStringOption("-f", ".ttf font file", "fonts/DejaVuSans.ttf");
  DoubleOption *font_size = new DoubleOption("-z", "Font size", 8.0f);
  DoubleOption *left_margin = new DoubleOption("-m", "Left margin", 20.0f);
  DoubleOption *line_spacing_ratio = new DoubleOption("-h", "Line spacing ratio", 1.5f);
  StdStringOption *output_file_name = new StdStringOption("-o", ".pdf output file name", "out-skiahf.pdf");

  std::map<std::string, BaseOption*> options = {
    { page_width->selector, page_width },
    { page_height->selector, page_height },
    { title->selector, title },
    { author->selector, author },
    { subject->selector, subject },
    { keywords->selector, keywords },
    { creator->selector, creator },
    { font_file->selector, font_file },
    { font_size->selector, font_size },
    { left_margin->selector, left_margin },
    { line_spacing_ratio->selector, line_spacing_ratio },
    { output_file_name->selector, output_file_name },
  };

  Config(int argc, char **argv) {
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
        for (auto it = options.begin(); it != options.end(); ++it) {
          printf("\t%s\t%s (%s)\n", it->first.c_str(),
            it->second->description.c_str(),
            it->second->valueToString().c_str());
        }
        exit(-1);
      }
    }
  } // end of Config::Config
};

const double FONT_SIZE_SCALE = 64.0f;

struct Face {
  struct HBFDel { void operator()(hb_face_t* f) { hb_face_destroy(f); } };
  std::unique_ptr<hb_face_t, HBFDel> fHarfBuzzFace;
  sk_sp<SkTypeface> fSkiaTypeface;

  Face(const char* path, int index) {
    // fairly portable mmap impl
    auto data = SkData::MakeFromFileName(path);
    assert(data);
    if (!data) { return; }
    fSkiaTypeface = SkTypeface::MakeFromStream(new SkMemoryStream(data), index);
    assert(fSkiaTypeface);
    if (!fSkiaTypeface) { return; }
    auto destroy = [](void *d) { static_cast<SkData*>(d)->unref(); };
    const char* bytes = (const char*)data->data();
    unsigned int size = (unsigned int)data->size();
    hb_blob_t* blob = hb_blob_create(bytes,
                                     size,
                                     HB_MEMORY_MODE_READONLY,
                                     data.release(),
                                     destroy);
    assert(blob);
    hb_blob_make_immutable(blob);
    hb_face_t* face = hb_face_create(blob, (unsigned)index);
    hb_blob_destroy(blob);
    assert(face);
    if (!face) {
        fSkiaTypeface.reset();
        return;
    }
    hb_face_set_index(face, (unsigned)index);
    hb_face_set_upem(face, fSkiaTypeface->getUnitsPerEm());
    fHarfBuzzFace.reset(face);
  }
};

class Placement {
 public:
  Placement(Config &_config, SkWStream* outputStream) : config(_config) {
    face = new Face(config.font_file->value.c_str(), 0 /* index */);
    hb_font = hb_font_create(face->fHarfBuzzFace.get());

    hb_font_set_scale(hb_font,
        FONT_SIZE_SCALE * config.font_size->value,
        FONT_SIZE_SCALE * config.font_size->value);
    hb_ot_font_set_funcs(hb_font);

    SkDocument::PDFMetadata pdf_info;
    pdf_info.fTitle = config.title->value;
    pdf_info.fAuthor = config.author->value;
    pdf_info.fSubject = config.subject->value;
    pdf_info.fKeywords = config.keywords->value;
    pdf_info.fCreator = config.creator->value;
    SkTime::DateTime now;
    SkTime::GetDateTime(&now);
    pdf_info.fCreation.fEnabled = true;
    pdf_info.fCreation.fDateTime = now;
    pdf_info.fModified.fEnabled = true;
    pdf_info.fModified.fDateTime = now;
    pdfDocument = SkDocument::MakePDF(outputStream, SK_ScalarDefaultRasterDPI,
                                      pdf_info, nullptr, true);
    assert(pdfDocument);

    white_paint.setColor(SK_ColorWHITE);

    glyph_paint.setFlags(
        SkPaint::kAntiAlias_Flag |
        SkPaint::kSubpixelText_Flag);  // ... avoid waggly text when rotating.
    glyph_paint.setColor(SK_ColorBLACK);
    glyph_paint.setTextSize(config.font_size->value);
    glyph_paint.setTypeface(face->fSkiaTypeface);
    glyph_paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

    NewPage();
  } // end of Placement

  ~Placement() {
    delete face;
    hb_font_destroy (hb_font);
  }

  void WriteLine(const char *text) {
    /* Create hb-buffer and populate. */
    hb_buffer_t *hb_buffer = hb_buffer_create ();
    hb_buffer_add_utf8 (hb_buffer, text, -1, 0, -1);
    hb_buffer_guess_segment_properties (hb_buffer);

    /* Shape it! */
    hb_shape (hb_font, hb_buffer, NULL, 0);

    DrawGlyphs(hb_buffer);

    hb_buffer_destroy (hb_buffer);

    // Advance to the next line.
    current_y += config.line_spacing_ratio->value * config.font_size->value;
    if (current_y > config.page_height->value) {
      pdfDocument->endPage();
      NewPage();
    }
  }

  bool Close() {
    return pdfDocument->close();
  }

private:
  Config config;

  Face *face;

  hb_font_t *hb_font;

  sk_sp<SkDocument> pdfDocument;

  SkCanvas* pageCanvas;

  SkPaint white_paint;
  SkPaint glyph_paint;

  double current_x;
  double current_y;

  void NewPage() {
    pageCanvas = pdfDocument->beginPage(config.page_width->value, config.page_height->value);

    pageCanvas->drawPaint(white_paint);

    current_x = config.left_margin->value;
    current_y = config.line_spacing_ratio->value * config.font_size->value;
  }

  bool DrawGlyphs(hb_buffer_t *hb_buffer) {
    SkTextBlobBuilder textBlobBuilder;
    unsigned len = hb_buffer_get_length (hb_buffer);
    if (len == 0) {
      return true;
    }
    hb_glyph_info_t *info = hb_buffer_get_glyph_infos (hb_buffer, NULL);
    hb_glyph_position_t *pos = hb_buffer_get_glyph_positions (hb_buffer, NULL);
    auto runBuffer = textBlobBuilder.allocRunPos(glyph_paint, len);

    double x = 0;
    double y = 0;
    for (unsigned int i = 0; i < len; i++)
    {
      runBuffer.glyphs[i] = info[i].codepoint;
      reinterpret_cast<SkPoint*>(runBuffer.pos)[i] = SkPoint::Make(
        x + pos[i].x_offset / FONT_SIZE_SCALE,
        y - pos[i].y_offset / FONT_SIZE_SCALE);
      x += pos[i].x_advance / FONT_SIZE_SCALE;
      y += pos[i].y_advance / FONT_SIZE_SCALE;
    }

    pageCanvas->drawTextBlob(textBlobBuilder.build(), current_x, current_y, glyph_paint);
    return true;
  } // end of DrawGlyphs
}; // end of Placement class

int main(int argc, char** argv) {
    Config config(argc, argv);

    Placement placement(config, new SkFILEWStream(config.output_file_name->value.c_str()));
    for (std::string line; std::getline(std::cin, line);) {
      placement.WriteLine(line.c_str());
    }
    placement.Close();

    return 0;
}
