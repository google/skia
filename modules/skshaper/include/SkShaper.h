/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkShaper_DEFINED
#define SkShaper_DEFINED

#include <memory>

#include "SkPoint.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"

class SkFont;

class RunIterator {
 public:
  virtual ~RunIterator() {}
  virtual void consume() = 0;
  // Pointer one past the last (utf8) element in the current run.
  virtual const char* endOfCurrentRun() const = 0;
  virtual bool atEnd() const = 0;
  bool operator<(const RunIterator& that) const {
    return this->endOfCurrentRun() < that.endOfCurrentRun();
  }
};

class FontRunIterator : public RunIterator {
 public:
  virtual const SkFont* currentFont() const = 0;
};

/**
   Shapes text using HarfBuzz and places the shaped text into a
   client-managed buffer.

   If compiled without HarfBuzz, fall back on SkPaint::textToGlyphs.
 */
class SkShaper {
public:
    SkShaper(sk_sp<SkTypeface> face);
    SkShaper();
    ~SkShaper();

    class RunHandler {
    public:
        virtual ~RunHandler() = default;

        struct RunInfo {
            SkVector fAdvance;
            SkScalar fAscent,
                     fDescent,
                     fLeading;
        };

        struct Buffer {
            SkGlyphID* glyphs;    // required
            SkPoint*   positions; // required
            char*      utf8text;  // optional
            uint32_t*  clusters;  // optional
        };

        // Callback per glyph run.
        virtual Buffer newRunBuffer(const RunInfo&, const SkFont&, int glyphCount,
                                    int utf8textCount) = 0;

        // Callback per line.
        virtual void commitLine() = 0;
        virtual void addWord(const char* start, const char* end, SkPoint point, SkVector advance, SkScalar baseline) { }
        virtual void addLine(const char* start, const char* end, SkPoint point, SkVector advance, size_t lineIndex, bool lastLine) { }
    };

    bool good() const;

  SkPoint shape(RunHandler* handler,
                FontRunIterator* font,
                const char* utf8text,
                size_t textBytes,
                bool leftToRight, // TODO: take from the font iterator
                SkPoint point,
                SkScalar width) const;

  SkPoint shape(RunHandler* handler,
                  const SkFont& srcFont,
                  const char* utf8text,
                  size_t textBytes,
                  bool leftToRight,
                  SkPoint point,
                  SkScalar width) const;

private:
    SkShaper(const SkShaper&) = delete;
    SkShaper& operator=(const SkShaper&) = delete;

    struct Impl;
    std::unique_ptr<Impl> fImpl;
};

/**
 * Helper for shaping text directly into a SkTextBlob.
 */
class SkTextBlobBuilderRunHandler final : public SkShaper::RunHandler {
public:
    sk_sp<SkTextBlob> makeBlob();

    SkShaper::RunHandler::Buffer newRunBuffer(const RunInfo&, const SkFont&, int, int) override;

    void commitLine() override {}

private:
    SkTextBlobBuilder fBuilder;
};

class SkPrinterRunHandler final : public SkShaper::RunHandler {
 public:

  sk_sp<SkTextBlob> makeBlob();
  SkShaper::RunHandler::Buffer newRunBuffer(const RunInfo&, const SkFont&, int, int) override;

  void commitLine() override { }
  void addWord(const char* start, const char* end, SkPoint point, SkVector advance, SkScalar baseline) override;
  void addLine(const char* start, const char* end, SkPoint point, SkVector advance, size_t lineIndex, bool lastLine) override;

 private:
  SkTextBlobBuilder fBuilder;
};

#endif  // SkShaper_DEFINED
