/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bookmaker.h"

static const TypeNames kTypeNames[] = {
    { "",           MarkType::kUnknown },
    { "class ",     MarkType::kClass },
    { "",           MarkType::kColumn },
    { "",           MarkType::kComment },
	{ "",           MarkType::kConst },
    { "define ",    MarkType::kDefine },
    { "",           MarkType::kDoxygen },
	{ "enum ",      MarkType::kEnum },
    { "",           MarkType::kExample },
    { "",           MarkType::kMethod },
	{ "",           MarkType::kParameter },
    { "",           MarkType::kRow },
    { "",           MarkType::kStdOut },
	{ "struct ",    MarkType::kStruct },
    { "",           MarkType::kTable },
	{ "template ",  MarkType::kTemplate },
    { "",           MarkType::kText },
    { "",           MarkType::kToDo },
	{ "typedef ",   MarkType::kTypedef },
	{ "union ",     MarkType::kUnion },
};

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
            if (1 != fBracketStack.size() || Bracket::kBrace != fBracketStack.top().fBracket) {
                return reportError<bool>("public: is unexpectedly nested");
            }
            break;
        }
        if (this->endsWith("};") && 1 == fBracketStack.size() && Bracket::kBrace == fBracketStack.top().fBracket) {
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
        if (1 != fBracketStack.size() && Bracket::kSlashSlash == fBracketStack.top().fBracket) {
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
    fTrackBrackets = false;
    while (++fLineCount, fLine += lineLen, fChar = fLine, !this->eof() && !foundEnd) {
        if (oneLine) {
            break;
        }
        lineLen = this->lineLength(); 
        foundEnd = this->endsWith("*/");
    }
    fTrackBrackets = true;
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
    std::stack<BracketStart>& stack = fBracketStack;
    int depth = 0;
    while (fChar < fEnd) {
        char ch = this->next();
        size_t stackSize = stack.size();
        if (depth <= stackSize) {
            depth = stackSize;
            continue;
        }
        Bracket top = fLastBracket.fBracket;
        bool inComment = Bracket::kSlashSlash == top || Bracket::kSlashStar == top;
        if (inComment) {
            fDefinitions.emplace_back(MarkType::kComment, fLastBracket.fStart, fParent);
            fParent->fChildren.push_back(&fDefinitions.back());
            depth = stackSize;
            continue;
        }
        if (Bracket::kBrace == top) {
        }
        bool inString = Bracket::kString == top;
        bool inChar = Bracket::kChar == top;
        bool inCharCommentString = inChar || inComment || inString;
        if (inCharCommentString) {
            lastSkip = stack.top();
            continue;
        }
        Bracket last = lastSkip.fBracket;
        
    }
	const char* comment = nullptr;  // TODO incomplete
    if (!strstr(comment, "Copyright")) {
        return this->reportError<bool>("expected Copyright");
    }
    this->skipWhiteSpace();
    if (!this->startsWith("#ifndef ")) {
        return this->reportError<bool>("expected #ifndef SkXXX_DEFINED");
    }
    if (!this->skipToLineStart()) {
        return this->reportError<bool>("unexpected eof");
    }
    if (!this->startsWith("#define ")) {
        return this->reportError<bool>("expected #define SkXXX_DEFINED");
    }
    do {  // skip includes
        if (!this->skipToLineStart()) {
            return this->reportError<bool>("unexpected eof");
        }
        if ('#' != this->peek()) {
            break;
        }
        if (!this->startsWith("#include ")) {
            return this->reportError<bool>("expected #include");
        }
    } while (true);
    do {  // skip forward declared classes and structs
        if (!this->skipToLineStart()) {
            return this->reportError<bool>("unexpected eof");
        }
        if ((!this->startsWith("class") && !this->startsWith("struct")) || !this->endsWith(";")) {
            break;
        }
    } while (true);
	return Parser::parseObjects();
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
    return MarkType::kUnknown;
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

bool Parser::trackBracket(char test) {
    char prev = &fChar[-2] >= fStart ? fChar[-2] : '\0';
    std::stack<BracketStart>& stack = fBracketStack;
    Bracket top = stack.empty() ? Bracket::kNone : stack.top().fBracket;
    bool inComment = Bracket::kSlashSlash == top || Bracket::kSlashStar == top;
    bool inString = Bracket::kString == top;
    bool inChar = Bracket::kChar == top;
    bool inCharCommentString = inChar || inComment || inString;
    switch (test) {
        case '\n':
            if ('\\' != prev) {
                if (inChar) {
                    return reportError<bool>("malformed char");
                }
                if (inString) {
                    return reportError<bool>("malformed string");
                }
                if (Bracket::kSlashSlash == top) {
                    this->popBracket();
                }
            }
            break;
        case '*':
            if (!inCharCommentString && '/' == prev) {
                stack.emplace(fChar, Bracket::kSlashStar);
            }
            break;
        case '/':
            if ('*' == prev) {
                if (!inCharCommentString) {
                    return reportError<bool>("malformed closing comment");
                }
                if (Bracket::kSlashStar == top) {
                    this->popBracket();
                }
            } else if (!inCharCommentString && '*' == prev) {
                stack.emplace(fChar, Bracket::kSlashSlash);
            }
            break;
        case '\'':
            if (Bracket::kChar == top) {
                this->popBracket();
            } else if (!inComment && !inString && prev != '\\') {
                stack.emplace(fChar, Bracket::kChar);
            }
            break;
        case '\"':
            if (Bracket::kString == top) {
                this->popBracket();
            } else if (!inComment && !inChar && prev != '\\') {
                stack.emplace(fChar, Bracket::kString);
            }
            break;
        case '(':
        case '[':
        case '{':
            if (inCharCommentString) {
                break;
            }
            stack.emplace(fChar, '(' == test ? Bracket::kParen :
                    '[' == test ? Bracket::kSquare : Bracket::kBrace);
            if ('{' == test) {
                // ToDo: this will fail if there's a comment between the name and the brace
                std::string name = this->nameBefore(fChar);
                if (name == "class") {
                    this->addDefinition(MarkType::kClass);
                    break;
                }
                if (name == "enum") {
                    this->addDefinition(MarkType::kEnum);
                    break;
                }
                if (name == "struct") {
                    this->addDefinition(MarkType::kStruct);
                    break;
                }

            }
            break;
        case ')':
        case ']':
        case '}':
            if (inCharCommentString) {
                break;
            }
            if ((')' == test ? Bracket::kParen :
                    ']' == test ? Bracket::kSquare : Bracket::kBrace) == top) {
                this->popBracket();
            } else {
                return reportError<bool>("malformed close bracket");
            }
            break;
        case '#': {
            if (inCharCommentString) {
                break;
            }
            // look ahead to see if this is an #if.. #el.. or #end
            {
                const char* chPtr = fChar;
                while (' ' == *chPtr || '\t' == *chPtr) {  // skip tabs and spaces
                    ++chPtr;
                }
                // see if this is an #if.. #el.. or #end
                if (!strncmp(chPtr, "if", 2)) {
                    stack.emplace(fChar, Bracket::kPoundIf);
                } else if (!strncmp(chPtr, "el", 2)) {
                    
                } else if (!strncmp(chPtr, "endif", 5)) {
                    if (Bracket::kPoundIf != top) {
                        return reportError<bool>("malformed #endif");
                    }
                    this->popBracket();
                }
            } break;
        }
    }
    return true;
}
