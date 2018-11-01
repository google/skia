/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "textParser.h"

void MethodParser::skipToMethodEnd(Resolvable resolvable) {
    if (this->eof()) {
        return;
    }
    string name = fLocalName.length() ? fLocalName : fClassName;
    if ('~' == this->peek()) {
        this->next();
        if (!this->startsWith(name.c_str())) {
            --fChar;
            return;
        }
    }
    if (Resolvable::kSimple != resolvable
            && Resolvable::kInclude != resolvable
            && (this->startsWith(name.c_str()) || this->startsWith("operator"))) {
        const char* ptr = this->anyOf("\n (");
        if (ptr && '(' ==  *ptr && strncmp(ptr, "(...", 4)) {
            this->skipToEndBracket(')');
            SkAssertResult(')' == this->next());
            this->skipExact("_const") || (Resolvable::kCode == resolvable
                    && this->skipExact(" const"));
            return;
        }
    }
    if (this->startsWith("Sk") && this->wordEndsWith(".h")) {  // allow include refs
        this->skipToNonName();
    } else {
        this->skipFullName();
        if (this->endsWith("operator")) {
            const char* ptr = this->anyOf("\n (");
            if (ptr && '(' ==  *ptr) {
                this->skipToEndBracket(')');
                SkAssertResult(')' == this->next());
                this->skipExact("_const");
            }
        }
    }
}
