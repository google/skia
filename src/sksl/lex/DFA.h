/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DFA
#define SKSL_DFA

#include <string>
#include <vector>

/**
 * Tables representing a deterministic finite automaton for matching regular expressions.
 */
struct DFA {
    DFA(std::vector<int> charMappings, std::vector<std::vector<int>> transitions,
        std::vector<int> accepts)
    : fCharMappings(charMappings)
    , fTransitions(transitions)
    , fAccepts(accepts) {}

    // maps chars to the row index of fTransitions, as multiple characters may map to the same row.
    // starting from state s and looking at char c, the new state is
    // fTransitions[fCharMappings[c]][s].
    std::vector<int> fCharMappings;

    // one row per character mapping, one column per state
    std::vector<std::vector<int>> fTransitions;

    // contains, for each state, the token id we should report when matching ends in that state (-1
    // for no match)
    std::vector<int> fAccepts;
};

#endif
