/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/TestFontDataProvider.h"

#include "include/core/SkData.h"
#include "include/private/base/SkDebug.h"
#include "src/utils/SkOSPath.h"
#include "tools/flags/CommandLineFlags.h"

const char kTestDataJsonFilename[] = "raster_test.json";
const char kTestDataBasePath[] = "third_party/externals/googlefonts_testdata/data/";

std::atomic<const char*> gFontTestDataBasePath{kTestDataBasePath};

namespace skiatest {
static DEFINE_string(fontTestDataPath,
                     "",
                     "Path to extracted data from googlefonts_testdata CIPD file.");

void SetFontTestDataDirectory() {
    if (FLAGS_fontTestDataPath.isEmpty()) {
        return;
    }
    if (strlen(FLAGS_fontTestDataPath[0])) {
        gFontTestDataBasePath = FLAGS_fontTestDataPath[0];
    }
}
}  // namespace skiatest

// Change size when rolling / changing googlefonts_testdata CIPD, see bin/fetch-fonts-testdata.
constexpr size_t kExpectNumFonts = 51;

namespace {
SkString prefixWithTestDataPath(SkString suffix) {
    return SkOSPath::Join(gFontTestDataBasePath, suffix.c_str());
}

SkString prefixWithFontsPath(SkString suffix) {
    SkString fontsPath = prefixWithTestDataPath(SkString("fonts"));
    return SkOSPath::Join(fontsPath.c_str(), suffix.c_str());
}

}  // namespace

TestFontDataProvider::TestFontDataProvider(const std::string& fontFilterRegexp,
                                           const std::string& langFilterRegexp)
        : fFontFilter(fontFilterRegexp), fLangFilter(langFilterRegexp) {
    SkString testDataLocation = prefixWithTestDataPath(SkString(kTestDataJsonFilename));
    sk_sp<SkData> jsonTestData = SkData::MakeFromFileName(testDataLocation.c_str());
    if (!jsonTestData || !jsonTestData->size()) {
        SkDebugf("Unable to access font test data at %s, check bin/fetch-fonts-testdata.\n",
                 testDataLocation.c_str());
        return;
    }
    fJsonDom = std::make_unique<skjson::DOM>(reinterpret_cast<const char*>(jsonTestData->bytes()),
                                             jsonTestData->size());
    const skjson::ObjectValue& root = fJsonDom->root().as<skjson::ObjectValue>();
    fFonts = root["fonts"];
    if (!fFonts) {
        SkDebugf("Unable to read font test data.\n");
        return;
    }
    if (fFonts->size() != kExpectNumFonts) {
        SkDebugf("Unable to access all %zu test fonts (got %zu), check bin/fetch-fonts-testdata.\n",
                 kExpectNumFonts, fFonts->size());
        return;
    }
    fSamples = root["samples"];
    if (!fSamples || !fSamples->size()) {
        SkDebugf("Unable to read font test data samples.\n");
        return;
    }
}

bool TestFontDataProvider::next(TestSet* testSet) {
    while (testSet && fFonts && fFontsIndex < fFonts->size()) {
        const skjson::ObjectValue* fontsEntry = (*fFonts)[fFontsIndex++];
        SkASSERT(fontsEntry);
        const skjson::StringValue* fontName = (*fontsEntry)["name"];
        SkASSERT(fontName);
        std::smatch match;
        std::string fontNameStr(fontName->str());
        if (std::regex_match(fontNameStr, match, fFontFilter)) {
            testSet->fontName = SkString(fontNameStr);
            const skjson::StringValue* fontFilename = (*fontsEntry)["path"];
            testSet->fontFilename = prefixWithFontsPath(
                    SkString(fontFilename->str().data(), fontFilename->str().size()));
            testSet->langSamples =
                    getLanguageSamples((*fontsEntry)["languages"].as<skjson::ArrayValue>());
            return true;
        }
    }
    return false;
}

void TestFontDataProvider::rewind() { fFontsIndex = 0; }

std::vector<TestFontDataProvider::LangSample> TestFontDataProvider::getLanguageSamples(
        const skjson::ArrayValue* languages) {
    std::vector<LangSample> samples;
    for (size_t i = 0; i < languages->size(); ++i) {
        const skjson::StringValue* langTag = (*languages)[i];
        std::string langTagStr(langTag->str());
        std::smatch match;
        if (std::regex_match(langTagStr, match, fLangFilter)) {
            const skjson::ObjectValue* sample = (*fSamples)[langTagStr.c_str()];
            const skjson::StringValue* shortSample = (*sample)["short_sample"];
            const skjson::StringValue* longSample = (*sample)["long_sample"];
            SkString sampleShort(shortSample->str().data(), shortSample->str().size());
            SkString sampleLong(longSample->str().data(), longSample->str().size());
            samples.push_back({SkString(langTagStr), sampleShort, sampleLong});
        }
    }
    SkASSERT_RELEASE(samples.size());
    return samples;
}
