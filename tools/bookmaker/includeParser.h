/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef includeParser_DEFINED
#define includeParser_DEFINED

#include "SkString.h"

#include "parserCommon.h"

class BmhParser;

struct IClassDefinition : public Definition {
    unordered_map<string, Definition*> fConsts;
    unordered_map<string, Definition*> fDefines;
    unordered_map<string, Definition*> fEnums;
    unordered_map<string, Definition*> fMembers;
    unordered_map<string, Definition*> fMethods;
    unordered_map<string, Definition*> fStructs;
    unordered_map<string, Definition*> fTypedefs;
};

class IncludeParser : public ParserCommon {
public:
    enum class IsStruct {
        kNo,
        kYes,
    };

    enum class Elided {
        kNo,
        kYes,
    };

    enum class Suggest {
        kMethodMissing,
        kMethodDiffers,
    };

    struct CheckCode {
        enum class State {
            kNone,
            kClassDeclaration,
            kConstructor,
            kForwardDeclaration,
            kMethod,
        };

        void reset() {
            fInDebugCode = nullptr;
            fPrivateBrace = 0;
            fBraceCount = 0;
            fIndent = 0;
            fDoubleReturn = 0;
            fState = State::kNone;
            fPrivateProtected = false;
            fTypedefReturn = false;
            fSkipAPI = false;
            fSkipInline = false;
            fSkipWarnUnused = false;
            fWriteReturn = false;
        }

        const char* fInDebugCode;
        int fPrivateBrace;
        int fBraceCount;
        int fIndent;
        int fDoubleReturn;
        State fState;
        bool fPrivateProtected;
        bool fTypedefReturn;
        bool fSkipAPI;
        bool fSkipInline;
        bool fSkipWarnUnused;
        bool fWriteReturn;
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

    bool advanceInclude(TextParser& i);
    bool inAlignAs() const;
    void checkForMissingParams(const vector<string>& methodParams,
                               const vector<string>& foundParams);
    bool checkForWord();
    void checkName(Definition* );
    void checkTokens(list<Definition>& tokens, string key, string className,
            RootDefinition* root, BmhParser& bmhParser);
    string className() const;

    string codeBlock(const Definition& def, bool inProgress) const {
        return codeBlock(def.fMarkType, def.fName, inProgress);
    }

    string codeBlock(MarkType markType, string name, bool inProgress) const {
        if (MarkType::kClass == markType || MarkType::kStruct == markType) {
            auto map = fIClassMap.find(name);
            SkASSERT(fIClassMap.end() != map || inProgress);
            return fIClassMap.end() != map ? map->second.fCode : "";
        }
        if (MarkType::kConst == markType) {
            auto map = fIConstMap.find(name);
            SkASSERT(fIConstMap.end() != map);
            return map->second->fCode;
        }
        if (MarkType::kDefine == markType) {
            auto map = fIDefineMap.find(name);
            SkASSERT(fIDefineMap.end() != map);
            return map->second->fCode;
        }
        if (MarkType::kEnum == markType || MarkType::kEnumClass == markType) {
            auto map = fIEnumMap.find(name);
            SkASSERT(fIEnumMap.end() != map);
            return map->second->fCode;
        }
        if (MarkType::kTypedef == markType) {
            auto map = fITypedefMap.find(name);
            SkASSERT(fITypedefMap.end() != map);
            return map->second->fCode;
        }
        SkASSERT(0);
        return "";
    }

    void codeBlockAppend(string& result, string ) const;
    void codeBlockAppend(string& result, char ch) const;
    void codeBlockSpaces(string& result, int indent) const;

    bool crossCheck(BmhParser& );
    IClassDefinition* defineClass(const Definition& includeDef, string className);
    void dumpClassTokens(IClassDefinition& classDef);
    void dumpComment(const Definition& );
    void dumpCommonTail(const Definition& );
    void dumpConst(const Definition& , string className);
    void dumpDefine(const Definition& );
    void dumpEnum(const Definition& , string name);
    bool dumpGlobals(string* globalFileName, long int* globalTell);
    bool dumpMethod(const Definition& , string className);
    void dumpMember(const Definition& );
    bool dumpTokens();
    bool dumpTokens(string skClassName, string globalFileName, long int* globalTell);
    void dumpTypedef(const Definition& , string className);

    string elidedCodeBlock(const Definition& );
    string filteredBlock(string inContents, string filterContents);
    bool findCommentAfter(const Definition& includeDef, Definition* markupDef);
    bool findComments(const Definition& includeDef, Definition* markupDef);
    Definition* findIncludeObject(const Definition& includeDef, MarkType markType,
                                  string typeName);
    static KeyWord FindKey(const char* start, const char* end);
    Definition* findMethod(const Definition& bmhDef);
    Bracket grandParentBracket() const;
    const Definition* include(string ) const;
    bool isClone(const Definition& token);
    bool isConstructor(const Definition& token, string className);
    bool isInternalName(const Definition& token);
    bool isMember(const Definition& token) const;
    bool isOperator(const Definition& token);
    bool isUndocumentable(string filename, const char* start, const char* end, int lineCount);
    Definition* parentBracket(Definition* parent) const;
    bool parseChar();
    bool parseComment(string filename, const char* start, const char* end, int lineCount,
            Definition* markupDef, bool* undocumentedPtr);
    bool parseClass(Definition* def, IsStruct);
    bool parseConst(Definition* child, Definition* markupDef);
    bool parseDefine(Definition* child, Definition* markupDef);
    bool parseEnum(Definition* child, Definition* markupDef);
    bool parseEnumConst(list<Definition>::iterator& tokenIter,
            const list<Definition>::iterator& tokenEnd, Definition* markupChild);

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
    bool parseOneEnumConst(list<Definition>& constList, Definition* markupChild, bool skipWord);
    bool parseTemplate(Definition* child, Definition* markupDef);
    bool parseTypedef(Definition* child, Definition* markupDef);
    bool parseUsing();
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

    void suggestFix(Suggest suggest, const Definition& iDef, const RootDefinition* root,
            const Definition* bDef);
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
    void writeCodeBlock();
    string writeCodeBlock(const Definition& );
    string writeCodeBlock(TextParser& i, MarkType , int indent);

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
    CheckCode fCheck;
    Definition* fRootTopic;
    Definition* fConstExpr;
    Definition* fInBrace;
    Definition* fLastObject;
    Definition* fPriorEnum;
    Definition* fPriorObject;
    const Definition* fPreviousDef;
    int fPriorIndex;
    const char* fIncludeWord;
    Elided fElided;
    MarkType fPreviousMarkType;
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

#endif
