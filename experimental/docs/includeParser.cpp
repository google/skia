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
    auto& classMap = fIClassMap[className];
    auto& tokens = classMap.fTokens;
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
        const Definition* def = root->find(prefixed);
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
        fprintf(fIOut, "%s\n", method.c_str());
    }
    root->dumpUnVisited();
    return true;
}

void InterfaceParser::dumpComment(const Definition& token) {
    bool foundComment = false;
    for (const auto& child : token.fTokens) {
        if (Definition::Type::kMark == child.fType && MarkType::kComment == child.fMarkType) {
            fprintf(fIOut, "%.*s\n", child.length(), child.fContentStart);
            foundComment = true;
        }
    }
    if (foundComment) {
        fprintf(fIOut, ""                                                                     "\n");
        fprintf(fIOut, ""                                                                     "\n");
    }
}

    // dump equivalent markup 
void InterfaceParser::dumpTokens()  {
    string skClassName = this->className();
    string fileName = skClassName + ".bmh";
    fIOut = fopen(fileName.c_str(), "w");
    auto& classMap = fIClassMap[skClassName];
    auto& tokens = classMap.fTokens;
    string className(skClassName.substr(2));
    fprintf(fIOut, "#SubTopic %s_Member_Functions"                                            "\n",
            className.c_str());
    fprintf(fIOut, "#Table"                                                                   "\n");
    fprintf(fIOut, "#Legend"                                                                  "\n");
    size_t maxLen = 0;
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
    fprintf(fIOut, "# %-*s # description                                                   ##" "\n",
            (int) maxLen, "function");
    fprintf(fIOut, "#Legend ##"                                                               "\n");
    for (auto& name : sortedNames) {
        size_t paren = name.find('(');
        size_t funcLen = string::npos == paren ? name.length() : paren;
        fprintf(fIOut, "# %-*s # ##"                                                          "\n",
                (int) maxLen, name.substr(0, funcLen).c_str());
    }
    fprintf(fIOut, "#Table ##"                                                                "\n");
    fprintf(fIOut, "#SubTopic ##"                                                             "\n");
    fprintf(fIOut, ""                                                                         "\n");
    for (const auto& token : tokens) {
        if (Definition::Type::kMark == token.fType && MarkType::kComment == token.fMarkType) {
            continue;
        }
        fprintf(fIOut, "%s",
              "# ------------------------------------------------------------------------------\n");
        fprintf(fIOut, ""                                                                     "\n");
        switch (token.fMarkType) {
            case MarkType::kEnum:
                fprintf(fIOut, "#Enum %s"                                                     "\n",
                        token.fName.c_str());
                fprintf(fIOut, ""                                                             "\n");
                fprintf(fIOut, "#Code"                                                        "\n");
                fprintf(fIOut, "    enum %s {"                                                "\n",
                        token.fName.c_str());
                for (auto& child : token.fChildren) {
                    fprintf(fIOut, "        %s %.*s"                                          "\n",
                            child->fName.c_str(), child->length(), child->fContentStart);
                }
                fprintf(fIOut, "    };"                                                       "\n");
                fprintf(fIOut, "##"                                                           "\n");
                fprintf(fIOut, ""                                                             "\n");
                this->dumpComment(token);
                for (auto& child : token.fChildren) {
                    fprintf(fIOut, "#Const %s", child->fName.c_str());
                    TextParser val(child->fContentStart, child->fContentEnd);
                    if (!val.eof()) {
                        SkASSERT('=' == val.fStart[0] || ',' == val.fStart[0]);
                        val.next();
                        val.skipSpace();
                        const char* valEnd = val.anyOf(",\n");
                        if (!valEnd) {
                            valEnd = val.fEnd;
                        }
                        fprintf(fIOut, " %.*s", (int) (valEnd - val.fStart), val.fStart);
                    }
                    fprintf(fIOut, ""                                                         "\n");
                    for (auto& token : child->fTokens) {
                        if (MarkType::kComment == token.fMarkType) {
                            this->dumpComment(token);
                        }
                    }
                    fprintf(fIOut, "##"                                                       "\n");
                }
                fprintf(fIOut, ""                                                             "\n");
            break;
            case MarkType::kMethod:
                fprintf(fIOut, "#Method %.*s"                                                 "\n",
                        token.length(), token.fStart);
                fprintf(fIOut, ""                                                             "\n");
                this->dumpComment(token);
            break;
            default:
                SkASSERT(0);
        }
        fprintf(fIOut, ""                                                                     "\n");
        fprintf(fIOut, "#Example"                                                             "\n");
        fprintf(fIOut, "##"                                                                   "\n");
        fprintf(fIOut, ""                                                                     "\n");
        fprintf(fIOut, "#ToDo incomplete ##"                                                  "\n");
        fprintf(fIOut, ""                                                                     "\n");
        fprintf(fIOut, "##"                                                                   "\n");
        fprintf(fIOut, ""                                                                     "\n");
    }
    fclose(fIOut);
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
        if (!this->parseComment(commentIter->fContentStart, commentIter->fContentEnd, 
                commentIter->fLineCount, markupDef)) {
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
        if ('{' == iter->fStart[0] && Definition::Type::kPunctuation == iter->fType) {
            break;
        }   
    } while ((iter = std::next(iter)), true);
    if (Punctuation::kLeftBrace != iter->fPunctuation) {
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
    if (1 != includeDef->fChildren.size()) {
        return false;  // fix me: SkCanvasClipVisitor isn't correctly parsed
    }
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

bool InterfaceParser::parseComment(const char* start, const char* end, int lineCount, 
        Definition* markupDef) {
    TextParser parser(start, end);
    parser.fLineCount = lineCount;
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
    Definition* comment = &markupDef->fTokens.back();
    while (!parser.eof() && parser.skipWhiteSpace()) {
        if ('*' == parser.peek()) {
            parser.next();
            if (parser.eof()) {
                break;
            }
            parser.skipWhiteSpace();
        }
        const char* lineEnd = parser.trimmedLineEnd();
        markupDef->fTokens.emplace_back(MarkType::kComment, parser.fChar, lineEnd, 
                parser.fLineCount, comment);
        parser.skipToEndBracket('\n');
    }
    return true;
}

bool InterfaceParser::parseDefine() {

    return true;
}

bool InterfaceParser::parseEnum(Definition* child, Definition* markupDef) {
    // todo: set up name to be unique
    // if enum is unnamed, make it anon_1 etc
    string nameStr;
    if (child->fTokens.size() > 0) {
        const Definition& firstToken = child->fTokens.front();
        if (Definition::Type::kWord == firstToken.fType) {
            nameStr += string(firstToken.fStart, firstToken.fContentEnd - firstToken.fStart);
        }
    }
    markupDef->fTokens.emplace_back(MarkType::kEnum, child->fContentStart, child->fContentEnd,
        child->fLineCount, markupDef);
    Definition* markupChild = &markupDef->fTokens.back();
    if (!this->findComments(*child, markupChild)) {
        return false;
    }
    TextParser parser(child->fContentStart, child->fContentEnd);
    parser.fLineCount = child->fLineCount;
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
        if (parser.startsWith("/*") || parser.startsWith("//")) {
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
            if (!this->parseComment(start, end, parser.fLineCount, comment)) {
                return false;
            }
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
        SkASSERT('=' == dataStart[0] || ',' == dataStart[0] || '}' == dataStart[0]);
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
    IClassDefinition& classDef = fIClassMap[markupDef->fName];
    SkASSERT(classDef.fStart);
    string uniqueName = this->uniqueName(classDef.fEnums, nameStr);
    markupChild->fName = uniqueName;
    classDef.fMethods[uniqueName] = markupChild;
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
    markupDef->fTokens.emplace_back(MarkType::kMethod, tokenIter->fStart, end, fLineCount,
            markupDef);
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
