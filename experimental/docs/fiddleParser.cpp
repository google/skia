/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bookmaker.h"

static Definition* find_fiddle(Definition* def, const string& name) {
    if (MarkType::kExample == def->fMarkType && name == def->fFiddle) {
        return def;
    }
    for (auto& child : def->fChildren) {
        Definition* result = find_fiddle(child, name);
        if (result) {
            return result;
        }
    }
    return nullptr;
}

Definition* FiddleParser::findExample(const string& name) const {
    for (auto& oneclass : fBmhParser->fClassMap) {
        Definition* def = find_fiddle(&oneclass.second, name);
        if (def) {
            return def;
        }
    }
    return nullptr;
}

bool FiddleParser::parseFiddles() {
    while (!this->eof()) {
        if (this->startsWith("fiddles.htm")) {
            break;
        }
        if (!this->skipToLineStart()) {
            return this->reportError<bool>("unexpected end of fiddle output");
        }
    }
    while (!this->eof()) {
        if (!this->startsWith("fiddles.htm")) {
            break;
        }
        const char* nameLoc;
        const char* hashLoc;
        const char* end = this->lineEnd();
        const char nameStr[] = "name:";
        const char hashStr[] = "fiddleHash:";
        if (this->contains(nameStr, end, &nameLoc)
                && this->contains(hashStr, end, &hashLoc)) {
            // store the generated hash in the markup tree
            fChar = nameLoc + sizeof(nameStr) - 1;
            this->skipSpace();
            const char* nameEnd = this->wordEnd();
            string name(fChar, nameEnd - fChar);
            Definition* example = this->findExample(name);
            if (!example) {
                return this->reportError<bool>("missing example");
            }
            fChar = hashLoc + sizeof(hashStr) - 1;
            this->skipSpace();
            const char* hashEnd = this->wordEnd();
            string hash(fChar, hashEnd - fChar);
            example->fHash = hash;
            if (!this->skipToLineStart()) {
                return this->reportError<bool>("unexpected end of fiddle output");
            }
            continue;
        } else {
            // compare the text output with the expected output in the markup tree
            this->skipToSpace();
            SkASSERT(' ' == fChar[0]);
            this->next();
            const char* nameLoc = fChar;
            this->skipToNonAlphaNum();
            const char* nameEnd = fChar;
            string name(nameLoc, nameEnd - nameLoc);
            const Definition* example = this->findExample(name);
            if (!example) {
                return this->reportError<bool>("missing stdout name");
            }
            SkASSERT(':' == fChar[0]);
            this->next();
            this->skipSpace();
            const char* stdOutLoc = fChar;
            do {
                this->skipToLineStart();
            } while (!this->eof() && !this->startsWith("fiddles.htm:"));
            const char* stdOutEnd = fChar;
            for (auto& textOut : example->fChildren) {
                if (MarkType::kStdOut != textOut->fMarkType) {
                    continue;
                }
                TextParser bmh(textOut->fContentStart, textOut->fContentEnd);
                TextParser fiddle(stdOutLoc, stdOutEnd);
                do {
                    bmh.skipWhiteSpace();
                    fiddle.skipWhiteSpace();
                    const char* bmhEnd = bmh.trimmedLineEnd();
                    const char* fiddleEnd = fiddle.trimmedLineEnd();
                    ptrdiff_t bmhLen = bmhEnd - bmh.fChar;
                    SkASSERT(bmhLen > 0);
                    ptrdiff_t fiddleLen = fiddleEnd - fiddle.fChar;
                    SkASSERT(fiddleLen > 0);
                    if (bmhLen != fiddleLen) {
                        return this->reportError<bool>("mismatched stdout len");
                    }
                    if (strncmp(bmh.fChar, fiddle.fChar, fiddleLen)) {
                        return this->reportError<bool>("mismatched stdout text");
                    }
                    bmh.skipToLineStart();
                    fiddle.skipToLineStart();
                } while (!bmh.eof() && !fiddle.eof());
                if (!bmh.eof() || (!fiddle.eof() && !fiddle.startsWith("</pre>"))) {
                    return this->reportError<bool>("mismatched stdout eof");
                }
                break;
            }
        }
    }
    return true;
}
