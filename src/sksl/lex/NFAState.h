/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_NFASTATE
#define SKSL_NFASTATE

#include <string>
#include <vector>

#include "LexUtil.h"

struct NFAState {
    enum Kind {
        kAccept_Kind,
        kChar_Kind,
        kDot_Kind,
        kRemapped_Kind,
        kTable_Kind
    };

    NFAState(Kind kind, std::vector<int> next)
    : fKind(kind)
    , fNext(std::move(next)) {}

    NFAState(char c, std::vector<int> next)
    : fKind(kChar_Kind)
    , fChar(c)
    , fNext(std::move(next)) {}

    NFAState(std::vector<int> states)
    : fKind(kRemapped_Kind)
    , fData(std::move(states)) {}

    NFAState(bool inverse, std::vector<bool> accepts, std::vector<int> next)
    : fKind(kTable_Kind)
    , fInverse(inverse)
    , fNext(std::move(next)) {
        for (bool b : accepts) {
            fData.push_back(b);
        }
    }

    NFAState(int token)
    : fKind(kAccept_Kind) {
        fData.push_back(token);
    }

    bool accept(char c) const {
        switch (fKind) {
            case kAccept_Kind:
                return false;
            case kChar_Kind:
                return c == fChar;
            case kDot_Kind:
                return c != '\n';
            case kTable_Kind: {
                bool value;
                if ((size_t) c < fData.size()) {
                    value = fData[c];
                } else {
                    value = false;
                }
                return value != fInverse;
            }
            default:
                ABORT("unreachable");
        }
    }

    std::string description() const {
        switch (fKind) {
            case kAccept_Kind:
                return "Accept(" + std::to_string(fData[0]) + ")";
            case kChar_Kind:
                return "Char(" + std::string(1, fChar) + ")";
            case kDot_Kind:
                return "Dot";
            case kRemapped_Kind:
                return "Remapped";
            case kTable_Kind:
                return "Table";
            default:
                ABORT("unreachable");
        }
    }

    Kind fKind;

    char fChar = 0;

    bool fInverse = false;

    std::vector<int> fData;

    // states we transition to upon a succesful match from this state
    std::vector<int> fNext;
};

#endif
