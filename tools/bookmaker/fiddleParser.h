/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef fiddleParser_DEFINED
#define fiddleParser_DEFINED

#include "parserCommon.h"

class BmhParser;

class FiddleBase : public JsonCommon {
protected:
    FiddleBase(BmhParser* bmh)
        : fBmhParser(bmh)
        , fContinuation(false)
        , fTextOut(false)
        , fPngOut(false)
    {
        this->reset();
    }

    void reset() override {
        INHERITED::reset();
    }

    Definition* findExample(string name) const;
    bool parseFiddles();
    virtual bool pngOut(Definition* example) = 0;
    virtual bool textOut(Definition* example, const char* stdOutStart,
        const char* stdOutEnd) = 0;

    BmhParser* fBmhParser;  // must be writable; writes example hash
    string fFullName;
    bool fContinuation;
    bool fTextOut;
    bool fPngOut;
private:
    typedef JsonCommon INHERITED;
};

class FiddleParser : public FiddleBase {
public:
    FiddleParser(BmhParser* bmh) : FiddleBase(bmh) {
       fTextOut = true;
    }

    bool parseFromFile(const char* path) override;

private:
    bool pngOut(Definition* example) override {
        return true;
    }

    bool textOut(Definition* example, const char* stdOutStart,
        const char* stdOutEnd) override;

    typedef FiddleBase INHERITED;
};

class Catalog : public FiddleBase {
public:
    Catalog(BmhParser* bmh) : FiddleBase(bmh) {}

    bool appendFile(string path);
    bool closeCatalog(const char* outDir);
    bool openCatalog(const char* inDir);
    bool openStatus(const char* inDir);

    bool parseFromFile(const char* path) override ;
private:
    bool pngOut(Definition* example) override;
    bool textOut(Definition* example, const char* stdOutStart,
        const char* stdOutEnd) override;

    string fDocsDir;

    typedef FiddleBase INHERITED;
};

#endif
