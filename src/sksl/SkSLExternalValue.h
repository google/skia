/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXTERNALVALUE
#define SKSL_EXTERNALVALUE

namespace SkSL {

class String;
class Type;

class ExternalValue {
public:
    ExternalValue(const char* name)
        : fName(name) {}

    virtual ~ExternalValue() {}

    virtual bool canRead() const {
        return false;
    }

    virtual bool canWrite() const {
        return false;
    }

    virtual bool canCall() const {
        return false;
    }

    /**
     * Returns the type for purposes of read and write operations.
     */
    virtual IRNode::ID type() const {
        return IRNode::ID();
    }

    virtual int callParameterCount() const {
        return -1;
    }

    /**
     * Fills in the outTypes array with pointers to the parameter types. outTypes must be able to
     * hold callParameterCount() pointers.
     */
    virtual void getCallParameterTypes(IRNode::ID* outTypes) const {
        SkASSERT(false);
    }

    /**
     * Returns the return type resulting from a call operation.
     */
    virtual IRNode::ID callReturnType() const {
        return this->type();
    }

    /**
     * Reads the external value and stores the resulting data in target. The caller must ensure
     * that target is a valid pointer to a region of sufficient size to hold the data contained
     * in this external value.
     * 'index' is the element index ([0 .. N-1]) within a call to ByteCode::run()
     */
    virtual void read(int index, float* target) {
        SkASSERT(false);
    }

    /**
     * Copies the value in src into this external value. The caller must ensure that src is a
     * pointer to the type of data expected by this external value.
     * 'index' is the element index ([0 .. N-1]) within a call to ByteCode::run()
     */
    virtual void write(int index, float* src) {
        SkASSERT(false);
    }

    /**
     * Calls the value as a function with the specified parameters. arguments must be a pointer to
     * a structure containing the arguments expected by the external value in source order, and
     * outResult must be a pointer to a region of sufficient size to hold the function's return
     * value.
     * 'index' is the element index ([0 .. N-1]) within a call to ByteCode::run()
     */
    virtual void call(int index, float* arguments, float* outResult) {
        SkASSERT(false);
    }

    /**
     * Resolves 'name' within this context and returns an ExternalValue which represents it, or
     * null if no such child exists. If the implementation of this method creates new
     * ExternalValues and there isn't a more convenient place for ownership of the objects to
     * reside, the compiler's takeOwnership method may be useful.
     *
     * The 'name' string may not persist after this call; do not store this pointer.
     */
    virtual ExternalValue* getChild(const char* name) const {
        return nullptr;
    }

    const char* fName;
};

} // namespace

#endif
