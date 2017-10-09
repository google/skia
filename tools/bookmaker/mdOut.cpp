/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bookmaker.h"

#include "SkOSFile.h"
#include "SkOSPath.h"

static void add_ref(const string& leadingSpaces, const string& ref, string* result) {
    *result += leadingSpaces + ref;
}

static string preformat(const string& orig) {
    string result;
    for (auto c : orig) {
        if ('<' == c) {
          result += "&lt;";
        } else if ('>' == c) {
          result += "&gt;";
        } else {
            result += c;
        }
    }
    return result;
}

// FIXME: preserve inter-line spaces and don't add new ones
string MdOut::addReferences(const char* refStart, const char* refEnd,
        BmhParser::Resolvable resolvable) {
    string result;
    MethodParser t(fRoot ? fRoot->fName : string(), fFileName, refStart, refEnd, fLineCount);
    bool lineStart = true;
    string ref;
    string leadingSpaces;
    int distFromParam = 99;
    do {
        ++distFromParam;
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
        if (const Definition* def = this->isDefined(t, ref,
                BmhParser::Resolvable::kOut != resolvable)) {
            SkASSERT(def->fFiddle.length());
            if (!t.eof() && '(' == t.peek() && t.strnchr(')', t.fEnd)) {
                if (!t.skipToEndBracket(')')) {
                    t.reportError("missing close paren");
                    return result;
                }
                t.next();
                string fullRef = string(start, t.fChar - start);
                // if _2 etc alternates are defined, look for paren match
                // may ignore () if ref is all lower case
                // otherwise flag as error
                int suffix = '2';
                bool foundMatch = false;
                const Definition* altDef = def;
                while (altDef && suffix <= '9') {
                    if ((foundMatch = altDef->paramsMatch(fullRef, ref))) {
                        def = altDef;
                        ref = fullRef;
                        break;
                    }
                    string altTest = ref + '_';
                    altTest += suffix++;
                    altDef = this->isDefined(t, altTest, false);
                }
                if (suffix > '9') {
                    t.reportError("too many alts");
                    return result;
                }
                if (!foundMatch) {
                    if (!(def = this->isDefined(t, fullRef, true))) {
                        if (!result.size()) {
                            t.reportError("missing method");
                        }
                        return result;
                    }
                    ref = fullRef;
                }
            }
            result += linkRef(leadingSpaces, def, ref);
            continue;
        }
        if (!t.eof() && '(' == t.peek()) {
            if (!t.skipToEndBracket(')')) {
                t.reportError("missing close paren");
                return result;
            }
            t.next();
            ref = string(start, t.fChar - start);
            if (const Definition* def = this->isDefined(t, ref, true)) {
                SkASSERT(def->fFiddle.length());
                result += linkRef(leadingSpaces, def, ref);
                continue;
            }
        }
// class, struct, and enum start with capitals
// methods may start with upper (static) or lower (most)

        // see if this should have been a findable reference

            // look for Sk / sk / SK ..
        if (!ref.compare(0, 2, "Sk") && ref != "Skew" && ref != "Skews" &&
              ref != "Skip" && ref != "Skips") {
            t.reportError("missed Sk prefixed");
            return result;
        }
        if (!ref.compare(0, 2, "SK")) {
            if (BmhParser::Resolvable::kOut != resolvable) {
                t.reportError("missed SK prefixed");
            }
            return result;
        }
        if (!isupper(start[0])) {
            // TODO:
            // look for all lowercase w/o trailing parens as mistaken method matches
            // will also need to see if Example Description matches var in example
            const Definition* def;
            if (fMethod && (def = fMethod->hasParam(ref))) {
                result += linkRef(leadingSpaces, def, ref);
                fLastParam = def;
                distFromParam = 0;
                continue;
            } else if (!fInDescription && ref[0] != '0'
                    && string::npos != ref.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ")) {
                // FIXME: see isDefined(); check to see if fXX is a member of xx.fXX
                if (('f' != ref[0] && string::npos == ref.find("()"))
//                        || '.' != t.backup(ref.c_str())
                        && ('k' != ref[0] && string::npos == ref.find("_Private"))) {
                    if ('.' == wordStart[0] && (distFromParam >= 1 && distFromParam <= 16)) {
                        const Definition* paramType = this->findParamType();
                        if (paramType) {
                            string fullName = paramType->fName + "::" + ref;
                            bool found = false;
                            for (auto child : paramType->fChildren) {
                                if (fullName == child->fName) {
                                    result += linkRef(leadingSpaces, paramType, ref);
                                    found = true;
                                    break;
                                }
                            }
                            if (found) {
                                continue;
                            }
                        }
                    }
                    if (BmhParser::Resolvable::kOut != resolvable) {
                        t.reportError("missed camelCase");
                        return result;
                    }
                }
            }
            add_ref(leadingSpaces, ref, &result);
            continue;
        }
        auto topicIter = fBmhParser.fTopicMap.find(ref);
        if (topicIter != fBmhParser.fTopicMap.end()) {
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
            TextParser next(t.fFileName, &t.fChar[1], t.fEnd, t.fLineCount);
            string nextWord(next.fChar, next.wordEnd() - next.fChar);
            if (this->isDefined(t, nextWord, true)) {
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
                if (const Definition* def = root->find(prefixed,
                        RootDefinition::AllowParens::kYes)) {
                    result += linkRef(leadingSpaces, def, ref);
                    goto found;
                }
            }
        } while ((test = test->fParent));
    found:
        if (!test) {
            if (BmhParser::Resolvable::kOut != resolvable) {
                t.reportError("undefined reference");
            }
        }
    } while (!t.eof());
    return result;
}

bool MdOut::buildReferences(const char* fileOrPath, const char* outDir) {
    if (!sk_isdir(fileOrPath)) {
        if (!this->buildRefFromFile(fileOrPath, outDir)) {
            SkDebugf("failed to parse %s\n", fileOrPath);
            return false;
        }
    } else {
        SkOSFile::Iter it(fileOrPath, ".bmh");
        for (SkString file; it.next(&file); ) {
            SkString p = SkOSPath::Join(fileOrPath, file.c_str());
            const char* hunk = p.c_str();
            if (!SkStrEndsWith(hunk, ".bmh")) {
                continue;
            }
            if (SkStrEndsWith(hunk, "markup.bmh")) {  // don't look inside this for now
                continue;
            }
            if (!this->buildRefFromFile(hunk, outDir)) {
                SkDebugf("failed to parse %s\n", hunk);
                return false;
            }
        }
    }
    return true;
}

bool MdOut::buildRefFromFile(const char* name, const char* outDir) {
    fFileName = string(name);
    string filename(name);
    if (filename.substr(filename.length() - 4) == ".bmh") {
        filename = filename.substr(0, filename.length() - 4);
    }
    size_t start = filename.length();
    while (start > 0 && (isalnum(filename[start - 1]) || '_' == filename[start - 1])) {
        --start;
    }
    string match = filename.substr(start);
    string header = match;
    filename = match + ".md";
    match += ".bmh";
    fOut = nullptr;
    string fullName;
    for (const auto& topic : fBmhParser.fTopicMap) {
        Definition* topicDef = topic.second;
        if (topicDef->fParent) {
            continue;
        }
        if (!topicDef->isRoot()) {
            return this->reportError<bool>("expected root topic");
        }
        fRoot = topicDef->asRoot();
        if (string::npos == fRoot->fFileName.rfind(match)) {
            continue;
        }
        if (!fOut) {
            fullName = outDir;
            if ('/' != fullName.back()) {
                fullName += '/';
            }
            fullName += filename;
            fOut = fopen(fullName.c_str(), "wb");
            if (!fOut) {
                SkDebugf("could not open output file %s\n", fullName.c_str());
                return false;
            }
            size_t underscorePos = header.find('_');
            if (string::npos != underscorePos) {
                header.replace(underscorePos, 1, " ");
            }
            SkASSERT(string::npos == header.find('_'));
            fprintf(fOut, "%s", header.c_str());
            this->lfAlways(1);
            fprintf(fOut, "===");
        }
        this->markTypeOut(topicDef);
    }
    if (fOut) {
        this->writePending();
        fclose(fOut);
        SkDebugf("wrote %s\n", fullName.c_str());
        fOut = nullptr;
    }
    return true;
}

bool MdOut::checkParamReturnBody(const Definition* def) const {
    TextParser paramBody(def);
    const char* descriptionStart = paramBody.fChar;
    if (!islower(descriptionStart[0])) {
        paramBody.skipToNonAlphaNum();
        string ref = string(descriptionStart, paramBody.fChar - descriptionStart);
        if (!this->isDefined(paramBody, ref, true)) {
            string errorStr = MarkType::kReturn == def->fMarkType ? "return" : "param";
            errorStr += " description must start with lower case";
            paramBody.reportError(errorStr.c_str());
            return false;
        }
    }
    if ('.' == paramBody.fEnd[-1]) {
        paramBody.reportError("make param description a phrase; should not end with period");
        return false;
    }
    return true;
}

void MdOut::childrenOut(const Definition* def, const char* start) {
    const char* end;
    fLineCount = def->fLineCount;
    if (def->isRoot()) {
        fRoot = const_cast<RootDefinition*>(def->asRoot());
    } else if (MarkType::kEnumClass == def->fMarkType) {
        fEnumClass = def;
    }
    BmhParser::Resolvable resolvable = this->resolvable(def->fMarkType);
    for (auto& child : def->fChildren) {
        end = child->fStart;
        if (BmhParser::Resolvable::kNo != resolvable) {
            this->resolveOut(start, end, resolvable);
        }
        this->markTypeOut(child);
        start = child->fTerminator;
    }
    if (BmhParser::Resolvable::kNo != resolvable) {
        end = def->fContentEnd;
        this->resolveOut(start, end, resolvable);
    }
    if (MarkType::kEnumClass == def->fMarkType) {
        fEnumClass = nullptr;
    }
}

const Definition* MdOut::findParamType() {
    SkASSERT(fMethod);
    TextParser parser(fMethod->fFileName, fMethod->fStart, fMethod->fContentStart,
            fMethod->fLineCount);
    string lastFull;
    do {
        parser.skipToAlpha();
        if (parser.eof()) {
            return nullptr;
        }
        const char* word = parser.fChar;
        parser.skipFullName();
        SkASSERT(!parser.eof());
        string name = string(word, parser.fChar - word);
        if (fLastParam->fName == name) {
            const Definition* paramType = this->isDefined(parser, lastFull, false);
            return paramType;
        }
        if (isupper(name[0])) {
            lastFull = name;
        }
    } while (true);
    return nullptr;
}

const Definition* MdOut::isDefined(const TextParser& parser, const string& ref, bool report) const {
    auto rootIter = fBmhParser.fClassMap.find(ref);
    if (rootIter != fBmhParser.fClassMap.end()) {
        return &rootIter->second;
    }
    auto typedefIter = fBmhParser.fTypedefMap.find(ref);
    if (typedefIter != fBmhParser.fTypedefMap.end()) {
        return &typedefIter->second;
    }
    auto enumIter = fBmhParser.fEnumMap.find(ref);
    if (enumIter != fBmhParser.fEnumMap.end()) {
        return &enumIter->second;
    }
    auto constIter = fBmhParser.fConstMap.find(ref);
    if (constIter != fBmhParser.fConstMap.end()) {
        return &constIter->second;
    }
    auto methodIter = fBmhParser.fMethodMap.find(ref);
    if (methodIter != fBmhParser.fMethodMap.end()) {
        return &methodIter->second;
    }
    auto aliasIter = fBmhParser.fAliasMap.find(ref);
    if (aliasIter != fBmhParser.fAliasMap.end()) {
        return aliasIter->second;
    }
    for (const auto& external : fBmhParser.fExternals) {
        if (external.fName == ref) {
            return &external;
        }
    }
    if (fRoot) {
        if (ref == fRoot->fName) {
            return fRoot;
        }
        if (const Definition* definition = fRoot->find(ref, RootDefinition::AllowParens::kYes)) {
            return definition;
        }
        Definition* test = fRoot;
        do {
            if (!test->isRoot()) {
                continue;
            }
            RootDefinition* root = test->asRoot();
            for (auto& leaf : root->fBranches) {
                if (ref == leaf.first) {
                    return leaf.second;
                }
                const Definition* definition = leaf.second->find(ref,
                        RootDefinition::AllowParens::kYes);
                if (definition) {
                    return definition;
                }
            }
            for (string prefix : { "::", "_" } ) {
                string prefixed = root->fName + prefix + ref;
                if (const Definition* definition = root->find(prefixed,
                        RootDefinition::AllowParens::kYes)) {
                    return definition;
                }
                if (isupper(prefixed[0])) {
                    auto topicIter = fBmhParser.fTopicMap.find(prefixed);
                    if (topicIter != fBmhParser.fTopicMap.end()) {
                        return topicIter->second;
                    }
                }
            }
        } while ((test = test->fParent));
    }
    size_t doubleColon = ref.find("::");
    if (string::npos != doubleColon) {
        string className = ref.substr(0, doubleColon);
        auto classIter = fBmhParser.fClassMap.find(className);
        if (classIter != fBmhParser.fClassMap.end()) {
            const RootDefinition& classDef = classIter->second;
            const Definition* result = classDef.find(ref, RootDefinition::AllowParens::kYes);
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
            for (auto const& iter : fBmhParser.fEnumMap) {
                if (iter.second.find(ref, RootDefinition::AllowParens::kYes)) {
                    return &iter.second;
                }
            }
            if (fEnumClass) {
                string fullName = fEnumClass->fName + "::" + ref;
                for (auto child : fEnumClass->fChildren) {
                    if (fullName == child->fName) {
                        return child;
                    }
                }
            }
            if (string::npos != ref.find("_Private")) {
                return nullptr;
            }
        }
        if ('f' == ref[0]) {
            // FIXME : find def associated with prior, e.g.: r.fX where 'SkPoint r' was earlier
                // need to have pushed last resolve on stack to do this
                // for now, just try to make sure that it's there and error if not
            if ('.' != parser.backup(ref.c_str())) {
                parser.reportError("fX member undefined");
                return nullptr;
            }
        } else {
            if (report) {
                parser.reportError("SK undefined");
            }
            return nullptr;
        }
    }
    if (isupper(ref[0])) {
        auto topicIter = fBmhParser.fTopicMap.find(ref);
        if (topicIter != fBmhParser.fTopicMap.end()) {
            return topicIter->second;
        }
        size_t pos = ref.find('_');
        if (string::npos != pos) {
            // see if it is defined by another base class
            string className(ref, 0, pos);
            auto classIter = fBmhParser.fClassMap.find(className);
            if (classIter != fBmhParser.fClassMap.end()) {
                if (const Definition* definition = classIter->second.find(ref,
                        RootDefinition::AllowParens::kYes)) {
                    return definition;
                }
            }
            auto enumIter = fBmhParser.fEnumMap.find(className);
            if (enumIter != fBmhParser.fEnumMap.end()) {
                if (const Definition* definition = enumIter->second.find(ref,
                        RootDefinition::AllowParens::kYes)) {
                    return definition;
                }
            }
            if (report) {
                parser.reportError("_ undefined");
            }
            return nullptr;
        }
    }
    return nullptr;
}

string MdOut::linkName(const Definition* ref) const {
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
string MdOut::linkRef(const string& leadingSpaces, const Definition* def,
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
    const Definition* temp = def;
    do {
        defRoot = temp;
        if (!(temp = temp->fParent)) {
            break;
        }
        classMatch |= temp != defRoot && temp->fName == classPart;
    } while (true);
    string namePart = string::npos != under ? str->substr(under + 1, str->length()) : *str;
    SkASSERT(fRoot);
    SkASSERT(fRoot->fFileName.length());
    if (classMatch) {
        buildup = "#";
        if (*str != classPart && "Sk" == classPart.substr(0, 2)) {
            buildup += classPart + "_";
        }
        buildup += namePart;
    } else {
        string filename = defRoot->asRoot()->fFileName;
        if (filename.substr(filename.length() - 4) == ".bmh") {
            filename = filename.substr(0, filename.length() - 4);
        }
        size_t start = filename.length();
        while (start > 0 && (isalnum(filename[start - 1]) || '_' == filename[start - 1])) {
            --start;
        }
        buildup = filename.substr(start) + "#" + (classMatch ? namePart : *str);
    }
    if (MarkType::kParam == def->fMarkType) {
        const Definition* parent = def->fParent;
        SkASSERT(MarkType::kMethod == parent->fMarkType);
        buildup = '#' + parent->fFiddle + '_' + ref;
    }
    string refOut(ref);
    std::replace(refOut.begin(), refOut.end(), '_', ' ');
    if (ref.length() > 2 && islower(ref[0]) && "()" == ref.substr(ref.length() - 2)) {
        refOut = refOut.substr(0, refOut.length() - 2);
    }
    return leadingSpaces + "<a href=\"" + buildup + "\">" + refOut + "</a>";
}

void MdOut::markTypeOut(Definition* def) {
    string printable = def->printableName();
    const char* textStart = def->fContentStart;
    if (MarkType::kParam != def->fMarkType && MarkType::kConst != def->fMarkType &&
            (!def->fParent || MarkType::kConst != def->fParent->fMarkType) &&
            TableState::kNone != fTableState) {
        this->writePending();
        fprintf(fOut, "</table>");
        this->lf(2);
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
            this->mdHeaderOut(1);
            fprintf(fOut, "<a name=\"%s\"></a> Class %s", this->linkName(def).c_str(),
                    def->fName.c_str());
            this->lf(1);
            break;
        case MarkType::kCode:
            this->lfAlways(2);
            fprintf(fOut, "<pre style=\"padding: 1em 1em 1em 1em;"
                    "width: 44em; background-color: #f0f0f0\">");
            this->lf(1);
            break;
        case MarkType::kColumn:
            this->writePending();
            if (fInList) {
                fprintf(fOut, "    <td>");
            } else {
                fprintf(fOut, "| ");
            }
            break;
        case MarkType::kComment:
            break;
        case MarkType::kConst: {
            if (TableState::kNone == fTableState) {
                this->mdHeaderOut(3);
                fprintf(fOut, "Constants\n"
                        "\n"
                        "<table>");
                fTableState = TableState::kRow;
                this->lf(1);
            }
            if (TableState::kRow == fTableState) {
                this->writePending();
                fprintf(fOut, "  <tr>");
                this->lf(1);
                fTableState = TableState::kColumn;
            }
            this->writePending();
            fprintf(fOut, "    <td><a name=\"%s\"> <code><strong>%s </strong></code> </a></td>",
                    def->fFiddle.c_str(), def->fName.c_str());
            const char* lineEnd = strchr(textStart, '\n');
            SkASSERT(lineEnd < def->fTerminator);
            SkASSERT(lineEnd > textStart);
            SkASSERT((int) (lineEnd - textStart) == lineEnd - textStart);
            fprintf(fOut, "<td>%.*s</td>", (int) (lineEnd - textStart), textStart);
            fprintf(fOut, "<td>");
            textStart = lineEnd;
        } break;
        case MarkType::kDefine:
            break;
        case MarkType::kDefinedBy:
            break;
        case MarkType::kDeprecated:
            break;
        case MarkType::kDescription:
            fInDescription = true;
            this->writePending();
            fprintf(fOut, "<div>");
            break;
        case MarkType::kDoxygen:
            break;
        case MarkType::kEnum:
        case MarkType::kEnumClass:
            this->mdHeaderOut(2);
            fprintf(fOut, "<a name=\"%s\"></a> Enum %s", def->fFiddle.c_str(), def->fName.c_str());
            this->lf(2);
            break;
        case MarkType::kError:
            break;
        case MarkType::kExample: {
            this->mdHeaderOut(3);
            fprintf(fOut, "Example\n"
                            "\n");
            fHasFiddle = true;
            const Definition* platform = def->hasChild(MarkType::kPlatform);
            if (platform) {
                TextParser platParse(platform);
                fHasFiddle = !platParse.strnstr("!fiddle", platParse.fEnd);
            }
            if (fHasFiddle) {
                fprintf(fOut, "<div><fiddle-embed name=\"%s\">", def->fHash.c_str());
            } else {
                fprintf(fOut, "<pre style=\"padding: 1em 1em 1em 1em;"
                        "width: 44em; background-color: #f0f0f0\">");
                this->lf(1);
            }
            } break;
        case MarkType::kExperimental:
            break;
        case MarkType::kExternal:
            break;
        case MarkType::kFile:
            break;
        case MarkType::kFormula:
            break;
        case MarkType::kFunction:
            break;
        case MarkType::kHeight:
            break;
        case MarkType::kImage:
            break;
        case MarkType::kLegend:
            break;
        case MarkType::kLink:
            break;
        case MarkType::kList:
            fInList = true;
            this->lfAlways(2);
            fprintf(fOut, "<table>");
            this->lf(1);
            break;
        case MarkType::kMarkChar:
            fBmhParser.fMC = def->fContentStart[0];
            break;
        case MarkType::kMember: {
            TextParser tp(def->fFileName, def->fStart, def->fContentStart, def->fLineCount);
            tp.skipExact("#Member");
            tp.skipWhiteSpace();
            const char* end = tp.trimmedBracketEnd('\n');
            this->lfAlways(2);
            fprintf(fOut, "<a name=\"%s\"> <code><strong>%.*s</strong></code> </a>",
                    def->fFiddle.c_str(), (int) (end - tp.fChar), tp.fChar);
            this->lf(2);
            } break;
        case MarkType::kMethod: {
            string method_name = def->methodName();
            string formattedStr = def->formatFunction();

            if (!def->isClone()) {
                this->lfAlways(2);
                fprintf(fOut, "<a name=\"%s\"></a>", def->fFiddle.c_str());
                this->mdHeaderOutLF(2, 1);
                fprintf(fOut, "%s", method_name.c_str());
                this->lf(2);
            }

            // TODO: put in css spec that we can define somewhere else (if markup supports that)
            // TODO: 50em below should match limt = 80 in formatFunction()
            this->writePending();
            string preformattedStr = preformat(formattedStr);
            fprintf(fOut, "<pre style=\"padding: 1em 1em 1em 1em;"
                                    "width: 50em; background-color: #f0f0f0\">\n"
                            "%s\n"
                            "</pre>",  preformattedStr.c_str());
            this->lf(2);
            fTableState = TableState::kNone;
            fMethod = def;
            } break;
        case MarkType::kNoExample:
            break;
        case MarkType::kParam: {
            if (TableState::kNone == fTableState) {
                this->mdHeaderOut(3);
                fprintf(fOut,
                        "Parameters\n"
                        "\n"
                        "<table>"
                        );
                this->lf(1);
                fTableState = TableState::kRow;
            }
            if (TableState::kRow == fTableState) {
                fprintf(fOut, "  <tr>");
                this->lf(1);
                fTableState = TableState::kColumn;
            }
            TextParser paramParser(def->fFileName, def->fStart, def->fContentStart,
                    def->fLineCount);
            paramParser.skipWhiteSpace();
            SkASSERT(paramParser.startsWith("#Param"));
            paramParser.next(); // skip hash
            paramParser.skipToNonAlphaNum(); // skip Param
            paramParser.skipSpace();
            const char* paramName = paramParser.fChar;
            paramParser.skipToSpace();
            string paramNameStr(paramName, (int) (paramParser.fChar - paramName));
            if (!this->checkParamReturnBody(def)) {
                return;
            }
            string refNameStr = def->fParent->fFiddle + "_" + paramNameStr;
            fprintf(fOut,
                    "    <td><a name=\"%s\"> <code><strong>%s </strong></code> </a></td> <td>",
                    refNameStr.c_str(), paramNameStr.c_str());
        } break;
        case MarkType::kPlatform:
            break;
        case MarkType::kPrivate:
            break;
        case MarkType::kReturn:
            this->mdHeaderOut(3);
            fprintf(fOut, "Return Value");
            if (!this->checkParamReturnBody(def)) {
                return;
            }
            this->lf(2);
            break;
        case MarkType::kRow:
            if (fInList) {
                fprintf(fOut, "  <tr>");
                this->lf(1);
            }
            break;
        case MarkType::kSeeAlso:
            this->mdHeaderOut(3);
            fprintf(fOut, "See Also");
            this->lf(2);
            break;
        case MarkType::kStdOut: {
            TextParser code(def);
            this->mdHeaderOut(4);
            fprintf(fOut,
                    "Example Output\n"
                    "\n"
                    "~~~~");
            this->lfAlways(1);
            code.skipSpace();
            while (!code.eof()) {
                const char* end = code.trimmedLineEnd();
                fprintf(fOut, "%.*s\n", (int) (end - code.fChar), code.fChar);
                code.skipToLineStart();
            }
            fprintf(fOut, "~~~~");
            this->lf(2);
            } break;
        case MarkType::kStruct:
            fRoot = def->asRoot();
            this->mdHeaderOut(1);
            fprintf(fOut, "<a name=\"%s\"></a> Struct %s", def->fFiddle.c_str(), def->fName.c_str());
            this->lf(1);
            break;
        case MarkType::kSubstitute:
            break;
        case MarkType::kSubtopic:
            this->mdHeaderOut(2);
            fprintf(fOut, "<a name=\"%s\"></a> %s", def->fName.c_str(), printable.c_str());
            this->lf(2);
            break;
        case MarkType::kTable:
            this->lf(2);
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
            this->mdHeaderOut(1);
            fprintf(fOut, "<a name=\"%s\"></a> %s", this->linkName(def).c_str(),
                    printable.c_str());
            this->lf(1);
            break;
        case MarkType::kTrack:
            // don't output children
            return;
        case MarkType::kTypedef:
            break;
        case MarkType::kUnion:
            break;
        case MarkType::kVolatile:
            break;
        case MarkType::kWidth:
            break;
        default:
            SkDebugf("fatal error: MarkType::k%s unhandled in %s()\n",
                    fBmhParser.fMaps[(int) def->fMarkType].fName, __func__);
            SkASSERT(0); // handle everything
            break;
    }
    this->childrenOut(def, textStart);
    switch (def->fMarkType) {  // post child work, at least for tables
        case MarkType::kCode:
            this->writePending();
            fprintf(fOut, "</pre>");
            this->lf(2);
            break;
        case MarkType::kColumn:
            if (fInList) {
                this->writePending();
                fprintf(fOut, "</td>");
                this->lf(1);
            } else {
                fprintf(fOut, " ");
            }
            break;
        case MarkType::kDescription:
            this->writePending();
            fprintf(fOut, "</div>");
            fInDescription = false;
            break;
        case MarkType::kEnum:
        case MarkType::kEnumClass:
            this->lfAlways(2);
            break;
        case MarkType::kExample:
            this->writePending();
            if (fHasFiddle) {
                fprintf(fOut, "</fiddle-embed></div>");
            } else {
                fprintf(fOut, "</pre>");
            }
            this->lf(2);
            break;
        case MarkType::kList:
            fInList = false;
            this->writePending();
            fprintf(fOut, "</table>");
            this->lf(2);
            break;
        case MarkType::kLegend: {
            SkASSERT(def->fChildren.size() == 1);
            const Definition* row = def->fChildren[0];
            SkASSERT(MarkType::kRow == row->fMarkType);
            size_t columnCount = row->fChildren.size();
            SkASSERT(columnCount > 0);
            this->writePending();
            for (size_t index = 0; index < columnCount; ++index) {
                fprintf(fOut, "| --- ");
            }
            fprintf(fOut, " |");
            this->lf(1);
            } break;
        case MarkType::kMethod:
            fMethod = nullptr;
            this->lfAlways(2);
            fprintf(fOut, "---");
            this->lf(2);
            break;
        case MarkType::kConst:
        case MarkType::kParam:
            SkASSERT(TableState::kColumn == fTableState);
            fTableState = TableState::kRow;
            this->writePending();
            fprintf(fOut, "</td>\n");
            fprintf(fOut, "  </tr>");
            this->lf(1);
            break;
        case MarkType::kReturn:
        case MarkType::kSeeAlso:
            this->lf(2);
            break;
        case MarkType::kRow:
            if (fInList) {
                fprintf(fOut, "  </tr>");
            } else {
                fprintf(fOut, "|");
            }
            this->lf(1);
            break;
        case MarkType::kStruct:
            fRoot = fRoot->rootParent();
            break;
        case MarkType::kTable:
            this->lf(2);
            break;
        case MarkType::kPrivate:
            break;
        default:
            break;
    }
}

void MdOut::mdHeaderOutLF(int depth, int lf) {
    this->lfAlways(lf);
    for (int index = 0; index < depth; ++index) {
        fprintf(fOut, "#");
    }
    fprintf(fOut, " ");
}

void MdOut::resolveOut(const char* start, const char* end, BmhParser::Resolvable resolvable) {
    // FIXME: this needs the markdown character present when the def was defined,
    // not the last markdown character the parser would have seen...
    while (fBmhParser.fMC == end[-1]) {
        --end;
    }
    if (start >= end) {
        return;
    }
    string resolved = this->addReferences(start, end, resolvable);
    trim_end_spaces(resolved);
    if (resolved.length()) {
        TextParser paragraph(fFileName, &*resolved.begin(), &*resolved.end(), fLineCount);
        TextParser original(fFileName, start, end, fLineCount);
        while (!original.eof() && '\n' == original.peek()) {
            original.next();
        }
        original.skipSpace();
        while (!paragraph.eof()) {
            paragraph.skipWhiteSpace();
            const char* contentStart = paragraph.fChar;
            paragraph.skipToEndBracket('\n');
            ptrdiff_t lineLength = paragraph.fChar - contentStart;
            if (lineLength) {
                this->writePending();
                fprintf(fOut, "%.*s", (int) lineLength, contentStart);
            }
            int linefeeds = 0;
            while (lineLength > 0 && '\n' == contentStart[--lineLength]) {
                ++linefeeds;
            }
            if (lineLength > 0) {
                this->nl();
            }
            fLinefeeds += linefeeds;
            if (paragraph.eof()) {
                break;
            }
            if ('\n' == paragraph.next()) {
                linefeeds = 1;
                if (!paragraph.eof() && '\n' == paragraph.peek()) {
                    linefeeds = 2;
                }
                this->lf(linefeeds);
            }
        }
#if 0
        while (end > start && end[0] == '\n') {
            fprintf(fOut, "\n");
            --end;
        }
#endif
    }
}
