/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/lex/RegexParser.h"

#include "src/sksl/lex/LexUtil.h"

RegexNode RegexParser::parse(std::string source) {
    fSource = source;
    fIndex = 0;
    SkASSERT(fStack.size() == 0);
    this->regex();
    SkASSERT(fStack.size() == 1);
    SkASSERT(fIndex == source.size());
    return this->pop();
}

char RegexParser::peek() {
    if (fIndex >= fSource.size()) {
        return END;
    }
    return fSource[fIndex];
}

void RegexParser::expect(char c) {
    if (this->peek() != c) {
        printf("expected '%c' at index %d, but found '%c'", c, (int) fIndex, this->peek());
        exit(1);
    }
    ++fIndex;
}

RegexNode RegexParser::pop() {
    RegexNode result = fStack.top();
    fStack.pop();
    return result;
}

void RegexParser::term() {
    switch (this->peek()) {
        case '(': this->group();  break;
        case '[': this->set();    break;
        case '.': this->dot();    break;
        default: this->literal(); break;
    }
}

void RegexParser::quantifiedTerm() {
    this->term();
    switch (this->peek()) {
        case '*': fStack.push(RegexNode(RegexNode::kStar_Kind,     this->pop())); ++fIndex; break;
        case '+': fStack.push(RegexNode(RegexNode::kPlus_Kind,     this->pop())); ++fIndex; break;
        case '?': fStack.push(RegexNode(RegexNode::kQuestion_Kind, this->pop())); ++fIndex; break;
        default:  break;
    }
}

void RegexParser::sequence() {
    this->quantifiedTerm();
    for (;;) {
        switch (this->peek()) {
            case END: [[fallthrough]];
            case '|': [[fallthrough]];
            case ')': return;
            default:
                this->sequence();
                RegexNode right = this->pop();
                RegexNode left = this->pop();
                fStack.emplace(RegexNode::kConcat_Kind, std::move(left), std::move(right));
                break;
        }
    }
}

RegexNode RegexParser::escapeSequence(char c) {
    switch (c) {
        case 'n': return RegexNode(RegexNode::kChar_Kind, '\n');
        case 'r': return RegexNode(RegexNode::kChar_Kind, '\r');
        case 't': return RegexNode(RegexNode::kChar_Kind, '\t');
        case 's': return RegexNode(RegexNode::kCharset_Kind, " \t\n\r");
        default:  return RegexNode(RegexNode::kChar_Kind, c);
    }
}

void RegexParser::literal() {
    char c = this->peek();
    if (c == '\\') {
        ++fIndex;
        fStack.push(this->escapeSequence(peek()));
        ++fIndex;
    }
    else {
        fStack.push(RegexNode(RegexNode::kChar_Kind, c));
        ++fIndex;
    }
}

void RegexParser::dot() {
    this->expect('.');
    fStack.push(RegexNode(RegexNode::kDot_Kind));
}

void RegexParser::group() {
    this->expect('(');
    this->regex();
    this->expect(')');
}

void RegexParser::setItem() {
    this->literal();
    if (this->peek() == '-') {
        ++fIndex;
        if (peek() == ']') {
            fStack.push(RegexNode(RegexNode::kChar_Kind, '-'));
        }
        else {
            literal();
            RegexNode end = this->pop();
            SkASSERT(end.fKind == RegexNode::kChar_Kind);
            RegexNode start = this->pop();
            SkASSERT(start.fKind == RegexNode::kChar_Kind);
            fStack.push(RegexNode(RegexNode::kRange_Kind, std::move(start), std::move(end)));
        }
    }
}

void RegexParser::set() {
    expect('[');
    size_t depth = fStack.size();
    RegexNode set(RegexNode::kCharset_Kind);
    if (this->peek() == '^') {
        ++fIndex;
        set.fPayload.fBool = true;
    }
    else {
        set.fPayload.fBool = false;
    }
    for (;;) {
        switch (this->peek()) {
            case ']':
                ++fIndex;
                while (fStack.size() > depth) {
                    set.fChildren.push_back(this->pop());
                }
                fStack.push(std::move(set));
                return;
            case END:
                printf("unterminated character set\n");
                exit(1);
            default:
                this->setItem();
                break;
        }
    }
}

void RegexParser::regex() {
    this->sequence();
    switch (this->peek()) {
        case '|': {
            ++fIndex;
            this->regex();
            RegexNode right = this->pop();
            RegexNode left = this->pop();
            fStack.push(RegexNode(RegexNode::kOr_Kind, left, right));
            break;
        }
        case END: // fall through
        case ')':
            return;
        default:
            SkASSERT(false);
    }
}
