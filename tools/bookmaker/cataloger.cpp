/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bookmaker.h"

#include "SkOSFile.h"
#include "SkOSPath.h"

bool Catalog::appendFile(const string& path) {
    FILE* file = fopen(path.c_str(), "r");
    if (!file) {
        SkDebugf("could not append %s\n", path.c_str());
        return false;
    }
    fseek(file, 0L, SEEK_END);
    int sz = (int) ftell(file);
    rewind(file);
    char* buffer = new char[sz];
    memset(buffer, ' ', sz);
    SkAssertResult(sz == (int)fread(buffer, 1, sz, file));
    fclose(file);
    this->writeBlock(sz, buffer);
    return true;
}

bool Catalog::openCatalog(const char* inDir, const char* outDir) {
    fDocsDir = inDir;
    if ('/' != fDocsDir.back()) {
        fDocsDir += '/';
    }
    string outie = outDir;
    if ('/' != outie.back()) {
        outie += '/';
    }
    fFullName = outie + "catalog.htm";
    fOut = fopen(fFullName.c_str(), "wb");
    if (!fOut) {
        SkDebugf("could not open output file %s\n", fFullName.c_str());
        return false;
    }
    fContinuation = false;
    if (!appendFile(fDocsDir + "catalogHeader.txt")) {
        return false;
    }
    this->lf(1);
    return true;
}

bool Catalog::openStatus(const char* statusFile, const char* outDir) {
    StatusIter iter(statusFile, ".bmh", StatusFilter::kInProgress);
    string unused;
    // FIXME: iterate through only chosen files by setting fDocsDir to iter
    // read one file to find directory
    if (!iter.next(&unused)) {
        return false;
    }
    return openCatalog(iter.baseDir().c_str(), outDir);
}

bool Catalog::closeCatalog() {
    if (fOut) {
        this->lf(1);
        this->writeString("}");
        this->lf(1);
        if (!appendFile(fDocsDir + "catalogTrailer.txt")) {
            return false;
        }
        this->lf(1);
        this->writePending();
        fclose(fOut);
        SkDebugf("wrote %s\n", fFullName.c_str());
        fOut = nullptr;
    }
    return true;
}

bool Catalog::parseFromFile(const char* path) {
    if (!INHERITED::parseSetup(path)) {
        return false;
    }
    fIndent = 4;
    this->writeString("var text = {");
    this->lf(1);
    fTextOut = true;
    TextParser::Save save(this);
    if (!parseFiddles()) {
        return false;
    }
    this->lf(1);
    this->writeString("}");
    this->lf(2);
    this->writeString("var pngs = {");
    fTextOut = false;
    fPngOut = true;
    save.restore();
    fContinuation = false;
    return parseFiddles();
}

bool Catalog::pngOut(Definition* example) {
    string result;
    if (!example->exampleToScript(&result, Definition::ExampleOptions::kPng)) {
        return false;
    }
    if (result.length() > 0) {
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

bool Catalog::textOut(Definition* def, const char* stdOutStart,
    const char* stdOutEnd) {
    string result;
    if (!def->exampleToScript(&result, Definition::ExampleOptions::kText)) {
        return false;
    }
    if (result.length() > 0) {
        if (fContinuation) {
            this->writeString(",");
            this->lf(1);
        } else {
            fContinuation = true;
        }
        fIndent = 8;
        this->writeBlock(result.size(), result.c_str());
        this->lf(1);
        this->writeString("\"stdout\": \"");
        size_t pos = 0;
        size_t len = stdOutEnd - stdOutStart;
        string example;
        const Definition* stdOut = def->hasChild(MarkType::kStdOut);
        const Definition* outVolatile = stdOut ? stdOut->hasChild(MarkType::kVolatile) : nullptr;
        if (outVolatile) {
            stdOutStart = outVolatile->fContentStart;
            while ('\n' == stdOutStart[0]) {
                ++stdOutStart;
            }
            len = stdOut->fContentEnd - stdOutStart;
        }
        while ((size_t) pos < len) {
            example += '"' == stdOutStart[pos] ? "\\\"" :
                '\\' == stdOutStart[pos] ? "\\\\" :
                '\n' == stdOutStart[pos] ? "\\\\n" :
                string(&stdOutStart[pos], 1);
            if (outVolatile && '\n' == stdOutStart[pos]) {
                ++pos;
                while ((size_t) pos < len && ' ' == stdOutStart[pos]) {
                    ++pos;
                }
                continue;
            }
            ++pos;
        }
        if (outVolatile) {
            example += "\\\\n";
        }
        this->writeBlock(example.length(), example.c_str());
        this->writeString("\"");
        this->lf(1);
        fIndent = 4;
        this->writeString("}");
    }
    return true;
}
