/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "include/core/SkString.h"
#include "include/utils/SkParsePath.h"

#include <stdlib.h>
#include <array>

// Most of this is taken from random_parse_path.cpp and adapted to use the Fuzz
// instead of SKRandom

struct Legal {
    char fSymbol;
    int fScalars;
};

static constexpr auto gLegal = std::to_array<Legal>({
    Legal{ 'M', 2 },
    Legal{ 'H', 1 },
    Legal{ 'V', 1 },
    Legal{ 'L', 2 },
    Legal{ 'Q', 4 },
    Legal{ 'T', 2 },
    Legal{ 'C', 6 },
    Legal{ 'S', 4 },
    Legal{ 'A', 4 },
    Legal{ 'Z', 0 },
});

static bool gEasy = false;  // set to true while debugging to suppress unusual whitespace

// mostly do nothing, then bias towards spaces
static constexpr auto gWhiteSpace = std::to_array<char>({
    0, 0, 0, 0, 0, 0, 0, 0, ' ', ' ', ' ', ' ', 0x09, 0x0D, 0x0A
});

static void add_white(Fuzz* fuzz, SkString* atom) {
    if (gEasy) {
        atom->append(" ");
        return;
    }
    // Use a uint8_t to conserve bytes.  This makes our "fuzzed bytes footprint"
    // smaller, which leads to more efficient fuzzing.
    uint8_t reps;
    fuzz->nextRange(&reps, 0, 2);
    for (uint8_t rep = 0; rep < reps; ++rep) {
        uint8_t index;
        fuzz->nextRange(&index, 0, (int) std::size(gWhiteSpace) - 1);
        if (gWhiteSpace[index]) {
            atom->append(&gWhiteSpace[index], 1);
        }
    }
}

static void add_some_white(Fuzz* fuzz, SkString* atom) {
    for(int i = 0; i < 10; i++) {
        add_white(fuzz, atom);
    }
}

static void add_comma(Fuzz* fuzz, SkString* atom) {
    if (gEasy) {
        atom->append(",");
        return;
    }
    add_white(fuzz, atom);
    bool b;
    fuzz->next(&b);
    if (b) {
        atom->append(",");
    }
    add_some_white(fuzz, atom);
}

SkString MakeRandomParsePathPiece(Fuzz* fuzz) {
    SkString atom;
    uint8_t legalIndex;
    fuzz->nextRange(&legalIndex, 0, (int) std::size(gLegal) - 1);
    const Legal& legal = gLegal[legalIndex];
    gEasy ? atom.append("\n") : add_white(fuzz, &atom);
    bool b;
    fuzz->next(&b);
    char symbol = legal.fSymbol | (b ? 0x20 : 0);
    atom.append(&symbol, 1);
    uint8_t reps;
    fuzz->nextRange(&reps, 1, 3);
    for (int rep = 0; rep < reps; ++rep) {
        for (int index = 0; index < legal.fScalars; ++index) {
            SkScalar coord;
            fuzz->nextRange(&coord, 0.0f, 100.0f);
            add_white(fuzz, &atom);
            atom.appendScalar(coord);
            if (rep < reps - 1 && index < legal.fScalars - 1) {
                add_comma(fuzz, &atom);
            } else {
                add_some_white(fuzz, &atom);
            }
            if ('A' == legal.fSymbol && 1 == index) {
                SkScalar s;
                fuzz->nextRange(&s, -720.0f, 720.0f);
                atom.appendScalar(s);
                add_comma(fuzz, &atom);
                fuzz->next(&b);
                atom.appendU32(b);
                add_comma(fuzz, &atom);
                fuzz->next(&b);
                atom.appendU32(b);
                add_comma(fuzz, &atom);
            }
        }
    }
    return atom;
}

DEF_FUZZ(ParsePath, fuzz) {
    SkString spec;
    uint8_t count;
    fuzz->nextRange(&count, 0, 40);
    for (uint8_t i = 0; i < count; ++i) {
        spec.append(MakeRandomParsePathPiece(fuzz));
    }
    SkDebugf("SkParsePath::FromSVGString(%s, &path);\n",spec.c_str());
    if (!SkParsePath::FromSVGString(spec.c_str())){
        SkDebugf("Could not decode path\n");
    }
}
