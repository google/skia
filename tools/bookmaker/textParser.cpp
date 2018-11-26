/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "definition.h"
#include "textParser.h"

#ifdef SK_BUILD_FOR_WIN
#include <Windows.h>
#endif

TextParser::TextParser(const Definition* definition) :
    TextParser(definition->fFileName, definition->fContentStart, definition->fContentEnd,
        definition->fLineCount) {
}

string TextParser::ReportFilename(string file) {
	string fullName;
#ifdef SK_BUILD_FOR_WIN
	TCHAR pathChars[MAX_PATH];
	DWORD pathLen = GetCurrentDirectory(MAX_PATH, pathChars);
	for (DWORD index = 0; index < pathLen; ++index) {
		fullName += pathChars[index] == (char)pathChars[index] ? (char)pathChars[index] : '?';
	}
	fullName += '\\';
#endif
	fullName += file;
    return fullName;
}

void TextParser::reportError(const char* errorStr) const {
    this->reportWarning(errorStr);
    SkDebugf("");  // convenient place to set a breakpoint
}

void TextParser::reportWarning(const char* errorStr) const {
    const char* lineStart = fLine;
    if (lineStart >= fEnd) {
        lineStart = fChar;
    }
    SkASSERT(lineStart < fEnd);
    TextParser err(fFileName, lineStart, fEnd, fLineCount);
    size_t lineLen = this->lineLength();
    ptrdiff_t spaces = fChar - lineStart;
    while (spaces > 0 && (size_t) spaces > lineLen) {
        ++err.fLineCount;
        err.fLine += lineLen;
        spaces -= lineLen;
        lineLen = err.lineLength();
    }
	string fullName = this->ReportFilename(fFileName);
    SkDebugf("\n%s(%zd): error: %s\n", fullName.c_str(), err.fLineCount, errorStr);
    if (0 == lineLen) {
        SkDebugf("[blank line]\n");
    } else {
        while (lineLen > 0 && '\n' == err.fLine[lineLen - 1]) {
            --lineLen;
        }
        SkDebugf("%.*s\n", (int) lineLen, err.fLine);
        SkDebugf("%*s^\n", (int) spaces, "");
    }
}

void TextParser::setForErrorReporting(const Definition* definition, const char* str) {
    fFileName = definition->fFileName;
    fStart = definition->fContentStart;
    fLine = str;
    while (fLine > fStart && fLine[-1] != '\n') {
        --fLine;
    }
    fChar = str;
    fEnd = definition->fContentEnd;
    fLineCount = definition->fLineCount;
    const char* lineInc = fStart;
    while (lineInc < str) {
        fLineCount += '\n' == *lineInc++;
    }
}

string TextParser::typedefName() {
    // look for typedef as one of three forms:
    // typedef return-type (*NAME)(params);
    // typedef alias NAME;
    // typedef std::function<alias> NAME;
    string builder;
    const char* end = this->doubleLF();
    if (!end) {
       end = fEnd;
    }
    const char* altEnd = this->strnstr("#Typedef ##", end);
    if (altEnd) {
        end = this->strnchr('\n', end);
    }
    if (!end) {
        return this->reportError<string>("missing typedef std::function end bracket >");
    }
    bool stdFunction = this->startsWith("std::function");
    if (stdFunction) {
        if (!this->skipToEndBracket('>')) {
            return this->reportError<string>("missing typedef std::function end bracket >");
        }
        this->next();
        this->skipWhiteSpace();
        builder += string(fChar, end - fChar);
    } else {
        const char* paren = this->strnchr('(', end);
        if (!paren) {
            const char* lastWord = nullptr;
            do {
                this->skipToWhiteSpace();
                if (fChar < end && isspace(fChar[0])) {
                    const char* whiteStart = fChar;
                    this->skipWhiteSpace();
                    // FIXME: test should be for fMC
                    if ('#' == fChar[0]) {
                        end = whiteStart;
                        break;
                    }
                    lastWord = fChar;
                } else {
                    break;
                }
            } while (true);
            if (!lastWord) {
                return this->reportError<string>("missing typedef name");
            }
            builder += string(lastWord, end - lastWord);
        } else {
            this->skipTo(paren);
            this->next();
            if ('*' != this->next()) {
                return this->reportError<string>("missing typedef function asterisk");
            }
            const char* nameStart = fChar;
            if (!this->skipToEndBracket(')')) {
                return this->reportError<string>("missing typedef function )");
            }
            builder += string(nameStart, fChar - nameStart);
            if (!this->skipToEndBracket('(')) {
                return this->reportError<string>("missing typedef params (");
            }
            if (! this->skipToEndBracket(')')) {
                return this->reportError<string>("missing typedef params )");
            }
            this->skipTo(end);
        }
    }
    return builder;
}

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
            Resolvable::kCode == resolvable && this->skipExact(" const");
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
                this->skipExact(" const");
            }
        }
    }
}
