/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef NFAtoDFA_DEFINED
#define NFAtoDFA_DEFINED

#include "src/sksl/lex/DFA.h"
#include "src/sksl/lex/DFAState.h"
#include "src/sksl/lex/NFA.h"
#include "src/sksl/lex/NFAState.h"

#include <algorithm>
#include <climits>
#include <memory>
#include <unordered_map>
#include <set>
#include <vector>

/**
 * Converts a nondeterministic finite automaton to a deterministic finite automaton. Since NFAs and
 * DFAs differ only in that an NFA allows multiple states at the same time, we can find each
 * possible combination of simultaneous NFA states and give this combination a label. These labelled
 * nodes are our DFA nodes, since we can only be in one such unique set of NFA states at a time.
 *
 * As an NFA can end up in multiple accept states at the same time (for instance, the token "while"
 * is valid for both WHILE and IDENTIFIER), we disambiguate by preferring the first matching regex
 * (in terms of the order in which they were added to the NFA).
 */
class NFAtoDFA {
public:
    inline static constexpr char START_CHAR = 9;
    inline static constexpr char END_CHAR = 126;

    NFAtoDFA(NFA* nfa)
    : fNFA(*nfa) {}

    /**
     * Returns a DFA created from the NFA.
     */
    DFA convert() {
        // create state 0, the "reject" state
        getState(DFAState::Label({}));
        // create a state representing being in all of the NFA's start states at once
        std::vector<int> startStates = fNFA.fStartStates;
        std::sort(startStates.begin(), startStates.end());
        // this becomes state 1, our start state
        DFAState* start = getState(DFAState::Label(startStates));
        this->scanState(start);

        this->computeMappings();

        int stateCount = 0;
        for (const auto& row : fTransitions) {
            stateCount = std::max(stateCount, (int) row.size());
        }
        return DFA(fCharMappings, fTransitions, fAccepts);
    }

private:
    /**
     * Returns an existing state with the given label, or creates a new one and returns it.
     */
    DFAState* getState(DFAState::Label label) {
        auto found = fStates.find(label);
        if (found == fStates.end()) {
            int id = fStates.size();
            fStates[label] = std::unique_ptr<DFAState>(new DFAState(id, label));
            return fStates[label].get();
        }
        return found->second.get();
    }

    void add(int nfaState, std::vector<int>* states) {
        NFAState state = fNFA.fStates[nfaState];
        if (state.fKind == NFAState::kRemapped_Kind) {
            for (int next : state.fData) {
                this->add(next, states);
            }
        } else {
            for (int entry : *states) {
                if (nfaState == entry) {
                    return;
                }
            }
            states->push_back(nfaState);
        }
    }

    void addTransition(char c, int start, int next) {
        while (fTransitions.size() <= (size_t) c) {
            fTransitions.push_back(std::vector<int>());
        }
        std::vector<int>& row = fTransitions[c];
        while (row.size() <= (size_t) start) {
            row.push_back(INVALID);
        }
        row[start] = next;
    }

    void scanState(DFAState* state) {
        state->fIsScanned = true;
        for (char c = START_CHAR; c <= END_CHAR; ++c) {
            std::vector<int> next;
            int bestAccept = INT_MAX;
            for (int idx : state->fLabel.fStates) {
                const NFAState& nfaState = fNFA.fStates[idx];
                if (nfaState.accept(c)) {
                    for (int nextState : nfaState.fNext) {
                        if (fNFA.fStates[nextState].fKind == NFAState::kAccept_Kind) {
                            bestAccept = std::min(bestAccept, fNFA.fStates[nextState].fData[0]);
                        }
                        this->add(nextState, &next);
                    }
                }
            }
            std::sort(next.begin(), next.end());
            DFAState* nextState = this->getState(DFAState::Label(next));
            this->addTransition(c, state->fId, nextState->fId);
            if (bestAccept != INT_MAX) {
                while (fAccepts.size() <= (size_t) nextState->fId) {
                    fAccepts.push_back(INVALID);
                }
                fAccepts[nextState->fId] = bestAccept;
            }
            if (!nextState->fIsScanned) {
                this->scanState(nextState);
            }
        }
    }

    // collapse rows with the same transitions to a single row. This is common, as each row
    // represents a character and often there are many characters for which all transitions are
    // identical (e.g. [0-9] are treated the same way by all lexer rules)
    void computeMappings() {
        // mappings[<input row>] = <output row>
        std::vector<std::vector<int>*> uniques;
        // this could be done more efficiently, but O(n^2) is plenty fast for our purposes
        for (size_t i = 0; i < fTransitions.size(); ++i) {
            int found = -1;
            for (size_t j = 0; j < uniques.size(); ++j) {
                if (*uniques[j] == fTransitions[i]) {
                    found = j;
                    break;
                }
            }
            if (found == -1) {
                found = (int) uniques.size();
                uniques.push_back(&fTransitions[i]);
            }
            fCharMappings.push_back(found);
        }
        std::vector<std::vector<int>> newTransitions;
        for (std::vector<int>* row : uniques) {
            newTransitions.push_back(*row);
        }
        fTransitions = newTransitions;
    }

    const NFA& fNFA;
    std::unordered_map<DFAState::Label, std::unique_ptr<DFAState>> fStates;
    std::vector<std::vector<int>> fTransitions;
    std::vector<int> fCharMappings;
    std::vector<int> fAccepts;
};
#endif  // NFAtoDFA_DEFINED
