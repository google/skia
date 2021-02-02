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

#include "src/sksl/lex/LexUtil.h"

struct NFAState {
    enum Kind {
        // represents an accept state - if the NFA ends up in this state, we have successfully
        // matched the token indicated by fData[0]
        kAccept_Kind,
        // matches the single character fChar
        kChar_Kind,
        // the regex '.'; matches any char but '\n'
        kDot_Kind,
        // a state which serves as a placeholder for the states indicated in fData. When we
        // transition to this state, we instead transition to all of the fData states.
        kRemapped_Kind,
        // contains a list of true/false values in fData. fData[c] tells us whether we accept the
        // character c.
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
                SkUNREACHABLE;
        }
    }

#ifdef SK_DEBUG
    std::string description() const {
        switch (fKind) {
            case kAccept_Kind:
                return "Accept(" + std::to_string(fData[0]) + ")";
            case kChar_Kind: {
                std::string result = "Char('" + std::string(1, fChar) + "'";
                for (int v : fNext) {
                    result += ", ";
                    result += std::to_string(v);
                }
                result += ")";
                return result;
            }
            case kDot_Kind: {
                std::string result = "Dot(";
                const char* separator = "";
                for (int v : fNext) {
                    result += separator;
                    result += std::to_string(v);
                    separator = ", ";
                }
                result += ")";
                return result;
            }
            case kRemapped_Kind: {
                std::string result = "Remapped(";
                const char* separator = "";
                for (int v : fData) {
                    result += separator;
                    result += std::to_string(v);
                    separator = ", ";
                }
                result += ")";
                return result;
            }
            case kTable_Kind: {
                std::string result = std::string("Table(") + (fInverse ? "true" : "false") + ", [";
                const char* separator = "";
                for (int v : fData) {
                    result += separator;
                    result += v ? "true" : "false";
                    separator = ", ";
                }
                result += "]";
                for (int n : fNext) {
                    result += ", ";
                    result += std::to_string(n);
                }
                result += ")";
                return result;
            }
            default:
                SkUNREACHABLE;
        }
    }
#endif

    Kind fKind;

    char fChar = 0;

    bool fInverse = false;

    std::vector<int> fData;

    // states we transition to upon a succesful match from this state
    std::vector<int> fNext;
};

#endif
