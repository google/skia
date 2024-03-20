// Copyright 2023 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/core/SkString.h"
#include "include/private/base/SkTo.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "src/base/SkBitmaskEnum.h"
#include "src/base/SkTime.h"
#include "tools/unicode_comparison/cpp/bridge.h"

namespace {
static std::unique_ptr<SkUnicode> gUnicode = nullptr;
static skia_private::TArray<SkUnicode::CodeUnitFlags, true> gCodeUnitFlags;
static std::vector<SkUnicode::Position> gSentences;
static std::vector<SkUnicode::Position> gWords;
}

bool init_skunicode_impl(char* impl) {

    SkString unicodeName(impl);
    if (unicodeName.equals("icu")) {
        gUnicode = SkUnicode::MakeIcuBasedUnicode();
    } else if (unicodeName.equals("libgrapheme")) {
        gUnicode = SkUnicode::MakeLibgraphemeBasedUnicode();
    } else {
        SkDebugf("Implementation '%s' not supported\n", impl);
        return false;
    }
    auto ptr = reinterpret_cast<void*>(gUnicode.get());
    if (ptr == nullptr) {
        SkDebugf("Could not create Unicode object\n");
        return false;
    }
    return true;
}

void cleanup_unicode_impl() {
    if (gUnicode == nullptr) {
        SkDebugf("Unicode object does not exist\n");
        return;
    }
    delete gUnicode.get();
}

double perf_compute_codeunit_flags(char* text) {
    if (gUnicode == nullptr) {
        SkDebugf("Unicode object does not exist\n");
        return -1;
    }
    double time = SkTime::GetNSecs();
    gUnicode->computeCodeUnitFlags(text, strlen(text), false, &gCodeUnitFlags);
    if (gCodeUnitFlags.size() < strlen(text)) {
        SkDebugf("computeCodeUnitFlags failed: %d < %zu\n%s\n\n\n", gCodeUnitFlags.size(), strlen(text),  text);
        return -1;
    }
    std::vector<SkUnicode::Position> positions;
    gUnicode->getUtf8Words(text, strlen(text), nullptr, &positions);
    double result = SkTime::GetNSecs() - time;
    for (auto pos : positions) {
        gCodeUnitFlags[pos] |= SkUnicode::CodeUnitFlags::kWordBreak;
    }
    return result;
}

int getFlags(int index) {
    if (gUnicode == nullptr) {
        SK_ABORT("Unicode object does not exist");
    } else if (gCodeUnitFlags.size() == 0) {
        SK_ABORT("Unicode object is empty or not initialized\n");
    } else if (index < 0 || index >= gCodeUnitFlags.size()) {
        SK_ABORT("Index value %d outside of valid range [%d:%d)\n", index, 0, gCodeUnitFlags.size());
    }
    return gCodeUnitFlags[index];
}

void* getSentences(char* text, int* length) {
    if (gUnicode == nullptr) {
        SkDebugf("Unicode object does not exist");
        return nullptr;
    }

    gSentences.clear();
    gUnicode->getSentences(text, strlen(text), nullptr, &gSentences);
    *length = gSentences.size();

    return reinterpret_cast<SkUnicode::Position*>(gSentences.data());
}

bool trimSentence(char* text, int* sentence, int wordLimit) {
    *sentence = 0;
    if (gUnicode == nullptr) {
        SkDebugf("Unicode object does not exist");
        return true;
    }

    gWords.clear();
    gUnicode->getUtf8Words(text, strlen(text), nullptr, &gWords);

    for (auto word : gWords) {
        if (word > wordLimit) {
            return true;
        } else {
            *sentence = word;
        }
    }
    if (strlen(text) <= wordLimit) {
        *sentence = strlen(text);
        return false;
    }
    return true;
}


void* toUpper(char* str) {
    if (gUnicode == nullptr) {
        SkDebugf("Unicode object does not exist");
        return nullptr;
    }
    auto res = new SkString(gUnicode->toUpper(SkString(str)));
    return reinterpret_cast<void*>(res);
}

void  print(void* str) {
    auto ptr = reinterpret_cast<SkString*>(str);
    SkDebugf("%s\n", ptr->c_str());
}
