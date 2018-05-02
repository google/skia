/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bookmaker.h"

bool FiddleBase::parseFiddles() {
    if (!this->skipExact("{\n")) {
        return false;
    }
    while (!this->eof()) {
        if (!this->skipExact("  \"")) {
            return false;
        }
        const char* nameLoc = fChar;
        if (!this->skipToEndBracket("\"")) {
            return false;
        }
        string name(nameLoc, fChar - nameLoc);
        if (!this->skipExact("\": {\n")) {
            return false;
        }
        if (!this->skipExact("    \"compile_errors\": [")) {
            return false;
        }
        if (']' != this->peek()) {
            // report compiler errors
            int brackets = 1;
            do {
                if ('[' == this->peek()) {
                    ++brackets;
                } else if (']' == this->peek()) {
                    --brackets;
                }
            } while (!this->eof() && this->next() && brackets > 0);
            this->reportError("fiddle compile error");
        }
        if (!this->skipExact("],\n")) {
            return false;
        }
        if (!this->skipExact("    \"runtime_error\": \"")) {
            return false;
        }
        if ('"' != this->peek()) {
            if (!this->skipToEndBracket('"')) {
                return false;
            }
            this->reportError("fiddle runtime error");
        }
        if (!this->skipExact("\",\n")) {
            return false;
        }
        if (!this->skipExact("    \"fiddleHash\": \"")) {
            return false;
        }
        const char* hashStart = fChar;
        if (!this->skipToEndBracket('"')) {
            return false;
        }
        Definition* example = this->findExample(name);
        if (!example) {
            this->reportError("missing example");
        }
        string hash(hashStart, fChar - hashStart);
        if (example) {
            example->fHash = hash;
        }
        if (!this->skipExact("\",\n")) {
            return false;
        }
        if (!this->skipExact("    \"text\": \"")) {
            return false;
        }
        if ('"' != this->peek()) {
            const char* stdOutStart = fChar;
            do {
                if ('\\' == this->peek()) {
                    this->next();
                } else if ('"' == this->peek()) {
                    break;
                }
            } while (!this->eof() && this->next());
            const char* stdOutEnd = fChar;
            if (example && fTextOut) {
                if (!this->textOut(example, stdOutStart, stdOutEnd)) {
                    return false;
                }
            }
        } else {
            if (example && fPngOut) {
                if (!this->pngOut(example)) {
                    return false;
                }
            }
        }
        if (!this->skipExact("\"\n")) {
            return false;
        }
        if (!this->skipExact("  }")) {
            return false;
        }
        if ('\n' == this->peek()) {
            break;
        }
        if (!this->skipExact(",\n")) {
            return false;
        }
    }
    return true;
}

bool FiddleParser::textOut(Definition* example, const char* stdOutStart,
        const char* stdOutEnd) {
    bool foundStdOut = false;
    for (auto& textOut : example->fChildren) {
        if (MarkType::kStdOut != textOut->fMarkType) {
            continue;
        }
        foundStdOut = true;
        bool foundVolatile = false;
        for (auto& stdOutChild : textOut->fChildren) {
                if (MarkType::kVolatile == stdOutChild->fMarkType) {
                    foundVolatile = true;
                    break;
                }
        }
        TextParser bmh(textOut);
        EscapeParser fiddle(stdOutStart, stdOutEnd);
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
                if (!foundVolatile) {
                    bmh.reportError("mismatched stdout len\n");
                }
            } else  if (strncmp(bmh.fChar, fiddle.fChar, fiddleLen)) {
                if (!foundVolatile) {
                    bmh.reportError("mismatched stdout text\n");
                }
            }
            bmh.skipToLineStart();
            fiddle.skipToLineStart();
        } while (!bmh.eof() && !fiddle.eof());
        if (!foundStdOut) {
            bmh.reportError("bmh %s missing stdout\n");
        } else if (!bmh.eof() || !fiddle.eof()) {
            if (!foundVolatile) {
                bmh.reportError("%s mismatched stdout eof\n");
            }
        }
    }
    return true;
}
