/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bookmaker.h"

#include "SkOSFile.h"
#include "SkOSPath.h"

static void debug_out(int len, const char* data) {
    // convenient place to intercept arbitrary output
    SkDebugf("%.*s", len, data);
}

bool ParserCommon::parseFile(const char* fileOrPath, const char* suffix) {
    if (!sk_isdir(fileOrPath)) {
        if (!this->parseFromFile(fileOrPath)) {
            SkDebugf("failed to parse %s\n", fileOrPath);
            return false;
        }
    } else {
        SkOSFile::Iter it(fileOrPath, suffix);
        for (SkString file; it.next(&file); ) {
            SkString p = SkOSPath::Join(fileOrPath, file.c_str());
            const char* hunk = p.c_str();
            if (!SkStrEndsWith(hunk, suffix)) {
                continue;
            }
            if (!this->parseFromFile(hunk)) {
                SkDebugf("failed to parse %s\n", hunk);
                return false;
            }
        }
    }
    return true;
}

bool ParserCommon::parseStatus(const char* statusFile, const char* suffix, StatusFilter filter) {
    StatusIter iter(statusFile, suffix, filter);
    for (SkString file; iter.next(&file); ) {
        SkString p = SkOSPath::Join(iter.baseDir(), file.c_str());
        const char* hunk = p.c_str();
        if (!this->parseFromFile(hunk)) {
            SkDebugf("failed to parse %s\n", hunk);
            return false;
        }
    }
    return true;
}

bool ParserCommon::parseSetup(const char* path) {
    sk_sp<SkData> data = SkData::MakeFromFileName(path);
    if (nullptr == data.get()) {
        SkDebugf("%s missing\n", path);
        return false;
    }
    const char* rawText = (const char*) data->data();
    bool hasCR = false;
    size_t dataSize = data->size();
    for (size_t index = 0; index < dataSize; ++index) {
        if ('\r' == rawText[index]) {
            hasCR = true;
            break;
        }
    }
    string name(path);
    if (hasCR) {
        vector<char> lfOnly;
        for (size_t index = 0; index < dataSize; ++index) {
            char ch = rawText[index];
            if ('\r' == rawText[index]) {
                ch = '\n';
                if ('\n' == rawText[index + 1]) {
                    ++index;
                }
            }
            lfOnly.push_back(ch);
        }
        fLFOnly[name] = lfOnly;
        dataSize = lfOnly.size();
        rawText = &fLFOnly[name].front();
    }
    fRawData[name] = data;
    fStart = rawText;
    fLine = rawText;
    fChar = rawText;
    fEnd = rawText + dataSize;
    fFileName = string(path);
    fLineCount = 1;
    return true;
}

void ParserCommon::writeBlockIndent(int size, const char* data) {
    while (size && ' ' >= data[size - 1]) {
        --size;
    }
    bool newLine = false;
    while (size) {
        while (size && ' ' > data[0]) {
            ++data;
            --size;
        }
        if (!size) {
            return;
        }
        if (newLine) {
            this->lf(1);
        }
        TextParser parser(fFileName, data, data + size, fLineCount);
        const char* lineEnd = parser.strnchr('\n', data + size);
        int len = lineEnd ? (int) (lineEnd - data) : size;
        this->writePending();
        this->indentToColumn(fIndent);
        if (fDebugOut) {
            debug_out(len, data);
        }
        fprintf(fOut, "%.*s", len, data);
        size -= len;
        data += len;
        newLine = true;
    }
}

bool ParserCommon::writeBlockTrim(int size, const char* data) {
    if (fOutdentNext) {
        fIndent -= 4;
        fOutdentNext = false;
    }
    while (size && ' ' >= data[0]) {
        ++data;
        --size;
    }
    while (size && ' ' >= data[size - 1]) {
        --size;
    }
    if (size <= 0) {
        fLastChar = '\0';
        return false;
    }
    SkASSERT(size < 16000);
    if (size > 3 && !strncmp("#end", data, 4)) {
        fMaxLF = 1;
    }
    if (this->leadingPunctuation(data, (size_t) size)) {
        fPendingSpace = 0;
    }
    this->writePending();
    if (fDebugOut) {
        debug_out(size, data);
    }
    fprintf(fOut, "%.*s", size, data);
    int added = 0;
    fLastChar = data[size - 1];
    while (size > 0 && '\n' != data[--size]) {
        ++added;
    }
    fColumn = size ? added : fColumn + added;
    fSpaces = 0;
    fLinefeeds = 0;
    fMaxLF = added > 2 && !strncmp("#if", &data[size + (size > 0)], 3) ? 1 : 2;
    if (fOutdentNext) {
        fIndent -= 4;
        fOutdentNext = false;
    }
    return true;
}

void ParserCommon::writePending() {
    fPendingLF = SkTMin(fPendingLF, fMaxLF);
    bool wroteLF = false;
    while (fLinefeeds < fPendingLF) {
        if (fDebugOut) {
            SkDebugf("\n");
        }
        fprintf(fOut, "\n");
        ++fLinefeeds;
        wroteLF = true;
    }
    fPendingLF = 0;
    if (wroteLF) {
        SkASSERT(0 == fColumn);
        SkASSERT(fIndent >= fSpaces);
        if (fDebugOut) {
            SkDebugf("%*s", fIndent - fSpaces, "");
        }
        fprintf(fOut, "%*s", fIndent - fSpaces, "");
        fColumn = fIndent;
        fSpaces = fIndent;
    }
    for (int index = 0; index < fPendingSpace; ++index) {
        if (fDebugOut) {
            SkDebugf(" ");
        }
        fprintf(fOut, " ");
        ++fColumn;
    }
    fPendingSpace = 0;
}

void ParserCommon::writeString(const char* str) {
    const size_t len = strlen(str);
    SkASSERT(len > 0);
    SkASSERT(' ' < str[0]);
    fLastChar = str[len - 1];
    SkASSERT(' ' < fLastChar);
    SkASSERT(!strchr(str, '\n'));
    if (this->leadingPunctuation(str, strlen(str))) {
        fPendingSpace = 0;
    }
    this->writePending();
    if (fDebugOut) {
        debug_out((int) strlen(str), str);
    }
    fprintf(fOut, "%s", str);
    fColumn += len;
    fSpaces = 0;
    fLinefeeds = 0;
    fMaxLF = 2;
}

StatusIter::StatusIter(const char* statusFile, const char* suffix, StatusFilter filter)
    : fSuffix(suffix)
    , fFilter(filter) {
    if (!this->parseFromFile(statusFile)) {
        return;
    }
}

// FIXME: need to compare fBlockName against fFilter
// need to compare fSuffix against next value returned
bool StatusIter::next(SkString* str) {
    if (this->eof()) {
        return false;
    }
    char bracket = this->peek();
    if ('"' == bracket) {
        const char* valueLoc = fChar;
        if (!this->skipToEndBracket("\"")) {
            return false;
        }
        string value = string(valueLoc, fChar - valueLoc);
        TextParser::next();
        this->skipWhiteSpace();
        bracket = this->peek();
        if (',' == bracket) {
            TextParser::next();
            this->skipWhiteSpace();
            return '"' == this->peek();
        }
        if (']' != bracket) {
            return false;
        }
        do {
            if (--fDepth < 0) {

                return false;
            }
            // pop fBaseDir by a slash
            size_t lastSlash = fBaseDir.find_last_of(SkOSPath::SEPARATOR);
            if (string::npos == lastSlash || lastSlash == 0) {
                return false;
            }
            fBaseDir = fBaseDir.substr(0, lastSlash);
            if (SkOSPath::SEPARATOR == fBaseDir.back()) {
                return false;
            }
            this->skipWhiteSpace();
            if (',' == this->peek()) {
                if (fDepth < 0) {
                    start here;  // switch block name

                }
                if (fDepth >= 0) {
                    if (!this->startDirectory()) {
                        return false;
                    }
                    break;
                }
            }
            if ('}' != this->peek()) {
                return false;
            }
        } while (true);
        this->next();  // skip comma
    } 
    return true;
}

bool StatusIter::parseFromFile(const char* path) {
    if (!this->parseSetup(path)) {
        return false;
    }
    if (this->eof()) {
        return false;
    }
    if (!this->skipExact("{")) {
        return false;
    }
    return this->startBlock();
}

bool StatusIter::startBlock() {
    if (this->eof()) {
        return false;
    }
    this->skipWhiteSpace();
    if ('"' != TextParser::next()) {
        return false;
    }
    const char* nameLoc = fChar;
    if (!this->skipToEndBracket("\"")) {
        return false;
    }
    fBlockName = string(nameLoc, fChar - nameLoc);
    if (!this->skipExact("\": {")) {
        return false;
    }
    return this->startDirectory();
}

bool StatusIter::startDirectory() {
    // each nested name from here is a directory
    fDepth = 0;
    fBaseDir = "";
    do {
        this->skipWhiteSpace();
        if ('"' != TextParser::next()) {
            return false;
        }
        const char* dirLoc = fChar;
        if (!this->skipToEndBracket("\"")) {
            return false;
        }
        string dir(dirLoc, fChar - dirLoc);
        if (fBaseDir.size()) {
            SkString baseDir = SkOSPath::Join(fBaseDir.c_str(), dir.c_str());
            fBaseDir = string(baseDir.c_str());
        }
        this->skipWhiteSpace();
        char bracket = this->peek();
        if ('{' == bracket) {
            TextParser::next();
            continue;
        }
        if ('[' != bracket) {
            return false;
        }
        TextParser::next(); // either open-quote or ']' if last
        return true;
    } while (++fDepth);
    return true;
}

void StatusIter::reset() {
    ParserCommon::reset();
}


