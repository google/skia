/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bookmaker.h"

enum class KeyProperty {
    kNone,
    kClassSection,
    kFunction,
    kModifier,
    kNumber,
    kObject,
    kPreprocessor,
};

struct IncludeKey {
    const char* fName;
    KeyWord fKeyWord;
    KeyProperty fProperty;
};

const IncludeKey kKeyWords[] = {
    { "",           KeyWord::kNone,         KeyProperty::kNone           },
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
    { "uint32_t",   KeyWord::kUint32_t,     KeyProperty::kNumber         },
    { "union",      KeyWord::kUnion,        KeyProperty::kObject         },
    { "unsigned",   KeyWord::kUnsigned,     KeyProperty::kNumber         },
    { "void",       KeyWord::kVoid,         KeyProperty::kNumber         },
};

const size_t kKeyWordCount = SK_ARRAY_COUNT(kKeyWords);

KeyWord InterfaceParser::FindKey(const char* start, const char* end) {
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
            return kKeyWords[index].fKeyWord;
        }
    }
    return KeyWord::kNone;
}

void InterfaceParser::ValidateKeyWords() {
    for (size_t index = 1; index < kKeyWordCount; ++index) {
        SkASSERT((int) kKeyWords[index - 1].fKeyWord + 1
                == (int) kKeyWords[index].fKeyWord);
        SkASSERT(0 > strcmp(kKeyWords[index - 1].fName, kKeyWords[index].fName));
    }
}

void InterfaceParser::addKeyword(KeyWord keyWord) {
    fParent->fTokens.emplace_back(keyWord, fIncludeWord, fChar, fLineCount, fParent);
    fIncludeWord = nullptr;
    if (KeyProperty::kObject == kKeyWords[(int) keyWord].fProperty) {
        Definition* def = &fParent->fTokens.back();
        this->addDefinition(def);
        if (KeyWord::kEnum == fParent->fKeyWord) {
            fInEnum = true;
        }
    }
}

void InterfaceParser::checkForMissingParams(const vector<string>& methodParams, 
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
            this->keywordStart("Param");
            fprintf(fOut, "%s  ", methodParam.c_str());
            this->keywordEnd();
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

bool InterfaceParser::checkForWord() {
    if (!fIncludeWord) {
        return true;
    }
    KeyWord keyWord = FindKey(fIncludeWord, fChar);
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
        case KeyWord::kInclude:
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

string InterfaceParser::className() const {
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

bool InterfaceParser::crossCheck(BmhParser& bmhParser) {
    string className = this->className();
    string classPrefix = className + "::";
    RootDefinition* root = &bmhParser.fClassMap[className];
    root->clearVisited();
    for (auto& classMapper : fIClassMap) {
        if (className != classMapper.first
                && classPrefix != classMapper.first.substr(0, classPrefix.length())) {
            continue;
        }
        auto& classMap = classMapper.second;
        auto& tokens = classMap.fTokens;
        for (const auto& token : tokens) {
            if (token.fPrivate) {
                continue;
            }
            string fullName = classMapper.first + "::" + token.fName;
            const Definition* def = root->find(fullName);
            switch (token.fMarkType) {
                case MarkType::kMethod: {
                    if (0 == token.fName.find("internal_")
                            || 0 == token.fName.find("Internal_")
                            || 0 == token.fName.find("legacy_")
                            || 0 == token.fName.find("temporary_")) {
                        continue;
                    }
                    const char* methodID = bmhParser.fMaps[(int) token.fMarkType].fName;
                    if (!def) {
                        string paramName = className + "::";
                        paramName += string(token.fContentStart,
                                token.fContentEnd - token.fContentStart);
                        def = root->find(paramName);
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
                            def = root->find(operatorName);
                        }
                    }
                    if (!def) {
                        int skip = !strncmp(token.fContentStart, "explicit ", 9) ? 9 : 0;
                        skip = !strncmp(token.fContentStart, "virtual ", 8) ? 8 : skip;
                        string constructorName = className + "::";
                        constructorName += string(token.fContentStart + skip,
                                token.fContentEnd - token.fContentStart - skip);
                        def = root->find(constructorName);
                    }
                    if (!def && 0 == token.fName.find("SK_")) {
                        string incName = token.fName + "()";
                        string macroName = className + "::" + incName;
                        def = root->find(macroName);
                        if (def) {
                            if (def->fName == incName) {
                                def->fVisited = true;
                                if ("SK_TO_STRING_NONVIRT" == token.fName) {
                                    def = root->find(className + "::toString");
                                    if (def) {
                                        def->fVisited = true;
                                    } else {
                                        SkDebugf("missing toString bmh: %s\n", fullName.c_str());
                                    }
                                }
                                break;
                            } else {
                                SkDebugf("method macro differs from bmh: %s\n", fullName.c_str());
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
                            def = root->find(lowerName);
                        }
                    }
                    if (!def) {
                        SkDebugf("method missing from bmh: %s\n", fullName.c_str());
                        break;
                    }
                    if (def->crossCheck(methodID, token)) {
                        def->fVisited = true;
                    } else {
                       SkDebugf("method differs from bmh: %s\n", fullName.c_str());
                    }
                } break;
                case MarkType::kComment:
                    break;
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
                        firstMember.skipToNonAlphaNum();
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
                            def = root->find(anonName);
                        }
                        if (!def) {
                            SkDebugf("enum missing from bmh: %s\n", fullName.c_str());
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
                        SkDebugf("enum code missing from bmh: %s\n", fullName.c_str());
                        break;
                    }
                    if (def->crossCheck(token)) {
                        def->fVisited = true;
                    } else {
                       SkDebugf("enum differs from bmh: %s\n", def->fName.c_str());
                    }
                    for (auto& child : token.fChildren) {
                        string constName = className + "::" + child->fName;
                        def = root->find(constName);
                        if (!def) {
                            string innerName = classMapper.first + "::" + child->fName;
                            def = root->find(innerName);
                        }
                        if (!def) {
                            if (string::npos == child->fName.find("Legacy_")) {
                                SkDebugf("const missing from bmh: %s\n", constName.c_str());
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
                    }
                    break;
                default:
                    SkASSERT(0);  // unhandled 
                    break;
            }
        }
    }
    if (!root->dumpUnVisited()) {
        SkDebugf("some struct elements not found; struct finding in includeParser is missing\n");
    }
    return true;
}

IClassDefinition* InterfaceParser::defineClass(const Definition& includeDef,
        const string& name) {
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
    markupDef.fMarkType = MarkType::kClass;
    markupDef.fKeyWord = KeyWord::kClass;
    markupDef.fType = Definition::Type::kMark;
    fParent = &markupDef;
    return &markupDef;
}

void InterfaceParser::dumpClassTokens(IClassDefinition& classDef) {
    auto& tokens = classDef.fTokens;
    for (auto& token : tokens) {
        if (Definition::Type::kMark == token.fType && MarkType::kComment == token.fMarkType) {
            continue;
        }
        if (MarkType::kMember != token.fMarkType) {
            fprintf(fOut, "%s",
              "# ------------------------------------------------------------------------------\n");
            fprintf(fOut, ""                                                                  "\n");
        }
        switch (token.fMarkType) {
            case MarkType::kEnum:
                fprintf(fOut, "#Enum %s"                                                     "\n",
                        token.fName.c_str());
                fprintf(fOut, ""                                                             "\n");
                fprintf(fOut, "#Code"                                                        "\n");
                fprintf(fOut, "    enum %s {"                                                "\n",
                        token.fName.c_str());
                for (auto& child : token.fChildren) {
                    fprintf(fOut, "        %s %.*s"                                          "\n",
                            child->fName.c_str(), child->length(), child->fContentStart);
                }
                fprintf(fOut, "    };"                                                       "\n");
                fprintf(fOut, "##"                                                           "\n");
                fprintf(fOut, ""                                                             "\n");
                this->dumpComment(&token);
                for (auto& child : token.fChildren) {
                    fprintf(fOut, "#Const %s", child->fName.c_str());
                    TextParser val(child);
                    if (!val.eof()) {
                        if ('=' == val.fStart[0] || ',' == val.fStart[0]) {
                            val.next();
                            val.skipSpace();
                            const char* valEnd = val.anyOf(",\n");
                            if (!valEnd) {
                                valEnd = val.fEnd;
                            }
                            fprintf(fOut, " %.*s", (int) (valEnd - val.fStart), val.fStart);
                        } else {
                            fprintf(fOut, " %.*s", 
                                    (int) (child->fContentEnd - child->fContentStart),
                                    child->fContentStart);
                        }
                    }
                    fprintf(fOut, ""                                                         "\n");
                    for (auto& token : child->fTokens) {
                        if (MarkType::kComment == token.fMarkType) {
                            this->dumpComment(&token);
                        }
                    }
                    fprintf(fOut, "##"                                                       "\n");
                }
                fprintf(fOut, ""                                                             "\n");
            break;
            case MarkType::kMethod:
                fprintf(fOut, "#Method %.*s"                                                 "\n",
                        token.length(), token.fStart);
                lfAlways(1);
                this->dumpComment(&token);
            break;
            case MarkType::kMember:
                this->keywordStart("Member");
                fprintf(fOut, "%.*s  %s  ", (int) (token.fContentEnd - token.fContentStart),
                        token.fContentStart, token.fName.c_str());           
                lfAlways(1);
                for (auto child : token.fChildren) {
                    fprintf(fOut, "%.*s", (int) (child->fContentEnd - child->fContentStart),
                            child->fContentStart);
                    lfAlways(1);
                }
                this->keywordEnd();
                continue;
            break;
            default:
                SkASSERT(0);
        }
        this->lf(2);
        fprintf(fOut, "#Example"                                                             "\n");
        fprintf(fOut, "##"                                                                   "\n");
        fprintf(fOut, ""                                                                     "\n");
        fprintf(fOut, "#ToDo incomplete ##"                                                  "\n");
        fprintf(fOut, ""                                                                     "\n");
        fprintf(fOut, "##"                                                                   "\n");
        fprintf(fOut, ""                                                                     "\n");
    }
}
void InterfaceParser::dumpComment(Definition* token) {
    fLineCount = token->fLineCount;
    fChar = fLine = token->fContentStart;
    fEnd = token->fContentEnd;
    bool sawParam = false;
    bool multiline = false;
    bool sawReturn = false;
    bool sawComment = false;
    bool methodHasReturn = false;
    vector<string> methodParams;
    vector<string> foundParams;
    Definition methodName;
    TextParser methodParser(token->fFileName, token->fContentStart, token->fContentEnd,
            token->fLineCount);
    if (MarkType::kMethod == token->fMarkType) {
        methodName.fName = string(token->fContentStart,
                (int) (token->fContentEnd - token->fContentStart));
        methodHasReturn = !methodParser.startsWith("void ")
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
    for (const auto& child : token->fTokens) {
        if (Definition::Type::kMark == child.fType && MarkType::kComment == child.fMarkType) {
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
                                    this->lfAlways(1);
                                }
                                this->keywordEnd();
                            } else {
                                if (sawComment) {
                                    this->nl();
                                }
                                this->lf(2);
                            }
                            foundParams.emplace_back(piece);
                            this->keywordStart("Param");
                            fprintf(fOut, "%s  ", piece.c_str());
                            fprintf(fOut, "%.*s", (int) (parser.fEnd - parser.fChar), parser.fChar);
                            this->lfAlways(1);
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
                                this->lfAlways(1);
                            }
                            this->keywordEnd();
                        }
                        this->checkForMissingParams(methodParams, foundParams);
                        sawParam = false;
                        sawComment = false;
                        multiline = false;
                        this->lf(2);
                        this->keywordStart("Return");
                        fprintf(fOut, "%.*s ", (int) (parser.fEnd - parser.fChar),
                                parser.fChar);
                        this->lfAlways(1);
                        sawReturn = true;
                        parser.skipTo(parser.fEnd);
                    } else {
                        this->reportError("unexpected doxygen directive");
                    }
                } while (!parser.eof());
            } else {
                if (sawComment) {
                    this->nl();
                }
                this->lf(1);
                fprintf(fOut, "%.*s ", child.length(), child.fContentStart);
                sawComment = true;
                if (sawParam || sawReturn) {
                    multiline = true;
                }
            }
        }
    }
    if (sawParam || sawReturn) {
        if (multiline) {
            this->lfAlways(1);
        }
        this->keywordEnd();
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
            this->keywordStart("Return");
            this->keywordEnd();
        }
    }
}

    // dump equivalent markup 
void InterfaceParser::dumpTokens()  {
    string skClassName = this->className();
    string fileName = skClassName + ".bmh";
    fOut = fopen(fileName.c_str(), "w");
    string prefixName = skClassName.substr(0, 2);
    string topicName = skClassName.length() > 2 && isupper(skClassName[2]) &&
        ("Sk" == prefixName || "Gr" == prefixName) ? skClassName.substr(2) : skClassName;
    fprintf(fOut, "#Topic %s", topicName.c_str());
    this->lfAlways(2);
    fprintf(fOut, "#Class %s", skClassName.c_str());
    this->lfAlways(2);
    auto& classMap = fIClassMap[skClassName];
    auto& tokens = classMap.fTokens;
    for (auto& token : tokens) {
        if (Definition::Type::kMark != token.fType || MarkType::kComment != token.fMarkType) {
            continue;
        }
        fprintf(fOut, "%.*s", (int) (token.fContentEnd - token.fContentStart),
                token.fContentStart);
        this->lfAlways(1);
    }
    this->lf(2);
    string className(skClassName.substr(2));
    vector<string> sortedClasses;
    size_t maxLen = 0;
    for (const auto& oneClass : fIClassMap) {
        if (skClassName + "::" != oneClass.first.substr(0, skClassName.length() + 2)) {
            continue;
        }
        string structName = oneClass.first.substr(skClassName.length() + 2);
        maxLen = SkTMax(maxLen, structName.length());
        sortedClasses.emplace_back(structName);
    }
    fprintf(fOut, "#Topic Overview");
    this->lfAlways(2);
    fprintf(fOut, "#Subtopic %s_Structs", className.c_str());
    this->lfAlways(1);
    fprintf(fOut, "#Table");
    this->lfAlways(1);
    fprintf(fOut, "#Legend");
    this->lfAlways(1);
    fprintf(fOut, "# %-*s # description ##", (int) maxLen, "struct");
    this->lfAlways(1);
    fprintf(fOut, "#Legend ##");
    this->lfAlways(1);
    fprintf(fOut, "#Table ##");
    this->lfAlways(1);
    for (auto& name : sortedClasses) {
        fprintf(fOut, "# %-*s # ##", (int) maxLen, name.c_str());
        this->lfAlways(1);
    }
    fprintf(fOut, "#Subtopic ##");
    this->lfAlways(2);
    fprintf(fOut, "#Subtopic %s_Member_Functions", className.c_str());
    this->lfAlways(1);
    fprintf(fOut, "#Table");
    this->lfAlways(1);
    fprintf(fOut, "#Legend");
    this->lfAlways(1);
    maxLen = 0;
    vector<string> sortedNames;
    for (const auto& token : classMap.fTokens) {
        if (Definition::Type::kMark != token.fType || MarkType::kMethod != token.fMarkType) {
            continue;
        }
        const string& name = token.fName;
        if (name.substr(0, 7) == "android" || string::npos != name.find("nternal_")) {
            continue;
        }
        if (name[name.length() - 2] == '_' && isdigit(name[name.length() - 1])) {
            continue;
        }
        size_t paren = name.find('(');
        size_t funcLen = string::npos == paren ? name.length() : paren;
        maxLen = SkTMax(maxLen, funcLen);
        sortedNames.emplace_back(name);
    }
    std::sort(sortedNames.begin(), sortedNames.end());
    fprintf(fOut, "# %-*s # description   ##" "\n",
            (int) maxLen, "function");
    fprintf(fOut, "#Legend ##"                                                               "\n");
    for (auto& name : sortedNames) {
        size_t paren = name.find('(');
        size_t funcLen = string::npos == paren ? name.length() : paren;
        fprintf(fOut, "# %-*s # ##"                                                          "\n",
                (int) maxLen, name.substr(0, funcLen).c_str());
    }
    fprintf(fOut, "#Table ##"                                                                "\n");
    fprintf(fOut, "#Subtopic ##"                                                             "\n");
    fprintf(fOut, ""                                                                         "\n");
    fprintf(fOut, "#Topic ##"                                                                "\n");
    fprintf(fOut, ""                                                                         "\n");

    for (auto& oneClass : fIClassMap) {
        if (skClassName + "::" != oneClass.first.substr(0, skClassName.length() + 2)) {
            continue;
        }
        string innerName = oneClass.first.substr(skClassName.length() + 2);
        fprintf(fOut, "%s",
            "# ------------------------------------------------------------------------------");
        this->lfAlways(2);
        fprintf(fOut, "#Struct %s", innerName.c_str());
        this->lfAlways(2);
        for (auto& token : oneClass.second.fTokens) {
            if (Definition::Type::kMark != token.fType || MarkType::kComment != token.fMarkType) {
                continue;
            }
            fprintf(fOut, "%.*s", (int) (token.fContentEnd - token.fContentStart),
                    token.fContentStart);
            this->lfAlways(1);
        }
        this->lf(2);
        this->dumpClassTokens(oneClass.second);
        this->lf(2);
        fprintf(fOut, "#Struct %s ##", innerName.c_str());
        this->lfAlways(2);
    }
    this->dumpClassTokens(classMap);
    fprintf(fOut, "#Class %s ##"                                                             "\n",
            skClassName.c_str());
    fprintf(fOut, ""                                                                         "\n");
    fprintf(fOut, "#Topic %s ##"                                                             "\n",
            topicName.c_str());
    fclose(fOut);
}

bool InterfaceParser::findComments(const Definition& includeDef, Definition* markupDef) {
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

// caller calls reportError, so just return false here
bool InterfaceParser::parseClass(Definition* includeDef, IsStruct isStruct) {
    SkASSERT(includeDef->fTokens.size() > 0);
    if (includeDef->fTokens.size() == 1) {
        return true;  // forward declaration only
    }
    // parse class header
    auto iter = includeDef->fTokens.begin();
    if (!strncmp(iter->fStart, "SK_API", iter->fContentEnd - iter->fStart)) {
        // todo : documentation is ignoring this for now
        iter = std::next(iter);
    }
    string nameStr(iter->fStart, iter->fContentEnd - iter->fStart);
    includeDef->fName = nameStr;
    do {
        if (iter == includeDef->fTokens.end()) {
            return false;
        }
        if ('{' == iter->fStart[0] && Definition::Type::kPunctuation == iter->fType) {
            break;
        }   
    } while (static_cast<void>(iter = std::next(iter)), true);
    if (Punctuation::kLeftBrace != iter->fPunctuation) {
        return false;
    }
    IClassDefinition* markupDef = this->defineClass(*includeDef, nameStr);
    if (!markupDef) {
        return false;
    }
    markupDef->fStart = iter->fStart;
    if (!this->findComments(*includeDef, markupDef)) {
        return false;
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
            iter = std::next(iter);
            ++publicIndex;
        }
    }
    auto childIter = includeDef->fChildren.begin();
    while (childIter != includeDef->fChildren.end() && (*childIter)->fParentIndex < publicIndex) {
        (*childIter)->fPrivate = true;
        childIter = std::next(childIter);
    }
    int lastPublic = publicIndex;
    const char* protectedName = kKeyWords[(int) KeyWord::kProtected].fName;
    size_t protectedLen = strlen(protectedName);
    const char* privateName = kKeyWords[(int) KeyWord::kPrivate].fName;
    size_t privateLen = strlen(privateName);
    while (iter != includeDef->fTokens.end()
            && (protectedLen != (size_t) (iter->fContentEnd - iter->fStart)
            || strncmp(iter->fStart, protectedName, protectedLen))
            && (privateLen != (size_t) (iter->fContentEnd - iter->fStart)
            || strncmp(iter->fStart, privateName, privateLen))) {
        iter = std::next(iter);
        ++lastPublic;
    }
    while (childIter != includeDef->fChildren.end() && (*childIter)->fParentIndex < lastPublic) {
        Definition* child = *childIter;
        if (!this->parseObject(child, markupDef)) {
            return false;
        }
        childIter = std::next(childIter);
    }
    while (childIter != includeDef->fChildren.end()) {
        (*childIter)->fPrivate = true;
        childIter = std::next(childIter);
    }
    SkASSERT(fParent->fParent);
    fParent = fParent->fParent;
    return true;
}

bool InterfaceParser::parseComment(const string& filename, const char* start, const char* end,
        int lineCount, Definition* markupDef) {
    TextParser parser(filename, start, end, lineCount);
    // parse doxygen if present
    if (parser.startsWith("**")) {
        parser.next();
        parser.next();
        parser.skipWhiteSpace();
        if ('\\' == parser.peek()) {
            parser.next();
            if (!parser.skipWord(kKeyWords[(int) markupDef->fKeyWord].fName)) {
                return reportError<bool>("missing object type");
            }
            if (!parser.skipWord(markupDef->fName.c_str())) {
                return reportError<bool>("missing object name");
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
                parser.fLineCount, parent);
        parser.skipToEndBracket('\n');
    }
    return true;
}

bool InterfaceParser::parseDefine() {

    return true;
}

bool InterfaceParser::parseEnum(Definition* child, Definition* markupDef) {
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
    markupDef->fTokens.emplace_back(MarkType::kEnum, child->fContentStart, child->fContentEnd,
        child->fLineCount, markupDef);
    Definition* markupChild = &markupDef->fTokens.back();
    if (!this->findComments(*child, markupChild)) {
        return false;
    }
    TextParser parser(child);
    parser.skipToEndBracket('{');
    const char* dataEnd;
    do {
        parser.next();
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
                    markupChild);
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
        parser.skipToNonAlphaNum();
        string memberName(memberStart, parser.fChar);
        parser.skipWhiteSpace();
        const char* dataStart = parser.fChar;
        SkASSERT('=' == dataStart[0] || ',' == dataStart[0] || '}' == dataStart[0]
                 || '/' == dataStart[0]);
        dataEnd = parser.anyOf(",}");
        markupChild->fTokens.emplace_back(MarkType::kMember, dataStart, dataEnd, parser.fLineCount,
                markupChild);
        Definition* member = &markupChild->fTokens.back();
        member->fName = memberName;
        if (comment) {
            member->fChildren.push_back(comment);
        }
        markupChild->fChildren.push_back(member);
        parser.skipToEndBracket(dataEnd[0]);
    } while (',' == dataEnd[0]);
    for (size_t index = 1; index < child->fChildren.size(); ++index) {
        const Definition* follower = child->fChildren[index];
        if (Definition::Type::kKeyWord == follower->fType) {
            markupChild->fTokens.emplace_back(MarkType::kMember, follower->fContentStart, 
                    follower->fContentEnd, follower->fLineCount, markupChild);
            Definition* member = &markupChild->fTokens.back();
            member->fName = follower->fName;
            markupChild->fChildren.push_back(member);
        }
    }
    IClassDefinition& classDef = fIClassMap[markupDef->fName];
    SkASSERT(classDef.fStart);
    string uniqueName = this->uniqueName(classDef.fEnums, nameStr);
    markupChild->fName = uniqueName;
    classDef.fEnums[uniqueName] = markupChild;
    return true;
}

bool InterfaceParser::parseInclude(const string& name) {
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

bool InterfaceParser::parseMember(Definition* child, Definition* markupDef) {
    const char* typeStart = child->fChildren[0]->fContentStart;
    markupDef->fTokens.emplace_back(MarkType::kMember, typeStart, child->fContentStart,
        child->fLineCount, markupDef);
    Definition* markupChild = &markupDef->fTokens.back();
    TextParser nameParser(child);
    nameParser.skipToNonAlphaNum();
    string nameStr = string(child->fContentStart, nameParser.fChar - child->fContentStart);
    IClassDefinition& classDef = fIClassMap[markupDef->fName];
    string uniqueName = this->uniqueName(classDef.fMethods, nameStr);
    markupChild->fName = uniqueName;
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
                const char* end = parser.trimmedBracketEnd('\n', OneLine::kYes);
                if (Bracket::kSlashStar == comment->fBracket) {
                    const char* commentEnd = parser.strnstr("*/", end);
                    if (commentEnd) {
                        end = commentEnd;
                    }
                }
                markupDef->fTokens.emplace_back(MarkType::kComment, start, end, child->fLineCount,
                        markupDef);
                Definition* commentChild = &markupDef->fTokens.back();
                markupChild->fChildren.emplace_back(commentChild);
                parser.skipTo(end);
            } while (!parser.eof());
        }
    }
    return true;
}

bool InterfaceParser::parseMethod(Definition* child, Definition* markupDef) {
    auto tokenIter = child->fParent->fTokens.begin();
    std::advance(tokenIter, child->fParentIndex);
    tokenIter = std::prev(tokenIter);
    string nameStr(tokenIter->fStart, tokenIter->fContentEnd - tokenIter->fStart);
    while (tokenIter != child->fParent->fTokens.begin()) {
        auto testIter = std::prev(tokenIter);
        switch (testIter->fType) {
            case Definition::Type::kWord:
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
    tokenIter->fName = nameStr;
    tokenIter->fMarkType = MarkType::kMethod;
    auto testIter = child->fParent->fTokens.begin();
    SkASSERT(child->fParentIndex > 0);
    std::advance(testIter, child->fParentIndex - 1);
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
    markupDef->fTokens.emplace_back(MarkType::kMethod, start, end, tokenIter->fLineCount,
            markupDef);
    Definition* markupChild = &markupDef->fTokens.back();
    // do find instead -- I wonder if there is a way to prevent this in c++
    IClassDefinition& classDef = fIClassMap[markupDef->fName];
    SkASSERT(classDef.fStart);
    string uniqueName = this->uniqueName(classDef.fMethods, nameStr);
    markupChild->fName = uniqueName;
    if (!this->findComments(*child, markupChild)) {
        return false;
    }
    classDef.fMethods[uniqueName] = markupChild;
    return true;
}

void InterfaceParser::keywordEnd() {
    fprintf(fOut, "##");
    this->lfAlways(1);
}

void InterfaceParser::keywordStart(const char* keyword) {
    this->lf(1);
    fprintf(fOut, "#%s ", keyword);
}

bool InterfaceParser::parseObjects(Definition* parent, Definition* markupDef) {
    for (auto& child : parent->fChildren) {
        if (!this->parseObject(child, markupDef)) {
            return false;
        }
    }
    return true;
}

bool InterfaceParser::parseObject(Definition* child, Definition* markupDef) {
    // set up for error reporting
    fLine = fChar = child->fStart;
    fEnd = child->fContentEnd;
    // todo: put original line number in child as well
    switch (child->fType) {
        case Definition::Type::kKeyWord:
            switch (child->fKeyWord) {
                case KeyWord::kClass: 
                    if (!this->parseClass(child, IsStruct::kNo)) {
                        return this->reportError<bool>("failed to parse class");
                    }
                    break;
                case KeyWord::kEnum:
                    if (!this->parseEnum(child, markupDef)) {
                        return this->reportError<bool>("failed to parse enum");
                    }
                    break;
                case KeyWord::kStruct:
                    if (!this->parseClass(child, IsStruct::kYes)) {
                        return this->reportError<bool>("failed to parse struct");
                    }
                    break;
                case KeyWord::kTemplate:
                    if (!this->parseTemplate()) {
                        return this->reportError<bool>("failed to parse template");
                    }
                    break;
                case KeyWord::kTypedef:
                    if (!this->parseTypedef()) {
                        return this->reportError<bool>("failed to parse typedef");
                    }
                    break;
                case KeyWord::kUnion:
                    if (!this->parseUnion()) {
                        return this->reportError<bool>("failed to parse union");
                    }
                    break;
                default:
                    return this->reportError<bool>("unhandled keyword");
            }
            break;
        case Definition::Type::kBracket:
            switch (child->fBracket) {
                case Bracket::kParen:
                    if (!this->parseMethod(child, markupDef)) {
                        return this->reportError<bool>("failed to parse method");
                    }
                    break;
                case Bracket::kSlashSlash:
                case Bracket::kSlashStar:
                    // comments are picked up by parsing objects first
                    break;
                case Bracket::kPound:
                    // special-case the #xxx xxx_DEFINED entries
                    switch (child->fKeyWord) {
                        case KeyWord::kIfndef:
                        case KeyWord::kIfdef:
                            if (child->boilerplateIfDef(fParent)) {
                                if (!this->parseObjects(child, markupDef)) {
                                    return false;
                                }
                                break;
                            }
                            goto preproError;
                        case KeyWord::kDefine:
                            if (child->boilerplateDef(fParent)) {
                                break;
                            }
                            goto preproError;
                        case KeyWord::kEndif:
                            if (child->boilerplateEndIf()) {
                                break;
                            }
                        case KeyWord::kInclude:
                            // ignored for now
                            break;
                        case KeyWord::kElse:
                        case KeyWord::kElif:
                            // todo: handle these
                            break;
                        default:
                        preproError:
                            return this->reportError<bool>("unhandled preprocessor");
                    }
                    break;
                case Bracket::kAngle:
                    // pick up templated function pieces when method is found
                    break;
                default:
                    return this->reportError<bool>("unhandled bracket");
            }
            break;
        case Definition::Type::kWord:
            if (MarkType::kMember != child->fMarkType) {
                return this->reportError<bool>("unhandled word type");
            }
            if (!this->parseMember(child, markupDef)) {
                return this->reportError<bool>("unparsable member");
            }
            break;
        default:
            return this->reportError<bool>("unhandled type");
            break;
    }
    return true;
}

bool InterfaceParser::parseTemplate() {

    return true;
}

bool InterfaceParser::parseTypedef() {

    return true;
}

bool InterfaceParser::parseUnion() {

    return true;
}

bool InterfaceParser::parseChar() {
    char test = *fChar;
    if ('\\' == fPrev) {
        if ('\n' == test) {
            ++fLineCount;
            fLine = fChar + 1;
        }
        goto done;
    }
    switch (test) {
        case '\n':
            ++fLineCount;
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
                if (KeyWord::kInclude == keyWord || KeyWord::kDefine == keyWord) {
                    this->popBracket();
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
                    this->popBracket();
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
        case ':':
        case '(':
        case '[':
        case '{': {
            if (fInCharCommentString) {
                break;
            }
            if (':' == test && (fInBrace || ':' == fChar[-1] || ':' == fChar[1])) {
                break;
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
            break;
        case ')':
        case ']':
        case '}': {
            if (fInCharCommentString) {
                break;
            }
            if (!fInBrace) {
                if (!this->checkForWord()) {
                    return false;
                }
            }
            bool popBraceParent = fInBrace == fParent;
            if ((')' == test ? Bracket::kParen :
                    ']' == test ? Bracket::kSquare : Bracket::kBrace) == this->topBracket()) {
                this->popBracket();
                if (!fInFunction) {
                    bool deprecatedMacro = false;
                    if (')' == test) {
                        auto iter = fParent->fTokens.end();
                        bool lookForWord = false;
                        while (fParent->fTokens.begin() != iter) {
                            --iter;
                            if (lookForWord) {
                                if (Definition::Type::kWord != iter->fType) {
                                    break;
                                }
                                string word(iter->fContentStart, iter->length());
                                if ("SK_ATTR_EXTERNALLY_DEPRECATED" == word) {
                                    deprecatedMacro = true;
                                    // remove macro paren (would confuse method parsing later)
                                    fParent->fTokens.pop_back();  
                                    fParent->fChildren.pop_back();
                                }
                                break;
                            }
                            if (Definition::Type::kBracket != iter->fType) {
                                break;
                            }
                            if (Bracket::kParen != iter->fBracket) {
                                break;
                            }
                            lookForWord = true;
                        }
                    }
                    fInFunction = ')' == test && !deprecatedMacro;
                } else {
                    fInFunction = '}' != test;
                }
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
            if (Bracket::kAngle == this->topBracket()) {
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
        case '&':
        case ',':
        case ' ':
            if (fInCharCommentString || fInBrace) {
                break;
            }
            if (!this->checkForWord()) {
                return false;
            }
            break;
        case ';':
            if (fInCharCommentString || fInBrace) {
                break;
            }
            if (!this->checkForWord()) {
                return false;
            }
            if (Definition::Type::kKeyWord == fParent->fType
                    && KeyProperty::kObject == (kKeyWords[(int) fParent->fKeyWord].fProperty)) {
                if (KeyWord::kEnum == fParent->fKeyWord) {
                    fInEnum = false;
                }
                this->popObject();
            } else if (Definition::Type::kBracket == fParent->fType
                    && fParent->fParent && Definition::Type::kKeyWord == fParent->fParent->fType
                    && KeyWord::kStruct == fParent->fParent->fKeyWord) {
                list<Definition>::iterator baseIter = fParent->fTokens.end();
                list<Definition>::iterator namedIter  = fParent->fTokens.end();
                for (auto tokenIter = fParent->fTokens.end();
                        fParent->fTokens.begin() != tokenIter--; ) {
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
                    fParent->fChildren.push_back(member);
                    for (auto nameType = baseIter; nameType != namedIter; ++nameType) {
                        member->fChildren.push_back(&*nameType);
                    }

                }
            } else if (fParent->fChildren.size() > 0) {
                auto lastIter = fParent->fChildren.end();
                Definition* priorEnum;
                while (fParent->fChildren.begin() != lastIter) {
                    std::advance(lastIter, -1);
                    priorEnum = *lastIter;
                    if (Definition::Type::kBracket != priorEnum->fType ||
                            (Bracket::kSlashSlash != priorEnum->fBracket
                            && Bracket::kSlashStar != priorEnum->fBracket)) {
                        break;
                    }
                }
                if (Definition::Type::kKeyWord == priorEnum->fType
                        && KeyWord::kEnum == priorEnum->fKeyWord) {
                    auto tokenWalker = fParent->fTokens.begin();
                    std::advance(tokenWalker, priorEnum->fParentIndex);
                    SkASSERT(KeyWord::kEnum == tokenWalker->fKeyWord);
                    while (tokenWalker != fParent->fTokens.end()) {
                        std::advance(tokenWalker, 1);
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
                    if (foundExpected && tokenWalker != fParent->fTokens.end()) {
                        const char* nameStart = tokenWalker->fStart;
                        std::advance(tokenWalker, 1);
                        if (tokenWalker != fParent->fTokens.end()) {
                            TextParser tp(fFileName, nameStart, tokenWalker->fStart, fLineCount);
                            tp.skipToNonAlphaNum();
                            start->fName = string(nameStart, tp.fChar - nameStart);
                            start->fContentEnd = fChar;
                            priorEnum->fChildren.emplace_back(start);
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
    ++fChar;
    return true;
}

void InterfaceParser::enumHeaderOut(const RootDefinition* root,
        const Definition& child) {
    const Definition* enumDef = nullptr;
    const char* bodyEnd = fDeferComment ? fDeferComment->fContentStart - 1 :
            child.fContentStart;
    // FIXME: roll end-trimming into writeBlockTrim call
    while (fStart < bodyEnd && ' ' >= bodyEnd[-1]) {
        --bodyEnd;
    }
    int blockSize = (int) (bodyEnd - fStart);
    if (blockSize) {
        this->writeBlock((int) (bodyEnd - fStart), fStart);
    }
    this->lf(2);
    this->lfcrIndent();
    fDeferComment = nullptr;
    fStart = child.fContentStart;
    const auto& nameDef = child.fTokens.front();
    string fullName;
    if (nullptr != nameDef.fContentEnd) {
        string enumName(nameDef.fContentStart,
                (int) (nameDef.fContentEnd - nameDef.fContentStart));
        fullName = root->fName + "::" + enumName;
        enumDef = root->find(enumName);
        if (!enumDef) {
            enumDef = root->find(fullName);
        }
        SkASSERT(enumDef);
        // child[0] should be #Code comment starts at child[0].fTerminator
            // though skip until #Code is found (in case there's a #ToDo, etc)
        // child[1] should be #Const comment ends at child[1].fStart
        // comment becomes enum header (if any)
    } else {
        string enumName(root->fName);
        enumName += "::_anonymous";
        if (fAnonymousEnumCount > 1) {
            enumName += '_' + std::to_string(fAnonymousEnumCount);
        }
        enumDef = root->find(enumName);
        SkASSERT(enumDef);
        ++fAnonymousEnumCount;
    }
    Definition* codeBlock = nullptr;
    Definition* constBlock = nullptr;
    for (auto test : enumDef->fChildren) {
        if (MarkType::kCode == test->fMarkType) {
            SkASSERT(!codeBlock);  // FIXME: check enum for correct order earlier
            codeBlock = test;
        }
        if (MarkType::kConst == test->fMarkType) {
            SkASSERT(codeBlock && !constBlock);  // FIXME: check enum for correct order earlier
            constBlock = test;
            break;
        }
    }
    SkASSERT(constBlock);  // FIXME: check enum for correct order earlier
    const char* commentStart = codeBlock->fTerminator;
    const char* commentEnd = constBlock->fStart;
    if (!this->contentFree((int) (commentEnd - commentStart), commentStart)) {
        this->writeString("/** \\enum ");
        this->writeString(fullName.c_str());
        fIndent += 4;
        this->lfcrIndent();
        this->rewriteBlock((int) (commentEnd - commentStart), commentStart);
        fIndent -= 4;
        this->lfcrIndent();
        this->writeString("*/");
    }
    this->lfcrIndent();
    bodyEnd = child.fChildren[0]->fContentStart;
    SkASSERT('{' == bodyEnd[0]);
    ++bodyEnd;
    this->writeBlock((int) (bodyEnd - fStart), fStart); // write include "enum Name {"
    fIndent += 4;
    this->lfcrIndent();
    fStart = bodyEnd;
    fEnumDef = enumDef;
}

void InterfaceParser::enumSizeItems(const Definition& child) {
    enum class State {
        kNoItem,
        kItemName,
        kItemValue,
        kItemComment,
    };
    State state = State::kNoItem;
    int longestName = 0;
    int longestValue = 0;
    int valueLen = 0;
    const char* lastEnd = nullptr;
    SkASSERT(child.fChildren.size() == 1 || child.fChildren.size() == 2);
    auto brace = child.fChildren[0];
    SkASSERT(Bracket::kBrace == brace->fBracket);
    for (auto& token : brace->fTokens) {
        if (Definition::Type::kBracket == token.fType) {
            if (Bracket::kSlashSlash == token.fBracket) {
                continue;  // ignore old inline comments
            }
            SkASSERT(0); // incomplete
        }
        if (Definition::Type::kWord != token.fType) {
            SkASSERT(0); // incomplete
        }
        if (State::kItemName == state) {
            TextParser enumLine(token.fFileName, lastEnd,
                    token.fContentStart, token.fLineCount);
            const char* end = enumLine.anyOf(",}=");
            SkASSERT(end);
            state = '=' == *end ? State::kItemValue : State::kItemComment;
            if (State::kItemValue == state) {
                valueLen = (int) (token.fContentEnd - token.fContentStart);
                continue;
            }
        }
        if (State::kItemValue == state) {
            TextParser valueEnd(token.fFileName, lastEnd,
                    token.fContentStart, token.fLineCount);
            const char* end = valueEnd.anyOf(",}");
            if (!end) {  // write expression continuation
                valueLen += (int) (token.fContentStart - lastEnd); 
                continue;
            }
        }
        if (State::kNoItem != state) {
            longestValue = SkTMax(longestValue, valueLen);
            state = State::kNoItem;
        }
        SkASSERT(State::kNoItem == state);
        lastEnd = token.fContentEnd;
        longestName = SkTMax(longestName, (int) (lastEnd - token.fContentStart));
        state = State::kItemName;
    }
    fEnumItemValueTab = longestName + fIndent + 1 /* space before = */ ;
    if (longestValue) {
        longestValue += 3; /* = space , */
    }
    fEnumItemCommentTab = fEnumItemValueTab + longestValue + 6 /* space //!< space */ ;
    // iterate through bmh children and see which comments fit on include lines
    for (auto& enumItem : fEnumDef->fChildren) {
        if (MarkType::kConst != enumItem->fMarkType) {
            continue;
        }
        TextParser enumLine(enumItem);
        enumLine.skipToLineStart(); // skip const value
        const char* commentStart = enumLine.fChar;
        enumLine.skipLine();
        ptrdiff_t lineLen = enumLine.fChar - commentStart;
        if (!enumLine.eof()) {
            enumLine.skipWhiteSpace();
        }
        enumItem->fShort = enumLine.eof() && fEnumItemCommentTab + lineLen < 100;
    }
}

// walk children and output complete method doxygen description
void InterfaceParser::methodOut(const Definition* method) {
    fContinuation = nullptr;
    fDeferComment = nullptr;
    bool deprecated = false;
    for (auto methodProp : method->fChildren) {
        if (MarkType::kDeprecated == methodProp->fMarkType) {
            deprecated = true;
            break;
        }
    }
    if (deprecated) {
        return;
    }
    if (0 == fIndent) {
        fIndent = 4;
    }
    const char* commentStart = method->fContentStart;
    const char* commentEnd;
    if (method->fChildren.size() > 0) {
        commentEnd = method->fChildren[0]->fStart;
    } else {
        commentEnd = method->fContentEnd;
    }
    this->lf(2);
    this->lfcrIndent();
    this->writeString("/** ");
    fIndent += 4;
    int commentLen = (int) (commentEnd - commentStart);
    SkASSERT(commentLen > 0 && commentLen < 1000);
    this->rewriteBlock(commentLen, commentStart);
    // compute indention column
    size_t column = 0;
    bool hasParmReturn = false;
    for (auto methodPart : method->fChildren) {
        if (MarkType::kParam == methodPart->fMarkType) {
            column = SkTMax(column, methodPart->fName.length());
            hasParmReturn = true;
        } else if (MarkType::kReturn == methodPart->fMarkType) {
            hasParmReturn = true;
        }
    }
    if (hasParmReturn) {
        this->lf(2);
        this->lfcr();
        column += fIndent + sizeof("@return ");
        int saveIndent = fIndent;
        for (auto methodPart : method->fChildren) {
            const char* partStart = methodPart->fContentStart;
            const char* partEnd = methodPart->fContentEnd; 
            if (MarkType::kParam == methodPart->fMarkType) {
                this->indentToEdge();
                this->writeString("@param ");
                this->writeString(methodPart->fName.c_str());
            } else if (MarkType::kReturn == methodPart->fMarkType) {
                this->indentToEdge();
                this->writeString("@return");
            } else {
                continue;
            }
            while ('\n' == partEnd[-1]) {
                --partEnd;
            }
            this->indentToColumn(column);
            int partLen = (int) (partEnd - partStart);
            SkASSERT(partLen > 0 && partLen < 200);
            fIndent = column;
            this->rewriteBlock(partLen, partStart);
            fIndent = saveIndent;
            this->lfcr();
        }
    } else {
        this->lfcr();
    }
    fIndent -= 4;
    this->indentToEdge();
    this->writeString("*/");
    this->lfcrIndent();
}

void InterfaceParser::structOut(const Definition* root, const Definition& child,
        const char* commentStart, const char* commentEnd) {
    this->writeString("/** \\");
    SkASSERT(MarkType::kClass == child.fMarkType || MarkType::kStruct == child.fMarkType);
    this->writeString(MarkType::kClass == child.fMarkType ? "class " : "struct ");
    this->writeString(child.fName.c_str());
    fIndent += 4;
    this->lfcrIndent();
    this->rewriteBlock((int) (commentEnd - commentStart), commentStart);
    fIndent -= 4;
    this->lfcrIndent();
    this->writeString("*/");
    this->lfcrIndent();
}

bool InterfaceParser::populate(Definition* def, RootDefinition* root) {
    // write bulk of original include up to class, method, enum, etc., excepting preceding comment
    // find associated bmh object
    // write any associated comments in Doxygen form
    // skip include comment
    // if there is a series of same named methods, write one set of comments, then write all methods
    string methodName;
    const Definition* method;
    fContinuation = nullptr;
    for (auto& child : def->fTokens) {
        if (child.fPrivate) {
            continue;
        }
        if (fContinuation) {
            if (Definition::Type::kKeyWord == child.fType) {
                if (KeyWord::kFriend == child.fKeyWord || KeyWord::kBool == child.fKeyWord) {
                    continue;
                }
            }
            if (Definition::Type::kBracket == child.fType && Bracket::kParen == child.fBracket) {
                continue;
            }
            if (Definition::Type::kWord == child.fType) {
                size_t len = (size_t) (child.fContentEnd - child.fContentStart);
                const char operatorStr[] = "operator";
                size_t operatorLen = sizeof(operatorStr) - 1;
                if (len >= operatorLen && !strncmp(child.fContentStart, operatorStr, operatorLen)) {
                    fContinuation = child.fContentEnd;
                    continue;
                }
            }
            if (Definition::Type::kPunctuation == child.fType &&
                    (Punctuation::kSemicolon == child.fPunctuation ||
                    Punctuation::kLeftBrace == child.fPunctuation)) {
                SkASSERT(fContinuation[0] == '(');
                const char* continueEnd = child.fContentStart;
                while (continueEnd > fContinuation && isspace(continueEnd[-1])) {
                    --continueEnd;
                }
                methodName += string(fContinuation, continueEnd - fContinuation);
                method = root->find(methodName);
                if (!method) {
                    fLineCount = child.fLineCount;
                    fclose(fOut);  // so we can see what we've written so far
                    return this->reportError<bool>("method not found");
                }
                this->methodOut(method);
                continue;
            }
            methodName += "()";
            method = root->find(methodName);
            if (method) {
                this->methodOut(method);
                continue;
            }
            fLineCount = child.fLineCount;
            fclose(fOut);  // so we can see what we've written so far
            return this->reportError<bool>("method not found");
        }
        if (Bracket::kSlashSlash == child.fBracket || Bracket::kSlashStar == child.fBracket) {
            if (!fDeferComment) {
                fDeferComment = &child;
            }
            continue;
        } 
        if (MarkType::kMethod == child.fMarkType) {
            const char* bodyEnd = fDeferComment ? fDeferComment->fContentStart - 1 :
                    child.fContentStart;
            // FIXME: roll end-trimming into writeBlockTrim call
            while (fStart < bodyEnd && ' ' >= bodyEnd[-1]) {
                --bodyEnd;
            }
            int blockSize = (int) (bodyEnd - fStart);
            if (blockSize) {
                this->writeBlock(blockSize, fStart);
            }
            fStart = child.fContentStart;
            methodName = root->fName + "::" + child.fName;
            fContinuation = child.fContentEnd;
            method = root->find(methodName);
            if (!method) {
                continue;
            }
            this->methodOut(method);
            continue;
        } 
        if (Definition::Type::kKeyWord == child.fType) {
            const Definition* structDef = nullptr;
            switch (child.fKeyWord) {
                case KeyWord::kClass:
                case KeyWord::kStruct:
                    if (child.fChildren.size() > 0) {
                        const char* bodyEnd = fDeferComment ? fDeferComment->fContentStart - 1 :
                                child.fContentStart;
                        this->writeBlock((int) (bodyEnd - fStart), fStart);
                        fStart = child.fContentStart;
                        if (child.fName == root->fName) {
                            if (Definition* parent = root->fParent) {
                                if (MarkType::kTopic == parent->fMarkType ||
                                        MarkType::kSubtopic == parent->fMarkType) {
                                    const char* commentStart = parent->fContentStart;
                                    const char* commentEnd = root->fStart;
                                    this->structOut(root, *root, commentStart, commentEnd);
                                } else {
                                    SkASSERT(0); // incomplete
                                }
                            } else {
                                SkASSERT(0); // incomplete
                            }
                        } else {
                            structDef = root->find(child.fName);
                            if (nullptr == structDef) {
                                structDef = root->find(root->fName + "::" + child.fName);
                            }
                            Definition* codeBlock = nullptr;
                            Definition* nextBlock = nullptr;
                            for (auto test : structDef->fChildren) {
                                if (MarkType::kCode == test->fMarkType) {
                                    SkASSERT(!codeBlock);  // FIXME: check enum for correct order earlier
                                    codeBlock = test;
                                    continue;
                                }
                                if (codeBlock) {
                                    nextBlock = test;
                                    break;
                                }
                            }
                            SkASSERT(nextBlock);  // FIXME: check enum for correct order earlier
                            const char* commentStart = codeBlock->fTerminator;
                            const char* commentEnd = nextBlock->fStart;
                            this->structOut(root, *structDef, commentStart, commentEnd);
                        }
                        fDeferComment = nullptr;
                    } else {
                       ; // empty forward reference, nothing to do here
                    }
                    break;
                case KeyWord::kEnum: {
                    this->enumHeaderOut(root, child);
                    this->enumSizeItems(child);
                } break;
            }
            if (structDef) {
                fIndent += 4;
                if (!this->populate(&child, const_cast<Definition*>(structDef)->asRoot())) {
                    return false;
                }
                fIndent -= 4;
                fContinuation = nullptr;
                fDeferComment = nullptr;
            } else {
                if (!this->populate(&child, root)) {
                    return false;
                }
            }
            continue;
        } 
        if (Definition::Type::kBracket == child.fType) {
            if (KeyWord::kEnum == child.fParent->fKeyWord) {
                // iterate through include tokens and find how much remains for 1 line comments
                // put ones that fit on same line, ones that are too big on preceding line?
                const Definition* currentEnumItem = nullptr;
                const char* commentStart = nullptr;
                const char* lastEnd = nullptr;
                int commentLen = 0;
                enum class State {
                    kNoItem,
                    kItemName,
                    kItemValue,
                    kItemComment,
                };
                State state = State::kNoItem;
                for (auto& token : child.fTokens) {
                    if (Definition::Type::kBracket == token.fType) {
                        if (Bracket::kSlashSlash == token.fBracket) {
                            fStart = token.fContentEnd;
                            continue;  // ignore old inline comments
                        }
                        SkASSERT(0); // incomplete
                    }
                    if (Definition::Type::kWord != token.fType) {
                        SkASSERT(0); // incomplete
                    }
                    if (State::kItemName == state) {
                        TextParser enumLine(token.fFileName, lastEnd,
                                token.fContentStart, token.fLineCount);
                        const char* end = enumLine.anyOf(",}=");
                        SkASSERT(end);
                        state = '=' == *end ? State::kItemValue : State::kItemComment;
                        if (State::kItemValue == state) {  // write enum value
                            this->indentToColumn(fEnumItemValueTab);
                            this->writeString("= ");
                            lastEnd = token.fContentEnd;
                            this->writeBlock((int) (lastEnd - token.fContentStart),
                                    token.fContentStart); // write const value if any
                            continue;
                        }
                    }
                    if (State::kItemValue == state) {
                        TextParser valueEnd(token.fFileName, lastEnd,
                                token.fContentStart, token.fLineCount);
                        const char* end = valueEnd.anyOf(",}");
                        if (!end) {  // write expression continuation
                            this->writeBlock((int) (token.fContentEnd - lastEnd), lastEnd); 
                            continue;
                        }
                    }
                    if (State::kNoItem != state) {
                        this->writeString(",");
                        SkASSERT(currentEnumItem);
                        if (currentEnumItem->fShort) {
                            this->indentToColumn(fEnumItemCommentTab);
                            this->writeString("//!< ");
                            this->rewriteBlock(commentLen, commentStart);
                        }
                        this->lfcrIndent();
                        if (State::kItemValue == state) {
                            fStart = token.fContentStart;
                        }
                        state = State::kNoItem;
                    }
                    SkASSERT(State::kNoItem == state);
                    string itemName = root->fName + "::" + string(token.fContentStart,
                        (int) (token.fContentEnd - token.fContentStart));
                    for (auto& enumItem : fEnumDef->fChildren) {
                        if (MarkType::kConst != enumItem->fMarkType) {
                            continue;
                        }
                        if (itemName != enumItem->fName) {
                            continue;
                        }
                        currentEnumItem = enumItem;
                        break;
                    }
                    SkASSERT(currentEnumItem);
                    // if description fits, it goes after item
                    commentStart = currentEnumItem->fContentStart;
                    const char* commentEnd;
                    if (currentEnumItem->fChildren.size() > 0) {
                        commentEnd = currentEnumItem->fChildren[0]->fStart;
                    } else {
                        commentEnd = currentEnumItem->fContentEnd;
                    }
                    TextParser enumComment(fFileName, commentStart, commentEnd,
                            currentEnumItem->fLineCount);
                    SkAssertResult(enumComment.skipToLineStart()); // skip const value
                    commentStart = enumComment.fChar;
                    commentLen = (int) (commentEnd - commentStart);
                    SkASSERT(commentLen > 0 && commentLen < 1000);
                    if (!currentEnumItem->fShort) {
                        this->writeString("/** ");
                        fIndent += 4;
                        this->rewriteBlock(commentLen, commentStart);
                        this->lfcr();
                        fIndent -= 4;
                        this->indentToEdge();
                        this->writeString("*/");
                        this->lfcrIndent();
                    }
                    lastEnd = token.fContentEnd;
                    while (fStart < lastEnd && ' ' >= fStart[0]) {
                        ++fStart;
                    }
                    this->writeBlock((int) (lastEnd - fStart), fStart);  // enum item name
                    fStart = token.fContentEnd;
                    state = State::kItemName;
                }
                fIndent -= 4;
                this->lfcrIndent();
                this->writeString("};");
                this->lfcrIndent();
                fStart = child.fParent->fContentEnd;
                SkASSERT(';' == fStart[0]);
                ++fStart;
                fDeferComment = nullptr;
                continue;
            } 
            fDeferComment = nullptr;
            if (!this->populate(&child, root)) {
                return false;
            }
            continue;
        }
        if (Definition::Type::kWord == child.fType) {
            continue;
        }
        if (Definition::Type::kPunctuation == child.fType) {
            if (Punctuation::kSemicolon == child.fPunctuation ||
                    Punctuation::kLeftBrace == child.fPunctuation ||
                    Punctuation::kColon == child.fPunctuation ||
                    Punctuation::kAsterisk == child.fPunctuation
                ) {
                continue;
            }
        }
        // incomplete
        SkDebugf("");
    }
    return true;
}

bool InterfaceParser::populate(BmhParser& bmhParser) {
    bool allPassed = true;
    for (auto& includeMapper : fIncludeMap) {
        size_t lastSlash = includeMapper.first.rfind('/');
        if (string::npos == lastSlash || lastSlash >= includeMapper.first.length() - 1) {
            return this->reportError<bool>("malformed include name");
        }
        string fileName = includeMapper.first.substr(lastSlash + 1);
        if (".h" != fileName.substr(fileName.length() - 2)) {
            return this->reportError<bool>("expected fileName.h");
        }
        string skClassName = fileName.substr(0, fileName.length() - 2);
        fOut = fopen(fileName.c_str(), "w");
        if (!fOut) {
            return this->reportError<bool>("could not open output file");
        }
        if (bmhParser.fClassMap.end() == bmhParser.fClassMap.find(skClassName)) {
            return this->reportError<bool>("could not find bmh class");
        }
        fBmhParser = &bmhParser;
        RootDefinition* root = &bmhParser.fClassMap[skClassName];
        fRootTopic = root->fParent;
        root->clearVisited();
        fStart = includeMapper.second.fContentStart;
        fEnd = includeMapper.second.fContentEnd;
        allPassed &= this->populate(&includeMapper.second, root);
        this->writeBlock((int) (fEnd - fStart), fStart);
        fclose(fOut);
    }
    return allPassed;
}

void InterfaceParser::rewriteBlock(int size, const char* data) {
    while (size > 0 && data[0] <= ' ') {
        --size;
        ++data;
    }
    while (size > 0 && data[size - 1] <= ' ') {
        --size;
    }
    if (0 == size) {
        return;
    }
    int run = 0;
    while (run < size) {
        // start here: check for isUpper instead to catch single cap letter words
        if (data[run] == '_') {
            // omit underscores between capitalized words
            if (run + 1 < size && isupper(data[run + 1])) {
                int start = run;
                while (start > 0 && islower(data[--start]))
                    ;
                if (start > 0 && isupper(data[start]) && !isalnum(data[start - 1])) {
                    int end = run + 1;
                    while (++end < size &&
                            (isalpha(data[end]) || '_' == data[end] || '-' == data[end])) {
                        ;
                    }
                    char c = data[end];
                    if (c <= ' ' || '.' == c || ',' == c || ';' == c || ':' == c) {
                        // look up Xxx_Xxx 
                        string undername(&data[start], end - start);
                        string remainder;
                        auto rootDefIter = fBmhParser->fTopicMap.find(undername);
                        string substitute;
                        if (fBmhParser->fTopicMap.end() == rootDefIter) {
                            undername = fRootTopic->fName + '_' + undername;
                            rootDefIter = fBmhParser->fTopicMap.find(undername);
                            if (fBmhParser->fTopicMap.end() == rootDefIter) {
                                SkDebugf("unfound: %s\n", undername.c_str());
                            }
                        }
                        if (fBmhParser->fTopicMap.end() != rootDefIter) {
                            for (auto child : rootDefIter->second->fChildren) {
                                if (MarkType::kSubstitute == child->fMarkType) {
                                    substitute = string(child->fContentStart,
                                            (int) (child->fContentEnd - child->fContentStart));
                                    break;
                                }
                                if (MarkType::kClass == child->fMarkType ||
                                        MarkType::kStruct == child->fMarkType ||
                                        MarkType::kEnum == child->fMarkType ||
                                        MarkType::kEnumClass == child->fMarkType) {
                                    substitute = child->fName;
                                    break;
                                }
                            }
                            if (!substitute.length()) {
                                auto parent = rootDefIter->second->fParent;
                                if (parent) {
                                    if (MarkType::kClass == parent->fMarkType ||
                                            MarkType::kStruct == parent->fMarkType ||
                                            MarkType::kEnum == parent->fMarkType ||
                                            MarkType::kEnumClass == parent->fMarkType) {
                                        substitute = parent->fName;
                                        remainder = string(&data[run], end - run);
                                    }
                                }
                            }
                        }
                        if (0 == substitute.length()) {
                            remainder = undername;
                        }
                        for (char c : remainder) {
                            if ('_' == c) {
                                c = ' ';  // change Xxx_Xxx to xxx xxx
                            } else if (isupper(c)) {
                                c = tolower(c);
                            }
                            substitute += c;
                        }
                        this->writeBlock(start, data);
                        this->writeString(substitute.c_str());
                        data += end;
                        size -= end;
                        run = -1;
                    } else {
                        SkDebugf("");
                    }
                }
            }
        } else if (data[run] == '\n') {
            if (run > 0) {
                this->writeBlock(run, data);
            }
            if (++run < size && data[run] == '\n') {
                this->lf(2);
            }
            this->lfcrIndent();
            while (run < size && data[run] <= ' ') {
                ++run;
            }
            data += run;
            size -= run;
            run = -1;
        }
        ++run;
    }
    if (size) {
        this->writeBlock(size, data);
    }
}

void InterfaceParser::validate() const {
    for (int index = 0; index <= (int) Last_MarkType; ++index) {
        SkASSERT(fMaps[index].fMarkType == (MarkType) index);
    }
    InterfaceParser::ValidateKeyWords();
}
