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
        fNames = &fBmhParser.fGlobalNames;
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

    struct DefinedState {
        DefinedState(const MdOut& mdOut, const char* refStart, const char* refEnd,
                Resolvable resolvable)
            : fBmhParser(&mdOut.fBmhParser)
            , fNames(mdOut.fNames)
            , fGlobals(&mdOut.fBmhParser.fGlobalNames)
            , fLastDef(mdOut.fLastDef)
            , fMethod(mdOut.fMethod)
            , fSubtopic(mdOut.fSubtopic)
            , fRoot(mdOut.fRoot)
            , fRefStart(refStart)
            , fRefEnd(refEnd)
            , fResolvable(resolvable)
            , fInProgress(mdOut.fInProgress) {
            TextParser matrixParser(fLastDef->fFileName, refStart, refEnd, fLastDef->fLineCount);
            const char* bracket = matrixParser.anyOf("|=\n");
            fInMatrix = bracket && ('|' == bracket[0] || '=' == bracket[0]);
        }

        void backup() {
            fPriorWord = fBack2Word;
            fPriorLink = "";
            fPriorSeparator = "";
            fSeparator = fBack2Separator;
        }

        bool findEnd(const char* start) {
            if (fEnd < fRefEnd && '~' == fEnd[0]) {
                ++fEnd;
            }
            do {
                while (fEnd < fRefEnd && (isalnum(fEnd[0]) || '-' == fEnd[0] || '_' == fEnd[0])) {
                    ++fEnd;
                }
                if (fEnd + 1 >= fRefEnd || '/' != fEnd[0] || start == fEnd || !isalpha(fEnd[-1])
                        || !isalpha(fEnd[1])) {
                    break;  // stop unless pattern is xxx/xxx as in I/O
                }
                ++fEnd; // skip slash
            } while (true);
            while (start != fEnd && '-' == fEnd[-1]) {
                --fEnd;
            }
            return start == fEnd;
        }

        bool findLink(string ref, string* linkPtr, bool addParens);
        bool findLink(string ref, string* linkPtr, unordered_map<string, Definition*>& map);
        bool hasWordSpace(string wordSpace) const;
        void setLink();

        string nextSeparator(const char* start) {
            fBack2Separator = fPriorSeparator;
            fPriorSeparator = fSeparator;
            fEnd = start;
            return fBack2Separator;
        }

        const char* nextWord() {
            fBack2Word = fPriorWord;
            fPriorWord = fWord;
            fPriorLink = fLink;
            return fEnd;
        }

        bool phraseContinues(string phrase, string* priorWord, string* priorLink) const;

        void setLower() {
            fAddParens = false;
            bool allLower = std::all_of(fWord.begin(), fWord.end(), [](char c) {
                return islower(c);
            });
            bool hasParens = fEnd + 2 <= fRefEnd && "()" == string(fEnd, 2);
            if (hasParens) {
                if (allLower) {
                    fWord += "()";
                    fEnd += 2;
                }
            } else if (allLower) {
                fAddParens = true;
            }
        }

        bool setPriorSpaceWord(const char** startPtr) {
            if (!fPriorSpace) {
                return false;
            }
            string phrase = fPriorWord + fWord;
            if (this->phraseContinues(phrase, &fPriorWord, &fPriorLink)) {
                *startPtr = fEnd;
                return true;
            }
            fPriorWord = fPriorWord.substr(0, fPriorWord.length() - 1);
            return false;
        }

        void skipParens() {
            if ("()" == fSeparator.substr(0, 2)) {
                string funcRef = fPriorWord + "()";
                if (this->findLink(funcRef, &fPriorLink, false)) {
                    fPriorWord = funcRef;
                    fSeparator = fSeparator.substr(2);
                }
            }
        }

        const char* skipWhiteSpace() {
            const char* start = fSeparatorStart;
            bool whiteSpace = start < fRefEnd && ' ' >= start[0];
            while (start < fRefEnd && !isalpha(start[0]) && '~' != start[0]) {
                whiteSpace &= ' ' >= start[0];
                ++start;
            }
            fPriorSpace = false;
            fSeparator = string(fSeparatorStart, start - fSeparatorStart);
            if ("" != fPriorWord && whiteSpace) {
                string wordSpace = fPriorWord + ' ';
                if (this->hasWordSpace(wordSpace)) {
                    fPriorWord = wordSpace;
                    fPriorSpace = true;
                }
            }
            return start;
        }

        string fRef;
        string fBack2Word;
        string fBack2Separator;
        string fPriorWord;
        string fPriorLink;
        string fPriorSeparator;
        string fWord;
        string fLink;
        string fSeparator;
        string fMethodName;
        BmhParser* fBmhParser;
        const NameMap* fNames;
        const NameMap* fGlobals;
        const Definition* fLastDef;
        const Definition* fMethod;
        const Definition* fSubtopic;
        const Definition* fPriorDef;
        const RootDefinition* fRoot;
        const char* fSeparatorStart;
        const char* fRefStart;
        const char* fRefEnd;
        const char* fEnd;
        Resolvable fResolvable;
        bool fAddParens;
        bool fInMatrix;
        bool fInProgress;
        bool fPriorSpace;
    };

    void addCodeBlock(const Definition* def, string& str) const;
    void addPopulators();
    string addReferences(const char* start, const char* end, Resolvable );
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
    bool isDefined(DefinedState& s);
    const Definition* isDefined(const TextParser& , Resolvable );
    string linkName(const Definition* ) const;
    void markTypeOut(Definition* , const Definition** prior);
    void mdHeaderOut(int depth) { mdHeaderOutLF(depth, 2); }
    void mdHeaderOutLF(int depth, int lf);
    void parameterHeaderOut(TextParser& paramParser, const Definition** prior, Definition* def);
    void parameterTrailerOut();
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
        fNames = nullptr;
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
