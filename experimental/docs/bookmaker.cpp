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

// names without formal definitions (e.g. Column) aren't included
// fill in other names once they're actually used (e.g. "Struct")
static const TypeNames kTypeNames[] = {
    { "",           MarkType::kNone },
    { "A",          MarkType::kAnchor },
    { "Bug",        MarkType::kBug },
    { "Class",      MarkType::kClass },
    { "",           MarkType::kColumn },
    { "",           MarkType::kComment },
	{ "Code",       MarkType::kCode },
	{ "",           MarkType::kConst },
	{ "",           MarkType::kDefine },
    { "Deprecated", MarkType::kDeprecated },
    { "Description", MarkType::kDescription },
    { "Doxygen",    MarkType::kDoxygen },
	{ "Enum",       MarkType::kEnum },
    { "Example",    MarkType::kExample },
    { "File",       MarkType::kFile },
    { "Formula",    MarkType::kFormula },
    { "Image",      MarkType::kImage },
    { "Import",     MarkType::kImport },
    { "List",       MarkType::kList },
    { "Method",     MarkType::kMethod },
	{ "Param",      MarkType::kParam },
	{ "Platform",   MarkType::kPlatform },
    { "Return",     MarkType::kReturn },
    { "",           MarkType::kRow },
    { "SeeAlso",    MarkType::kSeeAlso },
    { "StdOut",     MarkType::kStdOut },
	{ "",           MarkType::kStruct },
    { "Subtopic",   MarkType::kSubtopic },
    { "Table",      MarkType::kTable },
	{ "Template",   MarkType::kTemplate },
    { "",           MarkType::kText },
    { "Time",       MarkType::kTime },
    { "ToDo",       MarkType::kToDo },
    { "Topic",      MarkType::kTopic },
    { "Track",      MarkType::kTrack },
	{ "",           MarkType::kTypedef },
	{ "",           MarkType::kUnion },
    { "Width",      MarkType::kWidth },
};

static_assert((int) Last_MarkType == (int) SK_ARRAY_COUNT(kTypeNames) - 1,
		"MarkType / kTypeNames mismatch");

void Definition::dump() const {
    SkDebugf("// %s %s\n\n", kTypeNames[(int) fMarkType].fName, fName.c_str());
    string text = this->extractText();
    const char drawWrapper[] = "void draw(SkCanvas* canvas) {";
    bool hasFunc = !text.compare(0, sizeof(drawWrapper) - 1, drawWrapper);
    bool preprocessor = text[0] == '#';
    string example;
    if (!hasFunc && !preprocessor) {
        example += "void draw(SkCanvas* canvas) {\n";
    }
    // fixme? fix up indenting
    example += text;
    if (!hasFunc && !preprocessor) {
        example += "\n}";
    }
    example += "\n\n";
    SkDebugf("%s", example.c_str());
}

bool Parser::childOf(MarkType markType) const {
    auto childError = [this](MarkType markType) -> bool
    {
        string errStr = "expected ";
        errStr += kTypeNames[(int) markType].fName;
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

string Parser::className(MarkType markType) {
    string builder;
    Definition* parent = fParent;
    while (parent && parent->fMarkType != MarkType::kClass) {
        parent = parent->fParent;
    }
    if (parent && (parent != fParent || MarkType::kClass != markType)) {
        builder += parent->fName;
    }
    const char* end = this->lineEnd();
    const char* mc = this->strnchr(fMC, end);
    builder = this->word(builder);
    if (mc) {
        bool checkMatch;
        if (mc >= fChar) {
            this->skipTo(mc);
            this->next();
            checkMatch = true;
        } else {
            checkMatch = false;
        }
        if (fMC == this->peek()) {
            this->next();
            if (markType != fParent->fMarkType) {
			    return reportError<string>("unbalanced method");
            }
            if (checkMatch && builder.length() > 0 && builder != fParent->fName) {
			    return reportError<string>("name mismatch");
            }
            return fParent->fName;
        }
    }
    return builder;
}

int Parser::endHashCount() const {
	const char* end = fLine + this->lineLength();
	int count = 0;
	while (fLine < end && fMC == *--end) {
		count++;
	}
	return count;
}

string Parser::filePath() {
    this->skipWhiteSpace();
	const char* lineEnd = fLine + this->lineLength();
	const char* nameStart = fChar;
	while (fChar < lineEnd) {
		char ch = this->next();
		if (' ' >= ch) {
			break;
		}
        if (',' == ch) {
            break;
        }
        if (fMC == ch) {
            break;
        }
    }
    string builder(nameStart, fChar - nameStart - 1);
    return builder;
}

bool Parser::findDefinitions() {
	bool lineStart = true;
	fParent = nullptr;
	while (!this->eof()) {
		if (this->peek() == fMC) {
			this->next();
			if (this->peek() == fMC) {  // definition
				if (!lineStart) {
					this->next();
					if (' ' < this->peek()) {
						return reportError<bool>("expected definition");
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
				MarkType markType = this->getMarkType(MarkLookup::kRequire);
                if (!this->skipName(kTypeNames[(int) markType].fName)) {
                    return this->reportError<bool>("illegal markup character");
                }
                if (!this->skipSpace()) {
                    return this->reportError<bool>("unexpected end");
                }
                const char* defStart = fChar;
                        if (fLineCount >= 3627) {
                            SkDebugf("");
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
                    case MarkType::kEnum:
					case MarkType::kClass:
					case MarkType::kMethod:
				        if (!typeNameBuilder.size()) {
					        return this->reportError<bool>("unnamed markup");
				        }
                        if (typeNameBuilder.size() > 1) {
					        return this->reportError<bool>("expected one name only");
                        }
						definition = this->findBmhObject(markType, typeNameBuilder[0]);
				        if (this->hasEndToken()) {
					        if (!this->popParentStack(definition)) {
                                return false;
                            }
				        } else {
					        definition->fStart = defStart;
					        definition->fName = typeNameBuilder[0];
					        definition->fMarkType = markType;
                            this->setAsParent(definition);
				        }
						break;
                    case MarkType::kTopic: // may define multiple keys
                        SkASSERT(typeNameBuilder.size() > 0);
				        if (!this->hasEndToken()) {
				            if (!typeNameBuilder.size()) {
					            return this->reportError<bool>("unnamed topic");
				            }
                            fTopics.emplace_front(markType, defStart, fParent);
                            definition = &fTopics.front();
                            definition->fName = typeNameBuilder[0];
                            this->setAsParent(definition);
                        }
                        for (auto topic : typeNameBuilder) {
                            Definition* defPtr = fTopicMap[topic];
                            if (this->hasEndToken()) {
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
				        if (this->hasEndToken()) {
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
				        if (this->hasEndToken()) {
                            if (markType == fParent->fMarkType) {
                                definition = fParent;
                                if (!this->popParentStack(fParent)) { // if not one liner, pop
                                    return false;
                                }
                            } else {
                                fMarkup.emplace_front(markType, defStart, fParent);
                                definition = &fMarkup.front();
                                definition->fName = typeNameBuilder[0];
                                definition->fContentEnd = definition->fTerminator = this->lineEnd();
                                fParent->fChildren.push_back(definition);
                            }
                            break;
                        }
                    // not one-liners
                    case MarkType::kCode:
                    case MarkType::kDeprecated:
                    case MarkType::kExample:
                    case MarkType::kFormula:
                    case MarkType::kList:
                    case MarkType::kSubtopic:
					case MarkType::kTable:
                    case MarkType::kTrack:
				        if (this->hasEndToken()) {
                            definition = fParent;
                            if (markType != fParent->fMarkType) {
    					        return this->reportError<bool>("end element mismatch");
                            } else if (!this->popParentStack(fParent)) {
                                return false;
                            }
                        } else {
                            fMarkup.emplace_front(markType, defStart, fParent);
                            definition = &fMarkup.front();
                            definition->fName = typeNameBuilder[0];
					        if (MarkType::kTable == markType) { // fixme: skip table for now
						        if (!this->skipToDefinitionEnd(markType)) {
							        return false;
						        }
					        } else {
                                this->setAsParent(definition);
                            }
                        }
                        break;
                        // always treated as one-liners (can't detect misuse easily)
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
				        if (this->hasEndToken()) {
    					    return this->reportError<bool>("one liners omit end element");
                        }
                        fMarkup.emplace_front(markType, defStart, fParent);
                        definition = &fMarkup.front();
                        definition->fName = typeNameBuilder[0];
                        definition->fContentEnd = definition->fTerminator = this->lineEnd();
                        fParent->fChildren.push_back(definition);
                        break;
					default:
                        SkASSERT(0);  // fixme : don't let any types be invisible
						continue;
				}
				if (!fParent) {
					if (fLine + this->lineLength() != fEnd) {
                        return reportError<bool>("mismatched end");
                    }
                    return true;
				} else {
                    SkASSERT(definition);
                    SkASSERT(definition->fName.length() > 0);
                }
				continue;
			} else if (this->peek() == ' ') {
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
			}
		}
		lineStart = this->next() == '\n';
	}
	return true;
}

MarkType Parser::getMarkType(MarkLookup lookup) const {
	if (fLineCount == 74) {
		SkDebugf("");
	}
    for (int index = 0; index < (int) SK_ARRAY_COUNT(kTypeNames); ++index) {
        int typeLen = strlen(kTypeNames[index].fName);
        if (typeLen == 0) {
            continue;
        }
        if (fChar + typeLen >= fEnd || fChar[typeLen] > ' ') {
            continue;
        }
        int chCompare = strncmp(fChar, kTypeNames[index].fName, typeLen);
        if (chCompare < 0) {
            goto fail;
        }
        if (chCompare == 0) {
            return (MarkType) index;
        }
    }
fail:
    if (MarkLookup::kRequire == lookup) {
        reportError("unknown mark type");
    }
    return MarkType::kNone;
}

bool Parser::hasEndToken() const {
	const char* last = fLine + this->lineLength();
	while (last > fLine && ' ' >= *--last)
		;
	if (--last <= fLine) {
		return false;
	}
	return last[0] == fMC && last[1] == fMC;
}

    // if line has #, name follows
    // otherwise, name preceeds first paren
    // fixme: change markup.bmh to require # if duplicate name before parent
    // or first paren is not beginning of function parameter list
string Parser::methodName() {
    string builder;
    Definition* parent = fParent;
    while (parent && parent->fMarkType != MarkType::kClass) {
        parent = parent->fParent;
    }
    if (parent) {
        builder += parent->fName;
    }
    const char* end = this->lineEnd();
    const char* mc = this->strnchr(fMC, end);
    const char* paren = this->strnchr('(', end);
    if (mc) {
        this->skipToEndBracket(fMC);
        this->next();
        if (fMC == this->peek()) {
            this->next();
            if (MarkType::kMethod != fParent->fMarkType) {
			    return reportError<string>("unbalanced method");
            }
            return fParent->fName;
        }
        builder = this->word(builder);
    } else if (paren) {
        const char* nameStart = paren;
        char ch;
        while (nameStart > fChar && ' ' != (ch = *--nameStart)) {
		    if (!isalnum(ch) && '_' != ch && ':' != ch && '-' != ch) {
			    return reportError<string>("unexpected method name char");
		    }
        }
        if (' ' != nameStart[0]) {
            return reportError<string>("missing method name");
        }
        ++nameStart;
        if (nameStart >= paren) {
            return reportError<string>("missing method name length");
        }
        if (parent && parent->fName.length() > 0) {
            builder += '_';
        }
        builder.append(nameStart, paren - nameStart);
    } else {
        return reportError<string>("missing method name and reference");
    }
    if (fChar < --end) {
        fChar = end;
    }
    this->next();
    return builder;
}

bool Parser::parseFile(const char* fileOrPath) {
	if (!sk_isdir(fileOrPath)) {
		if (!this->parseFromFile(fileOrPath)) {
			SkDebugf("failed to parse %s\n", fileOrPath);
			return false;
		}
	} else {	
		SkOSFile::Iter it(fileOrPath, ".bmh");
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

bool Parser::parseFromFile(const char* path) {
    this->reset();
	sk_sp<SkData> data = SkData::MakeFromFileName(path);
    string name(path);
	fRawData[name] = data;
	const char* rawText = (const char*) data->data();
    fStart = rawText;
    fLine = rawText;
    fChar = rawText;
    fEnd = rawText + data->size();
	fFileName = path;
    fLineCount = 1;
	if (ParseType::kBmh == fParseType) {
		return findDefinitions();
	}
	SkASSERT(ParseType::kInclude == fParseType);
	return parseInclude(name);
}

void Parser::parseFromChild(Definition* child) {
    fStart = child->fStart;
    fEnd = child->fContentEnd;
    this->reset();
}

bool Parser::popParentStack(Definition* definition) {
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

void Parser::reportError(const char* errorStr) const {
    ReportError(errorStr, fLineCount, this->lineLength(), fLine, fChar);
}

bool Parser::reportError(const Definition& def, const char* errorStr) const {
    ReportError(errorStr, -1 /* todo: add line # to def */, def.fContentEnd - def.fStart,
            def.fStart, def.fStart);
    return false;
}

bool Parser::skipToDefinitionEnd(MarkType markType) {
    if (this->eof()) {
        return reportError<bool>("missing end");
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

vector<string> Parser::topicName() {
    vector<string> result;
    this->skipWhiteSpace();
	const char* lineEnd = fLine + this->lineLength();
	const char* nameStart = fChar;
	while (fChar < lineEnd) {
		char ch = this->next();
        if (',' == ch) {
            string builder(nameStart, fChar - nameStart - 1);
            trim(builder);
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
        trim(builder);
        result.push_back(builder);
    }
    return result;
}

// typeName parsing rules depend on mark type
vector<string> Parser::typeName(MarkType markType) {
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
        case MarkType::kClass:
        case MarkType::kEnum:
            // expect name
            // fixme: need a way to reference enum defined in another file (e.g. SkFilterQuality)
            builder = this->className(markType);
            break;
        case MarkType::kCode:
        case MarkType::kDeprecated:
        case MarkType::kDescription:
        case MarkType::kDoxygen:
        case MarkType::kExample:
        case MarkType::kFile:
        case MarkType::kFormula:
        case MarkType::kImport:
        case MarkType::kList:
        case MarkType::kPlatform:
        case MarkType::kReturn:
        case MarkType::kSubtopic:
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
        case MarkType::kMethod:
            builder = this->methodName();
           break;
        case MarkType::kParam:
           // fixme: expect camelCase
            builder = this->word(builder);
            break;
        case MarkType::kTable:
            // fixme: require table to be blank for now
            if ('\n' != this->peek()) {
                this->reportError("unexpected table content");
            }
            break;  // unnamed
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

void Parser::validate() const {
	for (int index = 0; index <= (int) Last_MarkType; ++index) {
		SkASSERT(fMaps[index].fMarkType == (MarkType) index);
	}
	for (int index = 0; index <= (int) Last_MarkType; ++index) {
		SkASSERT(kTypeNames[index].fMarkType == (MarkType) index);
	}
    ValidateKeyWords();
}

string Parser::word(const string& prefix) {
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
            break;
        }
        if (fMC == ch) {
            return builder;
        }
		if (!isalnum(ch) && '_' != ch && ':' != ch && '-' != ch) {
			return reportError<string>("unexpected char");
		}
        if (':' == ch) {
            // expect pair, and expect word to start with Sk
            if (nameStart[0] != 'S' || nameStart[1] != 'k') {
                return reportError<string>("expected Sk");
            }
            if (':' != this->peek()) {
                return reportError<string>("expected ::");
            }
            this->next();
        } else if ('-' == ch) {
            // expect word not to start with Sk or kX where X is A-Z
            if (nameStart[0] == 'k' && nameStart[1] >= 'A' && nameStart[1] <= 'Z') {
                return reportError<string>("didn't expected kX");
            }
            if (nameStart[0] == 'S' && nameStart[1] == 'k') {
                return reportError<string>("expected Sk");
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
extern void call_curl(const char* msg);

DEFINE_string2(bmh, b, "", "A path to a *.bmh file or a directory.");
DEFINE_string2(include, i, "", "A path to a *.h file or a directory.");

int main(int argc, char** const argv) {
	Parser parser;
	parser.validate();
	SkCommandLineFlags::SetUsage(
		"Usage: bookmaker -b path/to/file.bmh [-i path/to/file.h]\n");
    SkCommandLineFlags::Parse(argc, argv);
	if (FLAGS_bmh.isEmpty() && FLAGS_include.isEmpty()) {
        SkCommandLineFlags::PrintUsage();
        return 1;
	}
	if (!FLAGS_bmh.isEmpty()) {
		parser.setParseType(Parser::ParseType::kBmh);
		if (!parser.parseFile(FLAGS_bmh[0])) {
			return -1;
		}
	}
	if (!FLAGS_include.isEmpty()) {
		parser.setParseType(Parser::ParseType::kInclude);
		if (!parser.parseFile(FLAGS_include[0])) {
			return -1;
		}
	}
    int examples = 0;
    for (auto topic : parser.fTopics) {
        for (auto child : topic.fChildren ) {

            if (MarkType::kExample == child->fMarkType) {
                child->dump();
                ++examples;
            }
        }
    }
    for (auto topic : parser.fClassMap) {
        for (auto child : topic.second.fChildren ) {
            if (MarkType::kExample == child->fMarkType) {
                child->dump();
                ++examples;
            }
        }
    }
    for (auto topic : parser.fMethodMap) {
        for (auto child : topic.second.fChildren ) {
            if (MarkType::kExample == child->fMarkType) {
                child->dump();
                ++examples;
            }
        }
    }
	SkDebugf("topics=%d classes=%d methods=%d examples=%d\n", 
            parser.fTopicMap.size(), parser.fClassMap.size(),
			parser.fMethodMap.size(), examples);
	return 0;
}
