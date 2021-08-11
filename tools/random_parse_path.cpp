/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkRandom.h"
#include "tools/random_parse_path.h"

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

static void add_white(SkRandom* rand, SkString* atom) {
    if (gEasy) {
        atom->append(" ");
        return;
    }
    int reps = rand->nextRangeU(0, 2);
    for (int rep = 0; rep < reps; ++rep) {
        int index = rand->nextRangeU(0, (int) SK_ARRAY_COUNT(gWhiteSpace) - 1);
        if (gWhiteSpace[index]) {
            atom->append(&gWhiteSpace[index], 1);
        }
    }
}

static void add_comma(SkRandom* rand, SkString* atom) {
    if (gEasy) {
        atom->append(",");
        return;
    }
    size_t count = atom->size();
    add_white(rand, atom);
    if (rand->nextBool()) {
        atom->append(",");
    }
    do {
        add_white(rand, atom);
    } while (count == atom->size());
}

static void add_some_white(SkRandom* rand, SkString* atom) {
    size_t count = atom->size();
    do {
        add_white(rand, atom);
    } while (count == atom->size());
}

SkString MakeRandomParsePathPiece(SkRandom* rand) {
    SkString atom;
    int legalIndex = rand->nextRangeU(0, (int) SK_ARRAY_COUNT(gLegal) - 1);
    const Legal& legal = gLegal[legalIndex];
    gEasy ? atom.append("\n") : add_white(rand, &atom);
    char symbol = legal.fSymbol | (rand->nextBool() ? 0x20 : 0);
    atom.append(&symbol, 1);
    int reps = rand->nextRangeU(1, 3);
    for (int rep = 0; rep < reps; ++rep) {
        for (int index = 0; index < legal.fScalars; ++index) {
            SkScalar coord = rand->nextRangeF(0, 100);
            add_white(rand, &atom);
            atom.appendScalar(coord);
            if (rep < reps - 1 && index < legal.fScalars - 1) {
                add_comma(rand, &atom);
            } else {
                add_some_white(rand, &atom);
            }
            if ('A' == legal.fSymbol && 1 == index) {
                atom.appendScalar(rand->nextRangeF(-720, 720));
                add_comma(rand, &atom);
                atom.appendU32(rand->nextRangeU(0, 1));
                add_comma(rand, &atom);
                atom.appendU32(rand->nextRangeU(0, 1));
                add_comma(rand, &atom);
            }
        }
    }
    return atom;
}
