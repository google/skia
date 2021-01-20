/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXTERNALFUNCTION
#define SKSL_EXTERNALFUNCTION

#include "src/core/SkVM.h"
#include "src/sksl/ir/SkSLSymbol.h"

namespace SkSL {

class String;
class Type;

class ExternalFunction : public Symbol {
public:
    static constexpr Kind kSymbolKind = Kind::kExternal;

    ExternalFunction(const char* name, const Type& type)
        : INHERITED(-1, kSymbolKind, name, &type) {}

    virtual int callParameterCount() const = 0;

    /**
     * Fills in the outTypes array with pointers to the parameter types. outTypes must be able to
     * hold callParameterCount() pointers.
     */
    virtual void getCallParameterTypes(const Type** outTypes) const  = 0;

    /**
     * Calls the function with the specified parameters. arguments must be a pointer to a
     * structure containing the arguments expected by the external function in source order, and
     * outResult must be a pointer to a region of sufficient size to hold the function's return
     * value.
     * 'index' is the element index ([0 .. N-1]) within a call to ByteCode::run()
     */
    virtual void call(int index, float* arguments, float* outResult) const { SkASSERT(false); }

    virtual void call(skvm::Builder* builder,
                      skvm::F32* arguments,
                      skvm::F32* outResult,
                      skvm::I32 mask) const {
        SkASSERT(false);
    }

    String description() const override {
        return String("external<") + this->name() + ">";
    }

    // Disable IRNode pooling on external function nodes. ExternalFunction node lifetimes are
    // controlled by the calling code; we can't guarantee that they will be destroyed before a
    // Program is freed. (In fact, it's very unlikely that they would be.)
    static void* operator new(const size_t size) {
        return ::operator new(size);
    }

    static void operator delete(void* ptr) {
        ::operator delete(ptr);
    }

private:
    using INHERITED = Symbol;
};

}  // namespace SkSL

#endif
