/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bookmaker.h"

#include "SkOSFile.h"
#include "SkOSPath.h"


/*  recipe for generating timestamps for existing doxygen comments
find include/core -type f -name '*.h' -print -exec git blame {} \; > ~/all.blame.txt

space table better for Constants
should Return be on same line as 'Return Value'?
remove anonymous header, e.g. Enum SkPaint::::anonymous_2
Text Encoding anchors in paragraph are echoed instead of being linked to anchor names
    also should not point to 'undocumented' since they are resolvable links
#Member lost all formatting
#List needs '# content ##', formatting
consts like enum members need fully qualfied refs to make a valid link
enum comments should be disallowed unless after #Enum and before first #Const
    ... or, should look for enum comments in other places
trouble with aliases, plurals
    need to keep first letter of includeWriter @param / @return lowercase
    Quad -> quad, Quads -> quads
check for summary containing all methods
 */

static string normalized_name(string name) {
    string normalizedName = name;
    std::replace(normalizedName.begin(), normalizedName.end(), '-', '_');
    do {
        size_t doubleColon = normalizedName.find("::", 0);
        if (string::npos == doubleColon) {
            break;
        }
        normalizedName = normalizedName.substr(0, doubleColon)
            + '_' + normalizedName.substr(doubleColon + 2);
    } while (true);
    return normalizedName;
}

static size_t count_indent(const string& text, size_t test, size_t end) {
    size_t result = test;
    while (test < end) {
        if (' ' != text[test]) {
            break;
        }
        ++test;
    }
    return test - result;
}

static void add_code(const string& text, int pos, int end, 
        size_t outIndent, size_t textIndent, string& example) {
    do {
         // fix this to move whole paragraph in, out, but preserve doc indent
        int nextIndent = count_indent(text, pos, end);
        size_t len = text.find('\n', pos);
        if (string::npos == len) {
            len = end;
        }
        if ((size_t) (pos + nextIndent) < len) {
            size_t indent = outIndent + nextIndent;
            SkASSERT(indent >= textIndent);
            indent -= textIndent;
            for (size_t index = 0; index < indent; ++index) {
                example += ' ';
            }
            pos += nextIndent;
            while ((size_t) pos < len) {
                example += '"' == text[pos] ? "\\\"" :
                    '\\' == text[pos] ? "\\\\" : 
                    text.substr(pos, 1);
                ++pos;
            }
            example += "\\n";
        } else {
            pos += nextIndent;
        }
        if ('\n' == text[pos]) {
            ++pos;
        }
    } while (pos < end);
}

// fixme: this will need to be more complicated to handle all of Skia
// for now, just handle paint -- maybe fiddle will loosen naming restrictions
void Definition::setCanonicalFiddle() {
    fMethodType = Definition::MethodType::kNone;
    size_t doubleColons = fName.find("::", 0);
    SkASSERT(string::npos != doubleColons);
    string base = fName.substr(0, doubleColons);
    string result = base + "_";
    doubleColons += 2;
    if (string::npos != fName.find('~', doubleColons)) {
        fMethodType = Definition::MethodType::kDestructor;
        result += "destructor";
    } else {
        bool isMove = string::npos != fName.find("&&", doubleColons);
        const char operatorStr[] = "operator";
        size_t opPos = fName.find(operatorStr, doubleColons);
        if (string::npos != opPos) {
            fMethodType = Definition::MethodType::kOperator;
            opPos += sizeof(operatorStr) - 1;
            if ('!' == fName[opPos]) {
                SkASSERT('=' == fName[opPos + 1]);
                result += "not_equal_operator"; 
            } else if ('=' == fName[opPos]) {
                if ('(' == fName[opPos + 1]) {
                    result += isMove ? "move_" : "copy_"; 
                    result += "assignment_operator"; 
                } else {
                    SkASSERT('=' == fName[opPos + 1]);
                    result += "equal_operator"; 
                }
            } else {
                SkASSERT(0);  // todo: incomplete
            }
        } else {
            size_t parens = fName.find("()", doubleColons);
            if (string::npos != parens) {
                string methodName = fName.substr(doubleColons, parens - doubleColons);
                do {
                    size_t nextDouble = methodName.find("::");
                    if (string::npos == nextDouble) {
                        break;
                    }
                    base = methodName.substr(0, nextDouble);
                    result += base + '_';
                    methodName = methodName.substr(nextDouble + 2);
                    doubleColons += nextDouble + 2;
                } while (true);
                if (base == methodName) {
                    fMethodType = Definition::MethodType::kConstructor;
                    result += "empty_constructor"; 
                } else {
                    result += fName.substr(doubleColons, fName.length() - doubleColons - 2);
                }
            } else {
                size_t openParen = fName.find('(', doubleColons);
                if (string::npos == openParen) {
                    result += fName.substr(doubleColons);
                } else {
                    size_t comma = fName.find(',', doubleColons);
                    if (string::npos == comma) {
                        result += isMove ? "move_" : "copy_"; 
                    }
                    fMethodType = Definition::MethodType::kConstructor;
                    // name them by their param types,
                    //   e.g. SkCanvas__int_int_const_SkSurfaceProps_star
                    // TODO: move forward until parens are balanced and terminator =,)
                    TextParser params("", &fName[openParen] + 1, &*fName.end(), 0);
                    bool underline = false;
                    while (!params.eof()) {
    //                    SkDEBUGCODE(const char* end = params.anyOf("(),="));  // unused for now
    //                    SkASSERT(end[0] != '(');  // fixme: put off handling nested parentheseses
                        if (params.startsWith("const") || params.startsWith("int")
                                || params.startsWith("Sk")) {
                            const char* wordStart = params.fChar;
                            params.skipToNonAlphaNum();
                            if (underline) {
                                result += '_';
                            } else {
                                underline = true;
                            }
                            result += string(wordStart, params.fChar - wordStart);
                        } else {
                            params.skipToNonAlphaNum();
                        }
                        if (!params.eof() && '*' == params.peek()) {
                            if (underline) {
                                result += '_';
                            } else {
                                underline = true;
                            }
                            result += "star";
                            params.next();
                            params.skipSpace();
                        }
                        params.skipToAlpha();
                    }
                }
            }
        }
    }
    fFiddle = normalized_name(result);
}

bool Definition::exampleToScript(string* result) const {
    bool hasFiddle = true;
    const Definition* platform = this->hasChild(MarkType::kPlatform);
    if (platform) {
        TextParser platParse(platform);
        hasFiddle = !platParse.strnstr("!fiddle", platParse.fEnd);
    }
    if (!hasFiddle) {
        *result = "";
        return true;
    }
    string text = this->extractText(Definition::TrimExtract::kNo);
    const char drawWrapper[] = "void draw(SkCanvas* canvas) {";
    const char drawNoCanvas[] = "void draw(SkCanvas* ) {";
    size_t nonSpace = 0;
    while (nonSpace < text.length() && ' ' >= text[nonSpace]) {
        ++nonSpace;
    }
    bool hasFunc = !text.compare(nonSpace, sizeof(drawWrapper) - 1, drawWrapper);
    bool noCanvas = !text.compare(nonSpace, sizeof(drawNoCanvas) - 1, drawNoCanvas);
    bool hasCanvas = string::npos != text.find("SkCanvas canvas");
    SkASSERT(!hasFunc || !noCanvas);
    bool textOut = string::npos != text.find("SkDebugf(")
            || string::npos != text.find("dump(")
            || string::npos != text.find("dumpHex(");
    string heightStr = "256";
    string widthStr = "256";
    bool preprocessor = text[0] == '#';
    string normalizedName(fFiddle);
    string code;
    string imageStr = "0";
    for (auto const& iter : fChildren) {
        switch (iter->fMarkType) {
            case MarkType::kError:
                result->clear();
                return true;
            case MarkType::kHeight:
                heightStr = string(iter->fContentStart, iter->fContentEnd - iter->fContentStart);
                break;
            case MarkType::kWidth:
                widthStr = string(iter->fContentStart, iter->fContentEnd - iter->fContentStart);
                break;
            case MarkType::kDescription:
                // ignore for now
                break;
            case MarkType::kFunction: {
                // emit this, but don't wrap this in draw()
                string funcText(iter->fContentStart, iter->fContentEnd - iter->fContentStart - 1);
                size_t pos = 0;
                while (pos < funcText.length() && ' ' > funcText[pos]) {
                    ++pos;
                }
                size_t indent = count_indent(funcText, pos, funcText.length());
                add_code(funcText, pos, funcText.length(), 0, indent, code);
                code += "\\n";
                } break;
            case MarkType::kComment:
                break;
            case MarkType::kImage:
                imageStr = string(iter->fContentStart, iter->fContentEnd - iter->fContentStart);
                break;
            case MarkType::kToDo:
                break;
            case MarkType::kMarkChar:
            case MarkType::kPlatform:
                // ignore for now
                break;
            case MarkType::kStdOut:
                textOut = true;
                break;
            default:
                SkASSERT(0);  // more coding to do
        }
    }
    string textOutStr = textOut ? "true" : "false";
    size_t pos = 0;
    while (pos < text.length() && ' ' > text[pos]) {
        ++pos;
    }
    size_t end = text.length();
    size_t outIndent = 0;
    size_t textIndent = count_indent(text, pos, end);
    bool wrapCode = !hasFunc && !noCanvas && !preprocessor;
    if (wrapCode) {
        code += hasCanvas ? drawNoCanvas : drawWrapper;
        code += "\\n";
        outIndent = 4;
    }
    add_code(text, pos, end, outIndent, textIndent, code);
    if (wrapCode) {
        code += "}";
    }
    string example = "\"" + normalizedName + "\": {\n";
    example += "    \"code\": \"" + code + "\",\n";
    example += "    \"options\": {\n";
    example += "        \"width\": " + widthStr + ",\n";
    example += "        \"height\": " + heightStr + ",\n";
    example += "        \"source\": " + imageStr + ",\n";
    example += "        \"srgb\": false,\n";
    example += "        \"f16\": false,\n";
    example += "        \"textOnly\": " + textOutStr + ",\n";
    example += "        \"animated\": false,\n";
    example += "        \"duration\": 0\n";
    example += "    },\n";
    example += "    \"fast\": true\n";
    example += "}";
    *result = example;
    return true;
}

static void space_pad(string* str) {
    size_t len = str->length();
    if (len == 0) {
        return;
    }
    char last = (*str)[len - 1];
    if ('~' == last || ' ' >= last) {
        return;
    }
    *str += ' ';
}

//start here;
// see if it possible to abstract this a little bit so it can
// additionally be used to find params and return in method prototype that
// does not have corresponding doxygen comments
bool Definition::checkMethod() const {
    SkASSERT(MarkType::kMethod == fMarkType);
    // if method returns a value, look for a return child
    // for each parameter, look for a corresponding child
    const char* end = fContentStart;
    while (end > fStart && ' ' >= end[-1]) {
        --end;
    }
    TextParser methodParser(fFileName, fStart, end, fLineCount);
    methodParser.skipWhiteSpace();
    SkASSERT(methodParser.startsWith("#Method"));
    methodParser.skipName("#Method");
    methodParser.skipSpace();
    string name = this->methodName();
    if (MethodType::kNone == fMethodType && "()" == name.substr(name.length() - 2)) {
        name = name.substr(0, name.length() - 2);
    }
    bool expectReturn = this->methodHasReturn(name, &methodParser);
    bool foundReturn = false;
    bool foundException = false;
    for (auto& child : fChildren) {
        foundException |= MarkType::kDeprecated == child->fMarkType
                || MarkType::kExperimental == child->fMarkType;
        if (MarkType::kReturn != child->fMarkType) {
            if (MarkType::kParam == child->fMarkType) {
                child->fVisited = false;
            }
            continue;
        }
        if (!expectReturn) {
            return methodParser.reportError<bool>("no #Return expected");
        }
        if (foundReturn) {
            return methodParser.reportError<bool>("multiple #Return markers");
        }
        foundReturn = true;
    }
    if (expectReturn && !foundReturn && !foundException) {
        return methodParser.reportError<bool>("missing #Return marker");
    }
    const char* paren = methodParser.strnchr('(', methodParser.fEnd);
    if (!paren) {
        return methodParser.reportError<bool>("missing #Method function definition");
    }
    const char* nextEnd = paren;
    do {
        string paramName;
        methodParser.fChar = nextEnd + 1;
        methodParser.skipSpace();
        if (!this->nextMethodParam(&methodParser, &nextEnd, &paramName)) {
            continue;
        }
        bool foundParam = false;
        for (auto& child : fChildren) {
            if (MarkType::kParam != child->fMarkType) {
                continue;
            }
            if (paramName != child->fName) {
                continue;
            }
            if (child->fVisited) {
                return methodParser.reportError<bool>("multiple #Method param with same name");
            }
            child->fVisited = true;
            if (foundParam) {
                TextParser paramError(child);
                return methodParser.reportError<bool>("multiple #Param with same name");
            }
            foundParam = true;
            
        }
        if (!foundParam && !foundException) {
            return methodParser.reportError<bool>("no #Param found");
        }
        if (')' == nextEnd[0]) {
            break;
        }
    } while (')' != nextEnd[0]);
    for (auto& child : fChildren) {
        if (MarkType::kParam != child->fMarkType) {
            continue;
        }
        if (!child->fVisited) {
            TextParser paramError(child);
            return paramError.reportError<bool>("#Param without param in #Method");
        }
    }
    return true;
}

bool Definition::crossCheck2(const Definition& includeToken) const {
    TextParser parser(fFileName, fStart, fContentStart, fLineCount);
    parser.skipExact("#");
    bool isMethod = parser.skipName("Method");
    const char* contentEnd;
    if (isMethod) {
        contentEnd = fContentStart;
    } else if (parser.skipName("DefinedBy")) {
        contentEnd = fContentEnd;
        while (parser.fChar < contentEnd && ' ' >= contentEnd[-1]) {
            --contentEnd;
        }
        if (parser.fChar < contentEnd - 1 && ')' == contentEnd[-1] && '(' == contentEnd[-2]) {
            contentEnd -= 2;
        }
    } else {
        return parser.reportError<bool>("unexpected crosscheck marktype");
    }
    return crossCheckInside(parser.fChar, contentEnd, includeToken);
}

bool Definition::crossCheck(const Definition& includeToken) const {
    return crossCheckInside(fContentStart, fContentEnd, includeToken);
}

bool Definition::crossCheckInside(const char* start, const char* end,
        const Definition& includeToken) const {
    TextParser def(fFileName, start, end, fLineCount);
    TextParser inc("", includeToken.fContentStart, includeToken.fContentEnd, 0);
    if (inc.startsWith("SK_API")) {
        inc.skipWord("SK_API");
    }
    if (inc.startsWith("friend")) {
        inc.skipWord("friend");
    }
    if (inc.startsWith("SK_API")) {
        inc.skipWord("SK_API");
    }
    do {
        bool defEof;
        bool incEof;
        do {
            defEof = def.eof() || !def.skipWhiteSpace();
            incEof = inc.eof() || !inc.skipWhiteSpace();
            if (!incEof && '/' == inc.peek() && (defEof || '/' != def.peek())) {
                inc.next();
                if ('*' == inc.peek()) {
                    inc.skipToEndBracket("*/");
                    inc.next();
                } else if ('/' == inc.peek()) {
                    inc.skipToEndBracket('\n');
                }
            } else if (!incEof && '#' == inc.peek() && (defEof || '#' != def.peek())) {
                inc.next();
                if (inc.startsWith("if")) {
                    inc.skipToEndBracket("\n");
                } else if (inc.startsWith("endif")) {
                    inc.skipToEndBracket("\n");
                } else {
                    SkASSERT(0); // incomplete
                    return false;
                }
            } else {
                break;
            }
            inc.next();
        } while (true);
        if (defEof || incEof) {
            if (defEof == incEof || (!defEof && ';' == def.peek())) {
                return true;
            }
            return false;  // allow setting breakpoint on failure
        }
        char defCh;
        do {
            defCh = def.next();
            char incCh = inc.next();
            if (' ' >= defCh && ' ' >= incCh) {
                break;
            }
            if (defCh != incCh) {
                return false;
            }
            if (';' == defCh) {
                return true;
            }
        } while (!def.eof() && !inc.eof());
    } while (true);
    return false;
}

string Definition::formatFunction() const {
    const char* end = fContentStart;
    while (end > fStart && ' ' >= end[-1]) {
        --end;
    }
    TextParser methodParser(fFileName, fStart, end, fLineCount);
    methodParser.skipWhiteSpace();
    SkASSERT(methodParser.startsWith("#Method"));
    methodParser.skipName("#Method");
    methodParser.skipSpace();
    const char* lastStart = methodParser.fChar;
    const int limit = 80;  // todo: allow this to be set by caller or in global or something
    string methodStr;
    string name = this->methodName();
    const char* nameInParser = methodParser.strnstr(name.c_str(), methodParser.fEnd);
    methodParser.skipTo(nameInParser);
    const char* lastEnd = methodParser.fChar;
    const char* paren = methodParser.strnchr('(', methodParser.fEnd);
    size_t indent;
    if (paren) {
        indent = (size_t) (paren - lastStart) + 1;
    } else {
        indent = (size_t) (lastEnd - lastStart);
    }
    int written = 0;
    do {
        const char* nextStart = lastEnd;
        SkASSERT(written < limit);
        const char* delimiter = methodParser.anyOf(",)");
        const char* nextEnd = delimiter ? delimiter : methodParser.fEnd;
        if (delimiter) {
            while (nextStart < nextEnd && ' ' >= nextStart[0]) {
                ++nextStart;
            }
        }
        while (nextEnd > nextStart && ' ' >= nextEnd[-1]) {
            --nextEnd;
        }
        if (delimiter) {
            nextEnd += 1;
            delimiter += 1;
        }
        if (lastEnd > lastStart) {
            if (lastStart[0] != ' ') {
                space_pad(&methodStr);
            }
            methodStr += string(lastStart, (size_t) (lastEnd - lastStart));
            written += (size_t) (lastEnd - lastStart);
        }
        if (delimiter) {
            if (nextEnd - nextStart >= (ptrdiff_t) (limit - written)) {
                written = indent;
                methodStr += '\n';
                methodStr += string(indent, ' ');
            }
            methodParser.skipTo(delimiter);
        }
        lastStart = nextStart;
        lastEnd = nextEnd;
    } while (lastStart < lastEnd);
    return methodStr;
}

string Definition::fiddleName() const {
    string result;
    size_t start = 0;
    string parent;
    const Definition* parentDef = this;
    while ((parentDef = parentDef->fParent)) {
        if (MarkType::kClass == parentDef->fMarkType || MarkType::kStruct == parentDef->fMarkType) {
            parent = parentDef->fFiddle;
            break;
        }
    }
    if (parent.length() && 0 == fFiddle.compare(0, parent.length(), parent)) {
        start = parent.length();
        while (start < fFiddle.length() && '_' == fFiddle[start]) {
            ++start;
        }
    }
    size_t end = fFiddle.find_first_of('(', start);
    return fFiddle.substr(start, end - start);
}

const Definition* Definition::hasChild(MarkType markType) const {
    for (auto iter : fChildren) {
        if (markType == iter->fMarkType) {
            return iter;
        }
    }
    return nullptr;
}

const Definition* Definition::hasParam(const string& ref) const {
    SkASSERT(MarkType::kMethod == fMarkType);
    for (auto iter : fChildren) {
        if (MarkType::kParam != iter->fMarkType) {
            continue;
        }
        if (iter->fName == ref) {
            return &*iter;
        }

    }
    return nullptr;
}

bool Definition::methodHasReturn(const string& name, TextParser* methodParser) const {
    const char* lastStart = methodParser->fChar;
    const char* nameInParser = methodParser->strnstr(name.c_str(), methodParser->fEnd);
    methodParser->skipTo(nameInParser);
    const char* lastEnd = methodParser->fChar;
    const char* returnEnd = lastEnd;
    while (returnEnd > lastStart && ' ' == returnEnd[-1]) {
        --returnEnd;
    }
    bool expectReturn = 4 != returnEnd - lastStart || strncmp("void", lastStart, 4);
    if (MethodType::kNone != fMethodType && !expectReturn) {
        return methodParser->reportError<bool>("unexpected void");
    }
    switch (fMethodType) {
        case MethodType::kNone:
        case MethodType::kOperator:
            // either is fine
            break;
        case MethodType::kConstructor:
            expectReturn = true;
            break;
        case MethodType::kDestructor:
            expectReturn = false;
            break;
    }
    return expectReturn;
}

string Definition::methodName() const {
    string result;
    size_t start = 0;
    string parent;
    const Definition* parentDef = this;
    while ((parentDef = parentDef->fParent)) {
        if (MarkType::kClass == parentDef->fMarkType || MarkType::kStruct == parentDef->fMarkType) {
            parent = parentDef->fName;
            break;
        }
    }
    if (parent.length() && 0 == fName.compare(0, parent.length(), parent)) {
        start = parent.length();
        while (start < fName.length() && ':' == fName[start]) {
            ++start;
        }
    }
    if (fClone) {
        int lastUnder = fName.rfind('_');
        return fName.substr(start, (size_t) (lastUnder - start));
    }
    size_t end = fName.find_first_of('(', start);
    if (string::npos == end) {
        return fName.substr(start);
    }
    return fName.substr(start, end - start);
}

bool Definition::nextMethodParam(TextParser* methodParser, const char** nextEndPtr, 
        string* paramName) const {
    *nextEndPtr = methodParser->anyOf(",)");
    const char* nextEnd = *nextEndPtr;
    if (!nextEnd) {
        return methodParser->reportError<bool>("#Method function missing close paren");
    }
    const char* paramEnd = nextEnd;
    const char* assign = methodParser->strnstr(" = ", paramEnd);
    if (assign) {
        paramEnd = assign;
    }
    const char* closeBracket = methodParser->strnstr("]", paramEnd);
    if (closeBracket) {
        const char* openBracket = methodParser->strnstr("[", paramEnd);
        if (openBracket && openBracket < closeBracket) {
            while (openBracket < --closeBracket && isdigit(closeBracket[0]))
                ;
            if (openBracket == closeBracket) {
                paramEnd = openBracket;
            }
        }
    }
    while (paramEnd > methodParser->fChar && ' ' == paramEnd[-1]) {
        --paramEnd;
    }
    const char* paramStart = paramEnd;
    while (paramStart > methodParser->fChar && isalnum(paramStart[-1])) {
        --paramStart;
    }
    if (paramStart > methodParser->fChar && paramStart >= paramEnd) {
        return methodParser->reportError<bool>("#Method missing param name");
    }
    *paramName = string(paramStart, paramEnd - paramStart);
    if (!paramName->length()) {
        if (')' != nextEnd[0]) {
            return methodParser->reportError<bool>("#Method malformed param");
        }
        return false;
    }
    return true;
}

    bool ParserCommon::parseFile(const char* fileOrPath, const char* suffix) {
    if (!sk_isdir(fileOrPath)) {
        if (!this->parseFromFile(fileOrPath)) {
            SkDebugf("failed to parse %s\n", fileOrPath);
            return false;
        }
    } else {
        SkOSFile::Iter it(fileOrPath, suffix);
        for (SkString file; it.next(&file); ) {
            SkString p = SkOSPath::Join(fileOrPath, file.c_str());
            const char* hunk = p.c_str();
            if (!SkStrEndsWith(hunk, suffix)) {
                continue;
            }
            if (!this->parseFromFile(hunk)) {
                SkDebugf("failed to parse %s\n", hunk);
                return false;
            }
        }
    }
    return true;
}

bool Definition::paramsMatch(const string& match, const string& name) const {
    TextParser def(fFileName, fStart, fContentStart, fLineCount);
    const char* dName = def.strnstr(name.c_str(), fContentStart);
    if (!dName) {
        return false;
    }
    def.skipTo(dName);
    TextParser m(fFileName, &match.front(), &match.back() + 1, fLineCount);
    const char* mName = m.strnstr(name.c_str(), m.fEnd);
    if (!mName) {
        return false;
    }
    m.skipTo(mName);
    while (!def.eof() && ')' != def.peek() && !m.eof() && ')' != m.peek()) {
        const char* ds = def.fChar;
        const char* ms = m.fChar;
        const char* de = def.anyOf(") \n");
        const char* me = m.anyOf(") \n");
        def.skipTo(de);
        m.skipTo(me);
        if (def.fChar - ds != m.fChar - ms) {
            return false;
        }
        if (strncmp(ds, ms, (int) (def.fChar - ds))) {
            return false;
        }
        def.skipWhiteSpace();
        m.skipWhiteSpace();
    } 
    return !def.eof() && ')' == def.peek() && !m.eof() && ')' == m.peek();
}

void RootDefinition::clearVisited() {
    fVisited = false;
    for (auto& leaf : fLeaves) {
        leaf.second.fVisited = false;
    }
    for (auto& branch : fBranches) {
        branch.second->clearVisited();
    }
}

bool RootDefinition::dumpUnVisited() {
    bool allStructElementsFound = true;
    for (auto& leaf : fLeaves) {
        if (!leaf.second.fVisited) {
            // TODO: parse embedded struct in includeParser phase, then remove this condition
            size_t firstColon = leaf.first.find("::");
            size_t lastColon = leaf.first.rfind("::");
            if (firstColon != lastColon) {  // struct, two sets
                allStructElementsFound = false;
                continue;
            }
            SkDebugf("defined in bmh but missing in include: %s\n", leaf.first.c_str());
        }
    }
    for (auto& branch : fBranches) {
        allStructElementsFound &= branch.second->dumpUnVisited();
    }
    return allStructElementsFound;
}

const Definition* RootDefinition::find(const string& ref, AllowParens allowParens) const {
    const auto leafIter = fLeaves.find(ref);
    if (leafIter != fLeaves.end()) {
        return &leafIter->second;
    }
    if (AllowParens::kYes == allowParens && string::npos == ref.find("()")) {
        string withParens = ref + "()";
        const auto parensIter = fLeaves.find(withParens);
        if (parensIter != fLeaves.end()) {
            return &parensIter->second;
        }
    }
    const auto branchIter = fBranches.find(ref);
    if (branchIter != fBranches.end()) {
        const RootDefinition* rootDef = branchIter->second;
        return rootDef;
    }
    const Definition* result = nullptr;
    for (const auto& branch : fBranches) {
        const RootDefinition* rootDef = branch.second;
        result = rootDef->find(ref, allowParens);
        if (result) {
            break;
        }
    }
    return result;
}

/* 
  class contains named struct, enum, enum-member, method, topic, subtopic
     everything contained by class is uniquely named
     contained names may be reused by other classes
  method contains named parameters
     parameters may be reused in other methods
 */

bool BmhParser::addDefinition(const char* defStart, bool hasEnd, MarkType markType,
        const vector<string>& typeNameBuilder) {
    Definition* definition = nullptr;
    switch (markType) {
        case MarkType::kComment:
            if (!this->skipToDefinitionEnd(markType)) {
                return false;
            }
            return true;
        // these types may be referred to by name
        case MarkType::kClass:
        case MarkType::kStruct:
        case MarkType::kConst:
        case MarkType::kEnum:
        case MarkType::kEnumClass:
        case MarkType::kMember:
        case MarkType::kMethod:
        case MarkType::kTypedef: {
            if (!typeNameBuilder.size()) {
                return this->reportError<bool>("unnamed markup");
            }
            if (typeNameBuilder.size() > 1) {
                return this->reportError<bool>("expected one name only");
            }
            const string& name = typeNameBuilder[0];
            if (nullptr == fRoot) {
                fRoot = this->findBmhObject(markType, name);
                fRoot->fFileName = fFileName;
                definition = fRoot;
            } else {
                if (nullptr == fParent) {
                    return this->reportError<bool>("expected parent");
                }
                if (fParent == fRoot && hasEnd) {
                    RootDefinition* rootParent = fRoot->rootParent();
                    if (rootParent) {
                        fRoot = rootParent;
                    }
                    definition = fParent;
                } else {
                    if (!hasEnd && fRoot->find(name, RootDefinition::AllowParens::kNo)) {
                        return this->reportError<bool>("duplicate symbol");
                    }
                    if (MarkType::kStruct == markType || MarkType::kClass == markType) {
                        // if class or struct, build fRoot hierarchy
                        // and change isDefined to search all parents of fRoot
                        SkASSERT(!hasEnd);
                        RootDefinition* childRoot = new RootDefinition;
                        (fRoot->fBranches)[name] = childRoot;
                        childRoot->setRootParent(fRoot);
                        childRoot->fFileName = fFileName;
                        fRoot = childRoot;
                        definition = fRoot;
                    } else {
                        definition = &fRoot->fLeaves[name];
                    }
                }
            }
            if (hasEnd) {
                Exemplary hasExample = Exemplary::kNo;
                bool hasExcluder = false;
                for (auto child : definition->fChildren) {
                     if (MarkType::kExample == child->fMarkType) {
                        hasExample = Exemplary::kYes;
                     }
                     hasExcluder |= MarkType::kPrivate == child->fMarkType
                            || MarkType::kDeprecated == child->fMarkType
                            || MarkType::kExperimental == child->fMarkType
                            || MarkType::kNoExample == child->fMarkType;
                }
                if (fMaps[(int) markType].fExemplary != hasExample
                        && fMaps[(int) markType].fExemplary != Exemplary::kOptional) {
                    if (string::npos == fFileName.find("undocumented")
                            && !hasExcluder) {
                        hasExample == Exemplary::kNo ? 
                                this->reportWarning("missing example") : 
                                this->reportWarning("unexpected example");
                    }

                }
                if (MarkType::kMethod == markType) {
                    if (fCheckMethods && !definition->checkMethod()) {
                        return false;
                    }
                }
                if (!this->popParentStack(definition)) {
                    return false;
                }
            } else {
                definition->fStart = defStart;
                this->skipSpace();
                definition->fFileName = fFileName;
                definition->fContentStart = fChar;
                definition->fLineCount = fLineCount;
                definition->fClone = fCloned;
                if (MarkType::kConst == markType) {
                    // todo: require that fChar points to def on same line as markup
                    // additionally add definition to class children if it is not already there
                    if (definition->fParent != fRoot) {
//                        fRoot->fChildren.push_back(definition);
                    }
                }
                definition->fName = name;
                if (MarkType::kMethod == markType) {
                    if (string::npos != name.find(':', 0)) {
                        definition->setCanonicalFiddle();
                    } else {
                        definition->fFiddle = name;
                    }
                } else {
                    definition->fFiddle = normalized_name(name);
                }
                definition->fMarkType = markType;
                definition->fAnonymous = fAnonymous;
                this->setAsParent(definition);
            }
            } break;
        case MarkType::kTopic:
        case MarkType::kSubtopic:
            SkASSERT(1 == typeNameBuilder.size());
            if (!hasEnd) {
                if (!typeNameBuilder.size()) {
                    return this->reportError<bool>("unnamed topic");
                }
                fTopics.emplace_front(markType, defStart, fLineCount, fParent);
                RootDefinition* rootDefinition = &fTopics.front();
                definition = rootDefinition;
                definition->fFileName = fFileName;
                definition->fContentStart = fChar;
                definition->fName = typeNameBuilder[0];
                Definition* parent = fParent;
                while (parent && MarkType::kTopic != parent->fMarkType 
                        && MarkType::kSubtopic != parent->fMarkType) {
                    parent = parent->fParent;
                }
                definition->fFiddle = parent ? parent->fFiddle + '_' : "";
                definition->fFiddle += normalized_name(typeNameBuilder[0]);
                this->setAsParent(definition);
            }
            {
                const string& fullTopic = hasEnd ? fParent->fFiddle : definition->fFiddle;
                Definition* defPtr = fTopicMap[fullTopic];
                if (hasEnd) {
                    if (!definition) {
                        definition = defPtr;
                    } else if (definition != defPtr) {
                        return this->reportError<bool>("mismatched topic");
                    }
                } else {
                    if (nullptr != defPtr) {
                        return this->reportError<bool>("already declared topic");
                    }
                    fTopicMap[fullTopic] = definition;
                }
            }
            if (hasEnd) {
                if (!this->popParentStack(definition)) {
                    return false;
                }
            }
            break;
        // these types are children of parents, but are not in named maps
        case MarkType::kDefinedBy: {
            string prefixed(fRoot->fName);
            const char* start = fChar;
            string name(start, this->trimmedBracketEnd(fMC) - start);
            prefixed += "::" + name;
            this->skipToEndBracket(fMC);
            const auto leafIter = fRoot->fLeaves.find(prefixed);
            if (fRoot->fLeaves.end() != leafIter) {
                this->reportError<bool>("DefinedBy already defined");
            }
            definition = &fRoot->fLeaves[prefixed];
            definition->fParent = fParent;
            definition->fStart = defStart;
            definition->fContentStart = start;
            definition->fName = name;
            definition->fFiddle = normalized_name(name);
            definition->fContentEnd = fChar;
            this->skipToEndBracket('\n');
            definition->fTerminator = fChar;
            definition->fMarkType = markType;
            definition->fLineCount = fLineCount;
            fParent->fChildren.push_back(definition);
            } break;
        case MarkType::kDescription:
        case MarkType::kStdOut:
        // may be one-liner
        case MarkType::kBug:
        case MarkType::kNoExample:
        case MarkType::kParam:
        case MarkType::kReturn:
        case MarkType::kToDo:
            if (hasEnd) {
                if (markType == fParent->fMarkType) {
                    definition = fParent;
                    if (MarkType::kBug == markType || MarkType::kReturn == markType
                            || MarkType::kToDo == markType) {
                        this->skipNoName();
                    }
                    if (!this->popParentStack(fParent)) { // if not one liner, pop
                        return false;
                    }
                    if (MarkType::kParam == markType || MarkType::kReturn == markType) {
                        if (!this->checkParamReturn(definition)) {
                            return false;
                        }
                    }
                } else {
                    fMarkup.emplace_front(markType, defStart, fLineCount, fParent);
                    definition = &fMarkup.front();
                    definition->fName = typeNameBuilder[0];
                    definition->fFiddle = fParent->fFiddle;
                    definition->fContentStart = fChar;
                    definition->fContentEnd = this->trimmedBracketEnd(fMC);
                    this->skipToEndBracket(fMC);
                    SkAssertResult(fMC == this->next());
                    SkAssertResult(fMC == this->next());
                    definition->fTerminator = fChar;
                    fParent->fChildren.push_back(definition);
                }
                break;
            }
        // not one-liners
        case MarkType::kCode:
        case MarkType::kDeprecated:
        case MarkType::kExample:
        case MarkType::kExperimental:
        case MarkType::kFormula:
        case MarkType::kFunction:
        case MarkType::kLegend:
        case MarkType::kList:
        case MarkType::kPrivate:
        case MarkType::kTable:
        case MarkType::kTrack:
            if (hasEnd) {
                definition = fParent;
                if (markType != fParent->fMarkType) {
                    return this->reportError<bool>("end element mismatch");
                } else if (!this->popParentStack(fParent)) {
                    return false;
                }
                if (MarkType::kExample == markType) {
                    if (definition->fChildren.size() == 0) {
                        TextParser emptyCheck(definition);
                        if (emptyCheck.eof() || !emptyCheck.skipWhiteSpace()) {
                            return this->reportError<bool>("missing example body");
                        }
                    }
                }
            } else {
                fMarkup.emplace_front(markType, defStart, fLineCount, fParent);
                definition = &fMarkup.front();
                definition->fContentStart = fChar;
                definition->fName = typeNameBuilder[0];
                definition->fFiddle = fParent->fFiddle;
                char suffix = '\0';
                bool tryAgain;
                do {
                    tryAgain = false;
                    for (const auto& child : fParent->fChildren) {
                        if (child->fFiddle == definition->fFiddle) {
                            if (MarkType::kExample != child->fMarkType) {
                                continue;
                            }
                            if ('\0' == suffix) {
                                suffix = 'a';
                            } else if (++suffix > 'z') {
                                return reportError<bool>("too many examples");
                            }
                            definition->fFiddle = fParent->fFiddle + '_';
                            definition->fFiddle += suffix;
                            tryAgain = true;
                            break;
                        }
                    }
                } while (tryAgain);
                this->setAsParent(definition);
            }
            break;
            // always treated as one-liners (can't detect misuse easily)
        case MarkType::kAlias:
        case MarkType::kAnchor: 
        case MarkType::kDefine:
        case MarkType::kError:
        case MarkType::kFile:
        case MarkType::kHeight:
        case MarkType::kImage:
        case MarkType::kPlatform:
        case MarkType::kSeeAlso:
        case MarkType::kSubstitute:
        case MarkType::kTime:
        case MarkType::kVolatile:
        case MarkType::kWidth:
            if (hasEnd) {
                return this->reportError<bool>("one liners omit end element");
            }
            fMarkup.emplace_front(markType, defStart, fLineCount, fParent);
            definition = &fMarkup.front();
            definition->fName = typeNameBuilder[0];
            definition->fFiddle = normalized_name(typeNameBuilder[0]);
            definition->fContentStart = fChar;
            definition->fContentEnd = this->trimmedBracketEnd('\n');
            definition->fTerminator = this->lineEnd() - 1;
            fParent->fChildren.push_back(definition);
            if (MarkType::kAnchor == markType) {
                this->skipToEndBracket(fMC);
                fMarkup.emplace_front(MarkType::kLink, fChar, fLineCount, definition);
                SkAssertResult(fMC == this->next());
                this->skipWhiteSpace();
                Definition* link = &fMarkup.front();
                link->fContentStart = fChar;
                link->fContentEnd = this->trimmedBracketEnd(fMC);
                this->skipToEndBracket(fMC);
                SkAssertResult(fMC == this->next());
                SkAssertResult(fMC == this->next());
                link->fTerminator = fChar;
                definition->fContentEnd = link->fContentEnd;
                definition->fTerminator = fChar;
                definition->fChildren.emplace_back(link);
            } else if (MarkType::kAlias == markType) {
                this->skipWhiteSpace();
                const char* start = fChar;
                this->skipToNonAlphaNum();
                string alias(start, fChar - start);
                if (fAliasMap.end() != fAliasMap.find(alias)) {
                    return this->reportError<bool>("duplicate alias");
                }
                fAliasMap[alias] = definition;
            } 
            break;
        case MarkType::kExternal:
            (void) this->collectExternals();  // FIXME: detect errors in external defs?
            break;
        default:
            SkASSERT(0);  // fixme : don't let any types be invisible
            return true;
    }
    if (fParent) {
        SkASSERT(definition);
        SkASSERT(definition->fName.length() > 0);
    }
    return true;
}

void BmhParser::reportDuplicates(const Definition& def, const string& dup) const {
    if (MarkType::kExample == def.fMarkType && dup == def.fFiddle) {
        TextParser reporter(&def);
        reporter.reportError("duplicate example name");
    }
    for (auto& child : def.fChildren ) {
        reportDuplicates(*child, dup);
    }
}

static void find_examples(const Definition& def, vector<string>* exampleNames) {
    if (MarkType::kExample == def.fMarkType) {
        exampleNames->push_back(def.fFiddle);
    }
    for (auto& child : def.fChildren ) {
        find_examples(*child, exampleNames);
    }
}

bool BmhParser::checkExamples() const {
    vector<string> exampleNames;
    for (const auto& topic : fTopicMap) {
        if (topic.second->fParent) {
            continue;
        }
        find_examples(*topic.second, &exampleNames);
    }
    std::sort(exampleNames.begin(), exampleNames.end());
    string* last = nullptr;
    string reported;
    bool checkOK = true;
    for (auto& nameIter : exampleNames) {
        if (last && *last == nameIter && reported != *last) {
            reported = *last;
            SkDebugf("%s\n", reported.c_str());
            for (const auto& topic : fTopicMap) {
                if (topic.second->fParent) {
                    continue;
                }
                this->reportDuplicates(*topic.second, reported);
            }
            checkOK = false;
        }
        last = &nameIter;
    }
    return checkOK;
}

bool BmhParser::checkParamReturn(const Definition* definition) const {
    const char* parmEndCheck = definition->fContentEnd;
    while (parmEndCheck < definition->fTerminator) {
        if (fMC == parmEndCheck[0]) {
            break;
        }
        if (' ' < parmEndCheck[0]) {
            this->reportError<bool>(
                    "use full end marker on multiline #Param and #Return");
        }
        ++parmEndCheck;
    }
    return true;
}

bool BmhParser::childOf(MarkType markType) const {
    auto childError = [this](MarkType markType) -> bool {
        string errStr = "expected ";
        errStr += fMaps[(int) markType].fName;
        errStr += " parent";
        return this->reportError<bool>(errStr.c_str());
    };

    if (markType == fParent->fMarkType) {
        return true;
    }
    if (this->hasEndToken()) {
        if (!fParent->fParent) {
            return this->reportError<bool>("expected grandparent");
        }
        if (markType == fParent->fParent->fMarkType) {
            return true;
        }
    }
    return childError(markType);
}

string BmhParser::className(MarkType markType) {
    const char* end = this->lineEnd();
    const char* mc = this->strnchr(fMC, end);
    string classID;
    TextParser::Save savePlace(this);
    this->skipSpace();
    const char* wordStart = fChar;
    this->skipToNonAlphaNum();
    const char* wordEnd = fChar;
    classID = string(wordStart, wordEnd - wordStart);
    if (!mc) {
        savePlace.restore();
    }
    string builder;
    const Definition* parent = this->parentSpace();
    if (parent && parent->fName != classID) {
        builder += parent->fName;
    }
    if (mc) {
        if (mc + 1 < fEnd && fMC == mc[1]) {  // if ##
            if (markType != fParent->fMarkType) {
                return this->reportError<string>("unbalanced method");
            }
            if (builder.length() > 0 && classID.size() > 0) {
                if (builder != fParent->fName) {
                    builder += "::";
                    builder += classID;
                    if (builder != fParent->fName) {
                        return this->reportError<string>("name mismatch");
                    }
                }
            }
            this->skipLine();
            return fParent->fName;
        }
        fChar = mc;
        this->next();
    }
    this->skipWhiteSpace();
    if (MarkType::kEnum == markType && fChar >= end) {
        fAnonymous = true;
        builder += "::_anonymous";
        return uniqueRootName(builder, markType);
    }
    builder = this->word(builder, "::");
    return builder;
}

bool BmhParser::collectExternals() {
    do {
        this->skipWhiteSpace();
        if (this->eof()) {
            break;
        }
        if (fMC == this->peek()) {
            this->next();
            if (this->eof()) {
                break;
            }
            if (fMC == this->peek()) {
                this->skipLine();
                break;
            }
            if (' ' >= this->peek()) {
                this->skipLine();
                continue;
            }
            if (this->startsWith(fMaps[(int) MarkType::kExternal].fName)) {
                this->skipToNonAlphaNum();
                continue;
            }
        }
        this->skipToAlpha();
        const char* wordStart = fChar;
        this->skipToNonAlphaNum();
        if (fChar - wordStart > 0) {
            fExternals.emplace_front(MarkType::kExternal, wordStart, fChar, fLineCount, fParent);
            RootDefinition* definition = &fExternals.front();
            definition->fFileName = fFileName;
            definition->fName = string(wordStart ,fChar - wordStart);
            definition->fFiddle = normalized_name(definition->fName);
        }
    } while (!this->eof());
    return true;
}

static bool dump_examples(FILE* fiddleOut, const Definition& def, bool* continuation) {
    if (MarkType::kExample == def.fMarkType) {
        string result;
        if (!def.exampleToScript(&result)) {
            return false;
        }
        if (result.length() > 0) {
            if (*continuation) {
                fprintf(fiddleOut, ",\n");
            } else {
                *continuation = true;
            }
            fprintf(fiddleOut, "%s", result.c_str());
        }
        return true;
    }
    for (auto& child : def.fChildren ) {
        if (!dump_examples(fiddleOut, *child, continuation)) {
            return false;
        }
    }
    return true;
}

bool BmhParser::dumpExamples(const char* fiddleJsonFileName) const {
    FILE* fiddleOut = fopen(fiddleJsonFileName, "wb");
    if (!fiddleOut) {
        SkDebugf("could not open output file %s\n", fiddleJsonFileName);
        return false;
    }
    fprintf(fiddleOut, "{\n");
    bool continuation = false;
    for (const auto& topic : fTopicMap) {
        if (topic.second->fParent) {
            continue;
        }
        dump_examples(fiddleOut, *topic.second, &continuation);
    }
    fprintf(fiddleOut, "\n}\n");
    fclose(fiddleOut);
    SkDebugf("wrote %s\n", fiddleJsonFileName);
    return true;
}

int BmhParser::endHashCount() const {
    const char* end = fLine + this->lineLength();
    int count = 0;
    while (fLine < end && fMC == *--end) {
        count++;
    }
    return count;
}

bool BmhParser::endTableColumn(const char* end, const char* terminator) {
    if (!this->popParentStack(fParent)) {
        return false;
    }
    fWorkingColumn->fContentEnd = end;
    fWorkingColumn->fTerminator = terminator;
    fColStart = fChar - 1;
    this->skipSpace();
    fTableState = TableState::kColumnStart;
    return true;
}

// FIXME: some examples may produce different output on different platforms 
// if the text output can be different, think of how to author that

bool BmhParser::findDefinitions() {
    bool lineStart = true;
    const char* lastChar = nullptr;
    const char* lastMC = nullptr;
    fParent = nullptr;
    while (!this->eof()) {
        if (this->peek() == fMC) {
            lastMC = fChar;
            this->next();
            if (this->peek() == fMC) {
                this->next();
                if (!lineStart && ' ' < this->peek()) {
                    return this->reportError<bool>("expected definition");
                }
                if (this->peek() != fMC) {
                    if (MarkType::kColumn == fParent->fMarkType) {
                        SkASSERT(TableState::kColumnEnd == fTableState);
                        if (!this->endTableColumn(lastChar, lastMC)) {
                            return false;
                        }
                        SkASSERT(fRow);
                        if (!this->popParentStack(fParent)) {
                            return false;
                        }
                        fRow->fContentEnd = fWorkingColumn->fContentEnd;
                        fWorkingColumn = nullptr;
                        fRow = nullptr;
                        fTableState = TableState::kNone;
                    } else {
                        vector<string> parentName;
                        parentName.push_back(fParent->fName);
                        if (!this->addDefinition(fChar - 1, true, fParent->fMarkType, parentName)) {
                            return false;
                        }
                    }
                } else {
                    SkAssertResult(this->next() == fMC);
                    fMC = this->next();  // change markup character
                    if (' ' >= fMC) {
                        return this->reportError<bool>("illegal markup character");
                    }
                    fMarkup.emplace_front(MarkType::kMarkChar, fChar - 1, fLineCount, fParent);
                    Definition* markChar = &fMarkup.front();
                    markChar->fContentStart = fChar - 1;
                    this->skipToEndBracket('\n');
                    markChar->fContentEnd = fChar;
                    markChar->fTerminator = fChar;
                    fParent->fChildren.push_back(markChar);
                }
            } else if (this->peek() >= 'A' && this->peek() <= 'Z') {
                const char* defStart = fChar - 1;
                MarkType markType = this->getMarkType(MarkLookup::kRequire);
                bool hasEnd = this->hasEndToken();
                if (!hasEnd) {
                    MarkType parentType = fParent ? fParent->fMarkType : MarkType::kRoot;
                    uint64_t parentMask = fMaps[(int) markType].fParentMask;
                    if (parentMask && !(parentMask & (1LL << (int) parentType))) {
                        return this->reportError<bool>("invalid parent");
                    }
                }
                if (!this->skipName(fMaps[(int) markType].fName)) {
                    return this->reportError<bool>("illegal markup character");
                }
                if (!this->skipSpace()) {
                    return this->reportError<bool>("unexpected end");
                }
                bool expectEnd = true;
                vector<string> typeNameBuilder = this->typeName(markType, &expectEnd);
                if (fCloned && MarkType::kMethod != markType && MarkType::kExample != markType
                        && !fAnonymous) {
                    return this->reportError<bool>("duplicate name");
                }
                if (hasEnd && expectEnd) {
                    SkASSERT(fMC != this->peek());
                }
                if (!this->addDefinition(defStart, hasEnd, markType, typeNameBuilder)) {
                    return false;
                }
                continue;
            } else if (this->peek() == ' ') {
                if (!fParent || (MarkType::kTable != fParent->fMarkType
                        && MarkType::kLegend != fParent->fMarkType
                        && MarkType::kList != fParent->fMarkType)) {
                    int endHashes = this->endHashCount();
                    if (endHashes <= 1) {
                        if (fParent) {
                            if (TableState::kColumnEnd == fTableState) {
                                if (!this->endTableColumn(lastChar, lastMC)) {
                                    return false;
                                }
                            } else {  // one line comment
                                fMarkup.emplace_front(MarkType::kComment, fChar - 1, fLineCount,
                                        fParent);
                                Definition* comment = &fMarkup.front();
                                comment->fContentStart = fChar - 1;
                                this->skipToEndBracket('\n');
                                comment->fContentEnd = fChar;
                                comment->fTerminator = fChar;
                                fParent->fChildren.push_back(comment);
                            }
                        } else {
                            fChar = fLine + this->lineLength() - 1;
                        }
                    } else {  // table row
                        if (2 != endHashes) {
                            string errorStr = "expect ";
                            errorStr += fMC;
                            errorStr += fMC;
                            return this->reportError<bool>(errorStr.c_str());
                        }
                        if (!fParent || MarkType::kTable != fParent->fMarkType) {
                            return this->reportError<bool>("missing table");
                        }
                    }
                } else if (TableState::kNone == fTableState) {
                    // fixme? no nested tables for now
                    fColStart = fChar - 1;
                    fMarkup.emplace_front(MarkType::kRow, fColStart, fLineCount, fParent);
                    fRow = &fMarkup.front();
                    fRow->fName = fParent->fName;
                    this->skipWhiteSpace();
                    fRow->fContentStart = fChar;
                    this->setAsParent(fRow);
                    fTableState = TableState::kColumnStart;
                }
                if (TableState::kColumnStart == fTableState) {
                    fMarkup.emplace_front(MarkType::kColumn, fColStart, fLineCount, fParent);
                    fWorkingColumn = &fMarkup.front();
                    fWorkingColumn->fName = fParent->fName;
                    fWorkingColumn->fContentStart = fChar;
                    this->setAsParent(fWorkingColumn);
                    fTableState = TableState::kColumnEnd;
                    continue;
                }
            }
        }
        char nextChar = this->next();
        lineStart = nextChar == '\n';
        if (' ' < nextChar) {
            lastChar = fChar;
        }
    }
    if (fParent) {
        return this->reportError<bool>("mismatched end");
    }
    return true;
}

MarkType BmhParser::getMarkType(MarkLookup lookup) const {
    for (int index = 0; index <= Last_MarkType; ++index) {
        int typeLen = strlen(fMaps[index].fName);
        if (typeLen == 0) {
            continue;
        }
        if (fChar + typeLen >= fEnd || fChar[typeLen] > ' ') {
            continue;
        }
        int chCompare = strncmp(fChar, fMaps[index].fName, typeLen);
        if (chCompare < 0) {
            goto fail;
        }
        if (chCompare == 0) {
            return (MarkType) index;
        }
    }
fail:
    if (MarkLookup::kRequire == lookup) {
        return this->reportError<MarkType>("unknown mark type");
    }
    return MarkType::kNone;
}

bool HackParser::hackFiles() {
    string filename(fFileName);
    size_t len = filename.length() - 1;
    while (len > 0 && (isalnum(filename[len]) || '_' == filename[len] || '.' == filename[len])) {
        --len;
    }
    filename = filename.substr(len + 1);
    // remove trailing period from #Param and #Return
    FILE* out = fopen(filename.c_str(), "wb");
    if (!out) {
        SkDebugf("could not open output file %s\n", filename.c_str());
        return false;
    }
    const char* start = fStart;
    do {
        const char* match = this->strnchr('#', fEnd);
        if (!match) {
            break;
        }
        this->skipTo(match);
        this->next();
        if (!this->startsWith("Param") && !this->startsWith("Return")) {
            continue;
        }
        const char* end = this->strnstr("##", fEnd);
        while (true) {
            TextParser::Save lastPeriod(this);
            this->next();
            if (!this->skipToEndBracket('.', end)) {
                lastPeriod.restore();
                break;
            }
        }
        if ('.' == this->peek()) {
            fprintf(out, "%.*s", (int) (fChar - start), start);
            this->next();
            start = fChar;
        }
    } while (!this->eof());
    fprintf(out, "%.*s", (int) (fEnd - start), start);
    fclose(out);
    SkDebugf("wrote %s\n", filename.c_str());
    return true;
}

bool BmhParser::hasEndToken() const {
    const char* last = fLine + this->lineLength();
    while (last > fLine && ' ' >= *--last)
        ;
    if (--last < fLine) {
        return false;
    }
    return last[0] == fMC && last[1] == fMC;
}

string BmhParser::memberName() {
    const char* wordStart;
    const char* prefixes[] = { "static", "const" };
    do {
        this->skipSpace();
        wordStart = fChar;
        this->skipToNonAlphaNum();
    } while (this->anyOf(wordStart, prefixes, SK_ARRAY_COUNT(prefixes)));
    if ('*' == this->peek()) {
        this->next();
    }
    return this->className(MarkType::kMember);
}

string BmhParser::methodName() {
    if (this->hasEndToken()) {
        if (!fParent || !fParent->fName.length()) {
            return this->reportError<string>("missing parent method name");
        }
        SkASSERT(fMC == this->peek());
        this->next();
        SkASSERT(fMC == this->peek());
        this->next();
        SkASSERT(fMC != this->peek());
        return fParent->fName;
    }
    string builder;
    const char* end = this->lineEnd();
    const char* paren = this->strnchr('(', end);
    if (!paren) {
        return this->reportError<string>("missing method name and reference");
    }
    const char* nameStart = paren;
    char ch;
    bool expectOperator = false;
    bool isConstructor = false;
    const char* nameEnd = nullptr;
    while (nameStart > fChar && ' ' != (ch = *--nameStart)) {
        if (!isalnum(ch) && '_' != ch) {
            if (nameEnd) {
                break;
            }
            expectOperator = true;
            continue;
        }
        if (!nameEnd) {
            nameEnd = nameStart + 1;
        }
    }
    if (!nameEnd) {
         return this->reportError<string>("unexpected method name char");
    }
    if (' ' == nameStart[0]) {
        ++nameStart;
    }
    if (nameEnd <= nameStart) {
        return this->reportError<string>("missing method name");
    }
    if (nameStart >= paren) {
        return this->reportError<string>("missing method name length");
    }
    string name(nameStart, nameEnd - nameStart);
    bool allLower = true;
    for (int index = 0; index < (int) (nameEnd - nameStart); ++index) {
        if (!islower(nameStart[index])) {
            allLower = false;
            break;
        }
    }
    if (expectOperator && "operator" != name) {
         return this->reportError<string>("expected operator");
    }
    const Definition* parent = this->parentSpace();
    if (parent && parent->fName.length() > 0) {
        if (parent->fName == name) {
            isConstructor = true;
        } else if ('~' == name[0]) {
            if (parent->fName != name.substr(1)) {
                 return this->reportError<string>("expected destructor");
            }
            isConstructor = true;
        }
        builder = parent->fName + "::";
    } 
    if (isConstructor || expectOperator) {
        paren = this->strnchr(')', end) + 1;
    }
    builder.append(nameStart, paren - nameStart);
    if (!expectOperator && allLower) {
        builder.append("()");
    }
    int parens = 0;
    while (fChar < end || parens > 0) {
        if ('(' == this->peek()) {
            ++parens;
        } else if (')' == this->peek()) {
            --parens;
        }
        this->next();
    }
    TextParser::Save saveState(this);
    this->skipWhiteSpace();
    if (this->startsWith("const")) {
        this->skipName("const");
    } else {
        saveState.restore();
    }
//    this->next();
    return uniqueRootName(builder, MarkType::kMethod);
}

const Definition* BmhParser::parentSpace() const {
    Definition* parent = nullptr;
    Definition* test = fParent;
    while (test) {
        if (MarkType::kClass == test->fMarkType ||
                MarkType::kEnumClass == test->fMarkType ||
                MarkType::kStruct == test->fMarkType) {
            parent = test;
            break;
        }
        test = test->fParent;
    }
    return parent;
}

bool BmhParser::popParentStack(Definition* definition) {
    if (!fParent) {
        return this->reportError<bool>("missing parent");
    }
    if (definition != fParent) {
        return this->reportError<bool>("definition end is not parent");
    }
    if (!definition->fStart) {
        return this->reportError<bool>("definition missing start");
    }
    if (definition->fContentEnd) {
        return this->reportError<bool>("definition already ended");
    }
    definition->fContentEnd = fLine - 1;
    definition->fTerminator = fChar;
    fParent = definition->fParent;
    if (!fParent || (MarkType::kTopic == fParent->fMarkType && !fParent->fParent)) {
        fRoot = nullptr;
    }
    return true;
}

TextParser::TextParser(const Definition* definition) :
    TextParser(definition->fFileName, definition->fContentStart, definition->fContentEnd, 
        definition->fLineCount) {
}

void TextParser::reportError(const char* errorStr) const {
    this->reportWarning(errorStr);
    SkDebugf("");  // convenient place to set a breakpoint
}

void TextParser::reportWarning(const char* errorStr) const {
    TextParser err(fFileName, fLine, fEnd, fLineCount);
    size_t lineLen = this->lineLength();
    ptrdiff_t spaces = fChar - fLine;
    while (spaces > 0 && (size_t) spaces > lineLen) {
        ++err.fLineCount;
        err.fLine += lineLen;
        spaces -= lineLen;
        lineLen = err.lineLength();
    }
    SkDebugf("\n%s(%zd): error: %s\n", fFileName.c_str(), err.fLineCount, errorStr);
    if (0 == lineLen) {
        SkDebugf("[blank line]\n");
    } else {
        while (lineLen > 0 && '\n' == err.fLine[lineLen - 1]) {
            --lineLen;
        }
        SkDebugf("%.*s\n", (int) lineLen, err.fLine);
        SkDebugf("%*s^\n", (int) spaces, "");
    }
}

bool BmhParser::skipNoName() {
    if ('\n' == this->peek()) {
        this->next();
        return true;
    }
    this->skipWhiteSpace();
    if (fMC != this->peek()) {
        return this->reportError<bool>("expected end mark");
    }
    this->next();
    if (fMC != this->peek()) {
        return this->reportError<bool>("expected end mark");
    }
    this->next();
    return true;
}

bool BmhParser::skipToDefinitionEnd(MarkType markType) {
    if (this->eof()) {
        return this->reportError<bool>("missing end");
    }
    const char* start = fLine;
    int startLineCount = fLineCount;
    int stack = 1;
    ptrdiff_t lineLen;
    bool foundEnd = false;
    do {
        lineLen = this->lineLength();
        if (fMC != *fChar++) {
            continue;
        }
        if (fMC == *fChar) {
            continue;
        }
        if (' ' == *fChar) {
            continue;
        }
        MarkType nextType = this->getMarkType(MarkLookup::kAllowUnknown);
        if (markType != nextType) {
            continue;
        }
        bool hasEnd = this->hasEndToken();
        if (hasEnd) {
            if (!--stack) {
                foundEnd = true;
                continue;
            }
        } else {
            ++stack;
        }
    } while ((void) ++fLineCount, (void) (fLine += lineLen), (void) (fChar = fLine),
            !this->eof() && !foundEnd);
    if (foundEnd) {
        return true;
    }
    fLineCount = startLineCount;
    fLine = start;
    fChar = start;
    return this->reportError<bool>("unbalanced stack");
}

vector<string> BmhParser::topicName() {
    vector<string> result;
    this->skipWhiteSpace();
    const char* lineEnd = fLine + this->lineLength();
    const char* nameStart = fChar;
    while (fChar < lineEnd) {
        char ch = this->next();
        SkASSERT(',' != ch);
        if ('\n' == ch) {
            break;
        }
        if (fMC == ch) {
            break;
        }
    }
    if (fChar - 1 > nameStart) {
        string builder(nameStart, fChar - nameStart - 1);
        trim_start_end(builder);
        result.push_back(builder);
    }
    if (fChar < lineEnd && fMC == this->peek()) {
        this->next();
    }
    return result;
}

// typeName parsing rules depend on mark type
vector<string> BmhParser::typeName(MarkType markType, bool* checkEnd) {
    fAnonymous = false;
    fCloned = false;
    vector<string> result;
    string builder;
    if (fParent) {
        builder = fParent->fName;
    }
    switch (markType) {
        case MarkType::kEnum:
            // enums may be nameless
        case MarkType::kConst:
        case MarkType::kEnumClass:
        case MarkType::kClass:
        case MarkType::kStruct:
        case MarkType::kTypedef:
            // expect name
            builder = this->className(markType);
            break;
        case MarkType::kExample:
            // check to see if one already exists -- if so, number this one
            builder = this->uniqueName(string(), markType);
            this->skipNoName();
            break;
        case MarkType::kCode:
        case MarkType::kDeprecated:
        case MarkType::kDescription:
        case MarkType::kDoxygen:
        case MarkType::kExperimental:
        case MarkType::kExternal:
        case MarkType::kFormula:
        case MarkType::kFunction:
        case MarkType::kLegend:
        case MarkType::kList:
        case MarkType::kNoExample:
        case MarkType::kPrivate:
        case MarkType::kTrack:
            this->skipNoName();
            break;
        case MarkType::kAlias:
        case MarkType::kAnchor: 
        case MarkType::kBug:  // fixme: expect number
        case MarkType::kDefine:
        case MarkType::kDefinedBy:
        case MarkType::kError:
        case MarkType::kFile:
        case MarkType::kHeight:
        case MarkType::kImage:
        case MarkType::kPlatform:
        case MarkType::kReturn:
        case MarkType::kSeeAlso:
        case MarkType::kSubstitute:
        case MarkType::kTime:
        case MarkType::kToDo:
        case MarkType::kVolatile:
        case MarkType::kWidth:
            *checkEnd = false;  // no name, may have text body
            break;
        case MarkType::kStdOut:
            this->skipNoName();
            break;  // unnamed
        case MarkType::kMember:
            builder = this->memberName();
            break;
        case MarkType::kMethod:
            builder = this->methodName();
            break;
        case MarkType::kParam:
           // fixme: expect camelCase
            builder = this->word("", "");
            this->skipSpace();
            *checkEnd = false;
            break;
        case MarkType::kTable:
            this->skipNoName();
            break;  // unnamed
        case MarkType::kSubtopic:
        case MarkType::kTopic:
            // fixme: start with cap, allow space, hyphen, stop on comma
            // one topic can have multiple type names delineated by comma
            result = this->topicName();
            if (result.size() == 0 && this->hasEndToken()) {
                break;
            }
            return result;
        default:
            // fixme: don't allow silent failures
            SkASSERT(0);
    }
    result.push_back(builder);
    return result;
}

string BmhParser::uniqueName(const string& base, MarkType markType) {
    string builder(base);
    if (!builder.length()) {
        builder = fParent->fName;
    }
    if (!fParent) {
        return builder;
    }
    int number = 2;
    string numBuilder(builder);
    do {
        for (const auto& iter : fParent->fChildren) {
            if (markType == iter->fMarkType) {
                if (iter->fName == numBuilder) {
                    fCloned = true;
                    numBuilder = builder + '_' + to_string(number);
                    goto tryNext;
                }
            }
        }
        break;
tryNext: ;
    } while (++number);
    return numBuilder;
}

string BmhParser::uniqueRootName(const string& base, MarkType markType) {
    auto checkName = [markType](const Definition& def, const string& numBuilder) -> bool {
        return markType == def.fMarkType && def.fName == numBuilder;
    };

    string builder(base);
    if (!builder.length()) {
        builder = fParent->fName;
    }
    int number = 2;
    string numBuilder(builder);
    Definition* cloned = nullptr;
    do {
        if (fRoot) {
            for (auto& iter : fRoot->fBranches) {
                if (checkName(*iter.second, numBuilder)) {
                    cloned = iter.second;
                    goto tryNext;
                }
            }
            for (auto& iter : fRoot->fLeaves) {
                if (checkName(iter.second, numBuilder)) {
                    cloned = &iter.second;
                    goto tryNext;
                }
            }
        } else if (fParent) {
            for (auto& iter : fParent->fChildren) {
                if (checkName(*iter, numBuilder)) {
                    cloned = &*iter;
                    goto tryNext;
                }
            }
        }
        break;
tryNext: ;
        if ("()" == builder.substr(builder.length() - 2)) {
            builder = builder.substr(0, builder.length() - 2);
        }
        if (MarkType::kMethod == markType) {
            cloned->fCloned = true;
        }
        fCloned = true;
        numBuilder = builder + '_' + to_string(number);
    } while (++number);
    return numBuilder;
}

void BmhParser::validate() const {
    for (int index = 0; index <= (int) Last_MarkType; ++index) {
        SkASSERT(fMaps[index].fMarkType == (MarkType) index);
    }
    const char* last = "";
    for (int index = 0; index <= (int) Last_MarkType; ++index) {
        const char* next = fMaps[index].fName;
        if (!last[0]) {
            last = next;
            continue;
        }
        if (!next[0]) {
            continue;
        }
        SkASSERT(strcmp(last, next) < 0);
        last = next;
    }
}

string BmhParser::word(const string& prefix, const string& delimiter) {
    string builder(prefix);
    this->skipWhiteSpace();
    const char* lineEnd = fLine + this->lineLength();
    const char* nameStart = fChar;
    while (fChar < lineEnd) {
        char ch = this->next();
        if (' ' >= ch) {
            break;
        }
        if (',' == ch) {
            return this->reportError<string>("no comma needed");
            break;
        }
        if (fMC == ch) {
            return builder;
        }
        if (!isalnum(ch) && '_' != ch && ':' != ch && '-' != ch) {
            return this->reportError<string>("unexpected char");
        }
        if (':' == ch) {
            // expect pair, and expect word to start with Sk
            if (nameStart[0] != 'S' || nameStart[1] != 'k') {
                return this->reportError<string>("expected Sk");
            }
            if (':' != this->peek()) {
                return this->reportError<string>("expected ::");
            }
            this->next();
        } else if ('-' == ch) {
            // expect word not to start with Sk or kX where X is A-Z
            if (nameStart[0] == 'k' && nameStart[1] >= 'A' && nameStart[1] <= 'Z') {
                return this->reportError<string>("didn't expected kX");
            }
            if (nameStart[0] == 'S' && nameStart[1] == 'k') {
                return this->reportError<string>("expected Sk");
            }
        }
    }
    if (prefix.size()) {
        builder += delimiter;
    }
    builder.append(nameStart, fChar - nameStart - 1);
    return builder;
}

// pass one: parse text, collect definitions
// pass two: lookup references

DEFINE_string2(bmh, b, "", "Path to a *.bmh file or a directory.");
DEFINE_string2(examples, e, "", "File of fiddlecli input, usually fiddle.json (For now, disables -r -f -s)");
DEFINE_string2(fiddle, f, "", "File of fiddlecli output, usually fiddleout.json.");
DEFINE_string2(include, i, "", "Path to a *.h file or a directory.");
DEFINE_bool2(hack, k, false, "Do a find/replace hack to update all *.bmh files. (Requires -b)");
DEFINE_bool2(stdout, o, false, "Write file out to standard out.");
DEFINE_bool2(populate, p, false, "Populate include from bmh. (Requires -b -i)");
DEFINE_string2(ref, r, "", "Resolve refs and write bmh_*.md files to path. (Requires -b)");
DEFINE_string2(spellcheck, s, "", "Spell-check [once, all, mispelling]. (Requires -b)");
DEFINE_string2(tokens, t, "", "Directory to write bmh from include. (Requires -i)");
DEFINE_bool2(crosscheck, x, false, "Check bmh against includes. (Requires -b -i)");

static int count_children(const Definition& def, MarkType markType) {
    int count = 0;
    if (markType == def.fMarkType) {
        ++count;
    }
    for (auto& child : def.fChildren ) {
        count += count_children(*child, markType);
    }
    return count;
}

int main(int argc, char** const argv) {
    BmhParser bmhParser;
    bmhParser.validate();

    SkCommandLineFlags::SetUsage(
        "Common Usage: bookmaker -i path/to/include.h -t\n"
        "              bookmaker -b path/to/bmh_files -e fiddle.json\n"
        "              ~/go/bin/fiddlecli --input fiddle.json --output fiddleout.json\n"
        "              bookmaker -b path/to/bmh_files -f fiddleout.json -r path/to/md_files\n"
        "              bookmaker -b path/to/bmh_files -i path/to/include.h -x\n"
        "              bookmaker -b path/to/bmh_files -i path/to/include.h -p\n");
    bool help = false;
    for (int i = 1; i < argc; i++) {
        if (0 == strcmp("-h", argv[i]) || 0 == strcmp("--help", argv[i])) {
            help = true;
            for (int j = i + 1; j < argc; j++) {
                if (SkStrStartsWith(argv[j], '-')) {
                    break;
                }
                help = false;
            }
            break;
        }
    }
    if (!help) {
        SkCommandLineFlags::Parse(argc, argv);
    } else {
        SkCommandLineFlags::PrintUsage();
        const char* const commands[] = { "", "-h", "bmh", "-h", "examples", "-h", "include", "-h", "fiddle",
            "-h", "ref", "-h", "tokens",
            "-h", "crosscheck", "-h", "populate", "-h", "spellcheck" };
        SkCommandLineFlags::Parse(SK_ARRAY_COUNT(commands), commands);
        return 0;
    }
    if (FLAGS_bmh.isEmpty() && FLAGS_include.isEmpty()) {
        SkDebugf("requires -b or -i\n");
        SkCommandLineFlags::PrintUsage();
        return 1;
    }
    if (FLAGS_bmh.isEmpty() && !FLAGS_examples.isEmpty()) {
        SkDebugf("-e requires -b\n");
        SkCommandLineFlags::PrintUsage();
        return 1;
    }
    if (FLAGS_hack) {
        if (FLAGS_bmh.isEmpty()) {
            SkDebugf("-k or --hack requires -b\n");
            SkCommandLineFlags::PrintUsage();
            return 1;
        }
        HackParser hacker;
        if (!hacker.parseFile(FLAGS_bmh[0], ".bmh")) {
            SkDebugf("hack failed\n");
            return -1;
        }
        return 0;
    }
    if ((FLAGS_include.isEmpty() || FLAGS_bmh.isEmpty()) && FLAGS_populate) {
        SkDebugf("-p requires -b -i\n");
        SkCommandLineFlags::PrintUsage();
        return 1;
    }
    if (FLAGS_bmh.isEmpty() && !FLAGS_ref.isEmpty()) {
        SkDebugf("-r requires -b\n");
        SkCommandLineFlags::PrintUsage();
        return 1;
    }
    if (FLAGS_bmh.isEmpty() && !FLAGS_spellcheck.isEmpty()) {
        SkDebugf("-s requires -b\n");
        SkCommandLineFlags::PrintUsage();
        return 1;
    }
    if (FLAGS_include.isEmpty() && !FLAGS_tokens.isEmpty()) {
        SkDebugf("-t requires -i\n");
        SkCommandLineFlags::PrintUsage();
        return 1;
    }
    if ((FLAGS_include.isEmpty() || FLAGS_bmh.isEmpty()) && FLAGS_crosscheck) {
        SkDebugf("-x requires -b -i\n");
        SkCommandLineFlags::PrintUsage();
        return 1;
    }
    if (!FLAGS_bmh.isEmpty()) {
        if (!bmhParser.parseFile(FLAGS_bmh[0], ".bmh")) {
            return -1;
        }
    }
    bool done = false;
    if (!FLAGS_include.isEmpty()) {
        if (!FLAGS_tokens.isEmpty() || FLAGS_crosscheck) {
            IncludeParser includeParser;
            includeParser.validate();
            if (!includeParser.parseFile(FLAGS_include[0], ".h")) {
                return -1;
            }
            if (!FLAGS_tokens.isEmpty()) {
                includeParser.fDebugOut = FLAGS_stdout;
                if (includeParser.dumpTokens(FLAGS_tokens[0])) {
                    bmhParser.fWroteOut = true;
                }
                done = true;
            } else if (FLAGS_crosscheck) {
                if (!includeParser.crossCheck(bmhParser)) {
                    return -1;
                }
                done = true;
            }
        } else if (FLAGS_populate) {
            IncludeWriter includeWriter;
            includeWriter.validate();
            if (!includeWriter.parseFile(FLAGS_include[0], ".h")) {
                return -1;
            }
            includeWriter.fDebugOut = FLAGS_stdout;
            if (!includeWriter.populate(bmhParser)) {
                return -1;
            }
            bmhParser.fWroteOut = true;
            done = true;
        }
    }
    FiddleParser fparser(&bmhParser);
    if (!done && !FLAGS_fiddle.isEmpty() && FLAGS_examples.isEmpty()) {
        if (!fparser.parseFile(FLAGS_fiddle[0], ".txt")) {
            return -1;
        }
    }
    if (!done && !FLAGS_ref.isEmpty() && FLAGS_examples.isEmpty()) {
        MdOut mdOut(bmhParser);
        mdOut.fDebugOut = FLAGS_stdout;
        if (mdOut.buildReferences(FLAGS_bmh[0], FLAGS_ref[0])) {
            bmhParser.fWroteOut = true;
        }
    }
    if (!done && !FLAGS_spellcheck.isEmpty() && FLAGS_examples.isEmpty()) {
        bmhParser.spellCheck(FLAGS_bmh[0], FLAGS_spellcheck);
        done = true;
    }
    int examples = 0;
    int methods = 0;
    int topics = 0;
    if (!done && !FLAGS_examples.isEmpty()) {
        // check to see if examples have duplicate names
        if (!bmhParser.checkExamples()) {
            return -1;
        }
        bmhParser.fDebugOut = FLAGS_stdout;
        if (!bmhParser.dumpExamples(FLAGS_examples[0])) {
            return -1;
        }
        return 0;
    }
    if (!bmhParser.fWroteOut) {
        for (const auto& topic : bmhParser.fTopicMap) {
            if (topic.second->fParent) {
                continue;
            }
            examples += count_children(*topic.second, MarkType::kExample);
            methods += count_children(*topic.second, MarkType::kMethod);
            topics += count_children(*topic.second, MarkType::kSubtopic);
            topics += count_children(*topic.second, MarkType::kTopic);
        }
        SkDebugf("topics=%d classes=%d methods=%d examples=%d\n", 
                bmhParser.fTopicMap.size(), bmhParser.fClassMap.size(),
                methods, examples);
    }
    return 0;
}
