/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/utils/SkShaderUtils.h"

#include "include/core/SkString.h"
#include "include/private/SkSLString.h"

namespace SkShaderUtils {

class GLSLPrettyPrint {
public:
    GLSLPrettyPrint() {}

    SkSL::String prettify(const SkSL::String& string) {
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
            /* the heart and soul of our prettification algorithm.  The rules should hopefully
             * be self explanatory.  For '#' and '//' tokens we parse until we reach a newline.
             *
             * For long style comments like this one, we search for the ending token.  We also
             * preserve whitespace in these comments WITH THE CAVEAT that we do the newlines
             * ourselves.  This allows us to remain in control of line numbers, and matching
             * tabs Existing tabs in the input string are copied over too, but this will look
             *  funny
             *
             * '{' and '}' are handled in basically the same way.  We add a newline if we aren't
             * on a fresh line, dirty the line, then add a second newline, ie braces are always
             * on their own lines indented properly.  The one funkiness here is structs print
             * with the semicolon on its own line.  Its not a problem for a glsl compiler though
             *
             * '(' and ')' are basically ignored, except as a sign we need to ignore ';' ala
             * in for loops.
             *
             * ';' means add a new line
             *
             * '\t' and '\n' are ignored in general parsing for backwards compatability with
             * existing shader code and we also have a special case for handling whitespace
             * at the beginning of fresh lines.
             *
             * Otherwise just add the new character to the pretty string, indenting if
             * necessary.
             */
            if (fInParseUntilNewline) {
                this->parseUntilNewline();
            } else if (fInParseUntil) {
                this->parseUntil(fInParseUntilToken);
            } else if (this->hasToken("#") || this->hasToken("//")) {
                this->parseUntilNewline();
            } else if (this->hasToken("/*")) {
                this->parseUntil("*/");
            } else if ('{' == fInput[fIndex]) {
                this->newline();
                this->appendChar('{');
                fTabs++;
                this->newline();
            } else if ('}' == fInput[fIndex]) {
                fTabs--;
                this->newline();
                this->appendChar('}');
                this->newline();
            } else if (this->hasToken(")")) {
                parensDepth--;
            } else if (this->hasToken("(")) {
                parensDepth++;
            } else if (!parensDepth && this->hasToken(";")) {
                this->newline();
            } else if ('\t' == fInput[fIndex] || '\n' == fInput[fIndex] ||
                        (fFreshline && ' ' == fInput[fIndex])) {
                fIndex++;
            } else {
                this->appendChar(fInput[fIndex]);
            }
        }

        return fPretty;
    }

private:
    void appendChar(char c) {
        this->tabString();
        fPretty.appendf("%c", fInput[fIndex++]);
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
            if ('\n' == fInput[fIndex]) {
                fIndex++;
                this->newline();
                fInParseUntilNewline = false;
                break;
            }
            fPretty.appendf("%c", fInput[fIndex++]);
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
            if ('\n' == fInput[fIndex]) {
                this->newline();
                this->tabString();
                fIndex++;
            }
            if (this->hasToken(token)) {
                fInParseUntil = false;
                break;
            }
            fFreshline = false;
            fPretty.appendf("%c", fInput[fIndex++]);
            fInParseUntil = true;
            fInParseUntilToken = token;
        }
    }

    // We only tab if on a newline, otherwise consider the line tabbed
    void tabString() {
        if (fFreshline) {
            for (int t = 0; t < fTabs; t++) {
                fPretty.append("\t");
            }
        }
    }

    // newline is really a request to add a newline, if we are on a fresh line there is no reason
    // to add another newline
    void newline() {
        if (!fFreshline) {
            fFreshline = true;
            fPretty.append("\n");
        }
    }

    bool fFreshline;
    int fTabs;
    size_t fIndex, fLength;
    const char* fInput;
    SkSL::String fPretty;

    // Some helpers for parseUntil when we go over a string length
    bool fInParseUntilNewline;
    bool fInParseUntil;
    const char* fInParseUntilToken;
};

SkSL::String PrettyPrint(const SkSL::String& string) {
    GLSLPrettyPrint pp;
    return pp.prettify(string);
}

void VisitLineByLine(const SkSL::String& text,
                     const std::function<void(int lineNumber, const char* lineText)>& visitFn) {
    SkTArray<SkString> lines;
    SkStrSplit(text.c_str(), "\n", kStrict_SkStrSplitMode, &lines);
    for (int i = 0; i < lines.count(); ++i) {
        visitFn(i + 1, lines[i].c_str());
    }
}

SkSL::String BuildShaderErrorMessage(const char* shader, const char* errors) {
    SkSL::String abortText{"Shader compilation error\n"
                           "------------------------\n"};
    VisitLineByLine(shader, [&](int lineNumber, const char* lineText) {
        abortText.appendf("%4i\t%s\n", lineNumber, lineText);
    });
    abortText.appendf("Errors:\n%s", errors);
    return abortText;
}

void PrintShaderBanner(SkSL::ProgramKind programKind) {
    const char* typeName = "Unknown";
    switch (programKind) {
        case SkSL::ProgramKind::kVertex:   typeName = "Vertex";   break;
        case SkSL::ProgramKind::kFragment: typeName = "Fragment"; break;
        default: break;
    }
    SkDebugf("---- %s shader ----------------------------------------------------\n", typeName);
}

}  // namespace SkShaderUtils
