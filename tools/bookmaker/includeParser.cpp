/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bookmaker.h"
#include "SkOSFile.h"
#include "SkOSPath.h"

const char IncludeParser::gAttrDeprecated[] = "SK_ATTR_DEPRECATED";
const size_t IncludeParser::kAttrDeprecatedLen = sizeof(gAttrDeprecated) - 1;

const IncludeKey kKeyWords[] = {
    { "",           KeyWord::kNone,         KeyProperty::kNone           },
    { "SK_API",     KeyWord::kSK_API,       KeyProperty::kModifier       },
    { "SK_BEGIN_REQUIRE_DENSE", KeyWord::kSK_BEGIN_REQUIRE_DENSE, KeyProperty::kModifier },
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
    { "void",       KeyWord::kVoid,         KeyProperty::kNumber         },
};

const size_t kKeyWordCount = SK_ARRAY_COUNT(kKeyWords);

KeyWord IncludeParser::FindKey(const char* start, const char* end) {
    int ch = 0;
    for (size_t index = 0; index < kKeyWordCount; ) {
        if (start[ch] > kKeyWords[index].fName[ch]) {
            ++index;
            if (ch > 0 && kKeyWords[index - 1].fName[ch - 1] < kKeyWords[index].fName[ch - 1]) {
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
        case KeyWord::kElse: {
            this->popObject();  // pop elif
            if (Bracket::kPound != fParent->fBracket) {
                return this->reportError<bool>("expected preprocessor directive");
            }
            this->popBracket();  // pop if
            poundDef->fParent = fParent;
            this->addDefinition(poundDef);  // push elif back
        } break;
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

#include <sstream>
#include <iostream>

bool IncludeParser::crossCheck(BmhParser& bmhParser) {
    for (auto& classMapper : fIClassMap) {
        string className = classMapper.first;
        auto finder = bmhParser.fClassMap.find(className);
        if (bmhParser.fClassMap.end() == finder) {
            SkASSERT(string::npos != className.find("::"));
            continue;
        }
        RootDefinition* root = &finder->second;
        root->clearVisited();
    }
    for (auto& classMapper : fIClassMap) {
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
        auto& classMap = classMapper.second;
        auto& tokens = classMap.fTokens;
        for (const auto& token : tokens) {
            if (token.fPrivate) {
                continue;
            }
            string fullName = classMapper.first + "::" + token.fName;
            const Definition* def = root->find(fullName, RootDefinition::AllowParens::kYes);
            switch (token.fMarkType) {
                case MarkType::kMethod: {
                    if (this->isInternalName(token)) {
                        continue;
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
                        if (gAttrDeprecated == token.fName) {
                            fAttrDeprecated = &token;
                            break;
                        }
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
                        if (!root->fDeprecated) {
                            SkDebugf("method missing from bmh: %s\n", fullName.c_str());
                            fFailed = true;
                        }
                        break;
                    }
                    if (def->crossCheck2(token)) {
                        def->fVisited = true;
                        if (token.fDeprecated && !def->fDeprecated) {
                            fFailed = !def->reportError<bool>("expect bmh to be marked deprecated");
                        }
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
                            string anonName = className + "::" + string(lastUnderscore,
                                    wordEnd - lastUnderscore) + 's';
                            def = root->find(anonName, RootDefinition::AllowParens::kYes);
                        }
                        if (!def) {
                            if (!root->fDeprecated) {
                                SkDebugf("enum missing from bmh: %s\n", fullName.c_str());
                                fFailed = true;
                            }
                            break;
                        }
                    }
                    def->fVisited = true;
                    for (auto& child : def->fChildren) {
                        if (MarkType::kCode == child->fMarkType) {
                            def = child;
                            break;
                        }
                    }
                    if (MarkType::kCode != def->fMarkType) {
                        if (!root->fDeprecated) {
                            SkDebugf("enum code missing from bmh: %s\n", fullName.c_str());
                            fFailed = true;
                        }
                        break;
                    }
                    if (def->crossCheck(token)) {
                        def->fVisited = true;
                    } else {
                        SkDebugf("enum differs from bmh: %s\n", def->fName.c_str());
                        fFailed = true;
                    }
                    for (auto& child : token.fChildren) {
                        string constName = MarkType::kEnumClass == token.fMarkType ?
                                fullName : className;
                        constName += "::" + child->fName;
                        def = root->find(constName, RootDefinition::AllowParens::kYes);
                        if (!def) {
                            string innerName = classMapper.first + "::" + child->fName;
                            def = root->find(innerName, RootDefinition::AllowParens::kYes);
                        }
                        if (!def) {
                            if (string::npos == child->fName.find("Legacy_")) {
                                if (!root->fDeprecated) {
                                    SkDebugf("const missing from bmh: %s\n", constName.c_str());
                                    fFailed = true;
                                }
                            }
                        } else {
                            def->fVisited = true;
                        }
                    }
                    } break;
                case MarkType::kMember:
                    if (def) {
                        def->fVisited = true;
                    } else if (!root->fDeprecated) {
                        SkDebugf("member missing from bmh: %s\n", fullName.c_str());
                        fFailed = true;
                    }
                    break;
                case MarkType::kTypedef:
                    if (def) {
                        def->fVisited = true;
                    } else if (!root->fDeprecated) {
                        SkDebugf("typedef missing from bmh: %s\n", fullName.c_str());
                        fFailed = true;
                    }
                    break;
                case MarkType::kConst:
                    if (def) {
                        def->fVisited = true;
                    } else if (!root->fDeprecated) {
                        SkDebugf("const missing from bmh: %s\n", fullName.c_str());
                        fFailed = true;
                    }
                    break;
                default:
                    SkASSERT(0);  // unhandled
                    break;
            }
        }
    }
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
    markupDef.fContentEnd = includeDef.fContentEnd;
    markupDef.fTerminator = includeDef.fTerminator;
    markupDef.fParent = fParent;
    markupDef.fLineCount = fLineCount;
    markupDef.fMarkType = KeyWord::kStruct == includeDef.fKeyWord ?
            MarkType::kStruct : MarkType::kClass;
    markupDef.fKeyWord = includeDef.fKeyWord;
    markupDef.fType = Definition::Type::kMark;
    fParent = &markupDef;
    return &markupDef;
}

void IncludeParser::dumpClassTokens(IClassDefinition& classDef) {
    auto& tokens = classDef.fTokens;
    for (auto& token : tokens) {
        if (Definition::Type::kMark == token.fType && MarkType::kComment == token.fMarkType) {
            continue;
        }
        if (MarkType::kMember != token.fMarkType) {
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
                this->dumpMethod(token, classDef.fName);
            break;
            case MarkType::kMember:
                this->dumpMember(token);
                continue;
            break;
            default:
                SkASSERT(0);
        }
        this->dumpCommonTail(token);
    }
}
void IncludeParser::dumpComment(const Definition& token) {
    fLineCount = token.fLineCount;
    fChar = fLine = token.fContentStart;
    fEnd = token.fContentEnd;
    bool sawParam = false;
    bool multiline = false;
    bool sawReturn = false;
    bool sawComment = false;
    bool methodHasReturn = false;
    vector<string> methodParams;
    vector<string> foundParams;
    Definition methodName;
    TextParser methodParser(token.fFileName, token.fContentStart, token.fContentEnd,
            token.fLineCount);
    bool debugCode = methodParser.skipExact("SkDEBUGCODE(");
    if (MarkType::kMethod == token.fMarkType) {
        methodName.fName = debugCode ? token.fName : string(token.fContentStart,
                (int) (token.fContentEnd - token.fContentStart));
        methodHasReturn = !methodParser.startsWith("void ")
                && !methodParser.startsWith("static void ")
                && !methodParser.strnchr('~', methodParser.fEnd);
        const char* paren = methodParser.strnchr('(', methodParser.fEnd);
        const char* nextEnd = paren;
        do {
            string paramName;
            methodParser.fChar = nextEnd + 1;
            methodParser.skipSpace();
            if (!methodName.nextMethodParam(&methodParser, &nextEnd, &paramName)) {
                continue;
            }
            methodParams.push_back(paramName);
        } while (')' != nextEnd[0]);
    }
    for (const auto& child : token.fTokens) {
        if (Definition::Type::kMark == child.fType && MarkType::kMember == child.fMarkType) {
            break;
        }
        if (Definition::Type::kMark == child.fType && MarkType::kComment == child.fMarkType) {
            if (child.fPrivate) {
                break;
            }
            if ('@' == child.fContentStart[0]) {
                TextParser parser(&child);
                do {
                    parser.next();
                    if (parser.startsWith("param ")) {
                        parser.skipWord("param");
                        const char* parmStart = parser.fChar;
                        parser.skipToSpace();
                        string parmName = string(parmStart, (int) (parser.fChar - parmStart));
                        parser.skipWhiteSpace();
                        do {
                            size_t nextComma = parmName.find(',');
                            string piece;
                            if (string::npos == nextComma) {
                                piece = parmName;
                                parmName = "";
                            } else {
                                piece = parmName.substr(0, nextComma);
                                parmName = parmName.substr(nextComma + 1);
                            }
                            if (sawParam) {
                                if (multiline) {
                                    this->lf(1);
                                }
                                this->writeEndTag();
                            } else {
                                if (sawComment) {
                                    this->nl();
                                }
                                this->lf(2);
                            }
                            foundParams.emplace_back(piece);
                            this->writeTag("Param", piece);
                            this->writeSpace(2);
                            this->writeBlock(parser.fEnd - parser.fChar, parser.fChar);
                            this->lf(1);
                            sawParam = true;
                            sawComment = false;
                        } while (parmName.length());
                        parser.skipTo(parser.fEnd);
                    } else if (parser.startsWith("return ") || parser.startsWith("returns ")) {
                        parser.skipWord("return");
                        if ('s' == parser.peek()) {
                            parser.next();
                        }
                        if (sawParam) {
                            if (multiline) {
                                this->lf(1);
                            }
                            this->writeEndTag();
                        }
                        this->checkForMissingParams(methodParams, foundParams);
                        sawParam = false;
                        sawComment = false;
                        multiline = false;
                        this->lf(2);
                        this->writeTag("Return");
                        this->writeSpace(2);
                        this->writeBlock(parser.fEnd - parser.fChar, parser.fChar);
                        this->lf(1);
                        sawReturn = true;
                        parser.skipTo(parser.fEnd);
                    } else {
                        this->reportError("unexpected doxygen directive");
                    }
                } while (!parser.eof());
            } else if (child.length() > 1) {
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
                    this->lfAlways(sawComment || sawParam || sawReturn ? 1 : 2);
                    if (sawParam || sawReturn) {
                        this->indentToColumn(8);
                    }
                    this->writeBlock(length, start);
                    this->writeSpace();
                    sawComment = true;
                    if (sawParam || sawReturn) {
                        multiline = true;
                    }
                }
            }
        }
    }
    if (sawParam || sawReturn) {
        if (multiline) {
            this->lf(1);
        }
        this->writeEndTag();
    }
    if (!sawReturn) {
        if (!sawParam) {
            if (sawComment) {
                this->nl();
            }
            this->lf(2);
        }
        this->checkForMissingParams(methodParams, foundParams);
    }
    if (methodHasReturn != sawReturn) {
        if (!methodHasReturn) {
            this->reportError("unexpected doxygen return");
        } else {
            if (sawComment) {
                this->nl();
            }
            this->lf(2);
            this->writeIncompleteTag("Return");
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
    this->writeTag("Enum", name);
    this->lf(2);
    this->writeTag("Code");
    this->lfAlways(1);
    this->indentToColumn(4);
    this->writeString("enum");
    this->writeSpace();
    if ("_anonymous" != token.fName.substr(0, 10)) {
        this->writeString(token.fName);
        this->writeSpace();
    }
    this->writeString("{");
    this->lfAlways(1);
    for (auto& child : token.fChildren) {
        this->indentToColumn(8);
        this->writeString(child->fName);
        if (child->length()) {
            this->writeSpace();
            this->writeBlock(child->length(), child->fContentStart);
        }
        if (',' != fLastChar) {
            this->writeString(",");
        }
        this->lfAlways(1);
    }
    this->indentToColumn(4);
    this->writeString("};");
    this->lf(1);
    this->writeString("##");
    this->lf(2);
    this->dumpComment(token);
    for (auto& child : token.fChildren) {
    //     start here;
        // get comments before
        // or after const values
        this->writeTag("Const");
        this->writeSpace();
        this->writeString(child->fName);
        TextParser val(child);
        if (!val.eof()) {
            if ('=' == val.fStart[0] || ',' == val.fStart[0]) {
                val.next();
                val.skipSpace();
                const char* valEnd = val.anyOf(",\n");
                if (!valEnd) {
                    valEnd = val.fEnd;
                }
                this->writeSpace();
                this->writeBlock(valEnd - val.fStart, val.fStart);
            } else {
                this->writeSpace();
                this->writeDefinition(*child);
            }
        }
        this->lf(1);
        for (auto comment : child->fChildren) {
            if (MarkType::kComment == comment->fMarkType) {
                TextParser parser(comment);
                parser.skipExact("*");
                parser.skipExact("*");
                while (!parser.eof() && parser.skipWhiteSpace()) {
                    parser.skipExact("*");
                    parser.skipWhiteSpace();
                    const char* start = parser.fChar;
                    parser.skipToEndBracket('\n');
                    this->lf(1);
                    this->writeBlock(parser.fChar - start, start);
                }
            }
        }
        this->writeEndTag();
    }
    this->lf(2);
}

bool IncludeParser::dumpGlobals() {
    if (fIDefineMap.empty() && fIFunctionMap.empty() && fIEnumMap.empty() && fITemplateMap.empty()
            && fITypedefMap.empty() && fIUnionMap.empty()) {
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
    this->writeTag("Subtopic", "Overview");
    this->writeTag("Populate");
    this->writeEndTag();
    this->lf(2);
    if (!fIDefineMap.empty()) {
        this->writeTag("Subtopic", "Define");
        this->writeTag("Populate");
        this->writeEndTag();
        this->lf(2);
    }
    if (!fIFunctionMap.empty()) {
        this->writeTag("Subtopic", "Function");
        this->writeTag("Populate");
        this->writeEndTag();
        this->lf(2);
    }
    if (!fIEnumMap.empty()) {
        this->writeTag("Subtopic", "Enum");
        this->writeTag("Populate");
        this->writeEndTag();
        this->lf(2);
    }
    if (!fITemplateMap.empty()) {
        this->writeTag("Subtopic", "Template");
        this->writeTag("Populate");
        this->writeEndTag();
        this->lf(2);
    }
    if (!fITypedefMap.empty()) {
        this->writeTag("Subtopic", "Typedef");
        this->writeTag("Populate");
        this->writeEndTag();
        this->lf(2);
    }
    if (!fIUnionMap.empty()) {
        this->writeTag("Subtopic", "Union");
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
        sortedDefs[entry.second->fLineCount] = entry.second;
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
                this->dumpMethod(*def, globalsName);
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
    this->writeEndTag("Topic", topicName);
    this->lfAlways(1);
    fclose(fOut);
    SkDebugf("wrote %s\n", fileName.c_str());
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

bool IncludeParser::isOperator(const Definition& token) {
    return "operator" == token.fName.substr(0, 8);
}

void IncludeParser::dumpMethod(const Definition& token, string className) {
    this->writeTag("Method");
    this->writeSpace();

    string name = string(token.fStart ? token.fStart : token.fContentStart,
            token.length());
    if (this->isOperator(token)) {
        string spaceConst(" const");
        size_t constPos = name.rfind(spaceConst);
        if (name.length() - spaceConst.length() == constPos) {
            name = name.substr(0, constPos) + "_const";
        }
    }
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
    if (!this->dumpGlobals()) {
        return false;
    }
    for (const auto& member : fIClassMap) {
        if (string::npos != member.first.find("::")) {
            continue;
        }
        if (!this->dumpTokens(member.first)) {
            return false;
        }
    }
    return true;
}

    // dump equivalent markup
bool IncludeParser::dumpTokens(string skClassName) {
    string fileName = skClassName + "_Reference.bmh";
    fOut = fopen(fileName.c_str(), "wb");
    if (!fOut) {
        SkDebugf("could not open output file %s\n", fileName.c_str());
        return false;
    }
    string prefixName = skClassName.substr(0, 2);
    string topicName = skClassName.length() > 2 && isupper(skClassName[2]) &&
        ("Sk" == prefixName || "Gr" == prefixName) ? skClassName.substr(2) : skClassName;
    this->writeTagNoLF("Topic", topicName);
    this->writeEndTag("Alias", topicName + "_Reference");
    this->lf(2);
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
    bool hasClass = false;
    bool hasConst = !fIEnumMap.empty();
    bool hasConstructor = false;
    bool hasMember = false;
    bool hasOperator = false;
    bool hasStruct = false;
    for (const auto& oneClass : fIClassMap) {
        if (skClassName + "::" != oneClass.first.substr(0, skClassName.length() + 2)) {
            continue;
        }
        hasClass = true;
        break;
    }
    for (const auto& oneStruct : fIStructMap) {
        if (skClassName + "::" != oneStruct.first.substr(0, skClassName.length() + 2)) {
            continue;
        }
        hasStruct = true;
        break;
    }
    for (const auto& token : classMap.fTokens) {
        if (Definition::Type::kMark != token.fType || MarkType::kMethod != token.fMarkType) {
            continue;
        }
        if (this->isInternalName(token)) {
            continue;
        }
        if (this->isConstructor(token, skClassName)) {
            hasConstructor = true;
            continue;
        }
        if (this->isOperator(token)) {
            hasOperator = true;
            continue;
        }
        if (this->isClone(token)) {
            continue;
        }
        hasMember = true;
    }
    this->writeTag("Subtopic", "Overview");
    this->writeTag("Populate");
    this->writeEndTag();
    this->lf(2);

    if (hasClass) {
        this->writeTag("Subtopic", "Class");
        this->writeTag("Populate");
        this->writeEndTag();
        this->lf(2);
    }
    if (hasConst) {
        this->writeTag("Subtopic", "Constant");
        this->writeTag("Populate");
        this->writeEndTag();
        this->lf(2);
    }
    if (hasConstructor) {
        this->writeTag("Subtopic", "Constructor");
        this->writeTag("Populate");
        this->writeEndTag();
        this->lf(2);
    }
    if (hasOperator) {
        this->writeTag("Subtopic", "Operator");
        this->writeTag("Populate");
        this->writeEndTag();
        this->lf(2);
    }
    if (hasMember) {
        this->writeTag("Subtopic", "Member_Function");
        this->writeTag("Populate");
        this->writeEndTag();
        this->lf(2);
    }
    if (hasStruct) {
        this->writeTag("Subtopic", "Struct");
        this->writeTag("Populate");
        this->writeEndTag();
        this->lf(2);
    }
    for (auto& oneEnum : fIEnumMap) {
        this->writeBlockSeparator();
        this->dumpEnum(*oneEnum.second, oneEnum.first);
        this->lf(2);
        this->writeTag("Example");
        this->lfcr();
        this->writeString("// incomplete");
        this->writeEndTag();
        this->lf(2);
        this->writeTag("SeeAlso", "incomplete");
        this->lf(2);
        this->writeEndTag("Enum", oneEnum.first);
        this->lf(2);
    }
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

bool IncludeParser::findComments(const Definition& includeDef, Definition* markupDef) {
    // add comment preceding class, if any
    const Definition* parent = includeDef.fParent;
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
                commentIter->fContentEnd, commentIter->fLineCount, markupDef)) {
            return false;
        }
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

// caller just returns, so report error here
bool IncludeParser::parseClass(Definition* includeDef, IsStruct isStruct) {
    SkASSERT(includeDef->fTokens.size() > 0);
    // parse class header
    auto iter = includeDef->fTokens.begin();
    if (!strncmp(iter->fStart, "SK_API", iter->fContentEnd - iter->fStart)) {
        // todo : documentation is ignoring this for now
        iter = std::next(iter);
    }
    string nameStr(iter->fStart, iter->fContentEnd - iter->fStart);
    includeDef->fName = nameStr;
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
//    if (1 != includeDef->fChildren.size()) {
//        return false;  // fix me: SkCanvasClipVisitor isn't correctly parsed
//    }
    includeDef = includeDef->fChildren.front();
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
    while ((*childIter)->fPrivate) {
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

bool IncludeParser::parseComment(string filename, const char* start, const char* end,
        int lineCount, Definition* markupDef) {
    TextParser parser(filename, start, end, lineCount);
    // parse doxygen if present
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
        fIConstMap[globalUniqueName] = globalMarkupChild;
        return true;
    }
    markupDef->fTokens.emplace_back(MarkType::kConst, child->fContentStart, child->fContentEnd,
        child->fLineCount, markupDef, '\0');
    Definition* markupChild = &markupDef->fTokens.back();
    markupChild->fName = child->fName;
    markupChild->fTerminator = markupChild->fContentEnd;
    IClassDefinition& classDef = fIClassMap[markupDef->fName];
    classDef.fConsts[child->fName] = markupChild;
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
        fIDefineMap[globalUniqueName] = globalMarkupChild;
        for (Param param : params) {
            globalMarkupChild->fTokens.emplace_back(MarkType::kParam, param.fStart, param.fEnd,
                    child->fLineCount, globalMarkupChild, '\0');
            Definition* paramChild = &globalMarkupChild->fTokens.back();
            paramChild->fName = string(param.fStart, param.fEnd - param.fStart);
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
    classDef.fDefines[nameStr] = markupChild;
    return true;
}

bool IncludeParser::parseEnum(Definition* child, Definition* markupDef) {
	TextParser parser(child);
	parser.skipToEndBracket('{');
	if (parser.eof()) {
		return true;	// if enum is a forward declaration, do nothing
	}
	parser.next();
	string nameStr;
    if (child->fTokens.size() > 0) {
        auto token = child->fTokens.begin();
        if (Definition::Type::kKeyWord == token->fType && KeyWord::kClass == token->fKeyWord) {
            token = token->fTokens.begin();
        }
        if (Definition::Type::kWord == token->fType) {
            nameStr += string(token->fStart, token->fContentEnd - token->fStart);
        }
    }
    Definition* markupChild;
    if (!markupDef) {
        fGlobals.emplace_back(MarkType::kEnum, child->fContentStart, child->fContentEnd,
                child->fLineCount, fParent, '\0');
        markupChild = &fGlobals.back();
        string globalUniqueName = this->uniqueName(fIEnumMap, nameStr);
        markupChild->fName = globalUniqueName;
        markupChild->fTerminator = child->fContentEnd;
        fIEnumMap[globalUniqueName] = markupChild;
    } else {
        markupDef->fTokens.emplace_back(MarkType::kEnum, child->fContentStart, child->fContentEnd,
            child->fLineCount, markupDef, '\0');
        markupChild = &markupDef->fTokens.back();
    }
    SkASSERT(KeyWord::kNone == markupChild->fKeyWord);
    markupChild->fKeyWord = KeyWord::kEnum;
    TextParser enumName(child);
    enumName.skipExact("enum ");
    enumName.skipWhiteSpace();
    if (enumName.skipExact("class ")) {
        enumName.skipWhiteSpace();
        markupChild->fMarkType = MarkType::kEnumClass;
    }
    const char* nameStart = enumName.fChar;
    enumName.skipToSpace();
    if (markupDef) {
        markupChild->fName = markupDef->fName + "::" +
                string(nameStart, (size_t) (enumName.fChar - nameStart));
    }
    if (!this->findComments(*child, markupChild)) {
        return false;
    }
    const char* dataEnd;
    do {
        parser.skipWhiteSpace();
        if ('}' == parser.peek()) {
            break;
        }
        Definition* comment = nullptr;
        // note that comment, if any, can be before or after (on the same line, though) as member
        if ('#' == parser.peek()) {
            // fixme: handle preprecessor, but just skip it for now
            parser.skipToLineStart();
        }
        while (parser.startsWith("/*") || parser.startsWith("//")) {
            parser.next();
            const char* start = parser.fChar;
            const char* end;
            if ('*' == parser.peek()) {
                end = parser.strnstr("*/", parser.fEnd);
                parser.fChar = end;
                parser.next();
                parser.next();
            } else {
                end = parser.trimmedLineEnd();
                parser.skipToLineStart();
            }
            markupChild->fTokens.emplace_back(MarkType::kComment, start, end, parser.fLineCount,
                    markupChild, '\0');
            comment = &markupChild->fTokens.back();
            comment->fTerminator = end;
            if (!this->parseComment(parser.fFileName, start, end, parser.fLineCount, comment)) {
                return false;
            }
            parser.skipWhiteSpace();
        }
        parser.skipWhiteSpace();
        const char* memberStart = parser.fChar;
        if ('}' == memberStart[0]) {
            break;
        }
        // if there's comment on same the line as member def, output first as if it was before

        parser.skipToNonName();
        string memberName(memberStart, parser.fChar);
        if (parser.eof() || !parser.skipWhiteSpace()) {
            return parser.reportError<bool>("enum member must end with comma 1");
        }
        const char* dataStart = parser.fChar;
        if ('=' == parser.peek()) {
            parser.skipToEndBracket(',');
        }
        if (!parser.eof() && '#' == parser.peek()) {
            // fixme: handle preprecessor, but just skip it for now
            continue;
        }
        if (parser.eof() || ',' != parser.peek()) {
            return parser.reportError<bool>("enum member must end with comma 2");
        }
        dataEnd = parser.fChar;
        const char* start = parser.anyOf("/\n");
        SkASSERT(start);
        parser.skipTo(start);
        if ('/' == parser.next()) {
            char slashStar = parser.next();
            if ('/' == slashStar || '*' == slashStar) {
                TextParserSave save(&parser);
                char doxCheck = parser.next();
                if ((slashStar != doxCheck && '!' != doxCheck) || '<' != parser.next()) {
                    save.restore();
                }
            }
            parser.skipWhiteSpace();
            const char* commentStart = parser.fChar;
            if ('/' == slashStar) {
                parser.skipToEndBracket('\n');
            } else {
                parser.skipToEndBracket("*/");
            }
            SkASSERT(!parser.eof());
            const char* commentEnd = parser.fChar;
            markupChild->fTokens.emplace_back(MarkType::kComment, commentStart, commentEnd,
                    parser.fLineCount, markupChild, '\0');
            comment = &markupChild->fTokens.back();
            comment->fTerminator = commentEnd;
        }
        markupChild->fTokens.emplace_back(MarkType::kMember, dataStart, dataEnd, parser.fLineCount,
                markupChild, '\0');
        Definition* member = &markupChild->fTokens.back();
        member->fName = memberName;
        if (comment) {
            member->fChildren.push_back(comment);
            comment->fPrivate = true;
        }
        markupChild->fChildren.push_back(member);
    } while (true);
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
        // FIXME: ? add comment as well ?
        markupChild->fChildren.push_back(member);
    }
    if (markupDef) {
        IClassDefinition& classDef = fIClassMap[markupDef->fName];
        SkASSERT(classDef.fStart);
        string uniqueName = this->uniqueName(classDef.fEnums, nameStr);
        markupChild->fName = uniqueName;
        classDef.fEnums[uniqueName] = markupChild;
    }
    return true;
}

bool IncludeParser::parseInclude(string name) {
    fParent = &fIncludeMap[name];
    fParent->fName = name;
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
    markupChild->fTerminator = markupChild->fContentEnd;
    classDef.fMembers[uniqueName] = markupChild;
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
    if (string::npos != nameStr.find("sizeof")) {
        SkDebugf("");
    }
    if (addConst) {
        nameStr += "_const";
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
    tokenIter->fPrivate = string::npos != nameStr.find("::");
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
        while (testIter != child->fParent->fTokens.end()) {
            testIter = std::next(testIter);
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
        }
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
        fIFunctionMap[globalUniqueName] = globalMarkupChild;
        return true;
    }
    markupDef->fTokens.emplace_back(MarkType::kMethod, start, end, tokenIter->fLineCount,
            markupDef, '\0');
    Definition* markupChild = &markupDef->fTokens.back();
    // TODO: I wonder if there is a way to prevent looking up by operator[] (creating empty) ?
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
        classDef.fMethods[uniqueName] = markupChild;
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
                default:
                    return child->reportError<bool>("unhandled keyword");
            }
            break;
        case Definition::Type::kBracket:
            switch (child->fBracket) {
                case Bracket::kParen:
                    if (fLastObject) {
                        TextParser checkDeprecated(child->fFileName, fLastObject->fTerminator + 1,
                                child->fStart, fLastObject->fLineCount);
                        if (!checkDeprecated.eof()) {
                            checkDeprecated.skipWhiteSpace();
                            if (checkDeprecated.startsWith(gAttrDeprecated)) {
                                fAttrDeprecated = child;
                                break;
                            }
                        }
                    }
                    {
                        auto tokenIter = child->fParent->fTokens.begin();
                        std::advance(tokenIter, child->fParentIndex);
                        tokenIter = std::prev(tokenIter);
                        TextParser previousToken(&*tokenIter);
                        if (previousToken.startsWith(gAttrDeprecated)) {
                            fAttrDeprecated = &*tokenIter;
                            break;
                        }
                        if ('f' == previousToken.fStart[0] && isupper(previousToken.fStart[1])) {
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
                    if (fAttrDeprecated) {
                        Definition* lastMethod = &markupDef->fTokens.back();
                        lastMethod->fDeprecated = true;
                        fAttrDeprecated = nullptr;
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
        fITypedefMap[globalUniqueName] = globalMarkupChild;
        child->fName = nameStr;
        return true;
    }
    markupDef->fTokens.emplace_back(MarkType::kTypedef, child->fContentStart, child->fContentEnd,
        child->fLineCount, markupDef, '\0');
    Definition* markupChild = &markupDef->fTokens.back();
    markupChild->fName = nameStr;
    markupChild->fTerminator = markupChild->fContentEnd;
    IClassDefinition& classDef = fIClassMap[markupDef->fName];
    classDef.fTypedefs[nameStr] = markupChild;
    child->fName = markupDef->fName + "::" + nameStr;
    return true;
}

bool IncludeParser::parseUnion() {

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
                    fInFunction = ')' == test;
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
                        if ('f' == tokenIter->fStart[0] && isupper(tokenIter->fStart[1])) {
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
    SkString fullName = SkOSPath::Join(docs, baseName.c_str());
    remove(fullName.c_str());
}

Bracket IncludeParser::topBracket() const {
    Definition* parent = this->parentBracket(fParent);
    return parent ? parent->fBracket : Bracket::kNone;
}
