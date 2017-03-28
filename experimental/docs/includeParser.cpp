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
    fParent->fTokens.emplace_back(keyWord, fIncludeWord, fChar, fParent);
    fIncludeWord = nullptr;
    if (KeyProperty::kObject == kKeyWords[(int) keyWord].fProperty) {
        Definition* def = &fParent->fTokens.back();
        this->addDefinition(def);
        if (KeyWord::kEnum == fParent->fKeyWord) {
            fInEnum = true;
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
    auto classMap = fIClassMap[className];
    auto tokens = classMap.fTokens;
    RootDefinition* root = &bmhParser.fClassMap[className];
    root->clearVisited();
    for (const auto& token : tokens) {
        const char* paren = strchr(token.fStart, '(');
        const char* nameStart = paren;
        while (nameStart > token.fStart && ' ' != nameStart[-1]) {
            --nameStart;
        }
        const char* contentEnd =
                token.fContentEnd != paren ? token.fContentEnd : strchr(token.fStart, ')') + 1;
        string method(nameStart, contentEnd - nameStart);
        trim_end(method);
        if (';' == method.back()) {
            method = method.substr(0, method.length() - 1);
        }
        string prefixed = className + '_' + method;
        Definition* def = root->find(prefixed);
        if (def) {
            def->fVisited = true;
            continue;
        }
        prefixed = className + '_' + string(nameStart, paren - nameStart);
        def = root->find(prefixed);
        if (def) {
            def->fVisited = true;
            continue;
        }
        printf("%s\n", method.c_str());
    }
    root->dumpUnVisited();
    return true;
}

    // dump equivalent markup 
void InterfaceParser::dumpTokens()  {
    string className = this->className();
    auto classMap = fIClassMap[className];
    auto tokens = classMap.fTokens;
    for (const auto& token : tokens) {

printf("%s",
       "# ------------------------------------------------------------------------------" "\n");
printf(""                                                                                 "\n");
printf("#Method %.*s"                                                                     "\n",
                      (int) (token.fContentEnd - token.fStart), token.fStart);
printf(""                                                                                 "\n");
        for (const auto& child : token.fTokens) {
            printf("%s", child.fStart);                                                                              
        }
        if (token.fChildren.size() > 0) {
printf(""                                                                                 "\n");
        }
printf("#Example"                                                                         "\n");
printf("##"                                                                               "\n");
printf(""                                                                                 "\n");
printf("#ToDo incomplete ##"                                                              "\n");
printf(""                                                                                 "\n");
printf("##"                                                                               "\n");
printf(""                                                                                 "\n");
    }
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
            && Definition::Type::kKeyWord != testIter->fType) {
            break;
        }
        wordIter = testIter;
    }
    auto commentIter = wordIter;
    while (parent->fTokens.begin() != commentIter) {
        auto testIter = std::prev(commentIter);
        if (Definition::Type::kBracket != testIter->fType
                || (Bracket::kSlashSlash != testIter->fBracket
                && Bracket::kSlashStar != testIter->fBracket)) {
            break;
        }
        commentIter = testIter;
    }
    while (commentIter != wordIter) {
        if (!this->parseComment(*commentIter, markupDef)) {
            return false;
        }
        commentIter = std::next(commentIter);
    }
    return true;
}

bool InterfaceParser::parseClass(Definition* includeDef) {
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
    do {
        if (iter == includeDef->fTokens.end()) {
            return this->reportError<bool>("malformed class definition, no open brace");
        }
        if ('{' == iter->fStart[0] && Definition::Type::kBracket == iter->fType) {
            break;
        }   
    } while ((iter = std::next(iter)), true);
    if (Bracket::kBrace != iter->fBracket) {
        return this->reportError<bool>("malformed class definition, missing open brace");
    }
    IClassDefinition* markupDef = this->findClass(*includeDef, nameStr);
    if (!markupDef) {
        return false;
    }
    markupDef->fStart = iter->fStart;
    markupDef->fName = nameStr;
    if (!this->findComments(*includeDef, markupDef)) {
        return false;
    }
    SkASSERT(1 == includeDef->fChildren.size());
    includeDef = includeDef->fChildren.front();
    iter = includeDef->fTokens.begin();
    // skip until public
    int publicIndex = 0;
    const char* publicName = kKeyWords[(int) KeyWord::kPublic].fName;
    size_t publicLen = strlen(publicName);
    while (iter != includeDef->fTokens.end()
            && publicLen == (size_t) (iter->fContentEnd - iter->fStart)
            && strncmp(iter->fStart, publicName, publicLen)) {
        iter = std::next(iter);
        ++publicIndex;
    }
    auto childIter = includeDef->fChildren.begin();
    while (childIter != includeDef->fChildren.end() && (*childIter)->fParentIndex < publicIndex) {
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
        // start here:
        // todo: add the inner object to the class
        childIter = std::next(childIter);
    }
    return true;
}

bool InterfaceParser::parseComment(const Definition& includeDef, Definition* markupDef) {
    fChar = fLine = includeDef.fStart;
    fEnd = includeDef.fContentEnd;
    // parse doxygen if present
    if (!strncmp(fChar, "**", 2)) {
        fChar += 2;
        this->skipWhiteSpace();
        if ('\\' == fChar[0]) {
            ++fChar;
            if (!this->skipWord(kKeyWords[(int) markupDef->fKeyWord].fName)) {
                return reportError<bool>("missing object type");
            }
            if (!this->skipWord(markupDef->fName.c_str())) {
                return reportError<bool>("missing object name");
            }

        }
    }
    // remove leading '*' if present
#if 01
// rewrite following -- doesn't work to put it in string
// instead put series of children, one per line of comment
    markupDef->fTokens.emplace_back(MarkType::kComment, fChar, fEnd, 0, markupDef);
    Definition* comment = &markupDef->fTokens.back();
    while (fChar < fEnd) {
        this->skipWhiteSpace();
        if ('*' == *fChar) {
            ++fChar;
            if (this->eof()) {
                break;
            }
            this->skipWhiteSpace();
        }
        const char* next = this->lineEnd();
        comment->fTokens.emplace_back(MarkType::kComment, fChar, next, 0, comment);
        fChar = next;
    }
#endif
    return true;
}

bool InterfaceParser::parseDefine() {

    return true;
}

bool InterfaceParser::parseEnum(Definition* child, Definition* markupDef) {
    // todo: set up name to be unique
    // if enum is unnamed, make it anon_1 etc
    string nameStr;
    Definition* parent = child->iRootParent();
    if (parent->fName.size() > 0) {
        nameStr += parent->fName + "::";
    }
    if (child->fTokens.size() > 0) {
        const Definition& firstToken = child->fTokens.front();
        if (Definition::Type::kWord == firstToken.fType) {
            nameStr += string(firstToken.fStart, firstToken.fContentEnd - firstToken.fStart);
        } else {
            nameStr += "anon";
        }
    }
    markupDef = this->findIncludeObject(*child, MarkType::kEnum, nameStr);
    if (!markupDef) {
        return false;
    }
    if (!this->findComments(*child, markupDef)) {
        return false;
    }
    return true;
}

bool InterfaceParser::parseInclude(const string& name) {
    fParent = &fIncludeMap[name];
    fParent->fName = name;
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
                        || Punctuation::kLeftBrace == testIter->fPunctuation) {
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
    auto testIter = child->fParent->fTokens.begin();
    SkASSERT(child->fParentIndex > 0);
    std::advance(testIter, child->fParentIndex - 1);
    const char* end = testIter->fContentEnd;
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
    markupDef->fTokens.emplace_back(MarkType::kMethod, tokenIter->fStart, end, 0, markupDef);
    Definition* markupChild = &markupDef->fTokens.back();
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

bool InterfaceParser::parseObjects(Definition* parent, Definition* markupDef) {
    for (auto& child : parent->fChildren) {
        if (!this->parseObject(child, markupDef)) {
            return false;
        }
    }
    return true;
}

bool InterfaceParser::parseObject(Definition* child, Definition* markupDef) {
    bool success = true;
    // set up for error reporting
    fLine = fChar = child->fStart;
    fEnd = child->fContentEnd;
    // todo: put original line number in child as well
    switch (child->fType) {
        case Definition::Type::kKeyWord:
            switch (child->fKeyWord) {
                case KeyWord::kClass: 
                    success = this->parseClass(child);
                    break;
                case KeyWord::kEnum:
                    success = this->parseEnum(child, markupDef);
                    break;
                case KeyWord::kStruct:
                    success = this->parseClass(child);
                    break;
                case KeyWord::kTemplate:
                    success = this->parseTemplate();
                    break;
                case KeyWord::kTypedef:
                    success = this->parseTypedef();
                    break;
                case KeyWord::kUnion:
                    success = this->parseUnion();
                    break;
                default:
                    return this->reportError<bool>("unhandled keyword");
            }
            break;
        case Definition::Type::kBracket:
            switch (child->fBracket) {
                case Bracket::kParen:
                    if (!this->parseMethod(child, markupDef)) {
                        return false;
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
                                this->parseObjects(child, markupDef);
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
        default:
            return this->reportError<bool>("unhandled type");
            break;
    }
    return success;
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
        case '}':
        if (1292 <= fLineCount) {
            SkDebugf("");
        }
            if (fInCharCommentString) {
                break;
            }
            if (!fInBrace) {
                if (!this->checkForWord()) {
                    return false;
                }
            }
            if (fInBrace == fParent) {
                Definition* braceParent = fParent->fParent;
                braceParent->fChildren.pop_back();
                braceParent->fTokens.pop_back();
                fInBrace = nullptr;
            }
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
                                string word(iter->fContentStart, iter->fContentEnd - iter->fContentStart);
                                if ("SK_ATTR_EXTERNALLY_DEPRECATED" == word) {
                                    deprecatedMacro = true;
                                    fParent->fTokens.pop_back();  // remove macro paren (which will confuse method parsing later)
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
            break;
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

void InterfaceParser::validate() const {
    for (int index = 0; index <= (int) Last_MarkType; ++index) {
        SkASSERT(fMaps[index].fMarkType == (MarkType) index);
    }
    InterfaceParser::ValidateKeyWords();
}
