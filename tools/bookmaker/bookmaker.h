/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef bookmaker_DEFINED
#define bookmaker_DEFINED

#include "SkCommandLineFlags.h"
#include "SkData.h"
#include "SkJSONCPP.h"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <forward_list>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

// std::to_string isn't implemented on android
#include <sstream>

template <typename T>
std::string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}

using std::forward_list;
using std::list;
using std::unordered_map;
using std::string;
using std::vector;

enum class KeyWord {
    kNone,
    kSK_API,
    kSK_BEGIN_REQUIRE_DENSE,
    kBool,
    kChar,
    kClass,
    kConst,
    kConstExpr,
    kDefine,
    kDouble,
    kElif,
    kElse,
    kEndif,
    kEnum,
    kError,
    kFloat,
    kFriend,
    kIf,
    kIfdef,
    kIfndef,
    kInclude,
    kInline,
    kInt,
    kOperator,
    kPrivate,
    kProtected,
    kPublic,
    kSigned,
    kSize_t,
    kStatic,
    kStruct,
    kTemplate,
    kTypedef,
    kUint16_t,
    kUint32_t,
    kUint64_t,
    kUint8_t,
    kUnion,
    kUnsigned,
    kVoid,
};

enum class MarkType {
    kNone,
    kAnchor,
    kAlias,
    kBug,
    kClass,
    kCode,
    kColumn,
    kComment,
    kConst,
    kDefine,
    kDefinedBy,
    kDeprecated,
    kDescription,
    kDoxygen,
    kDuration,
    kEnum,
    kEnumClass,
    kExample,
    kExperimental,
    kExternal,
    kFile,
    kFormula,
    kFunction,
    kHeight,
    kIllustration,
    kImage,
	kIn,
    kLegend,
	kLine,
    kLink,
    kList,
    kLiteral,  // don't lookup hyperlinks, do substitution, etc
    kMarkChar,
    kMember,
    kMethod,
    kNoExample,
    kOutdent,
    kParam,
    kPlatform,
    kPopulate,
    kPrivate,
    kReturn,
    kRoot,
    kRow,
    kSeeAlso,
    kSet,
    kStdOut,
    kStruct,
    kSubstitute,
    kSubtopic,
    kTable,
    kTemplate,
    kText,
    kTime,
    kToDo,
    kTopic,
    kTrack,
    kTypedef,
    kUnion,
    kVolatile,
    kWidth,
};

enum {
    Last_MarkType = (int) MarkType::kWidth,
};

enum class Bracket {
    kNone,
    kParen,
    kSquare,
    kBrace,
    kAngle,
    kString,
    kChar,
    kSlashStar,
    kSlashSlash,
    kPound,
    kColon,
    kDebugCode,  // parens get special treatment so SkDEBUGCODE( isn't treated as method
};

enum class Punctuation {  // catch-all for misc symbols tracked in C
    kNone,
    kAsterisk,  // for pointer-to
    kSemicolon,  // e.g., to delinate xxx() const ; const int* yyy()
    kLeftBrace,
    kColon,     // for foo() : bar(1), baz(2) {}
};

enum class KeyProperty {
    kNone,
    kClassSection,
    kFunction,
    kModifier,
    kNumber,
    kObject,
    kPreprocessor,
};

enum class StatusFilter {
    kCompleted,
    kInProgress,
    kUnknown,
};

struct IncludeKey {
    const char* fName;
    KeyWord fKeyWord;
    KeyProperty fProperty;
};

extern const IncludeKey kKeyWords[];

static inline bool has_nonwhitespace(const string& s) {
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

class NonAssignable {
public:
    NonAssignable(NonAssignable const&) = delete;
    NonAssignable& operator=(NonAssignable const&) = delete;
    NonAssignable() {}
};

class Definition;

class TextParser : public NonAssignable {
    TextParser() {}  // only for ParserCommon to call
    friend class ParserCommon;
public:
    virtual ~TextParser() {}
    class Save {
    public:
        Save(TextParser* parser) {
            fParser = parser;
            fLine = parser->fLine;
            fChar = parser->fChar;
            fLineCount = parser->fLineCount;
        }

        void restore() const {
            fParser->fLine = fLine;
            fParser->fChar = fChar;
            fParser->fLineCount = fLineCount;
        }

    private:
        TextParser* fParser;
        const char* fLine;
        const char* fChar;
        int fLineCount;
    };

    TextParser(const string& fileName, const char* start, const char* end, int lineCount)
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

    bool contains(const char* match, const char* lineEnd, const char** loc) const {
        *loc = this->strnstr(match, lineEnd);
        return *loc;
    }

    const char* doubleLF() const {
        int count = 0;
        const char* ptr = fChar;
        const char* doubleStart = nullptr;
        while (ptr < fEnd) {
            if ('\n' == ptr[0]) {
                if (++count == 1) {
                    doubleStart = ptr;
                } else {
                    return doubleStart;
                }
            } else if (' ' < ptr[0]) {
                count = 0;
            }
            ++ptr;
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

    void skipToAlphaNum() {
        while (fChar < fEnd && !isalnum(fChar[0])) {
            fChar++;
        }
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
        while (fChar < fEnd && (isalnum(fChar[0])
                || '_' == fChar[0]  /* || '-' == fChar[0] */
                || (':' == fChar[0] && fChar +1 < fEnd && ':' == fChar[1]))) {
            if (':' == fChar[0] && fChar +1 < fEnd && ':' == fChar[1]) {
                fChar++;
            }
            fChar++;
        }
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

    void skipToNonAlphaNum() {
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

class RootDefinition;

class Definition : public NonAssignable {
public:
    enum Type {
        kNone,
        kWord,
        kMark,
        kKeyWord,
        kBracket,
        kPunctuation,
        kFileType,
    };

    enum class TrimExtract {
        kNo,
        kYes
    };

    enum class ExampleOptions {
        kText,
        kPng,
        kAll
    };

    enum class MethodType {
        kNone,
        kConstructor,
        kDestructor,
        kOperator,
    };

    enum class Operator {
        kUnknown,
        kAdd,
        kAddTo,
        kArray,
        kCast,
        kCopy,
        kDelete,
        kDereference,
        kEqual,
        kMinus,
        kMove,
        kMultiply,
        kMultiplyBy,
        kNew,
        kNotEqual,
        kSubtract,
        kSubtractFrom,
    };

    enum class Format {
        kIncludeReturn,
        kOmitReturn,
    };

    Definition() {}

    Definition(const char* start, const char* end, int line, Definition* parent)
        : fStart(start)
        , fContentStart(start)
        , fContentEnd(end)
        , fParent(parent)
        , fLineCount(line)
        , fType(Type::kWord) {
        if (parent) {
            SkASSERT(parent->fFileName.length() > 0);
            fFileName = parent->fFileName;
        }
        this->setParentIndex();
    }

    Definition(MarkType markType, const char* start, int line, Definition* parent)
        : Definition(markType, start, nullptr, line, parent) {
    }

    Definition(MarkType markType, const char* start, const char* end, int line, Definition* parent)
        : Definition(start, end, line, parent) {
        fMarkType = markType;
        fType = Type::kMark;
    }

    Definition(Bracket bracket, const char* start, int lineCount, Definition* parent)
        : Definition(start, nullptr, lineCount, parent) {
        fBracket = bracket;
        fType = Type::kBracket;
    }

    Definition(KeyWord keyWord, const char* start, const char* end, int lineCount,
            Definition* parent)
        : Definition(start, end, lineCount, parent) {
        fKeyWord = keyWord;
        fType = Type::kKeyWord;
    }

    Definition(Punctuation punctuation, const char* start, int lineCount, Definition* parent)
        : Definition(start, nullptr, lineCount, parent) {
        fPunctuation = punctuation;
        fType = Type::kPunctuation;
    }

    virtual ~Definition() {}

    virtual RootDefinition* asRoot() { SkASSERT(0); return nullptr; }
    virtual const RootDefinition* asRoot() const { SkASSERT(0); return nullptr; }
    bool boilerplateIfDef(Definition* parent);
    bool boilerplateDef(Definition* parent);

    bool boilerplateEndIf() {
        return true;
    }

    bool checkMethod() const;
    bool crossCheck2(const Definition& includeToken) const;
    bool crossCheck(const Definition& includeToken) const;
    bool crossCheckInside(const char* start, const char* end, const Definition& includeToken) const;

    const Definition* csParent() const {
        Definition* test = fParent;
        while (test) {
            if (MarkType::kStruct == test->fMarkType || MarkType::kClass == test->fMarkType) {
                return test;
            }
            test = test->fParent;
        }
        return nullptr;
    }

    bool exampleToScript(string* result, ExampleOptions ) const;
    string extractText(TrimExtract trimExtract) const;
    string fiddleName() const;
    const Definition* findClone(string match) const;
    string formatFunction(Format format) const;
    const Definition* hasChild(MarkType markType) const;
    bool hasMatch(const string& name) const;
    const Definition* hasParam(const string& ref) const;
    bool isClone() const { return fClone; }

    Definition* iRootParent() {
        Definition* test = fParent;
        while (test) {
            if (Type::kKeyWord == test->fType && KeyWord::kClass == test->fKeyWord) {
                return test;
            }
            test = test->fParent;
        }
        return nullptr;
    }

    virtual bool isRoot() const { return false; }
    bool isStructOrClass() const;

    int length() const {
        return (int) (fContentEnd - fContentStart);
    }

    bool methodHasReturn(const string& name, TextParser* methodParser) const;
    string methodName() const;
    bool nextMethodParam(TextParser* methodParser, const char** nextEndPtr,
                         string* paramName) const;
    static string NormalizedName(string name);
    bool paramsMatch(const string& fullRef, const string& name) const;
    bool parseOperator(size_t doubleColons, string& result);

    string printableName() const {
        string result(fName);
        std::replace(result.begin(), result.end(), '_', ' ');
        return result;
    }

    template <typename T> T reportError(const char* errorStr) const {
        TextParser tp(this);
        tp.reportError(errorStr);
        return T();
    }

    virtual RootDefinition* rootParent() { SkASSERT(0); return nullptr; }
    virtual const RootDefinition* rootParent() const { SkASSERT(0); return nullptr; }
    void setCanonicalFiddle();

    void setParentIndex() {
        fParentIndex = fParent ? (int) fParent->fTokens.size() : -1;
    }

    void setWrapper();

    const Definition* topicParent() const {
        Definition* test = fParent;
        while (test) {
            if (MarkType::kTopic == test->fMarkType) {
                return test;
            }
            test = test->fParent;
        }
        return nullptr;
    }

    string fText;  // if text is constructed instead of in a file, it's put here
    const char* fStart = nullptr;  // .. in original text file, or the start of fText
    const char* fContentStart;  // start past optional markup name
    string fName;
    string fFiddle;  // if its a constructor or operator, fiddle name goes here
    const char* fContentEnd = nullptr;  // the end of the contained text
    const char* fTerminator = nullptr;  // the end of the markup, normally ##\n or \n
    Definition* fParent = nullptr;
    list<Definition> fTokens;
    vector<Definition*> fChildren;
    string fHash;  // generated by fiddle
    string fFileName;
    mutable string fWrapper; // used by Example to wrap into proper function
    size_t fLineCount = 0;
    int fParentIndex = 0;
    MarkType fMarkType = MarkType::kNone;
    KeyWord fKeyWord = KeyWord::kNone;
    Bracket fBracket = Bracket::kNone;
    Punctuation fPunctuation = Punctuation::kNone;
    MethodType fMethodType = MethodType::kNone;
    Operator fOperator = Operator::kUnknown;
    Type fType = Type::kNone;
    bool fClone = false;
    bool fCloned = false;
    bool fDeprecated = false;
    bool fOperatorConst = false;
    bool fPrivate = false;
    bool fShort = false;
    bool fToBeDeprecated = false;
    bool fMemberStart = false;
    bool fAnonymous = false;
    mutable bool fVisited = false;
};

class RootDefinition : public Definition {
public:
    enum class AllowParens {
        kNo,
        kYes,
    };

    RootDefinition() {
    }

    RootDefinition(MarkType markType, const char* start, int line, Definition* parent)
            : Definition(markType, start, line, parent) {
    }

    RootDefinition(MarkType markType, const char* start, const char* end, int line,
            Definition* parent) : Definition(markType, start, end,  line, parent) {
    }

    ~RootDefinition() override {
        for (auto& iter : fBranches) {
            delete iter.second;
        }
    }

    RootDefinition* asRoot() override { return this; }
    const RootDefinition* asRoot() const override { return this; }
    void clearVisited();
    bool dumpUnVisited();
    const Definition* find(const string& ref, AllowParens ) const;
    bool isRoot() const override { return true; }
    RootDefinition* rootParent() override { return fRootParent; }
    const RootDefinition* rootParent() const override { return fRootParent; }
    void setRootParent(RootDefinition* rootParent) { fRootParent = rootParent; }

    unordered_map<string, RootDefinition*> fBranches;
    unordered_map<string, Definition> fLeaves;
private:
    RootDefinition* fRootParent = nullptr;
};

struct IClassDefinition : public Definition {
    unordered_map<string, Definition*> fEnums;
    unordered_map<string, Definition*> fMembers;
    unordered_map<string, Definition*> fMethods;
    unordered_map<string, Definition*> fStructs;
    unordered_map<string, Definition*> fTypedefs;
};

struct Reference {
    Reference()
        : fLocation(nullptr)
        , fDefinition(nullptr) {
    }

    const char* fLocation;  // .. in original text file
    const Definition* fDefinition;
};

struct TypeNames {
    const char* fName;
    MarkType fMarkType;
};

class ParserCommon : public TextParser {
public:

    ParserCommon() : TextParser()
        , fParent(nullptr)
        , fDebugOut(false)
    {
    }

    ~ParserCommon() override {
    }

    void addDefinition(Definition* def) {
        fParent->fChildren.push_back(def);
        fParent = def;
    }

    void indentToColumn(int column) {
        SkASSERT(column >= fColumn);
        if (fDebugOut) {
            SkDebugf("%*s", column - fColumn, "");
        }
        fprintf(fOut, "%*s", column - fColumn, "");
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
        fLinefeeds = 0;
        fSpaces = 0;
        fColumn = 0;
        fPendingSpace = 0;
    }

    bool parseFile(const char* file, const char* suffix);
    bool parseStatus(const char* file, const char* suffix, StatusFilter filter);
    virtual bool parseFromFile(const char* path) = 0;
    bool parseSetup(const char* path);

    void popObject() {
        fParent->fContentEnd = fParent->fTerminator = fChar;
        fParent = fParent->fParent;
    }

    const char* ReadToBuffer(string filename, int* size);

    virtual void reset() = 0;

    void resetCommon() {
        fLine = fChar = fStart;
        fLineCount = 0;
        fParent = nullptr;
        fIndent = 0;
        fOut = nullptr;
        fMaxLF = 2;
        fPendingLF = 0;
        fPendingSpace = 0;
        fOutdentNext = false;
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

    void writeBlock(int size, const char* data) {
        SkAssertResult(writeBlockTrim(size, data));
    }

    void writeBlockIndent(int size, const char* data);
    bool writeBlockTrim(int size, const char* data);

    void writeCommentHeader() {
        this->lf(2);
        this->writeString("/**");
        this->writeSpace();
    }

    void writeCommentTrailer() {
        this->writeString("*/");
        this->lfcr();
    }

    void writePending();

    // write a pending space, so that two consecutive calls
    // don't double write, and trailing spaces on lines aren't written
    void writeSpace(int count = 1) {
        SkASSERT(!fPendingLF);
        SkASSERT(!fLinefeeds);
        SkASSERT(fColumn > 0);
        SkASSERT(!fSpaces);
        fPendingSpace = count;
    }

    void writeString(const char* str);

    void writeString(const string& str) {
        this->writeString(str.c_str());
    }

    bool writtenFileDiffers(string filename, string readname);

    unordered_map<string, sk_sp<SkData>> fRawData;
    unordered_map<string, vector<char>> fLFOnly;
    Definition* fParent;
    FILE* fOut;
    int fLinefeeds;    // number of linefeeds last written, zeroed on non-space
    int fMaxLF;        // number of linefeeds allowed
    int fPendingLF;    // number of linefeeds to write (can be suppressed)
    int fSpaces;       // number of spaces (indent) last written, zeroed on non-space
    int fColumn;       // current column; number of chars past last linefeed
    int fIndent;       // desired indention
    int fPendingSpace; // one or two spaces should preceed the next string or block
    char fLastChar;    // last written
    bool fDebugOut;    // set true to write to std out
    bool fOutdentNext; // set at end of embedded struct to prevent premature outdent
private:
    typedef TextParser INHERITED;
};

struct JsonStatus {
    const Json::Value& fObject;
    Json::Value::iterator fIter;
    string fName;
};

class StatusIter : public ParserCommon {
public:
    StatusIter(const char* statusFile, const char* suffix, StatusFilter);
    ~StatusIter() override {}
    string baseDir();
    bool empty() { return fStack.empty(); }
    bool next(string* file);
protected:
    bool parseFromFile(const char* path) override;
    void reset() override;
private:
    vector<JsonStatus> fStack;
    Json::Value fRoot;
    const char* fSuffix;
    StatusFilter fFilter;
};

class BmhParser : public ParserCommon {
public:
    enum class MarkLookup {
        kRequire,
        kAllowUnknown,
    };

    enum class Resolvable {
        kNo,      // neither resolved nor output
        kYes,     // resolved, output
        kOut,     // not resolved, but output
        kLiteral, // output untouched (FIXME: is this really different from kOut?)
		kClone,   // resolved, output, with references to clones as well
    };

    enum class Exemplary {
        kNo,
        kYes,
        kOptional,
    };

    enum class TableState {
        kNone,
        kColumnStart,
        kColumnEnd,
    };

    enum class HasTag {
        kNo,
        kYes,
    };

#define M(mt) (1LL << (int) MarkType::k##mt)
#define M_D M(Description)
#define M_CS M(Class) | M(Struct)
#define M_ST M(Subtopic) | M(Topic)
#define M_CSST M_CS | M_ST
#ifdef M_E
#undef M_E
#endif
#define M_E M(Enum) | M(EnumClass)

#define R_Y Resolvable::kYes
#define R_N Resolvable::kNo
#define R_O Resolvable::kOut
#define R_C Resolvable::kClone

#define E_Y Exemplary::kYes
#define E_N Exemplary::kNo
#define E_O Exemplary::kOptional

    BmhParser(bool skip) : ParserCommon()
        , fMaps {
// names without formal definitions (e.g. Column) aren't included
// fill in other names once they're actually used
  { "",            nullptr,      MarkType::kNone,         R_Y, E_N, 0 }
, { "A",           nullptr,      MarkType::kAnchor,       R_N, E_N, 0 }
, { "Alias",       nullptr,      MarkType::kAlias,        R_N, E_N, 0 }
, { "Bug",         nullptr,      MarkType::kBug,          R_N, E_N, 0 }
, { "Class",       &fClassMap,   MarkType::kClass,        R_Y, E_O, M_CSST | M(Root) }
, { "Code",        nullptr,      MarkType::kCode,         R_O, E_N, M_CSST | M_E | M(Method) }
, { "",            nullptr,      MarkType::kColumn,       R_Y, E_N, M(Row) }
, { "",            nullptr,      MarkType::kComment,      R_N, E_N, 0 }
, { "Const",       &fConstMap,   MarkType::kConst,        R_Y, E_O, M_E | M_ST  }
, { "Define",      nullptr,      MarkType::kDefine,       R_O, E_N, M_ST }
, { "DefinedBy",   nullptr,      MarkType::kDefinedBy,    R_N, E_N, M(Method) }
, { "Deprecated",  nullptr,      MarkType::kDeprecated,   R_Y, E_N, 0 }
, { "Description", nullptr,      MarkType::kDescription,  R_Y, E_N, M(Example) | M(NoExample) }
, { "Doxygen",     nullptr,      MarkType::kDoxygen,      R_Y, E_N, 0 }
, { "Duration",    nullptr,      MarkType::kDuration,     R_N, E_N, M(Example) | M(NoExample) }
, { "Enum",        &fEnumMap,    MarkType::kEnum,         R_Y, E_O, M_CSST | M(Root) }
, { "EnumClass",   &fClassMap,   MarkType::kEnumClass,    R_Y, E_O, M_CSST | M(Root) }
, { "Example",     nullptr,      MarkType::kExample,
                                                     R_O, E_N, M_CSST | M_E | M(Method) | M(Const) }
, { "Experimental", nullptr,     MarkType::kExperimental, R_Y, E_N, 0 }
, { "External",    nullptr,      MarkType::kExternal,     R_Y, E_N, M(Root) }
, { "File",        nullptr,      MarkType::kFile,         R_N, E_N, M(Track) }
, { "Formula",     nullptr,      MarkType::kFormula,      R_O, E_N,
                                              M(Column) | M_E | M_ST | M(Member) | M(Method) | M_D }
, { "Function",    nullptr,      MarkType::kFunction,     R_O, E_N, M(Example) | M(NoExample) }
, { "Height",      nullptr,      MarkType::kHeight,       R_N, E_N, M(Example) | M(NoExample) }
, { "Illustration", nullptr,     MarkType::kIllustration, R_N, E_N, M(Subtopic) }
, { "Image",       nullptr,      MarkType::kImage,        R_N, E_N, M(Example) | M(NoExample) }
, { "In",          nullptr,      MarkType::kIn,           R_N, E_N,
                                                             M_CSST | M_E | M(Method) | M(Typedef) }
, { "Legend",      nullptr,      MarkType::kLegend,       R_Y, E_N, M(Table) }
, { "Line",        nullptr,      MarkType::kLine,         R_N, E_N,
                                                             M_CSST | M_E | M(Method) | M(Typedef) }
, { "",            nullptr,      MarkType::kLink,         R_N, E_N, M(Anchor) }
, { "List",        nullptr,      MarkType::kList,         R_Y, E_N, M(Method) | M_CSST | M_E | M_D }
, { "Literal",     nullptr,      MarkType::kLiteral,      R_N, E_N, M(Code) }
, { "",            nullptr,      MarkType::kMarkChar,     R_N, E_N, 0 }
, { "Member",      nullptr,      MarkType::kMember,       R_Y, E_N, M_CSST }
, { "Method",      &fMethodMap,  MarkType::kMethod,       R_Y, E_Y, M_CSST }
, { "NoExample",   nullptr,      MarkType::kNoExample,    R_O, E_N, M_CSST | M_E | M(Method) }
, { "Outdent",     nullptr,      MarkType::kOutdent,      R_N, E_N, M(Code) }
, { "Param",       nullptr,      MarkType::kParam,        R_Y, E_N, M(Method) }
, { "Platform",    nullptr,      MarkType::kPlatform,     R_N, E_N, M(Example) | M(NoExample) }
, { "Populate",    nullptr,      MarkType::kPopulate,     R_N, E_N, M(Subtopic) }
, { "Private",     nullptr,      MarkType::kPrivate,      R_N, E_N, 0 }
, { "Return",      nullptr,      MarkType::kReturn,       R_Y, E_N, M(Method) }
, { "",            nullptr,      MarkType::kRoot,         R_Y, E_N, 0 }
, { "",            nullptr,      MarkType::kRow,          R_Y, E_N, M(Table) | M(List) }
, { "SeeAlso",     nullptr,      MarkType::kSeeAlso,      R_C, E_N,
                                                             M_CSST | M_E | M(Method) | M(Typedef) }
, { "Set",         nullptr,      MarkType::kSet,          R_N, E_N, M(Example) | M(NoExample) }
, { "StdOut",      nullptr,      MarkType::kStdOut,       R_N, E_N, M(Example) | M(NoExample) }
, { "Struct",      &fClassMap,   MarkType::kStruct,       R_Y, E_O, M(Class) | M(Root) | M_ST }
, { "Substitute",  nullptr,      MarkType::kSubstitute,   R_N, E_N, M_ST }
, { "Subtopic",    nullptr,      MarkType::kSubtopic,     R_Y, E_Y, M_CSST }
, { "Table",       nullptr,      MarkType::kTable,        R_Y, E_N, M(Method) | M_CSST | M_E }
, { "Template",    nullptr,      MarkType::kTemplate,     R_Y, E_N, 0 }
, { "",            nullptr,      MarkType::kText,         R_N, E_N, 0 }
, { "Time",        nullptr,      MarkType::kTime,         R_Y, E_N, M(Track) }
, { "ToDo",        nullptr,      MarkType::kToDo,         R_N, E_N, 0 }
, { "Topic",       nullptr,      MarkType::kTopic,        R_Y, E_Y, M_CS | M(Root) | M(Topic) }
, { "Track",       nullptr,      MarkType::kTrack,        R_Y, E_N, M_E | M_ST }
, { "Typedef",     &fTypedefMap, MarkType::kTypedef,      R_Y, E_N, M(Class) | M_ST }
, { "",            nullptr,      MarkType::kUnion,        R_Y, E_N, 0 }
, { "Volatile",    nullptr,      MarkType::kVolatile,     R_N, E_N, M(StdOut) }
, { "Width",       nullptr,      MarkType::kWidth,        R_N, E_N, M(Example) | M(NoExample) } }
, fSkip(skip)
        {
            this->reset();
        }

#undef R_O
#undef R_N
#undef R_Y

#undef M_E
#undef M_CSST
#undef M_ST
#undef M_CS
#undef M_D
#undef M

    ~BmhParser() override {}

    bool addDefinition(const char* defStart, bool hasEnd, MarkType markType,
            const vector<string>& typeNameBuilder, HasTag hasTag);
    bool checkEndMarker(MarkType markType, string name) const;
    bool checkExamples() const;
    bool checkParamReturn(const Definition* definition) const;
    bool dumpExamples(const char* fiddleJsonFileName) const;
    bool childOf(MarkType markType) const;
    string className(MarkType markType);
    bool collectExternals();
    int endHashCount() const;
    bool endTableColumn(const char* end, const char* terminator);

    RootDefinition* findBmhObject(MarkType markType, const string& typeName) const {
        auto map = fMaps[(int) markType].fBmh;
        if (!map) {
            return nullptr;
        }
        return &(*map)[typeName];
    }

    bool findDefinitions();
    Definition* findExample(string name) const;
    MarkType getMarkType(MarkLookup lookup) const;
    bool hasEndToken() const;
    string memberName();
    string methodName();
    const Definition* parentSpace() const;

    bool parseFromFile(const char* path) override {
        if (!INHERITED::parseSetup(path)) {
            return false;
        }
        fCheckMethods = !strstr(path, "undocumented.bmh");
        return findDefinitions();
    }

    bool popParentStack(Definition* definition);
    void reportDuplicates(const Definition& def, const string& dup) const;

    void reset() override {
        INHERITED::resetCommon();
        fRoot = nullptr;
        fWorkingColumn = nullptr;
        fRow = nullptr;
        fTableState = TableState::kNone;
        fMC = '#';
        fInChar = false;
        fInCharCommentString = false;
        fInComment = false;
        fInEnum = false;
        fInString = false;
        fCheckMethods = false;
    }

    bool skipNoName();
    bool skipToDefinitionEnd(MarkType markType);
	bool skipToString();
    void spellCheck(const char* match, SkCommandLineFlags::StringArray report) const;
    void spellStatus(const char* match, SkCommandLineFlags::StringArray report) const;
    vector<string> topicName();
    vector<string> typeName(MarkType markType, bool* expectEnd);
    string typedefName() override;
    string uniqueName(const string& base, MarkType markType);
    string uniqueRootName(const string& base, MarkType markType);
    void validate() const;
    string word(const string& prefix, const string& delimiter);

public:
    struct DefinitionMap {
        const char* fName;
        unordered_map<string, RootDefinition>* fBmh;
        MarkType fMarkType;
        Resolvable fResolve;
        Exemplary fExemplary;  // worthy of an example
        uint64_t fParentMask;
    };

    DefinitionMap fMaps[Last_MarkType + 1];
    forward_list<RootDefinition> fTopics;
    forward_list<Definition> fMarkup;
    forward_list<RootDefinition> fExternals;
    vector<string> fInputFiles;
    unordered_map<string, RootDefinition> fClassMap;
    unordered_map<string, RootDefinition> fConstMap;
    unordered_map<string, RootDefinition> fEnumMap;
    unordered_map<string, RootDefinition> fMethodMap;
    unordered_map<string, RootDefinition> fTypedefMap;
    unordered_map<string, Definition*> fTopicMap;
    unordered_map<string, Definition*> fAliasMap;
    RootDefinition* fRoot;
    Definition* fWorkingColumn;
    Definition* fRow;
    const char* fColStart;
    TableState fTableState;
    mutable char fMC;  // markup character
    bool fAnonymous;
    bool fCloned;
    bool fInChar;
    bool fInCharCommentString;
    bool fInEnum;
    bool fInComment;
    bool fInString;
    bool fCheckMethods;
    bool fSkip = false;
    bool fWroteOut = false;
private:
    typedef ParserCommon INHERITED;
};

class IncludeParser : public ParserCommon {
public:
    enum class IsStruct {
        kNo,
        kYes,
    };

    IncludeParser() : ParserCommon()
        , fMaps {
          { nullptr,        MarkType::kNone }
        , { nullptr,        MarkType::kAnchor }
        , { nullptr,        MarkType::kAlias }
        , { nullptr,        MarkType::kBug }
        , { nullptr,        MarkType::kClass }
        , { nullptr,        MarkType::kCode }
        , { nullptr,        MarkType::kColumn }
        , { nullptr,        MarkType::kComment }
        , { nullptr,        MarkType::kConst }
        , { &fIDefineMap,   MarkType::kDefine }
        , { nullptr,        MarkType::kDefinedBy }
        , { nullptr,        MarkType::kDeprecated }
        , { nullptr,        MarkType::kDescription }
        , { nullptr,        MarkType::kDoxygen }
        , { nullptr,        MarkType::kDuration }
        , { &fIEnumMap,     MarkType::kEnum }
        , { &fIEnumMap,     MarkType::kEnumClass }
        , { nullptr,        MarkType::kExample }
        , { nullptr,        MarkType::kExperimental }
        , { nullptr,        MarkType::kExternal }
        , { nullptr,        MarkType::kFile }
        , { nullptr,        MarkType::kFormula }
        , { nullptr,        MarkType::kFunction }
        , { nullptr,        MarkType::kHeight }
        , { nullptr,        MarkType::kIllustration }
		, { nullptr,        MarkType::kImage }
		, { nullptr,        MarkType::kIn }
		, { nullptr,        MarkType::kLegend }
		, { nullptr,        MarkType::kLine }
		, { nullptr,        MarkType::kLink }
        , { nullptr,        MarkType::kList }
        , { nullptr,        MarkType::kLiteral }
        , { nullptr,        MarkType::kMarkChar }
        , { nullptr,        MarkType::kMember }
        , { nullptr,        MarkType::kMethod }
        , { nullptr,        MarkType::kNoExample }
        , { nullptr,        MarkType::kOutdent }
        , { nullptr,        MarkType::kParam }
        , { nullptr,        MarkType::kPlatform }
        , { nullptr,        MarkType::kPopulate }
        , { nullptr,        MarkType::kPrivate }
        , { nullptr,        MarkType::kReturn }
        , { nullptr,        MarkType::kRoot }
        , { nullptr,        MarkType::kRow }
        , { nullptr,        MarkType::kSeeAlso }
        , { nullptr,        MarkType::kSet }
        , { nullptr,        MarkType::kStdOut }
        , { &fIStructMap,   MarkType::kStruct }
        , { nullptr,        MarkType::kSubstitute }
        , { nullptr,        MarkType::kSubtopic }
        , { nullptr,        MarkType::kTable }
        , { &fITemplateMap, MarkType::kTemplate }
        , { nullptr,        MarkType::kText }
        , { nullptr,        MarkType::kTime }
        , { nullptr,        MarkType::kToDo }
        , { nullptr,        MarkType::kTopic }
        , { nullptr,        MarkType::kTrack }
        , { &fITypedefMap,  MarkType::kTypedef }
        , { &fIUnionMap,    MarkType::kUnion }
        , { nullptr,        MarkType::kVolatile }
        , { nullptr,        MarkType::kWidth } }
    {
        this->reset();
    }

    ~IncludeParser() override {}

    void addKeyword(KeyWord keyWord);

    void addPunctuation(Punctuation punctuation) {
        fParent->fTokens.emplace_back(punctuation, fChar, fLineCount, fParent);
    }

    void addWord() {
        fParent->fTokens.emplace_back(fIncludeWord, fChar, fLineCount, fParent);
        fIncludeWord = nullptr;
    }

    void checkForMissingParams(const vector<string>& methodParams,
                               const vector<string>& foundParams);
    bool checkForWord();
    string className() const;
    bool crossCheck(BmhParser& );
    IClassDefinition* defineClass(const Definition& includeDef, const string& className);
    void dumpClassTokens(IClassDefinition& classDef);
    void dumpComment(const Definition& );
    void dumpEnum(const Definition& , const string& name);
    void dumpMethod(const Definition& );
    void dumpMember(const Definition& );
    bool dumpTokens(const string& directory);
    bool dumpTokens(const string& directory, const string& skClassName);
    bool findComments(const Definition& includeDef, Definition* markupDef);

    Definition* findIncludeObject(const Definition& includeDef, MarkType markType,
            const string& typeName) {
        typedef Definition* DefinitionPtr;
        unordered_map<string, Definition>* map = fMaps[(int) markType].fInclude;
        if (!map) {
            return reportError<DefinitionPtr>("invalid mark type");
        }
        string name = this->uniqueName(*map, typeName);
        Definition& markupDef = (*map)[name];
        if (markupDef.fStart) {
            return reportError<DefinitionPtr>("definition already defined");
        }
        markupDef.fFileName = fFileName;
        markupDef.fStart = includeDef.fStart;
        markupDef.fContentStart = includeDef.fStart;
        markupDef.fName = name;
        markupDef.fContentEnd = includeDef.fContentEnd;
        markupDef.fTerminator = includeDef.fTerminator;
        markupDef.fParent = fParent;
        markupDef.fLineCount = includeDef.fLineCount;
        markupDef.fMarkType = markType;
        markupDef.fKeyWord = includeDef.fKeyWord;
        markupDef.fType = Definition::Type::kMark;
        return &markupDef;
    }

    static KeyWord FindKey(const char* start, const char* end);
    bool internalName(const Definition& ) const;
    bool parseChar();
    bool parseComment(const string& filename, const char* start, const char* end, int lineCount,
            Definition* markupDef);
    bool parseClass(Definition* def, IsStruct);
    bool parseDefine();
    bool parseEnum(Definition* child, Definition* markupDef);

    bool parseFromFile(const char* path) override {
        this->reset();
        if (!INHERITED::parseSetup(path)) {
            return false;
        }
        string name(path);
        return this->parseInclude(name);
    }

    bool parseInclude(const string& name);
    bool parseMember(Definition* child, Definition* markupDef);
    bool parseMethod(Definition* child, Definition* markupDef);
    bool parseObject(Definition* child, Definition* markupDef);
    bool parseObjects(Definition* parent, Definition* markupDef);
    bool parseTemplate();
    bool parseTypedef(Definition* child, Definition* markupDef);
    bool parseUnion();

    void popBracket() {
        SkASSERT(Definition::Type::kBracket == fParent->fType);
        this->popObject();
        Bracket bracket = this->topBracket();
        this->setBracketShortCuts(bracket);
    }

    void pushBracket(Bracket bracket) {
        this->setBracketShortCuts(bracket);
        fParent->fTokens.emplace_back(bracket, fChar, fLineCount, fParent);
        Definition* container = &fParent->fTokens.back();
        this->addDefinition(container);
    }

    static void RemoveFile(const char* docs, const char* includes);
    static void RemoveOneFile(const char* docs, const char* includesFileOrPath);

    void reset() override {
        INHERITED::resetCommon();
        fRootTopic = nullptr;
        fInBrace = nullptr;
        fIncludeWord = nullptr;
        fLastObject = nullptr;
        fPrev = '\0';
        fInChar = false;
        fInCharCommentString = false;
        fInComment = false;
        fInEnum = false;
        fInFunction = false;
        fInString = false;
        fFailed = false;
        fPriorEnum = nullptr;
    }

    void setBracketShortCuts(Bracket bracket) {
        fInComment = Bracket::kSlashSlash == bracket || Bracket::kSlashStar == bracket;
        fInString = Bracket::kString == bracket;
        fInChar = Bracket::kChar == bracket;
        fInCharCommentString = fInChar || fInComment || fInString;
    }

    Bracket topBracket() const {
        Definition* parent = fParent;
        while (parent && Definition::Type::kBracket != parent->fType) {
            parent = parent->fParent;
        }
        return parent ? parent->fBracket : Bracket::kNone;
    }

    template <typename T>
    string uniqueName(const unordered_map<string, T>& m, const string& typeName) {
        string base(typeName.size() > 0 ? typeName : "_anonymous");
        string name(base);
        int anonCount = 1;
        do {
            auto iter = m.find(name);
            if (iter == m.end()) {
                return name;
            }
            name = base + '_';
            name += to_string(++anonCount);
        } while (true);
        // should never get here
        return string();
    }

    void validate() const;

    void writeDefinition(const Definition& def) {
        if (def.length() > 1) {
            this->writeBlock((int) (def.fContentEnd - def.fContentStart), def.fContentStart);
            this->lf(1);
        }
    }

    void writeDefinition(const Definition& def, const string& name, int spaces) {
        this->writeBlock((int) (def.fContentEnd - def.fContentStart), def.fContentStart);
        this->writeSpace(spaces);
        this->writeString(name);
        this->lf(1);
    }

    void writeEndTag() {
        this->lf(1);
        this->writeString("##");
        this->lf(1);
    }

    void writeEndTag(const char* tagType) {
        this->lf(1);
        this->writeString(string("#") + tagType + " ##");
        this->lf(1);
    }

    void writeEndTag(const char* tagType, const char* tagID, int spaces = 1) {
        this->lf(1);
        this->writeString(string("#") + tagType + " " + tagID);
        this->writeSpace(spaces);
        this->writeString("##");
        this->lf(1);
    }

    void writeEndTag(const char* tagType, const string& tagID, int spaces = 1) {
        this->writeEndTag(tagType, tagID.c_str(), spaces);
    }

    void writeIncompleteTag(const char* tagType, const string& tagID, int spaces = 1) {
        this->writeString(string("#") + tagType + " " + tagID);
        this->writeSpace(spaces);
        this->writeString("incomplete");
        this->writeSpace();
        this->writeString("##");
        this->lf(1);
    }

    void writeIncompleteTag(const char* tagType) {
        this->writeString(string("#") + tagType + " incomplete ##");
        this->lf(1);
    }

    void writeTableHeader(const char* col1, size_t pad, const char* col2) {
        this->lf(1);
        this->writeString("#Table");
        this->lf(1);
        this->writeString("#Legend");
        this->lf(1);
        string legend = "# ";
        legend += col1;
        if (pad > strlen(col1)) {
            legend += string(pad - strlen(col1), ' ');
        }
        legend += " # ";
        legend += col2;
        legend += " ##";
        this->writeString(legend);
        this->lf(1);
        this->writeString("#Legend ##");
        this->lf(1);
    }

    void writeTableRow(size_t pad, const string& col1) {
        this->lf(1);
        string row = "# " + col1 + string(pad - col1.length(), ' ') + " # ##";
        this->writeString(row);
        this->lf(1);
    }

    void writeTableRow(size_t pad1, const string& col1, size_t pad2, const string& col2) {
        this->lf(1);
        string row = "# " + col1 + string(pad1 - col1.length(), ' ') + " # " +
                col2 + string(pad2 - col2.length(), ' ') + " ##";
        this->writeString(row);
        this->lf(1);
    }

    void writeTableTrailer() {
        this->lf(1);
        this->writeString("#Table ##");
        this->lf(1);
    }

    void writeTag(const char* tagType) {
        this->lf(1);
        this->writeString("#");
        this->writeString(tagType);
    }

    void writeTagNoLF(const char* tagType, const char* tagID) {
        this->writeString("#");
        this->writeString(tagType);
        this->writeSpace();
        this->writeString(tagID);
    }

    void writeTagNoLF(const char* tagType, const string& tagID) {
        this->writeTagNoLF(tagType, tagID.c_str());
    }

    void writeTag(const char* tagType, const char* tagID) {
        this->lf(1);
        this->writeTagNoLF(tagType, tagID);
    }

    void writeTag(const char* tagType, const string& tagID) {
        this->writeTag(tagType, tagID.c_str());
    }

protected:
    static void ValidateKeyWords();

    struct DefinitionMap {
        unordered_map<string, Definition>* fInclude;
        MarkType fMarkType;
    };

    DefinitionMap fMaps[Last_MarkType + 1];
    unordered_map<string, Definition> fIncludeMap;
    unordered_map<string, IClassDefinition> fIClassMap;
    unordered_map<string, Definition> fIDefineMap;
    unordered_map<string, Definition> fIEnumMap;
    unordered_map<string, Definition> fIFunctionMap;
    unordered_map<string, Definition> fIStructMap;
    unordered_map<string, Definition> fITemplateMap;
    unordered_map<string, Definition> fITypedefMap;
    unordered_map<string, Definition> fIUnionMap;
    Definition* fRootTopic;
    Definition* fInBrace;
    Definition* fLastObject;
    Definition* fPriorEnum;
    int fPriorIndex;
    const char* fIncludeWord;
    char fPrev;
    bool fInChar;
    bool fInCharCommentString;
    bool fInComment;
    bool fInEnum;
    bool fInFunction;
    bool fInString;
    bool fFailed;

    typedef ParserCommon INHERITED;
};

class IncludeWriter : public IncludeParser {
public:
    enum class Word {
        kStart,
        kCap,
        kFirst,
        kUnderline,
        kMixed,
    };

    enum class Phrase {
        kNo,
        kYes,
    };

    enum class PunctuationState {
        kStart,
        kDelimiter,
        kParen,     // treated as a delimiter unless following a space, and followed by word
        kPeriod,
        kSpace,
    };

    enum class RefType {
        kUndefined,
        kNormal,
        kExternal,
    };

	enum class SkipFirstLine {
		kNo,
		kYes,
	};

    enum class Wrote {
        kNone,
        kLF,
        kChars,
    };

    struct IterState {
        IterState (list<Definition>::iterator tIter, list<Definition>::iterator tIterEnd)
            : fDefIter(tIter)
            , fDefEnd(tIterEnd) {
        }
        list<Definition>::iterator fDefIter;
        list<Definition>::iterator fDefEnd;
    };

    struct ParentPair {
        const Definition* fParent;
        const ParentPair* fPrev;
    };

    IncludeWriter() : IncludeParser() {
        this->reset();
    }

    ~IncludeWriter() override {}

    bool contentFree(int size, const char* data) const {
        while (size > 0 && data[0] <= ' ') {
            --size;
            ++data;
        }
        while (size > 0 && data[size - 1] <= ' ') {
            --size;
        }
        return 0 == size;
    }

	void constOut(const Definition* memberStart, const Definition& child,
		const Definition* bmhConst);
    void descriptionOut(const Definition* def, SkipFirstLine );
    void enumHeaderOut(const RootDefinition* root, const Definition& child);
    void enumMembersOut(const RootDefinition* root, Definition& child);
    void enumSizeItems(const Definition& child);
	Definition* findMemberCommentBlock(const vector<Definition*>& bmhChildren, const string& name) const;
    int lookupMethod(const PunctuationState punctuation, const Word word,
            const int start, const int run, int lastWrite,
            const char* data, bool hasIndirection);
    int lookupReference(const PunctuationState punctuation, const Word word,
            const int start, const int run, int lastWrite, const char last,
            const char* data);
    void methodOut(const Definition* method, const Definition& child);
    bool populate(Definition* def, ParentPair* parentPair, RootDefinition* root);
    bool populate(BmhParser& bmhParser);

    void reset() override {
        INHERITED::resetCommon();
        fBmhMethod = nullptr;
        fBmhParser = nullptr;
        fEnumDef = nullptr;
        fMethodDef = nullptr;
        fBmhStructDef = nullptr;
        fAttrDeprecated = nullptr;
        fInStruct = false;
        fWroteMethod = false;
        fIndentNext = false;
        fPendingMethod = false;
    }

    string resolveMethod(const char* start, const char* end, bool first);
    string resolveRef(const char* start, const char* end, bool first, RefType* refType);
    Wrote rewriteBlock(int size, const char* data, Phrase phrase);
    Definition* structMemberOut(const Definition* memberStart, const Definition& child);
    void structOut(const Definition* root, const Definition& child,
            const char* commentStart, const char* commentEnd);
    void structSetMembersShort(const vector<Definition*>& bmhChildren);
    void structSizeMembers(const Definition& child);
private:
    BmhParser* fBmhParser;
    Definition* fDeferComment;
    Definition* fLastComment;
    const Definition* fBmhMethod;
    const Definition* fEnumDef;
    const Definition* fMethodDef;
    const Definition* fBmhStructDef;
    const Definition* fAttrDeprecated;
    const char* fContinuation;  // used to construct paren-qualified method name
    int fAnonymousEnumCount;
    int fEnumItemValueTab;
    int fEnumItemCommentTab;
    int fStructMemberTab;
    int fStructValueTab;
    int fStructCommentTab;
    bool fInStruct;
    bool fWroteMethod;
    bool fIndentNext;
    bool fPendingMethod;

    typedef IncludeParser INHERITED;
};

class FiddleBase : public ParserCommon {
protected:
    FiddleBase(BmhParser* bmh) : ParserCommon()
        , fBmhParser(bmh)
        , fContinuation(false)
        , fTextOut(false)
        , fPngOut(false)
    {
        this->reset();
    }

    void reset() override {
        INHERITED::resetCommon();
    }

    Definition* findExample(string name) const { return fBmhParser->findExample(name); }
    bool parseFiddles();
    virtual bool pngOut(Definition* example) = 0;
    virtual bool textOut(Definition* example, const char* stdOutStart,
        const char* stdOutEnd) = 0;

    BmhParser* fBmhParser;  // must be writable; writes example hash
    string fFullName;
    bool fContinuation;
    bool fTextOut;
    bool fPngOut;
private:
    typedef ParserCommon INHERITED;
};

class FiddleParser : public FiddleBase {
public:
    FiddleParser(BmhParser* bmh) : FiddleBase(bmh) {
       fTextOut = true;
    }

    bool parseFromFile(const char* path) override {
        if (!INHERITED::parseSetup(path)) {
            return false;
        }
        return parseFiddles();
    }

private:
    bool pngOut(Definition* example) override {
        return true;
    }

    bool textOut(Definition* example, const char* stdOutStart,
        const char* stdOutEnd) override;

    typedef FiddleBase INHERITED;
};

class Catalog : public FiddleBase {
public:
    Catalog(BmhParser* bmh) : FiddleBase(bmh) {}

    bool appendFile(const string& path);
    bool closeCatalog();
    bool openCatalog(const char* inDir, const char* outDir);
    bool openStatus(const char* inDir, const char* outDir);

    bool parseFromFile(const char* path) override ;
private:
    bool pngOut(Definition* example) override;
    bool textOut(Definition* example, const char* stdOutStart,
        const char* stdOutEnd) override;

    string fDocsDir;

    typedef FiddleBase INHERITED;
};

class HackParser : public ParserCommon {
public:
    HackParser(const BmhParser& bmhParser)
        : ParserCommon()
        , fBmhParser(bmhParser) {
        this->reset();
    }

    void addOneLiner(const Definition* defTable, const Definition* child, bool hasLine,
            bool lfAfter);

    bool parseFromFile(const char* path) override {
        if (!INHERITED::parseSetup(path)) {
            return false;
        }
        return hackFiles();
    }

    void reset() override {
        INHERITED::resetCommon();
    }

    string searchTable(const Definition* tableHolder, const Definition* match);

    void topicIter(const Definition* );

private:
    const BmhParser& fBmhParser;
    const Definition* fClassesAndStructs;
    const Definition* fConstants;
    const Definition* fConstructors;
    const Definition* fMemberFunctions;
    const Definition* fMembers;
    const Definition* fOperators;
    const Definition* fRelatedFunctions;
    bool hackFiles();

    typedef ParserCommon INHERITED;
};

class MdOut : public ParserCommon {
public:
    MdOut(const BmhParser& bmh) : ParserCommon()
        , fBmhParser(bmh) {
        this->reset();
    }

    bool buildReferences(const char* docDir, const char* mdOutDirOrFile);
    bool buildStatus(const char* docDir, const char* mdOutDir);

    static constexpr const char* kClassesAndStructs = "Class_or_Struct";
    static constexpr const char* kConstants = "Constant";
    static constexpr const char* kConstructors = "Constructor";
    static constexpr const char* kMemberFunctions = "Member_Function";
    static constexpr const char* kMembers = "Member";
    static constexpr const char* kOperators = "Operator";
    static constexpr const char* kOverview = "Overview";
    static constexpr const char* kRelatedFunctions = "Related_Function";
    static constexpr const char* kSubtopics = "Overview_Subtopic";

private:
    enum class TableState {
        kNone,
        kRow,
        kColumn,
    };

    struct TableContents {
        TableContents()
            : fShowClones(false) {
        }

        string fDescription;
        vector<const Definition*> fMembers;
        bool fShowClones;
    };

    string addReferences(const char* start, const char* end, BmhParser::Resolvable );
    bool buildRefFromFile(const char* fileName, const char* outDir);
    bool checkParamReturnBody(const Definition* def) const;
    void childrenOut(const Definition* def, const char* contentStart);
    const Definition* csParent() const;
    const Definition* findParamType();
    const Definition* isDefined(const TextParser& parser, const string& ref, bool report) const;
    string linkName(const Definition* ) const;
    string linkRef(const string& leadingSpaces, const Definition*, const string& ref,
			BmhParser::Resolvable ) const;
    void markTypeOut(Definition* );
    void mdHeaderOut(int depth) { mdHeaderOutLF(depth, 2); }
    void mdHeaderOutLF(int depth, int lf);
    void overviewOut();
    bool parseFromFile(const char* path) override { return true; }
    void populateTables(const Definition* def);

    TableContents& populator(const char* key) {
        auto entry = fPopulators.find(key);
        // FIXME: this should have been detected earlier
        SkASSERT(fPopulators.end() != entry);
        return entry->second;
    }

    void reset() override {
        INHERITED::resetCommon();
        fEnumClass = nullptr;
        fMethod = nullptr;
        fRoot = nullptr;
        fLastParam = nullptr;
        fTableState = TableState::kNone;
        fHasFiddle = false;
        fInDescription = false;
        fInList = false;
        fRespectLeadingSpace = false;
    }

    BmhParser::Resolvable resolvable(const Definition* definition) const {
        MarkType markType = definition->fMarkType;
        if (MarkType::kCode == markType) {
            for (auto child : definition->fChildren) {
                if (MarkType::kLiteral == child->fMarkType) {
                    return BmhParser::Resolvable::kLiteral;
                }
            }
        }
        if ((MarkType::kExample == markType
                || MarkType::kFunction == markType) && fHasFiddle) {
            return BmhParser::Resolvable::kNo;
        }
        return fBmhParser.fMaps[(int) markType].fResolve;
    }

    void resolveOut(const char* start, const char* end, BmhParser::Resolvable );
    void rowOut(const char * name, const string& description);
    void subtopicOut(const TableContents& tableContents);
    void subtopicsOut();

    unordered_map<string, TableContents> fPopulators;
    vector<const Definition*> fClassStack;

    const BmhParser& fBmhParser;
    const Definition* fEnumClass;
    Definition* fMethod;
    const RootDefinition* fRoot;
    const Definition* fLastParam;
    TableState fTableState;
    bool fHasFiddle;
    bool fInDescription;   // FIXME: for now, ignore unfound camelCase in description since it may
                           // be defined in example which at present cannot be linked to
    bool fInList;
    bool fRespectLeadingSpace;
    typedef ParserCommon INHERITED;
};


// some methods cannot be trivially parsed; look for class-name / ~ / operator
class MethodParser : public TextParser {
public:
    MethodParser(const string& className, const string& fileName,
            const char* start, const char* end, int lineCount)
        : TextParser(fileName, start, end, lineCount)
        , fClassName(className) {
    }

    ~MethodParser() override {}

    void skipToMethodStart() {
        if (!fClassName.length()) {
            this->skipToAlphaNum();
            return;
        }
        while (!this->eof() && !isalnum(this->peek()) && '~' != this->peek()) {
            this->next();
        }
    }

    void skipToMethodEnd() {
        if (this->eof()) {
            return;
        }
        if (fClassName.length()) {
            if ('~' == this->peek()) {
                this->next();
                if (!this->startsWith(fClassName.c_str())) {
                    --fChar;
                    return;
                }
            }
            if (this->startsWith(fClassName.c_str()) || this->startsWith("operator")) {
                const char* ptr = this->anyOf("\n (");
                if (ptr && '(' ==  *ptr) {
                    this->skipToEndBracket(')');
                    SkAssertResult(')' == this->next());
                    this->skipExact("_const");
                    return;
                }
            }
        }
        if (this->startsWith("Sk") && this->wordEndsWith(".h")) {  // allow include refs
            this->skipToNonAlphaNum();
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
    typedef TextParser INHERITED;
};

bool SelfCheck(const BmhParser& );

#endif
