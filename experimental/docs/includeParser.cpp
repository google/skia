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

KeyWord Parser::FindKey(const char* start, const char* end) {
    int ch = 0;
    for (size_t index = 0; index < kKeyWordCount; ) {
        if (start[ch] > kKeyWords[index].fName[ch]) {
            ++index;
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

void Parser::ValidateKeyWords() {
    for (size_t index = 1; index < kKeyWordCount; ++index) {
        SkASSERT((int) kKeyWords[index - 1].fKeyWord + 1
                == (int) kKeyWords[index].fKeyWord);
        SkASSERT(0 > strcmp(kKeyWords[index - 1].fName, kKeyWords[index].fName));
    }
}

void Parser::addKeyword(KeyWord keyWord) {
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

bool Parser::checkForWord() {
    if (fParent->fTokens.size() >= 305) {
        SkDebugf("");
    }
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
                this->reportError("expected preprocessor directive");
            }
            this->popBracket();  // pop if
            this->addDefinition(poundDef);  // push elif back
        } break;
        // this ends a # directive link
        case KeyWord::kEndif:
        // FIXME : should this be calling popBracket() instead?
            this->popObject();  // pop endif
            if (Bracket::kPound != fParent->fBracket) {
                this->reportError("expected preprocessor directive");
            }
            this->popBracket();  // pop if/else
        break;
        default:
            SkASSERT(0);
    }
    return true;
}

bool Parser::findComments(const Definition& includeDef, Definition* markupDef) {
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

bool Parser::parseClass(Definition* includeDef) {
    SkASSERT(includeDef->fTokens.size() > 0);
    if (includeDef->fTokens.size() == 1) {
        return true;  // forward declaration only
    }
	// parse class header
    auto iter = includeDef->fTokens.begin();
    if (!strncmp(iter->fStart, "SK_API", iter->fEnd - iter->fStart)) {
        // todo : documentation is ignoring this for now
        iter = std::next(iter);
    }
    string nameStr(iter->fStart, iter->fEnd - iter->fStart);
    if (string::npos != nameStr.find('{')) {
        return this->reportError(*iter, "malformed class definition, unexpected open brace");
    } 
    iter = std::next(iter);
    this->skipWhiteSpace();
    if ('{' != iter->fStart[0]) {
        return this->reportError(*iter, "malformed class definition, missing open brace");
    }
    Definition* asMarkupDef = this->findIncludeObject(*includeDef, MarkType::kClass, nameStr);
    if (!asMarkupDef) {
        return false;
    }
    asMarkupDef->fStart = iter->fStart;
    if (!this->findComments(*includeDef, asMarkupDef)) {
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
            && publicLen == iter->fEnd - iter->fStart
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
            && (protectedLen != iter->fEnd - iter->fStart
            || strncmp(iter->fStart, protectedName, protectedLen))
            && (privateLen != iter->fEnd - iter->fStart
            || strncmp(iter->fStart, privateName, privateLen))) {
        iter = std::next(iter);
        ++lastPublic;
    }
    while (childIter != includeDef->fChildren.end() && (*childIter)->fParentIndex < lastPublic) {
        Definition* child = *childIter;
        if (Definition::Type::kBracket == child->fType) {
            if (Bracket::kParen == child->fBracket) {
                auto tokenIter = includeDef->fTokens.begin();
                std::advance(tokenIter, child->fParentIndex);
                tokenIter = std::prev(tokenIter);
                string nameStr(tokenIter->fStart, tokenIter->fEnd - tokenIter->fStart);
                asMarkupDef->fTokens.emplace_back(MarkType::kMethod, child->fStart, asMarkupDef);
                Definition* asMarkupChild = &asMarkupDef->fTokens.back();
                asMarkupChild->fName = nameStr;
                if (!this->findComments(*child, asMarkupChild)) {
                    return false;
                }
            }
        }
        // start here:
        // todo: add the inner object to the class
        childIter = std::next(childIter);
    }
	return true;
}

bool Parser::parseComment(const Definition& includeDef, Definition* markupDef) {
    fChar = fLine = includeDef.fStart;
    fEnd = includeDef.fEnd;
    Definition* markupComment = &markupDef->fTokens.back();
    // parse doxygen if present
    if (!strncmp(fChar, "**", 2)) {
        fChar += 2;
        this->skipWhiteSpace();
        if ('\\' == fChar[0]) {
            ++fChar;
            if (!this->skipWord(kKeyWords[(int) markupDef->fKeyWord].fName)) {
                return reportError(includeDef, "missing object type");
            }
            if (!this->skipWord(markupDef->fName.c_str())) {
                return reportError(includeDef, "missing object name");
            }

        }
    }
    // remove leading '*' if present
    string comment;
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
        comment += string(fChar, next - fChar);
        fChar = next;
    }
    // start here : 
    // with markupDef->fName == "getHinting", comment == "Specifies the level ..."
    markupDef->fTokens.emplace_back(MarkType::kComment, comment, markupDef);
    return true;
}

bool Parser::parseDefine() {

	return true;
}

bool Parser::parseEnum() {

	return true;
}

bool Parser::parseInclude(const string& name) {
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
    if (!this->parseObjects(fParent)) {
        return false;
    }
    return true;
}

bool Parser::parseObjects(Definition* parent) {
    bool success = true;
	for (auto& child : parent->fChildren) {
        // set up for error reporting
        fLine = fChar = child->fStart;
        fEnd = child->fEnd;
        // todo: put original line number in child as well
        switch (child->fType) {
            case Definition::Type::kKeyWord:
	            switch (child->fKeyWord) {
		            case KeyWord::kClass: 
                        success = parseClass(child);
			            break;
		            case KeyWord::kEnum:
                        success = parseEnum();
			            break;
                    case KeyWord::kStruct:
                        success = parseClass(child);
			            break;
                    case KeyWord::kTemplate:
                        success = parseTemplate();
                        break;
                    case KeyWord::kTypedef:
                        success = parseTypedef();
                        break;
                    case KeyWord::kUnion:
                        success = parseUnion();
                        break;
                    default:
                        return this->reportError<bool>("unhandled keyword");
	            }
                break;
            case Definition::Type::kBracket:
                switch (child->fBracket) {
                    case Bracket::kSlashSlash:
                    case Bracket::kSlashStar:
                        // comments are picked up by parsing objects first
                        break;
                    case Bracket::kPound:
                        // special-case the #xxx xxx_DEFINED entries
                        switch (child->fKeyWord) {
                            case KeyWord::kIfndef:
                                if (child->boilerplateIfDef(fParent)) {
                                    if (!this->parseObjects(child)) {
                                        return false;
                                    }
                                    break;
                                }
                                goto preproError;
                            case KeyWord::kDefine:
                                if (child->boilerplateIfDef(fParent)) {
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
                            default:
                            preproError:
                                return this->reportError<bool>("unhandled preprocessor");
                        }
                        break;
                    default:
                        return this->reportError<bool>("unhandled bracket");
                }
                break;
            default:
                return this->reportError<bool>("unhandled type");
                break;
        }
    }
	return success;
}

bool Parser::parseTemplate() {

	return true;
}

bool Parser::parseTypedef() {

	return true;
}

bool Parser::parseUnion() {

	return true;
}

bool Parser::parseChar() {
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
            if (fLineCount == 81) 
                SkDebugf("");
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
        case '(':
        case '[':
        case '{':
            if (fLineCount == 88) {
                SkDebugf("");
            }
            if (fInCharCommentString) {
                break;
            }
            if (!fInBrace && !this->checkForWord()) {
                return false;
            }
            this->pushBracket('(' == test ? Bracket::kParen :
                    '[' == test ? Bracket::kSquare : Bracket::kBrace);
            if (!fInBrace && '{' == test && fParent->fParent->isFunction()) {
                fInBrace = fParent;
            }
            break;
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
        case ':':
            SkDebugf("");
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
