/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_REGEXNODE
#define SKSL_REGEXNODE

#include <string>
#include <utility>
#include <vector>

struct NFA;

/**
 * Represents a node in the parse tree of a regular expression.
 */
struct RegexNode {
    enum Kind {
        kChar_Kind,
        kCharset_Kind,
        kConcat_Kind,
        kDot_Kind,
        kOr_Kind,
        kPlus_Kind,
        kRange_Kind,
        kQuestion_Kind,
        kStar_Kind
    };

    RegexNode(Kind kind)
    : fKind(kind) {}

    RegexNode(Kind kind, char payload)
    : fKind(kind) {
        fPayload.fChar = payload;
    }

    RegexNode(Kind kind, const char* children)
    : fKind(kind) {
        fPayload.fBool = false;
        while (*children != '\0') {
            fChildren.emplace_back(kChar_Kind, *children);
            ++children;
        }
    }

    RegexNode(Kind kind, RegexNode child)
    : fKind(kind) {
        fChildren.push_back(std::move(child));
    }

    RegexNode(Kind kind, RegexNode child1, RegexNode child2)
    : fKind(kind) {
        fChildren.push_back(std::move(child1));
        fChildren.push_back(std::move(child2));
    }

    /**
     * Creates NFA states for this node, with a successful match against this node resulting in a
     * transition to all of the states in the accept vector.
     */
    std::vector<int> createStates(NFA* nfa, const std::vector<int>& accept) const;

    std::string description() const;

    Kind fKind;

    union Payload {
        char fChar;
        bool fBool;
    } fPayload;

    std::vector<RegexNode> fChildren;
};

#endif
