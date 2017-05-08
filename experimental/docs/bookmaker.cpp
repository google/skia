/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bookmaker.h"

#include "SkCommandLineFlags.h"
#include "SkOSFile.h"
#include "SkOSPath.h"


/*  recipe for generating timestamps for existing doxygen comments
find include/core -type f -name '*.h' -print -exec git blame {} \; > ~/all.blame.txt
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

static bool starts_with(const string& base, const char* startStr) {
   string start(startStr);
   return !base.compare(0, start.size(), start);
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
    bool addToString = false;
    do {
        if (addToString) {
            if ('"' == example.back()) {
                example += " +\n";
            }
        } else {
            addToString = true;
        }
         // fix this to move whole paragraph in, out, but preserve doc indent
        int nextIndent = count_indent(text, pos, end);
        size_t len = text.find('\n', pos);
        if (string::npos == len) {
            len = end;
        }
        if ((size_t) (pos + nextIndent) < len) {
            example += '"';
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
            example += "\\n\"";
        } else {
            pos += nextIndent;
            addToString = false;
        }
        if ('\n' == text[pos]) {
            ++pos;
        }
    } while (pos < end);
}

// fixme: this will need to be more complicated to handle all of Skia
// for now, just handle paint -- maybe fiddle will loosen naming restrictions
static string canonical_fiddle_name(const string& name) {
    size_t doubleColons = name.find("::", 0);
    SkASSERT(string::npos != doubleColons);
    string result = name.substr(0, doubleColons);
    result += "_";
    doubleColons += 2;
    if (string::npos != name.find('~', doubleColons)) {
        result += "destructor";
    } else {
        bool isMove = string::npos != name.find("&&", doubleColons);
        const char operatorStr[] = "operator";
        size_t opPos = name.find(operatorStr, doubleColons);
        if (string::npos != opPos) {
            opPos += sizeof(operatorStr) - 1;
            if ('!' == name[opPos]) {
                SkASSERT('=' == name[opPos + 1]);
                result += "not_equal_operator"; 
            } else if ('=' == name[opPos]) {
                if ('(' == name[opPos + 1]) {
                    result += isMove ? "move_" : "copy_"; 
                    result += "assignment_operator"; 
                } else {
                    SkASSERT('=' == name[opPos + 1]);
                    result += "equal_operator"; 
                }
            } else {
                SkASSERT(0);  // todo: incomplete
            }
        } else if (string::npos != name.find("()", doubleColons)) {
            result += "empty_constructor"; 
        } else {
            size_t comma = name.find(',', doubleColons);
            size_t start = name.find('(', doubleColons);
            if (string::npos == comma && string::npos != start) {
                result += isMove ? "move_" : "copy_"; 
                result += "constructor"; 
            } else if (string::npos == start) {
                result += name.substr(doubleColons);
            } else {
                // name them by their param types, e.g. SkCanvas__int_int_const_SkSurfaceProps_star
                SkASSERT(string::npos != start);
                // move forward until parens are balanced and terminator =,)
                TextParser params(&name[start] + 1, &*name.end());
                bool underline = false;
                while (!params.eof()) {
                    int parens = 0;
                    const char* end = params.anyOf("(),=");  // unused for now
                    SkASSERT(end[0] != '(');  // fixme: put off handling nested parentheseses
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
                    if ('*' == params.peek()) {
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
    result = normalized_name(result);
    return result;
}

bool Definition::exampleToScript(string* result) const {
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
    bool textOut = string::npos != text.find("SkDebugf(");  // path->dump() also prints text
    string widthStr = "256";
    bool preprocessor = text[0] == '#';
    if (string::npos != fFiddle.find("::")) {
        SkDebugf("");
    }
    string normalizedName(fFiddle);
    string example = "var " + normalizedName + "_code = \n";
    string imageStr = "0";
    for (auto const& iter : fChildren) {
        switch (iter->fMarkType) {
            case MarkType::kError:
                result->clear();
                return true;
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
                add_code(funcText, pos, funcText.length(), 0, indent, example);
                example += " +\n";
                example += "\"\\n\" +\n";
                } break;
            case MarkType::kComment:
                break;
            case MarkType::kImage:
                imageStr = string(iter->fContentStart, iter->fContentEnd - iter->fContentStart);
                break;
            case MarkType::kToDo:
                break;
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
        example += "\"";
        example += hasCanvas ? drawNoCanvas : drawWrapper;
        example += "\\n\" + \n";
        outIndent = 4;
    }
    add_code(text, pos, end, outIndent, textIndent, example);
    if (wrapCode) {
        if ('"' == example.back()) {
            example += " +\n";
        }
        example += "\"}\\n\"";
    }
    example += ";\n\nvar " + normalizedName + "_json = {\n";
    example += "    \"code\": " + normalizedName + "_code,\n";
    example += "    \"options\": {\n";
    example += "        \"width\": " + widthStr + ",\n";
    example += "        \"height\": 256,\n";
    example += "        \"source\": " + imageStr + ",\n";
    example += "        \"textOnly\": " + textOutStr + "\n";
    example += "    },\n";
    example += "    \"name\": \"" + normalizedName + "\",\n";
    example += "    \"overwrite\": true\n";
    example += "}\n\n";
    example += "runFiddle(" + normalizedName + "_json);\n";
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

string Definition::formatFunction() const {
    const char* end = fContentStart;
    while (end > fStart && ' ' >= end[-1]) {
        --end;
    }
    TextParser methodParser(fStart, end);
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
            space_pad(&methodStr);
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
    int start = 0;
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
    int end = fFiddle.find_first_of('(', start);
    return fFiddle.substr(start, (size_t) (end - start));
}

string Definition::methodName() const {
    string result;
    int start = 0;
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
    int end = fName.find_first_of('(', start);
    return fName.substr(start, (size_t) (end - start));
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

void RootDefinition::clearVisited() {
    fVisited = false;
    for (auto& leaf : fLeaves) {
        leaf.second.fVisited = false;
    }
    for (auto& branch : fBranches) {
        branch.second->clearVisited();
    }
}

void RootDefinition::dumpUnVisited() {
    for (auto& leaf : fLeaves) {
        if (!leaf.second.fVisited) {
            SkDebugf("%s\n", leaf.first.c_str());
        }
    }
    for (auto& branch : fBranches) {
        branch.second->dumpUnVisited();
    }
}

const Definition* RootDefinition::find(const string& ref) const {
    const auto leafIter = fLeaves.find(ref);
    if (leafIter != fLeaves.end()) {
        return &leafIter->second;
    }
    const auto branchIter = fBranches.find(ref);
    if (branchIter != fBranches.end()) {
        const RootDefinition* rootDef = branchIter->second;
        return rootDef;
    }
    const Definition* result = nullptr;
    for (const auto& branch : fBranches) {
        const RootDefinition* rootDef = branch.second;
        result = rootDef->find(ref);
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
            if (string::npos != name.find('_')) {
                SkDebugf("");
            }
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
                    if (!hasEnd && fRoot->find(name)) {
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
                if (!this->popParentStack(definition)) {
                    return false;
                }
            } else {
                definition->fStart = defStart;
                this->skipSpace();
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
                        definition->fFiddle = canonical_fiddle_name(name);
                    } else {
                        definition->fFiddle = name;
                    }
                } else {
                    definition->fFiddle = normalized_name(name);
                }
                if (string::npos != definition->fFiddle.find("::")) {
                    SkDebugf("");
                }
                definition->fMarkType = markType;
                this->setAsParent(definition);
            }
            } break;
        case MarkType::kTopic: // may define multiple keys
        case MarkType::kSubtopic:
            SkASSERT(typeNameBuilder.size() > 0);
            if (!hasEnd) {
                if (!typeNameBuilder.size()) {
                    return this->reportError<bool>("unnamed topic");
                }
                fTopics.emplace_front(markType, defStart, fLineCount, fParent);
                RootDefinition* rootDefinition = &fTopics.front();
                rootDefinition->fFileName = fFileName;
                definition = rootDefinition;
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
            for (auto& topic : typeNameBuilder) {
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
            if (!this->childOf(MarkType::kExample)) {
                return false;
            }
        // not required to be children of example, may be one-liner
        case MarkType::kBug:
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
                } else {
                    fMarkup.emplace_front(markType, defStart, fLineCount, fParent);
                    definition = &fMarkup.front();
                    definition->fName = typeNameBuilder[0];
                    definition->fFiddle = normalized_name(typeNameBuilder[0]);
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
        case MarkType::kFormula:
        case MarkType::kFunction:  // todo: only allow in examples
        case MarkType::kLegend:
        case MarkType::kList:
        case MarkType::kTable:
        case MarkType::kTrack:
            if (hasEnd) {
                definition = fParent;
                if (markType != fParent->fMarkType) {
                    return this->reportError<bool>("end element mismatch");
                } else if (!this->popParentStack(fParent)) {
                    return false;
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
        case MarkType::kError:
        case MarkType::kImage:
        case MarkType::kPlatform:
        case MarkType::kWidth:
            if (!this->childOf(MarkType::kExample)) {
                return false;
            }
        case MarkType::kAnchor: 
        case MarkType::kFile:
        case MarkType::kSeeAlso:
        case MarkType::kTime:
        case MarkType::kAlias:
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
                // fixme: expect text #reference ##
                // or            text #url ##
                definition->fContentEnd = link->fContentEnd;
                definition->fTerminator = fChar;
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
    string builder;
    const Definition* parent = this->parentSpace();
    if (parent && (parent != fParent || MarkType::kClass != markType)) {
        builder += parent->fName;
    }
    const char* end = this->lineEnd();
    const char* mc = this->strnchr(fMC, end);
    if (mc) {
        this->skipSpace();
        const char* wordStart = fChar;
        this->skipToNonAlphaNum();
        const char* wordEnd = fChar;
        if (mc + 1 < fEnd && fMC == mc[1]) {  // if ##
            if (markType != fParent->fMarkType) {
                return this->reportError<string>("unbalanced method");
            }
            if (builder.length() > 0 && wordEnd > wordStart) {
                if (builder != fParent->fName) {
                    builder += string(wordStart, wordEnd - wordStart);
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
        builder += "_anonymous";
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
            definition->fName = string(wordStart ,fChar - wordStart);
            definition->fFiddle = normalized_name(definition->fName);
            definition->fFileName = fFileName;
        }
    } while (!this->eof());
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

//start here;
// a definition may have more than one example
// either: examples should be namable, and author must name multiples differently
// or: multiple examples must be tracked and numbered uniquely when found
// maybe typeName() can check to see if parent already has a thing of this type if it is unnamed

// also, some examples may produce different output on different platforms 
// if the text output can be different, think of how to author that

bool BmhParser::findDefinitions() {
    bool lineStart = true;
    fParent = nullptr;
    while (!this->eof()) {
        if (this->peek() == fMC) {
            this->next();
            if (this->peek() == fMC) {
                this->next();
                if (!lineStart && ' ' < this->peek()) {
                    return this->reportError<bool>("expected definition");
                }
                if (this->peek() != fMC) {
                    vector<string> parentName;
                    parentName.push_back(fParent->fName);
                    if (!this->addDefinition(fChar - 1, true, fParent->fMarkType, parentName)) {
                        return false;
                    }
                } else {
                    SkAssertResult(this->next() == fMC);
                    fMC = this->next();  // change markup character
                    if (' ' >= fMC) {
                        return this->reportError<bool>("illegal markup character");
                    }
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
                        && MarkType::kLegend != fParent->fMarkType)) {
                    int endHashes = this->endHashCount();
                    if (endHashes <= 1) {  // one line comment
                        if (fParent) {
                            fMarkup.emplace_front(MarkType::kComment, fChar - 1, fLineCount, fParent);
                            Definition* comment = &fMarkup.front();
                            comment->fContentStart = fChar - 1;
                            this->skipToEndBracket('\n');
                            comment->fContentEnd = fChar;
                            comment->fTerminator = fChar;
                            fParent->fChildren.push_back(comment);
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
                } else {
                    // fixme? no nested tables for now
                    const char* colStart = fChar - 1;
                    fMarkup.emplace_front(MarkType::kRow, colStart, fLineCount, fParent);
                    Definition* row = &fMarkup.front();
                    this->skipWhiteSpace();
                    row->fContentStart = fChar;
                    this->setAsParent(row);
                    const char* lineEnd = this->lineEnd();
                    do {
                        fMarkup.emplace_front(MarkType::kColumn, colStart, fLineCount, fParent);
                        Definition* column = &fMarkup.front();
                        column->fContentStart = fChar;
                        column->fContentEnd = this->trimmedBracketEnd(fMC);
                        this->skipToEndBracket(fMC);
                        colStart = fChar;
                        SkAssertResult(fMC == this->next());
                        if (fMC == this->peek()) {
                            this->next();
                        }
                        column->fTerminator = fChar;
                        fParent->fChildren.push_back(column);
                        this->skipSpace();
                    } while (fChar < lineEnd && '\n' != this->peek());
                    if (!this->popParentStack(fParent)) {
                        return false;
                    }
                    const Definition* lastCol = row->fChildren.back();
                    row->fContentEnd = lastCol->fContentEnd;
                }
            }
        }
        lineStart = this->next() == '\n';
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

    // if line has #, name follows
    // otherwise, name preceeds first paren
    // fixme: change markup.bmh to require # if duplicate name before parent
    // or first paren is not beginning of function parameter list
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
    int parens = 0;
    while (fChar < end || parens > 0) {
        if ('(' == this->peek()) {
            ++parens;
        } else if (')' == this->peek()) {
            --parens;
        }
        this->next();
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

static void ReportError(const char* errorStr, int lineCount, int lineLen, 
        const char* line, const char* ch) {
    const char at_line[] = " at line ";
    const char colon_space_quote[] = ": \"";
    SkDebugf("%s%s%d%s%.*s\"\n", errorStr, at_line,
            lineCount, colon_space_quote, lineLen - 1, line);
    if (ch != line) {
        int spaces = ch - line;
        spaces += strlen(errorStr);
        spaces += sizeof(at_line) - 1;
        spaces += sizeof(colon_space_quote) - 1;
        spaces += log10(lineCount);
        SkDebugf("%*s %c\n", spaces, "", '^');
    }
    printf("");  // convenient place to set a breakpoint
}

void TextParser::reportError(const char* errorStr) const {
    ReportError(errorStr, fLineCount, this->lineLength(), fLine, fChar);
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
    ptrdiff_t startLen = this->lineLength();
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
    ReportError("unbalanced stack", startLineCount, startLen, start, start);
    return false;
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
        case MarkType::kAnchor: 
            *checkEnd = false;
            break;  // unnamed
        case MarkType::kEnum:
            // enums may be nameless
        case MarkType::kConst:
        case MarkType::kEnumClass:
        case MarkType::kClass:
        case MarkType::kStruct:
        case MarkType::kTypedef:
            if (MarkType::kEnum == markType) {
                SkDebugf("");
            }
            // expect name
            // fixme: need a way to reference enum defined in another file (e.g. SkFilterQuality)
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
        case MarkType::kExternal:
        case MarkType::kFormula:
        case MarkType::kFunction:
        case MarkType::kLegend:
        case MarkType::kList:
        case MarkType::kTrack:
            this->skipNoName();
            break;
        case MarkType::kAlias:
        case MarkType::kBug:  // fixme: expect number
        case MarkType::kDefinedBy:
        case MarkType::kError:
        case MarkType::kFile:
        case MarkType::kImage:
        case MarkType::kPlatform:
        case MarkType::kReturn:
        case MarkType::kSeeAlso:
        case MarkType::kTime:
        case MarkType::kToDo:
        case MarkType::kWidth:
            *checkEnd = false;  // no name, has text body
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
            builder = this->word(builder, "::");
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
    auto checkName = [this, markType](const Definition& def, const string& numBuilder) -> bool {
        return markType == def.fMarkType && def.fName == numBuilder;
    };

    string builder(base);
    if (!builder.length()) {
        builder = fParent->fName;
    }
    const RootDefinition* parent = fRoot;
    if (!parent) {
        return builder;
    }
    int number = 2;
    string numBuilder(builder);
    do {
        for (const auto& iter : parent->fBranches) {
            if (checkName(*iter.second, numBuilder)) {
                goto tryNext;
            }
        }
        for (const auto& iter : parent->fLeaves) {
            if (checkName(iter.second, numBuilder)) {
                goto tryNext;
            }
        }
        break;
tryNext: ;
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

// -b "C:/skia/experimental/docs/paint.bmh" -e
// cp out/skia/obj/fiddle.js experimental/docs   # or append...
// -b "C:/skia/experimental/docs" -r -f "C:/skia/experimental/docs/fiddles.out.txt"

DEFINE_string2(bmh, b, "", "A path to a *.bmh file or a directory.");
DEFINE_string2(include, i, "", "A path to a *.h file or a directory.");
DEFINE_string2(fiddle, f, "", "A path to fiddles.htm console output, usually fiddles.out.txt");
DEFINE_bool2(ref, r, false, "Build references.");
DEFINE_bool2(tokens, t, false, "Output include tokens. (Requires -i)");
DEFINE_bool2(crosscheck, x, false, "Check bmh against includes. (Requires -i)");
DEFINE_bool2(examples, e, false, "Output examples. (For now, disables -r -f)");

static bool dump_examples(FILE* fiddleOut, const Definition& def) {
    if (MarkType::kExample == def.fMarkType) {
        string result;
        if (!def.exampleToScript(&result)) {
            return false;
        }
        fprintf(fiddleOut, "%s\n", result.c_str());
        return true;
    }
    for (auto& child : def.fChildren ) {
        if (!dump_examples(fiddleOut, *child)) {
            return false;
        }
    }
    return true;
}

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
    InterfaceParser interfaceParser;
    interfaceParser.validate();

   SkCommandLineFlags::SetUsage(
        "Usage: bookmaker -b path/to/file.bmh [-i path/to/file.h -t -x]\n");
    SkCommandLineFlags::Parse(argc, argv);
    if (FLAGS_bmh.isEmpty() && FLAGS_include.isEmpty()) {
        SkDebugf("requires -b or -i\n");
        SkCommandLineFlags::PrintUsage();
        return 1;
    }
    if (FLAGS_include.isEmpty() && FLAGS_tokens) {
        SkDebugf("-t requires -i\n");
        SkCommandLineFlags::PrintUsage();
        return 1;
    }
    if (FLAGS_include.isEmpty() && FLAGS_crosscheck) {
        SkDebugf("-x requires -i\n");
        SkCommandLineFlags::PrintUsage();
        return 1;
    }
    if (FLAGS_bmh.isEmpty() && FLAGS_ref) {
        SkDebugf("-r requires -b\n");
        SkCommandLineFlags::PrintUsage();
        return 1;
    }
    if (!FLAGS_bmh.isEmpty()) {
//        parser.setParseType(Parser::ParseType::kBmh);
        if (!bmhParser.parseFile(FLAGS_bmh[0], ".bmh")) {
            return -1;
        }
    }
    if (!FLAGS_include.isEmpty()) {
//        parser.setParseType(Parser::ParseType::kInclude);
        if (!interfaceParser.parseFile(FLAGS_include[0], ".h")) {
            return -1;
        }
        if (FLAGS_tokens) {
            interfaceParser.dumpTokens();
        }
        if (FLAGS_crosscheck) {
            if (!interfaceParser.crossCheck(bmhParser)) {
                return -1;
            }
        }
    }
    FiddleParser fparser(&bmhParser);
    if (!FLAGS_fiddle.isEmpty() && !FLAGS_examples) {
        if (!fparser.parseFile(FLAGS_fiddle[0], ".txt")) {
            return -1;
        }
    }
    if (FLAGS_ref && !FLAGS_examples) {
        MdOut mdOut(bmhParser);
        mdOut.buildReferences(FLAGS_bmh[0]);
        SkDebugf("\n");
    }
    int examples = 0;
    int methods = 0;
    int topics = 0;
    FILE* fiddleOut;
    if (FLAGS_examples) {
        fiddleOut = fopen("fiddle.js", "w");
        fprintf(fiddleOut, "function testFiddles() {\n\n");
    }
    for (auto& oneclass : bmhParser.fClassMap) {
        examples += count_children(oneclass.second, MarkType::kExample);
        methods += count_children(oneclass.second, MarkType::kMethod);
        topics += count_children(oneclass.second, MarkType::kSubtopic);
        topics += count_children(oneclass.second, MarkType::kTopic);
        if (FLAGS_examples) {
            dump_examples(fiddleOut, oneclass.second);
        }
    }
    if (FLAGS_examples) {
        fprintf(fiddleOut, "}\n\n");
        fclose(fiddleOut);
    }
    SkDebugf("topics=%d classes=%d methods=%d examples=%d\n", 
            bmhParser.fTopicMap.size(), bmhParser.fClassMap.size(),
            methods, examples);
    return 0;
}
