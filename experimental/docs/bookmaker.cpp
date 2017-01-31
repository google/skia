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
static const TypeNames kTypeNames[] = {
    { "",           MarkType::kUnknown },
    { "Class",      MarkType::kClass },
    { "",           MarkType::kColumn },
    { "Comment",    MarkType::kComment },
	{ "Const",      MarkType::kConst },
	{ "Define",     MarkType::kDefine },
    { "Doxygen",    MarkType::kDoxygen },
	{ "Enum",       MarkType::kEnum },
    { "Example",    MarkType::kExample },
    { "Method",     MarkType::kMethod },
	{ "",           MarkType::kParameter },
    { "",           MarkType::kRow },
    { "StdOut",     MarkType::kStdOut },
	{ "Struct",     MarkType::kStruct },
    { "Table",      MarkType::kTable },
	{ "Template",   MarkType::kTemplate },
    { "",           MarkType::kText },
    { "ToDo",       MarkType::kToDo },
	{ "Typedef",    MarkType::kTypedef },
	{ "Union",      MarkType::kUnion },
};

static_assert((int) Last_MarkType == (int) SK_ARRAY_COUNT(kTypeNames) - 1,
		"MarkType / kTypeNames mismatch");

int Parser::endHashCount() const {
	const char* end = fLine + this->lineLength();
	int count = 0;
	while (fLine < end && fMC == *--end) {
		count++;
	}
	return count;
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
				MarkType markType = this->getMarkType(MarkLookup::kRequire);
				bool hasEnd = this->hasEndToken();
                std::string typeNameBuilder = this->typeName(hasEnd);
				if (!typeNameBuilder.length() && MarkType::kComment != markType) {
					return this->reportError<bool>("unnamed parent");
				}
        		Definition* definition = nullptr;
				switch (markType) {
					case MarkType::kComment:
						if (!this->skipToDefinitionEnd(markType)) {
							return false;
						}
						continue;
					case MarkType::kClass:
					case MarkType::kExample:
					case MarkType::kMethod:
					case MarkType::kStdOut:
					case MarkType::kTable:
						definition = this->findBmhObject(markType, typeNameBuilder);
						break;
					default:
						continue;
				}
				if (hasEnd) {
					if (!this->popParentStack(definition)) {
                        return false;
                    }
				} else {
					definition->fStart = fChar;
					definition->fName = typeNameBuilder;
					definition->fMarkType = markType;
                    this->setAsParent(definition);
					if (MarkType::kTable == markType) {
						if (!this->skipToDefinitionEnd(markType)) {
							return false;
						}
						if (!this->popParentStack(definition)) {
                            return false;
                        }
					}
				}
				if (!fParent) {
					if (fLine + this->lineLength() != fEnd) {
                        return reportError<bool>("mismatched end");
                    }
                    return true;
				}
				continue;
			} else if (this->peek() == ' ' && lineStart) {
				int endHashes = this->endHashCount();
				if (endHashes <= 1) {  // one line comment
					fChar = fLine + this->lineLength() - 1;
				} else {  // table row
					if (2 != endHashes) {
                        std::string errorStr = "expect ";
                        errorStr += fMC;
                        errorStr += fMC;
						return this->reportError<bool>(errorStr.c_str());
					}
					if (!fParent || MarkType::kTable != fParent->fMarkType) {
						return this->reportError<bool>("missing table");
					}
				}
			} else {  // reference
				(void) this->getMarkType(MarkLookup::kAllowUnknown);
				continue;
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
    return MarkType::kUnknown;
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
	sk_sp<SkData> data = SkData::MakeFromFileName(path);
    std::string name(path);
	fRawData[name] = data;
	const char* rawText = (const char*) data->data();
    fStart = rawText;
    fLine = rawText;
    fChar = rawText;
    fEnd = rawText + data->size();
	fFileName = path;
    fLineCount = 1;
	if (ParseType::kBmh == fParseType) {
		fMC = '#';
		return findDefinitions();
	}
	SkASSERT(ParseType::kInclude == fParseType);
    fTrackBrackets = true;
	return parseInclude(name);
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
	if (definition->fEnd) {
		return this->reportError<bool>("definition already ended");
	}
	definition->fEnd = fLine;
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
        if (*fChar++ != fMC) {
            continue;
        }
        if (*fChar++ != fMC) {
            continue;
        }
        if (*fChar == fMC) {
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

std::string Parser::typeName(bool hasEnd) {
    std::string builder;
    if (fParent) {
        builder = fParent->fName;
        if (hasEnd) {
            return builder;
        }
    } else if (hasEnd) {
        return reportError<std::string>("end without matching start");
    }
	const char* lineEnd = fLine + this->lineLength();
	SkAssertResult(fMC == fChar[-1] && fMC == fChar[-2]);  // hashes in mark type
	SkASSERT(fMC != this->peek());
	// look for a single markup character before the line end

	while (fChar < lineEnd && fMC != this->next())
		;
	if (fMC != fChar[-1]) {
		return builder;
	}
	if (fMC == fChar[0]) {
		return builder;
	}
	if (' ' >= fChar[0]) {
		return builder;
	}
	const char* nameStart = fChar;
	while (fChar < lineEnd) {
		char ch = this->next();
		if (' ' >= ch) {
			break;
		}
		if (!isalnum(ch) && '_' != ch) {
			return reportError<std::string>("unexpected char");
		}
	}
    builder += '_';
    builder.append(nameStart, fChar - nameStart - 1);
    return builder;
}

void Parser::validate() const {
	for (int index = 0; index <= (int) Last_MarkType; ++index) {
		SkASSERT(fMaps[index].fMarkType == (MarkType) index);
	}
	for (int index = 0; index <= (int) Last_MarkType; ++index) {
		SkASSERT(kTypeNames[index].fMarkType == (MarkType) index);
	}
}

// pass one: parse text, collect definitions
// pass two: lookup references

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
	SkDebugf("classes=%d methods=%d examples=%d\n", parser.fClassMap.size(),
			parser.fMethodMap.size(), parser.fExampleMap.size());
	return 0;
}
