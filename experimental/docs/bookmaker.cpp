/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkData.h"

#include <map>
#include <vector>

/*
find include/core -type f -name '*.h' -print -exec git blame {} \; > ~/all.blame.txt
 */

enum class MarkType {
    kClass,
    kColumn,
    kComment,
    kDoxygen,
    kExample,
    kMethod,
    kRow,
    kStdOut,
    kTable,
    kText,
    kToDo,
    kUnknown,
};

const char* kTypeNames[] = {
    "Class",
    "",         // names without formal defintions (e.g. Column) aren't included
    "Comment",
    "Doxygen",
    "Example",
    "Method",
    "",
    "StdOut",
    "Table",
    "",
    "ToDo",
    "Unknown",
};

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

struct RawFile {
    RawFile(const char* data, size_t size)
        : fStart(data)
        , fLine(data)
        , fChar(data)
        , fEnd(data + size)
        , fLineCount(1)
{
    }

	int endHashCount() const {
		const char* end = fLine + this->lineLength();
		int count = 0;
		while (fLine < end && '#' == *--end) {
			count++;
		}
		return count;
	}

    bool eof() const { return fChar >= fEnd; }

	bool findDefinitions() {
		bool lineStart = true;
		Definition* parent = nullptr;
		while (!this->eof()) {
			if (this->peek() == '#') {
				this->next();
				if (this->peek() == '#') {  // definition
					if (!lineStart) {
						return this->reportErrorMain("expected definition");
					}
					this->next();
					MarkType markType = this->getMarkType(false);
					std::string typeName = this->typeName();
					Definition* definition = nullptr;
					if (!typeName.length() && MarkType::kComment != markType) {
						if (!parent) {
							return this->reportErrorMain("unnamed definition");
						}
						typeName = this->typeName(parent);
						if (!typeName.length()) {
							return this->reportErrorMain("unnamed parent");
						}
					}
					switch (markType) {
						case MarkType::kComment:
							if (!this->skipToDefinitionEnd(markType)) {
								return false;
							}
							continue;
						case MarkType::kClass:
							definition = &fClassMap[typeName];
							break;
						case MarkType::kExample:
							definition = &fExampleMap[typeName];
							break;
						case MarkType::kMethod:
							definition = &fMethodMap[typeName];
							break;
						case MarkType::kStdOut:
							definition = &fStdOutMap[typeName];
							break;
						case MarkType::kTable:
							definition = &fTableMap[typeName];
							break;
						default:
							continue;
					}
					bool hasEnd = this->hasEndToken();
					if (hasEnd) {
						if (!parent) {
							return this->reportErrorMain("missing parent");
						}
						if (definition != parent) {
							return this->reportErrorMain("definition end is not parent");
						}
						if (!definition->fStart) {
							return this->reportErrorMain("definition missing start");
						}
						if (definition->fEnd) {
							return this->reportErrorMain("definition already ended");
						}
						definition->fEnd = fChar;
						parent = definition->fParent;
					} else {
						definition->fStart = fChar;
						definition->fMarkType = markType;
						definition->fParent = parent;
						if (parent) {
							definition->fSibling = parent->fChild; // constructed in reverse order
							parent->fChild = definition;  // fix up child order once done
						}
						parent = definition;
						if (MarkType::kTable == markType && !this->skipToDefinitionEnd(markType)) {
							return false;
						}
					}
					continue;
				} else if (this->peek() == ' ' && lineStart) {
					int endHashes = this->endHashCount();
					if (endHashes <= 1) {  // one line comment
						fChar = fLine + this->lineLength() - 1;
					} else {  // table row
						if (2 != endHashes) {
							return this->reportErrorMain("expect ##");
						}
						if (!parent || MarkType::kTable != parent->fMarkType) {
							return this->reportErrorMain("missing table");
						}
					}
				} else {  // reference
					(void) this->getMarkType(false);
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

    MarkType getMarkType(bool inComment) const {
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
        if (!inComment) {
            reportError("unknown mark type");
        }
        return MarkType::kUnknown;
    }

	bool hasEndToken() const {
		return 0 == strncmp("End", fLine + this->lineLength() - 3, 3);
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
    char peek() const { SkASSERT(fChar < fEnd); return *fChar; }

	static void ReportError(const char* errorStr, int lineCount, int lineLen, 
			const char* line, const char* ch) {
        SkDebugf("%s at line %d: \"%.*s\"\n", errorStr,
				lineCount, lineLen - 1, line);
		if (ch != line) {
			SkDebugf("%*s %c\n", ch - line, "", '^');
		}
	}

	void reportError(const char* errorStr) const {
        ReportError(errorStr, fLineCount, this->lineLength(), fLine, fChar);
	}

	bool reportErrorBool(const char* errorStr) const {
        reportError(errorStr);
		return false;
	}

	int reportErrorMain(const char* errorStr) const {
        reportError(errorStr);
		return -1;
	}

	std::string reportErrorString(const char* errorStr) const {
        reportError(errorStr);
		return std::string();
	}

    void reset() {
        fLine = fChar = fStart;
        fLineCount = 0;
    }

    bool skipToDefinitionEnd(MarkType markType) {
        if (this->eof()) {
            return reportErrorBool("missing end");
        }
        const char* start = fLine;
        ptrdiff_t startLen = this->lineLength();
        int startLineCount = fLineCount;
        int stack = 1;
        ptrdiff_t lineLen;
		bool foundEnd = false;
        do {
            lineLen = this->lineLength();
            if (this->next() != '#') {
                continue;
            }
            if (this->next() != '#') {
                continue;
            }
            if (this->peek() == '#') {
                return reportErrorBool("### found");
            }
            MarkType nextType = this->getMarkType(true);
            if (markType != nextType) {
                continue;
            }
            const char* lineEnd = fLine + lineLen;
            while (*--lineEnd <= ' ') 
                ;
			const char* endToken = lineEnd - 2;
            if (!strncmp(endToken, "End", 3)) {
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

	std::string typeName() {
		const char* lineEnd = fLine + this->lineLength();
		SkAssertResult('#' == fChar[-1] && '#' == fChar[-2]);  // hashes in mark type
		SkASSERT('#' != this->peek());
		// look for a single # before the line end
		while (fChar < lineEnd && '#' != this->next())
			;
		if ('#' != fChar[-1]) {
			return std::string();
		}
		if ('#' == fChar[0]) {
			return reportErrorString("unexpected #");
		}
		if (' ' >= fChar[0]) {
			return std::string();
		}
		const char* nameStart = fChar;
		while (fChar < lineEnd && ' ' < this->next())
			;
		const char* firstSpace = ' ' >= fChar[-1] ? &fChar[-1] : nullptr;
		// look for an optional second # before the line end
		while (fChar < lineEnd && '#' != this->next())
			;
		if ('#' == fChar[-1] && '#' == fChar[0]) {
			return reportErrorString("unexpected ##");
		}
		const char* nameEnd = '#' == fChar[-1] && ' ' < fChar[-2] ? &fChar[-1] : firstSpace;
		if (!nameEnd) {
			return reportErrorString("missing name end");
		}
		return std::string(nameStart, nameEnd - nameStart);
	}

	std::string typeName(const Definition* definition) {
		const char* saveChar = fChar;
		const char* saveLine = fLine;
		fChar = fLine = definition->fStart;
		std::string result = typeName();
		fChar = saveChar;
		fLine = saveLine;
		return result;
	}


	std::map<std::string, Definition> fClassMap;
	std::map<std::string, Definition> fExampleMap;
	std::map<std::string, Definition> fMethodMap;
	std::map<std::string, Definition> fStdOutMap;
	std::map<std::string, Definition> fTableMap;
	static std::vector<Reference> fReferences;
    char const * const fStart;
    const char* fLine;
    const char* fChar;
    char const * const fEnd;
    size_t fLineCount;
};

// pass zero: count the number of definitions, references
// pass one: parse text, collect definitions
// pass two: lookup references

int main(int argc, char * const argv[]) {
	if (argc < 2) {
		SkDebugf("missing input file (e.g., overview.txt)\n");
		return -1;
	}
    sk_sp<SkData> overview = SkData::MakeFromFileName(argv[1]);
	if (!overview) {
		SkDebugf("could not open %s\n", argv[1]);
		return -1;
	}
    RawFile rawFile((const char*) overview->data(), overview->size());
	if (!rawFile.findDefinitions()) {
		SkDebugf("findDefinitions could not parse %s\n", argv[1]);
		return -1;
	}
	SkDebugf("classes=%d methods=%d examples=%d\n", rawFile.fClassMap.size(),
			rawFile.fMethodMap.size(), rawFile.fExampleMap.size());
}
