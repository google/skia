/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DFASTATE
#define SKSL_DFASTATE

#include "src/sksl/lex/LexUtil.h"

#include <vector>
#include <string>

struct DFAState {
    struct Label {
        std::vector<int> fStates;

        Label(std::vector<int> states)
        : fStates(std::move(states)) {}

        bool operator==(const Label& other) const {
            return fStates == other.fStates;
        }

        bool operator!=(const Label& other) const {
            return !(*this == other);
        }

#ifdef SK_DEBUG
        std::string description() const {
            std::string result = "<";
            const char* separator = "";
            for (int s : fStates) {
                result += separator;
                result += std::to_string(s);
                separator = ", ";
            }
            result += ">";
            return result;
        }
#endif
    };

    DFAState()
    : fId(INVALID)
    , fLabel({}) {}

    DFAState(int id, Label label)
    : fId(id)
    , fLabel(std::move(label)) {}

    DFAState(const DFAState& other) = delete;

    int fId;

    Label fLabel;

    bool fIsScanned = false;
};

namespace std {
    template<> struct hash<DFAState::Label> {
        size_t operator()(const DFAState::Label& s) const {
            size_t result = 0;
            for (int i : s.fStates) {
                result = result * 101 + i;
            }
            return result;
        }
    };
} // namespace

#endif
