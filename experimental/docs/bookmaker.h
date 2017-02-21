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
    kCode,
    kConst,
    kDefine,
    kDeprecated,
    kDescription,
    kDoxygen,
    kEnum,
    kExample,
    kFile,
    kFormula,
    kImage,
    kImport,
    kList,
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

static inline void trim(string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
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

    bool skipToLineStart() {
        if (!skipLine()) {
            return false;
        }
        return this->skipWhiteSpace();
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

    Definition()
        : Definition(nullptr, nullptr, nullptr) {
        fType = Type::kNone;
    }

    Definition(const char* start, const char* end, Definition* parent) 
        : fStart(start)
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

    void dump() const;

    string extractText() const {
        string result;
        TextParser parser(fStart, fContentEnd);
        int childIndex = 0;
        char mc = '#';
        do {
            if (!parser.skipWhiteSpace()) {
                break;
            }
            if (parser.next() == mc) {
                if (parser.next() == mc) {
                    if (parser.next() == mc) {
                        mc = parser.next();
                    }
                } else {
                    // fixme : more work to do if # style comment is in text
                    if (' ' < parser.fChar[-1]) {
                        parser.skipTo(fChildren[childIndex]->fTerminator);
                        if (mc == parser.fChar[0] && mc == parser.fChar[1]) {
                            parser.next();
                            parser.next();
                        }
                        childIndex++;
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
            trim(fragment);
            if (result.length()) {
                result += ' ';
            }
            result += fragment;
            parser.skipTo(end);
        } while (parser.fChar < parser.fEnd);
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

    void setParentIndex() {
        fParentIndex = fParent ? (int) fParent->fTokens.size() : -1;
    }

    string fText;  // if text is constructed instead of in a file, it's put here
    const char* fStart;  // .. in original text file, or the start of fText
    string fName;
    const char* fContentEnd;  // the end of the contained text
    const char* fTerminator;  // the end of the markup, normally ##\n or \n
    Definition* fParent;
    list<Definition> fTokens;
    vector<Definition*> fChildren;
    int fParentIndex;
    MarkType fMarkType;
    KeyWord fKeyWord;
    Bracket fBracket;
    Punctuation fPunctuation;
    Type fType;
};

struct ClassDefinition : public Definition {
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

struct Parser : public TextParser {
    Parser()
        : fMaps { 
          { nullptr,        nullptr,        MarkType::kNone }
        , { nullptr,        nullptr,        MarkType::kAnchor }
        , { nullptr,        nullptr,        MarkType::kBug }
        , { &fClassMap,     nullptr,        MarkType::kClass }
        , { nullptr,        nullptr,        MarkType::kColumn }
        , { nullptr,        nullptr,        MarkType::kComment }
        , { nullptr,        nullptr,        MarkType::kCode }
        , { nullptr,        nullptr,        MarkType::kConst }
        , { nullptr,        &fIDefineMap,   MarkType::kDefine }
        , { nullptr,        nullptr,        MarkType::kDeprecated }
        , { nullptr,        nullptr,        MarkType::kDescription }
        , { nullptr,        nullptr,        MarkType::kDoxygen }
        , { &fEnumMap,      &fIEnumMap,     MarkType::kEnum }
        , { nullptr,        nullptr,        MarkType::kExample }
        , { nullptr,        nullptr,        MarkType::kFile }
        , { nullptr,        nullptr,        MarkType::kFormula }
        , { nullptr,        nullptr,        MarkType::kImage }
        , { nullptr,        nullptr,        MarkType::kImport }
        , { nullptr,        nullptr,        MarkType::kList }
        , { &fMethodMap,    nullptr,        MarkType::kMethod }
        , { nullptr,        nullptr,        MarkType::kParam }
        , { nullptr,        nullptr,        MarkType::kPlatform }
        , { nullptr,        nullptr,        MarkType::kReturn }
        , { nullptr,        nullptr,        MarkType::kRow }
        , { nullptr,        nullptr,        MarkType::kSeeAlso }
        , { nullptr,        nullptr,        MarkType::kStdOut }
        , { nullptr,        &fIStructMap,   MarkType::kStruct }
        , { nullptr,        nullptr,        MarkType::kSubtopic }
        , { nullptr,        nullptr,        MarkType::kTable }
        , { nullptr,        &fITemplateMap, MarkType::kTemplate }
        , { nullptr,        nullptr,        MarkType::kText }
        , { nullptr,        nullptr,        MarkType::kTime }
        , { nullptr,        nullptr,        MarkType::kToDo }
        , { nullptr,        nullptr,        MarkType::kTopic }
        , { nullptr,        nullptr,        MarkType::kTrack }
        , { nullptr,        &fITypedefMap,  MarkType::kTypedef }
        , { nullptr,        &fIUnionMap,    MarkType::kUnion }
        , { nullptr,        nullptr,        MarkType::kWidth } }

        , fParent(nullptr)
        , fFileName(nullptr)
        , fParseType(ParseType::kUndefined) {
        this->reset();
    }

    enum class MarkLookup {
        kRequire,
        kAllowUnknown,
    };

    enum class ParseType {
        kUndefined,
        kBmh,
        kInclude,
    };

    void addDefinition(MarkType markType) {
        fParent->fTokens.emplace_back(markType, fChar, fParent);
        Definition* def = &fParent->fTokens.back();
        this->addDefinition(def);
    }

    void addDefinition(Definition* def) {
        fParent->fChildren.push_back(def);
        fParent = def;
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
    bool childOf(MarkType markType) const;
    string className(MarkType markType);

    const char* contains(const char* match) const {
        size_t len = strlen(match);
        const char* end = this->lineEnd() - len;
        const char* test = fChar;
        while (test < end) {
             if (0 == strncmp(test, match, len)) {
                 return test;
             }
             ++test;
        }
        return nullptr;
    }

    int endHashCount() const;

    bool endsWith(const char* str) const {
        ptrdiff_t lineLen = this->lineLength(); 
        const char* check = fLine + lineLen;
        while (check > fLine && ' ' >= *--check)
            ;
        size_t len = strlen(str);
        // check points to char before end of line
        check -= len - 1;  // check points to compare start
        if (check < fLine) {
            return nullptr;
        }
        return 0 == strncmp(str, check, len);
    }

    bool eof() const { return fChar >= fEnd; }
    string filePath();

    Definition* findBmhObject(MarkType markType, const string& typeName) {
        unordered_map<string, Definition>* map = fMaps[(int) markType].fBmh;
        if (!map) {
            return nullptr;
        }
        return &(*map)[typeName];
    }

    ClassDefinition* findClass(const Definition& includeDef, const string& className) {
        unordered_map<string, ClassDefinition>& map = fIClassMap;
        ClassDefinition& markupDef = map[className];
        if (markupDef.fStart) {
            (void) reportError(includeDef, "class already defined");
            return nullptr;
        }
        markupDef.fStart = includeDef.fStart;
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
    bool findDefinitions();
    static KeyWord FindKey(const char* start, const char* end);

    Definition* findIncludeObject(const Definition& includeDef, MarkType markType,
            const string& typeName) {
        unordered_map<string, Definition>* map = fMaps[(int) markType].fInclude;
        if (!map) {
            (void) reportError(includeDef, "invalid mark type");
            return nullptr;
        }
        string name = this->uniqueName(*map, typeName);
        Definition& markupDef = (*map)[name];
        if (markupDef.fStart) {
            (void) reportError(includeDef, "definition already defined");
            return nullptr;
        }
        markupDef.fStart = includeDef.fStart;
        markupDef.fName = name;
        markupDef.fContentEnd = includeDef.fContentEnd;
        markupDef.fTerminator = includeDef.fTerminator;
        markupDef.fParent = fParent;
        markupDef.fMarkType = markType;
        markupDef.fKeyWord = includeDef.fKeyWord;
        markupDef.fType = Definition::Type::kMark;
        return &markupDef;
    }

    MarkType getMarkType(MarkLookup lookup) const;
    bool hasEndToken() const;
    string methodName();

    string nameBefore(const char* end) {
        while (' ' >= *--end)
            ;
        const char* last = end;
        while (' ' < *--end)
            ;
        SkASSERT(end > fStart);
        return string(end + 1, last - end);
    }

    bool parseChar();
    bool parseComment(const Definition& includeDef, Definition* markupDef);
    bool parseClass(Definition* def);
    bool parseDefine();
    bool parseEnum(Definition* child, Definition* markupDef);
    bool parseFile(const char* fileOrPath);
    void parseFromChild(Definition* child);
    bool parseFromFile(const char* path);
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

    void popObject() {
        fParent->fContentEnd = fParent->fTerminator = fChar;
        fParent = fParent->fParent;
    }

    bool popParentStack(Definition* definition);

    void pushBracket(Bracket bracket) {
        this->setBracketShortCuts(bracket);
        fParent->fTokens.emplace_back(bracket, fChar, fParent);
        Definition* container = &fParent->fTokens.back();
        this->addDefinition(container);
    }

    void reportError(const char* errorStr) const;
    bool reportError(const Definition& , const char* ) const;

    template <typename T> T reportError(const char* errorStr) const {
        this->reportError(errorStr);
	    return T();
    }

    void reset() {
        fLine = fChar = fStart;
        fLineCount = 0;
        fInChar = false;
        fInComment = false;
        fInString = false;
        fInCharCommentString = false;
        fInEnum = false;
        fPrev = '\0';
        fIncludeWord = nullptr;
        fInBrace = nullptr;
		fMC = '#';
    }

    void setBracketShortCuts(Bracket bracket) {
        fInComment = Bracket::kSlashSlash == bracket || Bracket::kSlashStar == bracket;
        fInString = Bracket::kString == bracket;
        fInChar = Bracket::kChar == bracket;
        fInCharCommentString = fInChar || fInComment || fInString;
    }

    void setAsParent(Definition* definition) {
		if (fParent) {
			fParent->fChildren.push_back(definition);
			definition->fParent = fParent;
		}
		fParent = definition;
    }

    void setParseType(ParseType parseType) {
        fParseType = parseType;
    }

    bool skipToDefinitionEnd(MarkType markType);

    Bracket topBracket() const {
        Definition* parent = fParent;
        while (parent && Definition::Type::kBracket != parent->fType) {
            parent = parent->fParent;
        }
        return parent ? parent->fBracket : Bracket::kNone;
    }

    vector<string> topicName();
    vector<string> typeName(MarkType markType);

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
    static void ValidateKeyWords();

    string word(const string& prefix);

    const char* wordBefore(const char* end) const {
        SkASSERT(' ' == end[-1]);
        SkASSERT(' ' < fChar[0]);  // fChar whitespace has already been skipped 
        SkDEBUGCODE(const char* strchrOut);
        SkASSERT((strchrOut = strchr(fChar, ' ')));
        SkASSERT(strchrOut < end); 
        while (' ' == *--end)  // skip white space after word
            ;
        while (' ' < *--end) // iterate until space before word is found
            ;
        SkASSERT(fChar < end);
        ++end;
        return end;
    }

    struct DefinitionMap {
        unordered_map<string, Definition>* fBmh;
        unordered_map<string, Definition>* fInclude;
        MarkType fMarkType;
    };
    
    DefinitionMap fMaps[Last_MarkType + 1];

    unordered_map<string, sk_sp<SkData>> fRawData;
    unordered_map<string, Definition> fClassMap;
    unordered_map<string, Definition> fEnumMap;
    unordered_map<string, Definition> fMethodMap;
    forward_list<Definition> fTopics;
    unordered_map<string, Definition*> fTopicMap;
    unordered_map<string, Definition> fIncludeMap;
    forward_list<Definition> fMarkup;
    unordered_map<string, ClassDefinition> fIClassMap;
    unordered_map<string, Definition> fIDefineMap;
    unordered_map<string, Definition> fIEnumMap;
    unordered_map<string, Definition> fIStructMap;
    unordered_map<string, Definition> fITemplateMap;
    unordered_map<string, Definition> fITypedefMap;
    unordered_map<string, Definition> fIUnionMap;
    vector<Reference> fReferences;
    Definition* fParent;
    Definition* fInBrace;
    const char* fFileName;
    const char* fIncludeWord;
    char fMC;  // markup character
    ParseType fParseType;
    char fPrev;
    bool fInComment;
    bool fInString;
    bool fInChar;
    bool fInCharCommentString;
    bool fInEnum;
};

#endif
