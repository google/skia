/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef bookmaker_DEFINED
#define bookmaker_DEFINED

#include "SkData.h"

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
    kUint32_t,
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
    kEnum,
    kEnumClass,
    kError,
    kExample,
    kExternal,
    kFile,
    kFormula,
    kFunction,
    kHeight,
    kImage,
    kLegend,
    kLink,
    kList,
    kMember,
    kMethod,
    kParam,
    kPlatform,
    kReturn,
    kRoot,
    kRow,
    kSeeAlso,
    kStdOut,
    kStruct,
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
};

enum class Punctuation {  // catch-all for misc symbols tracked in C
    kNone,
    kAsterisk,  // for pointer-to
    kSemicolon,  // e.g., to delinate xxx() const ; const int* yyy()
    kLeftBrace,
    kColon,     // for foo() : bar(1), baz(2) {}
};

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

struct TextParser : public NonAssignable {
    TextParser() 
        : TextParser(nullptr, nullptr)
    {
    }

    TextParser(const char* start, const char* end)
        : fStart(start)
        , fLine(start)
        , fChar(start)
        , fEnd(end)
        , fLineCount(0)
    {
    }

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

    bool contains(const char* match, const char* lineEnd, const char** loc) const {
        *loc = this->strnstr(match, lineEnd);
        return *loc;
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

    void reportError(const char* errorStr) const;

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

    bool skipToEndBracket(char endBracket) {
        while (fChar[0] != endBracket) {
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
                || (':' == fChar[0] && fChar +1 < fEnd && ':' == fChar[1])
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

    bool skipName(const char* word) {
        const char* end = fChar;
        while (*end > ' ') {
            ++end;
        }
        if (!end || end >= fEnd) {
            return false;
        }
        size_t len = strlen(word);
        if (len == (size_t) (end - fChar) && !strncmp(word, fChar, len)) {
            fChar += len;
            if (' ' < this->peek()) {
                return false;
            }
        }
        return true;
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
        if (!this->skipName(word)) {
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
        ptrdiff_t lineLen = this->lineLength(); 
        return len <= (size_t) lineLen && 0 == strncmp(str, fChar, len);
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
        ptrdiff_t len = end - fChar;
        SkASSERT(len >= 0);
        if ((size_t) len < matchLen ) {
            return nullptr;
        }
        size_t count = len - matchLen;
        for (size_t index = 0; index <= count; index++) {
            if (fChar[index] == match[0] && 0 == strncmp(&fChar[index], match, matchLen)) {
                return &fChar[index];
            }
        }
        return nullptr;
    }

    const char* trimmedBracketEnd(const char bracket) const {
        int max = (int) this->lineLength();
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

    const char* wordEnd() const {
        const char* end = fChar;
        while (isalnum(end[0]) || '_' == end[0] || '-' == end[0]) {
            ++end;
        }
        return end;
    }

    const char* fStart;
    const char* fLine;
    const char* fChar;
    const char* fEnd;
    size_t fLineCount;
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
    };

    enum class TrimExtract {
        kNo,
        kYes
    };

    Definition() {}

    Definition(const char* start, const char* end, int line, Definition* parent) 
        : fStart(start)
        , fContentStart(start)
        , fContentEnd(end)
        , fParent(parent)
        , fLineCount(line)
        , fType(Type::kWord) {
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

    bool boilerplateIfDef(Definition* parent) {
        const Definition& label = fTokens.front();
        if (Type::kWord != label.fType) {
            return false;
        }
        fName = string(label.fContentStart, label.fContentEnd - label.fContentStart);
        return true;
   }

    // todo: this is matching #ifndef SkXXX_DEFINED for no particular reason
    // it doesn't do anything useful with arbitrary input, e.g. #ifdef SK_SUPPORT_LEGACY_CANVAS_HELPERS
// also doesn't know what to do with SK_REQUIRE_LOCAL_VAR()
    bool boilerplateDef(Definition* parent) {
        if (!this->boilerplateIfDef(parent)) {
            return false;
        }
        const char* s = fName.c_str();
        const char* e = strchr(s, '_');
        return true; // fixme: if this is trying to do something useful with define, do it here
        if (!e) {
            return false;
        }
        string prefix(s, e - s);
        const char* inName = strstr(parent->fName.c_str(), prefix.c_str());
        if (!inName) {
            return false;
        }
        if ('/' != inName[-1] && '\\' != inName[-1]) {
            return false;
        }
        if (strcmp(inName + prefix.size(), ".h")) {
            return false;
        }
        return true;
    }

    bool boilerplateEndIf() {

        return true;
    }

    string colonFormName() const {
        string result(fName);
        size_t underscore = fName.find_first_of('_');
        if (string::npos != underscore) {
            result.replace(underscore, 1, "::");
        }
        return result;
    }

    bool exampleToScript(string* result) const;

    string extractText(TrimExtract trimExtract) const {
        string result;
        TextParser parser(fContentStart, fContentEnd);
        int childIndex = 0;
        char mc = '#';
        while (parser.fChar < parser.fEnd) {
            if (TrimExtract::kYes == trimExtract && !parser.skipWhiteSpace()) {
                break;
            }
            if (parser.next() == mc) {
                if (parser.next() == mc) {
                    if (parser.next() == mc) {
                        mc = parser.next();
                    }
                } else {
                    // fixme : more work to do if # style comment is in text
                    // if in method definition, could be alternate method name
                    --parser.fChar;
                    if (' ' < parser.fChar[0]) {
                        if (islower(parser.fChar[0])) {
                            result += '\n';
                            parser.skipLine();
                        } else {
                            SkASSERT(isupper(parser.fChar[0]));
                            parser.skipTo(fChildren[childIndex]->fTerminator);
                            if (mc == parser.fChar[0] && mc == parser.fChar[1]) {
                                parser.next();
                                parser.next();
                            }
                            childIndex++;
                        }
                    } else {
                        parser.skipLine();
                    }
                    continue;
                }
            } else {
                --parser.fChar;
            }
            const char* end = parser.fEnd;
            const char* mark = parser.strnchr(mc, end);
            if (mark) {
                end = mark;
            }
            string fragment(parser.fChar, end - parser.fChar);
            trim_end(fragment);
            if (TrimExtract::kYes == trimExtract) {
                trim_start(fragment);
                if (result.length()) {
                    result += '\n';
                    result += '\n';
                }
            }
            if (TrimExtract::kYes == trimExtract || has_nonwhitespace(fragment)) {
                result += fragment;
            }
            parser.skipTo(end);
        }
        return result;
    }

    string fiddleName() const;
    string formatFunction() const;

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

    int length() const {
        return (int) (fContentEnd - fContentStart);
    }

    string methodName() const;

    string printableName() const {
        string result(fName);
        std::replace(result.begin(), result.end(), '_', ' ');
        return result;
    }

    virtual RootDefinition* rootParent() { SkASSERT(0); return nullptr; }

    void setParentIndex() {
        fParentIndex = fParent ? (int) fParent->fTokens.size() : -1;
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
    int fLineCount = 0;
    int fParentIndex = 0;
    MarkType fMarkType = MarkType::kNone;
    KeyWord fKeyWord = KeyWord::kNone;
    Bracket fBracket = Bracket::kNone;
    Punctuation fPunctuation = Punctuation::kNone;
    Type fType = Type::kNone;
    bool fClone = false;
    mutable bool fVisited = false;
};

class RootDefinition : public Definition {
public:
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
    void dumpUnVisited();
    const Definition* find(const string& ref) const;
    bool isRoot() const override { return true; }
    RootDefinition* rootParent() override { return fRootParent; }
    void setRootParent(RootDefinition* rootParent) { fRootParent = rootParent; }

    unordered_map<string, RootDefinition*> fBranches;
    unordered_map<string, Definition> fLeaves;
    string fFileName;
private:
    RootDefinition* fRootParent = nullptr;
};

struct IClassDefinition : public Definition {
    unordered_map<string, Definition*> fEnums;
    unordered_map<string, Definition*> fMethods;
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
        , fFileName(nullptr)
        , fParent(nullptr)
    {
    }

    virtual ~ParserCommon() {
    }

    void addDefinition(Definition* def) {
        fParent->fChildren.push_back(def);
        fParent = def;
    }

    bool parseFile(const char* file, const char* suffix);

    virtual bool parseFromFile(const char* path) = 0;

    void parseSetup(const char* path) {
        this->reset();
        sk_sp<SkData> data = SkData::MakeFromFileName(path);
        if (nullptr == data.get()) {
            SkDebugf("%s missing\n", path);
            return;
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
        fFileName = path;
        fLineCount = 1;
    }

    void popObject() {
        fParent->fContentEnd = fParent->fTerminator = fChar;
        fParent = fParent->fParent;
    }

    virtual void reset() = 0;

    void resetCommon() {
        fLine = fChar = fStart;
        fLineCount = 0;
        fParent = nullptr;
    }

    template <typename T> T reportError(const char* errorStr) const {
        INHERITED::reportError(errorStr);
        return T();
    }

    void setAsParent(Definition* definition) {
        if (fParent) {
            fParent->fChildren.push_back(definition);
            definition->fParent = fParent;
        }
        fParent = definition;
    }

    unordered_map<string, sk_sp<SkData>> fRawData;
    unordered_map<string, vector<char>> fLFOnly;
    const char* fFileName;
    Definition* fParent;

private:
    typedef TextParser INHERITED;
};



class BmhParser : public ParserCommon {
public:
    enum class MarkLookup {
        kRequire,
        kAllowUnknown,
    };

    enum class Resolvable {
        kNo,
        kYes,
    };

#define M(mt) (1LL << (int) MarkType::k##mt)
#define M_CS M(Class) | M(Struct)
#define M_ST M(Subtopic) | M(Topic)
#define M_CSST M_CS | M_ST
#ifdef M_E
#undef M_E
#endif
#define M_E M(Enum) | M(EnumClass)

    BmhParser() : ParserCommon()
        , fMaps { 
// names without formal definitions (e.g. Column) aren't included
// fill in other names once they're actually used
          { "",            nullptr,        MarkType::kNone,        Resolvable::kYes,  0 }
        , { "A",           nullptr,        MarkType::kAnchor,      Resolvable::kYes,  0 }
        , { "Alias",       nullptr,        MarkType::kAlias,       Resolvable::kNo,   0 }
        , { "Bug",         nullptr,        MarkType::kBug,         Resolvable::kNo,   0 }
        , { "Class",       &fClassMap,     MarkType::kClass,       Resolvable::kYes,  M_CSST | M(Root) }
        , { "Code",        nullptr,        MarkType::kCode,        Resolvable::kYes,  M_CSST | M_E }      
        , { "",            nullptr,        MarkType::kColumn,      Resolvable::kYes,  M(Row) }
        , { "",            nullptr,        MarkType::kComment,     Resolvable::kNo,   0 }
        , { "Const",       &fConstMap,     MarkType::kConst,       Resolvable::kYes,  M_E | M_ST  }
        , { "",            nullptr,        MarkType::kDefine,      Resolvable::kYes,  0 }
        , { "DefinedBy",   nullptr,        MarkType::kDefinedBy,   Resolvable::kNo,   M(Method) }
        , { "Deprecated",  nullptr,        MarkType::kDeprecated,  Resolvable::kYes,  0 }
        , { "Description", nullptr,        MarkType::kDescription, Resolvable::kYes,  M(Example) }
        , { "Doxygen",     nullptr,        MarkType::kDoxygen,     Resolvable::kYes,  0 }
        , { "Enum",        &fEnumMap,      MarkType::kEnum,        Resolvable::kYes,  M_CSST | M(Root) }
        , { "EnumClass",   &fClassMap,     MarkType::kEnumClass,   Resolvable::kYes,  M_CSST | M(Root) }
        , { "Error",       nullptr,        MarkType::kError,       Resolvable::kNo,   M(Example) }
        , { "Example",     nullptr,        MarkType::kExample,     Resolvable::kNo,   M_CSST | M_E | M(Method) }
        , { "External",    nullptr,        MarkType::kExternal,    Resolvable::kYes,  M(Root) }
        , { "File",        nullptr,        MarkType::kFile,        Resolvable::kNo,   M(Track) }
        , { "Formula",     nullptr,        MarkType::kFormula,     Resolvable::kYes,  M(Subtopic) | M(Topic) }
        , { "Function",    nullptr,        MarkType::kFunction,    Resolvable::kNo,   M(Example) }
        , { "Height",      nullptr,        MarkType::kHeight,      Resolvable::kNo,   M(Example) }
        , { "Image",       nullptr,        MarkType::kImage,       Resolvable::kNo,   M(Example) }
        , { "Legend",      nullptr,        MarkType::kLegend,      Resolvable::kYes,  M(Table) }
        , { "",            nullptr,        MarkType::kLink,        Resolvable::kYes,  0 }
        , { "List",        nullptr,        MarkType::kList,        Resolvable::kYes,  M_ST | M(Method) }
        , { "Member",      nullptr,        MarkType::kMember,      Resolvable::kYes,  M(Class) | M(Struct) }
        , { "Method",      &fMethodMap,    MarkType::kMethod,      Resolvable::kYes,  M_CSST }
        , { "Param",       nullptr,        MarkType::kParam,       Resolvable::kYes,  M(Method) }
        , { "Platform",    nullptr,        MarkType::kPlatform,    Resolvable::kYes,  M(Example) }
        , { "Return",      nullptr,        MarkType::kReturn,      Resolvable::kYes,  M(Method) }
        , { "",            nullptr,        MarkType::kRoot,        Resolvable::kYes,  0 }
        , { "",            nullptr,        MarkType::kRow,         Resolvable::kYes,  M(Table) }
        , { "SeeAlso",     nullptr,        MarkType::kSeeAlso,     Resolvable::kYes,  M_CSST | M_E | M(Method) }
        , { "StdOut",      nullptr,        MarkType::kStdOut,      Resolvable::kNo,   M(Example) }
        , { "Struct",      &fClassMap,     MarkType::kStruct,      Resolvable::kYes,  M(Class) | M(Root) | M_ST }
        , { "Subtopic",    nullptr,        MarkType::kSubtopic,    Resolvable::kYes,  M_CSST }
        , { "Table",       nullptr,        MarkType::kTable,       Resolvable::kYes,  M(Method) | M_ST }
        , { "Template",    nullptr,        MarkType::kTemplate,    Resolvable::kYes,  0 }
        , { "",            nullptr,        MarkType::kText,        Resolvable::kYes,  0 }
        , { "Time",        nullptr,        MarkType::kTime,        Resolvable::kYes,  M(Track) }
        , { "ToDo",        nullptr,        MarkType::kToDo,        Resolvable::kNo,   0 }
        , { "Topic",       nullptr,        MarkType::kTopic,       Resolvable::kYes,  M_CS | M(Root) | M(Topic) }
        , { "Track",       nullptr,        MarkType::kTrack,       Resolvable::kYes,  M_E |M_ST }
        , { "Typedef",     &fTypedefMap,   MarkType::kTypedef,     Resolvable::kYes,  M(Subtopic) | M(Topic) }
        , { "",            nullptr,        MarkType::kUnion,       Resolvable::kYes,  0 }
        , { "Width",       nullptr,        MarkType::kWidth,       Resolvable::kNo,   M(Example) } }
        {
            this->reset();
        }

#undef M_E
#undef M_CSST
#undef M_ST
#undef M_CS
#undef M

    ~BmhParser() override {}

    bool addDefinition(const char* defStart, bool hasEnd, MarkType markType,
            const vector<string>& typeNameBuilder);
    bool childOf(MarkType markType) const;
    string className(MarkType markType);
    bool collectExternals();
    int endHashCount() const;

    RootDefinition* findBmhObject(MarkType markType, const string& typeName) {
        auto map = fMaps[(int) markType].fBmh;
        if (!map) {
            return nullptr;
        }
        return &(*map)[typeName];
    }

    bool findDefinitions();
    MarkType getMarkType(MarkLookup lookup) const;
    bool hasEndToken() const;
    string memberName();
    string methodName();
    const Definition* parentSpace() const;

    bool parseFromFile(const char* path) override {
        INHERITED::parseSetup(path);
        return findDefinitions();
    }

    bool popParentStack(Definition* definition);

    void reset() override {
        INHERITED::resetCommon();
        fRoot = nullptr;
        fMC = '#';
        fInChar = false;
        fInCharCommentString = false;
        fInComment = false;
        fInEnum = false;
        fInString = false;
    }

    bool skipNoName();
    bool skipToDefinitionEnd(MarkType markType);
    vector<string> topicName();
    vector<string> typeName(MarkType markType, bool* expectEnd);
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
    char fMC;  // markup character
    bool fAnonymous;
    bool fCloned;
    bool fInChar;
    bool fInCharCommentString;
    bool fInEnum;
    bool fInComment;
    bool fInString;

private:
    typedef ParserCommon INHERITED;
};

class InterfaceParser : public ParserCommon {
public:
    InterfaceParser() : ParserCommon()
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
        , { &fIEnumMap,     MarkType::kEnum }
        , { &fIEnumMap,     MarkType::kEnumClass }
        , { nullptr,        MarkType::kError }
        , { nullptr,        MarkType::kExample }
        , { nullptr,        MarkType::kExternal }
        , { nullptr,        MarkType::kFile }
        , { nullptr,        MarkType::kFormula }
        , { nullptr,        MarkType::kFunction }
        , { nullptr,        MarkType::kHeight }
        , { nullptr,        MarkType::kImage }
        , { nullptr,        MarkType::kLegend }
        , { nullptr,        MarkType::kLink }
        , { nullptr,        MarkType::kList }
        , { nullptr,        MarkType::kMember }
        , { nullptr,        MarkType::kMethod }
        , { nullptr,        MarkType::kParam }
        , { nullptr,        MarkType::kPlatform }
        , { nullptr,        MarkType::kReturn }
        , { nullptr,        MarkType::kRoot }
        , { nullptr,        MarkType::kRow }
        , { nullptr,        MarkType::kSeeAlso }
        , { nullptr,        MarkType::kStdOut }
        , { &fIStructMap,   MarkType::kStruct }
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
        , { nullptr,        MarkType::kWidth } }
    {
        this->reset();
    }

    ~InterfaceParser() override {}

    void addKeyword(KeyWord keyWord);

    void addPunctuation(Punctuation punctuation) {
        fParent->fTokens.emplace_back(punctuation, fChar, fLineCount, fParent);
    }

    void addWord() {
        fParent->fTokens.emplace_back(fIncludeWord, fChar, 0, fParent);
        fIncludeWord = nullptr;
    }

    bool checkForWord();
    string className() const;
    bool crossCheck(BmhParser& );
    void dumpComment(const Definition& token);
    void dumpTokens();

    IClassDefinition* findClass(const Definition& includeDef, const string& className) {
        unordered_map<string, IClassDefinition>& map = fIClassMap;
        IClassDefinition& markupDef = map[className];
        if (markupDef.fStart) {
            typedef IClassDefinition* IClassDefPtr;
            return INHERITED::reportError<IClassDefPtr>("class already defined");
        }
        markupDef.fStart = includeDef.fStart;
        markupDef.fContentStart = includeDef.fStart;
        markupDef.fName = className;
        markupDef.fContentEnd = includeDef.fContentEnd;
        markupDef.fTerminator = includeDef.fTerminator;
        markupDef.fParent = fParent;
        markupDef.fLineCount = includeDef.fLineCount;
        markupDef.fMarkType = MarkType::kClass;
        markupDef.fKeyWord = KeyWord::kNone;
        markupDef.fType = Definition::Type::kMark;
        return &markupDef;
    }

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
    bool parseChar();
    bool parseComment(const char* start, const char* end, int lineCount, Definition* markupDef);
    bool parseClass(Definition* def);
    bool parseDefine();
    bool parseEnum(Definition* child, Definition* markupDef);

    void parseFromChild(Definition* child);

    bool parseFromFile(const char* path) override {
        INHERITED::parseSetup(path);
        string name(path);
        return parseInclude(name);
    }

    bool parseInclude(const string& name);
    bool parseMethod(Definition* child, Definition* markupDef);
    bool parseObject(Definition* child, Definition* markupDef);
    bool parseObjects(Definition* parent, Definition* markupDef);
    bool parseTemplate();
    bool parseTypedef();
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

    void reset() override {
        INHERITED::resetCommon();
        fInBrace = nullptr;
        fIncludeWord = nullptr;
        fPrev = '\0';
        fInChar = false;
        fInCharCommentString = false;
        fInComment = false;
        fInEnum = false;
        fInFunction = false;
        fInString = false;
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
        string name(typeName.size() > 0 ? typeName : "anon");
        int anonCount = 1;
        do {
            auto iter = m.find(name);
            if (iter == m.end()) {
                return name;
            }
            name = typeName + '_';
            name += to_string(++anonCount);
        } while (true);
        // should never get here
        return string();
    }

    void validate() const;

private:
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
    unordered_map<string, Definition> fIStructMap;
    unordered_map<string, Definition> fITemplateMap;
    unordered_map<string, Definition> fITypedefMap;
    unordered_map<string, Definition> fIUnionMap;
    FILE* fIOut;
    Definition* fInBrace;
    const char* fIncludeWord;
    char fPrev;
    bool fInChar;
    bool fInCharCommentString;
    bool fInComment;
    bool fInEnum;
    bool fInFunction;
    bool fInString;

    typedef ParserCommon INHERITED;
};

class FiddleParser : public ParserCommon {
public:
    FiddleParser(BmhParser* bmh) : ParserCommon()
        , fBmhParser(bmh) {
        this->reset();
    }

    Definition* findExample(const string& name) const;

    bool parseFromFile(const char* path) override {
        INHERITED::parseSetup(path);
        return parseFiddles();
    }

    void reset() override {
        INHERITED::resetCommon();
    }

private:
    bool parseFiddles();

    BmhParser* fBmhParser;  // must be writable; writes example hash

    typedef ParserCommon INHERITED;
};

class MdOut : public ParserCommon {
public:
    MdOut(const BmhParser& bmh) : ParserCommon()
        , fBmhParser(bmh) {
        this->reset();
    }

    bool buildReferences(const char* path);
private:
    enum class TableState {
        kNone,
        kRow,
        kColumn,
    };

    string addReferences(const char* start, const char* end);
    bool buildRefFromFile(const char* fileName);
    void childrenOut(const Definition* def, const char* contentStart);
    const Definition* isDefined(const TextParser& parser, const string& ref) const;
    void lf(int count);
    void lfAlways(int count);
    string linkName(const Definition* ) const;
    string linkRef(const string& leadingSpaces, const Definition*, const string& ref) const;
    void markTypeOut(Definition* );
    void mdHeaderOut(int depth);
    void nl();
    bool parseFromFile(const char* path) override {
        return true;
    }

    void reset() override {
        INHERITED::resetCommon();
        fMdOut = nullptr;
        fTableState = TableState::kNone;
    }

    bool resolvable(MarkType markType) { 
        return BmhParser::Resolvable::kYes == fBmhParser.fMaps[(int) markType].fResolve;
    }

    void resolveOut(const char* start, const char* end);

    const BmhParser& fBmhParser;
    FILE* fMdOut;
    RootDefinition* fRoot;
    TableState fTableState;
    int fLinefeeds;

    typedef ParserCommon INHERITED;
};


// some methods don't cannot be trivially parsed; look for class-name / ~ / operator
class MethodParser : public TextParser {
public:
    MethodParser(const string& className, const char* start, const char* end)
        : TextParser(start, end)
        , fClassName(className) {
    }

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
                const char* ptr = this->anyOf(" (");
                if (ptr && '(' ==  *ptr) {
                    this->skipToEndBracket(')');
                    SkAssertResult(')' == this->next());
                    return;
                }
            }
        }
        this->skipToNonAlphaNum();
    }

private:
    string fClassName;
    typedef TextParser INHERITED;
};

#endif
