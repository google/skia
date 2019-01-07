/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOSFile.h"
#include "SkOSPath.h"

#include "bmhParser.h"
#include "includeParser.h"

const IncludeKey kKeyWords[] = {
    { "",           KeyWord::kNone,         KeyProperty::kNone           },
    { "SK_API",     KeyWord::kSK_API,       KeyProperty::kModifier       },
    { "SK_BEGIN_REQUIRE_DENSE", KeyWord::kSK_BEGIN_REQUIRE_DENSE, KeyProperty::kModifier },
    { "alignas",    KeyWord::kAlignAs,      KeyProperty::kModifier       },
    { "bool",       KeyWord::kBool,         KeyProperty::kNumber         },
    { "char",       KeyWord::kChar,         KeyProperty::kNumber         },
    { "class",      KeyWord::kClass,        KeyProperty::kObject         },
    { "const",      KeyWord::kConst,        KeyProperty::kModifier       },
    { "constexpr",  KeyWord::kConstExpr,    KeyProperty::kModifier       },
    { "define",     KeyWord::kDefine,       KeyProperty::kPreprocessor   },
    { "double",     KeyWord::kDouble,       KeyProperty::kNumber         },
    { "elif",       KeyWord::kElif,         KeyProperty::kPreprocessor   },
    { "else",       KeyWord::kElse,         KeyProperty::kPreprocessor   },
    { "endif",      KeyWord::kEndif,        KeyProperty::kPreprocessor   },
    { "enum",       KeyWord::kEnum,         KeyProperty::kObject         },
    { "error",      KeyWord::kError,        KeyProperty::kPreprocessor   },
    { "float",      KeyWord::kFloat,        KeyProperty::kNumber         },
    { "friend",     KeyWord::kFriend,       KeyProperty::kModifier       },
    { "if",         KeyWord::kIf,           KeyProperty::kPreprocessor   },
    { "ifdef",      KeyWord::kIfdef,        KeyProperty::kPreprocessor   },
    { "ifndef",     KeyWord::kIfndef,       KeyProperty::kPreprocessor   },
    { "include",    KeyWord::kInclude,      KeyProperty::kPreprocessor   },
    { "inline",     KeyWord::kInline,       KeyProperty::kModifier       },
    { "int",        KeyWord::kInt,          KeyProperty::kNumber         },
    { "operator",   KeyWord::kOperator,     KeyProperty::kFunction       },
    { "private",    KeyWord::kPrivate,      KeyProperty::kClassSection   },
    { "protected",  KeyWord::kProtected,    KeyProperty::kClassSection   },
    { "public",     KeyWord::kPublic,       KeyProperty::kClassSection   },
    { "signed",     KeyWord::kSigned,       KeyProperty::kNumber         },
    { "size_t",     KeyWord::kSize_t,       KeyProperty::kNumber         },
    { "static",     KeyWord::kStatic,       KeyProperty::kModifier       },
    { "struct",     KeyWord::kStruct,       KeyProperty::kObject         },
    { "template",   KeyWord::kTemplate,     KeyProperty::kObject         },
    { "typedef",    KeyWord::kTypedef,      KeyProperty::kObject         },
    { "typename",   KeyWord::kTypename,     KeyProperty::kObject         },
    { "uint16_t",   KeyWord::kUint16_t,     KeyProperty::kNumber         },
    { "uint32_t",   KeyWord::kUint32_t,     KeyProperty::kNumber         },
    { "uint64_t",   KeyWord::kUint64_t,     KeyProperty::kNumber         },
    { "uint8_t",    KeyWord::kUint8_t,      KeyProperty::kNumber         },
    { "uintptr_t",  KeyWord::kUintPtr_t,    KeyProperty::kNumber         },
    { "union",      KeyWord::kUnion,        KeyProperty::kObject         },
    { "unsigned",   KeyWord::kUnsigned,     KeyProperty::kNumber         },
    { "using",      KeyWord::kUsing,        KeyProperty::kObject         },
    { "void",       KeyWord::kVoid,         KeyProperty::kNumber         },
};

const size_t kKeyWordCount = SK_ARRAY_COUNT(kKeyWords);

KeyWord IncludeParser::FindKey(const char* start, const char* end) {
    int ch = 0;
    for (size_t index = 0; index < kKeyWordCount; ) {
        if (start[ch] > kKeyWords[index].fName[ch]) {
            ++index;
            if (ch > 0 && (index == kKeyWordCount ||
                    kKeyWords[index - 1].fName[ch - 1] < kKeyWords[index].fName[ch - 1])) {
                return KeyWord::kNone;
            }
            continue;
        }
        if (start[ch] < kKeyWords[index].fName[ch]) {
            return KeyWord::kNone;
        }
        ++ch;
        if (start + ch >= end) {
            if (end - start < (int) strlen(kKeyWords[index].fName)) {
                return KeyWord::kNone;
            }
            return kKeyWords[index].fKeyWord;
        }
    }
    return KeyWord::kNone;
}

void IncludeParser::ValidateKeyWords() {
    for (size_t index = 1; index < kKeyWordCount; ++index) {
        SkASSERT((int) kKeyWords[index - 1].fKeyWord + 1
                == (int) kKeyWords[index].fKeyWord);
        SkASSERT(0 > strcmp(kKeyWords[index - 1].fName, kKeyWords[index].fName));
    }
}

void IncludeParser::addKeyword(KeyWord keyWord) {
    fParent->fTokens.emplace_back(keyWord, fIncludeWord, fChar, fLineCount, fParent, '\0');
    fIncludeWord = nullptr;
    if (KeyProperty::kObject == kKeyWords[(int) keyWord].fProperty) {
        Definition* def = &fParent->fTokens.back();
        this->addDefinition(def);
        if (KeyWord::kEnum == fParent->fKeyWord) {
            fInEnum = true;
        }
    }
}

static bool looks_like_method(const TextParser& tp) {
    TextParser t(tp.fFileName, tp.fLine, tp.fChar, tp.fLineCount);
    t.skipSpace();
    if (!t.skipExact("struct") && !t.skipExact("class") && !t.skipExact("enum class")
            && !t.skipExact("enum")) {
        return true;
    }
    t.skipSpace();
    if (t.skipExact("SK_API")) {
        t.skipSpace();
    }
    if (!isupper(t.peek())) {
        return true;
    }
    return nullptr != t.strnchr('(', t.fEnd);
}

static bool looks_like_forward_declaration(const TextParser& tp) {
    TextParser t(tp.fFileName, tp.fChar, tp.lineEnd(), tp.fLineCount);
    t.skipSpace();
    if (!t.skipExact("struct") && !t.skipExact("class") && !t.skipExact("enum class")
            && !t.skipExact("enum")) {
        return false;
    }
    t.skipSpace();
    if (t.skipExact("SK_API")) {
        t.skipSpace();
    }
    if (!isupper(t.peek())) {
        return false;
    }
    t.skipToNonAlphaNum();
    if (t.eof() || ';' != t.next()) {
        return false;
    }
    if (t.eof() || '\n' != t.next()) {
        return false;
    }
    return t.eof();
}

static bool looks_like_constructor(const TextParser& tp) {
    TextParser t(tp.fFileName, tp.fLine, tp.lineEnd(), tp.fLineCount);
    t.skipSpace();
    if (!isupper(t.peek())) {
        if (':' == t.next() && ' ' >= t.peek()) {
            return true;
        }
        return false;
    }
    t.skipToNonAlphaNum();
    if ('(' != t.peek()) {
        return false;
    }
    if (!t.skipToEndBracket(')')) {
        return false;
    }
    SkAssertResult(')' == t.next());
    t.skipSpace();
    return tp.fChar == t.fChar;
}

static bool looks_like_class_decl(const TextParser& tp) {
    TextParser t(tp.fFileName, tp.fLine, tp.fChar, tp.fLineCount);
    t.skipSpace();
    if (!t.skipExact("class")) {
        return false;
    }
    t.skipSpace();
    if (t.skipExact("SK_API")) {
        t.skipSpace();
    }
    if (!isupper(t.peek())) {
        return false;
    }
    t.skipToNonAlphaNum();
    return !t.skipToEndBracket('(');
}

static bool looks_like_const(const TextParser& tp) {
    TextParser t(tp.fFileName, tp.fChar, tp.lineEnd(), tp.fLineCount);
    if (!t.startsWith("static constexpr ")) {
        return false;
    }
    if (t.skipToEndBracket(" k")) {
        SkAssertResult(t.skipExact(" k"));
    } else if (t.skipToEndBracket(" SK_")) {
        SkAssertResult(t.skipExact(" SK_"));
    } else {
        return false;
    }
    if (!isupper(t.peek())) {
        return false;
    }
    return t.skipToEndBracket(" = ");
}

static bool looks_like_member(const TextParser& tp) {
    TextParser t(tp.fFileName, tp.fChar, tp.lineEnd(), tp.fLineCount);
    const char* end = t.anyOf("(;");
    if (!end || '(' == *end) {
        return false;
    }
    bool foundMember = false;
    do {
        const char* next = t.anyOf(" ;");
        if (';' == *next) {
            break;
        }
        t.skipTo(next);
        t.skipSpace();
        foundMember = 'f' == t.fChar[0] && isupper(t.fChar[1]);
    } while (true);
    return foundMember;
}

static void skip_constructor_initializers(TextParser& t) {
    SkAssertResult(':' == t.next());
    do {
        t.skipWhiteSpace();
        t.skipToNonAlphaNum();
        t.skipWhiteSpace();
        if ('{' == t.peek()) {
            t.skipToBalancedEndBracket('{', '}');
        }
        do {
            const char* limiter = t.anyOf("(,{");
            t.skipTo(limiter);
            if ('(' != t.peek()) {
                break;
            }
            t.skipToBalancedEndBracket('(', ')');
        } while (true);
        if ('{' == t.peek()) {
            return;
        }
        SkAssertResult(',' == t.next());
    } while (true);
}

static const char kInline[] = "inline ";
static const char kSK_API[] = "SK_API ";
static const char kSK_WARN_UNUSED_RESULT[] = "SK_WARN_UNUSED_RESULT ";

bool IncludeParser::advanceInclude(TextParser& i) {
    if (!i.skipWhiteSpace(&fCheck.fIndent, &fCheck.fWriteReturn)) {
        return false;
    }
    if (fCheck.fPrivateBrace) {
        if (i.startsWith("};")) {
            if (fCheck.fPrivateBrace == fCheck.fBraceCount) {
                fCheck.fPrivateBrace = 0;
                fCheck.fDoubleReturn = true;
            } else {
                i.skipExact("};");
                fCheck.fBraceCount -= 1;
            }
            return false;
        }
        if (i.startsWith("public:")) {
            if (fCheck.fBraceCount <= fCheck.fPrivateBrace) {
                fCheck.fPrivateBrace = 0;
                if (fCheck.fPrivateProtected) {
                    i.skipExact("public:");
                }
            } else {
                i.skipExact("public:");
            }
        } else {
            fCheck.fBraceCount += i.skipToLineBalance('{', '}');
        }
        return false;
    } else if (i.startsWith("};")) {
        fCheck.fDoubleReturn = 2;
    }
    if (i.skipExact(kInline)) {
        fCheck.fSkipInline = true;
        return false;
    }
    if (i.skipExact(kSK_API)) {
        fCheck.fSkipAPI = true;
        return false;
    }
    if (i.skipExact(kSK_WARN_UNUSED_RESULT)) {
        fCheck.fSkipWarnUnused = true;
        return false;
    }
    if (i.skipExact("SK_ATTR_DEPRECATED")) {
        i.skipToLineStart(&fCheck.fIndent, &fCheck.fWriteReturn);
        return false;
    }
    if (i.skipExact("SkDEBUGCODE")) {
        i.skipWhiteSpace();
        if ('(' != i.peek()) {
            i.reportError("expected open paren");
        }
        TextParserSave save(&i);
        SkAssertResult(i.skipToBalancedEndBracket('(', ')'));
        fCheck.fInDebugCode = i.fChar - 1;
        save.restore();
        SkAssertResult('(' == i.next());
    }
    if ('{' == i.peek()) {
        if (looks_like_method(i)) {
            fCheck.fState = CheckCode::State::kMethod;
            bool inBalance = false;
            TextParser paren(i.fFileName, i.fStart, i.fEnd, i.fLineCount);
            paren.skipToEndBracket('(');
            paren.skipToBalancedEndBracket('(', ')');
            inBalance = i.fChar < paren.fChar;
            if (!inBalance) {
                if (!i.skipToBalancedEndBracket('{', '}')) {
                    i.reportError("unbalanced open brace");
                }
                i.skipToLineStart(&fCheck.fIndent, &fCheck.fWriteReturn);
                return false;
            }
        } else if (looks_like_class_decl(i)) {
            fCheck.fState = CheckCode::State::kClassDeclaration;
            fCheck.fPrivateBrace = fCheck.fBraceCount + 1;
            fCheck.fPrivateProtected = false;
        }
    }
    if (':' == i.peek() && looks_like_constructor(i)) {
        fCheck.fState = CheckCode::State::kConstructor;
        skip_constructor_initializers(i);
        return false;
    }
    if ('#' == i.peek()) {
        i.skipToLineStart(&fCheck.fIndent, &fCheck.fWriteReturn);
        return false;
    }
    if (i.startsWith("//")) {
        i.skipToLineStart(&fCheck.fIndent, &fCheck.fWriteReturn);
        return false;
    }
    if (i.startsWith("/*")) {
        i.skipToEndBracket("*/");
        i.skipToLineStart(&fCheck.fIndent, &fCheck.fWriteReturn);
        return false;
    }
    if (looks_like_forward_declaration(i)) {
        fCheck.fState = CheckCode::State::kForwardDeclaration;
        i.skipToLineStart(&fCheck.fIndent, &fCheck.fWriteReturn);
        return false;
    }
    if (i.skipExact("private:") || i.skipExact("protected:")) {
        if (!fCheck.fBraceCount) {
            i.reportError("expect private in brace");
        }
        fCheck.fPrivateBrace = fCheck.fBraceCount;
        fCheck.fPrivateProtected = true;
        return false;
    }
    const char* funcEnd = i.anyOf("(\n");
    if (funcEnd && '(' == funcEnd[0] && '_' == *i.anyOf("_(")
            && (i.contains("internal_", funcEnd, nullptr)
            || i.contains("private_", funcEnd, nullptr)
            || i.contains("legacy_", funcEnd, nullptr)
            || i.contains("temporary_", funcEnd, nullptr))) {
        i.skipTo(funcEnd);
        if (!i.skipToBalancedEndBracket('(', ')')) {
            i.reportError("unbalanced open parent");
        }
        i.skipSpace();
        i.skipExact("const ");
        i.skipSpace();
        if (';' == i.peek()) {
            i.next();
        }
        fCheck.fState = CheckCode::State::kNone;
        return false;
    }
    return true;
}

void IncludeParser::codeBlockAppend(string& result, string s) const {
    for (char c : s) {
        this->codeBlockAppend(result, c);
    }
}

void IncludeParser::codeBlockAppend(string& result, char ch) const {
    if (Elided::kYes == fElided && fCheck.fBraceCount) {
        return;
    }
    this->stringAppend(result, ch);
}

void IncludeParser::codeBlockSpaces(string& result, int indent) const {
    if (!indent) {
        return;
    }
    if (Elided::kYes == fElided && fCheck.fBraceCount) {
        return;
    }
    SkASSERT(indent > 0);
    if (fDebugWriteCodeBlock) {
        SkDebugf("%*c", indent, ' ');
    }
    result.append(indent, ' ');
}

string IncludeParser::writeCodeBlock(const Definition& iDef) {
    if (MarkType::kComment == iDef.fMarkType) {
        return "";
    }
    if (iDef.fUndocumented) {
        return "";
    }
    TextParser i(&iDef);
    (void) i.skipExact("SkDEBUGCODE(");
    if (MarkType::kConst == iDef.fMarkType && !i.fEnd) {
        // TODO: end should have been set earlier
        auto iter = iDef.fParent->fTokens.begin();
        std::advance(iter, iDef.fParentIndex + 1);
        SkASSERT(iter != iDef.fParent->fTokens.end());
        i.fEnd = iter->fContentStart;
    }
    const char* loc;
    if (MarkType::kMember == iDef.fMarkType) {
        const char* parentEnd = iDef.fParent->fContentEnd;
        TextParser newEnd(&iDef);
        newEnd.fEnd = parentEnd;
        const char* memberEnd = newEnd.anyOf(",};");
        if (memberEnd && (';' == memberEnd[0] || ',' == memberEnd[0])) {
            i.fEnd = memberEnd + 1;
        }
    }
    if (i.contains("//", i.fEnd, &loc)) {
        i.fEnd = loc;
    }
    if (i.contains("/*", i.fEnd, &loc)) {
        i.fEnd = loc;
    }
    if (i.contains("{", i.fEnd, &loc)) {
        bool inBalance = false;
        if (MarkType::kMethod == iDef.fMarkType) {
            TextParser paren(&iDef);
            paren.skipToEndBracket('(');
            paren.skipToBalancedEndBracket('(', ')');
            inBalance = loc < paren.fChar;
        }
        if (!inBalance) {
            i.fEnd = loc + 1;
            while (i.fEnd < iDef.fContentEnd && ' ' >= i.fEnd[0]) {
                ++i.fEnd;
            }
        }
    }
    while (i.fEnd > i.fStart && ' ' == i.fEnd[-1]) {
        --i.fEnd;
    }
    const char* before = iDef.fContentStart;
    while (' ' == *--before)
        ;
    int startIndent = iDef.fContentStart - before - 1;
    bool saveDebugWriteBlock = fDebugWriteCodeBlock;
    fDebugWriteCodeBlock = false;
    string result = writeCodeBlock(i, iDef.fMarkType, startIndent);
    fDebugWriteCodeBlock = saveDebugWriteBlock;
    if (!result.empty()) {
        if (MarkType::kNone != fPreviousMarkType && iDef.fMarkType != fPreviousMarkType
                && ((MarkType::kEnum != fPreviousMarkType
                && MarkType::kEnumClass != fPreviousMarkType)
                || MarkType::kMember != iDef.fMarkType)
                && (MarkType::kMember != fPreviousMarkType
                || iDef.fParent == fPreviousDef->fParent)) {
            result = "\n" + result;
        }
        if (fDebugWriteCodeBlock) {
            SkDebugf("%s", result.c_str());
        }
        fPreviousDef = &iDef;
        fPreviousMarkType = iDef.fMarkType;
    }
    for (auto& token : iDef.fTokens) {
        result += this->writeCodeBlock(token);
    }
    if (MarkType::kEnum == iDef.fMarkType || MarkType::kEnumClass == iDef.fMarkType
            || MarkType::kStruct == iDef.fMarkType || MarkType::kClass == iDef.fMarkType) {
        this->codeBlockSpaces(result, startIndent);
        this->codeBlockAppend(result, "};\n\n");
    }
    return result;
}

string IncludeParser::writeCodeBlock(TextParser& i, MarkType markType, int startIndent) {
    string result;
    char last;
    int lastIndent = 0;
    bool lastDoubleMeUp = false;
    fCheck.reset();
    if (MarkType::kDefine == markType) {
        result = "#define ";
    } else {
        this->codeBlockSpaces(result, startIndent);
    }
    do {
        if (!this->advanceInclude(i)) {
            continue;
        }
        do {
            last = i.peek();
            SkASSERT(' ' < last);
            if (fCheck.fInDebugCode == i.fChar) {
                fCheck.fInDebugCode = nullptr;
                i.next();   // skip close paren
                break;
            }
            if (CheckCode::State::kMethod == fCheck.fState) {
                this->codeBlockAppend(result, ';');
                fCheck.fState = CheckCode::State::kNone;
            }
            if (fCheck.fWriteReturn) {
                this->codeBlockAppend(result, '\n');
                bool doubleMeUp = i.startsWith("typedef ") || looks_like_const(i)
                        || (!strncmp("struct ", i.fStart, 7) && looks_like_member(i));
                if ((!--fCheck.fDoubleReturn && !i.startsWith("};")) || i.startsWith("enum ")
                        || i.startsWith("typedef ") || doubleMeUp || fCheck.fTypedefReturn
                        || (fCheck.fIndent && (i.startsWith("class ") || i.startsWith("struct ")))) {
                    if (lastIndent > 0 && (!doubleMeUp || !lastDoubleMeUp)) {
                        this->codeBlockAppend(result, '\n');
                    }
                    fCheck.fTypedefReturn = false;
                    lastDoubleMeUp = doubleMeUp;
                }
                if (doubleMeUp) {
                    fCheck.fTypedefReturn = true;
                }
                lastIndent = fCheck.fIndent;
            }
            if (fCheck.fIndent) {
                size_t indent = fCheck.fIndent;
                if (fCheck.fSkipInline && indent > sizeof(kInline)) {
                    indent -= sizeof(kInline) - 1;
                }
                if (fCheck.fSkipAPI && indent > sizeof(kSK_API)) {
                    indent -= sizeof(kSK_API) - 1;
                }
                if (fCheck.fSkipWarnUnused && indent > sizeof(kSK_WARN_UNUSED_RESULT)) {
                    indent -= sizeof(kSK_WARN_UNUSED_RESULT) - 1;
                }

                this->codeBlockSpaces(result, indent);
            }
            this->codeBlockAppend(result, last);
            fCheck.fWriteReturn = false;
            fCheck.fIndent = 0;
            fCheck.fBraceCount += '{' == last;
            fCheck.fBraceCount -= '}' == last;
            if (';' == last) {
                fCheck.fSkipInline = false;
                fCheck.fSkipAPI = false;
                fCheck.fSkipWarnUnused = false;
            }
            if (fCheck.fBraceCount < 0) {
                i.reportError("unbalanced close brace");
                return result;
            }
            i.next();
        } while (!i.eof() && ' ' < i.peek() && !i.startsWith("//"));
    } while (!i.eof());
    if (CheckCode::State::kMethod == fCheck.fState) {
        this->codeBlockAppend(result, ';');
    }
    bool elided = Elided::kYes == fElided;
    bool elidedTemplate = elided && !strncmp(i.fStart, "template ", 9);
    bool elidedTClass = elidedTemplate && MarkType::kClass == markType;
    bool statementEnd = !result.empty() && (MarkType::kMethod == markType
            || MarkType::kTypedef == markType || '}' == result.back());
    bool semiEnd = !result.empty() && (',' == result.back() || ';' == result.back());
    if (fCheck.fWriteReturn || elidedTClass) {
        this->codeBlockAppend(result, '\n');
    }
    if (elided && ((MarkType::kFunction != markType && lastIndent > startIndent) || elidedTClass)) {
        this->codeBlockAppend(result, '}');
        statementEnd = true;
    }
    if (elided || statementEnd) {
        this->codeBlockAppend(result, ";\n");
    } else if (elidedTemplate || semiEnd) {
        this->codeBlockAppend(result, '\n');
    }
    return result;
}

void IncludeParser::checkForMissingParams(const vector<string>& methodParams,
        const vector<string>& foundParams) {
    for (auto& methodParam : methodParams) {
        bool found = false;
        for (auto& foundParam : foundParams) {
            if (methodParam == foundParam) {
                found = true;
                break;
            }
        }
        if (!found) {
            this->writeIncompleteTag("Param", methodParam, 2);
        }
    }
    for (auto& foundParam : foundParams) {
        bool found = false;
        for (auto& methodParam : methodParams) {
            if (methodParam == foundParam) {
                found = true;
                break;
            }
        }
        if (!found) {
            this->reportError("doxygen param does not match method declaration");
        }
    }
}

bool IncludeParser::checkForWord() {
    if (!fIncludeWord) {
        return true;
    }
    KeyWord keyWord = FindKey(fIncludeWord, fChar);
    if (KeyWord::kClass == keyWord || KeyWord::kStruct == keyWord) {
        Bracket bracket = this->topBracket();
        if (Bracket::kParen == bracket) {
            return true;
        }
    }
    if (KeyWord::kNone != keyWord) {
        if (KeyProperty::kPreprocessor != kKeyWords[(int) keyWord].fProperty) {
            this->addKeyword(keyWord);
            return true;
        }
    } else {
        this->addWord();
        return true;
    }
    Definition* poundDef = fParent;
    if (!fParent) {
        return reportError<bool>("expected parent");
    }
    if (Definition::Type::kBracket != poundDef->fType) {
        return reportError<bool>("expected bracket");
    }
    if (Bracket::kPound != poundDef->fBracket) {
        return reportError<bool>("expected preprocessor");
    }
    if (KeyWord::kNone != poundDef->fKeyWord) {
        return reportError<bool>("already found keyword");
    }
    poundDef->fKeyWord = keyWord;
    fIncludeWord = nullptr;
    switch (keyWord) {
        // these do not link to other # directives
        case KeyWord::kDefine:
            if (!fInBrace) {
                SkASSERT(!fInDefine);
                fInDefine = true;
            }
        case KeyWord::kInclude:
        case KeyWord::kError:
        break;
        // these start a # directive link
        case KeyWord::kIf:
        case KeyWord::kIfdef:
        case KeyWord::kIfndef:
        break;
        // these continue a # directive link
        case KeyWord::kElif:
        case KeyWord::kElse:
            this->popObject();  // pop elif
            if (Bracket::kPound != fParent->fBracket) {
                return this->reportError<bool>("expected preprocessor directive");
            }
            this->popBracket();  // pop if
            poundDef->fParent = fParent;
            fParent = poundDef;  // push elif back
        break;
        // this ends a # directive link
        case KeyWord::kEndif:
        // FIXME : should this be calling popBracket() instead?
            this->popObject();  // pop endif
            if (Bracket::kPound != fParent->fBracket) {
                return this->reportError<bool>("expected preprocessor directive");
            }
            this->popBracket();  // pop if/else
        break;
        default:
            SkASSERT(0);
    }
    return true;
}

string IncludeParser::className() const {
    string name(fParent->fName);
    size_t slash = name.find_last_of("/");
    if (string::npos == slash) {
        slash = name.find_last_of("\\");
    }
    SkASSERT(string::npos != slash);
    string result = name.substr(slash);
    result = result.substr(1, result.size() - 3);
    return result;
}

void IncludeParser::writeCodeBlock() {
    fElided = Elided::kNo;
    for (auto& classMapper : fIClassMap) {
        fPreviousMarkType = MarkType::kNone;
        fPreviousDef = nullptr;
        classMapper.second.fCode = this->writeCodeBlock(classMapper.second);
    }
    for (auto& enumMapper : fIEnumMap) {
        fPreviousMarkType = MarkType::kNone;
        fPreviousDef = nullptr;
        enumMapper.second->fCode = this->writeCodeBlock(*enumMapper.second);
    }
    for (auto& typedefMapper : fITypedefMap) {
        fPreviousMarkType = MarkType::kNone;
        fPreviousDef = nullptr;
        typedefMapper.second->fCode = this->writeCodeBlock(*typedefMapper.second);
    }
    for (auto& defineMapper : fIDefineMap) {
        fPreviousMarkType = MarkType::kNone;
        fPreviousDef = nullptr;
        defineMapper.second->fCode = this->writeCodeBlock(*defineMapper.second);
    }
}

void IncludeParser::checkName(Definition* def) {
    SkASSERT(!def->fName.empty());
    TextParser parser(def->fFileName, &def->fName.front(), &def->fName.back() + 1, def->fLineCount);
    const vector<string> skipWords = { "deprecated", "experimental", "internal",  "private",
            "legacy", "temporary" };
    if (!parser.anyWord(skipWords, 0).empty()) {
        def->fUndocumented = true;
    }
}

#include <sstream>
#include <iostream>

void IncludeParser::checkTokens(list<Definition>& tokens, string key, string className,
        RootDefinition* root, BmhParser& bmhParser) {
    for (const auto& token : tokens) {
        if (token.fPrivate) {
            continue;
        }
        string fullName = key + "::" + token.fName;
        const Definition* def = nullptr;
        if (root) {
            def = root->find(fullName, RootDefinition::AllowParens::kYes);
        }
        switch (token.fMarkType) {
            case MarkType::kMethod: {
                if (this->isInternalName(token)) {
                    continue;
                }
                if (!root) {
                    if (token.fUndocumented) {
                        break;
                    }
                    auto methIter = bmhParser.fMethodMap.find(token.fName);
                    if (bmhParser.fMethodMap.end() != methIter) {
                        def = &methIter->second;
                        if (def->crossCheck2(token)) {
                            def->fVisited = true;
                        } else {
                            this->suggestFix(Suggest::kMethodDiffers, token, root, def);
                            fFailed = true;
                        }
                    } else {
                        this->suggestFix(Suggest::kMethodMissing, token, root, nullptr);
                        fFailed = true;
                    }
                    break;
                }
                if (!def) {
                    string paramName = className + "::";
                    paramName += string(token.fContentStart,
                            token.fContentEnd - token.fContentStart);
                    if (string::npos != paramName.find('\n')) {
                        paramName.erase(std::remove(paramName.begin(), paramName.end(), '\n'),
                                paramName.end());
                    }
                    def = root->find(paramName, RootDefinition::AllowParens::kYes);
                    if (!def && 0 == token.fName.find("operator")) {
                        string operatorName = className + "::";
                        TextParser oper("", token.fStart, token.fContentEnd, 0);
                        const char* start = oper.strnstr("operator", token.fContentEnd);
                        SkASSERT(start);
                        oper.skipTo(start);
                        oper.skipToEndBracket('(');
                        int parens = 0;
                        do {
                            if ('(' == oper.peek()) {
                                ++parens;
                            } else if (')' == oper.peek()) {
                                --parens;
                            }
                        } while (!oper.eof() && oper.next() && parens > 0);
                        operatorName += string(start, oper.fChar - start);
                        def = root->find(operatorName, RootDefinition::AllowParens::kYes);
                    }
                }
                if (!def) {
                    int skip = !strncmp(token.fContentStart, "explicit ", 9) ? 9 : 0;
                    skip = !strncmp(token.fContentStart, "virtual ", 8) ? 8 : skip;
                    const char* tokenEnd = token.methodEnd();
                    string constructorName = className + "::";
                    constructorName += string(token.fContentStart + skip,
                            tokenEnd - token.fContentStart - skip);
                    def = root->find(constructorName, RootDefinition::AllowParens::kYes);
                }
                if (!def && 0 == token.fName.find("SK_")) {
                    string incName = token.fName + "()";
                    string macroName = className + "::" + incName;
                    def = root->find(macroName, RootDefinition::AllowParens::kYes);
                    if (def) {
                        if (def->fName == incName) {
                            def->fVisited = true;
                            if ("SK_TO_STRING_NONVIRT" == token.fName) {
                                def = root->find(className + "::toString",
                                        RootDefinition::AllowParens::kYes);
                                if (def) {
                                    def->fVisited = true;
                                } else {
                                    SkDebugf("missing toString bmh: %s\n", fullName.c_str());
                                    fFailed = true;
                                }
                            }
                            break;
                        } else {
                            SkDebugf("method macro differs from bmh: %s\n", fullName.c_str());
                            fFailed = true;
                        }
                    }
                }
                if (!def) {
                    bool allLower = true;
                    for (size_t index = 0; index < token.fName.length(); ++index) {
                        if (!islower(token.fName[index])) {
                            allLower = false;
                            break;
                        }
                    }
                    if (allLower) {
                        string lowerName = className + "::" + token.fName + "()";
                        def = root->find(lowerName, RootDefinition::AllowParens::kYes);
                    }
                }
                if (!def) {
                    if (0 == token.fName.find("SkDEBUGCODE")) {
                        break;
                    }
                }
                if (!def) {
        // simple method names inside nested classes have a bug and are missing trailing parens
                    string withParens = fullName + "()"; // FIXME: this shouldn't be necessary
                    def = root->find(withParens, RootDefinition::AllowParens::kNo);
                }
                if (!def) {
                    if (!token.fUndocumented) {
                        this->suggestFix(Suggest::kMethodMissing, token, root, nullptr);
                        fFailed = true;
                    }
                    break;
                }
                if (token.fUndocumented) {
                    // we can't report an error yet; if bmh documents this unnecessarily,
                    // we'll detect that later. It may be that def points to similar
                    // documented function.
                    break;
                }
                if (def->crossCheck2(token)) {
                    def->fVisited = true;
                } else {
                    SkDebugf("method differs from bmh: %s\n", fullName.c_str());
                    fFailed = true;
                }
            } break;
            case MarkType::kComment:
                break;
            case MarkType::kEnumClass:
            case MarkType::kEnum: {
                if (!def) {
                    // work backwards from first word to deduce #Enum name
                    TextParser firstMember("", token.fStart, token.fContentEnd, 0);
                    SkAssertResult(firstMember.skipName("enum"));
                    SkAssertResult(firstMember.skipToEndBracket('{'));
                    firstMember.next();
                    firstMember.skipWhiteSpace();
                    SkASSERT('k' == firstMember.peek());
                    const char* savePos = firstMember.fChar;
                    firstMember.skipToNonName();
                    const char* wordEnd = firstMember.fChar;
                    firstMember.fChar = savePos;
                    const char* lastUnderscore = nullptr;
                    do {
                        if (!firstMember.skipToEndBracket('_')) {
                            break;
                        }
                        if (firstMember.fChar > wordEnd) {
                            break;
                        }
                        lastUnderscore = firstMember.fChar;
                    } while (firstMember.next());
                    if (lastUnderscore) {
                        ++lastUnderscore;
                        string enumName(lastUnderscore, wordEnd - lastUnderscore);
                        if (root) {
                            string anonName = className + "::" + enumName + 's';
                            def = root->find(anonName, RootDefinition::AllowParens::kYes);
                        } else {
                            auto enumIter = bmhParser.fEnumMap.find(enumName);
                            if (bmhParser.fEnumMap.end() != enumIter) {
                                RootDefinition* rootDef = &enumIter->second;
                                def = rootDef;
                            }
                        }
                    }
                    if (!def && !root) {
                        auto enumIter = bmhParser.fEnumMap.find(token.fName);
                        if (bmhParser.fEnumMap.end() != enumIter) {
                            def = &enumIter->second;
                        }
                        if (!def) {
                            auto enumClassIter = bmhParser.fClassMap.find(token.fName);
                            if (bmhParser.fClassMap.end() != enumClassIter) {
                                def = &enumClassIter->second;
                            }
                        }
                    }
                    if (!def) {
                        if (!token.fUndocumented) {
                            SkDebugf("enum missing from bmh: %s\n", fullName.c_str());
                            fFailed = true;
                        }
                        break;
                    }
                }
                def->fVisited = true;
                bool hasCode = false;
                bool hasPopulate = true;
                for (auto& child : def->fChildren) {
                    if (MarkType::kCode == child->fMarkType) {
                        hasPopulate = std::any_of(child->fChildren.begin(),
                                child->fChildren.end(), [](auto grandChild){
                                return MarkType::kPopulate == grandChild->fMarkType; });
                        if (!hasPopulate) {
                            def = child;
                        }
                        hasCode = true;
                        break;
                    }
                }
                if (!hasCode && !root) {
                    const Definition* topic = def->topicParent();
                    hasCode = std::any_of(topic->fChildren.begin(), topic->fChildren.end(),
                            [](Definition* def){ return MarkType::kCode == def->fMarkType
                            && def->fChildren.size() > 0 && MarkType::kPopulate ==
                            def->fChildren.front()->fMarkType; });
                }
                if (!hasCode) {
                    SkDebugf("enum code missing from bmh: %s\n", fullName.c_str());
                    fFailed = true;
                    break;
                }
                if (!hasPopulate) {
                    if (def->crossCheck(token)) {
                        def->fVisited = true;
                    } else {
                        SkDebugf("enum differs from bmh: %s\n", def->fName.c_str());
                        fFailed = true;
                    }
                }
                for (auto& member : token.fTokens) {
                    if (MarkType::kMember != member.fMarkType) {
                        continue;
                    }
                    string constName = MarkType::kEnumClass == token.fMarkType ?
                            fullName : className;
                    if (root) {
                        constName += "::" + member.fName;
                        def = root->find(constName, RootDefinition::AllowParens::kYes);
                    } else {
                        auto enumMapper = bmhParser.fEnumMap.find(token.fName);
                        if (bmhParser.fEnumMap.end() != enumMapper) {
                            auto& enumDoc = enumMapper->second;
                            auto memberIter = enumDoc.fLeaves.find(member.fName);
                            if (enumDoc.fLeaves.end() != memberIter) {
                                def = &memberIter->second;
                            }
                        }
                    }
                    if (!def) {
                        string innerName = key + "::" + member.fName;
                        def = root->find(innerName, RootDefinition::AllowParens::kYes);
                    }
                    if (!def) {
                        if (!member.fUndocumented) {
                            SkDebugf("const missing from bmh: %s\n", constName.c_str());
                            fFailed = true;
                        }
                    } else {
                        def->fVisited = true;
                    }
                }
                } break;
            case MarkType::kMember:
                if (def) {
                    def->fVisited = true;
                } else {
                    SkDebugf("member missing from bmh: %s\n", fullName.c_str());
                    fFailed = true;
                }
                break;
            case MarkType::kTypedef:
                if (!def && !root) {
                    auto typedefIter = bmhParser.fTypedefMap.find(token.fName);
                    if (bmhParser.fTypedefMap.end() != typedefIter) {
                        def = &typedefIter->second;
                    }
                }
                if (def) {
                    def->fVisited = true;
                } else {
                    SkDebugf("typedef missing from bmh: %s\n", fullName.c_str());
                    fFailed = true;
                }
                break;
            case MarkType::kConst:
                if (!def && !root) {
                    auto constIter = bmhParser.fConstMap.find(token.fName);
                    if (bmhParser.fConstMap.end() != constIter) {
                        def = &constIter->second;
                    }
                }
                if (def) {
                    def->fVisited = true;
                } else {
                    if (!token.fUndocumented) {
                        SkDebugf("const missing from bmh: %s\n", fullName.c_str());
                        fFailed = true;
                    }
                }
                break;
            case MarkType::kDefine:
                // TODO: incomplete
                break;
            default:
                SkASSERT(0);  // unhandled
                break;
        }
    }
}

bool IncludeParser::crossCheck(BmhParser& bmhParser) {
    for (auto& classMapper : fIClassMap) {
        string className = classMapper.first;
        auto finder = bmhParser.fClassMap.find(className);
        if (bmhParser.fClassMap.end() == finder) {
            SkASSERT(string::npos != className.find("::"));
            continue;
        }
    }
    for (auto& classMapper : fIClassMap) {
        if (classMapper.second.fUndocumented) {
           continue;
        }
        string className = classMapper.first;
        std::istringstream iss(className);
        string classStr;
        string classBase;
        RootDefinition* root = nullptr;
        while (std::getline(iss, classStr, ':')) {
            if (root) {
                if (!classStr.length()) {
                    continue;
                }
                classBase += "::" + classStr;
                auto finder = root->fBranches.find(classBase);
                if (root->fBranches.end() != finder) {
                    root = finder->second;
                } else {
                    SkASSERT(0);
                }
            } else {
                classBase = classStr;
                auto finder = bmhParser.fClassMap.find(classBase);
                if (bmhParser.fClassMap.end() != finder) {
                    root = &finder->second;
                } else {
                    SkASSERT(0);
                }
            }
        }
        this->checkTokens(classMapper.second.fTokens, classMapper.first, className, root,
                bmhParser);
    }
    this->checkTokens(fGlobals, "", "", nullptr, bmhParser);
    int crossChecks = 0;
    string firstCheck;
    for (auto& classMapper : fIClassMap) {
        string className = classMapper.first;
        auto finder = bmhParser.fClassMap.find(className);
        if (bmhParser.fClassMap.end() == finder) {
            continue;
        }
        RootDefinition* root = &finder->second;
        if (!root->dumpUnVisited()) {
            fFailed = true;
        }
        if (crossChecks) {
            SkDebugf(".");
        } else {
            SkDebugf("cross-check");
            firstCheck = className;
        }
        ++crossChecks;
    }
    if (crossChecks) {
        if (1 == crossChecks) {
            SkDebugf(" %s", firstCheck.c_str());
        }
        SkDebugf("\n");
    }
    bmhParser.fWroteOut = true;
    return !fFailed;
}

IClassDefinition* IncludeParser::defineClass(const Definition& includeDef,
        string name) {
    string className;
    const Definition* test = fParent;
    while (Definition::Type::kFileType != test->fType) {
        if (Definition::Type::kMark == test->fType && KeyWord::kClass == test->fKeyWord) {
            className = test->fName + "::";
            break;
        }
        test = test->fParent;
    }
    className += name;
    unordered_map<string, IClassDefinition>& map = fIClassMap;
    IClassDefinition& markupDef = map[className];
    if (markupDef.fStart) {
        typedef IClassDefinition* IClassDefPtr;
        return INHERITED::reportError<IClassDefPtr>("class already defined");
    }
    markupDef.fFileName = fFileName;
    markupDef.fStart = includeDef.fStart;
    markupDef.fContentStart = includeDef.fStart;
    markupDef.fName = className;
    this->checkName(&markupDef);
    markupDef.fContentEnd = includeDef.fContentEnd;
    markupDef.fTerminator = includeDef.fTerminator;
    markupDef.fParent = fParent;
    markupDef.fLineCount = fLineCount;
    markupDef.fMarkType = KeyWord::kStruct == includeDef.fKeyWord ?
            MarkType::kStruct : MarkType::kClass;
    markupDef.fKeyWord = includeDef.fKeyWord;
    markupDef.fType = Definition::Type::kMark;
    auto tokenIter = includeDef.fParent->fTokens.begin();
    SkASSERT(includeDef.fParentIndex > 0);
    std::advance(tokenIter, includeDef.fParentIndex - 1);
    const Definition* priorComment = &*tokenIter;
    markupDef.fUndocumented = priorComment->fUndocumented;
    fParent = &markupDef;
    return &markupDef;
}

void IncludeParser::dumpClassTokens(IClassDefinition& classDef) {
    auto& tokens = classDef.fTokens;
    bool wroteTail = true;
    for (auto& token : tokens) {
        if (Definition::Type::kMark == token.fType && MarkType::kComment == token.fMarkType) {
            continue;
        }
        if (wroteTail && MarkType::kMember != token.fMarkType) {
            this->writeBlockSeparator();
        }
        switch (token.fMarkType) {
            case MarkType::kConst:
                this->dumpConst(token, classDef.fName);
            break;
            case MarkType::kEnum:
            case MarkType::kEnumClass:
                this->dumpEnum(token, token.fName);
            break;
            case MarkType::kMethod:
                if (!this->dumpMethod(token, classDef.fName)) {
                    wroteTail = false;
                    continue;
                }
            break;
            case MarkType::kMember:
                this->dumpMember(token);
                continue;
            break;
            case MarkType::kTypedef:
                this->dumpTypedef(token, classDef.fName);
            break;
            default:
                SkASSERT(0);
        }
        this->dumpCommonTail(token);
        wroteTail = true;
    }
}
void IncludeParser::dumpComment(const Definition& token) {
    fLineCount = token.fLineCount;
    fChar = fLine = token.fContentStart;
    fEnd = token.fContentEnd;
    if (MarkType::kMethod == token.fMarkType) {
        this->lf(2);
        this->writeTag("Populate");
        this->lf(2);
        return;
    }
    for (const auto& child : token.fTokens) {
        if (Definition::Type::kMark == child.fType && MarkType::kMember == child.fMarkType) {
            break;
        }
        if (Definition::Type::kMark == child.fType && MarkType::kComment == child.fMarkType) {
            if (child.fPrivate) {
                break;
            }
            if (child.length() > 1) {
                const char* start = child.fContentStart;
                ptrdiff_t length = child.fContentEnd - start;
                SkASSERT(length >= 0);
                while (length && '/' == start[0]) {
                    start += 1;
                    --length;
                }
                while (length && '/' == start[length - 1]) {
                    length -= 1;
                    if (length && '*' == start[length - 1]) {
                        length -= 1;
                    }
                }
                if (length) {
                    this->lf(2);
                    if ("!< " == string(start, length).substr(0, 3)) {
                        return;
                    }
                    this->writeBlock(length, start);
                    this->lf(2);
                }
            }
        }
    }
}

void IncludeParser::dumpCommonTail(const Definition& token) {
    this->lf(2);
    this->writeTag("Example");
    this->lf(1);
    this->writeString("// incomplete");
    this->lf(1);
    this->writeEndTag();
    this->lf(2);
    this->writeTag("SeeAlso");
    this->writeSpace();
    this->writeString("incomplete");
    this->lf(2);
    this->writeEndTag(BmhParser::kMarkProps[(int) token.fMarkType].fName);
    this->lf(2);
}

void IncludeParser::dumpConst(const Definition& token, string className) {
    this->writeTag("Const");
    this->writeSpace();
    this->writeString(token.fName);
    this->writeTagTable("Line", "incomplete");
    this->lf(2);
    this->dumpComment(token);
}

void IncludeParser::dumpDefine(const Definition& token) {
    this->writeTag("Define", token.fName);
    this->lf(2);
    this->writeTag("Code");
    this->lfAlways(1);
    this->writeString("###$");
    this->lfAlways(1);
    this->indentToColumn(4);
    this->writeBlock(token.fTerminator - token.fStart, token.fStart);
    this->lf(1);
    this->indentToColumn(0);
    this->writeString("$$$#");

    this->writeEndTag();
    this->lf(2);
    this->dumpComment(token);
    for (auto& child : token.fTokens) {
        if (MarkType::kComment == child.fMarkType) {
            continue;
        }
        this->writeTag("Param", child.fName);
        this->writeSpace();
        this->writeString("incomplete");
        this->writeSpace();
        this->writeString("##");
        this->lf(1);
    }
}

void IncludeParser::dumpEnum(const Definition& token, string name) {
    string tagType(MarkType::kEnum == token.fMarkType ? "Enum" : "EnumClass");
    this->writeTag(tagType.c_str(), token.fName);
    this->lf(2);
    this->writeTag("Code");
    this->writeTag("Populate");
    this->writeEndTag();
    this->lf(2);
    this->dumpComment(token);
    string prior;
    for (auto& child : token.fTokens) {
        if (MarkType::kComment == child.fMarkType) {
            prior = string(child.fContentStart, child.length());
        }
        if (MarkType::kMember != child.fMarkType) {
            continue;
        }
        this->writeTag("Const");
        this->writeSpace();
        this->writeString(child.fName);
        this->writeSpace(2);
        this->writeString("0 # incomplete; replace '0' with member value");
        this->lf(1);
        this->writeTagNoLF("Line", "#");
        this->writeSpace();
        if ("/!< " == prior.substr(0, 4)) {
            this->writeString(prior.substr(4));
        } else {
            this->writeString("incomplete");
        }
        this->writeSpace();
        this->writeString("##");
        this->lf(1);
        this->writeString("# incomplete; add description or delete");
        this->writeEndTag();
    }
    this->lf(2);
    this->writeString("# incomplete; add description or delete");
    this->lf(2);
}

bool IncludeParser::dumpGlobals(string* globalFileName, long int* globalTell) {
    bool hasGlobals = !fIDefineMap.empty() || !fIFunctionMap.empty() || !fIEnumMap.empty()
            || !fITemplateMap.empty()|| !fITypedefMap.empty() || !fIUnionMap.empty();
    if (!hasGlobals) {
        return true;
    }
    size_t lastBSlash = fFileName.rfind('\\');
    size_t lastSlash = fFileName.rfind('/');
    size_t lastDotH = fFileName.rfind(".h");
    SkASSERT(string::npos != lastDotH);
    if (string::npos != lastBSlash && (string::npos == lastSlash
            || lastBSlash < lastSlash)) {
        lastSlash = lastBSlash;
    } else if (string::npos == lastSlash) {
        lastSlash = -1;
    }
    lastSlash += 1;
    string globalsName = fFileName.substr(lastSlash, lastDotH - lastSlash);
    string fileName = globalsName + "_Reference.bmh";
    *globalFileName = fileName;
    fOut = fopen(fileName.c_str(), "wb");
    if (!fOut) {
        SkDebugf("could not open output file %s\n", globalsName.c_str());
        return false;
    }
    string prefixName = globalsName.substr(0, 2);
    string topicName = globalsName.length() > 2 && isupper(globalsName[2]) &&
        ("Sk" == prefixName || "Gr" == prefixName) ? globalsName.substr(2) : globalsName;
    this->writeTagNoLF("Topic", topicName);
    this->writeEndTag("Alias", topicName + "_Reference");
    this->lf(2);
    if (!fIDefineMap.empty() || !fIFunctionMap.empty() || !fIEnumMap.empty()
            || !fITemplateMap.empty() || !fITypedefMap.empty() || !fIUnionMap.empty()) {
        this->writeTag("Code");
        this->writeTag("Populate");
        this->writeEndTag();
        this->lf(2);
    }
    std::map<int, Definition*> sortedDefs;
    for (const auto& entry : fIDefineMap) {
        sortedDefs[entry.second->fLineCount] = entry.second;
    }
    for (const auto& entry : fIFunctionMap) {
        sortedDefs[entry.second->fLineCount] = entry.second;
    }
    for (const auto& entry : fIEnumMap) {
        if (string::npos == entry.first.find("::")) {
            sortedDefs[entry.second->fLineCount] = entry.second;
        }
    }
    for (const auto& entry : fITemplateMap) {
        sortedDefs[entry.second->fLineCount] = entry.second;
    }
    for (const auto& entry : fITypedefMap) {
        sortedDefs[entry.second->fLineCount] = entry.second;
    }
    for (const auto& entry : fIUnionMap) {
        sortedDefs[entry.second->fLineCount] = entry.second;
    }
    for (const auto& entry : sortedDefs) {
        const Definition* def = entry.second;
        this->writeBlockSeparator();
        switch (def->fMarkType) {
            case MarkType::kDefine:
                this->dumpDefine(*def);
                break;
            case MarkType::kMethod:
                if (!this->dumpMethod(*def, globalsName)) {
                    continue;
                }
                break;
            case MarkType::kEnum:
            case MarkType::kEnumClass:
                this->dumpEnum(*def, globalsName);
                break;
            case MarkType::kTemplate:
                SkASSERT(0);  // incomplete
                break;
            case MarkType::kTypedef: {
                this->writeTag("Typedef");
                this->writeSpace();
                TextParser parser(def);
                if (!parser.skipExact("typedef")) {
                    return false;
                }
                if (!parser.skipSpace()) {
                    return false;
                }
                this->writeBlock(parser.fEnd - parser.fChar, parser.fChar);
                this->lf(2);
                this->dumpComment(*def);
                this->writeEndTag(BmhParser::kMarkProps[(int) entry.second->fMarkType].fName);
                this->lf(2);
                } continue;
            case MarkType::kUnion:
                SkASSERT(0);  // incomplete
                break;
            default:
                SkASSERT(0);
        }
        this->dumpCommonTail(*def);
    }
    *globalTell = ftell(fOut);
    this->writeEndTag("Topic", topicName);
    this->lfAlways(1);
//    fclose(fOut);     // defer closing in case class needs to be also written here
    return true;
}

bool IncludeParser::isClone(const Definition& token) {
    string name = token.fName;
    return name[name.length() - 2] == '_' && isdigit(name[name.length() - 1]);
}

bool IncludeParser::isConstructor(const Definition& token, string className) {
    string name = token.fName;
    return 0 == name.find(className) || '~' == name[0];
}

bool IncludeParser::isInternalName(const Definition& token) {
    string name = token.fName;
    // exception for this SkCanvas function .. for now
    if (0 == token.fName.find("androidFramework_setDeviceClipRestriction")) {
        return false;
    }
    return name.substr(0, 7) == "android"
            || 0 == token.fName.find("internal_")
            || 0 == token.fName.find("Internal_")
            || 0 == token.fName.find("legacy_")
            || 0 == token.fName.find("temporary_")
            || 0 == token.fName.find("private_");
}

bool IncludeParser::isMember(const Definition& token) const {
    if ('f' == token.fStart[0] && isupper(token.fStart[1])) {
        return true;
    }
    if (!islower(token.fStart[0])) {
        return false;
    }
    // make an exception for SkTextBlob::RunBuffer, sole struct with members not in fXxxx format
    if (string::npos != token.fFileName.find("SkTextBlob.h")) {
        const Definition* structToken = token.fParent;
        if (!structToken) {
            return false;
        }
        if (KeyWord::kStruct != structToken->fKeyWord) {
            structToken = token.fParent->fParent;
            if (!structToken) {
                return false;
            }
            if (KeyWord::kStruct != structToken->fKeyWord) {
                return false;
            }
        }
        SkASSERT(structToken->fTokens.size() > 0);
        const Definition& child = structToken->fTokens.front();
        string structName(child.fContentStart, child.length());
        if ("RunBuffer" != structName) {
            return false;
        }
        string tokenName(token.fContentStart, token.length());
        string allowed[] = { "glyphs", "pos", "utf8text", "clusters" };
        for (auto allow : allowed) {
            if (allow == tokenName) {
                return true;
            }
        }
    }
    return false;
}

bool IncludeParser::isOperator(const Definition& token) {
    return "operator" == token.fName.substr(0, 8);
}

bool IncludeParser::dumpMethod(const Definition& token, string className) {
    if (std::any_of(token.fTokens.begin(), token.fTokens.end(),
            [=](const Definition& def) { return MarkType::kComment == def.fMarkType
            && this->isUndocumentable(def.fFileName, def.fContentStart, def.fContentEnd,
            def.fLineCount); } )) {
        return false;
    }
    this->writeTag("Method");
    this->writeSpace();

    string name = string(token.fStart ? token.fStart : token.fContentStart,
            token.length());
    this->writeBlock((int) name.size(), name.c_str());
    string inType;
    if (this->isConstructor(token, className)) {
        inType = "Constructor";
    } else if (this->isOperator(token)) {
        inType = "Operator";
    } else {
        inType = "incomplete";
    }
    this->writeTag("In", inType);
    this->writeTagTable("Line", "incomplete");
    this->lf(2);
    this->dumpComment(token);
    return true;
}

void IncludeParser::dumpMember(const Definition& token) {
    this->writeTag("Member");
    this->writeSpace();
    this->writeDefinition(token, token.fName, 2);
    lf(1);
    for (auto child : token.fChildren) {
        this->writeDefinition(*child);
    }
    this->writeEndTag();
    lf(2);
}

bool IncludeParser::dumpTokens() {
    string globalFileName;
    long int globalTell = 0;
    if (!this->dumpGlobals(&globalFileName, &globalTell)) {
        return false;
    }
    for (const auto& member : fIClassMap) {
        if (string::npos != member.first.find("::")) {
            continue;
        }
        if (!this->dumpTokens(member.first, globalFileName, &globalTell)) {
            return false;
        }
    }
    if (globalTell) {
        fclose(fOut);
        SkDebugf("wrote %s\n", globalFileName.c_str());
    }
    return true;
}

    // dump equivalent markup
bool IncludeParser::dumpTokens(string skClassName, string globalFileName, long int* globalTell) {
    string fileName = skClassName + "_Reference.bmh";
    if (globalFileName != fileName) {
        fOut = fopen(fileName.c_str(), "wb");
        if (!fOut) {
            SkDebugf("could not open output file %s\n", fileName.c_str());
            return false;
        }
    } else {
        fseek(fOut, *globalTell, SEEK_SET);
        this->lf(2);
        this->writeBlockSeparator();
        *globalTell = 0;
    }
    string prefixName = skClassName.substr(0, 2);
    string topicName = skClassName.length() > 2 && isupper(skClassName[2]) &&
        ("Sk" == prefixName || "Gr" == prefixName) ? skClassName.substr(2) : skClassName;
    if (globalFileName != fileName) {
        this->writeTagNoLF("Topic", topicName);
        this->writeEndTag("Alias", topicName + "_Reference");
        this->lf(2);
    }
    auto& classMap = fIClassMap[skClassName];
    SkASSERT(KeyWord::kClass == classMap.fKeyWord || KeyWord::kStruct == classMap.fKeyWord);
    const char* containerType = KeyWord::kClass == classMap.fKeyWord ? "Class" : "Struct";
    this->writeTag(containerType, skClassName);
    this->lf(2);
    auto& tokens = classMap.fTokens;
    for (auto& token : tokens) {
        if (Definition::Type::kMark != token.fType || MarkType::kComment != token.fMarkType) {
            continue;
        }
        this->writeDefinition(token);
        this->lf(1);
    }
    this->lf(2);
    this->writeTag("Code");
    this->writeTag("Populate");
    this->writeEndTag();
    this->lf(2);
    for (auto& oneClass : fIClassMap) {
        if (skClassName + "::" != oneClass.first.substr(0, skClassName.length() + 2)) {
            continue;
        }
        string innerName = oneClass.first.substr(skClassName.length() + 2);
        this->writeBlockSeparator();
        KeyWord keyword = oneClass.second.fKeyWord;
        SkASSERT(KeyWord::kClass == keyword || KeyWord::kStruct == keyword);
        const char* containerType = KeyWord::kClass == keyword ? "Class" : "Struct";
        this->writeTag(containerType, innerName);
        this->writeTagTable("Line", "incomplete");
        this->lf(2);
        this->writeTag("Code");
        this->writeEndTag("ToDo", "fill this in manually");
        this->writeEndTag();
        this->lf(2);
        for (auto& token : oneClass.second.fTokens) {
            if (Definition::Type::kMark != token.fType || MarkType::kComment != token.fMarkType) {
                continue;
            }
            this->writeDefinition(token);
        }
        this->lf(2);
        this->dumpClassTokens(oneClass.second);
        this->lf(2);
        this->writeEndTag(containerType, innerName);
        this->lf(2);
    }
    this->dumpClassTokens(classMap);
    this->writeEndTag(containerType, skClassName);
    this->lf(2);
    this->writeEndTag("Topic", topicName);
    this->lfAlways(1);
    fclose(fOut);
    SkDebugf("wrote %s\n", fileName.c_str());
    return true;
}

void IncludeParser::dumpTypedef(const Definition& token, string className) {
    this->writeTag("Typedef");
    this->writeSpace();
    this->writeString(token.fName);
    this->writeTagTable("Line", "incomplete");
    this->lf(2);
    this->dumpComment(token);
}

string IncludeParser::elidedCodeBlock(const Definition& iDef) {
    SkASSERT(KeyWord::kStruct == iDef.fKeyWord || KeyWord::kClass == iDef.fKeyWord
            || KeyWord::kTemplate == iDef.fKeyWord);
    TextParser i(&iDef);
    fElided = Elided::kYes;
    MarkType markType = MarkType::kClass;
    if (KeyWord::kTemplate == iDef.fKeyWord) {  // may be function
        for (auto child : iDef.fChildren) {
            if (MarkType::kMethod == child->fMarkType) {
                markType = MarkType::kFunction;
                break;
            }
        }
    }
    return this->writeCodeBlock(i, markType, 0);
}

 string IncludeParser::filteredBlock(string inContents, string filterContents) {
    string result;
    const unordered_map<string, Definition*>* mapPtr = nullptr;
    if ("Constant" == inContents) {
        mapPtr = &fIConstMap;
    } else {
        SkASSERT(0); // only Constant supported for now
    }
    vector<Definition*> consts;
    for (auto entry : *mapPtr) {
        if (string::npos == entry.first.find(filterContents)) {
            continue;
        }
        consts.push_back(entry.second);
    }
    std::sort(consts.begin(), consts.end(), [](Definition* def1, Definition* def2) {
        return def1->fLineCount < def2->fLineCount;
    } );
    for (auto oneConst : consts) {
        result += this->writeCodeBlock(*oneConst);
    }
    return result;
}

bool IncludeParser::findCommentAfter(const Definition& includeDef, Definition* markupDef) {
    this->checkName(markupDef);
    const Definition* parent = includeDef.fParent;
    int index = includeDef.fParentIndex;
    auto wordIter = parent->fTokens.begin();
    std::advance(wordIter, index);
    SkASSERT(&*wordIter == &includeDef);
    size_t commentLine = 0;
    do {
        wordIter = std::next(wordIter);
        if (parent->fTokens.end() == wordIter) {
            break;
        }
        commentLine = wordIter->fLineCount;
    } while (Punctuation::kSemicolon != wordIter->fPunctuation);
    wordIter = std::next(wordIter);
    if (parent->fTokens.end() != wordIter && Bracket::kSlashSlash == wordIter->fBracket
            && wordIter->fLineCount == commentLine) {
        return this->parseComment(wordIter->fFileName, wordIter->fContentStart,
                wordIter->fContentEnd, wordIter->fLineCount, markupDef, &markupDef->fUndocumented);
    }
    return true;
}

bool IncludeParser::findComments(const Definition& includeDef, Definition* markupDef) {
    this->checkName(markupDef);
    // add comment preceding class, if any
    Definition* parent = includeDef.fParent;
    int index = includeDef.fParentIndex;
    auto wordIter = parent->fTokens.begin();
    std::advance(wordIter, index);
    SkASSERT(&*wordIter == &includeDef);
    while (parent->fTokens.begin() != wordIter) {
        auto testIter = std::prev(wordIter);
        if (Definition::Type::kWord != testIter->fType
            && Definition::Type::kKeyWord != testIter->fType
            && (Definition::Type::kBracket != testIter->fType
            || Bracket::kAngle != testIter->fBracket)
            && (Definition::Type::kPunctuation != testIter->fType
            || Punctuation::kAsterisk != testIter->fPunctuation)) {
            break;
        }
        wordIter = testIter;
    }
    auto commentIter = wordIter;
    while (parent->fTokens.begin() != commentIter) {
        auto testIter = std::prev(commentIter);
        bool isComment = Definition::Type::kBracket == testIter->fType
                && (Bracket::kSlashSlash == testIter->fBracket
                || Bracket::kSlashStar == testIter->fBracket);
        if (!isComment) {
            break;
        }
        commentIter = testIter;
    }
    while (commentIter != wordIter) {
        if (!this->parseComment(commentIter->fFileName, commentIter->fContentStart,
                commentIter->fContentEnd, commentIter->fLineCount, markupDef,
                &markupDef->fUndocumented)) {
            return false;
        }
        commentIter->fUndocumented = markupDef->fUndocumented;
        commentIter = std::next(commentIter);
    }
    return true;
}

Definition* IncludeParser::findIncludeObject(const Definition& includeDef, MarkType markType,
        string typeName) {
    typedef Definition* DefinitionPtr;
    auto mapIter = std::find_if(fMaps.begin(), fMaps.end(),
            [markType](DefinitionMap& defMap){ return markType == defMap.fMarkType; } );
    if (mapIter == fMaps.end()) {
        return nullptr;
    }
    if (mapIter->fInclude->end() == mapIter->fInclude->find(typeName)) {
        return reportError<DefinitionPtr>("invalid mark type");
    }
    string name = this->uniqueName(*mapIter->fInclude, typeName);
    Definition& markupDef = *(*mapIter->fInclude)[name];
    if (markupDef.fStart) {
        return reportError<DefinitionPtr>("definition already defined");
    }
    markupDef.fFileName = fFileName;
    markupDef.fStart = includeDef.fStart;
    markupDef.fContentStart = includeDef.fStart;
    this->checkName(&markupDef);
    markupDef.fName = name;
    markupDef.fContentEnd = includeDef.fContentEnd;
    markupDef.fTerminator = includeDef.fTerminator;
    markupDef.fParent = fParent;
    markupDef.fLineCount = includeDef.fLineCount;
    markupDef.fMarkType = markType;
    markupDef.fKeyWord = includeDef.fKeyWord;
    markupDef.fType = Definition::Type::kMark;
    return &markupDef;
}

Definition* IncludeParser::findMethod(const Definition& bmhDef) {
    auto doubleColon = bmhDef.fName.rfind("::");
    if (string::npos == doubleColon) {
        const auto& iGlobalMethod = fIFunctionMap.find(bmhDef.fName);
        SkASSERT(fIFunctionMap.end() != iGlobalMethod);
        return iGlobalMethod->second;
    }
    string className = bmhDef.fName.substr(0, doubleColon);
    const auto& iClass = fIClassMap.find(className);
    if (fIClassMap.end() == iClass) {
        return nullptr;
    }
    string methodName = bmhDef.fName.substr(doubleColon + 2);
    auto& iTokens = iClass->second.fTokens;
    const auto& iMethod = std::find_if(iTokens.begin(), iTokens.end(),
            [methodName](Definition& token) {
            return MarkType::kMethod == token.fMarkType
                    && !token.fUndocumented
                    && (methodName == token.fName
                    || methodName == token.fName + "()"); } );
    if (iTokens.end() != iMethod) {
        return &*iMethod;
    }
    size_t subClassPos = className.rfind("::");
    if (string::npos != subClassPos) {
        className = className.substr(subClassPos + 2);
    }
    // match may be constructor; compare strings to see if this is so
    if (string::npos == methodName.find('(')) {
        return nullptr;
    }
    auto stripper = [](string s) -> string {
        bool last = false;
        string result;
        for (char c : s) {
            if (' ' >= c) {
                if (!last) {
                    last = true;
                    result += ' ';
                }
                continue;
            }
            result += c;
            last = false;
        }
        return result;
    };
    string strippedMethodName = stripper(methodName);
    if (strippedMethodName == methodName) {
        strippedMethodName = "";
    }
    const auto& cMethod = std::find_if(iTokens.begin(), iTokens.end(),
            [className, methodName, stripper, strippedMethodName](Definition& token) {
        if (MarkType::kMethod != token.fMarkType) {
            return false;
        }
        if (token.fUndocumented) {
            return false;
        }
        TextParser parser(&token);
        const char* match = parser.strnstr(className.c_str(), parser.fEnd);
        if (!match) {
            return false;
        }
        parser.skipTo(match);
        parser.skipExact(className.c_str());
        if ('(' != parser.peek()) {
            return false;
        }
        parser.skipToBalancedEndBracket('(', ')');
        string iMethodName(match, parser.fChar - match);
        if (methodName == iMethodName) {
            return true;
        }
        if (strippedMethodName.empty()) {
            return false;
        }
        string strippedIName = stripper(iMethodName);
        return strippedIName == strippedMethodName;
    } );
    SkAssertResult(iTokens.end() != cMethod);
    return &*cMethod;
}

Definition* IncludeParser::parentBracket(Definition* parent) const {
    while (parent && Definition::Type::kBracket != parent->fType) {
        parent = parent->fParent;
    }
    return parent;
}

Bracket IncludeParser::grandParentBracket() const {
    Definition* parent = parentBracket(fParent);
    parent = parentBracket(parent ? parent->fParent : nullptr);
    return parent ? parent->fBracket : Bracket::kNone;
}

bool IncludeParser::inAlignAs() const {
    if (fParent->fTokens.size() < 2) {
        return false;
    }
    auto reverseIter = fParent->fTokens.end();
    bool checkForBracket = true;
    while (fParent->fTokens.begin() != reverseIter) {
        std::advance(reverseIter, -1);
        if (checkForBracket) {
            if (Definition::Type::kBracket != reverseIter->fType) {
                return false;
            }
            if (Bracket::kParen != reverseIter->fBracket) {
                return false;
            }
            checkForBracket = false;
            continue;
        }
        if (Definition::Type::kKeyWord != reverseIter->fType) {
            return false;
        }
        return KeyWord::kAlignAs == reverseIter->fKeyWord;
    }
    return false;
}

const Definition* IncludeParser::include(string match) const {
    for (auto& entry : fIncludeMap) {
        if (string::npos == entry.first.find(match)) {
            continue;
        }
        return &entry.second;
    }
    SkASSERT(0);
    return nullptr;
}

// caller just returns, so report error here
bool IncludeParser::parseClass(Definition* includeDef, IsStruct isStruct) {
    SkASSERT(includeDef->fTokens.size() > 0);
    // parse class header
    auto iter = includeDef->fTokens.begin();
    if (!strncmp(iter->fStart, "SK_API", iter->fContentEnd - iter->fStart)) {
        // todo : documentation is ignoring this for now
        iter = std::next(iter);
    }
    bool hasAlignAs = iter->fKeyWord == KeyWord::kAlignAs;
    if (hasAlignAs) {
        iter = std::next(iter);
        if (Definition::Type::kBracket != iter->fType || Bracket::kParen != iter->fBracket) {
            return includeDef->reportError<bool>("expected alignas argument");
        }
        iter = std::next(iter);
    }
    string nameStr(iter->fStart, iter->fContentEnd - iter->fStart);
    includeDef->fName = nameStr;
    this->checkName(includeDef);
    iter = std::next(iter);
    if (iter == includeDef->fTokens.end()) {
        return true;  // forward declaration only
    }
    do {
        if (iter == includeDef->fTokens.end()) {
            return includeDef->reportError<bool>("unexpected end");
        }
        if ('{' == iter->fStart[0] && Definition::Type::kPunctuation == iter->fType) {
            break;
        }
    } while (static_cast<void>(iter = std::next(iter)), true);
    if (Punctuation::kLeftBrace != iter->fPunctuation) {
        return iter->reportError<bool>("expected left brace");
    }
    IClassDefinition* markupDef = this->defineClass(*includeDef, nameStr);
    if (!markupDef) {
        return iter->reportError<bool>("expected markup definition");
    }
    markupDef->fStart = iter->fStart;
    if (!this->findComments(*includeDef, markupDef)) {
        return iter->reportError<bool>("find comments failed");
    }
    if (markupDef->fUndocumented) {
        includeDef->fUndocumented = true;
    }
//    if (1 != includeDef->fChildren.size()) {
//        return false;  // fix me: SkCanvasClipVisitor isn't correctly parsed
//    }
    auto includeDefIter = includeDef->fChildren.begin();
    if (hasAlignAs) {
        SkASSERT(includeDef->fChildren.end() != includeDefIter);
        SkASSERT(Bracket::kParen == (*includeDefIter)->fBracket);
        std::advance(includeDefIter, 1);
    }
    if (includeDef->fChildren.end() != includeDefIter
            && Bracket::kAngle == (*includeDefIter)->fBracket) {
        std::advance(includeDefIter, 1);
    }
    includeDef = *includeDefIter;
    SkASSERT(Bracket::kBrace == includeDef->fBracket);
    iter = includeDef->fTokens.begin();
    // skip until public
    int publicIndex = 0;
    if (IsStruct::kNo == isStruct) {
        const char* publicName = kKeyWords[(int) KeyWord::kPublic].fName;
        size_t publicLen = strlen(publicName);
        while (iter != includeDef->fTokens.end()
                && (publicLen != (size_t) (iter->fContentEnd - iter->fStart)
                || strncmp(iter->fStart, publicName, publicLen))) {
            iter->fPrivate = true;
            iter = std::next(iter);
            ++publicIndex;
        }
    }
    int keyIndex = publicIndex;
    KeyWord currentKey = KeyWord::kPublic;
    const char* publicName = kKeyWords[(int) KeyWord::kPublic].fName;
    size_t publicLen = strlen(publicName);
    const char* protectedName = kKeyWords[(int) KeyWord::kProtected].fName;
    size_t protectedLen = strlen(protectedName);
    const char* privateName = kKeyWords[(int) KeyWord::kPrivate].fName;
    size_t privateLen = strlen(privateName);
    auto childIter = includeDef->fChildren.begin();
    while (includeDef->fChildren.end() != childIter && (*childIter)->fPrivate) {
        std::advance(childIter, 1);
    }
    while (childIter != includeDef->fChildren.end()) {
        Definition* child = *childIter;
        while (child->fParentIndex > keyIndex && iter != includeDef->fTokens.end()) {
            iter->fPrivate = KeyWord::kPublic != currentKey;
            const char* testStart = iter->fStart;
            size_t testLen = (size_t) (iter->fContentEnd - testStart);
            iter = std::next(iter);
            ++keyIndex;
            if (publicLen == testLen && !strncmp(testStart, publicName, testLen)) {
                currentKey = KeyWord::kPublic;
                break;
            }
            if (protectedLen == testLen && !strncmp(testStart, protectedName, testLen)) {
                currentKey = KeyWord::kProtected;
                break;
            }
            if (privateLen == testLen && !strncmp(testStart, privateName, testLen)) {
                currentKey = KeyWord::kPrivate;
                break;
            }
        }
        fLastObject = nullptr;
        if (KeyWord::kPublic == currentKey) {
            if (!this->parseObject(child, markupDef)) {
                return false;
            }
        }
        fLastObject = child;
        childIter = std::next(childIter);
    }
    while (iter != includeDef->fTokens.end()) {
        iter->fPrivate = KeyWord::kPublic != currentKey;
        iter = std::next(iter);
    }
    SkASSERT(fParent->fParent);
    fParent = fParent->fParent;
    return true;
}

bool IncludeParser::isUndocumentable(string filename, const char* start, const char* end,
        int lineCount) {
    TextParser parser(filename, start, end, lineCount);
    const vector<string> skipWords = { "deprecated", "experimental", "private" };
    const vector<string> butNot = { "to be deprecated", "may be deprecated" };
    const vector<string> alsoNot = { "todo" };
    string match = parser.anyWord(skipWords, 0);
    if ("" != match) {
        if (parser.anyWord(alsoNot, 0).empty()
                && ("deprecated" != match || parser.anyWord(butNot, 2).empty())) {
            return true;
        }
    }
    return false;
}

bool IncludeParser::parseComment(string filename, const char* start, const char* end,
        int lineCount, Definition* markupDef, bool* undocumentedPtr) {
    if (this->isUndocumentable(filename, start, end, lineCount)) {
        *undocumentedPtr = true;
    }
    // parse doxygen if present
    TextParser parser(filename, start, end, lineCount);
    if (parser.startsWith("**")) {
        parser.next();
        parser.next();
        parser.skipWhiteSpace();
        if ('\\' == parser.peek()) {
            parser.next();
            // Doxygen tag may be "file" or "fn" in addition to "class", "enum", "struct"
            if (parser.skipExact("file")) {
                if (Definition::Type::kFileType != fParent->fType) {
                    return reportError<bool>("expected parent is file");
                }
                string filename = markupDef->fileName();
                if (!parser.skipWord(filename.c_str())) {
                    return reportError<bool>("missing object type");
                }
            } else if (parser.skipExact("fn")) {
                SkASSERT(0);  // incomplete
            } else {
                if (!parser.skipWord(kKeyWords[(int) markupDef->fKeyWord].fName)) {
                    return reportError<bool>("missing object type");
                }
                if (!parser.skipWord(markupDef->fName.c_str()) &&
                        KeyWord::kEnum != markupDef->fKeyWord) {
                    return reportError<bool>("missing object name");
                }
            }
        }
    }
    // remove leading '*' if present
    Definition* parent = markupDef->fTokens.size() ? &markupDef->fTokens.back() : markupDef;
    while (!parser.eof() && parser.skipWhiteSpace()) {
        while ('*' == parser.peek()) {
            parser.next();
            if (parser.eof()) {
                break;
            }
            parser.skipWhiteSpace();
        }
        if (parser.eof()) {
            break;
        }
        const char* lineEnd = parser.trimmedLineEnd();
        markupDef->fTokens.emplace_back(MarkType::kComment, parser.fChar, lineEnd,
                parser.fLineCount, parent, '\0');
        parser.skipToEndBracket('\n');
    }
    return true;
}

/*
    find comment either in front of or after the const def and then extract if the
    const is undocumented
 */
bool IncludeParser::parseConst(Definition* child, Definition* markupDef) {
    if (!markupDef) {
        fGlobals.emplace_back(MarkType::kConst, child->fContentStart, child->fContentEnd,
                child->fLineCount, fParent, '\0');
        Definition* globalMarkupChild = &fGlobals.back();
        string globalUniqueName = this->uniqueName(fIConstMap, child->fName);
        globalMarkupChild->fName = globalUniqueName;
        if (!this->findComments(*child, globalMarkupChild)) {
            return false;
        }
        if (!this->findCommentAfter(*child, globalMarkupChild)) {
            return false;
        }
        if (globalMarkupChild->fUndocumented) {
            child->fUndocumented = true;
        } else {
            fIConstMap[globalUniqueName] = globalMarkupChild;
        }
        return true;
    }
    markupDef->fTokens.emplace_back(MarkType::kConst, child->fContentStart, child->fContentEnd,
        child->fLineCount, markupDef, '\0');
    Definition* markupChild = &markupDef->fTokens.back();
    markupChild->fName = child->fName;
    markupChild->fTerminator = markupChild->fContentEnd;
    IClassDefinition& classDef = fIClassMap[markupDef->fName];
    classDef.fConsts[child->fName] = markupChild;
    if (!this->findComments(*child, markupChild)) {
        return false;
    }
    if (!this->findCommentAfter(*child, markupChild)) {
        return false;
    }
    if (markupChild->fUndocumented) {
        child->fUndocumented = true;
    } else {
        fIConstMap[child->fName] = markupChild;
    }
    return true;
}

bool IncludeParser::parseDefine(Definition* child, Definition* markupDef) {
    TextParser parser(child);
    if (!parser.skipExact("#define")) {
        return false;
    }
    if (!parser.skipSpace()) {
        return false;
    }
    const char* nameStart = parser.fChar;
    parser.skipToNonAlphaNum(); // FIXME: just want to skip isalnum() and '_'
    if (parser.eof()) {
        return true;    // do nothing if #define doesn't define anything
    }
    string nameStr(nameStart, parser.fChar - nameStart);
    struct Param {
        const char* fStart;
        const char* fEnd;
    };
    vector<Param> params;
    if ('(' == parser.peek()) {
        parser.next();
        if (!parser.skipSpace()) {
            return false;
        }
        do {
            const char* paramStart = parser.fChar;
            if (!parser.skipExact("...")) {
                parser.skipToNonAlphaNum();
            }
            if (parser.eof()) {
                return false;
            }
            params.push_back({paramStart, parser.fChar});
            if (!parser.skipSpace()) {
                return false;
            }
            if (')' == parser.peek()) {
                parser.next();
                break;
            }
            if (',' != parser.next()) {
                return false;
            }
            if (!parser.skipSpace()) {
                return false;
            }
        } while (true);
    }
    if (!parser.skipSpace()) {
        return false;
    }
    if (!markupDef) {
        fGlobals.emplace_back(MarkType::kDefine, nameStart, child->fContentEnd,
                child->fLineCount, fParent, '\0');
        Definition* globalMarkupChild = &fGlobals.back();
        string globalUniqueName = this->uniqueName(fIDefineMap, nameStr);
        globalMarkupChild->fName = globalUniqueName;
        globalMarkupChild->fTerminator = child->fContentEnd;
        if (!this->findComments(*child, globalMarkupChild)) {
            return false;
        }
        if (!globalMarkupChild->fUndocumented) {
            fIDefineMap[globalUniqueName] = globalMarkupChild;
        }
        for (Param param : params) {
            globalMarkupChild->fTokens.emplace_back(MarkType::kParam, param.fStart, param.fEnd,
                    child->fLineCount, globalMarkupChild, '\0');
            Definition* paramChild = &globalMarkupChild->fTokens.back();
            paramChild->fName = string(param.fStart, param.fEnd - param.fStart);
            this->checkName(paramChild);
            paramChild->fTerminator = param.fEnd;
        }
        return true;
    }
    markupDef->fTokens.emplace_back(MarkType::kDefine, child->fContentStart, child->fContentEnd,
            child->fLineCount, markupDef, '\0');
    Definition* markupChild = &markupDef->fTokens.back();
    markupChild->fName = nameStr;
    markupChild->fTerminator = markupChild->fContentEnd;
    IClassDefinition& classDef = fIClassMap[markupDef->fName];
    if (!this->findComments(*child, markupChild)) {
        return false;
    }
    if (markupChild->fUndocumented) {
        child->fUndocumented = true;
    } else {
        classDef.fDefines[nameStr] = markupChild;
        fIDefineMap[nameStr] = markupChild;
    }
    return true;
}

bool IncludeParser::parseEnum(Definition* child, Definition* markupDef) {
	if (!child->fTokens.size()) {
		return true;	// if enum is a forward declaration, do nothing
	}
    bool isEnumClass = false;
    Definition* parent = child;
    auto token = parent->fTokens.begin();
    if (Definition::Type::kKeyWord == token->fType && KeyWord::kClass == token->fKeyWord) {
        isEnumClass = true;
        parent = &*token;
        token = parent->fTokens.begin();
    }
    SkASSERT(Definition::Type::kWord == token->fType);
    string nameStr = string(token->fStart, token->fContentEnd - token->fStart);
    Definition* markupChild;
    if (!markupDef) {
        fGlobals.emplace_back(MarkType::kEnum, child->fContentStart, child->fContentEnd,
                child->fLineCount, fParent, '\0');
        markupChild = &fGlobals.back();
        string globalUniqueName = this->uniqueName(fIEnumMap, nameStr);
        markupChild->fName = globalUniqueName;
        markupChild->fTerminator = child->fContentEnd;
        if (!markupChild->fUndocumented) {
            fIEnumMap[globalUniqueName] = markupChild;
        }
    } else {
        markupDef->fTokens.emplace_back(MarkType::kEnum, child->fContentStart, child->fContentEnd,
            child->fLineCount, markupDef, '\0');
        markupChild = &markupDef->fTokens.back();
    }
    SkASSERT(KeyWord::kNone == markupChild->fKeyWord);
    markupChild->fKeyWord = KeyWord::kEnum;
    if (isEnumClass) {
        markupChild->fMarkType = MarkType::kEnumClass;
    }
    if (markupDef) {
        markupChild->fName = markupDef->fName + "::" + nameStr;
    }
    if (!this->findComments(*child, markupChild)) {
        return false;
    }
    if (markupChild->fUndocumented) {
        child->fUndocumented = true;
    }
    if (!this->parseEnumConst(token, parent->fTokens.end(), markupChild)) {
        return false;
    }
    for (auto outsideMember : child->fChildren) {
        if (Definition::Type::kBracket == outsideMember->fType) {
            continue;
        }
        SkASSERT(Definition::Type::kKeyWord == outsideMember->fType);
        if (KeyWord::kClass == outsideMember->fKeyWord) {
            continue;
        }
        SkASSERT(KeyWord::kStatic == outsideMember->fKeyWord);
        markupChild->fTokens.emplace_back(MarkType::kMember, outsideMember->fContentStart,
                outsideMember->fContentEnd, outsideMember->fLineCount, markupChild, '\0');
        Definition* member = &markupChild->fTokens.back();
        member->fName = outsideMember->fName;
        this->checkName(member);
        // FIXME: ? add comment as well ?
        markupChild->fChildren.push_back(member);
    }
    if (markupDef) {
        IClassDefinition& classDef = fIClassMap[markupDef->fName];
        SkASSERT(classDef.fStart);
        string uniqueName = this->uniqueName(classDef.fEnums, nameStr);
        string fullName = markupChild->fName;
        markupChild->fName = uniqueName;
        classDef.fEnums[uniqueName] = markupChild;
        if (!markupChild->fUndocumented) {
            fIEnumMap[fullName] = markupChild;
        }
    }
    return true;
}

bool IncludeParser::parseOneEnumConst(list<Definition>& constList,
        Definition* markupChild, bool skipWord) {
    auto memberIter = constList.begin();
    const auto memberIterEnd = constList.end();
    if (skipWord) {
        SkASSERT(Definition::Type::kWord == memberIter->fType);
        memberIter = std::next(memberIter);
        SkASSERT(memberIterEnd != memberIter);
    }
    // token array has parse atoms; child array has comments
    bool undocumented = false;
    while (memberIterEnd != memberIter) {
        while (Bracket::kSlashStar == memberIter->fBracket) {
            if (!this->parseComment(memberIter->fFileName, memberIter->fContentStart,
                    memberIter->fContentEnd, memberIter->fLineCount, markupChild, &undocumented)) {
                return false;
            }
            memberIter = std::next(memberIter);
            if (memberIterEnd == memberIter) {
                return false;
            }
        }
        if (Bracket::kPound == memberIter->fBracket) {
            KeyWord keyWord = memberIter->fKeyWord;
            bool sawIf = KeyWord::kIfdef == keyWord || KeyWord::kIf == keyWord
                    || KeyWord::kElif == keyWord;
            if (sawIf || KeyWord::kElse == keyWord) {
                if (!parseOneEnumConst(memberIter->fTokens, markupChild, sawIf)) {
                    return false;
                }
            } else {
                SkASSERT(KeyWord::kEndif == keyWord || KeyWord::kError == keyWord);
            }
            memberIter = std::next(memberIter);
            if (memberIterEnd == memberIter) {
                break;
            }
            continue;
        }
        while (Definition::Type::kWord != memberIter->fType) {
            memberIter = std::next(memberIter);
            if (memberIterEnd == memberIter) {
                return false;
            }
        }
        auto memberStart = memberIter;
        Definition* memberEnd = nullptr;
        const char* last;
        do {
            last = memberIter->fContentEnd;
            memberIter = std::next(memberIter);
            if (memberIterEnd == memberIter) {
                break;
            }
            memberEnd = &*memberIter;
        } while (string::npos == string(last, memberIter->fContentStart).find(','));
        if (!memberEnd) {
            return false;
        }
        if (memberIterEnd != memberIter && Bracket::kSlashSlash == memberIter->fBracket) {
            if (!this->parseComment(memberIter->fFileName, memberIter->fContentStart,
                    memberIter->fContentEnd, memberIter->fLineCount, markupChild, &undocumented)) {
                return false;
            }
            memberIter = std::next(memberIter);
        }
        markupChild->fTokens.emplace_back(MarkType::kMember, memberStart->fContentStart,
                memberEnd->fContentEnd, memberStart->fLineCount, markupChild, '\0');
        Definition* markupMember = &markupChild->fTokens.back();
        string name = string(memberStart->fContentStart, memberStart->length());
        memberStart->fName = name;
        markupMember->fName = name;
        this->checkName(markupMember);
        memberStart->fUndocumented = markupMember->fUndocumented;
        memberStart->fMarkType = MarkType::kMember;
        undocumented = false;
    }
    return true;
}

bool IncludeParser::parseEnumConst(list<Definition>::iterator& tokenIter,
        const list<Definition>::iterator& tokenEnd, Definition* markupChild) {
    SkASSERT(Definition::Type::kWord == tokenIter->fType);  // should be enum name
    tokenIter = std::next(tokenIter);
    SkASSERT(tokenEnd != tokenIter);
    if (Definition::Type::kKeyWord == tokenIter->fType) {
        SkASSERT((unsigned) tokenIter->fKeyWord < SK_ARRAY_COUNT(kKeyWords));
        SkASSERT(KeyProperty::kNumber == kKeyWords[(int) tokenIter->fKeyWord].fProperty);
        tokenIter = std::next(tokenIter);
        SkASSERT(tokenEnd != tokenIter);
    }
    SkASSERT(Punctuation::kLeftBrace == tokenIter->fPunctuation);
    tokenIter = std::next(tokenIter);
    SkASSERT(tokenEnd != tokenIter);
    SkASSERT(Bracket::kBrace == tokenIter->fBracket);
    return parseOneEnumConst(tokenIter->fTokens, markupChild, false);
}

bool IncludeParser::parseInclude(string name) {
    fParent = &fIncludeMap[name];
    fParent->fName = name;
    this->checkName(fParent);
    fParent->fFileName = fFileName;
    fParent->fType = Definition::Type::kFileType;
    fParent->fContentStart = fChar;
    fParent->fContentEnd = fEnd;
    // parse include file into tree
    while (fChar < fEnd) {
        if (!this->parseChar()) {
            return false;
        }
    }
    // parse tree and add named objects to maps
    fParent = &fIncludeMap[name];
    if (!this->parseObjects(fParent, nullptr)) {
        return false;
    }
    return true;
}

bool IncludeParser::parseMember(Definition* child, Definition* markupDef) {
    const char* typeStart = child->fChildren[0]->fContentStart;
    markupDef->fTokens.emplace_back(MarkType::kMember, typeStart, child->fContentStart,
        child->fLineCount, markupDef, '\0');
    Definition* markupChild = &markupDef->fTokens.back();
    TextParser nameParser(child);
    nameParser.skipToNonName();
    string nameStr = string(child->fContentStart, nameParser.fChar - child->fContentStart);
    IClassDefinition& classDef = fIClassMap[markupDef->fName];
    string uniqueName = this->uniqueName(classDef.fMethods, nameStr);
    markupChild->fName = uniqueName;
    this->checkName(markupChild);
    markupChild->fTerminator = markupChild->fContentEnd;
    if (!markupChild->fUndocumented) {
        classDef.fMembers[uniqueName] = markupChild;
    }
    if (child->fParentIndex >= 2) {
        auto comment = child->fParent->fTokens.begin();
        std::advance(comment, child->fParentIndex - 2);
        if (Definition::Type::kBracket == comment->fType
                && (Bracket::kSlashStar == comment->fBracket
                || Bracket::kSlashSlash == comment->fBracket)) {
            TextParser parser(&*comment);
            do {
                parser.skipToAlpha();
                if (parser.eof()) {
                    break;
                }
                const char* start = parser.fChar;
                const char* end = parser.trimmedBracketEnd('\n');
                if (Bracket::kSlashStar == comment->fBracket) {
                    const char* commentEnd = parser.strnstr("*/", end);
                    if (commentEnd) {
                        end = commentEnd;
                    }
                }
                markupDef->fTokens.emplace_back(MarkType::kComment, start, end, child->fLineCount,
                        markupDef, '\0');
                Definition* commentChild = &markupDef->fTokens.back();
                markupChild->fChildren.emplace_back(commentChild);
                parser.skipTo(end);
            } while (!parser.eof());
        }
    }
    return true;
}

bool IncludeParser::parseMethod(Definition* child, Definition* markupDef) {
    auto tokenIter = child->fParent->fTokens.begin();
    std::advance(tokenIter, child->fParentIndex);
    tokenIter = std::prev(tokenIter);
    const char* nameEnd = tokenIter->fContentEnd;
    bool addConst = false;
    auto operatorCheck = tokenIter;
    if ('[' == tokenIter->fStart[0] || '*' == tokenIter->fStart[0]) {
        operatorCheck = std::prev(tokenIter);
    }
    if (KeyWord::kOperator == operatorCheck->fKeyWord) {
        auto closeParen = std::next(tokenIter);
        SkASSERT(Definition::Type::kBracket == closeParen->fType &&
                '(' == closeParen->fContentStart[0]);
        nameEnd = closeParen->fContentEnd + 1;
        closeParen = std::next(closeParen);
        if (Definition::Type::kKeyWord == closeParen->fType &&
                KeyWord::kConst == closeParen->fKeyWord) {
            addConst = true;
        }
        tokenIter = operatorCheck;
    }
    string nameStr(tokenIter->fStart, nameEnd - tokenIter->fStart);
    if (addConst) {
        nameStr += " const";
    }
    while (tokenIter != child->fParent->fTokens.begin()) {
        auto testIter = std::prev(tokenIter);
        switch (testIter->fType) {
            case Definition::Type::kWord:
                if (testIter == child->fParent->fTokens.begin() &&
                        (KeyWord::kIfdef == child->fParent->fKeyWord ||
                        KeyWord::kIfndef == child->fParent->fKeyWord ||
                        KeyWord::kIf == child->fParent->fKeyWord)) {
                    std::next(tokenIter);
                    break;
                }
                goto keepGoing;
            case Definition::Type::kKeyWord: {
                KeyProperty keyProperty = kKeyWords[(int) testIter->fKeyWord].fProperty;
                if (KeyProperty::kNumber == keyProperty || KeyProperty::kModifier == keyProperty) {
                    goto keepGoing;
                }
            } break;
            case Definition::Type::kBracket:
                if (Bracket::kAngle == testIter->fBracket) {
                    goto keepGoing;
                }
                break;
            case Definition::Type::kPunctuation:
                if (Punctuation::kSemicolon == testIter->fPunctuation
                        || Punctuation::kLeftBrace == testIter->fPunctuation
                        || Punctuation::kColon == testIter->fPunctuation) {
                    break;
                }
            keepGoing:
                tokenIter = testIter;
                continue;
            default:
                break;
        }
        break;
    }
    tokenIter->fName = nameStr;     // simple token stream, OK if name is duplicate
    tokenIter->fMarkType = MarkType::kMethod;
    tokenIter->fPrivate = string::npos != nameStr.find("::")
            && KeyWord::kTemplate != child->fParent->fKeyWord;
    this->checkName(&*tokenIter);
    auto testIter = child->fParent->fTokens.begin();
    SkASSERT(child->fParentIndex > 0);
    std::advance(testIter, child->fParentIndex - 1);
    if (tokenIter->fParent && KeyWord::kIfdef == tokenIter->fParent->fKeyWord &&
            0 == tokenIter->fParentIndex) {
        tokenIter = std::next(tokenIter);
    }
    const char* start = tokenIter->fContentStart;
    const char* end = tokenIter->fContentEnd;
    const char kDebugCodeStr[] = "SkDEBUGCODE";
    const size_t kDebugCodeLen = sizeof(kDebugCodeStr) - 1;
    if (end - start == kDebugCodeLen && !strncmp(start, kDebugCodeStr, kDebugCodeLen)) {
        std::advance(testIter, 1);
        start = testIter->fContentStart + 1;
        end = testIter->fContentEnd - 1;
    } else {
        end = testIter->fContentEnd;
        do {
            std::advance(testIter, 1);
            if (testIter == child->fParent->fTokens.end()) {
                break;
            }
            switch (testIter->fType) {
                case Definition::Type::kPunctuation:
                    SkASSERT(Punctuation::kSemicolon == testIter->fPunctuation
                            || Punctuation::kLeftBrace == testIter->fPunctuation
                            || Punctuation::kColon == testIter->fPunctuation);
                    end = testIter->fStart;
                    break;
                case Definition::Type::kKeyWord: {
                    KeyProperty keyProperty = kKeyWords[(int) testIter->fKeyWord].fProperty;
                    if (KeyProperty::kNumber == keyProperty || KeyProperty::kModifier == keyProperty) {
                        continue;
                    }
                    } break;
                default:
                    continue;
            }
            break;
        } while (true);
    }
    while (end > start && ' ' >= end[-1]) {
        --end;
    }
    if (!markupDef) {
        auto parentIter = child->fParent->fTokens.begin();
        SkASSERT(child->fParentIndex > 0);
        std::advance(parentIter, child->fParentIndex - 1);
        Definition* methodName = &*parentIter;
        TextParser nameParser(methodName);
        if (nameParser.skipToEndBracket(':') && nameParser.startsWith("::")) {
            return true;  // expect this is inline class definition outside of class
        }
        fGlobals.emplace_back(MarkType::kMethod, start, end, tokenIter->fLineCount,
                fParent, '\0');
        Definition* globalMarkupChild = &fGlobals.back();
        string globalUniqueName = this->uniqueName(fIFunctionMap, nameStr);
        globalMarkupChild->fName = globalUniqueName;
        if (!this->findComments(*child, globalMarkupChild)) {
            return false;
        }
        if (globalMarkupChild->fUndocumented) {
            child->fUndocumented = true;
        } else {
            fIFunctionMap[globalUniqueName] = globalMarkupChild;
        }
        return true;
    }
    markupDef->fTokens.emplace_back(MarkType::kMethod, start, end, tokenIter->fLineCount,
            markupDef, '\0');
    Definition* markupChild = &markupDef->fTokens.back();
    {
        auto mapIter = fIClassMap.find(markupDef->fName);
        SkASSERT(fIClassMap.end() != mapIter);
        IClassDefinition& classDef = mapIter->second;
        SkASSERT(classDef.fStart);
        string uniqueName = this->uniqueName(classDef.fMethods, nameStr);
        markupChild->fName = uniqueName;
        if (!this->findComments(*child, markupChild)) {
            return false;
        }
        if (markupChild->fUndocumented) {
            tokenIter->fUndocumented = true;
        } else {
            classDef.fMethods[uniqueName] = markupChild;
        }
    }
    return true;
}

bool IncludeParser::parseObjects(Definition* parent, Definition* markupDef) {
    fPriorObject = nullptr;
    for (auto child : parent->fChildren) {
        if (!this->parseObject(child, markupDef)) {
            return false;
        }
        fPriorObject = child;
    }
    return true;
}

bool IncludeParser::parseObject(Definition* child, Definition* markupDef) {
    // set up for error reporting
    fLine = fChar = child->fStart;
    fEnd = child->fContentEnd;
    // todo: put original line number in child as well
    switch (child->fType) {
        case Definition::Type::kKeyWord:
            switch (child->fKeyWord) {
                case KeyWord::kClass:
                    if (!this->parseClass(child, IsStruct::kNo)) {
                        return false;
                    }
                    break;
                case KeyWord::kStatic:
                case KeyWord::kConst:
                case KeyWord::kConstExpr:
                    if (!this->parseConst(child, markupDef)) {
                        return child->reportError<bool>("failed to parse const or constexpr");
                    }
                    break;
                case KeyWord::kEnum:
                    if (!this->parseEnum(child, markupDef)) {
                        return child->reportError<bool>("failed to parse enum");
                    }
                    break;
                case KeyWord::kStruct:
                    if (!this->parseClass(child, IsStruct::kYes)) {
                        return child->reportError<bool>("failed to parse struct");
                    }
                    break;
                case KeyWord::kTemplate:
                    if (!this->parseTemplate(child, markupDef)) {
                        return child->reportError<bool>("failed to parse template");
                    }
                    break;
                case KeyWord::kTypedef:
                    if (!this->parseTypedef(child, markupDef)) {
                        return child->reportError<bool>("failed to parse typedef");
                    }
                    break;
                case KeyWord::kUnion:
                    if (!this->parseUnion()) {
                        return child->reportError<bool>("failed to parse union");
                    }
                    break;
                case KeyWord::kUsing:
                    if (!this->parseUsing()) {
                        return child->reportError<bool>("failed to parse using");
                    }
                    break;
                default:
                    return child->reportError<bool>("unhandled keyword");
            }
            break;
        case Definition::Type::kBracket:
            switch (child->fBracket) {
                case Bracket::kParen:
                    {
                        auto tokenIter = child->fParent->fTokens.begin();
                        std::advance(tokenIter, child->fParentIndex);
                        tokenIter = std::prev(tokenIter);
                        TextParser previousToken(&*tokenIter);
                        if (this->isMember(*tokenIter)) {
                            break;
                        }
                        if (Bracket::kPound == child->fParent->fBracket &&
                                KeyWord::kIf == child->fParent->fKeyWord) {
                            // TODO: this will skip methods named defined() -- for the
                            // moment there aren't any
                            if (previousToken.startsWith("defined")) {
                                break;
                            }
                        }
                        if (previousToken.startsWith("sizeof") && 6 == previousToken.lineLength()) {
                            break;
                        }
                    }
                    if (fPriorObject && MarkType::kConst == fPriorObject->fMarkType) {
                        break;
                    }
                    if (!this->parseMethod(child, markupDef)) {
                        return child->reportError<bool>("failed to parse method");
                    }
                break;
                case Bracket::kSlashSlash:
                case Bracket::kSlashStar:
                    // comments are picked up by parsing objects first
                    break;
                case Bracket::kPound:
                    // special-case the #xxx xxx_DEFINED entries
                    switch (child->fKeyWord) {
                        case KeyWord::kIf:
                        case KeyWord::kIfndef:
                        case KeyWord::kIfdef:
                            if (child->boilerplateIfDef()) {
                                if (!this->parseObjects(child, markupDef)) {
                                    return false;
                                }
                                break;
                            }
                            goto preproError;
                        case KeyWord::kDefine:
                            if (this->parseDefine(child, markupDef)) {
                                break;
                            }
                            goto preproError;
                        case KeyWord::kEndif:
                            if (child->boilerplateEndIf()) {
                                break;
                            }
                        case KeyWord::kError:
                        case KeyWord::kInclude:
                            // ignored for now
                            break;
                        case KeyWord::kElse:
                            if (!this->parseObjects(child, markupDef)) {
                                return false;
                            }
                            break;
                        case KeyWord::kElif:
                            // todo: handle these
                            break;
                        default:
                        preproError:
                            return child->reportError<bool>("unhandled preprocessor");
                    }
                    break;
                case Bracket::kAngle:
                    // pick up templated function pieces when method is found
                    break;
                case Bracket::kDebugCode:
                    if (!this->parseObjects(child, markupDef)) {
                        return false;
                    }
                    break;
                case Bracket::kSquare: {
                    // check to see if parent is operator, the only case we handle so far
                    auto prev = child->fParent->fTokens.begin();
                    std::advance(prev, child->fParentIndex - 1);
                    if (KeyWord::kOperator != prev->fKeyWord) {
                        return child->reportError<bool>("expected operator overload");
                    }
                    } break;
                default:
                    return child->reportError<bool>("unhandled bracket");
            }
            break;
        case Definition::Type::kWord:
            if (MarkType::kMember != child->fMarkType) {
                return child->reportError<bool>("unhandled word type");
            }
            if (!this->parseMember(child, markupDef)) {
                return child->reportError<bool>("unparsable member");
            }
            break;
        default:
            return child->reportError<bool>("unhandled type");
            break;
    }
    return true;
}

bool IncludeParser::parseTemplate(Definition* child, Definition* markupDef) {
    return this->parseObjects(child, markupDef);
}

bool IncludeParser::parseTypedef(Definition* child, Definition* markupDef) {
    TextParser typedefParser(child);
    typedefParser.skipExact("typedef");
    typedefParser.skipWhiteSpace();
    string nameStr = typedefParser.typedefName();
    if (!markupDef) {
        fGlobals.emplace_back(MarkType::kTypedef, child->fContentStart, child->fContentEnd,
                child->fLineCount, fParent, '\0');
        Definition* globalMarkupChild = &fGlobals.back();
        string globalUniqueName = this->uniqueName(fITypedefMap, nameStr);
        globalMarkupChild->fName = globalUniqueName;
        if (!this->findComments(*child, globalMarkupChild)) {
            return false;
        }
        if (globalMarkupChild->fUndocumented) {
            child->fUndocumented = true;
        } else {
            fITypedefMap[globalUniqueName] = globalMarkupChild;
        }
        child->fName = nameStr;
        return true;
    }
    markupDef->fTokens.emplace_back(MarkType::kTypedef, child->fContentStart, child->fContentEnd,
        child->fLineCount, markupDef, '\0');
    Definition* markupChild = &markupDef->fTokens.back();
    markupChild->fName = nameStr;
    this->checkName(markupChild);
    markupChild->fTerminator = markupChild->fContentEnd;
    IClassDefinition& classDef = fIClassMap[markupDef->fName];
    classDef.fTypedefs[nameStr] = markupChild;
    child->fName = markupDef->fName + "::" + nameStr;
    this->checkName(child);
    fITypedefMap[child->fName] = markupChild;
    return true;
}

bool IncludeParser::parseUnion() {
    // incomplete
    return true;
}

bool IncludeParser::parseUsing() {
    // incomplete
    return true;
}

bool IncludeParser::parseChar() {
    char test = *fChar;
    if ('\\' == fPrev) {
        if ('\n' == test) {
//            ++fLineCount;
            fLine = fChar + 1;
        }
        goto done;
    }
    switch (test) {
        case '\n':
//            ++fLineCount;
            fLine = fChar + 1;
            if (fInChar) {
                return reportError<bool>("malformed char");
            }
            if (fInString) {
                return reportError<bool>("malformed string");
            }
            if (!this->checkForWord()) {
                return false;
            }
            if (Bracket::kPound == this->topBracket()) {
                KeyWord keyWord = fParent->fKeyWord;
                if (KeyWord::kNone == keyWord) {
                    return this->reportError<bool>("unhandled preprocessor directive");
                }
                if (fInDefine) {
                    SkASSERT(KeyWord::kDefine == keyWord);
                    fInDefine = false;
                }
                if (KeyWord::kInclude == keyWord || KeyWord::kDefine == keyWord || KeyWord::kError == keyWord) {
                    this->popBracket();
                }
                if (fInBrace) {
                    SkASSERT(KeyWord::kDefine == fInBrace->fKeyWord);
                    fInBrace = nullptr;
                }
            } else if (Bracket::kSlashSlash == this->topBracket()) {
                this->popBracket();
            }
            break;
        case '*':
            if (!fInCharCommentString && '/' == fPrev) {
                this->pushBracket(Bracket::kSlashStar);
            }
            if (!this->checkForWord()) {
                return false;
            }
            if (!fInCharCommentString) {
                this->addPunctuation(Punctuation::kAsterisk);
            }
            break;
        case '/':
            if ('*' == fPrev) {
                if (!fInCharCommentString) {
                    return reportError<bool>("malformed closing comment");
                }
                if (Bracket::kSlashStar == this->topBracket()) {
                    TextParserSave save(this);
                    this->next();  // include close in bracket
                    this->popBracket();
                    save.restore(); // put things back so nothing is skipped
                }
                break;
            }
            if (!fInCharCommentString && '/' == fPrev) {
                this->pushBracket(Bracket::kSlashSlash);
                break;
            }
            if (!this->checkForWord()) {
                return false;
            }
            break;
        case '\'':
            if (Bracket::kChar == this->topBracket()) {
                this->popBracket();
            } else if (!fInComment && !fInString) {
                if (fIncludeWord) {
                    return this->reportError<bool>("word then single-quote");
                }
                this->pushBracket(Bracket::kChar);
            }
            break;
        case '\"':
            if (Bracket::kString == this->topBracket()) {
                this->popBracket();
            } else if (!fInComment && !fInChar) {
                if (fIncludeWord) {
                    return this->reportError<bool>("word then double-quote");
                }
                this->pushBracket(Bracket::kString);
            }
            break;
        case '(':
            if (fIncludeWord && fChar - fIncludeWord >= 10 &&
                    !strncmp("SkDEBUGCODE", fIncludeWord, 10)) {
                this->pushBracket(Bracket::kDebugCode);
                break;
            }
        case ':':
        case '[':
        case '{': {
            if (fInCharCommentString) {
                break;
            }
            if (fInDefine && fInBrace) {
                break;
            }
            if (':' == test && (fInBrace || ':' == fChar[-1] || ':' == fChar[1])) {
                break;
            }
            if (fConstExpr) {
                fConstExpr->fContentEnd = fParent->fTokens.back().fContentEnd;
                fConstExpr = nullptr;
            }
            if (!fInBrace) {
                if (!this->checkForWord()) {
                    return false;
                }
                if (':' == test && !fInFunction) {
                    break;
                }
                if ('{' == test) {
                    this->addPunctuation(Punctuation::kLeftBrace);
                } else if (':' == test) {
                    this->addPunctuation(Punctuation::kColon);
                }
            }
            if (fInBrace && '{' == test && Definition::Type::kBracket == fInBrace->fType
                    && Bracket::kColon == fInBrace->fBracket) {
                Definition* braceParent = fParent->fParent;
                braceParent->fChildren.pop_back();
                braceParent->fTokens.pop_back();
                fParent = braceParent;
                fInBrace = nullptr;
            }
            this->pushBracket(
                    '(' == test ? Bracket::kParen :
                    '[' == test ? Bracket::kSquare :
                    '{' == test ? Bracket::kBrace :
                                  Bracket::kColon);
            if (!fInBrace
                    && ('{' == test || (':' == test && ' ' >= fChar[1]))
                    && fInFunction) {
                fInBrace = fParent;
            }
            } break;
        case '<':
            if (fInCharCommentString || fInBrace) {
                break;
            }
            if (!this->checkForWord()) {
                return false;
            }
            if (fInEnum) {
                break;
            }
            this->pushBracket(Bracket::kAngle);
            // this angle bracket may be an operator or may be a bracket
            // wait for balancing close angle, if any, to decide
            break;
        case ')':
        case ']':
        case '}': {
            if (fInCharCommentString) {
                break;
            }
            if (fInDefine && fInBrace) {
                break;
            }
            if (!fInBrace) {
                if (!this->checkForWord()) {
                    return false;
                }
            }
            bool popBraceParent = fInBrace == fParent;
            Bracket match = ')' == test ? Bracket::kParen :
                    ']' == test ? Bracket::kSquare : Bracket::kBrace;
            if (match == this->topBracket()) {
                this->popBracket();
                if (!fInFunction) {
                    fInFunction = ')' == test && !this->inAlignAs();
                } else {
                    fInFunction = '}' != test;
                }
            } else if (')' == test && Bracket::kDebugCode == this->topBracket()) {
                this->popBracket();
            } else if (Bracket::kAngle == this->topBracket()
                    && match == this->grandParentBracket()) {
                this->popBracket();
                this->popBracket();
            } else {
                return reportError<bool>("malformed close bracket");
            }
            if (popBraceParent) {
                Definition* braceParent = fInBrace->fParent;
                braceParent->fChildren.pop_back();
                braceParent->fTokens.pop_back();
                fInBrace = nullptr;
            }
            } break;
        case '>':
            if (fInCharCommentString || fInBrace) {
                break;
            }
            if (!this->checkForWord()) {
                return false;
            }
            if (fInEnum) {
                break;
            }
            if (Bracket::kPound == this->topBracket()) {
                break;
            }
            if (Bracket::kAngle == this->topBracket()) {
                // looks like angle pair are braces, not operators
                this->popBracket();
            } else {
                return reportError<bool>("malformed close angle bracket");
            }
            break;
        case '#': {
            if (fInCharCommentString || fInBrace) {
                break;
            }
            SkASSERT(!fIncludeWord);  // don't expect this, curious if it is triggered
            this->pushBracket(Bracket::kPound);
            break;
        }
        case ' ':
            if (fInDefine && !fInBrace && Bracket::kPound == this->topBracket()) {
                SkASSERT(KeyWord::kDefine == fParent->fKeyWord);
                fInBrace = fParent;
                // delimiting brackets are space ... unescaped-linefeed
            }
        case '&':
        case ',':
        case '+':
        case '-':
        case '!':
            if (fInCharCommentString || fInBrace) {
                break;
            }
            if (!this->checkForWord()) {
                return false;
            }
            break;
        case '=':
            if (fInCharCommentString || fInBrace) {
                break;
            }
            if (!this->checkForWord()) {
                return false;
            }
            if (!fParent->fTokens.size()) {
                break;
            }
            {
                const Definition& lastToken = fParent->fTokens.back();
                if (lastToken.fType != Definition::Type::kWord) {
                    break;
                }
                string name(lastToken.fContentStart, lastToken.length());
                if ("SK_" != name.substr(0, 3) && 'k' != name[0]) {
                    break;
                }
                // find token on start of line
                auto lineIter = fParent->fTokens.end();
                do {
                    if (fParent->fTokens.begin() == lineIter) {
                        break;
                    }
                    --lineIter;
                } while (lineIter->fContentStart > fLine);
                if (lineIter->fContentStart < fLine && fParent->fTokens.end() != lineIter) {
                    ++lineIter;
                }
                Definition* lineStart = &*lineIter;
                // walk tokens looking for [template <typename T>] [static] [const | constexpr]
                bool sawConst = false;
                bool sawStatic = false;
                bool sawTemplate = false;
                bool sawType = false;
                while (&lastToken != &*lineIter) {
                    if (KeyWord::kTemplate == lineIter->fKeyWord) {
                        if (sawConst || sawStatic || sawTemplate) {
                            sawConst = false;
                            break;
                        }
                        if (&lastToken == &*++lineIter) {
                            break;
                        }
                        if (KeyWord::kTypename != lineIter->fKeyWord) {
                            break;
                        }
                        if (&lastToken == &*++lineIter) {
                            break;
                        }
                        if (Definition::Type::kWord != lineIter->fType) {
                            break;
                        }
                        sawTemplate = true;
                    } else if (KeyWord::kStatic == lineIter->fKeyWord) {
                        if (sawConst || sawStatic) {
                            sawConst = false;
                            break;
                        }
                        sawStatic = true;
                    } else if (KeyWord::kConst == lineIter->fKeyWord
                            || KeyWord::kConstExpr == lineIter->fKeyWord) {
                        if (sawConst) {
                            sawConst = false;
                            break;
                        }
                        sawConst = true;
                    } else {
                        if (sawType) {
                            sawType = false;
                            break;
                        }
                        if (Definition::Type::kKeyWord == lineIter->fType
                                && KeyProperty::kNumber
                                == kKeyWords[(int) lineIter->fKeyWord].fProperty) {
                            sawType = true;
                        } else if (Definition::Type::kWord == lineIter->fType) {
                            string typeName(lineIter->fContentStart, lineIter->length());
                            if ("Sk" != name.substr(0, 2)) {
                                sawType = true;
                            }
                        }
                    }
                    ++lineIter;
                }
                if (sawType && sawConst) {
                    // if found, name first
                    lineStart->fName = name;
                    lineStart->fMarkType = MarkType::kConst;
                    this->checkName(lineStart);
                    fParent->fChildren.emplace_back(lineStart);
                    fConstExpr = lineStart;
                }
            }
            break;
        case ';':
            if (fInCharCommentString || fInBrace) {
                break;
            }
            if (!this->checkForWord()) {
                return false;
            }
            if (fConstExpr) {
                fConstExpr->fContentEnd = fParent->fTokens.back().fContentEnd;
                fConstExpr = nullptr;
            }
            if (Definition::Type::kKeyWord == fParent->fType
                    && KeyProperty::kObject == (kKeyWords[(int) fParent->fKeyWord].fProperty)) {
                bool parentIsClass = KeyWord::kClass == fParent->fKeyWord;
                if (parentIsClass && fParent->fParent &&
                        KeyWord::kEnum == fParent->fParent->fKeyWord) {
                    this->popObject();
                }
                if (KeyWord::kEnum == fParent->fKeyWord) {
                    fInEnum = false;
                }
                parentIsClass |= KeyWord::kStruct == fParent->fKeyWord;
                this->popObject();
                if (parentIsClass && fParent && KeyWord::kTemplate == fParent->fKeyWord) {
                    this->popObject();
                }
                fPriorEnum = nullptr;
            } else if (Definition::Type::kBracket == fParent->fType
                    && fParent->fParent && Definition::Type::kKeyWord == fParent->fParent->fType
                    && KeyWord::kStruct == fParent->fParent->fKeyWord) {
                list<Definition>::iterator baseIter = fParent->fTokens.end();
                list<Definition>::iterator namedIter  = fParent->fTokens.end();
                for (auto tokenIter = fParent->fTokens.end();
                        fParent->fTokens.begin() != tokenIter; ) {
                    --tokenIter;
                    if (tokenIter->fLineCount == fLineCount) {
                        if (this->isMember(*tokenIter)) {
                            if (namedIter != fParent->fTokens.end()) {
                                return reportError<bool>("found two named member tokens");
                            }
                            namedIter = tokenIter;
                        }
                        baseIter = tokenIter;
                    } else {
                        break;
                    }
                }
                // FIXME: if a member definition spans multiple lines, this won't work
                if (namedIter != fParent->fTokens.end()) {
                    if (baseIter == namedIter) {
                        return this->reportError<bool>("expected type before named token");
                    }
                    Definition* member = &*namedIter;
                    member->fMarkType = MarkType::kMember;
                    if (!member->fTerminator) {
                        member->fTerminator = member->fContentEnd;
                    }
                    fParent->fChildren.push_back(member);
                    for (auto nameType = baseIter; nameType != namedIter; ++nameType) {
                        member->fChildren.push_back(&*nameType);
                    }
                }
                fPriorEnum = nullptr;
            } else if (fParent->fChildren.size() > 0) {
                auto lastIter = fParent->fChildren.end();
                Definition* priorEnum = fPriorEnum;
                fPriorEnum = nullptr;
                if (!priorEnum) {
                    while (fParent->fChildren.begin() != lastIter) {
                        std::advance(lastIter, -1);
                        priorEnum = *lastIter;
                        if (Definition::Type::kBracket != priorEnum->fType ||
                                (Bracket::kSlashSlash != priorEnum->fBracket
                                && Bracket::kSlashStar != priorEnum->fBracket)) {
                            break;
                        }
                    }
                    fPriorIndex = priorEnum->fParentIndex;
                }
                if (Definition::Type::kKeyWord == priorEnum->fType
                        && KeyWord::kEnum == priorEnum->fKeyWord) {
                    auto tokenWalker = fParent->fTokens.begin();
                    std::advance(tokenWalker, fPriorIndex);
                    while (tokenWalker != fParent->fTokens.end()) {
                        std::advance(tokenWalker, 1);
                        ++fPriorIndex;
                        if (Punctuation::kSemicolon == tokenWalker->fPunctuation) {
                            break;
                        }
                    }
                    while (tokenWalker != fParent->fTokens.end()) {
                        std::advance(tokenWalker, 1);
                        const Definition* test = &*tokenWalker;
                        if (Definition::Type::kBracket != test->fType ||
                                (Bracket::kSlashSlash != test->fBracket
                                && Bracket::kSlashStar != test->fBracket)) {
                            break;
                        }
                    }
                    auto saveTokenWalker = tokenWalker;
                    Definition* start = &*tokenWalker;
                    bool foundExpected = true;
                    for (KeyWord expected : {KeyWord::kStatic, KeyWord::kConstExpr, KeyWord::kInt}){
                        const Definition* test = &*tokenWalker;
                        if (expected != test->fKeyWord) {
                            foundExpected = false;
                            break;
                        }
                        if (tokenWalker == fParent->fTokens.end()) {
                            break;
                        }
                        std::advance(tokenWalker, 1);
                    }
                    if (!foundExpected) {
                        foundExpected = true;
                        tokenWalker = saveTokenWalker;
                        for (KeyWord expected : {KeyWord::kStatic, KeyWord::kConst, KeyWord::kNone}){
                            const Definition* test = &*tokenWalker;
                            if (expected != test->fKeyWord) {
                                foundExpected = false;
                                break;
                            }
                            if (tokenWalker == fParent->fTokens.end()) {
                                break;
                            }
                            if (KeyWord::kNone != expected) {
                                std::advance(tokenWalker, 1);
                            }
                        }
                        if (foundExpected) {
                            auto nameToken = priorEnum->fTokens.begin();
                            string enumName = string(nameToken->fContentStart,
                                    nameToken->fContentEnd - nameToken->fContentStart);
                            const Definition* test = &*tokenWalker;
                            string constType = string(test->fContentStart,
                                    test->fContentEnd - test->fContentStart);
                            if (enumName != constType) {
                                foundExpected = false;
                            } else {
                                std::advance(tokenWalker, 1);
                            }
                        }
                    }
                    if (foundExpected && tokenWalker != fParent->fTokens.end()) {
                        const char* nameStart = tokenWalker->fStart;
                        std::advance(tokenWalker, 1);
                        if (tokenWalker != fParent->fTokens.end()) {
                            TextParser tp(fFileName, nameStart, tokenWalker->fStart, fLineCount);
                            tp.skipToNonName();
                            start->fName = string(nameStart, tp.fChar - nameStart);
                            this->checkName(start);
                            start->fContentEnd = fChar;
                            priorEnum->fChildren.emplace_back(start);
                            fPriorEnum = priorEnum;
                        }
                    }
                }
            }
            this->addPunctuation(Punctuation::kSemicolon);
            fInFunction = false;
            break;
        case '~':
            if (fInEnum) {
                break;
            }
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            // TODO: don't want to parse numbers, but do need to track for enum defs
        //    break;
        case 'A': case 'B': case 'C': case 'D': case 'E':
        case 'F': case 'G': case 'H': case 'I': case 'J':
        case 'K': case 'L': case 'M': case 'N': case 'O':
        case 'P': case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X': case 'Y':
        case 'Z': case '_':
        case 'a': case 'b': case 'c': case 'd': case 'e':
        case 'f': case 'g': case 'h': case 'i': case 'j':
        case 'k': case 'l': case 'm': case 'n': case 'o':
        case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y':
        case 'z':
            if (fInCharCommentString || fInBrace) {
                break;
            }
            if (!fIncludeWord) {
                fIncludeWord = fChar;
            }
            break;
    }
done:
    fPrev = test;
    this->next();
    return true;
}

void IncludeParser::validate() const {
    IncludeParser::ValidateKeyWords();
}

bool IncludeParser::references(const SkString& file) const {
    // if includes weren't passed one at a time, assume all references are valid
    if (fIncludeMap.empty()) {
        return true;
    }
    SkASSERT(file.endsWith(".bmh") );
    string root(file.c_str(), file.size() - 4);
    string kReference("_Reference");
    if (string::npos != root.find(kReference)) {
        root = root.substr(0, root.length() - kReference.length());
    }
    if (fIClassMap.end() != fIClassMap.find(root)) {
        return true;
    }
    if (fIStructMap.end() != fIStructMap.find(root)) {
        return true;
    }
    if (fIEnumMap.end() != fIEnumMap.find(root)) {
        return true;
    }
    if (fITypedefMap.end() != fITypedefMap.find(root)) {
        return true;
    }
    if (fIFunctionMap.end() != fIFunctionMap.find(root)) {
        return true;
    }
    return false;
}

void IncludeParser::RemoveFile(const char* docs, const char* includes) {
    if (!sk_isdir(includes)) {
        IncludeParser::RemoveOneFile(docs, includes);
    } else {
        SkOSFile::Iter it(includes, ".h");
        for (SkString file; it.next(&file); ) {
            SkString p = SkOSPath::Join(includes, file.c_str());
            const char* hunk = p.c_str();
            if (!SkStrEndsWith(hunk, ".h")) {
                continue;
            }
            IncludeParser::RemoveOneFile(docs, hunk);
        }
    }
}

void IncludeParser::RemoveOneFile(const char* docs, const char* includesFile) {
    const char* lastForward = strrchr(includesFile, '/');
    const char* lastBackward = strrchr(includesFile, '\\');
    const char* last = lastForward > lastBackward ? lastForward : lastBackward;
    if (!last) {
        last = includesFile;
    } else {
        last += 1;
    }
    SkString baseName(last);
    SkASSERT(baseName.endsWith(".h"));
    baseName.remove(baseName.size() - 2, 2);
    baseName.append("_Reference.bmh");
    SkString fullName = docs ? SkOSPath::Join(docs, baseName.c_str()) : baseName;
    remove(fullName.c_str());
}

static const char kMethodMissingStr[] =
    "If the method requires documentation, add to "
    "%s at minimum:\n"  // path to bmh file
    "\n"
    "#Method %s\n" // method declaration less implementation details
    "#In  SomeSubtopicName\n"
    "#Line # add a one line description here ##\n"
    "#Populate\n"
    "#NoExample\n"
    "// or better yet, use #Example and put C++ code here\n"
    "##\n"
    "#SeeAlso optional related symbols\n"
    "#Method ##\n"
    "\n"
    "Add to %s, at minimum:\n"  // path to include
    "\n"
    "/** (description) Starts with present tense action verb\n"
    "    and end with a period.\n"
    "%s"   // @param, @return if needed go here
    "*/\n"
    "%s ...\n" // method declaration
    "\n"
    "If the method does not require documentation,\n"
    "add \"private\" or \"experimental\", as in:\n"
    "\n"
    "/** Experimental, do not use. And so on...\n"
    "*/\n"
    "%s ...\n" // method declaration
    "\n"
    ;

// bDef does not have #Populate
static const char kMethodDiffersNoPopStr[] =
    "In %s:\n"              // path to bmh file
    "#Method %s\n"          // method declaration less implementation details
    "does not match doxygen comment of:\n"
    "%s.\n"                 // method declaration
    "\n"
    ;

static const char kMethodDiffersStr[] =
    "In %s:\n"                        // path to include
    "%s\n"                            // method declaration
    "does not match doxygen comment.\n"
    "\n"
    ;

void IncludeParser::suggestFix(Suggest suggest, const Definition& iDef,
        const RootDefinition* root, const Definition* bDef) {
    string methodNameStr(iDef.fContentStart, iDef.length());
    const char* methodName = methodNameStr.c_str();
    TextParser lessImplParser(&iDef);
    if (lessImplParser.skipExact("static")) {
        lessImplParser.skipWhiteSpace();
    }
    // TODO : handle debug wrapper
    /* bool inDebugWrapper = */ Definition::SkipImplementationWords(lessImplParser);
    string lessImplStr(lessImplParser.fChar, lessImplParser.fEnd - lessImplParser.fChar);
    const char* methodNameLessImpl = lessImplStr.c_str();
    // return result, if any is substr from 0 to location of iDef.fName
    size_t namePos = methodNameStr.find(iDef.fName);
    SkASSERT(string::npos != namePos);
    size_t funcEnd = namePos;
    while (funcEnd > 0 && ' ' >= methodNameStr[funcEnd - 1]) {
        funcEnd -= 1;
    }
    string funcRet = methodNameStr.substr(0, funcEnd);
// parameters, if any, are delimited by () and separate by ,
    TextParser parser(&iDef);
    parser.fChar += namePos + iDef.fName.length();
    const char* start = parser.fChar;
    vector<string> paramStrs;
    if ('(' == start[0]) {
        parser.skipToBalancedEndBracket('(', ')');
        TextParser params(&iDef);
        params.fChar = start + 1;
        params.fEnd = parser.fChar;
        while (!params.eof()) {
            const char* paramEnd = params.anyOf("=,)");
            const char* paramStart = paramEnd;
            while (paramStart > params.fChar && ' ' >= paramStart[-1]) {
                paramStart -= 1;
            }
            while (paramStart > params.fChar && (isalnum(paramStart[-1])
                    || '_' == paramStart[-1])) {
                paramStart -= 1;
            }
            string param(paramStart, paramEnd - paramStart);
            paramStrs.push_back(param);
            params.fChar = params.anyOf(",)") + 1;
        }
    }
    string bmhFile = root ? root->fFileName : bDef ? bDef->fFileName : "a *.bmh file";
    bool hasFuncReturn = "" != funcRet && "void" != funcRet;
    switch(suggest) {
        case Suggest::kMethodMissing: {
            // if include @param, @return are missing, request them as well
            string paramDox;
            bool firstParam = true;
            for (auto paramStr : paramStrs) {
                if (firstParam) {
                    paramDox += "\n";
                    firstParam = false;
                }
                paramDox += "    @param " + paramStr + "  descriptive phrase\n";
            }
            if (hasFuncReturn) {
                paramDox += "\n";
                paramDox += "    @return descriptive phrase\n";
            }
            SkDebugf(kMethodMissingStr, bmhFile.c_str(), methodNameLessImpl, iDef.fFileName.c_str(),
                    paramDox.c_str(), methodName, methodName);
            } break;
        case Suggest::kMethodDiffers: {
            bool hasPop = std::any_of(bDef->fChildren.begin(), bDef->fChildren.end(),
                    [](Definition* def) { return MarkType::kPopulate == def->fMarkType; });
            if (!hasPop) {
                SkDebugf(kMethodDiffersNoPopStr, bmhFile.c_str(), methodNameLessImpl, methodName);
            }
            SkDebugf(kMethodDiffersStr, iDef.fFileName.c_str(), methodName);
            } break;
        default:
            SkASSERT(0);
    }
}

Bracket IncludeParser::topBracket() const {
    Definition* parent = this->parentBracket(fParent);
    return parent ? parent->fBracket : Bracket::kNone;
}
