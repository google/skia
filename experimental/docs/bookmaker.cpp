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
    size_t doubleColon = normalizedName.find("::", 0);
    if (string::npos != doubleColon) {
        normalizedName = normalizedName.substr(0, doubleColon)
            + '_' + normalizedName.substr(doubleColon + 2);
    }
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
        while (start < fName.length() && '_' == fName[start]) {
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
    for (auto& leafIter : fLeaves) {
        leafIter.second.fVisited = false;
    }
    for (auto& branchIter : fBranches) {
        branchIter.second->clearVisited();
    }
}

void RootDefinition::dumpUnVisited() {
    for (auto& leafIter : fLeaves) {
        if (!leafIter.second.fVisited) {
            SkDebugf("%s\n", leafIter.first.c_str());
        }
    }
    for (auto& branchIter : fBranches) {
        branchIter.second->dumpUnVisited();
    }
}

const Definition* RootDefinition::find(const string& ref) const {
    const auto& leafIter = fLeaves.find(ref);
    if (leafIter != fLeaves.end()) {
        return &leafIter->second;
    }
    const auto branchIter = fBranches.find(ref);
    if (branchIter != fBranches.end()) {
        const RootDefinition* rootDef = branchIter->second;
        return rootDef;
    }
    const Definition* result = nullptr;
    for (const auto iter : fBranches) {
        const RootDefinition* rootDef = iter.second;
        if (ref == "SkPaint_FontMetrics_FontMetricsFlags") {
            SkDebugf("");
        }
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

const Definition* BmhParser::isDefined(const TextParser& parser, const string& ref) const {
    auto rootIter = fClassMap.find(ref);
    if (rootIter != fClassMap.end()) {
        return &rootIter->second;
    }
    auto typedefIter = fTypedefMap.find(ref);
    if (typedefIter != fTypedefMap.end()) {
        return &typedefIter->second;
    }
    auto enumIter = fEnumMap.find(ref);
    if (enumIter != fEnumMap.end()) {
        return &enumIter->second;
    }
    auto constIter = fConstMap.find(ref);
    if (constIter != fConstMap.end()) {
        return &constIter->second;
    }
    auto methodIter = fMethodMap.find(ref);
    if (methodIter != fMethodMap.end()) {
        return &methodIter->second;
    }
    auto aliasIter = fAliasMap.find(ref);
    if (aliasIter != fAliasMap.end()) {
        return aliasIter->second;
    }
    for (const auto& external : fExternals) {
        if (external.fName == ref) {
            return &external;
        }
    }
    if (fRoot) {
        if (const Definition* definition = fRoot->find(ref)) {
            return definition;
        }
        Definition* test = fRoot;
        do {
            if (!test->isRoot()) {
                continue;
            }
            RootDefinition* root = test->asRoot();
            for (string prefix : { "::", "_" } ) {
                string prefixed = root->fName + prefix + ref;
                if (const Definition* definition = root->find(prefixed)) {
                    return definition;
                }
                if (isupper(prefixed[0])) {
                    auto topicIter = fTopicMap.find(prefixed);
                    if (topicIter != fTopicMap.end()) {
                        return topicIter->second;
                    }
                }
            }
        } while ((test = test->fParent));
    }
    size_t doubleColon = ref.find("::");
    if (string::npos != doubleColon) {
        string className = ref.substr(0, doubleColon);
        auto classIter = fClassMap.find(className);
        if (classIter != fClassMap.end()) {
            const RootDefinition& classDef = classIter->second;
            const Definition* result = classDef.find(ref);
            if (result) {
                return result;
            }
        }

    }
    if (!ref.compare(0, 2, "SK") || !ref.compare(0, 3, "sk_")
            || (('k' == ref[0] || 'g' == ref[0] || 'f' == ref[0]) &&
                ref.length() > 1 && isupper(ref[1]))) {
        // try with a prefix
        if ('k' == ref[0]) {
            for (auto const& iter : fEnumMap) {
                if (iter.second.find(ref)) {
                    return &iter.second;
                }
            }
        }
        parser.reportError("SK undefined");
        return nullptr;
    }
    if (isupper(ref[0])) {
        auto topicIter = fTopicMap.find(ref);
        if (topicIter != fTopicMap.end()) {
            return topicIter->second;
        }
        size_t pos = ref.find('_');
        if (string::npos != pos) {
            // see if it is defined by another base class
            string className(ref, 0, pos);
            auto classIter = fClassMap.find(className);
            if (classIter != fClassMap.end()) {
                if (const Definition* definition = classIter->second.find(ref)) {
                    return definition;
                }
            }
            auto enumIter = fEnumMap.find(className);
            if (enumIter != fEnumMap.end()) {
                if (const Definition* definition = enumIter->second.find(ref)) {
                    return definition;
                }
            }
            parser.reportError("_ undefined");
            return nullptr;
        }
    }
    return nullptr;
}

static void add_ref(const string& leadingSpaces, const string& ref, string* result) {
    *result += leadingSpaces + ref;
}

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
            if (MarkType::kEnum == markType && fLineCount >= 3895 && fLineCount <= 3910) {
                SkDebugf("");
            }
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
                    }
                } else {
                    definition->fFiddle = normalized_name(name);
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
                if ("Device_Text" == typeNameBuilder[0]) {
                    SkDebugf("");
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
            prefixed += '_' + name;
            this->skipToEndBracket(fMC);
            auto leafIter = fRoot->fLeaves.find(prefixed);
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
                if ("Dither" == typeNameBuilder[0]) {
                    SkDebugf("");
                }
                // fixme : doesn't handle case where e.g. operator method has
                //         more than one example
                definition->fFiddle = fParent->fFiddle;
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
            }
            break;
        case MarkType::kAlias: {
            if (hasEnd) {
                return this->reportError<bool>("one liners omit end element");
            }
            this->skipWhiteSpace();
            const char* start = fChar;
            this->skipToNonAlphaNum();
            string alias(start, fChar - start);
            if (fAliasMap.end() != fAliasMap.find(alias)) {
                return this->reportError<bool>("duplicate alias");
            }
            definition = fParent;
            fAliasMap[alias] = definition;
            } break;
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

// FIXME: preserve inter-line spaces and don't add new ones
string BmhParser::addReferences(const char* refStart, const char* refEnd) {
    string result;
    MethodParser t(fRoot ? fRoot->fName : string(), refStart, refEnd);
    t.fLineCount = fLineCount;
    bool lineStart = true;
    string ref;
    string leadingSpaces;
    do {
        const char* base = t.fChar;
        t.skipWhiteSpace();
        const char* wordStart = t.fChar;
        t.skipToMethodStart();
        const char* start = t.fChar;
        if (wordStart < start) {
            if (lineStart) {
                lineStart = false;
            } else {
                wordStart = base;
            }
            result += string(wordStart, start - wordStart);
            if ('\n' != result.back()) {
                while (start > wordStart && '\n' == start[-1]) {
                    result += '\n';
                    --start;
                }
            }
        }
        if (lineStart) {
            lineStart = false;
        } else {
            leadingSpaces = string(base, wordStart - base);
         }
        t.skipToMethodEnd();
        if (base == t.fChar) {
            break;
        }
        if (start >= t.fChar) {
            continue;
        }
        if (!t.eof() && '"' == t.peek() && start > wordStart && '"' == start[-1]) {
            continue;
        }
        ref = string(start, t.fChar - start);
        if ("Clip" == ref) {
            SkDebugf("");
        }
        if (const Definition* def = this->isDefined(t, ref)) {
            SkASSERT(def->fFiddle.length());
            result += linkRef(leadingSpaces, def, ref);
            continue;
        }
        if (!t.eof() && '(' == t.peek()) {
            start here; // search again with paren expression
        }
// class, struct, and enum start with capitals
// methods may start with upper (static) or lower (most)

        // see if this should have been a findable reference
                 
            // look for Sk / sk / SK ..
        if (!ref.compare(0, 2, "Sk") && ref != "Skew" && ref != "Skews") {
            t.reportError("missed Sk prefixed");
            return result;
        } 
        if (!ref.compare(0, 2, "SK")) {
            t.reportError("missed SK prefixed");
            return result;
        } 
        if (!isupper(start[0])) {
            add_ref(leadingSpaces, ref, &result);
            continue;
        }
        auto topicIter = fTopicMap.find(ref);
        if (topicIter != fTopicMap.end()) {
            result += linkRef(leadingSpaces, topicIter->second, ref);
            continue;
        }
        bool startsSentence = t.sentenceEnd(start);
        if (!t.eof() && ' ' != t.peek()) {
            add_ref(leadingSpaces, ref, &result);
            continue;
        }
        if (t.fChar + 1 >= t.fEnd || (!isupper(t.fChar[1]) && startsSentence)) {
            add_ref(leadingSpaces, ref, &result);
            continue;
        }
        if (isupper(t.fChar[1]) && startsSentence) {
            TextParser next(&t.fChar[1], t.fEnd);
            string nextWord(next.fChar, next.wordEnd() - next.fChar);
            if (this->isDefined(t, nextWord)) {
                add_ref(leadingSpaces, ref, &result);
                continue;
            }
        }
        Definition* test = fRoot;
        do {
            if (!test->isRoot()) {
                continue;
            }
            for (string prefix : { "_", "::" } ) {
                RootDefinition* root = test->asRoot();
                string prefixed = root->fName + prefix + ref;
                if (const Definition* def = root->find(prefixed)) {
                    result += linkRef(leadingSpaces, def, ref);
                    goto found;
                }
            }
        } while ((test = test->fParent));
    found:
        if (!test) {
            t.reportError("undefined reference");
        }
    } while (!t.eof());
    return result;
}

void BmhParser::buildReferences() {
#if 0
    auto addRef = [this](const Definition& def) -> void {
        string text = def.extractText(Definition::TrimExtract::kYes);
        // parse C++ declaration and description
        string refStr = this->addReferences(&text.front(), &text.back() + 1);
        if (refStr.length()) {
            printf("%s\n", refStr.c_str());
        }
    };
#endif

    fMdHeaderDepth = 1;
    for (auto& oneclass : fClassMap) {
        string name = "bmh_" + oneclass.first + ".md";
        RootDefinition& classDef = oneclass.second;

        fRoot = &classDef;
        fMdOut = fopen(name.c_str(), "w");
        fprintf(fMdOut, "<style>\n");
        fprintf(fMdOut, "pre { padding: 1em 1em 1em 1em; width: 44em; background-color: #f0f0f0 }");
        fprintf(fMdOut, "</style>\n");
        this->markTypeOut(&classDef);
        fclose(fMdOut);
        fMdOut = nullptr;
        continue;

#if 0
        // todo : add class itself
        string text = classDef.extractText(Definition::TrimExtract::kYes);
        string ref = this->addReferences(&text.front(), &text.back() + 1);
        if (ref.length()) {
            printf("%s\n", ref.c_str());
        }
        for (auto& branch : *classDef.fBranches) {
            addRef(branch.second);
        }
        for (auto& leaf : classDef.fLeaves) {
            addRef(leaf.second);
        }
#endif
    }
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

void BmhParser::childrenOut(const Definition* def, const char* start) {
    ++fMdHeaderDepth;
    const char* end;
    for (auto& child : def->fChildren) {
        end = child->fStart;
        fLineCount = def->fLineCount;
        if (this->resolvable(def->fMarkType)) {
            this->resolveOut(start, end);
        }
        this->markTypeOut(child);
        start = child->fTerminator;
    }
    if (this->resolvable(def->fMarkType)) {
        end = def->fContentEnd;
        this->resolveOut(start, end);
    }
    --fMdHeaderDepth;
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
        builder += "_anonymous";
        return uniqueRootName(builder, markType);
    }
    builder = this->word(builder);
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
            Definition* definition = &fExternals.front();
            definition->fName = string(wordStart ,fChar - wordStart);
            definition->fFiddle = normalized_name(definition->fName);
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
                if (MarkType::kClass == markType && fLineCount > 5100) {
                    SkDebugf("");
                }
                vector<string> typeNameBuilder = this->typeName(markType, &expectEnd);
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

string BmhParser::linkName(const Definition* ref) const {
    string result = ref->fName;
    size_t under = result.find('_');
    if (string::npos != under) {
        string classPart = result.substr(0, under);
        string namePart = result.substr(under + 1, result.length());
        if (fRoot && (fRoot->fName == classPart
                || (fRoot->fParent && fRoot->fParent->fName == classPart))) {
            result = namePart;
        }
    }
    return result;
}

// for now, hard-code to html links
// def should not include SkXXX_
string BmhParser::linkRef(const string& leadingSpaces, const Definition* def,
        const string& ref) const {
    string buildup;
    const string* str = &def->fFiddle;
    SkASSERT(str->length() > 0);
    size_t under = str->find('_');
    Definition* curRoot = fRoot;
    string classPart = string::npos != under ? str->substr(0, under) : *str;
    bool classMatch = curRoot->fName == classPart;
    while (curRoot->fParent) {
        curRoot = curRoot->fParent;
        classMatch |= curRoot->fName == classPart;
    }
    const Definition* defRoot;
    do {
        defRoot = def;
        if (!(def = def->fParent)) {
            break;
        }
        classMatch |= def != defRoot && def->fName == classPart;
    } while (true);
    string namePart = string::npos != under ? str->substr(under + 1, str->length()) : *str;
    SkASSERT(fRoot);
    SkASSERT(fRoot->fFileName.length());
    if (classMatch) {
        str = &namePart;
    } else if (curRoot != defRoot && defRoot->isRoot()) {
        string filename = defRoot->asRoot()->fFileName;
        if (filename.substr(filename.length() - 4) == ".bmh") {
            filename = filename.substr(0, filename.length() - 4);
        }
        size_t start = filename.length();
        while (start > 0 && (isalnum(filename[start - 1]) || '_' == filename[start - 1])) {
            --start;
        }
        buildup = "bmh_" + filename.substr(start) + "?cl=9919#"
                + (classMatch ? namePart : *str);
        str = &buildup;
    }
    return leadingSpaces + "<a href=\"" + *str + "\">" + ref + "</a>";
}

void BmhParser::markTypeOut(Definition* def) {
    string printable = def->printableName();
    string colonform = def->colonFormName();
    const char* textStart = def->fContentStart;
    if (MarkType::kParam != def->fMarkType && MarkType::kConst != def->fMarkType &&
            TableState::kNone != fTableState) {
        fprintf(fMdOut, 
                "</table>\n"
                "\n"
        );
        fTableState = TableState::kNone;
    }
    switch (def->fMarkType) {
        case MarkType::kAlias:
            break;
        case MarkType::kAnchor:
            break;
        case MarkType::kBug:
            break;
        case MarkType::kClass:
            this->mdHeaderOut();
            fprintf(fMdOut, "<a name=\"%s\"></a> Class %s\n", this->linkName(def).c_str(),
                    colonform.c_str());
            break;
        case MarkType::kCode:
            fCodeIndent = true;
            break;
        case MarkType::kColumn:
            fprintf(fMdOut, "| ");
            break;
        case MarkType::kComment:
            break;
        case MarkType::kConst: {
            if (TableState::kNone == fTableState) {
                this->mdHeaderOut();
                fprintf(fMdOut, 
                        "Constants\n"
                        "\n"
                        "<table>\n"
                        );
                fTableState = TableState::kRow;
            }
            if (TableState::kRow == fTableState) {
                fprintf(fMdOut, "  <tr>\n");
                fTableState = TableState::kColumn;
            }
            fprintf(fMdOut, "    <td><a name=\"%s\"></a> <code><strong>%s</strong></code></td>",
                    def->fName.c_str(), colonform.c_str());
            const char* lineEnd = strchr(textStart, '\n');
            SkASSERT(lineEnd < def->fTerminator);
            SkASSERT(lineEnd > textStart);
            SkASSERT((int) (lineEnd - textStart) == lineEnd - textStart);
            fprintf(fMdOut, "<td>%.*s</td>", (int) (lineEnd - textStart), textStart);
            fprintf(fMdOut, "<td>");
            textStart = lineEnd;
        } break;
        case MarkType::kDefine:
            break;
        case MarkType::kDefinedBy:
            break;
        case MarkType::kDeprecated:
            break;
        case MarkType::kDescription:
            break;
        case MarkType::kDoxygen:
            break;
        case MarkType::kEnum:
        case MarkType::kEnumClass:
            this->mdHeaderOut();
            fprintf(fMdOut, 
                    "<a name=\"%s\"></a> Enum %s\n"
                    "\n"
                    , def->fName.c_str(), colonform.c_str());
            break;
        case MarkType::kError:
            break;
        case MarkType::kExample:
            this->mdHeaderOut();
            fprintf(fMdOut, 
                    "Example\n"
                    "\n"
                    "<fiddle-embed name=\"%s\"></fiddle-embed>\n"
                    "\n", 
                    def->fHash.c_str());
            break;
        case MarkType::kExternal:
            break;
        case MarkType::kFile:
            break;
        case MarkType::kFormula:
            break;
        case MarkType::kFunction:
            break;
        case MarkType::kImage:
            break;
        case MarkType::kLegend:
            break;
        case MarkType::kLink:
            break;
        case MarkType::kList:
            break;
        case MarkType::kMember:

            break;
        case MarkType::kMethod: {
            string method_name = def->methodName();
            string formattedStr = def->formatFunction();

        //    start here;
            // put in css spec that we can define somewhere else (if markup supports that)
                // display: block
                // font-family: monospace
                // white-space: pre
                // background: #faf8f0
            fprintf(fMdOut, "<a name=\"%s\"></a>\n", def->fFiddle.c_str());
            // add a header for method here, before method code def
            this->mdHeaderOut();
            fprintf(fMdOut, 
                    "%s\n"
                    "\n", method_name.c_str());

            fprintf(fMdOut, 
                "<pre>\n"
                "%s\n"  // four spaces notes code follows
                "</pre>\n",  formattedStr.c_str());
            fTableState = TableState::kNone;
            } break;
        case MarkType::kParam: {
            if (TableState::kNone == fTableState) {
                this->mdHeaderOut();
                fprintf(fMdOut, 
                        "Parameters\n"
                        "\n"
                        "<table>\n"
                        );
                fTableState = TableState::kRow;
            }
            if (TableState::kRow == fTableState) {
                fprintf(fMdOut, "  <tr>\n");
                fTableState = TableState::kColumn;
            }
            TextParser paramParser(def->fStart, def->fContentStart);
            paramParser.skipWhiteSpace();
            SkASSERT(paramParser.startsWith("#Param"));
            paramParser.next(); // skip hash
            paramParser.skipToNonAlphaNum(); // skip Param
            paramParser.skipSpace();
            const char* paramName = paramParser.fChar;
            paramParser.skipToSpace();
            fprintf(fMdOut, 
                    "    <td><code><strong>%.*s</strong></code></td> <td>",
                    (int) (paramParser.fChar - paramName), paramName);
        } break;
        case MarkType::kPlatform:
            break;
        case MarkType::kReturn:
            this->mdHeaderOut();
            fprintf(fMdOut, 
                    "Return Value\n"
                    "\n");
            break;
        case MarkType::kRow:
            break;
        case MarkType::kSeeAlso:
            this->mdHeaderOut();
            fprintf(fMdOut, 
                    "See Also\n"
                    "\n"
                    );
            break;
        case MarkType::kStdOut: {
            TextParser code(def->fContentStart, def->fContentEnd);
            this->mdHeaderOut();
            fprintf(fMdOut, 
                    "Example Output\n"
                    "\n"
                    "~~~~\n");
            code.skipSpace();
            while (!code.eof()) {
                const char* end = code.trimmedLineEnd();
                fprintf(fMdOut, "%.*s\n", (int) (end - code.fChar), code.fChar);
                code.skipToLineStart();
            }
            fprintf(fMdOut, "~~~~\n\n");
            } break;
        case MarkType::kStruct:
            fRoot = def->asRoot();
            this->mdHeaderOut();
            fprintf(fMdOut, "<a name=\"%s\"></a> Struct %s\n", def->fName.c_str(), colonform.c_str());
            break;
        case MarkType::kSubtopic:
            this->mdHeaderOut();
            fprintf(fMdOut, "<a name=\"%s\"></a> %s\n", def->fName.c_str(), printable.c_str());
            break;
        case MarkType::kTable:
            fprintf(fMdOut, "\n");
            break;
        case MarkType::kTemplate:
            break;
        case MarkType::kText:
            break;
        case MarkType::kTime:
            break;
        case MarkType::kToDo:
            break;
        case MarkType::kTopic:
            this->mdHeaderOut();
            fprintf(fMdOut, "<a name=\"%s\"></a> %s\n", this->linkName(def).c_str(),
                    printable.c_str());
            break;
        case MarkType::kTrack:
            // don't output children
            return;
        case MarkType::kTypedef:
            break;
        case MarkType::kUnion:
            break;
        case MarkType::kWidth:
            break;
        default:
            SkASSERT(0); // handle everything
            break;
    }
    this->childrenOut(def, textStart);
    switch (def->fMarkType) {  // post child work, at least for tables
        case MarkType::kCode:
            fCodeIndent = false;
            fprintf(fMdOut, 
                    "\n"
                    "\n"
            );
            break;
        case MarkType::kColumn:
            fprintf(fMdOut, " ");
            break;
        case MarkType::kDescription:
            fprintf(fMdOut, "\n");
            break;
        case MarkType::kLegend: {
            SkASSERT(def->fChildren.size() == 1);
            const Definition* row = def->fChildren[0];
            SkASSERT(MarkType::kRow == row->fMarkType);
            size_t columnCount = row->fChildren.size();
            SkASSERT(columnCount > 0);
            for (size_t index = 0; index < columnCount; ++index) {
                fprintf(fMdOut, "| --- ");
            }
            fprintf(fMdOut, " |\n");
            } break;
        case MarkType::kMethod:
            fprintf(fMdOut, 
                    "---\n"
                    "\n"
            );
            break;
        case MarkType::kConst:
        case MarkType::kParam:
            SkASSERT(TableState::kColumn == fTableState);
            fTableState = TableState::kRow;
            fprintf(fMdOut, "</td>\n");
            fprintf(fMdOut, "  </tr>\n");
            break;
        case MarkType::kReturn:
            fprintf(fMdOut, 
                    "\n"
                    "\n"
            );
            break;
        case MarkType::kRow:
            fprintf(fMdOut, "|\n");
            break;
        case MarkType::kStruct:
            fRoot = fRoot->rootParent();
            break;
        case MarkType::kTable:
            fprintf(fMdOut, "\n");
            break;
        default:
            break;
    }
}

void BmhParser::mdHeaderOut() {
    for (int index = 0; index < fMdHeaderDepth; ++index) {
        fprintf(fMdOut, "#");
    }
    if (fMdHeaderDepth) {
        fprintf(fMdOut, " ");
    }
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

void BmhParser::resolveOut(const char* start, const char* end) {
    while (fMC == end[-1]) {
        --end;
    }
    if (start >= end) {
        return;
    }
    string resolved = this->addReferences(start, end);
    trim_end_spaces(resolved);
    if (resolved.length()) {
        TextParser paragraph(&*resolved.begin(), &*resolved.end());
        TextParser original(start, end);
        while (!original.eof() && '\n' == original.peek()) {
            original.next();
        }
        const char* originalStart = original.fChar;
        original.skipSpace();
        int baseIndent = (int) (original.fChar - originalStart);
        int matchIndent = fCodeIndent ? 4 : 0;
        bool firstLine = true;
        while (!paragraph.eof()) {
            const char* lineStart = paragraph.fChar;
            paragraph.skipSpace();
            const char* contentStart = paragraph.fChar;
            int lineIndent = (int) (contentStart - lineStart);
            paragraph.skipToEndBracket('\n');
            ptrdiff_t lineLength = paragraph.fChar - contentStart;
            int indent = matchIndent + (firstLine ? 0 : SkTMax(0, lineIndent - baseIndent));
            if (indent) {
                fprintf(fMdOut, "%.*s", indent, "                                                ");
            }
            fprintf(fMdOut, "%.*s", (int) lineLength, contentStart);
            if (paragraph.eof()) {
                break;
            }
            if ('\n' == paragraph.next()) {
                fprintf(fMdOut, "\n");
            }
            firstLine = false;
        }
#if 0
        while (end > start && end[0] == '\n') {
            fprintf(fMdOut, "\n");
            --end;
        }
#endif
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
            builder = this->word(builder);
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
        for (auto& iter : fParent->fChildren) {
            if (markType == iter->fMarkType) {
                if (iter->fName == numBuilder) {
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
        for (auto& iter : parent->fBranches) {
            if (checkName(*iter.second, numBuilder)) {
                goto tryNext;
            }
        }
        for (auto& iter : parent->fLeaves) {
            if (checkName(iter.second, numBuilder)) {
                goto tryNext;
            }
        }
        break;
tryNext: ;
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

string BmhParser::word(const string& prefix) {
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
        builder += '_';
    }
    builder.append(nameStart, fChar - nameStart - 1);
    return builder;
}

// pass one: parse text, collect definitions
// pass two: lookup references

// -b "C:/skia/experimental/docs/paint.bmh" -e
// cp out/skia/obj/fiddle.js experimental/docs   # or append...
// -b "C:/skia/experimental/docs" -r -f "C:/skia/experimental/docs/fiddles.out.txt" -t

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
    FiddleParser fparser(bmhParser);
    if (!FLAGS_fiddle.isEmpty() && !FLAGS_examples) {
        if (!fparser.parseFile(FLAGS_fiddle[0], ".txt")) {
            return -1;
        }
    }
    if (FLAGS_ref && !FLAGS_examples) {
        bmhParser.buildReferences();
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
