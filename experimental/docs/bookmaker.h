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

enum class MarkType {
    kUnknown,
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
    kString,
    kChar,
    kSlashStar,
    kSlashSlash,
    kPoundIf
};

struct Definition {
    Definition()
        : fStart(nullptr)
        , fEnd(nullptr)
        , fParent(nullptr)
        , fSibling(nullptr)
        , fChild(nullptr)
        , fMarkType(MarkType::kUnknown)
    {
    }

    const char* fStart;  // .. in original text file
    std::string fName;
    const char* fEnd;
    Definition* fParent;
    Definition* fSibling;
    Definition* fChild;
    MarkType fMarkType;
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
          { nullptr,        nullptr,        MarkType::kUnknown }
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
        , fLine(nullptr)
        , fChar(nullptr)
        , fEnd(nullptr)
        , fFileName(nullptr)
        , fLineCount(0)
        , fParseType(ParseType::kUndefined)
        , fTrackBrackets{false)
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

    const char* contains(const char* match) const {
        size_t len = strlen(match);
        const char* end = this->lineEnd() - len;
        const char* test = fChar;
        while (test < end) {
             if (0 == strncmp(test, match, len)) {
                 return test;
             }
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

    Definition* findIncludeObject(MarkType markType, const std::string& typeName) {
        std::map<std::string, Definition>* map = fMaps[(int) markType].fInclude;
        if (!map) {
            return nullptr;
        }
        return &(*map)[typeName];
    }

    bool findDefinitions();
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

    char next() { 
        SkASSERT(fChar < fEnd);
        char result = *fChar++;
        if (fTrackBrackets) {
            this->trackBracket(result);
        }
        if ('\n' == result) {
            ++fLineCount;
            fLine = fChar;
        }
        return result; 
    }

    bool parseClass(const char* comment);
    bool parseConst(const char* comment);
    bool parseDefine(const char* comment);
    bool parseEnum(const char* comment);
    bool parseEnumMember(const char* comment);
    const char* parseComment();
    bool parseFile(const char* fileOrPath);
    bool parseFromFile(const char* path);
    bool parseInclude();
    bool parseMethod(const char* comment);
    bool parseObjects();
    bool parseParam();
    MarkType parsePeek();
    bool parseTemplate(const char* comment);
    bool parseTypedef(const char* comment);
    bool parseUnion(const char* comment);
    char peek() const { SkASSERT(fChar < fEnd); return *fChar; }
    bool popParentStack(Definition* definition);
    void reportError(const char* errorStr) const;

    template <typename T> T reportError(const char* errorStr) const {
        reportError(errorStr);
	    return T();
    }

    void reset() {
        fLine = fChar = fStart;
        fLineCount = 0;
    }

    void setAsParent(Definition* definition) {
		if (fParent) {
			definition->fSibling = fParent->fChild; // constructed in reverse order
			fParent->fChild = definition;  // fix up child order once done
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

    bool trackBracket(char test) {
        char prev = &fChar[-2] >= fStart ? fChar[-2] : '\0';
        std::stack<Bracket>& stack = fBracketStack;
        Bracket top = stack.empty() ? Bracket::kNone : stack.top();
        bool inComment = Bracket::kSlashSlash == top || Bracket::kSlashStar == top;
        bool inString = Bracket::kString == top;
        bool inChar = Bracket::kChar == top;
        switch (test) {
        case '\n':
            if ('\\' != prev) {
                if (Bracket::kSlashSlash == top) {
                    stack.pop();
                }
            }
            break;
        case '*':
            if (!inComment && '/' == prev) {
                stack.push(Bracket::kSlashStar);
            }
            break;
        case '/':
            if ('*' == prev) {
                if (!inComment) {
                    return reportError<bool>("malformed comment");
                }
                stack.pop();
            } else if (!inComment && '*' == prev) {
                stack.push(Bracket::kSlashSlash);
            }
            break;
        case '\'':
            if (Bracket::kChar == top) {
                stack.pop();
            } else if (!inComment && !inString && prev != '\\') {
                stack.push(Bracket::kChar);
            }
            break;
        case '\"':
            if (Bracket::kString == top) {
                stack.pop();
            } else if (!inComment && !inChar && prev != '\\') {
                stack.push(Bracket::kString);
            }
            break;
        }
        case '(':

            break;
    }

    std::string typeName(bool hasEnd);
    void validate() const;

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
    std::map<std::string, Definition> fIClassMap;
    std::map<std::string, Definition> fIDefineMap;
    std::map<std::string, Definition> fIEnumMap;
    std::map<std::string, Definition> fIStructMap;
    std::map<std::string, Definition> fITemplateMap;
    std::map<std::string, Definition> fITypedefMap;
    std::map<std::string, Definition> fIUnionMap;
    std::vector<Reference> fReferences;
    std::stack<Bracket> fBracketStack;
    Definition* fParent;
    const char* fStart;
    const char* fLine;
    const char* fChar;
    const char* fEnd;
    const char* fFileName;
    size_t fLineCount;
    char fMC;  // markup character
    ParseType fParseType;
    bool fTrackBrackets;
};

#endif
