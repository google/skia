/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/lex/RegexNode.h"

#include "src/sksl/lex/NFA.h"

std::vector<int> RegexNode::createStates(NFA* nfa, const std::vector<int>& accept) const {
    std::vector<int> result;
    switch (fKind) {
        case kChar_Kind:
            result.push_back(nfa->addState(NFAState(fPayload.fChar, accept)));
            break;
        case kCharset_Kind: {
            std::vector<bool> chars;
            for (const RegexNode& child : fChildren) {
                if (child.fKind == kChar_Kind) {
                    while (chars.size() <= (size_t) child.fPayload.fChar) {
                        chars.push_back(false);
                    }
                    chars[child.fPayload.fChar] = true;
                } else {
                    SkASSERT(child.fKind == kRange_Kind);
                    while (chars.size() <= (size_t) child.fChildren[1].fPayload.fChar) {
                        chars.push_back(false);
                    }
                    for (char c = child.fChildren[0].fPayload.fChar;
                         c <= child.fChildren[1].fPayload.fChar;
                         ++c) {
                        chars[c] = true;
                    }
                }
            }
            result.push_back(nfa->addState(NFAState(fPayload.fBool, chars, accept)));
            break;
        }
        case kConcat_Kind: {
            std::vector<int> right = fChildren[1].createStates(nfa, accept);
            result = fChildren[0].createStates(nfa, right);
            break;
        }
        case kDot_Kind:
            result.push_back(nfa->addState(NFAState(NFAState::kDot_Kind, accept)));
            break;
        case kOr_Kind: {
            std::vector<int> states = fChildren[0].createStates(nfa, accept);
            result.insert(result.end(), states.begin(), states.end());
            states = fChildren[1].createStates(nfa, accept);
            result.insert(result.end(), states.begin(), states.end());
            break;
        }
        case kPlus_Kind: {
            std::vector<int> next = accept;
            std::vector<int> placeholder;
            int id = nfa->addState(NFAState(placeholder));
            next.push_back(id);
            result = fChildren[0].createStates(nfa, next);
            nfa->fStates[id] = NFAState(result);
            break;
        }
        case kQuestion_Kind:
            result = fChildren[0].createStates(nfa, accept);
            result.insert(result.end(), accept.begin(), accept.end());
            break;
        case kRange_Kind:
            ABORT("unreachable");
        case kStar_Kind: {
            std::vector<int> next = accept;
            std::vector<int> placeholder;
            int id = nfa->addState(NFAState(placeholder));
            next.push_back(id);
            result = fChildren[0].createStates(nfa, next);
            result.insert(result.end(), accept.begin(), accept.end());
            nfa->fStates[id] = NFAState(result);
            break;
        }
    }
    return result;
}

#ifdef SK_DEBUG
std::string RegexNode::description() const {
    switch (fKind) {
        case kChar_Kind:
            return std::string(1, fPayload.fChar);
        case kCharset_Kind: {
            std::string result("[");
            if (fPayload.fBool) {
                result += "^";
            }
            for (const RegexNode& c : fChildren) {
                result += c.description();
            }
            result += "]";
            return result;
        }
        case kConcat_Kind:
            return fChildren[0].description() + fChildren[1].description();
        case kDot_Kind:
            return ".";
        case kOr_Kind:
            return "(" + fChildren[0].description() + "|" + fChildren[1].description() + ")";
        case kPlus_Kind:
            return fChildren[0].description() + "+";
        case kQuestion_Kind:
            return fChildren[0].description() + "?";
        case kRange_Kind:
            return fChildren[0].description() + "-" + fChildren[1].description();
        case kStar_Kind:
            return fChildren[0].description() + "*";
        default:
            return "<" + std::to_string(fKind) + ">";
    }
}
#endif
