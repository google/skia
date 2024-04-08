/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/utils/SkShaderUtils.h"

#include "include/core/SkString.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkStringUtils.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLString.h"

#include <cstddef>

using namespace skia_private;

namespace SkShaderUtils {

class GLSLPrettyPrint {
public:
    GLSLPrettyPrint() {}

    std::string prettify(const std::string& string) {
        fTabs = 0;
        fFreshline = true;

        // If a string breaks while in the middle 'parse until' we need to continue parsing on the
        // next string
        fInParseUntilNewline = false;
        fInParseUntil = false;

        int parensDepth = 0;

        // setup pretty state
        fIndex = 0;
        fLength = string.length();
        fInput = string.c_str();

        while (fLength > fIndex) {
            /* The heart and soul of our prettification algorithm.  The rules should hopefully
             * be self explanatory.  For '#' and '//' tokens, we parse until we reach a newline.
             *
             * For long style comments like this one, we search for the ending token.  We also
             * preserve whitespace in these comments WITH THE CAVEAT that we do the newlines
             * ourselves.  This allows us to remain in control of line numbers, and matching
             * tabs.  Existing tabs in the input string are copied over too, but this will look
             * funny.
             *
             * '{' and '}' are handled in basically the same way.  We add a newline if we aren't
             * on a fresh line, dirty the line, then add a second newline, i.e. braces are always
             * on their own lines indented properly.
             *
             * '(' and ')' are basically ignored, except as a sign that we need to ignore ';', since
             * we want to keep for loops on a single line.
             *
             * ';' means add a new line.  If the previous character was a '}', we make sure that the
             * semicolon comes directly after the brace, not on a newline.
             *
             * ',' doesn't add a new line, but does have special handling to ensure it is on the
             * same line as a '}', much like the semicolon.
             *
             * '\t' and '\n' are ignored in general parsing for backwards compatibility with
             * existing shader code.  We also have a special case for handling whitespace at the
             * beginning of fresh lines.
             *
             * Otherwise, just add the new character to the pretty string, indenting if
             * necessary.
             */
            if (fInParseUntilNewline) {
                this->parseUntilNewline();
                continue;
            }
            if (fInParseUntil) {
                this->parseUntil(fInParseUntilToken);
                continue;
            }
            if (this->hasToken("#") || this->hasToken("//")) {
                this->parseUntilNewline();
                continue;
            }
            if (this->hasToken("/*")) {
                this->parseUntil("*/");
                continue;
            }
            if (fInput[fIndex] == '{') {
                this->newline();
                this->appendChar('{');
                fTabs++;
                this->newline();
                continue;
            }
            if (fInput[fIndex] == '}') {
                fTabs--;
                this->newline();
                this->appendChar('}');
                this->newline();
                continue;
            }
            if (fFreshline && fInput[fIndex] == ';') {
                this->undoNewlineAfter('}');
                this->appendChar(fInput[fIndex]);
                this->newline();
                continue;
            }
            if (fFreshline && fInput[fIndex] == ',') {
                this->undoNewlineAfter('}');
                this->appendChar(fInput[fIndex]);
                continue;
            }
            if (this->hasToken(")")) {
                parensDepth--;
                continue;
            }
            if (this->hasToken("(")) {
                parensDepth++;
                continue;
            }
            if (this->hasToken(")")) {
                parensDepth--;
                continue;
            }
            if (!parensDepth && this->hasToken(";")) {
                this->newline();
                continue;
            }
            if (fInput[fIndex] == '\t' || fInput[fIndex] == '\n' ||
                (fFreshline && fInput[fIndex] == ' ')) {
                fIndex++;
                continue;
            }

            this->appendChar(fInput[fIndex]);
        }

        return fPretty;
    }

private:
    void appendChar(char c) {
        this->tabString();
        fPretty += fInput[fIndex++];
        fFreshline = false;
    }

    // hasToken automatically consumes the next token, if it is a match, and then tabs
    // if necessary, before inserting the token into the pretty string
    bool hasToken(const char* token) {
        size_t i = fIndex;
        for (size_t j = 0; token[j] && fLength > i; i++, j++) {
            if (token[j] != fInput[i]) {
                return false;
            }
        }
        this->tabString();
        fIndex = i;
        fPretty.append(token);
        fFreshline = false;
        return true;
    }

    void parseUntilNewline() {
        while (fLength > fIndex) {
            if (fInput[fIndex] == '\n') {
                fIndex++;
                this->newline();
                fInParseUntilNewline = false;
                break;
            }
            fPretty += fInput[fIndex++];
            fInParseUntilNewline = true;
        }
    }

    // this code assumes it is not actually searching for a newline.  If you need to search for a
    // newline, then use the function above.  If you do search for a newline with this function
    // it will consume the entire string and the output will certainly not be prettified
    void parseUntil(const char* token) {
        while (fLength > fIndex) {
            // For embedded newlines,  this code will make sure to embed the newline in the
            // pretty string, increase the linecount, and tab out the next line to the appropriate
            // place
            if (fInput[fIndex] == '\n') {
                this->newline();
                this->tabString();
                fIndex++;
            }
            if (this->hasToken(token)) {
                fInParseUntil = false;
                break;
            }
            fFreshline = false;
            fPretty += fInput[fIndex++];
            fInParseUntil = true;
            fInParseUntilToken = token;
        }
    }

    // We only tab if on a newline, otherwise consider the line tabbed
    void tabString() {
        if (fFreshline) {
            for (int t = 0; t < fTabs; t++) {
                fPretty += '\t';
            }
        }
    }

    // newline is really a request to add a newline, if we are on a fresh line there is no reason
    // to add another newline
    void newline() {
        if (!fFreshline) {
            fFreshline = true;
            fPretty += '\n';
        }
    }

    // undoNewlineAfter() attempts to undo the effects of newline(), if the last character before
    // the newline matches `c`.
    void undoNewlineAfter(char c) {
        if (fFreshline) {
            if (fPretty.size() >= 2 && fPretty.rbegin()[0] == '\n' && fPretty.rbegin()[1] == c) {
                fFreshline = false;
                fPretty.pop_back();
            }
        }
    }

    bool fFreshline;
    int fTabs;
    size_t fIndex, fLength;
    const char* fInput;
    std::string fPretty;

    // Some helpers for parseUntil when we go over a string length
    bool fInParseUntilNewline;
    bool fInParseUntil;
    const char* fInParseUntilToken;
};

std::string PrettyPrint(const std::string& string) {
    GLSLPrettyPrint pp;
    return pp.prettify(string);
}

void VisitLineByLine(const std::string& text,
                     const std::function<void(int lineNumber, const char* lineText)>& visitFn) {
    TArray<SkString> lines;
    SkStrSplit(text.c_str(), "\n", kStrict_SkStrSplitMode, &lines);
    for (int i = 0; i < lines.size(); ++i) {
        visitFn(i + 1, lines[i].c_str());
    }
}

std::string BuildShaderErrorMessage(const char* shader, const char* errors) {
    std::string abortText{"Shader compilation error\n"
                          "------------------------\n"};
    VisitLineByLine(shader, [&](int lineNumber, const char* lineText) {
        SkSL::String::appendf(&abortText, "%4i\t%s\n", lineNumber, lineText);
    });
    SkSL::String::appendf(&abortText, "Errors:\n%s", errors);
    return abortText;
}

void PrintShaderBanner(SkSL::ProgramKind programKind) {
    const char* typeName = "Unknown";
    if (SkSL::ProgramConfig::IsVertex(programKind)) {
        typeName = "Vertex";
    } else if (SkSL::ProgramConfig::IsFragment(programKind)) {
        typeName = "Fragment";
    }
    SkDebugf("---- %s shader ----------------------------------------------------\n", typeName);
}

}  // namespace SkShaderUtils
