/*
* Copyright 2019 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkReflected_DEFINED
#define SkReflected_DEFINED

#include "SkRefCnt.h"
#include "SkTArray.h"

#include <functional>

class SkFieldVisitor;

class SkReflected : public SkRefCnt {
public:
    typedef sk_sp<SkReflected>(*Factory)();
    struct Type {
        const char* fName;
        const Type* fBase;
        Factory     fFactory;

        bool isDerivedFrom(const Type* t) const {
            const Type* base = fBase;
            while (base) {
                if (base == t) {
                    return true;
                }
                base = base->fBase;
            }
            return false;
        }
    };

    virtual const Type* getType() const = 0;
    static const Type* GetType() {
        static Type gType{ "SkReflected", nullptr, nullptr };
        return &gType;
    }

    bool isOfType(const Type* t) const {
        const Type* thisType = this->getType();
        return thisType == t || thisType->isDerivedFrom(t);
    }

    static void Register(const Type* type) {
        gTypes.push_back(type);
    }

    static sk_sp<SkReflected> CreateInstance(const char* name) {
        for (const Type* type : gTypes) {
            if (0 == strcmp(name, type->fName)) {
                return type->fFactory();
            }
        }
        return nullptr;
    }

    virtual void visitFields(SkFieldVisitor*) = 0;

    static void VisitTypes(std::function<void(const Type*)> visitor,
                           const Type* baseType = nullptr);
private:
    static SkSTArray<16, const Type*, true> gTypes;
};

#define REFLECTED(TYPE, BASE)                                    \
    static sk_sp<SkReflected> CreateProc() {                     \
        return sk_sp<SkReflected>(new TYPE());                   \
    }                                                            \
    static const Type* GetType() {                               \
        static Type gType{ #TYPE, BASE::GetType(), CreateProc }; \
        return &gType;                                           \
    }                                                            \
    const Type* getType() const override { return GetType(); }

#define REFLECTED_ABSTRACT(TYPE, BASE)                          \
    static const Type* GetType() {                              \
        static Type gType{ #TYPE, BASE::GetType(), nullptr };   \
        return &gType;                                          \
    }                                                           \
    const Type* getType() const override { return GetType(); }

#define REGISTER_REFLECTED(TYPE) SkReflected::Register(TYPE::GetType())

#endif
