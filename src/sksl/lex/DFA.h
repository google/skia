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
    DFA(std::vector<std::vector<int>> transitions, std::vector<int> accepts)
    : fTransitions(transitions)
    , fAccepts(accepts) {}

    static constexpr char END_CHAR = 126;

    // one row per character, one column per state
    std::vector<std::vector<int>> fTransitions;

    // contains, for each state, the token id we should report when matching ends in that state (-1
    // for no match)
    std::vector<int> fAccepts;
};

#endif
