/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bookmaker.h"

#include "SkOSFile.h"
#include "SkOSPath.h"

#define FPRINTF(...)                \
    if (fDebugOut) {                \
        SkDebugf(__VA_ARGS__);      \
    }                               \
    fprintf(fOut, __VA_ARGS__)

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

static bool all_lower(const string& ref) {
	for (auto ch : ref) {
		if (!islower(ch)) {
			return false;
		}
	}
	return true;
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
            if (!t.eof() && '~' == base[0] && !isalnum(base[1])) {
                t.next();
            } else {
                break;
            }
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
            if (MarkType::kExternal == def->fMarkType) {
                add_ref(leadingSpaces, ref, &result);
                continue;
            }
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
                    if (!(def = this->isDefined(t, fullRef,
                            BmhParser::Resolvable::kOut != resolvable))) {
                        if (!result.size()) {
                            t.reportError("missing method");
                        }
                        return result;
                    }
                    ref = fullRef;
                }
			} else if (BmhParser::Resolvable::kClone != resolvable &&
					all_lower(ref) && (t.eof() || '(' != t.peek())) {
				add_ref(leadingSpaces, ref, &result);
				continue;
			}
			result += linkRef(leadingSpaces, def, ref, resolvable);
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
				result += linkRef(leadingSpaces, def, ref, resolvable);
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
				result += linkRef(leadingSpaces, def, ref, resolvable);
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
                            if (paramType->hasMatch(fullName)) {
								result += linkRef(leadingSpaces, paramType, ref, resolvable);
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
			result += linkRef(leadingSpaces, topicIter->second, ref, resolvable);
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
        const Definition* test = fRoot;
        do {
            if (!test->isRoot()) {
                continue;
            }
            for (string prefix : { "_", "::" } ) {
                const RootDefinition* root = test->asRoot();
                string prefixed = root->fName + prefix + ref;
                if (const Definition* def = root->find(prefixed,
                        RootDefinition::AllowParens::kYes)) {
					result += linkRef(leadingSpaces, def, ref, resolvable);
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



bool MdOut::buildReferences(const char* docDir, const char* mdFileOrPath) {
    if (!sk_isdir(mdFileOrPath)) {
        SkString mdFile = SkOSPath::Basename(mdFileOrPath);
        SkString bmhFile = SkOSPath::Join(docDir, mdFile.c_str());
        bmhFile.remove(bmhFile.size() - 3, 3);
        bmhFile += ".bmh";
        SkString mdPath = SkOSPath::Dirname(mdFileOrPath);
        if (!this->buildRefFromFile(bmhFile.c_str(), mdPath.c_str())) {
            SkDebugf("failed to parse %s\n", mdFileOrPath);
            return false;
        }
    } else {
        SkOSFile::Iter it(docDir, ".bmh");
        for (SkString file; it.next(&file); ) {
            SkString p = SkOSPath::Join(docDir, file.c_str());
            if (!this->buildRefFromFile(p.c_str(), mdFileOrPath)) {
                SkDebugf("failed to parse %s\n", p.c_str());
                return false;
            }
        }
    }
    return true;
}

bool MdOut::buildStatus(const char* statusFile, const char* outDir) {
    StatusIter iter(statusFile, ".bmh", StatusFilter::kInProgress);
    for (string file; iter.next(&file); ) {
        SkString p = SkOSPath::Join(iter.baseDir().c_str(), file.c_str());
        const char* hunk = p.c_str();
        if (!this->buildRefFromFile(hunk, outDir)) {
            SkDebugf("failed to parse %s\n", hunk);
            return false;
        }
    }
    return true;
}

bool MdOut::buildRefFromFile(const char* name, const char* outDir) {
    if (!SkStrEndsWith(name, ".bmh")) {
        return true;
    }
    if (SkStrEndsWith(name, "markup.bmh")) {  // don't look inside this for now
        return true;
    }
    if (SkStrEndsWith(name, "illustrations.bmh")) {  // don't look inside this for now
        return true;
    }
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

    vector<string> keys;
    keys.reserve(fBmhParser.fTopicMap.size());
    for (const auto& it : fBmhParser.fTopicMap) {
        keys.push_back(it.first);
    }
    std::sort(keys.begin(), keys.end());
    for (auto key : keys) {
        string s(key);
        auto topicDef = fBmhParser.fTopicMap.at(s);
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
            fOut = fopen(filename.c_str(), "wb");
            if (!fOut) {
                SkDebugf("could not open output file %s\n", fullName.c_str());
                return false;
            }
            size_t underscorePos = header.find('_');
            if (string::npos != underscorePos) {
                header.replace(underscorePos, 1, " ");
            }
            SkASSERT(string::npos == header.find('_'));
            FPRINTF("%s", header.c_str());
            this->lfAlways(1);
            FPRINTF("===");
        }
        fPopulators.clear();
        fPopulators[kClassesAndStructs].fDescription = "embedded struct and class members";
        fPopulators[kConstants].fDescription = "enum and enum class, const values";
        fPopulators[kConstructors].fDescription = "functions that construct";
        fPopulators[kMemberFunctions].fDescription = "static functions and member methods";
        fPopulators[kMembers].fDescription = "member values";
        fPopulators[kOperators].fDescription = "operator overloading methods";
        fPopulators[kRelatedFunctions].fDescription = "similar methods grouped together";
        fPopulators[kSubtopics].fDescription = "";
        this->populateTables(fRoot);
        this->markTypeOut(topicDef);
    }
    if (fOut) {
        this->writePending();
        fclose(fOut);
        fflush(fOut);
        if (this->writtenFileDiffers(filename, fullName)) {
            fOut = fopen(fullName.c_str(), "wb");
            int writtenSize;
            const char* written = ReadToBuffer(filename, &writtenSize);
            fwrite(written, 1, writtenSize, fOut);
            fclose(fOut);
            fflush(fOut);
            SkDebugf("wrote updated %s\n", fullName.c_str());
        }
        remove(filename.c_str());
        fOut = nullptr;
    }
    return true;
}

bool MdOut::checkParamReturnBody(const Definition* def) const {
    TextParser paramBody(def);
    const char* descriptionStart = paramBody.fChar;
    if (!islower(descriptionStart[0]) && !isdigit(descriptionStart[0])) {
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
    BmhParser::Resolvable resolvable = this->resolvable(def);
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

const Definition* MdOut::csParent() const {
    const Definition* csParent = fRoot->csParent();
    if (!csParent) {
        const Definition* topic = fRoot;
        while (topic && MarkType::kTopic != topic->fMarkType) {
            topic = topic->fParent;
        }
        for (auto child : topic->fChildren) {
            if (child->isStructOrClass() || MarkType::kTypedef == child->fMarkType) {
                csParent = child;
                break;
            }
        }
        SkASSERT(csParent || string::npos == fRoot->fFileName.find("Sk"));
    }
    return csParent;
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
        const Definition* test = fRoot;
        do {
            if (!test->isRoot()) {
                continue;
            }
            const RootDefinition* root = test->asRoot();
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
            string fiddlePrefixed = root->fFiddle + "_" + ref;
            auto topicIter = fBmhParser.fTopicMap.find(fiddlePrefixed);
            if (topicIter != fBmhParser.fTopicMap.end()) {
                return topicIter->second;
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
                auto def = iter.second.find(ref, RootDefinition::AllowParens::kYes);
                if (def) {
                    return def;
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
        const string& ref, BmhParser::Resolvable resolvable) const {
    string buildup;
    string refName;
    const string* str = &def->fFiddle;
    SkASSERT(str->length() > 0);
    string classPart = *str;
    bool globalEnumMember = false;
    if (MarkType::kAlias == def->fMarkType) {
        def = def->fParent;
        SkASSERT(def);
        SkASSERT(MarkType::kSubtopic == def->fMarkType ||MarkType::kTopic == def->fMarkType);
    }
    if (MarkType::kSubtopic == def->fMarkType) {
        const Definition* topic = def->topicParent();
        SkASSERT(topic);
        classPart = topic->fName;
        refName = def->fName;
    } else if (MarkType::kTopic == def->fMarkType) {
        refName = def->fName;
    } else {
        if ('k' == (*str)[0] && string::npos != str->find("_Sk")) {
            globalEnumMember = true;
        } else {
            SkASSERT("Sk" == str->substr(0, 2) || "SK" == str->substr(0, 2)
                    // FIXME: kitchen sink catch below, need to do better
                    || string::npos != def->fFileName.find("undocumented"));
            size_t under = str->find('_');
            classPart = string::npos != under ? str->substr(0, under) : *str;
        }
        refName = def->fFiddle;
    }
    bool classMatch = fRoot->fFileName == def->fFileName;
    SkASSERT(fRoot);
    SkASSERT(fRoot->fFileName.length());
    if (!classMatch) {
        string filename = def->fFileName;
        if (filename.substr(filename.length() - 4) == ".bmh") {
            filename = filename.substr(0, filename.length() - 4);
        }
        size_t start = filename.length();
        while (start > 0 && (isalnum(filename[start - 1]) || '_' == filename[start - 1])) {
            --start;
        }
        buildup = filename.substr(start);
    }
    buildup += "#" + refName;
    if (MarkType::kParam == def->fMarkType) {
        const Definition* parent = def->fParent;
        SkASSERT(MarkType::kMethod == parent->fMarkType);
        buildup = '#' + parent->fFiddle + '_' + ref;
    }
    string refOut(ref);
    if (!globalEnumMember) {
        std::replace(refOut.begin(), refOut.end(), '_', ' ');
    }
    if (ref.length() > 2 && islower(ref[0]) && "()" == ref.substr(ref.length() - 2)) {
        refOut = refOut.substr(0, refOut.length() - 2);
    }
    string result = leadingSpaces + "<a href=\"" + buildup + "\">" + refOut + "</a>";
	if (BmhParser::Resolvable::kClone == resolvable && MarkType::kMethod == def->fMarkType &&
			def->fCloned && !def->fClone) {
		bool found = false;
		string match = def->fName;
		if ("()" == match.substr(match.length() - 2)) {
			match = match.substr(0, match.length() - 2);
		}
		match += '_';
		auto classIter = fBmhParser.fClassMap.find(classPart);
		if (fBmhParser.fClassMap.end() != classIter) {
			for (char num = '2'; num <= '9'; ++num) {
				string clone = match + num;
				const auto& leafIter = classIter->second.fLeaves.find(clone);
				if (leafIter != classIter->second.fLeaves.end()) {
					result += "<sup><a href=\"" + buildup + "_" + num + "\">[" + num + "]</a></sup>";
					found = true;
				}
			}
		}
		if (!found) {
			SkDebugf("");  // convenient place to set a breakpoint
		}
	}
	return result;
}

void MdOut::markTypeOut(Definition* def) {
    string printable = def->printableName();
    const char* textStart = def->fContentStart;
    if (MarkType::kParam != def->fMarkType && MarkType::kConst != def->fMarkType &&
            (!def->fParent || MarkType::kConst != def->fParent->fMarkType) &&
            TableState::kNone != fTableState) {
        this->writePending();
        FPRINTF("</table>");
        this->lf(2);
        fTableState = TableState::kNone;
    }
    switch (def->fMarkType) {
        case MarkType::kAlias:
            break;
        case MarkType::kAnchor: {
            if (fColumn > 0) {
                this->writeSpace();
            }
            this->writePending();
            TextParser parser(def);
            const char* start = parser.fChar;
            parser.skipToEndBracket(" # ");
            string anchorText(start, parser.fChar - start);
            parser.skipExact(" # ");
            string anchorLink(parser.fChar, parser.fEnd - parser.fChar);
            FPRINTF("<a href=\"%s\">%s", anchorLink.c_str(), anchorText.c_str());
            } break;
        case MarkType::kBug:
            break;
        case MarkType::kClass:
            this->mdHeaderOut(1);
            FPRINTF("<a name=\"%s\"></a> Class %s", this->linkName(def).c_str(),
                    def->fName.c_str());
            this->lf(1);
            break;
        case MarkType::kCode:
            this->lfAlways(2);
            FPRINTF("<pre style=\"padding: 1em 1em 1em 1em;"
                    "width: 62.5em; background-color: #f0f0f0\">");
            this->lf(1);
            break;
        case MarkType::kColumn:
            this->writePending();
            if (fInList) {
                FPRINTF("    <td>");
            } else {
                FPRINTF("| ");
            }
            break;
        case MarkType::kComment:
            break;
        case MarkType::kConst: {
            if (TableState::kNone == fTableState) {
                this->mdHeaderOut(3);
                FPRINTF("Constants\n"
                        "\n"
                        "<table>");
                fTableState = TableState::kRow;
                this->lf(1);
            }
            if (TableState::kRow == fTableState) {
                this->writePending();
                FPRINTF("  <tr>");
                this->lf(1);
                fTableState = TableState::kColumn;
            }
            this->writePending();
            FPRINTF("    <td><a name=\"%s\"> <code><strong>%s </strong></code> </a></td>",
                    def->fFiddle.c_str(), def->fName.c_str());
            const char* lineEnd = strchr(textStart, '\n');
            SkASSERT(lineEnd < def->fTerminator);
            SkASSERT(lineEnd > textStart);
            SkASSERT((int) (lineEnd - textStart) == lineEnd - textStart);
            FPRINTF("<td>%.*s</td>", (int) (lineEnd - textStart), textStart);
            FPRINTF("<td>");
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
            FPRINTF("<div>");
            break;
        case MarkType::kDoxygen:
            break;
        case MarkType::kDuration:
            break;
        case MarkType::kEnum:
        case MarkType::kEnumClass:
            this->mdHeaderOut(2);
            FPRINTF("<a name=\"%s\"></a> Enum %s", def->fFiddle.c_str(), def->fName.c_str());
            this->lf(2);
            break;
        case MarkType::kExample: {
            this->mdHeaderOut(3);
            FPRINTF("Example\n"
                            "\n");
            fHasFiddle = true;
            bool showGpu = false;
            bool gpuAndCpu = false;
            const Definition* platform = def->hasChild(MarkType::kPlatform);
            if (platform) {
                TextParser platParse(platform);
                fHasFiddle = !platParse.strnstr("!fiddle", platParse.fEnd);
                showGpu = platParse.strnstr("gpu", platParse.fEnd);
                if (showGpu) {
                    gpuAndCpu = platParse.strnstr("cpu", platParse.fEnd);
                }
            }
            if (fHasFiddle) {
                SkASSERT(def->fHash.length() > 0);
                FPRINTF("<div><fiddle-embed name=\"%s\"", def->fHash.c_str());
                if (showGpu) {
                    FPRINTF(" gpu=\"true\"");
                    if (gpuAndCpu) {
                        FPRINTF(" cpu=\"true\"");
                    }
                }
                FPRINTF(">");
            } else {
                SkASSERT(def->fHash.length() == 0);
                FPRINTF("<pre style=\"padding: 1em 1em 1em 1em; font-size: 13px"
                        " width: 62.5em; background-color: #f0f0f0\">");
                this->lfAlways(1);
                if (def->fWrapper.length() > 0) {
                    FPRINTF("%s", def->fWrapper.c_str());
                }
                fRespectLeadingSpace = true;
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
        case MarkType::kIllustration: {
            string illustName = "Illustrations_" + def->fParent->fFiddle;
            auto illustIter = fBmhParser.fTopicMap.find(illustName);
            SkASSERT(fBmhParser.fTopicMap.end() != illustIter);
            Definition* illustDef = illustIter->second;
            SkASSERT(MarkType::kSubtopic == illustDef->fMarkType);
            SkASSERT(1 == illustDef->fChildren.size());
            Definition* illustExample = illustDef->fChildren[0];
            SkASSERT(MarkType::kExample == illustExample->fMarkType);
            string hash = illustExample->fHash;
            SkASSERT("" != hash);
            string title;
            this->writePending();
            FPRINTF("![%s](https://fiddle.skia.org/i/%s_raster.png \"%s\")",
                    def->fName.c_str(), hash.c_str(), title.c_str());
            this->lf(2);
        } break;
        case MarkType::kImage:
            break;
        case MarkType::kIn:
            break;
        case MarkType::kLegend:
            break;
        case MarkType::kLine:
            break;
        case MarkType::kLink:
            break;
        case MarkType::kList:
            fInList = true;
            this->lfAlways(2);
            FPRINTF("<table>");
            this->lf(1);
            break;
        case MarkType::kLiteral:
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
            FPRINTF("<a name=\"%s\"> <code><strong>%.*s</strong></code> </a>",
                    def->fFiddle.c_str(), (int) (end - tp.fChar), tp.fChar);
            this->lf(2);
            } break;
        case MarkType::kMethod: {
            string method_name = def->methodName();
            string formattedStr = def->formatFunction(Definition::Format::kIncludeReturn);

			this->lfAlways(2);
			FPRINTF("<a name=\"%s\"></a>", def->fFiddle.c_str());
			if (!def->isClone()) {
                this->mdHeaderOutLF(2, 1);
                FPRINTF("%s", method_name.c_str());
			}
			this->lf(2);

            // TODO: put in css spec that we can define somewhere else (if markup supports that)
            // TODO: 50em below should match limit = 80 in formatFunction()
            this->writePending();
            string preformattedStr = preformat(formattedStr);
            FPRINTF("<pre style=\"padding: 1em 1em 1em 1em;"
                                    "width: 62.5em; background-color: #f0f0f0\">\n"
                            "%s\n"
                            "</pre>",  preformattedStr.c_str());
            this->lf(2);
            fTableState = TableState::kNone;
            fMethod = def;
            } break;
        case MarkType::kNoExample:
            break;
        case MarkType::kOutdent:
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
                FPRINTF("  <tr>");
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
        case MarkType::kPopulate: {
            SkASSERT(MarkType::kSubtopic == def->fParent->fMarkType);
            string name = def->fParent->fName;
            if (kSubtopics == name) {
                this->subtopicsOut();
            } else {
                this->subtopicOut(this->populator(name.c_str()));
            }
            } break;
        case MarkType::kPrivate:
            break;
        case MarkType::kReturn:
            this->mdHeaderOut(3);
            FPRINTF("Return Value");
            if (!this->checkParamReturnBody(def)) {
                return;
            }
            this->lf(2);
            break;
        case MarkType::kRow:
            if (fInList) {
                FPRINTF("  <tr>");
                this->lf(1);
            }
            break;
        case MarkType::kSeeAlso:
            this->mdHeaderOut(3);
            FPRINTF("See Also");
            this->lf(2);
            break;
        case MarkType::kSet:
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
                FPRINTF("%.*s\n", (int) (end - code.fChar), code.fChar);
                code.skipToLineStart();
            }
            FPRINTF("~~~~");
            this->lf(2);
            } break;
        case MarkType::kStruct:
            fRoot = def->asRoot();
            this->mdHeaderOut(1);
            FPRINTF("<a name=\"%s\"></a> Struct %s", def->fFiddle.c_str(), def->fName.c_str());
            this->lf(1);
            break;
        case MarkType::kSubstitute:
            break;
        case MarkType::kSubtopic:
            this->mdHeaderOut(2);
            FPRINTF("<a name=\"%s\"></a> %s", def->fName.c_str(), printable.c_str());
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
            FPRINTF("<a name=\"%s\"></a> %s", this->linkName(def).c_str(),
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
        case MarkType::kAnchor:
            if (fColumn > 0) {
                this->writeSpace();
            }
            break;
        case MarkType::kCode:
            this->writePending();
            FPRINTF("</pre>");
            this->lf(2);
            break;
        case MarkType::kColumn:
            if (fInList) {
                this->writePending();
                FPRINTF("</td>");
                this->lf(1);
            } else {
                FPRINTF(" ");
            }
            break;
        case MarkType::kDescription:
            this->writePending();
            FPRINTF("</div>");
            fInDescription = false;
            break;
        case MarkType::kEnum:
        case MarkType::kEnumClass:
            this->lfAlways(2);
            break;
        case MarkType::kExample:
            this->writePending();
            if (fHasFiddle) {
                FPRINTF("</fiddle-embed></div>");
            } else {
                this->lfAlways(1);
                if (def->fWrapper.length() > 0) {
                    FPRINTF("}");
                    this->lfAlways(1);
                }
                FPRINTF("</pre>");
            }
            this->lf(2);
            fRespectLeadingSpace = false;
            break;
        case MarkType::kLink:
            this->writeString("</a>");
            this->writeSpace();
            break;
        case MarkType::kList:
            fInList = false;
            this->writePending();
            FPRINTF("</table>");
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
                FPRINTF("| --- ");
            }
            FPRINTF(" |");
            this->lf(1);
            } break;
        case MarkType::kMethod:
            fMethod = nullptr;
            this->lfAlways(2);
            FPRINTF("---");
            this->lf(2);
            break;
        case MarkType::kConst:
        case MarkType::kParam:
            SkASSERT(TableState::kColumn == fTableState);
            fTableState = TableState::kRow;
            this->writePending();
            FPRINTF("</td>\n");
            FPRINTF("  </tr>");
            this->lf(1);
            break;
        case MarkType::kReturn:
        case MarkType::kSeeAlso:
            this->lf(2);
            break;
        case MarkType::kRow:
            if (fInList) {
                FPRINTF("  </tr>");
            } else {
                FPRINTF("|");
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
        FPRINTF("#");
    }
    FPRINTF(" ");
}

void MdOut::populateTables(const Definition* def) {
    const Definition* csParent = this->csParent();
    if (!csParent) {
        return;
    }
    for (auto child : def->fChildren) {
        if (MarkType::kTopic == child->fMarkType || MarkType::kSubtopic == child->fMarkType) {
            string name = child->fName;
            bool builtInTopic = name == kClassesAndStructs || name == kConstants
                    || name == kConstructors || name == kMemberFunctions || name == kMembers
                    || name == kOperators || name == kOverview || name == kRelatedFunctions
                    || name == kSubtopics;
            if (!builtInTopic && child->fName != kOverview) {
                this->populator(kRelatedFunctions).fMembers.push_back(child);
            }
            this->populateTables(child);
            continue;
        }
        if (child->isStructOrClass()) {
            if (fClassStack.size() > 0) {
                this->populator(kClassesAndStructs).fMembers.push_back(child);
            }
            fClassStack.push_back(child);
            this->populateTables(child);
            fClassStack.pop_back();
            continue;
        }
        if (MarkType::kEnum == child->fMarkType || MarkType::kEnumClass == child->fMarkType) {
            this->populator(kConstants).fMembers.push_back(child);
            continue;
        }
        if (MarkType::kMember == child->fMarkType) {
            this->populator(kMembers).fMembers.push_back(child);
            continue;
        }
        if (MarkType::kMethod != child->fMarkType) {
            continue;
        }
        if (child->fClone) {
            continue;
        }
        if (Definition::MethodType::kConstructor == child->fMethodType
                || Definition::MethodType::kDestructor == child->fMethodType) {
            this->populator(kConstructors).fMembers.push_back(child);
            continue;
        }
        if (Definition::MethodType::kOperator == child->fMethodType) {
            this->populator(kOperators).fMembers.push_back(child);
            continue;
        }
        this->populator(kMemberFunctions).fMembers.push_back(child);
        if (csParent && (0 == child->fName.find(csParent->fName + "::Make")
                || 0 == child->fName.find(csParent->fName + "::make"))) {
            this->populator(kConstructors).fMembers.push_back(child);
            continue;
        }
        for (auto item : child->fChildren) {
            if (MarkType::kIn == item->fMarkType) {
                string name(item->fContentStart, item->fContentEnd - item->fContentStart);
                fPopulators[name].fMembers.push_back(child);
                fPopulators[name].fShowClones = true;
                break;
            }
        }
    }
}

void MdOut::resolveOut(const char* start, const char* end, BmhParser::Resolvable resolvable) {
    if ((BmhParser::Resolvable::kLiteral == resolvable || fRespectLeadingSpace) && end > start) {
        while ('\n' == *start) {
            ++start;
        }
        const char* spaceStart = start;
        while (' ' == *start) {
            ++start;
        }
        if (start > spaceStart) {
            fIndent = start - spaceStart;
        }
        this->writeBlockTrim(end - start, start);
        if ('\n' == end[-1]) {
            this->lf(1);
        }
        fIndent = 0;
        return;
    }
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
                while (lineLength && contentStart[lineLength - 1] <= ' ') {
                    --lineLength;
                }
                string str(contentStart, lineLength);
                this->writeString(str.c_str());
            }
#if 0
            int linefeeds = 0;
            while (lineLength > 0 && '\n' == contentStart[--lineLength]) {

                ++linefeeds;
            }
            if (lineLength > 0) {
                this->nl();
            }
            fLinefeeds += linefeeds;
#endif
            if (paragraph.eof()) {
                break;
            }
            if ('\n' == paragraph.next()) {
                int linefeeds = 1;
                if (!paragraph.eof() && '\n' == paragraph.peek()) {
                    linefeeds = 2;
                }
                this->lf(linefeeds);
            }
        }
#if 0
        while (end > start && end[0] == '\n') {
            FPRINTF("\n");
            --end;
        }
#endif
    }
}

void MdOut::rowOut(const char* name, const string& description) {
    this->lfAlways(1);
    FPRINTF("| ");
    this->resolveOut(name, name + strlen(name), BmhParser::Resolvable::kYes);
    FPRINTF(" | ");
    this->resolveOut(&description.front(), &description.back() + 1, BmhParser::Resolvable::kYes);
    FPRINTF(" |");
    this->lf(1);
}

void MdOut::subtopicsOut() {
    const Definition* csParent = this->csParent();
    SkASSERT(csParent);
    this->rowOut("name", "description");
    this->rowOut("---", "---");
    for (auto item : { kClassesAndStructs, kConstants, kConstructors, kMemberFunctions,
            kMembers, kOperators, kRelatedFunctions } ) {
        for (auto entry : this->populator(item).fMembers) {
            if (entry->csParent() == csParent) {
                string description = fPopulators.find(item)->second.fDescription;
                if (kConstructors == item) {
                    description += " " + csParent->fName;
                }
                this->rowOut(item, description);
                break;
            }
        }
    }
}

void MdOut::subtopicOut(const TableContents& tableContents) {
    const auto& data = tableContents.fMembers;
    const Definition* csParent = this->csParent();
    SkASSERT(csParent);
    fRoot = csParent->asRoot();
    this->rowOut("name", "description");
    this->rowOut("---", "---");
    std::map<string, const Definition*> items;
    for (auto entry : data) {
        if (entry->csParent() != csParent) {
            continue;
        }
        size_t start = entry->fName.find_last_of("::");
        string name = entry->fName.substr(string::npos == start ? 0 : start + 1);
        items[name] = entry;
    }
    for (auto entry : items) {
        if (entry.second->fDeprecated) {
            continue;
        }
        const Definition* oneLiner = nullptr;
        for (auto child : entry.second->fChildren) {
            if (MarkType::kLine == child->fMarkType) {
                oneLiner = child;
                break;
            }
        }
        if (!oneLiner) {
            SkDebugf(""); // convenient place to set a breakpoint
        }
        // TODO: detect this earlier? throw error here?
        SkASSERT(oneLiner);
        this->rowOut(entry.first.c_str(), string(oneLiner->fContentStart,
            oneLiner->fContentEnd - oneLiner->fContentStart));
        if (tableContents.fShowClones && entry.second->fCloned) {
            int cloneNo = 2;
            string builder = entry.second->fName;
            if ("()" == builder.substr(builder.length() - 2)) {
                builder = builder.substr(0, builder.length() - 2);
            }
            builder += '_';
            this->rowOut("",
                    preformat(entry.second->formatFunction(Definition::Format::kOmitReturn)));
            do {
                string match = builder + to_string(cloneNo);
                auto child = csParent->findClone(match);
                if (!child) {
                    break;
                }
                this->rowOut("", preformat(child->formatFunction(Definition::Format::kOmitReturn)));
            } while (++cloneNo);
        }
    }
}
