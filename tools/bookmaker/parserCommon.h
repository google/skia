/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef parserCommon_DEFINED
#define parserCommon_DEFINED

#include "SkData.h"
#include "SkJSONCPP.h"

#include "definition.h"
#include "textParser.h"

enum class StatusFilter {
    kCompleted,
    kInProgress,
    kUnknown,
};

class ParserCommon : public TextParser {
public:
    enum class OneFile {
        kNo,
        kYes,
    };

    enum class OneLine {
        kNo,
        kYes,
    };

    enum class IndentKind {
        kConstOut,
        kEnumChild,
        kEnumChild2,
        kEnumHeader,
        kEnumHeader2,
        kMethodOut,
        kStructMember,
    };

    struct IndentState {
        IndentState(IndentKind kind, int indent)
            : fKind(kind)
            , fIndent(indent) {
        }

        IndentKind fKind;
        int fIndent;
    };

    ParserCommon() : TextParser()
        , fParent(nullptr)
        , fDebugOut(false)
        , fValidate(false)
        , fReturnOnWrite(false)
    {
    }

    ~ParserCommon() override {
    }

    void addDefinition(Definition* def) {
        fParent->fChildren.push_back(def);
        fParent = def;
    }

    void checkLineLength(size_t len, const char* str);
    static string ConvertRef(const string str, bool first);
    static void CopyToFile(string oldFile, string newFile);
    static char* FindDateTime(char* buffer, int size);
    static string HtmlFileName(string bmhFileName);

    void indentIn(IndentKind kind) {
        fIndent += 4;
        fIndentStack.emplace_back(kind, fIndent);
    }

    void indentOut() {
        SkASSERT(fIndent >= 4);
        SkASSERT(fIndentStack.back().fIndent == fIndent);
        fIndent -= 4;
        fIndentStack.pop_back();
    }

    void indentToColumn(int column) {
        SkASSERT(column >= fColumn);
        SkASSERT(!fReturnOnWrite);
        SkASSERT(column < 80);
        FPRINTF("%*s", column - fColumn, "");
        fColumn = column;
        fSpaces += column - fColumn;
    }

    bool leadingPunctuation(const char* str, size_t len) const {
        if (!fPendingSpace) {
            return false;
        }
        if (len < 2) {
            return false;
        }
        if ('.' != str[0] && ',' != str[0] && ';' != str[0] && ':' != str[0]) {
            return false;
        }
        return ' ' >= str[1];
    }

    void lf(int count) {
        fPendingLF = SkTMax(fPendingLF, count);
        this->nl();
    }

    void lfAlways(int count) {
        this->lf(count);
        this->writePending();
    }

    void lfcr() {
        this->lf(1);
    }

    void nl() {
        SkASSERT(!fReturnOnWrite);
        fLinefeeds = 0;
        fSpaces = 0;
        fColumn = 0;
        fPendingSpace = 0;
    }

    bool parseFile(const char* file, const char* suffix, OneFile );
    bool parseStatus(const char* file, const char* suffix, StatusFilter filter);
    virtual bool parseFromFile(const char* path) = 0;
    bool parseSetup(const char* path);

    void popObject() {
        fParent->fContentEnd = fParent->fTerminator = fChar;
        fParent = fParent->fParent;
    }

    static char* ReadToBuffer(string filename, int* size);

    virtual void reset() = 0;

    void resetCommon() {
        fLine = fChar = fStart;
        fLineCount = 0;
        fLinesWritten = 1;
        fParent = nullptr;
        fIndent = 0;
        fOut = nullptr;
        fMaxLF = 2;
        fPendingLF = 0;
        fPendingSpace = 0;
        fOutdentNext = false;
        fWritingIncludes = false;
        fDebugWriteCodeBlock = false;
        nl();
   }

    void setAsParent(Definition* definition) {
        if (fParent) {
            fParent->fChildren.push_back(definition);
            definition->fParent = fParent;
        }
        fParent = definition;
    }

    void singleLF() {
        fMaxLF = 1;
    }

    void stringAppend(string& result, char ch) const;
    void stringAppend(string& result, string str) const;
    void stringAppend(string& result, const Definition* ) const;

    void writeBlock(int size, const char* data) {
        SkAssertResult(writeBlockTrim(size, data));
    }

    bool writeBlockIndent(int size, const char* data, bool ignoreIndent);

    void writeBlockSeparator() {
            this->writeString(
              "# ------------------------------------------------------------------------------");
            this->lf(2);
    }

    bool writeBlockTrim(int size, const char* data);

    void writeCommentHeader() {
        this->lf(2);
        this->writeString("/**");
        this->writeSpace();
    }

    void writeCommentTrailer(OneLine oneLine) {
        if (OneLine::kNo == oneLine) {
            this->lf(1);
        } else {
            this->writeSpace();
        }
        this->writeString("*/");
        this->lfcr();
    }

    void writePending();

    // write a pending space, so that two consecutive calls
    // don't double write, and trailing spaces on lines aren't written
    void writeSpace(int count = 1) {
        SkASSERT(!fReturnOnWrite);
        SkASSERT(!fPendingLF);
        SkASSERT(!fLinefeeds);
        SkASSERT(fColumn > 0);
        SkASSERT(!fSpaces);
        fPendingSpace = count;
    }

    void writeString(const char* str);

    void writeString(string str) {
        this->writeString(str.c_str());
    }

    static bool WrittenFileDiffers(string filename, string readname);

    unordered_map<string, sk_sp<SkData>> fRawData;
    unordered_map<string, vector<char>> fLFOnly;
    vector<IndentState> fIndentStack;
    Definition* fParent;
    FILE* fOut;
    string fRawFilePathDir;
    int fLinefeeds;    // number of linefeeds last written, zeroed on non-space
    int fMaxLF;        // number of linefeeds allowed
    int fPendingLF;    // number of linefeeds to write (can be suppressed)
    int fSpaces;       // number of spaces (indent) last written, zeroed on non-space
    int fColumn;       // current column; number of chars past last linefeed
    int fIndent;       // desired indention
    int fPendingSpace; // one or two spaces should preceed the next string or block
    size_t fLinesWritten; // as opposed to fLineCount, number of lines read
    char fLastChar;    // last written
    bool fDebugOut;    // set true to write to std out
    bool fValidate;    // set true to check anchor defs and refs
    bool fOutdentNext; // set at end of embedded struct to prevent premature outdent
    bool fWroteSomething; // used to detect empty content; an alternative source is preferable
    bool fReturnOnWrite; // used to detect non-empty content; allowing early return
    bool fWritingIncludes; // set true when writing includes to check >100 columns
    mutable bool fDebugWriteCodeBlock;

private:
    typedef TextParser INHERITED;
};

struct JsonStatus {
    const Json::Value& fObject;
    Json::Value::iterator fIter;
    string fName;
    StatusFilter fStatusFilter;
};

class JsonCommon : public ParserCommon {
public:
    bool empty() { return fStack.empty(); }
    bool parseFromFile(const char* path) override;

    void reset() override {
        fStack.clear();
        INHERITED::resetCommon();
    }

    vector<JsonStatus> fStack;
    Json::Value fRoot;
private:
    typedef ParserCommon INHERITED;
};

class StatusIter : public JsonCommon {
public:
    StatusIter(const char* statusFile, const char* suffix, StatusFilter);
    ~StatusIter() override {}
    string baseDir();
    bool next(string* file, StatusFilter* filter);
private:
    const char* fSuffix;
    StatusFilter fFilter;
};

class HackParser : public ParserCommon {
public:
    HackParser(const BmhParser& bmhParser)
        : ParserCommon()
        , fBmhParser(bmhParser) {
        this->reset();
    }

    bool parseFromFile(const char* path) override {
        if (!INHERITED::parseSetup(path)) {
            return false;
        }
        return hackFiles();
    }

    void reset() override {
        INHERITED::resetCommon();
    }

    void replaceWithPop(const Definition* );

private:
    const BmhParser& fBmhParser;
    bool hackFiles();

    typedef ParserCommon INHERITED;
};

#endif
