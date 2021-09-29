/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INTRINSICMAP
#define SKSL_INTRINSICMAP

#include <memory>
#include <unordered_map>

namespace SkSL {

class ProgramElement;
class String;

/**
 * Represents the intrinsics in the Context.
 */
class IntrinsicMap {
public:
    IntrinsicMap(IntrinsicMap* parent) : fParent(parent) {}

    void insertOrDie(String key, std::unique_ptr<ProgramElement> element);

    const ProgramElement* find(const String& key);

    const ProgramElement* findAndInclude(const String& key);

    void resetAlreadyIncluded();

private:
    struct Intrinsic {
        std::unique_ptr<ProgramElement> fIntrinsic;
        bool fAlreadyIncluded = false;
    };

    std::unordered_map<String, Intrinsic> fIntrinsics;
    IntrinsicMap* fParent = nullptr;
};

} // namespace SkSL

#endif
