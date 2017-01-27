/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkData.h"
#include "SkOSFile.h"
#include "SkOSPath.h"

#include <cmath>
#include <cctype>
#include <map>
#include <vector>

/*
find include/core -type f -name '*.h' -print -exec git blame {} \; > ~/all.blame.txt
 */

enum class MarkType {
    kUnknown,
    kClass,
    kColumn,
    kComment,
	kConst,
	kDefine,
    kDoxygen,
	kEnum,
    kExample,
    kMethod,
    kRow,
    kStdOut,
	kStruct,
    kTable,
	kTemplate,
    kText,
    kToDo,
	kTypedef,
	kUnion,
};

const MarkType gLast_MarkType = MarkType::kUnion;

const char* kTypeNames[] = {
    "",
    "Class",
    "",         // names without formal defintions (e.g. Column) aren't included
    "Comment",
	"Const",
	"Define",
    "Doxygen",
	"Enum",
    "Example",
    "Method",
    "",
    "StdOut",
	"Struct",
    "Table",
	"Template",
    "",
    "ToDo",
	"Typedef",
	"Union",
};

static_assert((int) gLast_MarkType == SK_ARRAY_COUNT(kTypeNames) - 1,
		"MarkType / kTypeNames mismatch");

struct Definition {
	Definition()
		: fStart(nullptr)
		, fEnd(nullptr)
		, fParent(nullptr)
		, fSibling(nullptr)
		, fChild(nullptr)
		, fMarkType(MarkType::kUnknown) {
	}

    const char* fStart;  // .. in original text file
	std::string fName;
	const char* fEnd;
	Definition* fParent;
	Definition* fSibling;
	Definition* fChild;
    MarkType fMarkType;
};

struct Reference {
	Reference()
		: fLocation(nullptr)
		, fDefinition(nullptr) {
	}

    const char* fLocation;  // .. in original text file
    const Definition* fDefinition;
};

struct Parser {
    Parser()
        : fStart(nullptr)
        , fLine(nullptr)
        , fChar(nullptr)
        , fEnd(nullptr)
		, fFileName(nullptr)
        , fLineCount(0) {
    }

	enum class MarkLookup {
		kRequire,
		kAllowUknown,
	};

	int endHashCount() const {
		const char* end = fLine + this->lineLength();
		int count = 0;
		while (fLine < end && fMC == *--end) {
			count++;
		}
		return count;
	}

    bool eof() const { return fChar >= fEnd; }

	Definition* find(MarkType markType, const std::string& typeName) {
		std::map<std::string, Definition>* map = fMaps[(int) markType].fMap;
		if (!map) {
			return nullptr;
		}
		return &(*map)[typeName];
	}

	bool findDefinitions() {
		bool lineStart = true;
		Definition* parent = nullptr;
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
                    std::string typeNameBuilder = this->typeName(parent, hasEnd);
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
							definition = this->find(markType, typeNameBuilder);
							break;
						default:
							continue;
					}
					if (hasEnd) {
						parent = this->popParentStack(parent, definition);
					} else {
						definition->fStart = fChar;
						definition->fName = typeNameBuilder;
						definition->fMarkType = markType;
						definition->fParent = parent;
						if (parent) {
							definition->fSibling = parent->fChild; // constructed in reverse order
							parent->fChild = definition;  // fix up child order once done
						}
						parent = definition;
						if (MarkType::kTable == markType) {
							if (!this->skipToDefinitionEnd(markType)) {
								return false;
							}
							parent = this->popParentStack(parent, definition);
						}
					}
					if (!parent) {
						return fLine + this->lineLength() == fEnd;
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
						if (!parent || MarkType::kTable != parent->fMarkType) {
							return this->reportError<bool>("missing table");
						}
					}
				} else {  // reference
					(void) this->getMarkType(MarkLookup::kAllowUknown);
					continue;
				}
			}
			if ((lineStart = this->next() == '\n')) {
				++fLineCount;
				fLine = fChar;
			}
		}
		return true;
	}

    MarkType getMarkType(MarkLookup lookup) const {
		if (fLineCount == 74) {
			SkDebugf("");
		}
        for (int index = 0; index < (int) SK_ARRAY_COUNT(kTypeNames); ++index) {
            int typeLen = strlen(kTypeNames[index]);
            if (typeLen == 0) {
                continue;
            }
            if (fChar + typeLen >= fEnd || fChar[typeLen] > ' ') {
                continue;
            }
            int chCompare = strncmp(fChar, kTypeNames[index], typeLen);
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

	bool hasEndToken() const {
		const char* last = fLine + this->lineLength();
		while (last > fLine && ' ' >= *--last)
			;
		if (--last <= fLine) {
			return false;
		}
		return last[0] == fMC && last[1] == fMC;
	}

    ptrdiff_t lineLength() const {
        const char* ptr = fLine;
        do {
            SkASSERT(ptr < fEnd);
            char test = *ptr++;
            if (test == '\n' || test == '\0') {
                break;
            }
        } while (true);
        return ptr - fLine;
    }

    char next() { SkASSERT(fChar < fEnd); return *fChar++; }

	bool parseFromFile(const char* path) {
	    sk_sp<SkData> data = SkData::MakeFromFileName(path);
		fRawData[std::string(path)] = data;
		const char* rawText = (const char*) data->data();
        fStart = rawText;
        fLine = rawText;
        fChar = rawText;
        fEnd = rawText +  data->size();
		fFileName = path;
        fLineCount = 1;
        fMC = '#';
		return findDefinitions();
	}

    char peek() const { SkASSERT(fChar < fEnd); return *fChar; }

	Definition* popParentStack(Definition* parent, Definition* definition) const {
		if (!parent) {
			return this->reportError<Definition*>("missing parent");
		}
		if (definition != parent) {
			return this->reportError<Definition*>("definition end is not parent");
		}
		if (!definition->fStart) {
			return this->reportError<Definition*>("definition missing start");
		}
		if (definition->fEnd) {
			return this->reportError<Definition*>("definition already ended");
		}
		definition->fEnd = fLine;
		return definition->fParent;
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

	void reportError(const char* errorStr) const {
        ReportError(errorStr, fLineCount, this->lineLength(), fLine, fChar);
	}

	template <typename T>
	T reportError(const char* errorStr) const {
        reportError(errorStr);
		return T();
	}

    void reset() {
        fLine = fChar = fStart;
        fLineCount = 0;
    }

    bool skipToDefinitionEnd(MarkType markType) {
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
            if (this->next() != fMC) {
                continue;
            }
            if (this->next() != fMC) {
                continue;
            }
            if (this->peek() == fMC) {
                continue;
            }
            MarkType nextType = this->getMarkType(MarkLookup::kAllowUknown);
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

	std::string typeName(const Definition* parent, bool hasEnd) {
        std::string builder;
        if (parent) {
            builder = parent->fName;
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

	void validate() const {
		for (int index = 0; index < (int) gLast_MarkType; ++index) {
			SkASSERT(fMaps[index].fMarkType == (MarkType) index);
		}
	}

	// mark types aren't required but ensure this stays in sync with the enum
	struct DefinitionMap {
		std::map<std::string, Definition>* fMap;
		MarkType fMarkType;
	} fMaps[SK_ARRAY_COUNT(kTypeNames)] = {
		{ nullptr,		MarkType::kUnknown },
		{ &fClassMap,	MarkType::kClass },
		{ nullptr,		MarkType::kColumn },
		{ nullptr,		MarkType::kComment },
		{ nullptr,		MarkType::kConst },
		{ nullptr,		MarkType::kDefine },
		{ nullptr,		MarkType::kDoxygen },
		{ nullptr,		MarkType::kEnum },
		{ &fExampleMap, MarkType::kExample },
		{ &fMethodMap,	MarkType::kMethod },
		{ nullptr,		MarkType::kRow },
		{ &fStdOutMap,  MarkType::kStdOut },
		{ nullptr,		MarkType::kStruct },
		{ &fTableMap,   MarkType::kTable },
		{ nullptr,		MarkType::kTemplate },
		{ nullptr,		MarkType::kText },
		{ nullptr,		MarkType::kToDo },
		{ nullptr,		MarkType::kTypedef },
		{ nullptr,		MarkType::kUnion },
	};

	std::map<std::string, sk_sp<SkData>> fRawData;
	std::map<std::string, Definition> fClassMap;
	std::map<std::string, Definition> fExampleMap;
	std::map<std::string, Definition> fMethodMap;
	std::map<std::string, Definition> fStdOutMap;
	std::map<std::string, Definition> fTableMap;
	static std::vector<Reference> fReferences;
    const char* fStart;
    const char* fLine;
    const char* fChar;
    const char* fEnd;
	const char* fFileName;
    size_t fLineCount;
    char fMC;  // markup character
};

// pass one: parse text, collect definitions
// pass two: lookup references

int main(int argc, char * const argv[]) {
	if (argc < 2) {
		SkDebugf("missing input file (e.g., overview.txt)\n");
		return -1;
	}
	const char* path = argv[1];
	Parser parser;
	parser.validate();
    if (!sk_isdir(path)) {
        if (!parser.parseFromFile(path)) {
			SkDebugf("failed to parse %s\n", path);
			return -1;
		}
    } else {	
		SkOSFile::Iter it(path, ".bmh");
		for (SkString file; it.next(&file); ) {
			SkString p = SkOSPath::Join(path, file.c_str());
			const char* hunk = p.c_str();
			if (!parser.parseFromFile(hunk)) {
				SkDebugf("failed to parse %s\n", hunk);
				return -1;
			}
		}
	}
	SkDebugf("classes=%d methods=%d examples=%d\n", parser.fClassMap.size(),
			parser.fMethodMap.size(), parser.fExampleMap.size());
	return 0;
}
