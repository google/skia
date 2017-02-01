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
#include <map>
#include <stack>
#include <vector>

enum class KeyWord {
    kNone,
    kBool,
    kChar,
    kClass,
    kConst,
    kConstExpr,
    kDefine,
    kDouble,
    kElIf,
    kElse,
    kEndif,
    kEnum,
    kFloat,
    kFriend,
    kIf,
    kIfdef,
    kIfndef,
    kInclude,
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
    kPoundIf
};

struct Definition {
    enum Type {
        kNone,
        kWord,
        kMark,
        kKeyword,
        kBracket,
    };

    Definition()
        : fStart(nullptr)
        , fEnd(nullptr)
        , fParent(nullptr)
        , fMarkType(MarkType::kNone)
        , fKeyWord(KeyWord::kNone)
        , fBracket(Bracket::kNone)
        , fType(Type::kNone) {
    }

    Definition(const char* start, const char* end, Definition* parent) 
        : fStart(start)
        , fEnd(end)
        , fParent(parent)
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
    }

    Definition(Bracket bracket, const char* start, Definition* parent) 
        : fStart(start)
        , fEnd(nullptr)
        , fParent(parent)
        , fMarkType(MarkType::kNone)
        , fKeyWord(KeyWord::kNone)
        , fBracket(bracket)
        , fType(Type::kBracket) {
    }

    Definition(KeyWord keyWord, const char* start, const char* end, Definition* parent) 
        : fStart(start)
        , fEnd(end)
        , fParent(parent)
        , fMarkType(MarkType::kNone)
        , fKeyWord(KeyWord::kNone)
        , fBracket(Bracket::kNone)
        , fType(Type::kKeyword) {
    }

    const char* fStart;  // .. in original text file
    std::string fName;
    const char* fEnd;
    Definition* fParent;
    std::vector<Definition*> fChildren;
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
        fDefinitions.emplace_back(markType, fChar, fParent);
        Definition* def = &fDefinitions.back();
        fParent->fChildren.push_back(def);
        fParent = def;
    }

    void checkForWord() {
        if (fIncludeWord) {
            KeyWord keyWord = FindKey(fIncludeWord, fChar);
            if (KeyWord::kNone != keyWord) {
                fDefinitions.emplace_back(keyWord, fIncludeWord, fChar, fParent);
            } else {
                fDefinitions.emplace_back(fIncludeWord, fChar, fParent);
            }
            fIncludeWord = nullptr;
        }
    }

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

    Definition* findBmhObject(MarkType markType, const std::string& typeName) {
        std::map<std::string, Definition>* map = fMaps[(int) markType].fBmh;
        if (!map) {
            return nullptr;
        }
        return &(*map)[typeName];
    }

    bool findDefinitions();
    static KeyWord FindKey(const char* start, const char* end);

    Definition* findIncludeObject(MarkType markType, const std::string& typeName) {
        std::map<std::string, Definition>* map = fMaps[(int) markType].fInclude;
        if (!map) {
            return nullptr;
        }
        return &(*map)[typeName];
    }

    MarkType getMarkType(MarkLookup lookup) const;
    bool hasEndToken() const;

    const char* lineEnd() const {
        const char* ptr = fChar;
        do {
            SkASSERT(ptr < fEnd);
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

    std::string nameBefore(const char* end) {
        while (' ' >= *--end)
            ;
        const char* last = end;
        while (' ' < *--end)
            ;
        SkASSERT(end > fStart);
        return std::string(end + 1, last - end);
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
    bool parseClass(const char* comment);
    bool parseConst(const char* comment);
    bool parseDefine(const char* comment);
    bool parseEnum(const char* comment);
    bool parseEnumMember(const char* comment);
    const char* parseComment();
    bool parseFile(const char* fileOrPath);
    bool parseFromFile(const char* path);
    bool parseInclude(const std::string& name);
    bool parseMethod(const char* comment);
    bool parseObjects();
    bool parseParam();
    MarkType parsePeek();
    bool parseTemplate(const char* comment);
    bool parseTypedef(const char* comment);
    bool parseUnion(const char* comment);
    char peek() const { SkASSERT(fChar < fEnd); return *fChar; }

    void popBracket() {
        Definition* lastBracket = fBracketStack.top();
        SkASSERT(!lastBracket->fEnd);
        lastBracket->fEnd = fChar;
        SkASSERT(fBracketStack.size() > 0);
        fBracketStack.pop();
        fParent = lastBracket->fParent;
    }

    bool popParentStack(Definition* definition);

    void pushBracket(Bracket bracket) {
        fDefinitions.emplace_back(bracket, fChar, fParent);
        fBracketStack.push(&fDefinitions.back());
        fInComment = Bracket::kSlashSlash == bracket || Bracket::kSlashStar == bracket;
        fInString = Bracket::kString == bracket;
        fInChar = Bracket::kChar == bracket;
        fInCharCommentString = fInChar || fInComment || fInString;
        fParent = &fDefinitions.back();
    }

    void reportError(const char* errorStr) const;

    template <typename T> T reportError(const char* errorStr) const {
        reportError(errorStr);
	    return T();
    }

    void reset() {
        fLine = fChar = fStart;
        fLineCount = 0;
        fTopBracket = Bracket::kNone;
        fInChar = false;
        fInComment = false;
        fInString = false;
        fInCharCommentString = false;
        fPrev = '\0';
        fIncludeWord = nullptr;
        fInBrace = nullptr;
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

    std::string typeName(bool hasEnd);
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
        std::map<std::string, Definition>* fBmh;
        std::map<std::string, Definition>* fInclude;
        MarkType fMarkType;
    };
    
    DefinitionMap fMaps[Last_MarkType + 1];

    std::map<std::string, sk_sp<SkData>> fRawData;
    std::map<std::string, Definition> fClassMap;
    std::map<std::string, Definition> fExampleMap;
    std::map<std::string, Definition> fMethodMap;
    std::map<std::string, Definition> fStdOutMap;
    std::map<std::string, Definition> fTableMap;
    std::map<std::string, Definition> fIncludeMap;
    std::map<std::string, Definition> fIClassMap;
    std::map<std::string, Definition> fIDefineMap;
    std::map<std::string, Definition> fIEnumMap;
    std::map<std::string, Definition> fIStructMap;
    std::map<std::string, Definition> fITemplateMap;
    std::map<std::string, Definition> fITypedefMap;
    std::map<std::string, Definition> fIUnionMap;
    std::vector<Definition> fDefinitions;  // unnamed definitions, named are in a map
    std::vector<Reference> fReferences;
    std::stack<Definition*> fBracketStack;
    Definition* fParent;
    Definition* fInBrace;
    Bracket fTopBracket;
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
};

#endif
