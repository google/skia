/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PROGRAMUSAGE
#define SKSL_PROGRAMUSAGE

#include "include/core/SkTypes.h"
#include "src/core/SkTHash.h"

namespace SkSL {

class Expression;
class FunctionDeclaration;
class ProgramElement;
class Statement;
class Symbol;
class Variable;

/**
 * Side-car class holding mutable information about a Program's IR
 */
class ProgramUsage {
public:
    struct VariableCounts {
        int fVarExists = 0;  // if this is zero, the Variable might have already been deleted
        int fRead = 0;
        int fWrite = 0;
    };
    VariableCounts get(const Variable&) const;
    bool isDead(const Variable&) const;

    int get(const FunctionDeclaration&) const;

    void add(const Expression* expr);
    void add(const Statement* stmt);
    void add(const ProgramElement& element);
    void remove(const Expression* expr);
    void remove(const Statement* stmt);
    void remove(const ProgramElement& element);

    bool operator==(const ProgramUsage& that) const;
    bool operator!=(const ProgramUsage& that) const { return !(*this == that); }

    // All Symbol* objects in fStructCounts must be StructType*.
    skia_private::THashMap<const Symbol*, int> fStructCounts;
    // All Symbol* objects in fCallCounts must be FunctionDeclaration*.
    skia_private::THashMap<const Symbol*, int> fCallCounts;
    skia_private::THashMap<const Variable*, VariableCounts> fVariableCounts;
};

}  // namespace SkSL

#endif
