/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_REGEXPARSER
#define SKSL_REGEXPARSER

#include "src/sksl/lex/RegexNode.h"

#include <stack>
#include <string>

/**
 * Turns a simple regular expression into a parse tree. The regular expression syntax supports only
 * the basic quantifiers ('*', '+', and '?'), alternation ('|'), character sets ('[a-z]'), and
 * groups ('()').
 */
class RegexParser {
public:
    RegexNode parse(std::string source);

private:
    static constexpr char END = '\0';

    char peek();

    void expect(char c);

    RegexNode pop();

    /**
     * Matches a char literal, parenthesized group, character set, or dot ('.').
     */
    void term();

    /**
     * Matches a term followed by an optional quantifier ('*', '+', or '?').
     */
    void quantifiedTerm();

    /**
     * Matches a sequence of quantifiedTerms.
     */
    void sequence();

    /**
     * Returns a node representing the given escape character (e.g. escapeSequence('n') returns a
     * node which matches a newline character).
     */
    RegexNode escapeSequence(char c);

    /**
     * Matches a literal character or escape sequence.
     */
    void literal();

    /**
     * Matches a dot ('.').
     */
    void dot();

    /**
     * Matches a parenthesized group.
     */
    void group();

    /**
     * Matches a literal character, escape sequence, or character range from a character set.
     */
    void setItem();

    /**
     * Matches a character set.
     */
    void set();

    void regex();

    std::string fSource;

    size_t fIndex;

    std::stack<RegexNode> fStack;
};

#endif
