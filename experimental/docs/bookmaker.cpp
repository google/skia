/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkData.h"
// #include "SkTDArray.h"

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
    const char* fLocation;  // .. in original text file
    MarkType fMarkType;
};

struct Reference {
    const char* fLocation;  // .. in original text file
    const Definition* fDefinition;
};

struct RawFile {
    RawFile(const char* data, size_t size)
        : fStart(data)
        , fLine(data)
        , fChar(data)
        , fEnd(data + size)
        , fLineCount(0) {
    }

    bool eof() const { return fChar >= fEnd; }

    MarkType getMarkType(bool inComment) const {
        for (int index = 0; index < (int) SK_ARRAY_COUNT(kTypeNames); ++index) {
            int typeLen = strlen(kTypeNames[index]);
            if (typeLen == 0) {
                continue;
            }
            if (fChar + typeLen >= fEnd || fChar[typeLen] > ' ') {
                continue;
            }
            int chCompare = strncmp(fChar, kTypeNames[index], typeLen);
            if (chCompare > 0) {
                goto fail;
            }
            if (chCompare == 0) {
                return (MarkType) index;
            }
        }
fail:
        if (!inComment) {
            SkDebugf("unknown mark type %.*s at line %d\n", this->lineLength(), fLine, fLineCount);
        }
        return MarkType::kUnknown;
    }

    char next() { SkASSERT(fChar < fEnd); return *fChar++; }
    char peek() const { SkASSERT(fChar < fEnd); return *fChar; }

    void skipToDefinitionEnd(MarkType markType) {
        if (this->eof()) {
            SkDebugf("unexpected end of file looking for %s End\n", kTypeNames[(int) markType]);
            return;
        }
        const char* start = fLine;
        ptrdiff_t startLen = this->lineLength();
        int startLineCount = fLineCount;
        int stack = 1;
        ptrdiff_t lineLen;
        do {
            lineLen = this->lineLength();
            if (this->next() != '#') {
                continue;
            }
            if (this->next() != '#') {
                continue;
            }
            if (this->peek() == '#') {
                SkDebugf("### found : %.*s at line %d\n", lineLen, fLine, fLineCount);
                return;
            }
            MarkType nextType = this->getMarkType(true);
            if (markType != nextType) {
                continue;
            }
            const char* lineEnd = fLine + lineLen;
            while (*--lineEnd <= ' ') 
                ;
            if (!strncmp(lineEnd - 3, "End", 3)) {
                if (!--stack) {
                    return;
                }
            } else {
                ++stack;
            }
        } while (++fLineCount, fLine += lineLen + 1, fChar = fLine, !this->eof());
        SkDebugf("unbalanced stack starting at %.*s line %d\n", startLen, start, startLineCount);
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

    void reset() {
        fLine = fChar = fStart;
        fLineCount = 0;
    }

    size_t tokenLength() const {
        const char* ptr = fChar;
        do {
            SkASSERT(ptr < fEnd);
            if (*ptr++ <= ' ') {
                break;
            }
        } while (true);
        return ptr - fChar;
    }

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
    sk_sp<SkData> overview = SkData::MakeFromFileName("C:/puregit/experimental/docs/overview.txt");
    RawFile rawFile((const char*) overview->data(), overview->size());
    int definitions = 0;
    int references = 0;
    bool lineStart = true;
    while (!rawFile.eof()) {
        if (rawFile.peek() == '#') {
            rawFile.next();
            if (rawFile.peek() == '#') {  // definition
                if (!lineStart) {
                    SkDebugf("expected definition at line %d at start: \"%.*s\"", rawFile.fLineCount,
                            rawFile.lineLength(), rawFile.fLine);
                }
                ++definitions;
                rawFile.next();
                MarkType markType = rawFile.getMarkType(false);
                if (MarkType::kComment == markType) {
                    rawFile.skipToDefinitionEnd(markType);
                }
            } else {  // reference
                ++references;
                (void) rawFile.getMarkType(false);
            }
        } else {
            if ((lineStart = rawFile.next() == '\n')) {
                ++rawFile.fLineCount;
                rawFile.fLine = rawFile.fChar;
            }
        }
    }
}
