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

class SkTextShaper {
public:
    struct Range {
        uint32_t  start;    // first value
        unnt32_t  end;      // 1 past last value
    };

    struct LineInfo {
        Range   textRange;
        float   baseline;   // y-coordintate of baseline
        float   above;      // signed distance (neg) above baseline
        float   below;      // signed distance (pos) below baseline
    };

    struct GlyphRun {
        SkFont          font;
        unsigned        flags;
        int             count;
        const uint16_t* glyphs;     // [count]]
        const SkPoint*  positions;  // [count + 1]
        const uint32_t* utf8Starts; // [count + 1]
    };

    struct Placeholder {
        Range   textRange;
        SkRect  bounds;
    };

    class Visitor {
    public:
        virtual ~Visitor() {};

        virtual void onBeginLine(const LineInfo&) = 0;
        virtual void onGlyphRun(const GlyphRun&) = 0;
        virtual void onPlaceholderRun(const Placeholder&) = 0;
        virtual void onEndLine(const LineInfo&) = 0;
    };


    struct Block {
        size_t  textLength;
        int     fontIndex;
        // if fontIndex < 0, this is a placeholder
        float   width;
        float   above;      // signed distance (neg) above baseline y == 0
        float   below;      // signed distance (pos) below baseline y == 0
    };

    struct Settings {
        float   width;
        int     rtl_level;
    };

    static bool Shape(const void* text, size_t textBytes,
                      const Block blocks[], int blockCount,
                      const SkFont fonts[], int fontCount,
                      const Settings&, Visitor*);
};

#endif
