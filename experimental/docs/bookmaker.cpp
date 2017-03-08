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
            } else {
                SkDebugf("");
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
        if (len > 0 && '\r' == text[len - 1]) {
            --len;
        }
        if (pos + nextIndent < len) {
            example += '"';
            size_t indent = outIndent + nextIndent;
            SkASSERT(indent >= textIndent);
            indent -= textIndent;
            for (size_t index = 0; index < indent; ++index) {
                example += ' ';
            }
            pos += nextIndent;
            while (pos < len) {
                example += '"' == text[pos] ? "\\\"" :
                    '\\' == text[pos] ? "\\\\" : 
                    text.substr(pos, 1);
                ++pos;
            }
            example += "\\n\"";
            if ('\r' == text[pos]) {
                ++pos;
            }
        } else {
            pos += nextIndent;
            addToString = false;
        }
        if ('\n' == text[pos]) {
            ++pos;
        }
    } while (pos < end);
}

bool Definition::exampleToScript(string* result) const {
    string text = this->extractText(Definition::TrimExtract::kNo);
    const char drawWrapper[] = "void draw(SkCanvas* canvas) {";
    int nonSpace = 0;
    while (nonSpace < text.length() && ' ' >= text[nonSpace]) {
        ++nonSpace;
    }
    bool hasFunc = !text.compare(nonSpace, sizeof(drawWrapper) - 1, drawWrapper);
    bool textOnly = string::npos != text.find("SkDebugf(");  // path->dump() also prints text
    string textOnlyStr = textOnly ? "true" : "false";
    string widthStr = "256";
    bool drawOnly = string::npos != text.find("canvas->draw");  // tho canvas->clear() also draws
    bool preprocessor = text[0] == '#';
    if (textOnly && drawOnly) {
        return false;
    }
    string normalizedName(fName);
    std::replace(normalizedName.begin(), normalizedName.end(), '-', '_');
    string example = "var " + normalizedName + "_code = \n";
    for (auto const& iter : fChildren) {
        switch (iter->fMarkType) {
            case MarkType::kError:
                result->clear();
                return true;
            case MarkType::kWidth:
                widthStr = string(iter->fContentStart, iter->fContentEnd - iter->fContentStart - 1);
                break;
            case MarkType::kDescription:
                // ignore for now
                break;
            case MarkType::kFunction: {
                // emit this, but don't wrap this in draw()
                string funcText(iter->fContentStart, iter->fContentEnd - iter->fContentStart - 1);
                int pos = 0;
                while (pos < text.length() && ' ' > text[pos]) {
                    ++pos;
                }
                size_t indent = count_indent(text, pos, funcText.length());
                add_code(funcText, pos, funcText.length(), 0, indent, example);
                example += " +\n";
                example += "\"\\n\" +\n";
                } break;
            case MarkType::kImage:
                // ignore for now
                break;
            case MarkType::kToDo:
                break;
            case MarkType::kPlatform:
                // ignore for now
                break;
            case MarkType::kStdOut:
                // ignore for now
                break;
            default:
                SkASSERT(0);  // more coding to do
        }
    }
    size_t pos = 0;
    while (pos < text.length() && ' ' > text[pos]) {
        ++pos;
    }
    size_t end = text.length();
    size_t outIndent = 0;
    size_t textIndent = count_indent(text, pos, end);
    if (!hasFunc && !preprocessor) {
        example += "\"void draw(SkCanvas* canvas) {\\n\" + \n";
        outIndent = 4;
    }
    add_code(text, pos, end, outIndent, textIndent, example);
    if (!hasFunc && !preprocessor) {
        if ('"' == example.back()) {
            example += " +\n";
        } else {
            SkDebugf("");
        }
        example += "\"}\\n\"";
    }
    example += ";\n\nvar " + normalizedName + "_json = {\n";
    example += "    \"code\": " + normalizedName + "_code,\n";
    example += "    \"options\": {\n";
    example += "        \"width\": " + widthStr + ",\n";
    example += "        \"height\": 256,\n";
    example += "        \"source\": 0,\n";
    example += "        \"textOnly\": " + textOnlyStr + "\n";
    example += "    },\n";
    example += "    \"name\": \"" + fName + "\",\n";
    example += "    \"overwrite\": true\n";
    example += "}\n\n";
    example += "runFiddle(" + normalizedName + "_json);\n";
    *result = example;
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
			if (!this->parseFromFile(hunk)) {
				SkDebugf("failed to parse %s\n", hunk);
				return false;
			}
		}
	}
	return true;
}

/* 
  class contains named struct, enum, enum-member, method, topic, subtopic
     everything contained by class is uniquely named
     contained names may be reused by other classes
  method contains named parameters
     parameters may be reused in other methods
 */

bool BmhParser::isDefined(const string& ref) const {
    auto rootIter = fClassMap.find(ref);
    if (rootIter != fClassMap.end()) {
        return true;
    }
    auto typedefIter = fTypedefMap.find(ref);
    if (typedefIter != fTypedefMap.end()) {
        return true;
    }
    auto enumIter = fEnumMap.find(ref);
    if (enumIter != fEnumMap.end()) {
        return true;
    }
    auto constIter = fConstMap.find(ref);
    if (constIter != fConstMap.end()) {
        return true;
    }
    auto methodIter = fMethodMap.find(ref);
    if (methodIter != fMethodMap.end()) {
        return true;
    }
    auto externalIter = std::find(fExternals.begin(), fExternals.end(), ref);
    if (externalIter != fExternals.end()) {
        return true;
    }
    auto iter = fRoot->fChildDefinitions.find(ref);
    if (iter != fRoot->fChildDefinitions.end()) {
        return true;
    }
    if (!ref.compare(0, 2, "SK") || !ref.compare(0, 3, "sk_")
            || (('k' == ref[0] || 'g' == ref[0] || 'f' == ref[0]) &&
                ref.length() > 1 && isupper(ref[1]))) {
        // try with a prefix
        string prefixed = fRoot->fName + "_" + ref;
        auto iter = fRoot->fChildDefinitions.find(prefixed);
        if (iter != fRoot->fChildDefinitions.end()) {
            return &iter->second;
        }
        if ('k' == ref[0]) {
            for (auto const& iter : fEnumMap) {
                auto childIter = iter.second.fChildDefinitions.find(ref);
                if (childIter != iter.second.fChildDefinitions.end()) {
                    return &childIter->second;
                }
            }
        }
        SkDebugf("%s\n", ref.c_str());
        return false;
    } 
    if (isupper(ref[0])) {
        size_t pos = ref.find('_');
        if (string::npos != pos) {
            auto topicIter = fTopicMap.find(ref);
            if (topicIter != fTopicMap.end()) {
                return topicIter->second;
            }
            // see if it is defined by another base class
            string className(ref, 0, pos);
            auto classIter = fClassMap.find(className);
            if (classIter != fClassMap.end()) {
                auto iter = classIter->second.fChildDefinitions.find(ref);
                if (iter != classIter->second.fChildDefinitions.end()) {
                    return &iter->second;
                }
            }
            auto enumIter = fEnumMap.find(className);
            if (enumIter != fEnumMap.end()) {
                auto iter = enumIter->second.fChildDefinitions.find(ref);
                if (iter != enumIter->second.fChildDefinitions.end()) {
                    return &iter->second;
                }
            }
            SkDebugf("%s\n", ref.c_str());
            return false;
        }
    }
    return false;
}

// for now, hard-code to markdown
static void add_link(const string& ref, string* result) {
    if (result->length() > 0 && ' ' < result->back()) {
        *result += ' ';
    }
    *result += "[" + ref + "]";
}

static void add_ref(const string& ref, string* result) {
    if (result->length() > 0 && ' ' < result->back()) {
        *result += ' ';
    }
    *result += ref;
}

string BmhParser::addReferences(const char* refStart, const char* refEnd) {
    string result;
    TextParser t(refStart, refEnd);
    do {
        const char* base = t.fChar;
        t.skipWhiteSpace();
        const char* wordStart = t.fChar;
        t.skipToAlpha();
        const char* start = t.fChar;
        if (wordStart < start) {
            add_ref(string(wordStart, start - wordStart), &result);
        }
        t.skipToNonAlphaNum();
        if (base == t.fChar) {
            break;
        }
        if (start < t.fChar) {
            const string ref(start, t.fChar - start);
            if ("SkPaint" == ref) {
                SkDebugf("");
            }
            if (this->isDefined(ref)) {
                add_link(ref, &result);
                continue;
            }
    // class, struct, and enum start with capitals
    // methods may start with upper (static) or lower (most)

            // see if this should have been a findable reference
                 
                // look for Sk / sk / SK ..
            if (!ref.compare(0, 2, "Sk")) {
                SkDebugf("%s\n", ref.c_str());
                add_link(ref, &result);
                continue;
            }
            if (!ref.compare(0, 2, "SK")) {
                SkDebugf("%s\n", ref.c_str());
                add_link(ref, &result);
                continue;
            }
            if (isupper(start[0])) {
                auto topicIter = fTopicMap.find(ref);
                if (topicIter != fTopicMap.end()) {
                    add_link(ref, &result);
                    continue;
                }
                bool startsSentence = t.sentenceEnd(start);
                if (t.eof()) {
                    continue;
                }
                if (' ' == t.peek()) {
                    SkAssertResult(t.skipSpace());
                    if (t.eof()) {
                        continue;
                    }
                }
                if (!isupper(t.fChar[0]) && startsSentence) {
                    add_ref(ref, &result);
                    continue;
                }
                if (isupper(t.fChar[0]) && startsSentence) {
                    string nextWord(t.fChar, t.wordEnd() - t.fChar);
                    if (this->isDefined(nextWord)) {
                        add_ref(ref, &result);
                        continue;
                    }
                }
                string prefixed = fRoot->fName + "_" + ref;
                auto iter = fRoot->fChildDefinitions.find(prefixed);
                if (iter != fRoot->fChildDefinitions.end()) {
                    add_link(ref, &result);
                    continue;
                }
                SkDebugf("%s\n", ref.c_str());
            }
            add_ref(ref, &result);
        }
        
    } while (!t.eof());
    return result;
}

void BmhParser::buildReferences() {
    fMdHeaderDepth = 1;
    for (auto oneclass : fClassMap) {
        string name = oneclass.first;
        RootDefinition& classDef = oneclass.second;

        fRoot = &classDef;
        this->markTypeOut(&classDef);
        continue;

        // todo : add class itself
        string text = classDef.extractText(Definition::TrimExtract::kYes);
        string ref = this->addReferences(&text.front(), &text.back() + 1);
        if (ref.length()) {
            printf("%s\n", ref.c_str());
        }
        for (auto child : classDef.fChildDefinitions) {
            string text = child.second.extractText(Definition::TrimExtract::kYes);
            // parse C++ declaration and description
            ref = this->addReferences(&text.front(), &text.back() + 1);
            if (ref.length()) {
                printf("%s\n", ref.c_str());
            }
        }
    }
}

bool BmhParser::childOf(MarkType markType) const {
    auto childError = [this](MarkType markType) -> bool
    {
        string errStr = "expected ";
        errStr += fMaps[(int) markType].fName;
        errStr += " parent";
	    return this->reportError<bool>(errStr.c_str());
    };

    if (this->hasEndToken()) {
        if (!fParent->fParent) {
			return this->reportError<bool>("expected grandparent");
        }
        if (markType != fParent->fParent->fMarkType) {
			return childError(markType);
        }
    } else {
        if (MarkType::kExample != fParent->fMarkType) {
			return childError(markType);
        }
    }
    return true;
}

void BmhParser::childrenOut(const Definition* def, const char* start) {
    ++fMdHeaderDepth;
    const char* end;
    for (auto child : def->fChildren) {
        end = child->fStart;
        if (MarkType::kExample != child->fMarkType && MarkType::kStdOut != child->fMarkType) {
            this->resolveOut(start, end);
        }
        this->markTypeOut(child);
        start = child->fTerminator;
    }
    if (MarkType::kExample != def->fMarkType && MarkType::kFile != def->fMarkType) {
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
                builder += string(wordStart, wordEnd - wordStart);
                if (builder != fParent->fName) {
			        return this->reportError<string>("name mismatch");
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
            fExternals.push_back(string(wordStart, fChar - wordStart));
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

// fixme: this will need to be more complicated to handle all of Skia
// for now, just handle paint -- maybe fiddle will loosen naming restrictions
static string canonical_fiddle_name(const string& name) {
    size_t underscore = name.find('_', 0);
    SkASSERT(string::npos != underscore);
    string result = name.substr(0, underscore + 1);
    if (string::npos != name.find('~', underscore)) {
        result += "destructor";
    } else {
        bool isMove = string::npos != name.find("&&", underscore);
        const char operatorStr[] = "operator";
        size_t opPos = name.find(operatorStr, underscore);
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
        } else if (string::npos == name.find('&', underscore)) {
            SkASSERT(string::npos != name.find("()", underscore));
            result += "empty_constructor"; 
        } else {
            result += isMove ? "move_" : "copy_"; 
            result += "constructor"; 
        }
    }
    return result;
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
			if (this->peek() == fMC) {  // definition
				if (!lineStart) {
					this->next();
					if (' ' < this->peek()) {
						return this->reportError<bool>("expected definition");
					}
					continue;
				}
				this->next();
                if (this->peek() == fMC) {
                    this->next();
                    fMC = this->next();  // change markup character
                    if (' ' >= fMC) {
                        return this->reportError<bool>("illegal markup character");
                    }
                    continue;
                }
            } else if (this->peek() >= 'A' && this->peek() <= 'Z') {
                const char* defStart = fChar - 1;
				MarkType markType = this->getMarkType(MarkLookup::kRequire);
                bool hasEnd = this->hasEndToken();
                if (!this->skipName(fMaps[(int) markType].fName)) {
                    return this->reportError<bool>("illegal markup character");
                }
                if (!this->skipSpace()) {
                    return this->reportError<bool>("unexpected end");
                }
                vector<string> typeNameBuilder = this->typeName(markType);
        		Definition* definition = nullptr;
				switch (markType) {
					case MarkType::kComment:
						if (!this->skipToDefinitionEnd(markType)) {
							return false;
						}
						continue;
                    // these types may be referred to by name
                    case MarkType::kConst:
                    case MarkType::kEnum:
                    case MarkType::kEnumClass:
					case MarkType::kClass:
                    case MarkType::kMember:
					case MarkType::kMethod:
                    case MarkType::kStruct:
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
                            definition = fRoot;
                        } else {
                            if (nullptr == fParent) {
					            return this->reportError<bool>("expected parent");
                            }
                            if (fParent == fRoot && hasEnd) {
                                definition = fParent;
                            } else {
                                auto dup = fRoot->fChildDefinitions.find(name);
                                if (!hasEnd && fRoot->fChildDefinitions.end() != dup) {
    					            return this->reportError<bool>("duplicate symbol");
                                }
                                definition = &fRoot->fChildDefinitions[name];
                            }
                        }
				        if (hasEnd) {
					        if (!this->popParentStack(definition)) {
                                return false;
                            }
				        } else {
					        definition->fStart = defStart;
                            definition->fContentStart = fChar;
                            if (MarkType::kConst == markType) {
                                // todo: require that fChar points to def on same line as markup
                            }
					        definition->fName = name;
                            definition->fFiddle = name;
                            if (MarkType::kMethod == markType) {
                                if (string::npos != name.find('(', 0)) {
                                    definition->fFiddle = canonical_fiddle_name(name);
                                }
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
                            fTopics.emplace_front(markType, defStart, fParent);
                            definition = &fTopics.front();
                            definition->fContentStart = fChar;
                            definition->fName = typeNameBuilder[0];
                            definition->fFiddle = typeNameBuilder[0];
                            this->setAsParent(definition);
                        }
                        for (auto topic : typeNameBuilder) {
                            Definition* defPtr = fTopicMap[topic];
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
                                fTopicMap[topic] = definition;
                            }
                        }
				        if (hasEnd) {
					        if (!this->popParentStack(definition)) {
                                return false;
                            }
                        }
                        break;
                    // these types are children of parents, but are not in named maps
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
                                if (!this->popParentStack(fParent)) { // if not one liner, pop
                                    return false;
                                }
                            } else {
                                fMarkup.emplace_front(markType, defStart, fParent);
                                definition = &fMarkup.front();
                                definition->fName = typeNameBuilder[0];
                                definition->fFiddle = typeNameBuilder[0];
                                definition->fContentEnd = definition->fTerminator = this->lineEnd();
                                fParent->fChildren.push_back(definition);
                            }
                            break;
                        }
                    // not one-liners
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
                            fMarkup.emplace_front(markType, defStart, fParent);
                            definition = &fMarkup.front();
                            definition->fContentStart = fChar;
                            definition->fName = typeNameBuilder[0];
                            // fixme : doesn't handle case where e.g. operator method has
                            //         more than one example
                            definition->fFiddle = string::npos != typeNameBuilder[0].find('(', 0) 
                                    ? fParent->fFiddle : typeNameBuilder[0];
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
                        // fixme: expect text #reference ##
                        // or            text #url ##
                    case MarkType::kFile:
                    case MarkType::kImport:
                    case MarkType::kSeeAlso:
                    case MarkType::kTime:
				        if (hasEnd) {
    					    return this->reportError<bool>("one liners omit end element");
                        }
                        fMarkup.emplace_front(markType, defStart, fParent);
                        definition = &fMarkup.front();
                        definition->fName = typeNameBuilder[0];
                        definition->fFiddle = typeNameBuilder[0];
                        definition->fContentEnd = definition->fTerminator = this->lineEnd();
                        fParent->fChildren.push_back(definition);
                        break;
                    case MarkType::kExternal:
                        (void) this->collectExternals();  // FIXME: detect errors in external defs?
                        break;
					default:
                        SkASSERT(0);  // fixme : don't let any types be invisible
						continue;
				}
				if (fParent) {
                    SkASSERT(definition);
                    SkASSERT(definition->fName.length() > 0);
                }
				continue;
			} else if (this->peek() == ' ') {
                if (!fParent || (MarkType::kTable != fParent->fMarkType
                        && MarkType::kLegend != fParent->fMarkType)) {
				    int endHashes = this->endHashCount();
				    if (endHashes <= 1) {  // one line comment
					    fChar = fLine + this->lineLength() - 1;
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
                    fMarkup.emplace_front(MarkType::kRow, fChar, fParent);
                    Definition* row = &fMarkup.front();
                    this->setAsParent(row);
                    const char* lineEnd = this->lineEnd();
                    do {
                        this->skipWhiteSpace();
                        fMarkup.emplace_front(MarkType::kColumn, fChar, fParent);
                        Definition* column = &fMarkup.front();
                        column->fContentEnd = this->trimmedBracketEnd(fMC);
                        column->fTerminator = column->fContentEnd;
                        fParent->fChildren.push_back(column);
                        this->skipToEndBracket(fMC);
                        this->next();
                    } while (fMC != this->peek() && fChar < lineEnd);
                    if (!this->popParentStack(fParent)) {
                        return false;
                    }
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
	if (--last <= fLine) {
		return false;
	}
	return last[0] == fMC && last[1] == fMC;
}

void BmhParser::markTypeOut(const Definition* def) {
    string printable = def->printableName();
    string colonform = def->colonFormName();
    const char* textStart = def->fContentStart;
    switch (def->fMarkType) {
        case MarkType::kAnchor:
            break;
        case MarkType::kBug:
            break;
        case MarkType::kClass:
            this->mdHeaderOut();
            printf("<a name=\"%s\"></a> Class %s\n", def->fName.c_str(), colonform.c_str());
            break;
        case MarkType::kColumn:
            printf("| ");
            break;
        case MarkType::kComment:
            break;
        case MarkType::kConst: {
            this->mdHeaderOut();
            printf("<a name=\"%s\"></a> Const %s", def->fName.c_str(), colonform.c_str());
            const char* lineEnd = strchr(textStart, '\n') + 1;
            SkASSERT(lineEnd < fEnd);
            SkASSERT(lineEnd > textStart);
            SkASSERT((int) (lineEnd - textStart) == lineEnd - textStart);
            printf(" %.*s", (int) (lineEnd - textStart), textStart);
            textStart = lineEnd;
            } break;
        case MarkType::kDefine:
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
            printf("<a name=\"%s\"></a> Enum %s\n", def->fName.c_str(), colonform.c_str());
            break;
        case MarkType::kError:
            break;
        case MarkType::kExample:
            printf("<fiddle-embed hash=\"%s\"></fiddle-embed>\n", def->fHash.c_str());
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
        case MarkType::kImport:
            break;
        case MarkType::kLegend:
            break;
        case MarkType::kList:
            break;
        case MarkType::kMember:

            break;
        case MarkType::kMethod: {
            this->mdHeaderOut();
            printf("<a name=\"%s\"></a> %s\n", def->fName.c_str(), colonform.c_str());
            } break;
        case MarkType::kParam:

            break;
        case MarkType::kPlatform:
            break;
        case MarkType::kReturn:

            break;
        case MarkType::kRow:
            break;
        case MarkType::kSeeAlso:
            break;
        case MarkType::kStdOut:

            break;
        case MarkType::kStruct:
            this->mdHeaderOut();
            printf("<a name=\"%s\"></a> Struct %s\n", def->fName.c_str(), colonform.c_str());
            break;
        case MarkType::kSubtopic:
            this->mdHeaderOut();
            printf("<a name=\"Subtopic_%s\"></a> %s\n", def->fName.c_str(), printable.c_str());
            break;
        case MarkType::kTable:
            printf("\n");
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
            printf("<a name=\"Topic_%s\"></a> %s\n", def->fName.c_str(), printable.c_str());
            break;
        case MarkType::kTrack:
            break;
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
        case MarkType::kColumn:
            printf(" ");
            break;
        case MarkType::kLegend: {
            SkASSERT(def->fChildren.size() == 1);
            const Definition* row = def->fChildren[0];
            SkASSERT(MarkType::kRow == row->fMarkType);
            size_t columnCount = row->fChildren.size();
            SkASSERT(columnCount > 0);
            for (size_t index = 0; index < columnCount; ++index) {
                printf("| --- ");
            }
            printf(" |\n");
            } break;
        case MarkType::kRow:
            printf("|\n");
            break;
        case MarkType::kTable:
            printf("\n");
            break;
    }
}

void BmhParser::mdHeaderOut() {
    for (int index = 0; index < fMdHeaderDepth; ++index) {
        printf("#");
    }
    if (fMdHeaderDepth) {
        printf(" ");
    }
}

string BmhParser::memberName() {
    this->skipSpace();
    this->skipToNonAlphaNum();
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
    isConstructor = false;
    if (parent && parent->fName.length() > 0) {
        if (parent->fName == name) {
            isConstructor = true;
        }
        builder = parent->fName + '_';
    }
    if (isConstructor || expectOperator) {
        paren = this->strnchr(')', end) + 1;
    }
    builder.append(nameStart, paren - nameStart);
    if (fChar < --end) {
        fChar = end;
    }
    this->next();
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
	definition->fContentEnd = fLine;
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
}

void TextParser::reportError(const char* errorStr) const {
    ReportError(errorStr, fLineCount, this->lineLength(), fLine, fChar);
}

void BmhParser::resolveOut(const char* start, const char* end) {
    if (start >= end) {
        return;
    }
    string resolved = this->addReferences(start, end);
    printf("%s", resolved.c_str());  // dump to console while debugging
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
    } while (++fLineCount, fLine += lineLen, fChar = fLine, !this->eof() && !foundEnd);
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
        if (',' == ch) {
            string builder(nameStart, fChar - nameStart - 1);
            trim_start_end(builder);
            result.push_back(builder);
            this->skipWhiteSpace();
            nameStart = fChar;
            break;
        }
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
    return result;
}

// typeName parsing rules depend on mark type
vector<string> BmhParser::typeName(MarkType markType) {
    vector<string> result;
    string builder;
    if (fParent) {
        builder = fParent->fName;
    }
    switch (markType) {
        case MarkType::kAnchor: 
            break;  // unnamed
        case MarkType::kBug:
            // fixme: expect number
            builder = this->word(builder);
            break;
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
        case MarkType::kExample: // todo: break out example to check if there is more than one
            // allow (but don't require) name
            // check to see if one already exists -- if so, number this one
            builder = this->uniqueName(string(), markType);
            break;
        case MarkType::kDeprecated:
        case MarkType::kDescription:
        case MarkType::kDoxygen:
        case MarkType::kError:
        case MarkType::kExternal:
        case MarkType::kFile:
        case MarkType::kFormula:
        case MarkType::kFunction:
        case MarkType::kImport:
        case MarkType::kLegend:
        case MarkType::kList:
        case MarkType::kPlatform:
        case MarkType::kReturn:
        case MarkType::kTime:
        case MarkType::kToDo:
        case MarkType::kTrack:
        case MarkType::kWidth:
            // fixme : expect unnamed data for most of these
            break;
        case MarkType::kSeeAlso:
             // fixme : should be delineated by comma, not quotes
            builder = this->word(builder);
            break;
        case MarkType::kImage:
        case MarkType::kStdOut:
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
            break;
        case MarkType::kTable:
            // fixme: require table to be blank for now
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
        bool foundMatch = false;
        for (auto iter : fParent->fChildren) {
            if (markType == iter->fMarkType) {
                if (iter->fName == numBuilder) {
                    numBuilder = builder + '_' + std::to_string(number);
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
        bool foundMatch = false;
        for (auto iter : parent->fChildDefinitions) {
            if (markType == iter.second.fMarkType) {
                if (iter.second.fName == numBuilder) {
                    numBuilder = builder + '_' + std::to_string(number);
                    goto tryNext;
                }
            }
        }
        break;
tryNext: ;
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

DEFINE_string2(bmh, b, "", "A path to a *.bmh file or a directory.");
DEFINE_string2(include, i, "", "A path to a *.h file or a directory.");
DEFINE_string2(fiddle, f, "", "A path to fiddle input/output file, usually example.htm");
DEFINE_bool2(ref, r, false, "Build references.");
DEFINE_bool2(examples, e, false, "Output examples.");

static bool dump_examples(const Definition& def) {
    if (MarkType::kExample == def.fMarkType) {
        string result;
        if (!def.exampleToScript(&result)) {
            return false;
        }
        printf("%s\n", result.c_str());
        return true;
    }
    for (auto child : def.fChildren ) {
        if (!dump_examples(*child)) {
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
    for (auto child : def.fChildren ) {
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
		"Usage: bookmaker -b path/to/file.bmh [-i path/to/file.h]\n");
    SkCommandLineFlags::Parse(argc, argv);
	if (FLAGS_bmh.isEmpty() && FLAGS_include.isEmpty()) {
        SkCommandLineFlags::PrintUsage();
        return 1;
	}
	if (!FLAGS_bmh.isEmpty()) {
//		parser.setParseType(Parser::ParseType::kBmh);
		if (!bmhParser.parseFile(FLAGS_bmh[0], ".bmh")) {
			return -1;
		}
	}
	if (!FLAGS_include.isEmpty()) {
//		parser.setParseType(Parser::ParseType::kInclude);
		if (!interfaceParser.parseFile(FLAGS_include[0], ".h")) {
			return -1;
		}
	}
    FiddleParser fparser(bmhParser);
    if (!FLAGS_fiddle.isEmpty()) {
        if (!fparser.parseFile(FLAGS_fiddle[0], ".htm")) {
			return -1;
		}
    }
	if (FLAGS_ref) {
        bmhParser.buildReferences();
        SkDebugf("\n");
    }
    int examples = 0;
    int methods = 0;
    int topics = 0;
    if (FLAGS_examples) {
        printf("function testFiddles() {\n\n");
    }
    for (auto oneclass : bmhParser.fClassMap) {
        examples += count_children(oneclass.second, MarkType::kExample);
        methods += count_children(oneclass.second, MarkType::kMethod);
        topics += count_children(oneclass.second, MarkType::kSubtopic);
        topics += count_children(oneclass.second, MarkType::kTopic);
        if (FLAGS_examples) {
            dump_examples(oneclass.second);
        }
    }
    if (FLAGS_examples) {
        printf("}\n\n");
    }
	SkDebugf("topics=%d classes=%d methods=%d examples=%d\n", 
            bmhParser.fTopicMap.size(), bmhParser.fClassMap.size(),
			methods, examples);
	return 0;
}
