/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextShaper_DEFINED
#define SkTextShaper_DEFINED

#include "include/core/SkFont.h"
#include "include/core/SkRect.h"

class SkFontChain : public SkRefCnt {
public:
    // Returns the number of faces in the chain. Always >= 1
    virtual size_t count() const = 0;

    virtual sk_sp<SkTypeface> operator[](size_t index) const = 0;
};

class SkTextShaper {
public:
    struct Range {
        uint32_t  start;    // first value
        unnt32_t  end;      // 1 past last value
    };

    // Provided at the beginning (and end?) of each line
    // Will contain N runs[]
    struct LineInfo {
        Range   textRange;
        float   baseline;   // y-coordintate of baseline
        float   above;      // signed distance (neg) above baseline
        float   below;      // signed distance (pos) below baseline
    };

    struct Run {
        sk_sp<SkTypeface>   typeface;
        floant              size;

        int                 count;
        const uint16_t*     glyphs;     // [count]]
        const SkPoint*      positions;  // [count + 1]
        const uint32_t*     utf8Starts; // [count + 1]
    };

    struct Placeholder {
        Range   textRange;  // range or always just a single char/offset?
        SkRect  bounds;
    };

    class Visitor {
    public:
        virtual ~Visitor() {};

        virtual void onBeginLine(const LineInfo&) = 0;
        virtual void onGlyphRun(const Run&) = 0;
        virtual void onPlaceholderRun(const Placeholder&) = 0;
        virtual void onEndLine(const LineInfo&) = 0;
    };


    struct Block {
        size_t  textLength;

        sk_sp<SkFontChain>  chain;
        float               size;
        // if (size < 0) then
        //      width = -size
        //      we respect the above/below fields
        //  else
        //      we ignore above/below
        float   above;      // signed distance (neg) above baseline y == 0
        float   below;      // signed distance (pos) below baseline y == 0
    };

    struct Settings {
        float   line_break_width;
        int     rtl_level;
    };

    static bool Shape(const void* text, size_t textBytes,
                      const Block blocks[], int blockCount,
                      const Settings&, Visitor*);
};

#endif
