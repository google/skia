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

    // Allow callers to set, search and iterate directly, like a THashMap.
    void set(const Variable* v, std::unique_ptr<Expression>* e) { fDefinitions.set(v, e); }
    std::unique_ptr<Expression>** find(const Variable* v) const { return fDefinitions.find(v); }
    std::unique_ptr<Expression>* get(const Variable* v) { return fDefinitions[v]; }
    MapType::Iter begin() const { return fDefinitions.begin(); }
    MapType::Iter end() const { return fDefinitions.end(); }

    // These methods populate the definition map from the CFG.
    void computeStartState(const CFG& cfg);
    void addDefinition(const Context& context, const Expression* lvalue,
                       std::unique_ptr<Expression>* expr);
    void addDefinitions(const Context& context, const BasicBlockNode& node);

private:
    SkTHashMap<const Variable*, std::unique_ptr<Expression>*> fDefinitions;
};

}  // namespace SkSL

#endif
