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
    kBug,
    kClass,
    kColumn,
    kComment,
    kConst,
    kDefine,
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
    kImage,
    kImport,
    kLegend,
    kList,
    kMember,
    kMethod,
    kParam,
    kPlatform,
    kReturn,
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
    Last_MarkType = MarkType::kWidth,
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
    kPound
};

enum class Punctuation {  // catch-all for misc symbols tracked in C
    kNone,
    kAsterisk,  // for pointer-to
    kSemicolon,  // e.g., to delinate xxx() const ; const int* yyy()
    kLeftBrace,
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

static inline void trim_start(string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
}

static inline void trim_start_end(string& s) {
    trim_start(s);
    trim_end(s);
}

struct TextParser {
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

    bool sentenceEnd(const char* check) {
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

    bool skipToLineStart() {
        if (!skipLine()) {
            return false;
        }
        return this->skipWhiteSpace();
    }

    void skipToNonAlphaNum() {
        while (fChar < fEnd
                && (isalnum(fChar[0]) || '_' == fChar[0] || '-' == fChar[0])) {
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
        if (len == end - fChar && !strncmp(word, fChar, len)) {
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
        const char* check = fChar;
        size_t count = len - matchLen;
        for (size_t index = 0; index <= count; index++) {
            if (fChar[index] == match[0] && 0 == strncmp(&fChar[index], match, matchLen)) {
                return &fChar[index];
            }
        }
        return nullptr;
    }

    const char* trimmedBracketEnd(const char bracket) const {
        const char* result = this->lineEnd();
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

struct Definition {
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

    Definition()
        : Definition(nullptr, nullptr, nullptr) {
        fType = Type::kNone;
    }

    Definition(const char* start, const char* end, Definition* parent) 
        : fStart(start)
        , fContentStart(start)
        , fContentEnd(end)
        , fTerminator(nullptr)
        , fParent(parent)
        , fParentIndex(0)
        , fMarkType(MarkType::kNone)
        , fKeyWord(KeyWord::kNone)
        , fBracket(Bracket::kNone)
        , fPunctuation(Punctuation::kNone)
        , fType(Type::kWord) {
        this->setParentIndex();
    }

    Definition(MarkType markType, const char* start, Definition* parent) 
        : Definition(markType, start, nullptr, parent) {
    }

    Definition(MarkType markType, const char* start, const char* end, Definition* parent) 
        : Definition(start, end, parent) {
        fMarkType = markType;
        fType = Type::kMark; 
    }

    Definition(MarkType markType, const string& text, Definition* parent) 
        : Definition(markType, text.c_str(), parent) {
        fText = text;
    }

    Definition(Bracket bracket, const char* start, Definition* parent) 
        : Definition(start, nullptr, parent) {
        fBracket = bracket;
        fType = Type::kBracket;
    }

    Definition(KeyWord keyWord, const char* start, const char* end, Definition* parent) 
        : Definition(start, end, parent) {
        fKeyWord = keyWord;
        fType = Type::kKeyWord;
    }

    Definition(Punctuation punctuation, const char* start, Definition* parent) 
        : Definition(start, nullptr, parent) {
        fPunctuation = punctuation;
        fType = Type::kPunctuation;
    }

    bool boilerplateIfDef(Definition* root) {
        const Definition& label = fTokens.front();
        if (Type::kWord != label.fType) {
            return false;
        }
        const char* s = label.fStart;
        const char* e = strchr(s, '_');
        if (!e) {
            return false;
        }
        string name(s, e - s);
        const char* inName = strstr(root->fName.c_str(), name.c_str());
        if (!inName) {
            return false;
        }
        if ('/' != inName[-1]) {
            return false;
        }
        if (strcmp(inName + name.size(), ".h")) {
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

    // called to skip function bodies defined inline
    // also need to detect auto-initializers ?
    bool isFunction() const {
        if (fChildren.size() < 2) {
            return false;
        }
        Definition* lastChild = *std::prev(fChildren.end(), 2);
        // TODO : also need to check if bracket is closed
        return Type::kBracket == lastChild->fType
                && Bracket::kParen == lastChild->fBracket;
    }

    string printableName() const {
        string result(fName);
        std::replace(result.begin(), result.end(), '_', ' ');
        return result;
    }

    void setParentIndex() {
        fParentIndex = fParent ? (int) fParent->fTokens.size() : -1;
    }

    string fText;  // if text is constructed instead of in a file, it's put here
    const char* fStart;  // .. in original text file, or the start of fText
    const char* fContentStart;  // start past optional markup name
    string fName;
    const char* fContentEnd;  // the end of the contained text
    const char* fTerminator;  // the end of the markup, normally ##\n or \n
    Definition* fParent;
    list<Definition> fTokens;
    vector<Definition*> fChildren;
    string fHash;  // generated by fiddle
    int fParentIndex;
    MarkType fMarkType;
    KeyWord fKeyWord;
    Bracket fBracket;
    Punctuation fPunctuation;
    Type fType;
};

struct RootDefinition : public Definition {
    unordered_map<string, Definition> fChildDefinitions;
};

struct IClassDefinition : public Definition {
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

    void addDefinition(Definition* def) {
        fParent->fChildren.push_back(def);
        fParent = def;
    }

    bool parseFile(const char* file, const char* suffix);

    virtual bool parseFromFile(const char* path) = 0;

    void parseSetup(const char* path) {
        this->reset();
	    sk_sp<SkData> data = SkData::MakeFromFileName(path);
        string name(path);
	    fRawData[name] = data;
	    const char* rawText = (const char*) data->data();
        fStart = rawText;
        fLine = rawText;
        fChar = rawText;
        fEnd = rawText + data->size();
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

    BmhParser() : ParserCommon()
        , fMaps { 
// names without formal definitions (e.g. Column) aren't included
// fill in other names once they're actually used
          { "",            nullptr,        MarkType::kNone }
        , { "A",           nullptr,        MarkType::kAnchor }
        , { "Bug",         nullptr,        MarkType::kBug }
        , { "Class",       &fClassMap,     MarkType::kClass }
        , { "",            nullptr,        MarkType::kColumn }
        , { "",            nullptr,        MarkType::kComment }
        , { "Const",       &fConstMap,     MarkType::kConst }
        , { "",            nullptr,        MarkType::kDefine }
        , { "Deprecated",  nullptr,        MarkType::kDeprecated }
        , { "Description", nullptr,        MarkType::kDescription }
        , { "Doxygen",     nullptr,        MarkType::kDoxygen }
        , { "Enum",        &fEnumMap,      MarkType::kEnum }
        , { "EnumClass",   &fEnumMap,      MarkType::kEnumClass }
        , { "Error",       nullptr,        MarkType::kError }
        , { "Example",     nullptr,        MarkType::kExample }
        , { "External",    nullptr,        MarkType::kExternal }
        , { "File",        nullptr,        MarkType::kFile }
        , { "Formula",     nullptr,        MarkType::kFormula }
        , { "Function",    nullptr,        MarkType::kFunction }
        , { "Image",       nullptr,        MarkType::kImage }
        , { "Import",      nullptr,        MarkType::kImport }
        , { "Legend",      nullptr,        MarkType::kLegend }
        , { "List",        nullptr,        MarkType::kList }
        , { "Member",      nullptr,        MarkType::kMember }
        , { "Method",      &fMethodMap,    MarkType::kMethod }
        , { "Param",       nullptr,        MarkType::kParam }
        , { "Platform",    nullptr,        MarkType::kPlatform }
        , { "Return",      nullptr,        MarkType::kReturn }
        , { "",            nullptr,        MarkType::kRow }
        , { "SeeAlso",     nullptr,        MarkType::kSeeAlso }
        , { "StdOut",      nullptr,        MarkType::kStdOut }
        , { "Struct",      &fClassMap,     MarkType::kStruct }
        , { "Subtopic",    nullptr,        MarkType::kSubtopic }
        , { "Table",       nullptr,        MarkType::kTable }
        , { "Template",    nullptr,        MarkType::kTemplate }
        , { "",            nullptr,        MarkType::kText }
        , { "Time",        nullptr,        MarkType::kTime }
        , { "ToDo",        nullptr,        MarkType::kToDo }
        , { "Topic",       nullptr,        MarkType::kTopic }
        , { "Track",       nullptr,        MarkType::kTrack }
        , { "Typedef",     &fTypedefMap,   MarkType::kTypedef }
        , { "",            nullptr,        MarkType::kUnion }
        , { "Width",       nullptr,        MarkType::kWidth } }
        {
            this->reset();
        }

    string addReferences(const char* start, const char* end);
    void buildReferences();
    bool childOf(MarkType markType) const;
    void childrenOut(const Definition* def, const char* contentStart);
    string className(MarkType markType);
    bool collectExternals();
    int endHashCount() const;

    RootDefinition* findBmhObject(MarkType markType, const string& typeName) {
        unordered_map<string, RootDefinition>* map = fMaps[(int) markType].fBmh;
        if (!map) {
            return nullptr;
        }
        return &(*map)[typeName];
    }

    bool findDefinitions();
    MarkType getMarkType(MarkLookup lookup) const;
    bool hasEndToken() const;
    const Definition* isDefined(const string& ref) const;
    void markTypeOut(const Definition* );
    void mdHeaderOut();
    string memberName();
    string methodName(bool* isConstructor);
    string optionalName(MarkType );
    const Definition* parentClass() const;

    bool parseFromFile(const char* path) override {
        INHERITED::parseSetup(path);
		return findDefinitions();
    }

    bool popParentStack(Definition* definition);

    void reset() override {
        INHERITED::resetCommon();
        fInChar = false;
        fInComment = false;
        fInString = false;
        fInCharCommentString = false;
        fInEnum = false;
        fRoot = nullptr;
		fMC = '#';
    }

    void resolveOut(const char* start, const char* end);

    bool skipToDefinitionEnd(MarkType markType);
    vector<string> topicName();
    vector<string> typeName(MarkType markType, bool* isConstructor);
    string uniqueName(MarkType markType);
    void validate() const;
    string word(const string& prefix);

public:
    struct DefinitionMap {
        const char* fName;
        unordered_map<string, RootDefinition>* fBmh;
        MarkType fMarkType;
    };
    
    DefinitionMap fMaps[Last_MarkType + 1];
    forward_list<Definition> fTopics;
    forward_list<Definition> fMarkup;
    vector<string> fExternals;
    unordered_map<string, RootDefinition> fClassMap;
    unordered_map<string, RootDefinition> fConstMap;
    unordered_map<string, RootDefinition> fEnumMap;
    unordered_map<string, RootDefinition> fMethodMap;
    unordered_map<string, RootDefinition> fTypedefMap;
    unordered_map<string, Definition*> fTopicMap;
    int fMdHeaderDepth;
    char fMC;  // markup character
    RootDefinition* fRoot;
    bool fInComment;
    bool fInString;
    bool fInChar;
    bool fInCharCommentString;
    bool fInEnum;

    typedef ParserCommon INHERITED;
};

class InterfaceParser : public ParserCommon {
public:
    InterfaceParser() : ParserCommon()
        , fMaps {
          { nullptr,        MarkType::kNone }
        , { nullptr,        MarkType::kAnchor }
        , { nullptr,        MarkType::kBug }
        , { nullptr,        MarkType::kClass }
        , { nullptr,        MarkType::kColumn }
        , { nullptr,        MarkType::kComment }
        , { nullptr,        MarkType::kConst }
        , { &fIDefineMap,   MarkType::kDefine }
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
        , { nullptr,        MarkType::kImage }
        , { nullptr,        MarkType::kImport }
        , { nullptr,        MarkType::kLegend }
        , { nullptr,        MarkType::kList }
        , { nullptr,        MarkType::kMember }
        , { nullptr,        MarkType::kMethod }
        , { nullptr,        MarkType::kParam }
        , { nullptr,        MarkType::kPlatform }
        , { nullptr,        MarkType::kReturn }
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

    void addKeyword(KeyWord keyWord);

    void addPunctuation(Punctuation punctuation) {
        fParent->fTokens.emplace_back(punctuation, fChar, fParent);
    }

    void addWord() {
        fParent->fTokens.emplace_back(fIncludeWord, fChar, fParent);
        fIncludeWord = nullptr;
    }

    bool checkForWord();

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
        markupDef.fMarkType = markType;
        markupDef.fKeyWord = includeDef.fKeyWord;
        markupDef.fType = Definition::Type::kMark;
        return &markupDef;
    }

    static KeyWord FindKey(const char* start, const char* end);
    bool parseChar();
    bool parseComment(const Definition& includeDef, Definition* markupDef);
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
    bool parseObjects(Definition* parent);
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
        fParent->fTokens.emplace_back(bracket, fChar, fParent);
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
            unordered_map<string, T>::const_iterator iter = m.find(name);
            if (iter == m.end()) {
                return name;
            }
            name = typeName + '_';
            name += std::to_string(++anonCount);
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
    Definition* fInBrace;
    const char* fIncludeWord;
    char fPrev;
    bool fInChar;
    bool fInCharCommentString;
    bool fInComment;
    bool fInEnum;
    bool fInString;

    typedef ParserCommon INHERITED;
};

class FiddleParser : public ParserCommon {
public:
    FiddleParser(BmhParser& bmh) : ParserCommon()
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

    BmhParser& fBmhParser;

    typedef ParserCommon INHERITED;
};

#endif
