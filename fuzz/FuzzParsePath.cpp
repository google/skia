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

const struct Legal {
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

bool gEasy = false;  // set to true while debugging to suppress unusual whitespace

// mostly do nothing, then bias towards spaces
const char gWhiteSpace[] = { 0, 0, 0, 0, 0, 0, 0, 0, ' ', ' ', ' ', ' ', 0x09, 0x0D, 0x0A };

static void add_white(Fuzz* fuzz, SkString* atom) {
    if (gEasy) {
        atom->append(" ");
        return;
    }
    int reps = fuzz->nextRangeU(0, 2);
    for (int rep = 0; rep < reps; ++rep) {
        int index = fuzz->nextRangeU(0, (int) SK_ARRAY_COUNT(gWhiteSpace) - 1);
        if (gWhiteSpace[index]) {
            atom->append(&gWhiteSpace[index], 1);
        }
    }
}

static void add_comma(Fuzz* fuzz, SkString* atom) {
    if (gEasy) {
        atom->append(",");
        return;
    }
    size_t count = atom->size();
    add_white(fuzz, atom);
    if (fuzz->nextBool()) {
        atom->append(",");
    }
    do {
        add_white(fuzz, atom);
    } while (count == atom->size());
}

static void add_some_white(Fuzz* fuzz, SkString* atom) {
    size_t count = atom->size();
    do {
        add_white(fuzz, atom);
    } while (count == atom->size());
}

SkString MakeRandomParsePathPiece(Fuzz* fuzz) {
    SkString atom;
    int index = fuzz->nextRangeU(0, (int) SK_ARRAY_COUNT(gLegal) - 1);
    const Legal& legal = gLegal[index];
    gEasy ? atom.append("\n") : add_white(fuzz, &atom);
    char symbol = legal.fSymbol | (fuzz->nextBool() ? 0x20 : 0);
    atom.append(&symbol, 1);
    int reps = fuzz->nextRangeU(1, 3);
    for (int rep = 0; rep < reps; ++rep) {
        for (int index = 0; index < legal.fScalars; ++index) {
            SkScalar coord = fuzz->nextRangeF(0, 100);
            add_white(fuzz, &atom);
            atom.appendScalar(coord);
            if (rep < reps - 1 && index < legal.fScalars - 1) {
                add_comma(fuzz, &atom);
            } else {
                add_some_white(fuzz, &atom);
            }
            if ('A' == legal.fSymbol && 1 == index) {
                atom.appendScalar(fuzz->nextRangeF(-720, 720));
                add_comma(fuzz, &atom);
                atom.appendU32(fuzz->nextRangeU(0, 1));
                add_comma(fuzz, &atom);
                atom.appendU32(fuzz->nextRangeU(0, 1));
                add_comma(fuzz, &atom);
            }
        }
    }
    return atom;
}

DEF_FUZZ(ParsePath, fuzz) {
    SkPath path;
    SkString spec;
    uint32_t count = fuzz->nextRangeU(0, 40);
    for (uint32_t i = 0; i < count; ++i) {
        spec.append(MakeRandomParsePathPiece(fuzz));
    }
    SkDebugf("SkParsePath::FromSVGString(%s, &path);\n",spec.c_str());
    if (!SkParsePath::FromSVGString(spec.c_str(), &path)){
        fuzz->signalBug();
    }
}
