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
//    start here;
    // look for end of class "};" ? as well in case class has no public section
    int braceCount = 0;
    do {
        if (!this->skipToLineStart()) {
            return false;
        }
        if (this->startsWith("public:")) {
            break;
        }
        if (this->endsWith("};")) {
            return true;
        }
    } while (true);
    
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
    if ('/' != this->peek() && '*' != this->peek()) {
        return this->reportError<const char*>("expected comment");
    }
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

bool Parser::parseInclude() {
	// skip copyright
	const char* comment = this->parseComment();
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
        if (!(this->startsWith("class") && !this->startsWith("struct"))
                || !this->endsWith(";")) {
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

