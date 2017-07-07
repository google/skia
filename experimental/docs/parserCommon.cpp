/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bookmaker.h"

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
