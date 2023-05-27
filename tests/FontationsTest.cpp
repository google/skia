/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkTypeface_fontations.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <memory>

namespace {
const char kFontResource[] = "fonts/ahem.ttf";
const char kTtcResource[] = "fonts/test.ttc";
}  // namespace

DEF_TEST(Fontations_DoNotMakeFromNull, reporter) {
    std::unique_ptr<SkStreamAsset> nullStream = SkMemoryStream::MakeDirect(nullptr, 0);
    sk_sp<SkTypeface> probeTypeface(
            SkTypeface_Make_Fontations(std::move(nullStream), SkFontArguments()));
    REPORTER_ASSERT(reporter, !probeTypeface);
}

DEF_TEST(Fontations_DoNotMakeFromNonSfnt, reporter) {
    char notAnSfnt[] = "I_AM_NOT_AN_SFNT";
    std::unique_ptr<SkStreamAsset> notSfntStream =
            SkMemoryStream::MakeDirect(notAnSfnt, std::size(notAnSfnt));
    sk_sp<SkTypeface> probeTypeface(
            SkTypeface_Make_Fontations(std::move(notSfntStream), SkFontArguments()));
    REPORTER_ASSERT(reporter, !probeTypeface);
}

DEF_TEST(Fontations_MakeFromFont, reporter) {
    sk_sp<SkTypeface> probeTypeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kFontResource), SkFontArguments()));
    REPORTER_ASSERT(reporter, probeTypeface);
}

DEF_TEST(Fontations_MakeFromCollection, reporter) {
    sk_sp<SkTypeface> probeTypeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kTtcResource), SkFontArguments()));
    REPORTER_ASSERT(reporter, probeTypeface);
}

DEF_TEST(Fontations_MakeFromCollectionNonNullIndex, reporter) {
    SkFontArguments args;
    args.setCollectionIndex(1);
    sk_sp<SkTypeface> probeTypeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kTtcResource), args));
    REPORTER_ASSERT(reporter, probeTypeface);
}

DEF_TEST(Fontations_DoNotMakeFromCollection_Invalid_Index, reporter) {
    SkFontArguments args;
    args.setCollectionIndex(1000);
    sk_sp<SkTypeface> probeTypeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kTtcResource), args));
    REPORTER_ASSERT(reporter, !probeTypeface);
}
