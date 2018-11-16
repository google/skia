/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef definition_DEFINED
#define definition_DEFINED

#include "textParser.h"

class RootDefinition;
class TextParser;

class Definition : public NonAssignable {
public:
    enum Type {
        kNone,
        kWord,
        kMark,
        kKeyWord,
        kBracket,
        kPunctuation,
        kFileType,
    };

    enum class MethodType {
        kNone,
        kConstructor,
        kDestructor,
        kOperator,
    };

    enum class Operator {
        kUnknown,
        kAdd,
        kAddTo,
        kArray,
        kCast,
        kCopy,
        kDelete,
        kDereference,
        kEqual,
        kMinus,
        kMove,
        kMultiply,
        kMultiplyBy,
        kNew,
        kNotEqual,
        kSubtract,
        kSubtractFrom,
    };

    enum class Format {
        kIncludeReturn,
        kOmitReturn,
    };

    enum class Details {
        kNone,
        kSoonToBe_Deprecated,
        kTestingOnly_Experiment,
        kDoNotUse_Experiment,
        kNotReady_Experiment,
    };

    enum class DetailsType {
        kPhrase,
        kSentence,
    };

    Definition() {}

    Definition(const char* start, const char* end, int line, Definition* parent, char mc)
        : fStart(start)
        , fContentStart(start)
        , fContentEnd(end)
        , fParent(parent)
        , fLineCount(line)
        , fType(Type::kWord)
        , fMC(mc) {
        if (parent) {
            SkASSERT(parent->fFileName.length() > 0);
            fFileName = parent->fFileName;
        }
        this->setParentIndex();
    }

    Definition(MarkType markType, const char* start, int line, Definition* parent, char mc)
        : Definition(markType, start, nullptr, line, parent, mc) {
    }

    Definition(MarkType markType, const char* start, const char* end, int line, Definition* parent, char mc)
        : Definition(start, end, line, parent, mc) {
        fMarkType = markType;
        fType = Type::kMark;
    }

    Definition(Bracket bracket, const char* start, int lineCount, Definition* parent, char mc)
        : Definition(start, nullptr, lineCount, parent, mc) {
        fBracket = bracket;
        fType = Type::kBracket;
    }

    Definition(KeyWord keyWord, const char* start, const char* end, int lineCount,
            Definition* parent, char mc)
        : Definition(start, end, lineCount, parent, mc) {
        fKeyWord = keyWord;
        fType = Type::kKeyWord;
    }

    Definition(Punctuation punctuation, const char* start, int lineCount, Definition* parent, char mc)
        : Definition(start, nullptr, lineCount, parent, mc) {
        fPunctuation = punctuation;
        fType = Type::kPunctuation;
    }

    virtual ~Definition() {}

    virtual RootDefinition* asRoot() { SkASSERT(0); return nullptr; }
    bool boilerplateIfDef();

    bool boilerplateEndIf() {
        return true;
    }

    bool checkMethod() const;
    bool crossCheck2(const Definition& includeToken) const;
    bool crossCheck(const Definition& includeToken) const;
    bool crossCheckInside(const char* start, const char* end, const Definition& includeToken) const;

    Definition* csParent() {
        Definition* test = fParent;
        while (test) {
            if (MarkType::kStruct == test->fMarkType || MarkType::kClass == test->fMarkType) {
                return test;
            }
            test = test->fParent;
        }
        return nullptr;
    }

    string fiddleName() const;
    string fileName() const;
    const Definition* findClone(string match) const;
    string formatFunction(Format format) const;
    const Definition* hasChild(MarkType markType) const;
    bool hasMatch(string name) const;
    Definition* hasParam(string ref);
    bool isClone() const { return fClone; }

    const Definition* iRootParent() const {
        const Definition* test = fParent;
        while (test) {
            if (KeyWord::kClass == test->fKeyWord || KeyWord::kStruct == test->fKeyWord) {
                return test;
            }
            test = test->fParent;
        }
        return nullptr;
    }

    virtual bool isRoot() const { return false; }
    bool isStructOrClass() const;

    int length() const {
        return (int) (fContentEnd - fContentStart);
    }

    const char* methodEnd() const;
    bool methodHasReturn(string name, TextParser* methodParser) const;
    string methodName() const;
    bool nextMethodParam(TextParser* methodParser, const char** nextEndPtr,
                         string* paramName) const;
    static string NormalizedName(string name);
    bool paramsMatch(string fullRef, string name) const;
    bool parseOperator(size_t doubleColons, string& result);

    string printableName() const {
        string result(fName);
        std::replace(result.begin(), result.end(), '_', ' ');
        return result;
    }

    template <typename T> T reportError(const char* errorStr) const {
        TextParser tp(this);
        tp.reportError(errorStr);
        return T();
    }

    virtual RootDefinition* rootParent() { SkASSERT(0); return nullptr; }
    virtual const RootDefinition* rootParent() const { SkASSERT(0); return nullptr; }
    void setCanonicalFiddle();

    void setParentIndex() {
        fParentIndex = fParent ? (int) fParent->fTokens.size() : -1;
    }

    static bool SkipImplementationWords(TextParser& inc);

    string simpleName() {
        size_t doubleColon = fName.rfind("::");
        return string::npos == doubleColon ? fName : fName.substr(doubleColon + 2);
    }

    const Definition* subtopicParent() const {
        Definition* test = fParent;
        while (test) {
            if (MarkType::kTopic == test->fMarkType || MarkType::kSubtopic == test->fMarkType) {
                return test;
            }
            test = test->fParent;
        }
        return nullptr;
    }

    const Definition* topicParent() const {
        Definition* test = fParent;
        while (test) {
            if (MarkType::kTopic == test->fMarkType) {
                return test;
            }
            test = test->fParent;
        }
        return nullptr;
    }

    void trimEnd();

    string fText;  // if text is constructed instead of in a file, it's put here
    const char* fStart = nullptr;  // .. in original text file, or the start of fText
    const char* fContentStart;  // start past optional markup name
    string fName;
    string fFiddle;  // if its a constructor or operator, fiddle name goes here
    string fCode;  // suitable for autogeneration of #Code blocks in bmh
    const char* fContentEnd = nullptr;  // the end of the contained text
    const char* fTerminator = nullptr;  // the end of the markup, normally ##\n or \n
    Definition* fParent = nullptr;
    list<Definition> fTokens;
    vector<Definition*> fChildren;
    string fHash;  // generated by fiddle
    string fFileName;
    mutable string fWrapper; // used by Example to wrap into proper function
    size_t fLineCount = 0;
    int fParentIndex = 0;
    MarkType fMarkType = MarkType::kNone;
    KeyWord fKeyWord = KeyWord::kNone;
    Bracket fBracket = Bracket::kNone;
    Punctuation fPunctuation = Punctuation::kNone;
    MethodType fMethodType = MethodType::kNone;
    Operator fOperator = Operator::kUnknown;
    Type fType = Type::kNone;
    char fMC = '#';
    bool fClone = false;
    bool fCloned = false;
    bool fOperatorConst = false;
    bool fPrivate = false;
    Details fDetails = Details::kNone;
    bool fMemberStart = false;
    bool fAnonymous = false;
    bool fUndocumented = false;  // include symbol comment has deprecated, private, experimental
    mutable bool fVisited = false;
};

class RootDefinition : public Definition {
public:
    enum class AllowParens {
        kNo,
        kYes,
    };

    struct SubtopicContents {
        SubtopicContents()
            : fShowClones(false) {
        }

        vector<Definition*> fMembers;
        bool fShowClones;
    };

    RootDefinition() {
    }

    RootDefinition(MarkType markType, const char* start, int line, Definition* parent, char mc)
            : Definition(markType, start, line, parent, mc) {
        if (MarkType::kSubtopic != markType && MarkType::kTopic != markType) {
            if (parent) {
                fNames.fName = parent->fName;
                fNames.fParent = &parent->asRoot()->fNames;
            }
        }
    }

    RootDefinition(MarkType markType, const char* start, const char* end, int line,
            Definition* parent, char mc) : Definition(markType, start, end, line, parent, mc) {
    }

    ~RootDefinition() override {
        for (auto& iter : fBranches) {
            delete iter.second;
        }
    }

    RootDefinition* asRoot() override { return this; }
    void clearVisited();
    bool dumpUnVisited();
    Definition* find(string ref, AllowParens );
    bool isRoot() const override { return true; }

    SubtopicContents& populator(const char* key) {
        return fPopulators[key];
    }

    RootDefinition* rootParent() override { return fRootParent; }
    const RootDefinition* rootParent() const override { return fRootParent; }
    void setRootParent(RootDefinition* rootParent) { fRootParent = rootParent; }

    unordered_map<string, RootDefinition*> fBranches;
    unordered_map<string, Definition> fLeaves;
    unordered_map<string, SubtopicContents> fPopulators;
    NameMap fNames;
private:
    RootDefinition* fRootParent = nullptr;
};

#endif
