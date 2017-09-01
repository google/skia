/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DFASTATE
#define SKSL_DFASTATE

struct DFAState {
    struct Label {
        std::vector<int> fStates;

        Label(std::vector<int> states)
        : fStates(std::move(states)) {}

        bool operator==(const Label& other) const {
            if (fStates.size() != other.fStates.size()) {
                return false;
            }
            for (size_t i = 0; i < fStates.size(); ++i) {
                if (fStates[i] != other.fStates[i]) {
                    return false;
                }
            }
            return true;
        }

        bool operator!=(const Label& other) const {
            return !(*this == other);
        }
    };

    DFAState()
    : fId(-1)
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
            int result = 0;
            for (int i : s.fStates) {
                result = result * 101 + i;
            }
            return result;
        }
    };
} // namespace

#endif
