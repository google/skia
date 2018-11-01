/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef mdOut_DEFINED
#define mdOut_DEFINED

#include "parserCommon.h"

class IncludeParser;

class MdOut : public ParserCommon {
public:
    struct SubtopicDescriptions {
        string fSingular;
        string fPlural;
        string fOneLiner;
        string fDetails;
    };

    MdOut(BmhParser& bmh, IncludeParser& inc) : ParserCommon()
        , fBmhParser(bmh)
        , fIncludeParser(inc) {
        this->reset();
        this->addPopulators();
        fBmhParser.setUpGlobalSubstitutes();
    }

    bool buildReferences(const char* docDir, const char* mdOutDirOrFile);
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

    void addCodeBlock(const Definition* def, string& str) const;
    void addPopulators();
    string addIncludeReferences(const char* refStart, const char* refEnd);
    string addReferences(const char* start, const char* end, Resolvable );
    string anchorDef(string def, string name);
    string anchorLocalRef(string ref, string name);
    string anchorRef(string def, string name);
    bool buildRefFromFile(const char* fileName, const char* outDir);
    bool checkParamReturnBody(const Definition* def);
    Definition* checkParentsForMatch(Definition* test, string ref) const;
    void childrenOut(Definition* def, const char* contentStart);
    Definition* csParent();
    bool findLink(string ref, string* link);
    Definition* findParamType();
    string getMemberTypeName(const Definition* def, string* memberType);
    static bool HasDetails(const Definition* def);
    bool hasWordSpace(string ) const;
    void htmlOut(string );
    Definition* isDefined(const TextParser& , string ref, Resolvable );
    Definition* isDefinedByParent(RootDefinition* root, string ref);
    string linkName(const Definition* ) const;
    string linkRef(string leadingSpaces, Definition*, string ref, Resolvable );
    void markTypeOut(Definition* , const Definition** prior);
    void mdHeaderOut(int depth) { mdHeaderOutLF(depth, 2); }
    void mdHeaderOutLF(int depth, int lf);
    void parameterHeaderOut(TextParser& paramParser, const Definition** prior, Definition* def);
    void parameterTrailerOut();
    bool parseFromFile(const char* path) override { return true; }
    bool phraseContinues(string phrase, string* priorWord, string* priorLink) const;
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
        fParamEnd = nullptr;
        fInProgress = false;
    }

    Resolvable resolvable(const Definition* definition) const {
        MarkType markType = definition->fMarkType;
        if (MarkType::kCode == markType) {
            for (auto child : definition->fChildren) {
                if (MarkType::kLiteral == child->fMarkType) {
                    return Resolvable::kLiteral;
                }
            }
        }
        if ((MarkType::kExample == markType
                || MarkType::kFunction == markType) && fHasFiddle) {
            return Resolvable::kNo;
        }
        return BmhParser::kMarkProps[(int) markType].fResolve;
    }

    void resolveOut(const char* start, const char* end, Resolvable );
    void returnHeaderOut(const Definition** prior, Definition* def);
    void rowOut(string col1, const Definition* col2);
    void rowOut(const char * name, string description, bool literalName);

    void subtopicOut(string name);
    void subtopicsOut(Definition* def);
    void subtopicOut(string key, const vector<Definition*>& data, const Definition* csParent,
        const Definition* topicParent, bool showClones);
    bool subtopicRowOut(string keyName, const Definition* entry);
    void summaryOut(const Definition* def, MarkType , string name);
    string tableDataCodeDef(const Definition* def);
    string tableDataCodeDef(string def, string name);
    string tableDataCodeLocalRef(string name);
    string tableDataCodeLocalRef(string ref, string name);
    string tableDataCodeRef(const Definition* ref);
    string tableDataCodeRef(string ref, string name);
    void writeSubtopicTableHeader(string key);

    vector<const Definition*> fClassStack;
    unordered_map<string, vector<AnchorDef> > fAllAnchorDefs;
    unordered_map<string, vector<string> > fAllAnchorRefs;
    NameMap* fNames;
    BmhParser& fBmhParser;
    IncludeParser& fIncludeParser;
    const Definition* fEnumClass;
    const Definition* fLastDef;
    Definition* fMethod;
    RootDefinition* fRoot;  // used in generating populated tables; always struct or class
    RootDefinition* fSubtopic; // used in resolving symbols
    const Definition* fLastParam;
    TableState fTableState;
    unordered_map<string, SubtopicDescriptions> fPopulators;
    unordered_map<string, string> fPhraseParams;
    const char* fParamEnd;
    bool fAddRefFailed;
    bool fHasFiddle;
    bool fInDescription;   // FIXME: for now, ignore unfound camelCase in description since it may
                           // be defined in example which at present cannot be linked to
    bool fInList;
    bool fLiteralAndIndent;
    bool fResolveAndIndent;
    bool fOddRow;
    bool fHasDetails;
    bool fInProgress;
    typedef ParserCommon INHERITED;
};

#endif
