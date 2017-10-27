/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bookmaker.h"

#include "SkOSFile.h"
#include "SkOSPath.h"

const char header[] =
"<!DOCTYPE html>" "\n"
"" "\n"
"<html lang=\"en\" xmlns=\"http://www.w3.org/1999/xhtml\">" "\n"
"<head>" "\n"
"    <meta charset=\"utf-8\" />" "\n"
"    <title></title>" "\n"
"" "\n"
"<script type=\"text/javascript\">" "\n"
"" "\n"
"var data = ["
;

const char trailer[] =
"]" "\n"
"" "\n"
"var mouseDown = false;" "\n"
"" "\n"
"function handleMouseOver() {" "\n"
"}" "\n"
"" "\n"
"function handleMouseClick() {" "\n"
"}" "\n"
"" "\n"
"function start() {" "\n"
"}" "\n"
"" "\n"
"</script>" "\n"
"</head>" "\n"
"" "\n"
"<body onLoad=\"start();\">" "\n"
"" "\n"
"<canvas id=\"canvas\" width=\"750\" height=\"500\"" "\n"
"    onmousedown=\"mouseDown = true\"" "\n"
"    onmouseup=\"mouseDown = false\"" "\n"
"    onmousemove=\"handleMouseOver()\"" "\n"
"    onclick=\"handleMouseClick()\"" "\n"
"    ></canvas >" "\n"
"</body>" "\n"
"</html>"
;

bool FiddleParser::openCatalog(const char* outDir) {
    fFullName = outDir;
    if ('/' != fFullName.back()) {
        fFullName += '/';
    }
    fFullName += "catalog.htm";
    fOut = fopen(fFullName.c_str(), "wb");
    if (!fOut) {
        SkDebugf("could not open output file %s\n", fFullName.c_str());
        return false;
    }
    fContinuation = false;
    this->writeString("{");
    this->lf(1);
    return true;
}


void FiddleParser::closeCatalog() {
    if (fOut) {
        this->lf(1);
        this->writeString("}");
        this->lf(1);
        this->writePending();
        fclose(fOut);
        SkDebugf("wrote %s\n", fFullName.c_str());
        fOut = nullptr;
    }
}

bool FiddleParser::catalog(Definition* example, const char* stdOutStart,
    const char* stdOutEnd) {
    string result;
    if (!example->exampleToScript(&result)) {
        return false;
    }
    if (result.length() > 0) {
        result += ",\n";
        result += "    \"hash\": " + example->fHash + ",\n";
        result += "    \"name\": " + example->fName + "\n";
        result += "}";
        if (fContinuation) {
            this->writeString(",");
            this->lf(1);
        } else {
            fContinuation = true;
        }
        this->writeBlock(result.size(), result.c_str());
    }
    return true;
}
