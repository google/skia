/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef bookmaker_DEFINED
#define bookmaker_DEFINED

#include "SkData.h"

#include <cmath>
#include <cctype>
#include <list>
#include <map>
#include <vector>

using std::list;
using std::map;
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
    kClass,
    kColumn,
    kComment,
    kConst,
    kDefine,
    kDoxygen,
    kEnum,
    kExample,
    kMethod,
    kParameter,
    kRow,
    kStdOut,
    kStruct,
    kTable,
    kTemplate,
    kText,
    kToDo,
    kTypedef,
    kUnion,
};

enum {
    Last_MarkType = MarkType::kUnion,
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

struct Definition {
    enum Type {
        kNone,
        kWord,
        kMark,
        kKeyWord,
        kBracket,
    };

    Definition()
        : fStart(nullptr)
        , fEnd(nullptr)
        , fParent(nullptr)
        , fParentIndex(0)
        , fMarkType(MarkType::kNone)
        , fKeyWord(KeyWord::kNone)
        , fBracket(Bracket::kNone)
        , fType(Type::kNone) {
    }

    Definition(const char* start, const char* end, Definition* parent) 
        : fStart(start)
        , fEnd(end)
        , fParent(parent)
        , fParentIndex(0)
        , fMarkType(MarkType::kNone)
        , fKeyWord(KeyWord::kNone)
        , fBracket(Bracket::kNone)
        , fType(Type::kWord) {
    }

    Definition(MarkType markType, const char* start, Definition* parent) 
        : fStart(start)
        , fEnd(nullptr)
        , fParent(parent)
        , fMarkType(markType)
        , fKeyWord(KeyWord::kNone)
        , fBracket(Bracket::kNone)
        , fType(Type::kMark) {
        this->setParentIndex();
    }

    Definition(MarkType markType, const string& text, Definition* parent) 
        : fText(text)
        , fStart(fText.c_str())
        , fEnd(nullptr)
        , fParent(parent)
        , fMarkType(markType)
        , fKeyWord(KeyWord::kNone)
        , fBracket(Bracket::kNone)
        , fType(Type::kMark) {
        this->setParentIndex();
    }

    Definition(Bracket bracket, const char* start, Definition* parent) 
        : fStart(start)
        , fEnd(nullptr)
        , fParent(parent)
        , fMarkType(MarkType::kNone)
        , fKeyWord(KeyWord::kNone)
        , fBracket(bracket)
        , fType(Type::kBracket) {
        this->setParentIndex();
    }

    Definition(KeyWord keyWord, const char* start, const char* end, Definition* parent) 
        : fStart(start)
        , fEnd(end)
        , fParent(parent)
        , fMarkType(MarkType::kNone)
        , fKeyWord(keyWord)
        , fBracket(Bracket::kNone)
        , fType(Type::kKeyWord) {
        this->setParentIndex();
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
    const char* fEnd;
    Definition* fParent;
    list<Definition> fTokens;
    vector<Definition*> fChildren;
    int fParentIndex;
    MarkType fMarkType;
    KeyWord fKeyWord;
    Bracket fBracket;
    Type fType;
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

struct Parser {
    Parser()
        : fMaps { 
          { nullptr,        nullptr,        MarkType::kNone }
        , { &fClassMap,     &fIClassMap,    MarkType::kClass }
        , { nullptr,        nullptr,        MarkType::kColumn }
        , { nullptr,        nullptr,        MarkType::kComment }
        , { nullptr,        nullptr,        MarkType::kConst }
        , { nullptr,        &fIDefineMap,   MarkType::kDefine }
        , { nullptr,        nullptr,        MarkType::kDoxygen }
        , { nullptr,        &fIEnumMap,     MarkType::kEnum }
        , { &fExampleMap,   nullptr,        MarkType::kExample }
        , { &fMethodMap,    nullptr,        MarkType::kMethod }
        , { nullptr,        nullptr,        MarkType::kParameter }
        , { nullptr,        nullptr,        MarkType::kRow }
        , { &fStdOutMap,    nullptr,        MarkType::kStdOut }
        , { nullptr,        &fIStructMap,   MarkType::kStruct }
        , { &fTableMap,     nullptr,        MarkType::kTable }
        , { nullptr,        &fITemplateMap, MarkType::kTemplate }
        , { nullptr,        nullptr,        MarkType::kText }
        , { nullptr,        nullptr,        MarkType::kToDo }
        , { nullptr,        &fITypedefMap,   MarkType::kTypedef }
        , { nullptr,        &fIUnionMap,     MarkType::kUnion } }
        , fParent(nullptr)
        , fStart(nullptr)
        , fEnd(nullptr)
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

    void addWord() {
        fParent->fTokens.emplace_back(fIncludeWord, fChar, fParent);
        fIncludeWord = nullptr;
    }

    bool checkForWord();

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

    Definition* findBmhObject(MarkType markType, const string& typeName) {
        map<string, Definition>* map = fMaps[(int) markType].fBmh;
        if (!map) {
            return nullptr;
        }
        return &(*map)[typeName];
    }

    bool findComments(const Definition& includeDef, Definition* markupDef);
    bool findDefinitions();
    static KeyWord FindKey(const char* start, const char* end);

    Definition* findIncludeObject(const Definition& includeDef, MarkType markType,
            const string& typeName) {
        map<string, Definition>* map = fMaps[(int) markType].fInclude;
        if (!map) {
            (void) reportError(includeDef, "invalid mark type");
            return nullptr;
        }
        Definition* markupDef = &(*map)[typeName];
        if (markupDef->fStart) {
            (void) reportError(includeDef, "definition already defined");
            return nullptr;
        }
        if (markupDef->fName.size()) {
            (void) reportError(includeDef, "definition already named");
            return nullptr;
        }
        markupDef->fStart = includeDef.fStart;
        markupDef->fName = typeName;
        markupDef->fEnd = includeDef.fEnd;
        markupDef->fParent = fParent;
        markupDef->fMarkType = markType;
        markupDef->fKeyWord = includeDef.fKeyWord;
        markupDef->fType = Definition::Type::kMark;
        return markupDef;
    }

    MarkType getMarkType(MarkLookup lookup) const;
    bool hasEndToken() const;

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

    string nameBefore(const char* end) {
        while (' ' >= *--end)
            ;
        const char* last = end;
        while (' ' < *--end)
            ;
        SkASSERT(end > fStart);
        return string(end + 1, last - end);
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

    bool parseChar();
    bool parseComment(const Definition& includeDef, Definition* markupDef);
    bool parseClass(Definition* def);
    bool parseDefine();
    bool parseEnum();
    bool parseFile(const char* fileOrPath);
    bool parseFromFile(const char* path);
    bool parseInclude(const string& name);
    bool parseObjects(Definition* parent);
    bool parseTemplate();
    bool parseTypedef();
    bool parseUnion();
    char peek() const { SkASSERT(fChar < fEnd); return *fChar; }

    void popBracket() {
        SkASSERT(Definition::Type::kBracket == fParent->fType);
        this->popObject();
        Bracket bracket = this->topBracket();
        this->setBracketShortCuts(bracket);
    }

    void popObject() {
        fParent->fEnd = fChar;
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

    bool skipToLineStart() {
        if (!skipLine()) {
            return false;
        }
        return this->skipWhiteSpace();
    }

    bool skipWord(const char* word) {
        if (!this->skipWhiteSpace()) {
            return false;
        }
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
            if (!this->skipWhiteSpace()) {
                return false;
            }
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

    bool startsWith(const char* str) {
        size_t len = strlen(str);
        ptrdiff_t lineLen = this->lineLength(); 
        return len <= (size_t) lineLen && 0 == strncmp(str, fChar, len);
    }

    Bracket topBracket() const {
        Definition* parent = fParent;
        while (parent && Definition::Type::kBracket != parent->fType) {
            parent = parent->fParent;
        }
        return parent ? parent->fBracket : Bracket::kNone;
    }

    string typeName(bool hasEnd);
    void validate() const;
    static void ValidateKeyWords();

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
        map<string, Definition>* fBmh;
        map<string, Definition>* fInclude;
        MarkType fMarkType;
    };
    
    DefinitionMap fMaps[Last_MarkType + 1];

    map<string, sk_sp<SkData>> fRawData;
    map<string, Definition> fClassMap;
    map<string, Definition> fExampleMap;
    map<string, Definition> fMethodMap;
    map<string, Definition> fStdOutMap;
    map<string, Definition> fTableMap;
    map<string, Definition> fIncludeMap;
    map<string, Definition> fIClassMap;
    map<string, Definition> fIDefineMap;
    map<string, Definition> fIEnumMap;
    map<string, Definition> fIStructMap;
    map<string, Definition> fITemplateMap;
    map<string, Definition> fITypedefMap;
    map<string, Definition> fIUnionMap;
    vector<Reference> fReferences;
    Definition* fParent;
    Definition* fInBrace;
    const char* fStart;
    const char* fLine;
    const char* fChar;
    const char* fEnd;
    const char* fFileName;
    const char* fIncludeWord;
    size_t fLineCount;
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
