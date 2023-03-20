/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLMangler.h"

#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "src/base/SkStringView.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

#include <algorithm>
#include <cstring>
#include <ctype.h>

namespace SkSL {

std::string Mangler::uniqueName(std::string_view baseName, SymbolTable* symbolTable) {
    SkASSERT(symbolTable);

    // Private names might begin with a $. Strip that off.
    if (skstd::starts_with(baseName, '$')) {
        baseName.remove_prefix(1);
    }

    // The inliner runs more than once, so the base name might already have been mangled and have a
    // prefix like "_123_x". Let's strip that prefix off to make the generated code easier to read.
    if (skstd::starts_with(baseName, '_')) {
        // Determine if we have a string of digits.
        int offset = 1;
        while (isdigit(baseName[offset])) {
            ++offset;
        }
        // If we found digits, another underscore, and anything else, that's the mangler prefix.
        // Strip it off.
        if (offset > 1 && baseName[offset] == '_' && baseName[offset + 1] != '\0') {
            baseName.remove_prefix(offset + 1);
        } else {
            // This name doesn't contain a mangler prefix, but it does start with an underscore.
            // OpenGL disallows two consecutive underscores anywhere in the string, and we'll be
            // adding one as part of the mangler prefix, so strip the leading underscore.
            baseName.remove_prefix(1);
        }
    }

    // Append a unique numeric prefix to avoid name overlap. Check the symbol table to make sure
    // we're not reusing an existing name. (Note that within a single compilation pass, this check
    // isn't fully comprehensive, as code isn't always generated in top-to-bottom order.)

    // This code is a performance hotspot. Assemble the string manually to save a few cycles.
    char uniqueName[256];
    uniqueName[0] = '_';
    char* uniqueNameEnd = uniqueName + std::size(uniqueName);
    for (;;) {
        // _123
        char* endPtr = SkStrAppendS32(uniqueName + 1, fCounter++);

        // _123_
        *endPtr++ = '_';

        // _123_baseNameTruncatedToFit (no null terminator, because string_view doesn't require one)
        int baseNameCopyLength = std::min<int>(baseName.size(), uniqueNameEnd - endPtr);
        memcpy(endPtr, baseName.data(), baseNameCopyLength);
        endPtr += baseNameCopyLength;

        std::string_view uniqueNameView(uniqueName, endPtr - uniqueName);
        if (symbolTable->find(uniqueNameView) == nullptr) {
            return std::string(uniqueNameView);
        }
    }
}

} // namespace SkSL
