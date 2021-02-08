/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DEFINITION_MAP
#define SKSL_DEFINITION_MAP

#include <memory>

#include "include/private/SkTHash.h"

namespace SkSL {

class BasicBlockNode;
struct CFG;
class Context;
class Expression;
class Variable;

class DefinitionMap {
public:
    using MapType = SkTHashMap<const Variable*, std::unique_ptr<Expression>*>;

    /**
     * These accessors allow callers to set, search and iterate directly, like a THashMap.
     */
    void set(const Variable* v, std::unique_ptr<Expression>* e) { fDefinitions.set(v, e); }
    std::unique_ptr<Expression>** find(const Variable* v) const { return fDefinitions.find(v); }
    std::unique_ptr<Expression>* get(const Variable* v) { return fDefinitions[v]; }
    MapType::Iter begin() const { return fDefinitions.begin(); }
    MapType::Iter end() const { return fDefinitions.end(); }

    /**
     * Retrieves the compile-time-constant definition of this variable. Returns null if unknown, or
     * if the definition is not a compile-time constant.
     */
    Expression* getKnownDefinition(const Variable* v) const;

    /**
     * Maps all local variables in the function to null, indicating that their value is initially
     * unknown.
     */
    void computeStartState(const CFG& cfg);

    /**
     * Walks the CFG and populates the definition map with compile-time lvalues.
     */
    void addDefinitions(const Context& context, const BasicBlockNode& node);

private:
    void addDefinition(const Context& context, const Variable* var,
                       std::unique_ptr<Expression>* expr);
    void addDefinition(const Context& context, const Expression* lvalue,
                       std::unique_ptr<Expression>* expr);

    SkTHashMap<const Variable*, std::unique_ptr<Expression>*> fDefinitions;
};

}  // namespace SkSL

#endif
