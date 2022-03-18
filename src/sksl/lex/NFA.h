/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_NFA
#define SKSL_NFA

#include "src/sksl/lex/NFAState.h"
#include "src/sksl/lex/RegexNode.h"

#include <string>
#include <utility>
#include <vector>

/**
 * A nondeterministic finite automaton for matching regular expressions. The NFA is initialized with
 * a number of regular expressions, and then matches a string against all of them simultaneously.
 */
struct NFA {
    /**
     * Adds a new regular expression to the set of expressions matched by this automaton, returning
     * its index.
     */
    int addRegex(const RegexNode& regex) {
        std::vector<int> accept;
        // we reserve token 0 for END_OF_FILE, so this starts at 1
        accept.push_back(this->addState(NFAState(++fRegexCount)));
        std::vector<int> startStates = regex.createStates(this, accept);
        fStartStates.insert(fStartStates.end(), startStates.begin(), startStates.end());
        return fStartStates.size() - 1;
    }

    /**
     * Adds a new state to the NFA, returning its index.
     */
    int addState(NFAState s) {
        fStates.push_back(std::move(s));
        return fStates.size() - 1;
    }

    /**
     * Matches a string against all of the regexes added to this NFA. Returns the index of the first
     * (in addRegex order) matching expression, or -1 if no match. This is relatively slow and used
     * only for debugging purposes; the NFA should be converted to a DFA before actual use.
     */
    int match(std::string s) const;

    int fRegexCount = 0;

    std::vector<NFAState> fStates;

    std::vector<int> fStartStates;
};

#endif
