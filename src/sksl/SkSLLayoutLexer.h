/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SKSL_LayoutLexer
#define SKSL_LayoutLexer

#include <inttypes.h>
#include <stddef.h>

namespace SkSL {

struct LayoutToken {
    enum Kind {
        END_OF_FILE,
        LOCATION,
        OFFSET,
        BINDING,
        INDEX,
        SET,
        BUILTIN,
        INPUT_ATTACHMENT_INDEX,
        ORIGIN_UPPER_LEFT,
        OVERRIDE_COVERAGE,
        BLEND_SUPPORT_ALL_EQUATIONS,
        PUSH_CONSTANT,
        POINTS,
        LINES,
        LINE_STRIP,
        LINES_ADJACENCY,
        TRIANGLES,
        TRIANGLE_STRIP,
        TRIANGLES_ADJACENCY,
        MAX_VERTICES,
        INVOCATIONS,
        WHEN,
        KEY,
        INVALID
    };

    LayoutToken()
    : fKind(Kind::INVALID)
    , fOffset(-1)
    , fLength(-1) {}

    LayoutToken(Kind kind, int offset, int length)
    : fKind(kind)
    , fOffset(offset)
    , fLength(length) {}

    Kind fKind;
    int fOffset;
    int fLength;
};

class LayoutLexer {
public:

    void start(const char* text, size_t length) {
        fText = text;
        fLength = length;
        fOffset = 0;
    }

    LayoutToken next();

private:
    const char* fText;
    int fLength;
    int fOffset;
};

} // namespace

#endif
