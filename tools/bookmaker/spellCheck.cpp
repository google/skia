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
    void report(SkCommandLineFlags::StringArray report);
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
        fInFormula = false;
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
    bool fInFormula;
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
void BmhParser::spellCheck(const char* match, SkCommandLineFlags::StringArray report) const {
    SpellCheck checker(*this);
    checker.check(match);
    checker.report(report);
}

void BmhParser::spellStatus(const char* statusFile, SkCommandLineFlags::StringArray report) const {
    SpellCheck checker(*this);
    StatusIter iter(statusFile, ".bmh", StatusFilter::kInProgress);
    string match = iter.baseDir();
    checker.check(match.c_str());
    checker.report(report);
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

static bool all_lower(const string& str) {
    for (auto c : str) {
        if (!islower(c)) {
            return false;
        }
    }
    return true;
}

bool SpellCheck::check(Definition* def) {
    fFileName = def->fFileName;
    fLineCount = def->fLineCount;
    string printable = def->printableName();
    const char* textStart = def->fContentStart;
    if (MarkType::kParam != def->fMarkType && MarkType::kConst != def->fMarkType &&
            MarkType::kPrivate != def->fMarkType && TableState::kNone != fTableState) {
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
        case MarkType::kDuration:
            break;
        case MarkType::kEnum:
        case MarkType::kEnumClass:
            this->wordCheck(def->fName);
            break;
        case MarkType::kError:
            break;
        case MarkType::kExample:
            break;
        case MarkType::kExperimental:
            break;
        case MarkType::kExternal:
            break;
        case MarkType::kFile:
            break;
        case MarkType::kFormula:
            fInFormula = true;
            break;
        case MarkType::kFunction:
            break;
        case MarkType::kHeight:
            break;
        case MarkType::kImage:
            break;
        case MarkType::kLegend:
            break;
        case MarkType::kLink:
            break;
        case MarkType::kList:
            break;
        case MarkType::kLiteral:
            break;
        case MarkType::kMarkChar:
            break;
        case MarkType::kMember:
            break;
        case MarkType::kMethod: {
            string method_name = def->methodName();
            if (all_lower(method_name)) {
                method_name += "()";
            }
            string formattedStr = def->formatFunction();
            if (!def->isClone() && Definition::MethodType::kOperator != def->fMethodType) {
                this->wordCheck(method_name);
            }
            fTableState = TableState::kNone;
            fMethod = def;
            } break;
        case MarkType::kNoExample:
            break;
        case MarkType::kOutdent:
            break;
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
        case MarkType::kPrivate:
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
        case MarkType::kSubstitute:
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
        case MarkType::kVolatile:
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
        case MarkType::kFormula:
            fInFormula = false;
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
    const char* chPtr = start;
    int inAngles = 0;
    int inParens = 0;
    bool inQuotes = false;
    bool allLower = true;
    char priorCh = 0;
    char lastCh = 0;
    const char* wordStart = nullptr;
    const char* wordEnd = nullptr;
    const char* possibleEnd = nullptr;
    do {
        if (wordStart && wordEnd) {
            if (!allLower || (!inQuotes && '\"' != lastCh && !inParens
                    && ')' != lastCh && !inAngles && '>' != lastCh)) {
                string word(wordStart, (possibleEnd ? possibleEnd : wordEnd) - wordStart);
                wordCheck(word);
            }
            wordStart = nullptr;
        }
        if (chPtr == end) {
            break;
        }
        switch (*chPtr) {
            case '>':
                if (isalpha(lastCh)) {
                    --inAngles;
                    SkASSERT(inAngles >= 0);
                }
                wordEnd = chPtr;
                break;
            case '(':
                ++inParens;
                possibleEnd = chPtr;
                break;
            case ')':
                --inParens;
                if ('(' == lastCh) {
                    wordEnd = chPtr + 1;
                } else {
                    wordEnd = chPtr;
                }
                SkASSERT(inParens >= 0 || fInStdOut);
                break;
            case '\"':
                inQuotes = !inQuotes;
                wordEnd = chPtr;
                SkASSERT(inQuotes == !wordStart);
                break;
            case 'A': case 'B': case 'C': case 'D': case 'E':
            case 'F': case 'G': case 'H': case 'I': case 'J':
            case 'K': case 'L': case 'M': case 'N': case 'O':
            case 'P': case 'Q': case 'R': case 'S': case 'T':
            case 'U': case 'V': case 'W': case 'X': case 'Y':
            case 'Z':
                allLower = false;
            case 'a': case 'b': case 'c': case 'd': case 'e':
            case 'f': case 'g': case 'h': case 'i': case 'j':
            case 'k': case 'l': case 'm': case 'n': case 'o':
            case 'p': case 'q': case 'r': case 's': case 't':
            case 'u': case 'v': case 'w': case 'x': case 'y':
            case 'z':
                if (!wordStart) {
                    wordStart = chPtr;
                    wordEnd = nullptr;
                    possibleEnd = nullptr;
                    allLower = 'a' <= *chPtr;
                    if ('<' == lastCh || ('<' == priorCh && '/' == lastCh)) {
                        ++inAngles;
                    }
                }
                break;
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
            case '_':
                allLower = false;
            case '-':  // note that dash doesn't clear allLower
                break;
            default:
                wordEnd = chPtr;
                break;
        }
        priorCh = lastCh;
        lastCh = *chPtr;
    } while (++chPtr <= end);
}

void SpellCheck::printCheck(const string& str) {
    string word;
    for (std::stringstream stream(str); stream >> word; ) {
        wordCheck(word);
    }
}

static bool stringCompare(const std::pair<string, CheckEntry>& i, const std::pair<string, CheckEntry>& j) {
    return i.first.compare(j.first) < 0;
}

void SpellCheck::report(SkCommandLineFlags::StringArray report) {
    vector<std::pair<string, CheckEntry>> elems(fWords.begin(), fWords.end());
    std::sort(elems.begin(), elems.end(), stringCompare);
    if (report.contains("once")) {
        for (auto iter : elems) {
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
                SkDebugf("%s(%d): %s\n", iter.second.fFile.c_str(), iter.second.fLine,
                        iter.first.c_str());
            }
        }
        SkDebugf("\n");
        return;
    }
    if (report.contains("all")) {
        int column = 0;
        char lastInitial = 'a';
        int count = 0;
        for (auto iter : elems) {
            if (string::npos != iter.second.fFile.find("undocumented.bmh")) {
                continue;
            }
            if (string::npos != iter.second.fFile.find("markup.bmh")) {
                continue;
            }
            if (string::npos != iter.second.fFile.find("usingBookmaker.bmh")) {
                continue;
            }
            string check = iter.first.c_str();
            bool allLower = true;
            for (auto c : check) {
                if (isupper(c)) {
                    allLower = false;
                    break;
                }
            }
            if (!allLower) {
                continue;
            }
            if (column + check.length() > 100 || check[0] != lastInitial) {
                SkDebugf("\n");
                column = 0;
            }
            if (check[0] != lastInitial) {
                SkDebugf("\n");
                lastInitial = check[0];
            }
            SkDebugf("%s ", check.c_str());
            column += check.length();
            ++count;
        }
        SkDebugf("\n\ncount = %d\n", count);
        return;
    }
    int index = 0;
    const char* mispelled = report[0];
    for (auto iter : elems) {
        if (string::npos != iter.second.fFile.find("undocumented.bmh")) {
            continue;
        }
        if (string::npos != iter.second.fFile.find("markup.bmh")) {
            continue;
        }
        if (string::npos != iter.second.fFile.find("usingBookmaker.bmh")) {
            continue;
        }
        string check = iter.first.c_str();
        while (check.compare(mispelled) > 0) {
            SkDebugf("%s not found\n", mispelled);
            if (report.count() == ++index) {
                break;
            }
        }
        if (report.count() == index) {
            break;
        }
        if (check.compare(mispelled) == 0) {
            SkDebugf("%s(%d): %s\n", iter.second.fFile.c_str(), iter.second.fLine,
                    iter.first.c_str());
            if (report.count() == ++index) {
                break;
            }
        }
    }
}

void SpellCheck::wordCheck(const string& str) {
    if ("nullptr" == str) {
        return;  // doesn't seem worth it, treating nullptr as a word in need of correction
    }
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
        bool isParen = '(' == ch || ')' == ch || '~' == ch || '=' == ch || '!' == ch ||
                '[' == ch || ']' == ch;
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
            if (token.length()) {
                this->wordCheck(token);
            }
        }
        return;
    }
    if (!hasColon && !hasDot && !hasParen && !hasUnderscore
            && !fInStdOut && !inCode && !fInConst && !sawDigit
            && islower(str[0]) && isupper(str[1])) {
        inCode = true;
    }
    bool methodParam = false;
    if (fMethod) {
        for (auto child : fMethod->fChildren) {
            if (MarkType::kParam == child->fMarkType && str == child->fName) {
                methodParam = true;
                break;
            }
        }
    }
    auto& mappy = hasColon ? fColons :
                  hasDot ? fDots :
                  hasParen ? fParens :
                  hasUnderscore ? fUnderscores :
                  fInStdOut || fInFormula || inCode || fInConst || methodParam ? fCode :
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
