/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef textParser_DEFINED
#define textParser_DEFINED

#include <functional>

#include "bookmaker.h"

class BmhParser;
class Definition;

class TextParser : public NonAssignable {
    TextParser() {}  // only for ParserCommon, TextParserSave
    friend class ParserCommon;
    friend class TextParserSave;
public:
    virtual ~TextParser() {}

    TextParser(string fileName, const char* start, const char* end, int lineCount)
        : fFileName(fileName)
        , fStart(start)
        , fLine(start)
        , fChar(start)
        , fEnd(end)
        , fLineCount(lineCount)
    {
    }

    TextParser(const Definition* );

    const char* anyOf(const char* str) const {
        const char* ptr = fChar;
        while (ptr < fEnd) {
            if (strchr(str, ptr[0])) {
                return ptr;
            }
            ++ptr;
        }
        return nullptr;
    }

    const char* anyOf(const char* wordStart, const char* wordList[], size_t wordListCount) const {
        const char** wordPtr = wordList;
        const char** wordEnd = wordPtr + wordListCount;
        const size_t matchLen = fChar - wordStart;
        while (wordPtr < wordEnd) {
            const char* word = *wordPtr++;
            if (strlen(word) == matchLen && !strncmp(wordStart, word, matchLen)) {
                return word;
            }
        }
        return nullptr;
    }

    // words must be alpha only
    string anyWord(const vector<string>& wordList, int spaces) const {
        const char* matchStart = fChar;
        do {
            int count = spaces;
            while (matchStart < fEnd && !isalpha(matchStart[0])) {
                ++matchStart;
            }
            const char* matchEnd = matchStart;
            const char* nextWord = nullptr;
            while (matchEnd < fEnd) {
                if (isalpha(matchEnd[0])) {
                } else if (' ' == matchEnd[0] && --count >= 0) {
                    if (!nextWord) {
                        nextWord = matchEnd;
                    }
                } else {
                    break;
                }
                ++matchEnd;
            }
            size_t matchLen = matchEnd - matchStart;
            for (auto word : wordList) {
                if (word.length() != matchLen) {
                    continue;
                }
                for (unsigned index = 0; index < matchLen; ++index) {
                    if (tolower(matchStart[index]) != word[index]) {
                        goto nextWord;
                    }
                }
                return word;
        nextWord: ;
            }
            matchStart = nextWord ? nextWord : matchEnd;
        } while (matchStart < fEnd);
        return "";
    }

    bool back(const char* pattern) {
        size_t len = strlen(pattern);
        const char* start = fChar - len;
        if (start <= fStart) {
            return false;
        }
        if (strncmp(start, pattern, len)) {
            return false;
        }
        fChar = start;
        return true;
    }

    char backup(const char* pattern) const {
        size_t len = strlen(pattern);
        const char* start = fChar - len;
        if (start <= fStart) {
            return '\0';
        }
        if (strncmp(start, pattern, len)) {
            return '\0';
        }
        return start[-1];
    }

    void backupWord() {
        while (fChar > fStart && isalpha(fChar[-1])) {
            --fChar;
        }
    }

    bool contains(const char* match, const char* lineEnd, const char** loc) const {
        const char* result = this->strnstr(match, lineEnd);
        if (loc) {
            *loc = result;
        }
        return result;
    }

    bool containsWord(const char* match, const char* lineEnd, const char** loc) {
        size_t len = strlen(match);
        do {
            const char* result = this->strnstr(match, lineEnd);
            if (!result) {
                return false;
            }
            if ((result > fStart && isalnum(result[-1])) || (result + len < fEnd
                    && isalnum(result[len]))) {
                fChar = result + len;
                continue;
            }
            if (loc) {
                *loc = result;
            }
            return true;
        } while (true);
    }

    // either /n/n or /n# will stop parsing a typedef
    const char* doubleLF() const {
        const char* ptr = fChar - 1;
        const char* doubleStart = nullptr;
        while (++ptr < fEnd) {
            if (!doubleStart) {
                if ('\n' == ptr[0]) {
                    doubleStart = ptr;
                }
                continue;
            }
            if ('\n' == ptr[0] || '#' == ptr[0]) {
                return doubleStart;
            }
            if (' ' < ptr[0]) {
                doubleStart = nullptr;
            }
        }
        return nullptr;
    }

    bool endsWith(const char* match) {
        int matchLen = strlen(match);
        if (matchLen > fChar - fLine) {
            return false;
        }
        return !strncmp(fChar - matchLen, match, matchLen);
    }

    bool eof() const { return fChar >= fEnd; }

    const char* lineEnd() const {
        const char* ptr = fChar;
        do {
            if (ptr >= fEnd) {
                return ptr;
            }
            char test = *ptr++;
            if (test == '\n' || test == '\0') {
                break;
            }
        } while (true);
        return ptr;
    }

    ptrdiff_t lineLength() const {
        return this->lineEnd() - fLine;
    }

    bool match(TextParser* );

    char next() {
        SkASSERT(fChar < fEnd);
        char result = *fChar++;
        if ('\n' == result) {
            ++fLineCount;
            fLine = fChar;
        }
        return result;
    }

    char peek() const { SkASSERT(fChar < fEnd); return *fChar; }

    void restorePlace(const TextParser& save) {
        fChar = save.fChar;
        fLine = save.fLine;
        fLineCount = save.fLineCount;
    }

    void savePlace(TextParser* save) {
        save->fChar = fChar;
        save->fLine = fLine;
        save->fLineCount = fLineCount;
    }

    void reportError(const char* errorStr) const;
    static string ReportFilename(string file);
    void reportWarning(const char* errorStr) const;

    template <typename T> T reportError(const char* errorStr) const {
        this->reportError(errorStr);
        return T();
    }

    bool sentenceEnd(const char* check) const {
        while (check > fStart) {
            --check;
            if (' ' < check[0] && '.' != check[0]) {
                return false;
            }
            if ('.' == check[0]) {
                return ' ' >= check[1];
            }
            if ('\n' == check[0] && '\n' == check[1]) {
                return true;
            }
        }
        return true;
    }

    void setForErrorReporting(const Definition* , const char* );

    bool skipToBalancedEndBracket(char startB, char endB) {
        SkASSERT(fChar < fEnd);
        SkASSERT(startB == fChar[0]);
        int startCount = 0;
        do {
            char test = this->next();
            startCount += startB == test;
            startCount -= endB  == test;
        } while (startCount && fChar < fEnd);
        return !startCount;
    }

    bool skipToEndBracket(char endBracket, const char* end = nullptr) {
        if (nullptr == end) {
            end = fEnd;
        }
        while (fChar[0] != endBracket) {
            if (fChar >= end) {
                return false;
            }
            (void) this->next();
        }
        return true;
    }

    bool skipToEndBracket(const char* endBracket) {
        size_t len = strlen(endBracket);
        while (strncmp(fChar, endBracket, len)) {
            if (fChar >= fEnd) {
                return false;
            }
            (void) this->next();
        }
        return true;
    }

    bool skipLine() {
        return skipToEndBracket('\n');
    }

    void skipTo(const char* skip) {
       while (fChar < skip) {
           this->next();
       }
    }

    void skipToAlpha() {
        while (fChar < fEnd && !isalpha(fChar[0])) {
            fChar++;
        }
    }

    // returns true if saw close brace
    bool skipToAlphaNum() {
        bool sawCloseBrace = false;
        while (fChar < fEnd && !isalnum(fChar[0])) {
            sawCloseBrace |= '}' == *fChar++;
        }
        return sawCloseBrace;
    }

    bool skipExact(const char* pattern) {
        if (!this->startsWith(pattern)) {
            return false;
        }
        this->skipName(pattern);
        return true;
    }

    // differs from skipToNonAlphaNum in that a.b isn't considered a full name,
    // since a.b can't be found as a named definition
    void skipFullName() {
        do {
            char last = '\0';
            while (fChar < fEnd && (isalnum(fChar[0])
                    || '_' == fChar[0]  /* || '-' == fChar[0] */
                    || (':' == fChar[0] && fChar + 1 < fEnd && ':' == fChar[1]))) {
                if (':' == fChar[0] && fChar + 1 < fEnd && ':' == fChar[1]) {
                    fChar++;
                }
                last = fChar[0];
                fChar++;
            }
            if (fChar + 1 >= fEnd || '/' != fChar[0] || !isalpha(last) || !isalpha(fChar[1])) {
                break;  // stop unless pattern is xxx/xxx as in I/O
            }
            fChar++; // skip slash
        } while (true);
    }

    int skipToLineBalance(char open, char close) {
        int match = 0;
        while (!this->eof() && '\n' != fChar[0]) {
            match += open == this->peek();
            match -= close == this->next();
        }
        return match;
    }

    bool skipToLineStart() {
        if (!this->skipLine()) {
            return false;
        }
        if (!this->eof()) {
            return this->skipWhiteSpace();
        }
        return true;
    }

    void skipToLineStart(int* indent, bool* sawReturn) {
        SkAssertResult(this->skipLine());
        this->skipWhiteSpace(indent, sawReturn);
    }

    void skipLower() {
        while (fChar < fEnd && (islower(fChar[0]) || '_' == fChar[0])) {
            fChar++;
        }
    }

    void skipToNonAlphaNum() {
        while (fChar < fEnd && (isalnum(fChar[0]) || '_' == fChar[0])) {
            fChar++;
        }
    }

    void skipToNonName() {
        while (fChar < fEnd && (isalnum(fChar[0])
                || '_' == fChar[0] || '-' == fChar[0]
                || (':' == fChar[0] && fChar + 1 < fEnd && ':' == fChar[1])
                || ('.' == fChar[0] && fChar + 1 < fEnd && isalpha(fChar[1])))) {
            if (':' == fChar[0] && fChar +1 < fEnd && ':' == fChar[1]) {
                fChar++;
            }
            fChar++;
        }
    }

    void skipPhraseName() {
        while (fChar < fEnd && (islower(fChar[0]) || '_' == fChar[0])) {
            fChar++;
        }
    }

    void skipToSpace() {
        while (fChar < fEnd && ' ' != fChar[0]) {
            fChar++;
        }
    }

    void skipToWhiteSpace() {
        while (fChar < fEnd && ' ' < fChar[0]) {
            fChar++;
        }
    }

    bool skipName(const char* word) {
        size_t len = strlen(word);
        if (len <= (size_t) (fEnd - fChar) && !strncmp(word, fChar, len)) {
            for (size_t i = 0; i < len; ++i) {
                this->next();
            }
        }
        return this->eof() || ' ' >= fChar[0];
    }

    bool skipSpace() {
        while (' ' == this->peek()) {
            (void) this->next();
            if (fChar >= fEnd) {
                return false;
            }
        }
        return true;
    }

    bool skipWord(const char* word) {
        if (!this->skipWhiteSpace()) {
            return false;
        }
        const char* save = fChar;
        if (!this->skipName(word)) {
            fChar = save;
            return false;
        }
        if (!this->skipWhiteSpace()) {
            return false;
        }
        return true;
    }

    bool skipWhiteSpace() {
        while (' ' >= this->peek()) {
            (void) this->next();
            if (fChar >= fEnd) {
                return false;
            }
        }
        return true;
    }

    bool skipWhiteSpace(int* indent, bool* skippedReturn) {
        while (' ' >= this->peek()) {
            *indent = *skippedReturn ? *indent + 1 : 1;
            if ('\n' == this->peek()) {
                *skippedReturn |= true;
                *indent = 0;
            }
            (void) this->next();
            if (fChar >= fEnd) {
                return false;
            }
        }
        return true;
    }

    bool startsWith(const char* str) const {
        size_t len = strlen(str);
        ptrdiff_t lineLen = fEnd - fChar;
        return len <= (size_t) lineLen && 0 == strncmp(str, fChar, len);
    }

    // ignores minor white space differences
    bool startsWith(const char* str, size_t oLen) const {
        size_t tIndex = 0;
        size_t tLen = fEnd - fChar;
        size_t oIndex = 0;
        while (oIndex < oLen && tIndex < tLen) {
            bool tSpace = ' ' >= fChar[tIndex];
            bool oSpace = ' ' >= str[oIndex];
            if (tSpace != oSpace) {
                break;
            }
            if (tSpace) {
                do {
                    ++tIndex;
                } while (tIndex < tLen && ' ' >= fChar[tIndex]);
                do {
                    ++oIndex;
                } while (oIndex < oLen && ' ' >= str[oIndex]);
                continue;
            }
            if (fChar[tIndex] != str[oIndex]) {
                break;
            }
            ++tIndex;
            ++oIndex;
        }
        return oIndex >= oLen;
    }

    const char* strnchr(char ch, const char* end) const {
        const char* ptr = fChar;
        while (ptr < end) {
            if (ptr[0] == ch) {
                return ptr;
            }
            ++ptr;
        }
        return nullptr;
    }

    const char* strnstr(const char *match, const char* end) const {
        size_t matchLen = strlen(match);
        SkASSERT(matchLen > 0);
        ptrdiff_t len = end - fChar;
        SkASSERT(len >= 0);
        if ((size_t) len < matchLen ) {
            return nullptr;
        }
        size_t count = len - matchLen;
        for (size_t index = 0; index <= count; index++) {
            if (0 == strncmp(&fChar[index], match, matchLen)) {
                return &fChar[index];
            }
        }
        return nullptr;
    }

    const char* trimmedBracketEnd(const char bracket) const {
        int max = (int) (this->lineLength());
        int index = 0;
        while (index < max && bracket != fChar[index]) {
            ++index;
        }
        SkASSERT(index < max);
        while (index > 0 && ' ' >= fChar[index - 1]) {
            --index;
        }
        return fChar + index;
    }

    const char* trimmedBracketEnd(string bracket) const {
        size_t max = (size_t) (this->lineLength());
        string line(fChar, max);
        size_t index = line.find(bracket);
        SkASSERT(index < max);
        while (index > 0 && ' ' >= fChar[index - 1]) {
            --index;
        }
        return fChar + index;
    }

    const char* trimmedBracketNoEnd(const char bracket) const {
        int max = (int) (fEnd - fChar);
        int index = 0;
        while (index < max && bracket != fChar[index]) {
            ++index;
        }
        SkASSERT(index < max);
        while (index > 0 && ' ' >= fChar[index - 1]) {
            --index;
        }
        return fChar + index;
    }

    const char* trimmedLineEnd() const {
        const char* result = this->lineEnd();
        while (result > fChar && ' ' >= result[-1]) {
            --result;
        }
        return result;
    }

    void trimEnd() {
        while (fEnd > fStart && ' ' >= fEnd[-1]) {
            --fEnd;
        }
    }

    // FIXME: nothing else in TextParser knows from C++ --
    // there could be a class between TextParser and ParserCommon
    virtual string typedefName();

    const char* wordEnd() const {
        const char* end = fChar;
        while (isalnum(end[0]) || '_' == end[0] || '-' == end[0]) {
            ++end;
        }
        return end;
    }

    string fFileName;
    const char* fStart;
    const char* fLine;
    const char* fChar;
    const char* fEnd;
    size_t fLineCount;
};

class TextParserSave {
public:
    TextParserSave(TextParser* parser) {
        fParser = parser;
        fSave.fFileName = parser->fFileName;
        fSave.fStart = parser->fStart;
        fSave.fLine = parser->fLine;
        fSave.fChar = parser->fChar;
        fSave.fEnd = parser->fEnd;
        fSave.fLineCount = parser->fLineCount;
    }

    void restore() const {
        fParser->fFileName = fSave.fFileName;
        fParser->fStart = fSave.fStart;
        fParser->fLine = fSave.fLine;
        fParser->fChar = fSave.fChar;
        fParser->fEnd = fSave.fEnd;
        fParser->fLineCount = fSave.fLineCount;
    }

private:
    TextParser* fParser;
    TextParser fSave;
};

static inline bool has_nonwhitespace(string s) {
    bool nonwhite = false;
    for (const char& c : s) {
        if (' ' < c) {
            nonwhite = true;
            break;
        }
    }
    return nonwhite;
}

static inline void trim_end(string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

static inline void trim_end_spaces(string &s) {
    while (s.length() > 0 && ' ' == s.back()) {
        s.pop_back();
    }
}

static inline void trim_start(string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
}

static inline void trim_start_end(string& s) {
    trim_start(s);
    trim_end(s);
}

static inline string trim_inline_spaces(string s) {
    bool lastSpace = false;
    string trimmed;
    for (const char* ptr = &s.front(); ptr <= &s.back(); ++ptr) {
        char c = *ptr;
        if (' ' >= c) {
            if (!lastSpace) {
                trimmed += ' ';
            }
            lastSpace = true;
            continue;
        }
        lastSpace = false;
        trimmed += c;
    }
    return trimmed;
}

class EscapeParser : public TextParser {
public:
    EscapeParser(const char* start, const char* end) :
            TextParser("", start, end, 0) {
        const char* reader = fStart;
        fStorage = new char[end - start];
        char* writer = fStorage;
        while (reader < fEnd) {
            char ch = *reader++;
            if (ch != '\\') {
                *writer++ = ch;
            } else {
                char ctrl = *reader++;
                if (ctrl == 'u') {
                    unsigned unicode = 0;
                    for (int i = 0; i < 4; ++i) {
                        unicode <<= 4;
                        SkASSERT((reader[0] >= '0' && reader[0] <= '9') ||
                            (reader[0] >= 'A' && reader[0] <= 'F') ||
                            (reader[0] >= 'a' && reader[0] <= 'f'));
                        int nibble = *reader++ - '0';
                        if (nibble > 9) {
                            nibble = (nibble & ~('a' - 'A')) - 'A' + '9' + 1;
                        }
                        unicode |= nibble;
                    }
                    SkASSERT(unicode < 256);
                    *writer++ = (unsigned char) unicode;
                } else {
                    SkASSERT(ctrl == 'n');
                    *writer++ = '\n';
                }
            }
        }
        fStart = fLine = fChar = fStorage;
        fEnd = writer;
    }

    ~EscapeParser() override {
        delete fStorage;
    }
private:
    char* fStorage;
};

// some methods cannot be trivially parsed; look for class-name / ~ / operator
class MethodParser : public TextParser {
public:
    MethodParser(string className, string fileName,
            const char* start, const char* end, int lineCount)
        : TextParser(fileName, start, end, lineCount)
        , fClassName(className) {
        size_t doubleColons = className.find_last_of("::");
        if (string::npos != doubleColons) {
            fLocalName = className.substr(doubleColons + 1);
            SkASSERT(fLocalName.length() > 0);
        }
    }

    ~MethodParser() override {}

    string localName() const {
        return fLocalName;
    }

    void setLocalName(string name) {
        if (name == fClassName) {
            fLocalName = "";
        } else {
            fLocalName = name;
        }
    }

    // returns true if close brace was skipped
    int skipToMethodStart() {
        if (!fClassName.length()) {
            return this->skipToAlphaNum();
        }
        int braceCount = 0;
        while (!this->eof() && !isalnum(this->peek()) && '~' != this->peek()) {
            braceCount += '{' == this->peek();
            braceCount -= '}' == this->peek();
            this->next();
        }
        return braceCount;
    }

    void skipToMethodEnd(Resolvable resolvable);

    bool wordEndsWith(const char* str) const {
        const char* space = this->strnchr(' ', fEnd);
        if (!space) {
            return false;
        }
        size_t len = strlen(str);
        if (space < fChar + len) {
            return false;
        }
        return !strncmp(str, space - len, len);
    }

private:
    string fClassName;
    string fLocalName;
    typedef TextParser INHERITED;
};

#endif
