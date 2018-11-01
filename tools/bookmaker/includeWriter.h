/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef includeWriter_DEFINED
#define includeWriter_DEFINED

#include "includeParser.h"

class IncludeWriter : public IncludeParser {
public:
    enum class Word {
        kStart,
        kCap,
        kFirst,
        kUnderline,
        kMixed,
    };

    enum class Phrase {
        kNo,
        kYes,
    };

    enum class PunctuationState {
        kStart,
        kDelimiter,
        kParen,     // treated as a delimiter unless following a space, and followed by word
        kPeriod,
        kSpace,
    };

    enum class RefType {
        kUndefined,
        kNormal,
        kExternal,
    };

	enum class SkipFirstLine {
		kNo,
		kYes,
	};

    enum class Wrote {
        kNone,
        kLF,
        kChars,
    };

    enum class MemberPass {
        kCount,
        kOut,
    };

    enum class ItemState {
        kNone,
        kName,
        kValue,
        kComment,
    };

    struct IterState {
        IterState (list<Definition>::iterator tIter, list<Definition>::iterator tIterEnd)
            : fDefIter(tIter)
            , fDefEnd(tIterEnd) {
        }
        list<Definition>::iterator fDefIter;
        list<Definition>::iterator fDefEnd;
    };

    struct ParentPair {
        const Definition* fParent;
        const ParentPair* fPrev;
    };

    struct Preprocessor {
        Preprocessor() {
            reset();
        }

        void reset() {
            fDefinition = nullptr;
            fStart = nullptr;
            fEnd = nullptr;
            fWord = false;
        }

        const Definition* fDefinition;
        const char* fStart;
        const char* fEnd;
        bool fWord;
    };

    struct Item {
        void reset() {
            fName = "";
            fValue = "";
        }

        string fName;
        string fValue;
    };

    struct LastItem {
        const char* fStart;
        const char* fEnd;
    };

    struct ItemLength {
        int fCurName;
        int fCurValue;
        int fLongestName;
        int fLongestValue;
    };

    IncludeWriter() : IncludeParser() {
        this->reset();
    }

    ~IncludeWriter() override {}

    bool contentFree(int size, const char* data) const {
        while (size > 0 && data[0] <= ' ') {
            --size;
            ++data;
        }
        while (size > 0 && data[size - 1] <= ' ') {
            --size;
        }
        return 0 == size;
    }

    bool checkChildCommentLength(const Definition* parent, MarkType childType) const;
    void checkEnumLengths(const Definition& child, string enumName, ItemLength* length) const;
	void constOut(const Definition* memberStart, const Definition* bmhConst);
    void constSizeMembers(const RootDefinition* root);
    bool defineOut(const Definition& );
    bool descriptionOut(const Definition* def, SkipFirstLine , Phrase );
    void enumHeaderOut(RootDefinition* root, const Definition& child);
    string enumMemberComment(const Definition* currentEnumItem, const Definition& child) const;
    const Definition* enumMemberForComment(const Definition* currentEnumItem) const;
    ItemState enumMemberName(const Definition& child,
            const Definition* token, Item* , LastItem* , const Definition** enumItem);
    void enumMemberOut(const Definition* currentEnumItem, const Definition& child,
            const Item& , Preprocessor& );
    void enumMembersOut(Definition& child);
    bool enumPreprocessor(Definition* token, MemberPass pass,
        vector<IterState>& iterStack, IterState** iterState, Preprocessor* );
    void enumSizeItems(const Definition& child);
    bool findEnumSubtopic(string undername, const Definition** ) const;
    void firstBlock(int size, const char* data);
    bool firstBlockTrim(int size, const char* data);
	Definition* findMemberCommentBlock(const vector<Definition*>& bmhChildren, string name) const;
    Definition* findMethod(string name, RootDefinition* ) const;

    void indentDeferred(IndentKind kind) {
        if (fIndentNext) {
            this->indentIn(kind);
            fIndentNext = false;
        }
    }

    int lookupMethod(const PunctuationState punctuation, const Word word,
            const int start, const int run, int lastWrite,
            const char* data, bool hasIndirection);
    int lookupReference(const PunctuationState punctuation, const Word word,
            const int start, const int run, int lastWrite, const char last,
            const char* data);
    const Definition* matchMemberName(string matchName, const Definition& child) const;
    void methodOut(Definition* method, const Definition& child);
    bool populate(Definition* def, ParentPair* parentPair, RootDefinition* root);
    bool populate(BmhParser& bmhParser);

    void reset() override {
        INHERITED::resetCommon();
        fBmhParser = nullptr;
        fDeferComment = nullptr;
        fBmhMethod = nullptr;
        fEnumDef = nullptr;
        fMethodDef = nullptr;
        fBmhConst = nullptr;
        fConstDef = nullptr;
        fLastDescription = nullptr;
        fStartSetter = nullptr;
        fBmhStructDef = nullptr;
        fContinuation = nullptr;
        fInStruct = false;
        fWroteMethod = false;
        fIndentNext = false;
        fPendingMethod = false;
        fFirstWrite = false;
        fStructEnded = false;
        fWritingIncludes = true;
   }

    string resolveAlias(const Definition* );
    string resolveMethod(const char* start, const char* end, bool first);
    string resolveRef(const char* start, const char* end, bool first, RefType* refType);
    Wrote rewriteBlock(int size, const char* data, Phrase phrase);
    void setStart(const char* start, const Definition * );
    void setStartBack(const char* start, const Definition * );
    Definition* structMemberOut(const Definition* memberStart, const Definition& child);
    void structOut(const Definition* root, const Definition& child,
            const char* commentStart, const char* commentEnd);
    void structSizeMembers(const Definition& child);
    bool writeHeader(std::pair<const string, Definition>& );
private:
    vector<const Definition* > fICSStack;
    BmhParser* fBmhParser;
    Definition* fDeferComment;
    const Definition* fBmhMethod;
    const Definition* fEnumDef;
    const Definition* fMethodDef;
    const Definition* fBmhConst;
    const Definition* fConstDef;
    const Definition* fLastDescription;
    const Definition* fStartSetter;
    Definition* fBmhStructDef;
    const char* fContinuation;  // used to construct paren-qualified method name
    int fAnonymousEnumCount;
    int fEnumItemValueTab;
    int fEnumItemCommentTab;
    int fStructMemberTab;
    int fStructValueTab;
    int fStructCommentTab;
    int fStructMemberLength;
    int fConstValueTab;
    int fConstCommentTab;
    int fConstLength;
    bool fInStruct;  // set if struct is inside class
    bool fWroteMethod;
    bool fIndentNext;
    bool fPendingMethod;
    bool fFirstWrite;  // set to write file information just after includes
    bool fStructEnded;  // allow resetting indent after struct is complete

    typedef IncludeParser INHERITED;
};

#endif
