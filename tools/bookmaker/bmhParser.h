/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef bmhParser_DEFINED
#define bmhParser_DEFINED

#include "CommandLineFlags.h"

#include "definition.h"
#include "parserCommon.h"

class BmhParser : public ParserCommon {
public:
    enum class MarkLookup {
        kRequire,
        kAllowUnknown,
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
        kYes,
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
        , { &fClassMap,    MarkType::kTemplate }
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
    static bool IsExemplary(const Definition* );
    string loweredTopic(string name, Definition* def);
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

    void parseHashAnchor(Definition* );
    void parseHashFormula(Definition* );
    void parseHashLine(Definition* );
    bool popParentStack(Definition* );
    void reportDuplicates(const Definition& , string dup) const;
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

    void setUpGlobalSubstitutes();
    void setUpPartialSubstitute(string name);
    void setUpSubstitute(string name, Definition* def);
    void setUpSubstitutes(const Definition* parent, NameMap* );
    void setWrapper(Definition* def) const;
    bool skipNoName();
    bool skipToDefinitionEnd(MarkType markType);
	bool skipToString();
    void           spellCheck(const char* match, CommandLineFlags::StringArray report) const;
    void           spellStatus(const char* match, CommandLineFlags::StringArray report) const;
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
    NameMap fGlobalNames;
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

#endif
