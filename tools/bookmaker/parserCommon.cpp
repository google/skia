/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bookmaker.h"

static void debug_out(int len, const char* data) {
    // convenient place to intercept arbitrary output
    SkDebugf("%.*s", len, data);
}

bool ParserCommon::parseSetup(const char* path) {
    this->reset();
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
