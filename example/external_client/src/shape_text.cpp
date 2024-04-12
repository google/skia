/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/ports/SkFontMgr_empty.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skunicode/include/SkUnicode_icu.h"

#include <cstdio>
#include <cstdlib>
#include <memory>

// https://www.gutenberg.org/ebooks/72339
constexpr const char* story =
    "The landing port at Titan had not changed much in five years.\n"
    "The ship settled down on the scarred blast shield, beside the same trio "
    "of squat square buildings, and quickly disgorged its scanty quota of "
    "cargo and a lone passenger into the flexible tube that linked the loading "
    "hatch with the main building.\n"
    "As soon as the tube was disconnected, the ship screamed off through the "
    "murky atmosphere, seemingly glad to get away from Titan and head back to "
    "the more comfortable and settled parts of the Solar System.";

class OneFontStyleSet : public SkFontStyleSet {
 public:
  explicit OneFontStyleSet(sk_sp<SkTypeface> face) : face_(face) {}

 protected:
  int count() override { return 1; }
  void getStyle(int, SkFontStyle* out_style, SkString*) override {
    *out_style = SkFontStyle();
  }
  sk_sp<SkTypeface> createTypeface(int index) override { return face_; }
  sk_sp<SkTypeface> matchStyle(const SkFontStyle&) override { return face_; }

 private:
  sk_sp<SkTypeface> face_;
};

class OneFontMgr : public SkFontMgr {
 public:
  explicit OneFontMgr(sk_sp<SkTypeface> face)
      : face_(face), style_set_(sk_make_sp<OneFontStyleSet>(face)) {}

 protected:
  int onCountFamilies() const override { return 1; }
  void onGetFamilyName(int index, SkString* familyName) const override {
    *familyName = SkString("the-only-font-I-have");
  }
  sk_sp<SkFontStyleSet> onCreateStyleSet(int index) const override {
    return style_set_;
  }
  sk_sp<SkFontStyleSet> onMatchFamily(const char[]) const override {
    return style_set_;
  }

  sk_sp<SkTypeface> onMatchFamilyStyle(const char[],
                                       const SkFontStyle&) const override {
    return face_;
  }
  sk_sp<SkTypeface> onMatchFamilyStyleCharacter(
      const char familyName[], const SkFontStyle& style, const char* bcp47[],
      int bcp47Count, SkUnichar character) const override {
    return face_;
  }
  sk_sp<SkTypeface> onLegacyMakeTypeface(const char[],
                                         SkFontStyle) const override {
    return face_;
  }

  sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData>, int) const override {
    std::abort();
    return nullptr;
  }
  sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset>,
                                          int) const override {
    std::abort();
    return nullptr;
  }
  sk_sp<SkTypeface> onMakeFromStreamArgs(
      std::unique_ptr<SkStreamAsset>, const SkFontArguments&) const override {
    std::abort();
    return nullptr;
  }
  sk_sp<SkTypeface> onMakeFromFile(const char[], int) const override {
    std::abort();
    return nullptr;
  }

 private:
  sk_sp<SkTypeface> face_;
  sk_sp<SkFontStyleSet> style_set_;
};

int main(int argc, char** argv) {
  if (argc != 3) {
    printf("Usage: %s <font.ttf> <name.png>", argv[0]);
    return 1;
  }

  SkFILEStream input(argv[1]);
  if (!input.isValid()) {
    printf("Cannot open input file %s\n", argv[1]);
    return 1;
  }
  sk_sp<SkData> font_data = SkData::MakeFromStream(&input, input.getLength());
  sk_sp<SkFontMgr> mgr = SkFontMgr_New_Custom_Empty();
  sk_sp<SkTypeface> face = mgr->makeFromData(font_data);
  if (!face) {
    printf("input font %s was not parsable by Freetype\n", argv[1]);
    return 1;
  }

  SkFILEWStream output(argv[2]);
  if (!output.isValid()) {
    printf("Cannot open output file %s\n", argv[2]);
    return 1;
  }

  auto fontCollection = sk_make_sp<skia::textlayout::FontCollection>();
  sk_sp<SkFontMgr> one_mgr = sk_make_sp<OneFontMgr>(face);
  fontCollection->setDefaultFontManager(one_mgr);

  constexpr int width = 200;
  sk_sp<SkSurface> surface =
      SkSurfaces::Raster(SkImageInfo::MakeN32(width, 200, kOpaque_SkAlphaType));
  SkCanvas* canvas = surface->getCanvas();
  canvas->clear(SK_ColorWHITE);

  SkPaint paint;
  paint.setAntiAlias(true);
  paint.setColor(SK_ColorBLACK);

  skia::textlayout::TextStyle style;
  style.setForegroundColor(paint);
  style.setFontFamilies({SkString("sans-serif")});
  style.setFontSize(10.5);
  skia::textlayout::ParagraphStyle paraStyle;
  paraStyle.setTextStyle(style);
  paraStyle.setTextAlign(skia::textlayout::TextAlign::kRight);

  sk_sp<SkUnicode> unicode = SkUnicodes::ICU::Make();
  if (!unicode) {
    printf("Could not load unicode data\n");
    return 1;
  }
  using skia::textlayout::ParagraphBuilder;
  std::unique_ptr<ParagraphBuilder> builder =
      ParagraphBuilder::make(paraStyle, fontCollection, unicode);
  builder->addText(story);

  std::unique_ptr<skia::textlayout::Paragraph> paragraph = builder->Build();
  paragraph->layout(width - 20);
  paragraph->paint(canvas, 10, 10);

  SkPixmap pixmap;
  if (surface->peekPixels(&pixmap)) {
    if (!SkJpegEncoder::Encode(&output, pixmap, {})) {
      printf("Cannot write output\n");
      return 1;
    }
  } else {
    printf("Cannot readback on surface\n");
    return 1;
  }

  return 0;
}
