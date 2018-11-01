/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bmhParser.h"
#include "fiddleParser.h"

// could make this more elaborate and look up the example definition in the bmh file;
// see if a simpler hint provided is sufficient
static bool report_error(const char* blockName, const char* errorMessage) {
    SkDebugf("%s: %s\n", blockName, errorMessage);
    return false;
}

Definition* FiddleBase::findExample(string name) const {
    return fBmhParser->findExample(name);
}

bool FiddleBase::parseFiddles() {
    if (fStack.empty()) {
        return false;
    }
    JsonStatus* status = &fStack.back();
    while (status->fIter != status->fObject.end()) {
        const char* blockName = status->fIter.memberName();
        Definition* example = nullptr;
        string textString;
        if (!status->fObject.isObject()) {
            return report_error(blockName, "expected object");
        }
        for (auto iter = status->fIter->begin(); status->fIter->end() != iter; ++iter) {
            const char* memberName = iter.memberName();
            if (!strcmp("compile_errors", memberName)) {
                if (!iter->isArray()) {
                    return report_error(blockName, "expected array");
                }
                if (iter->size()) {
                    return report_error(blockName, "fiddle compiler error");
                }
                continue;
            }
            if (!strcmp("runtime_error", memberName)) {
                if (!iter->isString()) {
                    return report_error(blockName, "expected string 1");
                }
                if (iter->asString().length()) {
                    return report_error(blockName, "fiddle runtime error");
                }
                continue;
            }
            if (!strcmp("fiddleHash", memberName)) {
                if (!iter->isString()) {
                    return report_error(blockName, "expected string 2");
                }
                example = this->findExample(blockName);
                if (!example) {
                    return report_error(blockName, "missing example");
                }
                if (example->fHash.length() && example->fHash != iter->asString()) {
                    return example->reportError<bool>("mismatched hash");
                }
                example->fHash = iter->asString();
                continue;
            }
            if (!strcmp("text", memberName)) {
                if (!iter->isString()) {
                    return report_error(blockName, "expected string 3");
                }
                textString = iter->asString();
                continue;
            }
            return report_error(blockName, "unexpected key");
        }
        if (!example) {
            return report_error(blockName, "missing fiddleHash");
        }
        size_t strLen = textString.length();
        if (strLen) {
            if (fTextOut
                    && !this->textOut(example, textString.c_str(), textString.c_str() + strLen)) {
                return false;
            }
        } else if (fPngOut && !this->pngOut(example)) {
            return false;
        }
        status->fIter++;
    }
    return true;
}

bool FiddleParser::parseFromFile(const char* path)  {
    if (!INHERITED::parseFromFile(path)) {
        return false;
    }
    fBmhParser->resetExampleHashes();
    if (!INHERITED::parseFiddles()) {
        return false;
    }
    return fBmhParser->checkExampleHashes();
}

bool FiddleParser::textOut(Definition* example, const char* stdOutStart,
        const char* stdOutEnd) {
    bool foundStdOut = false;
    for (auto& textOut : example->fChildren) {
        if (MarkType::kStdOut != textOut->fMarkType) {
            continue;
        }
        foundStdOut = true;
        bool foundVolatile = false;
        for (auto& stdOutChild : textOut->fChildren) {
                if (MarkType::kVolatile == stdOutChild->fMarkType) {
                    foundVolatile = true;
                    break;
                }
        }
        TextParser bmh(textOut);
        EscapeParser fiddle(stdOutStart, stdOutEnd);
        do {
            bmh.skipWhiteSpace();
            fiddle.skipWhiteSpace();
            const char* bmhEnd = bmh.trimmedLineEnd();
            const char* fiddleEnd = fiddle.trimmedLineEnd();
            ptrdiff_t bmhLen = bmhEnd - bmh.fChar;
            SkASSERT(bmhLen > 0);
            ptrdiff_t fiddleLen = fiddleEnd - fiddle.fChar;
            SkASSERT(fiddleLen > 0);
            if (bmhLen != fiddleLen) {
                if (!foundVolatile) {
                    bmh.reportError("mismatched stdout len\n");
                }
            } else  if (strncmp(bmh.fChar, fiddle.fChar, fiddleLen)) {
                if (!foundVolatile) {
                    SkDebugf("%.*s\n", fiddleLen, fiddle.fChar);
                    bmh.reportError("mismatched stdout text\n");
                }
            }
            bmh.skipToLineStart();
            fiddle.skipToLineStart();
        } while (!bmh.eof() && !fiddle.eof());
        if (!foundStdOut) {
            bmh.reportError("bmh %s missing stdout\n");
        } else if (!bmh.eof() || !fiddle.eof()) {
            if (!foundVolatile) {
                bmh.reportError("%s mismatched stdout eof\n");
            }
        }
    }
    return true;
}
