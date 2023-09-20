/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef RasterTestData_DEFINED
#define RasterTestData_DEFINED

#include "include/core/SkString.h"
#include "src/utils/SkJSON.h"

#include <regex>
#include <string>

// Iterates over test data from the font tests CIPD, allows filtering for fonts and test samples
// languages using C++ regular expressions.
class RasterTestDataIterator {
public:
    struct LangSample {
        SkString langTag;
        SkString sampleShort;
        SkString sampleLong;
    };

    struct TestSet {
        SkString fontName;
        SkString fontFilename;
        std::vector<LangSample> langSamples;
    };

    RasterTestDataIterator(const std::string fontFilterRegexp, const std::string langFilterRegexp);

    bool next(TestSet* testSet);

    void rewind();

private:
    std::vector<LangSample> getLanguageSamples(const skjson::ArrayValue* languages);
    std::regex fFontFilter;
    std::regex fLangFilter;
    size_t fFontsIndex = 0;
    std::unique_ptr<skjson::DOM> fJsonDom;
    const skjson::ArrayValue* fFonts;
    const skjson::ObjectValue* fSamples;
};

#endif
