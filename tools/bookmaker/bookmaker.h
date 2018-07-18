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
    kTypename,
    kUint16_t,
    kUint32_t,
    kUint64_t,
    kUint8_t,
    kUintPtr_t,
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
    kDeprecated,
    kDescription,
    kDetails,  // used by #Const to specify #Subtopic details with examples and so on
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
    kLink,     // used internally by #Anchor
    kList,
    kLiteral,  // don't lookup hyperlinks, do substitution, etc
    kMarkChar,
    kMember,
    kMethod,
    kNoExample,
    kNoJustify, // don't contribute this #Line to tabular comment measure, even if it fits
    kOutdent,
    kParam,
    kPhraseDef,
    kPhraseParam,
    kPhraseRef,
    kPlatform,
    kPopulate,
    kPrivate,
    kReturn,
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
    kToDo,
    kTopic,
    kTypedef,
    kUnion,
    kVolatile,
    kWidth,
};

static inline bool IncompleteAllowed(MarkType markType) {
    return MarkType::kDeprecated == markType || MarkType::kExperimental == markType;
}

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

class NonAssignable {
public:
    NonAssignable(NonAssignable const&) = delete;
    NonAssignable& operator=(NonAssignable const&) = delete;
    NonAssignable() {}
};

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

    enum class Details {
        kNone,
        kSoonToBe_Deprecated,
        kTestingOnly_Experiment,
        kDoNotUse_Experiment,
        kNotReady_Experiment,
    };

    enum class DetailsType {
        kPhrase,
        kSentence,
    };

    Definition() {}

    Definition(const char* start, const char* end, int line, Definition* parent, char mc)
        : fStart(start)
        , fContentStart(start)
        , fContentEnd(end)
        , fParent(parent)
        , fLineCount(line)
        , fType(Type::kWord)
        , fMC(mc) {
        if (parent) {
            SkASSERT(parent->fFileName.length() > 0);
            fFileName = parent->fFileName;
        }
        this->setParentIndex();
    }

    Definition(MarkType markType, const char* start, int line, Definition* parent, char mc)
        : Definition(markType, start, nullptr, line, parent, mc) {
    }

    Definition(MarkType markType, const char* start, const char* end, int line, Definition* parent, char mc)
        : Definition(start, end, line, parent, mc) {
        fMarkType = markType;
        fType = Type::kMark;
    }

    Definition(Bracket bracket, const char* start, int lineCount, Definition* parent, char mc)
        : Definition(start, nullptr, lineCount, parent, mc) {
        fBracket = bracket;
        fType = Type::kBracket;
    }

    Definition(KeyWord keyWord, const char* start, const char* end, int lineCount,
            Definition* parent, char mc)
        : Definition(start, end, lineCount, parent, mc) {
        fKeyWord = keyWord;
        fType = Type::kKeyWord;
    }

    Definition(Punctuation punctuation, const char* start, int lineCount, Definition* parent, char mc)
        : Definition(start, nullptr, lineCount, parent, mc) {
        fPunctuation = punctuation;
        fType = Type::kPunctuation;
    }

    virtual ~Definition() {}

    virtual RootDefinition* asRoot() { SkASSERT(0); return nullptr; }
    bool boilerplateIfDef();

    bool boilerplateEndIf() {
        return true;
    }

    bool checkMethod() const;
    bool crossCheck2(const Definition& includeToken) const;
    bool crossCheck(const Definition& includeToken) const;
    bool crossCheckInside(const char* start, const char* end, const Definition& includeToken) const;

    Definition* csParent() {
        Definition* test = fParent;
        while (test) {
            if (MarkType::kStruct == test->fMarkType || MarkType::kClass == test->fMarkType) {
                return test;
            }
            test = test->fParent;
        }
        return nullptr;
    }

    string fiddleName() const;
    string fileName() const;
    const Definition* findClone(string match) const;
    string formatFunction(Format format) const;
    const Definition* hasChild(MarkType markType) const;
    bool hasMatch(string name) const;
    const Definition* hasParam(string ref) const;
    string incompleteMessage(DetailsType ) const;
    bool isClone() const { return fClone; }

    const Definition* iRootParent() const {
        const Definition* test = fParent;
        while (test) {
            if (KeyWord::kClass == test->fKeyWord || KeyWord::kStruct == test->fKeyWord) {
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

    const char* methodEnd() const;
    bool methodHasReturn(string name, TextParser* methodParser) const;
    string methodName() const;
    bool nextMethodParam(TextParser* methodParser, const char** nextEndPtr,
                         string* paramName) const;
    static string NormalizedName(string name);
    bool paramsMatch(string fullRef, string name) const;
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

    const Definition* subtopicParent() const {
        Definition* test = fParent;
        while (test) {
            if (MarkType::kTopic == test->fMarkType || MarkType::kSubtopic == test->fMarkType) {
                return test;
            }
            test = test->fParent;
        }
        return nullptr;
    }

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
    char fMC = '#';
    bool fClone = false;
    bool fCloned = false;
    bool fDeprecated = false;
    bool fOperatorConst = false;
    bool fPrivate = false;
    Details fDetails = Details::kNone;
    bool fMemberStart = false;
    bool fAnonymous = false;
    mutable bool fVisited = false;
};

class SubtopicKeys {
public:
    static constexpr const char* kClasses = "Class";
    static constexpr const char* kConstants = "Constant";
    static constexpr const char* kConstructors = "Constructor";
    static constexpr const char* kDefines = "Define";
    static constexpr const char* kMemberFunctions = "Member_Function";
    static constexpr const char* kMembers = "Member";
    static constexpr const char* kOperators = "Operator";
    static constexpr const char* kOverview = "Overview";
    static constexpr const char* kRelatedFunctions = "Related_Function";
    static constexpr const char* kStructs = "Struct";
    static constexpr const char* kTypedefs = "Typedef";

    static const char* kGeneratedSubtopics[];
};

class RootDefinition : public Definition {
public:
    enum class AllowParens {
        kNo,
        kYes,
    };

    struct SubtopicContents {
        SubtopicContents()
            : fShowClones(false) {
        }

        vector<Definition*> fMembers;
        bool fShowClones;
    };

    RootDefinition() {
    }

    RootDefinition(MarkType markType, const char* start, int line, Definition* parent, char mc)
            : Definition(markType, start, line, parent, mc) {
    }

    RootDefinition(MarkType markType, const char* start, const char* end, int line,
            Definition* parent, char mc) : Definition(markType, start, end,  line, parent, mc) {
    }

    ~RootDefinition() override {
        for (auto& iter : fBranches) {
            delete iter.second;
        }
    }

    RootDefinition* asRoot() override { return this; }
    void clearVisited();
    bool dumpUnVisited();
    Definition* find(string ref, AllowParens );
    bool isRoot() const override { return true; }

    SubtopicContents& populator(const char* key) {
        return fPopulators[key];
    }

    RootDefinition* rootParent() override { return fRootParent; }
    const RootDefinition* rootParent() const override { return fRootParent; }
    void setRootParent(RootDefinition* rootParent) { fRootParent = rootParent; }

    unordered_map<string, RootDefinition*> fBranches;
    unordered_map<string, Definition> fLeaves;
    unordered_map<string, SubtopicContents> fPopulators;
private:
    RootDefinition* fRootParent = nullptr;
};

struct IClassDefinition : public Definition {
    unordered_map<string, Definition*> fConsts;
    unordered_map<string, Definition*> fDefines;
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

    char* FindDateTime(char* buffer, int size);

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

    char* ReadToBuffer(string filename, int* size);

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

    bool writeBlockIndent(int size, const char* data);

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

    bool writtenFileDiffers(string filename, string readname);

    unordered_map<string, sk_sp<SkData>> fRawData;
    unordered_map<string, vector<char>> fLFOnly;
    vector<IndentState> fIndentStack;
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
    bool fValidate;    // set true to check anchor defs and refs
    bool fOutdentNext; // set at end of embedded struct to prevent premature outdent
    bool fWroteSomething; // used to detect empty content; an alternative source is preferable
    bool fReturnOnWrite; // used to detect non-empty content; allowing early return

private:
    typedef TextParser INHERITED;
};

struct JsonStatus {
    const Json::Value& fObject;
    Json::Value::iterator fIter;
    string fName;
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
    bool next(string* file);
private:
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
        kOut,     // mostly resolved, output (FIXME: is this really different from kYes?)
        kFormula, // resolve methods as they are used, not as they are prototyped
        kLiteral, // output untouched
		kClone,   // resolved, output, with references to clones as well
        kSimple,  // resolve simple words (used to resolve method declarations)
    };

    enum class ExampleOptions {
        kText,
        kPng,
        kAll
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

    enum class TrimExtract {
        kNo,
        kYes
    };

    BmhParser(bool skip) : ParserCommon()
        , fMaps {
          { &fClassMap,    MarkType::kClass }
        , { &fConstMap,    MarkType::kConst }
        , { &fDefineMap,   MarkType::kDefine }
        , { &fEnumMap,     MarkType::kEnum }
        , { &fClassMap,    MarkType::kEnumClass }
        , { &fMethodMap,   MarkType::kMethod }
        , { &fClassMap,    MarkType::kStruct }
        , { &fTypedefMap,  MarkType::kTypedef }
        }
        , fSkip(skip) {
            this->reset();
        }

    ~BmhParser() override {}

    bool addDefinition(const char* defStart, bool hasEnd, MarkType markType,
            const vector<string>& typeNameBuilder, HasTag hasTag);
    bool checkEndMarker(MarkType markType, string name) const;
    bool checkExamples() const;
    const char* checkForFullTerminal(const char* end, const Definition* ) const;
    bool checkParamReturn(const Definition* definition) const;
    bool dumpExamples(FILE* fiddleOut, Definition& def, bool* continuation) const;
    bool dumpExamples(const char* fiddleJsonFileName) const;
    bool checkExampleHashes() const;
    bool childOf(MarkType markType) const;
    string className(MarkType markType);
    bool collectExternals();
    int endHashCount() const;
    bool endTableColumn(const char* end, const char* terminator);
    bool exampleToScript(Definition*, ExampleOptions, string* result ) const;
    string extractText(const Definition* , TrimExtract ) const;
    RootDefinition* findBmhObject(MarkType markType, string typeName);
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
    void reportDuplicates(const Definition& def, string dup) const;
    void resetExampleHashes();

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

    void setWrapper(Definition* def) const;
    bool skipNoName();
    bool skipToDefinitionEnd(MarkType markType);
	bool skipToString();
    void spellCheck(const char* match, SkCommandLineFlags::StringArray report) const;
    void spellStatus(const char* match, SkCommandLineFlags::StringArray report) const;
    vector<string> topicName();
    vector<string> typeName(MarkType markType, bool* expectEnd);
    string typedefName() override;
    string uniqueName(string base, MarkType markType);
    string uniqueRootName(string base, MarkType markType);
    void validate() const;
    string word(string prefix, string delimiter);

public:
    struct MarkProps {
        const char* fName;
        MarkType fMarkType;
        Resolvable fResolve;
        Exemplary fExemplary;  // worthy of an example
        uint64_t fParentMask;
    };

    struct DefinitionMap {
        unordered_map<string, RootDefinition>* fMap;
        MarkType fMarkType;
    };

    vector<DefinitionMap> fMaps;

    static MarkProps kMarkProps[Last_MarkType + 1];
    forward_list<RootDefinition> fTopics;
    forward_list<Definition> fMarkup;
    forward_list<RootDefinition> fExternals;
    vector<string> fInputFiles;
    unordered_map<string, RootDefinition> fClassMap;
    unordered_map<string, RootDefinition> fConstMap;
    unordered_map<string, RootDefinition> fDefineMap;
    unordered_map<string, RootDefinition> fEnumMap;
    unordered_map<string, RootDefinition> fMethodMap;
    unordered_map<string, RootDefinition> fTypedefMap;
    unordered_map<string, Definition*> fTopicMap;
    unordered_map<string, Definition*> fAliasMap;
    unordered_map<string, Definition*> fPhraseMap;
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
          { &fIConstMap,    MarkType::kConst }
        , { &fIDefineMap,   MarkType::kDefine }
        , { &fIEnumMap,     MarkType::kEnum }
        , { &fIEnumMap,     MarkType::kEnumClass }
        , { &fIStructMap,   MarkType::kStruct }
        , { &fITemplateMap, MarkType::kTemplate }
        , { &fITypedefMap,  MarkType::kTypedef }
        , { &fIUnionMap,    MarkType::kUnion }
        }
    {
        this->reset();
    }

    ~IncludeParser() override {}

    void addKeyword(KeyWord keyWord);

    void addPunctuation(Punctuation punctuation) {
        fParent->fTokens.emplace_back(punctuation, fChar, fLineCount, fParent, '\0');
    }

    void addWord() {
        fParent->fTokens.emplace_back(fIncludeWord, fChar, fLineCount, fParent, '\0');
        fIncludeWord = nullptr;
    }

    void checkForMissingParams(const vector<string>& methodParams,
                               const vector<string>& foundParams);
    bool checkForWord();
    string className() const;
    bool crossCheck(BmhParser& );
    IClassDefinition* defineClass(const Definition& includeDef, string className);
    void dumpClassTokens(IClassDefinition& classDef);
    void dumpComment(const Definition& );
    void dumpCommonTail(const Definition& );
    void dumpConst(const Definition& , string className);
    void dumpDefine(const Definition& );
    void dumpEnum(const Definition& , string name);
    bool dumpGlobals();
    void dumpMethod(const Definition& , string className);
    void dumpMember(const Definition& );
    bool dumpTokens();
    bool dumpTokens(string skClassName);
    bool findComments(const Definition& includeDef, Definition* markupDef);
    Definition* findIncludeObject(const Definition& includeDef, MarkType markType,
                                  string typeName);
    static KeyWord FindKey(const char* start, const char* end);
    Bracket grandParentBracket() const;
    bool isClone(const Definition& token);
    bool isConstructor(const Definition& token, string className);
    bool isInternalName(const Definition& token);
    bool isOperator(const Definition& token);
    Definition* parentBracket(Definition* parent) const;
    bool parseChar();
    bool parseComment(string filename, const char* start, const char* end, int lineCount,
            Definition* markupDef);
    bool parseClass(Definition* def, IsStruct);
    bool parseConst(Definition* child, Definition* markupDef);
    bool parseDefine(Definition* child, Definition* markupDef);
    bool parseEnum(Definition* child, Definition* markupDef);

    bool parseFromFile(const char* path) override {
        this->reset();
        if (!INHERITED::parseSetup(path)) {
            return false;
        }
        string name(path);
        return this->parseInclude(name);
    }

    bool parseInclude(string name);
    bool parseMember(Definition* child, Definition* markupDef);
    bool parseMethod(Definition* child, Definition* markupDef);
    bool parseObject(Definition* child, Definition* markupDef);
    bool parseObjects(Definition* parent, Definition* markupDef);
    bool parseTemplate(Definition* child, Definition* markupDef);
    bool parseTypedef(Definition* child, Definition* markupDef);
    bool parseUnion();

    void popBracket() {
        if (Definition::Type::kKeyWord == fParent->fType
                && KeyWord::kTypename == fParent->fKeyWord) {
            this->popObject();
        }
        SkASSERT(Definition::Type::kBracket == fParent->fType);
        this->popObject();
        Bracket bracket = this->topBracket();
        this->setBracketShortCuts(bracket);
    }

    void pushBracket(Bracket bracket) {
        this->setBracketShortCuts(bracket);
        fParent->fTokens.emplace_back(bracket, fChar, fLineCount, fParent, '\0');
        Definition* container = &fParent->fTokens.back();
        this->addDefinition(container);
    }

    bool references(const SkString& file) const;

    static void RemoveFile(const char* docs, const char* includes);
    static void RemoveOneFile(const char* docs, const char* includesFileOrPath);

    void reset() override {
        INHERITED::resetCommon();
        fRootTopic = nullptr;
        fConstExpr = nullptr;
        fInBrace = nullptr;
        fIncludeWord = nullptr;
        fLastObject = nullptr;
        fPriorEnum = nullptr;
        fPriorObject = nullptr;
        fAttrDeprecated = nullptr;
        fPrev = '\0';
        fInChar = false;
        fInCharCommentString = false;
        fInComment = false;
        fInDefine = false;
        fInEnum = false;
        fInFunction = false;
        fInString = false;
        fFailed = false;
    }

    void setBracketShortCuts(Bracket bracket) {
        fInComment = Bracket::kSlashSlash == bracket || Bracket::kSlashStar == bracket;
        fInString = Bracket::kString == bracket;
        fInChar = Bracket::kChar == bracket;
        fInCharCommentString = fInChar || fInComment || fInString;
    }

    Bracket topBracket() const;

    template <typename T>
    string uniqueName(const unordered_map<string, T>& m, string typeName) {
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

    void writeDefinition(const Definition& def, string name, int spaces) {
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

    void writeEndTag(const char* tagType, string tagID, int spaces = 1) {
        this->writeEndTag(tagType, tagID.c_str(), spaces);
    }

    void writeIncompleteTag(const char* tagType, string tagID, int spaces = 1) {
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

    void writeTableRow(size_t pad, string col1) {
        this->lf(1);
        string row = "# " + col1 + string(pad - col1.length(), ' ') + " # ##";
        this->writeString(row);
        this->lf(1);
    }

    void writeTableRow(size_t pad1, string col1, size_t pad2, string col2) {
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

    void writeTagNoLF(const char* tagType, string tagID) {
        this->writeTagNoLF(tagType, tagID.c_str());
    }

    void writeTag(const char* tagType, const char* tagID) {
        this->lf(1);
        this->writeTagNoLF(tagType, tagID);
    }

    void writeTag(const char* tagType, string tagID) {
        this->writeTag(tagType, tagID.c_str());
    }

    void writeTagTable(string tagType, string body) {
        this->writeTag(tagType.c_str());
        this->writeSpace(1);
        this->writeString("#");
        this->writeSpace(1);
        this->writeString(body);
        this->writeSpace(1);
        this->writeString("##");
    }

protected:
    static void ValidateKeyWords();

    struct DefinitionMap {
        unordered_map<string, Definition*>* fInclude;
        MarkType fMarkType;
    };

    static const char gAttrDeprecated[];
    static const size_t kAttrDeprecatedLen;

    vector<DefinitionMap> fMaps;
    unordered_map<string, Definition> fIncludeMap;
    list<Definition> fGlobals;
    unordered_map<string, IClassDefinition> fIClassMap;
    unordered_map<string, Definition*> fIConstMap;
    unordered_map<string, Definition*> fIDefineMap;
    unordered_map<string, Definition*> fIEnumMap;
    unordered_map<string, Definition*> fIFunctionMap;
    unordered_map<string, Definition*> fIStructMap;
    unordered_map<string, Definition*> fITemplateMap;
    unordered_map<string, Definition*> fITypedefMap;
    unordered_map<string, Definition*> fIUnionMap;
    Definition* fRootTopic;
    Definition* fConstExpr;
    Definition* fInBrace;
    Definition* fLastObject;
    Definition* fPriorEnum;
    Definition* fPriorObject;
    const Definition* fAttrDeprecated;
    int fPriorIndex;
    const char* fIncludeWord;
    char fPrev;
    bool fInChar;
    bool fInCharCommentString;
    bool fInComment;
    bool fInDefine;
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

    enum class MemberPass {
        kCount,
        kOut,
    };

    enum class ItemState {
        kNone,
        kName,
        kValue,
        kComment,
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

    struct Preprocessor {
        Preprocessor() {
            reset();
        }

        void reset() {
            fDefinition = nullptr;
            fStart = nullptr;
            fEnd = nullptr;
            fWord = false;
        }

        const Definition* fDefinition;
        const char* fStart;
        const char* fEnd;
        bool fWord;
    };

    struct Item {
        void reset() {
            fName = "";
            fValue = "";
        }

        string fName;
        string fValue;
    };

    struct LastItem {
        const char* fStart;
        const char* fEnd;
    };

    struct ItemLength {
        int fCurName;
        int fCurValue;
        int fLongestName;
        int fLongestValue;
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

    bool checkChildCommentLength(const Definition* parent, MarkType childType) const;
    void checkEnumLengths(const Definition& child, string enumName, ItemLength* length) const;
	void constOut(const Definition* memberStart, const Definition* bmhConst);
    void constSizeMembers(const RootDefinition* root);
    bool defineOut(const Definition& );
    bool descriptionOut(const Definition* def, SkipFirstLine , Phrase );
    void enumHeaderOut(RootDefinition* root, const Definition& child);
    string enumMemberComment(const Definition* currentEnumItem, const Definition& child) const;
    const Definition* enumMemberForComment(const Definition* currentEnumItem) const;
    ItemState enumMemberName(const Definition& child,
            const Definition* token, Item* , LastItem* , const Definition** enumItem);
    void enumMemberOut(const Definition* currentEnumItem, const Definition& child,
            const Item& , Preprocessor& );
    void enumMembersOut(Definition& child);
    bool enumPreprocessor(Definition* token, MemberPass pass,
        vector<IterState>& iterStack, IterState** iterState, Preprocessor* );
    void enumSizeItems(const Definition& child);
    bool findEnumSubtopic(string undername, const Definition** ) const;
    void firstBlock(int size, const char* data);
    bool firstBlockTrim(int size, const char* data);
	Definition* findMemberCommentBlock(const vector<Definition*>& bmhChildren, string name) const;
    Definition* findMethod(string name, RootDefinition* ) const;

    void indentDeferred(IndentKind kind) {
        if (fIndentNext) {
            this->indentIn(kind);
            fIndentNext = false;
        }
    }

    int lookupMethod(const PunctuationState punctuation, const Word word,
            const int start, const int run, int lastWrite,
            const char* data, bool hasIndirection);
    int lookupReference(const PunctuationState punctuation, const Word word,
            const int start, const int run, int lastWrite, const char last,
            const char* data);
    const Definition* matchMemberName(string matchName, const Definition& child) const;
    void methodOut(Definition* method, const Definition& child);
    bool populate(Definition* def, ParentPair* parentPair, RootDefinition* root);
    bool populate(BmhParser& bmhParser);

    void reset() override {
        INHERITED::resetCommon();
        fBmhParser = nullptr;
        fDeferComment = nullptr;
        fBmhMethod = nullptr;
        fEnumDef = nullptr;
        fMethodDef = nullptr;
        fBmhConst = nullptr;
        fConstDef = nullptr;
        fLastDescription = nullptr;
        fStartSetter = nullptr;
        fBmhStructDef = nullptr;
        fContinuation = nullptr;
        fInStruct = false;
        fWroteMethod = false;
        fIndentNext = false;
        fPendingMethod = false;
        fFirstWrite = false;
        fStructEnded = false;
    }

    string resolveAlias(const Definition* );
    string resolveMethod(const char* start, const char* end, bool first);
    string resolveRef(const char* start, const char* end, bool first, RefType* refType);
    Wrote rewriteBlock(int size, const char* data, Phrase phrase);
    void setStart(const char* start, const Definition * );
    void setStartBack(const char* start, const Definition * );
    Definition* structMemberOut(const Definition* memberStart, const Definition& child);
    void structOut(const Definition* root, const Definition& child,
            const char* commentStart, const char* commentEnd);
    void structSizeMembers(const Definition& child);
    bool writeHeader(std::pair<const string, Definition>& );
private:
    vector<const Definition* > fICSStack;
    BmhParser* fBmhParser;
    Definition* fDeferComment;
    const Definition* fBmhMethod;
    const Definition* fEnumDef;
    const Definition* fMethodDef;
    const Definition* fBmhConst;
    const Definition* fConstDef;
    const Definition* fLastDescription;
    const Definition* fStartSetter;
    Definition* fBmhStructDef;
    const char* fContinuation;  // used to construct paren-qualified method name
    int fAnonymousEnumCount;
    int fEnumItemValueTab;
    int fEnumItemCommentTab;
    int fStructMemberTab;
    int fStructValueTab;
    int fStructCommentTab;
    int fStructMemberLength;
    int fConstValueTab;
    int fConstCommentTab;
    int fConstLength;
    bool fInStruct;  // set if struct is inside class
    bool fWroteMethod;
    bool fIndentNext;
    bool fPendingMethod;
    bool fFirstWrite;  // set to write file information just after includes
    bool fStructEnded;  // allow resetting indent after struct is complete

    typedef IncludeParser INHERITED;
};

class FiddleBase : public JsonCommon {
protected:
    FiddleBase(BmhParser* bmh)
        : fBmhParser(bmh)
        , fContinuation(false)
        , fTextOut(false)
        , fPngOut(false)
    {
        this->reset();
    }

    void reset() override {
        INHERITED::reset();
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
    typedef JsonCommon INHERITED;
};

class FiddleParser : public FiddleBase {
public:
    FiddleParser(BmhParser* bmh) : FiddleBase(bmh) {
       fTextOut = true;
    }

    bool parseFromFile(const char* path) override {
        if (!INHERITED::parseFromFile(path)) {
            return false;
        }
        fBmhParser->resetExampleHashes();
        if (!INHERITED::parseFiddles()) {
            return false;
        }
        return fBmhParser->checkExampleHashes();
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

    bool appendFile(string path);
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
    const Definition* fClasses;
    const Definition* fConstants;
    const Definition* fConstructors;
    const Definition* fMemberFunctions;
    const Definition* fMembers;
    const Definition* fOperators;
    const Definition* fRelatedFunctions;
    const Definition* fStructs;
    bool hackFiles();

    typedef ParserCommon INHERITED;
};

class MdOut : public ParserCommon {
public:
    struct SubtopicDescriptions {
        string fName;
        string fOneLiner;
        string fDetails;
    };

    MdOut(BmhParser& bmh) : ParserCommon()
        , fBmhParser(bmh) {
        this->reset();
        this->addPopulators();
    }

    bool buildReferences(const IncludeParser& , const char* docDir, const char* mdOutDirOrFile);
    bool buildStatus(const char* docDir, const char* mdOutDir);
    void checkAnchors();

private:
    enum class TableState {
        kNone,
        kRow,
        kColumn,
    };

    struct AnchorDef {
        string fDef;
        MarkType fMarkType;
    };

    void addPopulators();
    string addReferences(const char* start, const char* end, BmhParser::Resolvable );
    string anchorDef(string def, string name);
    string anchorLocalRef(string ref, string name);
    string anchorRef(string def, string name);

    bool buildRefFromFile(const char* fileName, const char* outDir);
    bool checkParamReturnBody(const Definition* def);
    Definition* checkParentsForMatch(Definition* test, string ref) const;
    void childrenOut(Definition* def, const char* contentStart);
    Definition* csParent();
    const Definition* findParamType();
    string getMemberTypeName(const Definition* def, string* memberType);
    static bool HasDetails(const Definition* def);
    void htmlOut(string );
    const Definition* isDefined(const TextParser& , string ref, BmhParser::Resolvable );
    const Definition* isDefinedByParent(RootDefinition* root, string ref);
    string linkName(const Definition* ) const;
    string linkRef(string leadingSpaces, const Definition*, string ref, BmhParser::Resolvable );
    void markTypeOut(Definition* , const Definition** prior);
    void mdHeaderOut(int depth) { mdHeaderOutLF(depth, 2); }
    void mdHeaderOutLF(int depth, int lf);
    bool parseFromFile(const char* path) override { return true; }
    void populateOne(Definition* def,
            unordered_map<string, RootDefinition::SubtopicContents>& populator);
    void populateTables(const Definition* def, RootDefinition* );

    SubtopicDescriptions& populator(string key) {
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
        fSubtopic = nullptr;
        fLastParam = nullptr;
        fTableState = TableState::kNone;
        fAddRefFailed = false;
        fHasFiddle = false;
        fInDescription = false;
        fInList = false;
        fResolveAndIndent = false;
        fLiteralAndIndent = false;
        fLastDef = nullptr;
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
        return BmhParser::kMarkProps[(int) markType].fResolve;
    }

    void resolveOut(const char* start, const char* end, BmhParser::Resolvable );
    void rowOut(const char * name, string description, bool literalName);

    void subtopicOut(string name);
    void subtopicsOut(Definition* def);
    void summaryOut(const Definition* def, MarkType , string name);
    string tableDataCodeDef(const Definition* def);
    string tableDataCodeDef(string def, string name);
    string tableDataCodeLocalRef(string name);
    string tableDataCodeLocalRef(string ref, string name);
    string tableDataCodeRef(const Definition* ref);
    string tableDataCodeRef(string ref, string name);

    vector<const Definition*> fClassStack;
    unordered_map<string, vector<AnchorDef> > fAllAnchorDefs;
    unordered_map<string, vector<string> > fAllAnchorRefs;

    BmhParser& fBmhParser;
    const Definition* fEnumClass;
     const Definition* fLastDef;
    Definition* fMethod;
    RootDefinition* fRoot;  // used in generating populated tables; always struct or class
    RootDefinition* fSubtopic; // used in resolving symbols
    const Definition* fLastParam;
    TableState fTableState;
    unordered_map<string, SubtopicDescriptions> fPopulators;
    unordered_map<string, string> fPhraseParams;
    bool fAddRefFailed;
    bool fHasFiddle;
    bool fInDescription;   // FIXME: for now, ignore unfound camelCase in description since it may
                           // be defined in example which at present cannot be linked to
    bool fInList;
    bool fLiteralAndIndent;
    bool fResolveAndIndent;
    bool fOddRow;
    bool fHasDetails;
    typedef ParserCommon INHERITED;
};


// some methods cannot be trivially parsed; look for class-name / ~ / operator
class MethodParser : public TextParser {
public:
    MethodParser(string className, string fileName,
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

    void skipToMethodEnd(BmhParser::Resolvable resolvable) {
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
            if (BmhParser::Resolvable::kSimple != resolvable
                    && (this->startsWith(fClassName.c_str()) || this->startsWith("operator"))) {
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
            this->skipToNonName();
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

