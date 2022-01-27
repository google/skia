/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLMangler.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

namespace SkSL {

String Mangler::uniqueName(skstd::string_view baseName, SymbolTable* symbolTable) {
    SkASSERT(symbolTable);
    // The inliner runs more than once, so the base name might already have been mangled and have a
    // prefix like "_123_x". Let's strip that prefix off to make the generated code easier to read.
    if (baseName.starts_with("_")) {
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
    String uniqueName;
    for (;;) {
        uniqueName = String::printf("_%d_%.*s", fCounter++, (int)baseName.size(), baseName.data());
        if ((*symbolTable)[uniqueName] == nullptr) {
            break;
        }
    }

    return uniqueName;
}

} // namespace SkSL
