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

void Parser::ValidateKeyWords() {
    for (size_t index = 1; index < kKeyWordCount; ++index) {
        SkASSERT((int) kKeyWords[index - 1].fKeyWord + 1
                == (int) kKeyWords[index].fKeyWord);
        SkASSERT(0 > strcmp(kKeyWords[index - 1].fName, kKeyWords[index].fName));
    }
}

void Parser::addKeyword(KeyWord keyWord) {
    fDefinitions.emplace_back(keyWord, fIncludeWord, fChar, fParent);
    Definition* def = &fDefinitions.back();
    fParent->fChildren.push_back(def);
    if (false && KeyProperty::kObject == kKeyWords[(int) keyWord].fProperty) {
        fParent = def;
    }
}

bool Parser::checkForWord() {
    if (fIncludeWord) {
        KeyWord keyWord = FindKey(fIncludeWord, fChar);
        if (KeyWord::kNone != keyWord) {
            Definition* poundDef = fBracketStack.top();
            if (Bracket::kPound == poundDef->fBracket && KeyWord::kNone == poundDef->fKeyWord) {
                poundDef->fKeyWord = keyWord;
                switch (keyWord) {
                    // these do not link to other # directives
                    case KeyWord::kDefine:
                    case KeyWord::kInclude:
                    // start here
                        this->popBracket();
                    break;
                    // these start a # directive link
                    case KeyWord::kIf:
                    case KeyWord::kIfdef:
                    case KeyWord::kIfndef:
                    break;
                    // these continue a # directive link
                    case KeyWord::kElif:
                    case KeyWord::kElse: {
                        fBracketStack.pop();  // pop elif
                        if (Bracket::kPound != fBracketStack.top()->fBracket) {
                            this->reportError("expected preprocessor directive");
                        }
                        fBracketStack.pop();  // pop if
                        fBracketStack.push(poundDef);  // push elif back
                    } break;
                    // this ends a # directive link
                    case KeyWord::kEndif:
                    // FIXME : should this be calling popBracket() instead?
                        fBracketStack.pop();  // pop endif
                        if (Bracket::kPound != fBracketStack.top()->fBracket) {
                            this->reportError("expected preprocessor directive");
                        }
                        fBracketStack.pop();  // pop if/else
                    break;
                    default:
                        SkASSERT(0);
                }
            } else {
                this->addKeyword(keyWord);
            }
        } else {
            this->addWord();
        }
        fIncludeWord = nullptr;
    }
    return true;
}

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

bool Parser::parseClass(const char* comment) {
	// parse class header
    ptrdiff_t len = this->lineEnd() - fChar;
    const char* colon = this->contains(" : ");
    const char* brace = (const char*) memchr(fChar, '{', (size_t) len);
    const char* semi = (const char*) memchr(fChar, ';', (size_t) len);
    if (!brace) {
        return this->reportError<bool>("malformed class definition, missing open brace");
    } else if (semi && semi < brace) {
        return this->reportError<bool>("malformed class definition, semi before brace");
    }
    const char* name = this->wordBefore(colon && colon < brace ? colon : brace);
    std::string nameStr(name, strchr(name, ' ') - name);
    Definition* definition =  this->findIncludeObject(MarkType::kClass, nameStr);
    if (definition->fStart) {
        return this->reportError<bool>("class already defined");
    }
    definition->fStart = fChar;
    this->setAsParent(definition);
	// skip until public
    // look for end of class "};" as well in case class has no public section
    do {
        if (!this->skipToLineStart()) {
            return false;
        }
        if (this->startsWith("public:")) {
            if (1 != fBracketStack.size() || Bracket::kBrace != fBracketStack.top()->fBracket) {
                return reportError<bool>("public: is unexpectedly nested");
            }
            break;
        }
        if (this->endsWith("};") && 1 == fBracketStack.size() && Bracket::kBrace == fBracketStack.top()->fBracket) {
            return true;
        }
    } while (true);
    SkDebugf("");
	// parse until protected / private
	while (!this->eof()) {
		/* 

			*/
	}
	return true;
}

const char* Parser::parseComment() {
    this->skipWhiteSpace();
    if ('/' != this->peek()) {
        return nullptr;
    }

    this->next();
    if ('/' == this->peek()) {
        this->next();  // should push comment on token stack
        if (1 != fBracketStack.size() && Bracket::kSlashSlash == fBracketStack.top()->fBracket) {
            return this->reportError<const char*>("expected comment");
        }
    } else if ('*' == this->peek()) {

    }
    this->next();  // should push comment on token stack
//    if (1 != fBracketStack.size() && Bracket::kSlashSlash == fBracketStack.top().fBracket
    bool oneLine = '/' == this->next();
    const char* result = fChar;
    const char* start = fLine;
    ptrdiff_t lineLen = this->lineLength();
	bool foundEnd = false;
    while (++fLineCount, fLine += lineLen, fChar = fLine, !this->eof() && !foundEnd) {
        if (oneLine) {
            break;
        }
        lineLen = this->lineLength(); 
        foundEnd = this->endsWith("*/");
    }
    return result;
}

bool Parser::parseConst(const char* comment) {

	return true;
}

bool Parser::parseDefine(const char* comment) {

	return true;
}

bool Parser::parseEnum(const char* comment) {

	return true;
}

bool Parser::parseInclude(const std::string& name) {
    Definition& root = fIncludeMap[name];
    root.fName = name;
    fParent = &root;
	// skip copyright
    std::stack<Definition*>& stack = fBracketStack;
    int depth = 0;
    while (fChar < fEnd) {
        if (!this->parseChar()) {
            return false;
        }
    }
    return true;
}

bool Parser::parseObjects() {
    bool success = true;
	while (success && !this->eof()) {
	    const char* comment = this->parseComment();
        MarkType type = this->parsePeek();
	    switch (type) {
		    case MarkType::kClass: 
                success = parseClass(comment);
			    break;
            case MarkType::kConst:
                success = parseConst(comment);
                break;
		    case MarkType::kDefine:
                success = parseDefine(comment);
			    break;
		    case MarkType::kEnum:
                success = parseEnum(comment);
			    break;
            case MarkType::kStruct:
                success = parseClass(comment);
			    break;
            case MarkType::kTemplate:
                success = parseTemplate(comment);
                break;
            case MarkType::kTypedef:
                success = parseTypedef(comment);
                break;
            case MarkType::kUnion:
                success = parseUnion(comment);
                break;
            default:
                return this->reportError<bool>("unknown type");
	    }
    }
	return success;
}

// leaving this as a bunch of tests rather than a loop, anticipating future
// logic will be required
MarkType Parser::parsePeek() {
    this->skipWhiteSpace();
#if 0
    if (this->startsWith(kTypeNames[(int) MarkType::kClass].fName)) {
        return MarkType::kClass;
    }
    if (this->startsWith("constexpr ") || this->startsWith("static constexpr ")) {
        return MarkType::kConst;
    }
    if ('#' == fChar[0]) {
        (void) this->next();
        this->skipWhiteSpace();
        if (this->startsWith(kTypeNames[(int) MarkType::kDefine].fName)) {
            return MarkType::kDefine;
        }
        // TODO : support ifdef / endif
    }
    if (this->startsWith(kTypeNames[(int) MarkType::kEnum].fName)) {
        return MarkType::kEnum;
    }
    if (this->startsWith(kTypeNames[(int) MarkType::kStruct].fName)) {
        return MarkType::kStruct;
    }
    if (this->startsWith(kTypeNames[(int) MarkType::kTemplate].fName)) {
        return MarkType::kTemplate;
    }
    if (this->startsWith(kTypeNames[(int) MarkType::kTypedef].fName)) {
        return MarkType::kTypedef;
    }
    if (this->startsWith(kTypeNames[(int) MarkType::kUnion].fName)) {
        return MarkType::kUnion;
    }
    #endif
    return MarkType::kNone;
}

bool Parser::parseTemplate(const char* comment) {

	return true;
}

bool Parser::parseTypedef(const char* comment) {

	return true;
}

bool Parser::parseUnion(const char* comment) {

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
            if (Bracket::kPound == this->topBracket()
                    && KeyWord::kNone == fBracketStack.top()->fKeyWord) {
                return this->reportError<bool>("unhandled preprocessor directive");
            }
            if (Bracket::kSlashSlash == this->topBracket()) {
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
            } else if (!fInComment && !fInString && fPrev != '\\') {
                if (fIncludeWord) {
                    return this->reportError<bool>("word then single-quote");
                }
                this->pushBracket(Bracket::kChar);
            }
            break;
        case '\"':
            if (Bracket::kString == this->topBracket()) {
                this->popBracket();
            } else if (!fInComment && !fInChar && fPrev != '\\') {
                if (fIncludeWord) {
                    return this->reportError<bool>("word then double-quote");
                }
                this->pushBracket(Bracket::kString);
            }
            break;
        case '(':
        case '[':
        case '{':
            if (fInCharCommentString) {
                break;
            }
            this->pushBracket('(' == test ? Bracket::kParen :
                    '[' == test ? Bracket::kSquare : Bracket::kBrace);
            if (!fInBrace) {
                if (!this->checkForWord()) {
                    return false;
                }
                if ('{' == test) {
                    fInBrace = fBracketStack.top();
                }
            }
            break;
        case '<':
            if (fInCharCommentString) {
                break;
            }
            if (fInBrace) {
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
            } else if (fInBrace == fBracketStack.top()) {
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
            if (fInCharCommentString) {
                break;
            }
            if (fInBrace) {
                break;
            }
            if (!this->checkForWord()) {
                return false;
            }
            if (Bracket::kAngle == this->topBracket()) {
                this->popBracket();
            } else {
                return reportError<bool>("malformed close angle bracket");
            }
            break;
        case '#': {
            if (fInCharCommentString) {
                break;
            }
            SkASSERT(!fIncludeWord);  // don't expect this, curious if it is triggered
            this->pushBracket(Bracket::kPound);
            break;
        }
        case '&':
        case ':':
        case ';':
        case ' ':
        case ',':
            if (!this->checkForWord()) {
                return false;
            }
            break;
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
            if (!fInBrace && !fInCharCommentString && !fIncludeWord) {
                fIncludeWord = fChar;
            }
            break;
    }
done:
    fPrev = test;
    ++fChar;
    return true;
}
