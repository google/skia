/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bookmaker.h"

#include "SkOSFile.h"
#include "SkOSPath.h"

/* 
things to do
if cap word is beginning of sentence, add it to table as lower-case
   word must have only a single initial capital

if word is camel cased, look for :: matches on suffix

when function crosses lines, whole thing isn't seen as a 'word' e.g., search for largeArc in path

words in external not seen
 */
struct CheckEntry {
    string fFile;
    int fLine;
    int fCount;
};

class SpellCheck : public ParserCommon {
public:
    SpellCheck(const BmhParser& bmh) : ParserCommon()
        , fBmhParser(bmh) {
        this->reset();
    }
    bool check(const char* match);
    void report();
private:
    enum class TableState {
        kNone,
        kRow,
        kColumn,
    };

    bool check(Definition* );
    bool checkable(MarkType markType);
    void childCheck(const Definition* def, const char* start);
    void leafCheck(const char* start, const char* end);
    bool parseFromFile(const char* path) override { return true; }
    void printCheck(const string& str);

    void reset() override {
        INHERITED::resetCommon();
        fMethod = nullptr;
        fRoot = nullptr;
        fTableState = TableState::kNone;
        fInCode = false;
        fInConst = false;
        fInDescription = false;
        fInStdOut = false;
    }

    void wordCheck(const string& str);
    void wordCheck(ptrdiff_t len, const char* ch);

    unordered_map<string, CheckEntry> fCode;
    unordered_map<string, CheckEntry> fColons;
    unordered_map<string, CheckEntry> fDigits;
    unordered_map<string, CheckEntry> fDots;
    unordered_map<string, CheckEntry> fParens;  // also hold destructors, operators
    unordered_map<string, CheckEntry> fUnderscores;
    unordered_map<string, CheckEntry> fWords;
    const BmhParser& fBmhParser;
    Definition* fMethod;
    RootDefinition* fRoot;
    TableState fTableState;
    bool fInCode;
    bool fInConst;
    bool fInDescription;
    bool fInStdOut;
    typedef ParserCommon INHERITED;
};

/* This doesn't perform a traditional spell or grammar check, although
   maybe it should. Instead it looks for words used uncommonly and lower
   case words that match capitalized words that are not sentence starters.
   It also looks for articles preceeding capitalized words and their
   modifiers to try to maintain a consistent voice.
   Maybe also look for passive verbs (e.g. 'is') and suggest active ones?
 */
void BmhParser::spellCheck(const char* match) const {
    SpellCheck checker(*this);
    checker.check(match);
    checker.report();
}

bool SpellCheck::check(const char* match) {
    for (const auto& topic : fBmhParser.fTopicMap) {
        Definition* topicDef = topic.second;
        if (topicDef->fParent) {
            continue;
        }
        if (!topicDef->isRoot()) {
            return this->reportError<bool>("expected root topic");
        }
        fRoot = topicDef->asRoot();
        if (string::npos == fRoot->fFileName.rfind(match)) {
            continue;
        }
       this->check(topicDef);
    }
    return true;
}

bool SpellCheck::check(Definition* def) {
    fFileName = def->fFileName;
    fLineCount = def->fLineCount;
    string printable = def->printableName();
    const char* textStart = def->fContentStart;
    if (MarkType::kParam != def->fMarkType && MarkType::kConst != def->fMarkType &&
            TableState::kNone != fTableState) {
        fTableState = TableState::kNone;
    }
    switch (def->fMarkType) {
        case MarkType::kAlias:
            break;
        case MarkType::kAnchor:
            break;
        case MarkType::kBug:
            break;
        case MarkType::kClass:
            this->wordCheck(def->fName);
            break;
        case MarkType::kCode:
            fInCode = true;
            break;
        case MarkType::kColumn:
            break;
        case MarkType::kComment:
            break;
        case MarkType::kConst: {
            fInConst = true;
            if (TableState::kNone == fTableState) {
                fTableState = TableState::kRow;
            }
            if (TableState::kRow == fTableState) {
                fTableState = TableState::kColumn;
            }
            this->wordCheck(def->fName);
            const char* lineEnd = strchr(textStart, '\n');
            this->wordCheck(lineEnd - textStart, textStart);
            textStart = lineEnd;
        } break;
        case MarkType::kDefine:
            break;
        case MarkType::kDefinedBy:
            break;
        case MarkType::kDeprecated:
            break;
        case MarkType::kDescription:
            fInDescription = true;
            break;
        case MarkType::kDoxygen:
            break;
        case MarkType::kEnum:
        case MarkType::kEnumClass:
            this->wordCheck(def->fName);
            break;
        case MarkType::kError:
            break;
        case MarkType::kExample:
            break;
        case MarkType::kExternal:
            break;
        case MarkType::kFile:
            break;
        case MarkType::kFormula:
            break;
        case MarkType::kFunction:
            break;
        case MarkType::kHeight:
            break;
        case MarkType::kImage:
            break;
        case MarkType::kLegend:
            break;
        case MarkType::kList:
            break;
        case MarkType::kMember:
            break;
        case MarkType::kMethod: {
            string method_name = def->methodName();
            string formattedStr = def->formatFunction();
            if (!def->isClone()) {
                this->wordCheck(method_name);
            }
            fTableState = TableState::kNone;
            fMethod = def;
            } break;
        case MarkType::kParam: {
            if (TableState::kNone == fTableState) {
                fTableState = TableState::kRow;
            }
            if (TableState::kRow == fTableState) {
                fTableState = TableState::kColumn;
            }
            TextParser paramParser(def->fFileName, def->fStart, def->fContentStart,
                    def->fLineCount);
            paramParser.skipWhiteSpace();
            SkASSERT(paramParser.startsWith("#Param"));
            paramParser.next(); // skip hash
            paramParser.skipToNonAlphaNum(); // skip Param
            paramParser.skipSpace();
            const char* paramName = paramParser.fChar;
            paramParser.skipToSpace();
            fInCode = true;
            this->wordCheck(paramParser.fChar - paramName, paramName);
            fInCode = false;
       } break;
        case MarkType::kPlatform:
            break;
        case MarkType::kReturn:
            break;
        case MarkType::kRow:
            break;
        case MarkType::kSeeAlso:
            break;
        case MarkType::kStdOut: {
            fInStdOut = true;
            TextParser code(def);
            code.skipSpace();
            while (!code.eof()) {
                const char* end = code.trimmedLineEnd();
                this->wordCheck(end - code.fChar, code.fChar);
                code.skipToLineStart();
            }
            fInStdOut = false;
            } break;
        case MarkType::kStruct:
            fRoot = def->asRoot();
            this->wordCheck(def->fName);
            break;
        case MarkType::kSubtopic:
            this->printCheck(printable);
            break;
        case MarkType::kTable:
            break;
        case MarkType::kTemplate:
            break;
        case MarkType::kText:
            break;
        case MarkType::kTime:
            break;
        case MarkType::kToDo:
            break;
        case MarkType::kTopic:
            this->printCheck(printable);
            break;
        case MarkType::kTrack:
            // don't output children
            return true;
        case MarkType::kTypedef:
            break;
        case MarkType::kUnion:
            break;
        case MarkType::kWidth:
            break;
        default:
            SkASSERT(0); // handle everything
            break;
    }
    this->childCheck(def, textStart);
    switch (def->fMarkType) {  // post child work, at least for tables
        case MarkType::kCode:
            fInCode = false;
            break;
        case MarkType::kColumn:
            break;
        case MarkType::kDescription:
            fInDescription = false;
            break;
        case MarkType::kEnum:
        case MarkType::kEnumClass:
            break;
        case MarkType::kExample:
            break;
        case MarkType::kLegend:
            break;
        case MarkType::kMethod:
            fMethod = nullptr;
            break;
        case MarkType::kConst:
            fInConst = false;
        case MarkType::kParam:
            SkASSERT(TableState::kColumn == fTableState);
            fTableState = TableState::kRow;
            break;
        case MarkType::kReturn:
        case MarkType::kSeeAlso:
            break;
        case MarkType::kRow:
            break;
        case MarkType::kStruct:
            fRoot = fRoot->rootParent();
            break;
        case MarkType::kTable:
            break;
        default:
            break;
    }
    return true;
}

bool SpellCheck::checkable(MarkType markType) {
    return BmhParser::Resolvable::kYes == fBmhParser.fMaps[(int) markType].fResolve;
}

void SpellCheck::childCheck(const Definition* def, const char* start) {
    const char* end;
    fLineCount = def->fLineCount;
    if (def->isRoot()) {
        fRoot = const_cast<RootDefinition*>(def->asRoot());
    }
    for (auto& child : def->fChildren) {
        end = child->fStart;
        if (this->checkable(def->fMarkType)) {
            this->leafCheck(start, end);
        }
        this->check(child);
        start = child->fTerminator;
    }
    if (this->checkable(def->fMarkType)) {
        end = def->fContentEnd;
        this->leafCheck(start, end);
    }
}

void SpellCheck::leafCheck(const char* start, const char* end) {
    TextParser text("", start, end, fLineCount);
    do {
        const char* lineStart = text.fChar;
        text.skipToAlpha();
        if (text.eof()) {
            break;
        }
        const char* wordStart = text.fChar;
        text.fChar = lineStart;
        text.skipTo(wordStart);  // advances line number
        text.skipToNonAlphaNum();
        fLineCount = text.fLineCount;
        string word(wordStart, text.fChar - wordStart);
        wordCheck(word);
    } while (!text.eof());
}

void SpellCheck::printCheck(const string& str) {
    string word;
    for (std::stringstream stream(str); stream >> word; ) {
        wordCheck(word);
    }
}

void SpellCheck::report() {
    for (auto iter : fWords) {
        if (string::npos != iter.second.fFile.find("undocumented.bmh")) {
            continue;
        }
        if (string::npos != iter.second.fFile.find("markup.bmh")) {
            continue;
        }
        if (string::npos != iter.second.fFile.find("usingBookmaker.bmh")) {
            continue;
        }
        if (iter.second.fCount == 1) {
            SkDebugf("%s %s %d\n", iter.first.c_str(), iter.second.fFile.c_str(),
                    iter.second.fLine);
        }
    }
}

void SpellCheck::wordCheck(const string& str) {
    bool hasColon = false;
    bool hasDot = false;
    bool hasParen = false;
    bool hasUnderscore = false;
    bool sawDash = false;
    bool sawDigit = false;
    bool sawSpecial = false;
    SkASSERT(str.length() > 0);
    SkASSERT(isalpha(str[0]) || '~' == str[0]);
    for (char ch : str) {
        if (isalpha(ch) || '-' == ch) {
            sawDash |= '-' == ch;
            continue;
        }
        bool isColon = ':' == ch;
        hasColon |= isColon;
        bool isDot = '.' == ch;
        hasDot |= isDot;
        bool isParen = '(' == ch || ')' == ch || '~' == ch || '=' == ch || '!' == ch;
        hasParen |= isParen;
        bool isUnderscore = '_' == ch;
        hasUnderscore |= isUnderscore;
        if (isColon || isDot || isUnderscore || isParen) {
            continue;
        }
        if (isdigit(ch)) {
            sawDigit = true;
            continue;
        }
        if ('&' == ch || ',' == ch || ' ' == ch) {
            sawSpecial = true;
            continue;
        }
        SkASSERT(0);
    }
    if (sawSpecial && !hasParen) {
        SkASSERT(0);
    }
    bool inCode = fInCode;
    if (hasUnderscore && isupper(str[0]) && ('S' != str[0] || 'K' != str[1])
            && !hasColon && !hasDot && !hasParen && !fInStdOut && !inCode && !fInConst 
            && !sawDigit && !sawSpecial && !sawDash) {
        std::istringstream ss(str);
        string token;
        while (std::getline(ss, token, '_')) {
            this->wordCheck(token);
        }
        return;
    }
    if (!hasColon && !hasDot && !hasParen && !hasUnderscore 
            && !fInStdOut && !inCode && !fInConst && !sawDigit
            && islower(str[0]) && isupper(str[1])) {
        inCode = true;
    }
    auto& mappy = hasColon ? fColons : 
                  hasDot ? fDots :
                  hasParen ? fParens :
                  hasUnderscore ? fUnderscores :
                  fInStdOut || inCode || fInConst ? fCode :
                  sawDigit ? fDigits : fWords;
    auto iter = mappy.find(str);
    if (mappy.end() != iter) {
        iter->second.fCount += 1;
    } else {
        CheckEntry* entry = &mappy[str];
        entry->fFile = fFileName;
        entry->fLine = fLineCount;
        entry->fCount = 1;
    }
}

void SpellCheck::wordCheck(ptrdiff_t len, const char* ch) {
    leafCheck(ch, ch + len);
}
