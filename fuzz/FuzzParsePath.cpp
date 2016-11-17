/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Fuzz.h"
#include "SkString.h"
#include "SkParsePath.h"
#include <stdlib.h>

// Most of this is taken from random_parse_path.cpp and adapted to use the Fuzz
// instead of SKRandom

static const struct Legal {
    char fSymbol;
    int fScalars;
} gLegal[] = {
    { 'M', 2 },
    { 'H', 1 },
    { 'V', 1 },
    { 'L', 2 },
    { 'Q', 4 },
    { 'T', 2 },
    { 'C', 6 },
    { 'S', 4 },
    { 'A', 4 },
    { 'Z', 0 },
};

static bool gEasy = false;  // set to true while debugging to suppress unusual whitespace

// mostly do nothing, then bias towards spaces
static const char gWhiteSpace[] = { 0, 0, 0, 0, 0, 0, 0, 0, ' ', ' ', ' ', ' ', 0x09, 0x0D, 0x0A };

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
        fuzz->nextRange(&index, 0, (int) SK_ARRAY_COUNT(gWhiteSpace) - 1);
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
    uint8_t index;
    fuzz->nextRange(&index, 0, (int) SK_ARRAY_COUNT(gLegal) - 1);
    const Legal& legal = gLegal[index];
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
    SkPath path;
    SkString spec;
    uint8_t count;
    fuzz->nextRange(&count, 0, 40);
    for (uint8_t i = 0; i < count; ++i) {
        spec.append(MakeRandomParsePathPiece(fuzz));
    }
    SkDebugf("SkParsePath::FromSVGString(%s, &path);\n",spec.c_str());
    if (!SkParsePath::FromSVGString(spec.c_str(), &path)){
        SkDebugf("Could not decode path\n");
    }
}
