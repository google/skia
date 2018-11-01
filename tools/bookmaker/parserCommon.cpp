/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOSFile.h"
#include "SkOSPath.h"

#include "parserCommon.h"

void ParserCommon::checkLineLength(size_t len, const char* str) {
    if (!fWritingIncludes) {
        return;
    }
    int column = fColumn;
    const char* lineStart = str;
    for (size_t index = 0; index < len; ++index) {
        if ('\n' == str[index]) {
            if (column > 100) {
                SkDebugf("> 100 columns in %s line %d\n", fFileName.c_str(), fLinesWritten);
                SkDebugf("%.*s\n", &str[index + 1] - lineStart, lineStart);
                SkDebugf("");  // convenient place to set breakpoints
            }
            fLinesWritten++;
            column = 0;
            lineStart = &str[index + 1];
        } else {
            column++;
        }
    }
}

// change Xxx_Xxx to xxx xxx
string ParserCommon::ConvertRef(const string str, bool first) {
    string substitute;
    for (char c : str) {
        if ('_' == c) {
            c = ' ';
        } else if (isupper(c) && !first) {
            c = tolower(c);
        }
        substitute += c;
        first = false;
    }
    return substitute;
}

void ParserCommon::CopyToFile(string oldFile, string newFile) {
    int bufferSize;
    char* buffer = ParserCommon::ReadToBuffer(newFile, &bufferSize);
    FILE* oldOut = fopen(oldFile.c_str(), "wb");
    if (!oldOut) {
        SkDebugf("could not open file %s\n", oldFile.c_str());
        return;
    }
    fwrite(buffer, 1, bufferSize, oldOut);
    fclose(oldOut);
    remove(newFile.c_str());
}

string ParserCommon::HtmlFileName(string bmhFileName) {
    SkASSERT("docs" == bmhFileName.substr(0, 4));
    SkASSERT('\\' == bmhFileName[4] || '/' == bmhFileName[4]);
    SkASSERT(".bmh" == bmhFileName.substr(bmhFileName.length() - 4));
    string result = bmhFileName.substr(5, bmhFileName.length() - 4 - 5);
    return result;
}

bool ParserCommon::parseFile(const char* fileOrPath, const char* suffix, OneFile oneFile) {
    fRawFilePathDir = string(fileOrPath);
    if (!sk_isdir(fileOrPath)) {
        if (!this->parseFromFile(fileOrPath)) {
            SkDebugf("failed to parse %s\n", fileOrPath);
            return false;
        }
    } else if (OneFile::kNo == oneFile) {
        SkOSFile::Iter it(fileOrPath, suffix);
        for (SkString file; it.next(&file); ) {
            // FIXME: skip difficult file for now
            if (string::npos != string(file.c_str()).find("SkFontArguments")) {
                continue;
            }
            if (string::npos != string(file.c_str()).find("SkFontStyle")) {
                continue;
            }
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
    fRawFilePathDir = string(statusFile);
    StatusIter iter(statusFile, suffix, filter);
    if (iter.empty()) {
        return false;
    }
    for (string file; iter.next(&file, nullptr); ) {
        SkString p = SkOSPath::Join(iter.baseDir().c_str(), file.c_str());
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

void ParserCommon::stringAppend(string& result, char ch) const {
    if (fDebugWriteCodeBlock) {
        SkDebugf("%c", ch);
    }
    result += ch;
}

void ParserCommon::stringAppend(string& result, string str) const {
    string condense;
    char last = result.size() ? result.back() : '\n';
    for (auto c : str) {
        if (' ' == c && ' ' == last) {
            continue;
        }
        condense += c;
        if ('\n' != last || ' ' != c) {
            last = c;
        }
    }
    if (fDebugWriteCodeBlock) {
        SkDebugf("%s", condense.c_str());
    }
    result += condense;
}

void ParserCommon::stringAppend(string& result, const Definition* def) const {
    this->stringAppend(result, string(def->fContentStart, def->length()));
}

bool ParserCommon::writeBlockIndent(int size, const char* data, bool ignoreIdent) {
    bool wroteSomething = false;
    while (size && ' ' >= data[size - 1]) {
        --size;
    }
    bool newLine = false;
    char firstCh = 0;
    while (size) {
        while (size && (' ' > data[0] || (' ' == data[0] && ignoreIdent))) {
            ++data;
            --size;
        }
        if (!size) {
            return wroteSomething;
        }
        if (fReturnOnWrite) {
            return true;
        }
        if (newLine) {
            this->lf(1);
        }
        int indent = fIndent;
        if (!firstCh) {
            firstCh = data[0];
        } else if ('(' == firstCh) {
            indent += 1;
        }
        TextParser parser(fFileName, data, data + size, fLineCount);
        const char* lineEnd = parser.strnchr('\n', data + size);
        int len = lineEnd ? (int) (lineEnd - data) : size;
        this->writePending();
        this->indentToColumn(indent);
        FPRINTF("%.*s", len, data);
        checkLineLength(len, data);
        size -= len;
        data += len;
        newLine = true;
        wroteSomething = true;
    }
    return wroteSomething;
}

bool ParserCommon::writeBlockTrim(int size, const char* data) {
    SkASSERT(size >= 0);
    if (!fReturnOnWrite && fOutdentNext) {
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
        if (!fReturnOnWrite) {
            fLastChar = '\0';
        }
        return false;
    }
    if (fReturnOnWrite) {
        return true;
    }
    SkASSERT(size < 20000);
    if (size > 3 && !strncmp("#end", data, 4)) {
        fMaxLF = 1;
    }
    if (this->leadingPunctuation(data, (size_t) size)) {
        fPendingSpace = 0;
    }
    this->writePending();
    FPRINTF("%.*s", size, data);
    checkLineLength(size, data);
    fWroteSomething = true;
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
    SkASSERT(!fReturnOnWrite);
    fPendingLF = SkTMin(fPendingLF, fMaxLF);
    bool wroteLF = false;
    while (fLinefeeds < fPendingLF) {
        if (fDebugOut) {
            SkDebugf("\n");
        }
        fprintf(fOut, "\n");
        checkLineLength(1, "\n");
        ++fLinefeeds;
        wroteLF = true;
    }
    fPendingLF = 0;
    if (wroteLF) {
        SkASSERT(0 == fColumn);
        SkASSERT(fIndent >= fSpaces);
        SkASSERT(fIndent - fSpaces < 80);
        if (fDebugOut) {
            SkDebugf("%*s", fIndent - fSpaces, "");
        }
        fprintf(fOut, "%*s", fIndent - fSpaces, "");
        fColumn = fIndent;
        fSpaces = fIndent;
    }
    SkASSERT(!fWritingIncludes || fColumn + fPendingSpace < 100);
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
    SkASSERT(!fReturnOnWrite);
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
    FPRINTF("%s", str);
    checkLineLength(strlen(str), str);
    fColumn += len;
    fSpaces = 0;
    fLinefeeds = 0;
    fMaxLF = 2;
}

char* ParserCommon::ReadToBuffer(string filename, int* size) {
    FILE* file = fopen(filename.c_str(), "rb");
    if (!file) {
        return nullptr;
    }
    fseek(file, 0L, SEEK_END);
    *size = (int) ftell(file);
    rewind(file);
    char* buffer = new char[*size];
    memset(buffer, ' ', *size);
    SkAssertResult(*size == (int)fread(buffer, 1, *size, file));
    fclose(file);
    fflush(file);
    return buffer;
}

char* ParserCommon::FindDateTime(char* buffer, int size) {
    int index = -1;
    int lineCount = 8;
    while (++index < size && ('\n' != buffer[index] || --lineCount))
        ;
    if (lineCount) {
        return nullptr;
    }
    if (strncmp("\n   on 20", &buffer[index], 9)) {
        return nullptr;
    }
    return &buffer[index];
}

bool ParserCommon::WrittenFileDiffers(string filename, string readname) {
    int writtenSize, readSize;
    char* written = ParserCommon::ReadToBuffer(filename, &writtenSize);
    if (!written) {
        return true;
    }
    char* read = ParserCommon::ReadToBuffer(readname, &readSize);
    if (!read) {
        delete[] written;
        return true;
    }
#if 0  // enable for debugging this
    int smaller = SkTMin(writtenSize, readSize);
    for (int index = 0; index < smaller; ++index) {
        if (written[index] != read[index]) {
            SkDebugf("%.*s\n", 40, &written[index]);
            SkDebugf("%.*s\n", 40, &read[index]);
            break;
        }
    }
#endif
    if (readSize != writtenSize) {
        return true;
    }
    // force the date/time to be the same, if present in both
    const char* newDateTime = FindDateTime(written, writtenSize);
    char* oldDateTime = FindDateTime(read, readSize);
    if (newDateTime && oldDateTime) {
        memcpy(oldDateTime, newDateTime, 26);
    }
    bool result = !!memcmp(written, read, writtenSize);
    delete[] written;
    delete[] read;
    return result;
}

StatusIter::StatusIter(const char* statusFile, const char* suffix, StatusFilter filter)
    : fSuffix(suffix)
    , fFilter(filter) {
    if (!this->parseFromFile(statusFile)) {
        return;
    }
}

static const char* block_names[] = {
    "Completed",
    "InProgress",
};

string StatusIter::baseDir() {
    SkASSERT(fStack.back().fObject.isArray());
    SkASSERT(fStack.size() > 2);
    string dir;
    for (unsigned index = 2; index < fStack.size(); ++index) {
        dir += fStack[index].fName;
        if (index < fStack.size() - 1) {
            dir += SkOSPath::SEPARATOR;
        }
    }
    return dir;
}

// FIXME: need to compare fBlockName against fFilter
// need to compare fSuffix against next value returned
bool StatusIter::next(string* strPtr, StatusFilter *filter) {
    string str;
    JsonStatus* status;
    StatusFilter blockType = StatusFilter::kCompleted;
    do {
        do {
            if (fStack.empty()) {
                return false;
            }
            status = &fStack.back();
            if (status->fIter != status->fObject.end()) {
                break;
            }
            fStack.pop_back();
        } while (true);
        if (1 == fStack.size()) {
            do {
                blockType = StatusFilter::kUnknown;
                for (unsigned index = 0; index < SK_ARRAY_COUNT(block_names); ++index) {
                    if (status->fIter.key().asString() == block_names[index]) {
                        blockType = (StatusFilter) index;
                        break;
                    }
                }
                if (blockType <= fFilter) {
                    break;
                }
                status->fIter++;
            } while (status->fIter != status->fObject.end());
            if (status->fIter == status->fObject.end()) {
                continue;
            }
        }
        if (!status->fObject.isArray()) {
            SkASSERT(status->fIter != status->fObject.end());
            JsonStatus block = {
                *status->fIter,
                status->fIter->begin(),
                status->fIter.key().asString(),
                blockType
            };
            fStack.emplace_back(block);
            status = &(&fStack.back())[-1];
            status->fIter++;
            status = &fStack.back();
            continue;
        }
        str = status->fIter->asString();
        if (strPtr) {
            *strPtr = str;
        }
        if (filter) {
            *filter = status->fStatusFilter;
        }
        status->fIter++;
        if (str.length() - strlen(fSuffix) == str.find(fSuffix)) {
            return true;
        }
    } while (true);
    return true;
}

bool JsonCommon::parseFromFile(const char* path) {
    sk_sp<SkData> json(SkData::MakeFromFileName(path));
    if (!json) {
        SkDebugf("file %s:\n", path);
        return this->reportError<bool>("file not readable");
    }
    Json::Reader reader;
    const char* data = (const char*)json->data();
    if (!reader.parse(data, data + json->size(), fRoot)) {
        SkDebugf("file %s:\n", path);
        return this->reportError<bool>("file not parsable");
    }
    JsonStatus block = { fRoot, fRoot.begin(), "", StatusFilter::kUnknown };
    fStack.emplace_back(block);
    return true;
}

