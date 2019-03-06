/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkReflected_DEFINED
#define SkReflected_DEFINED

#include "SkColor.h"
#include "SkRefCnt.h"
#include "SkString.h"
#include "SkTArray.h"

#include <string.h>

struct SkCurve;
class SkFieldVisitor;
class SkRandom;

/**
 * Classes and macros for a lightweight reflection system.
 *
 * Classes that derive from SkReflected have several features:
 *   - Access to an SkReflected::Type instance, via static GetType() or virtual getType()
 *     The Type instance can be used to create additional instances (fFactory), get the name
 *     of the type, and answer queries of the form "is X derived from Y".
 *   - Given a string containing a type name, SkReflected can create an instance of that type.
 *   - SkReflected::VisitTypes can be used to enumerate all Types.
 *
 * Together, this simplifies the implementation of serialization and other dynamic type factories.
 *
 * Finally, all SkReflected-derived types must implement visitFields, which provides field-level
 * reflection, in conjunction with SkFieldVisitor. See SkFieldVisitor, below.
 *
 * To create a new reflected class:
 *   - Derive the class (directly or indirectly) from SkReflected.
 *   - Ensure that the class can be default constructed.
 *   - In the public area of the class declaration, add REFLECTED(<ClassName>, <BaseClassName>).
 *     If the class is abstract, use REFLECTED_ABSTRACT(<ClassName>, <BaseClassName>) instead.
 *   - Add a one-time call to REGISTER_REFLECTED(<ClassName>) at initialization time.
 *   - Implement visitFields(), as described below.
 */
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

    static void VisitTypes(std::function<void(const Type*)> visitor);

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

///////////////////////////////////////////////////////////////////////////////

struct SkPoint;

/**
 * SkFieldVisitor is an interface that can be implemented by any class to visit all fields of
 * SkReflected types, and of types that implement the visitFields() function.
 *
 * Classes implementing the interface must supply implementations of virtual functions that visit
 * basic types (float, int, bool, SkString, etc...), as well as helper methods for entering the
 * scope of an object or array.
 *
 * All visit functions supply a field name, and a non-constant reference to an actual field.
 * This allows visitors to serialize or deserialize collections of objects, or perform edits on
 * existing objects.
 *
 * Classes that implement visitFields (typically derived from SkReflected) should simply call
 * visit() for each of their fields, passing a (unique) field name, and the actual field. If your
 * class has derived fields, it's best to only visit() the fields that you would serialize, then
 * enforce any constraints afterwards.
 *
 * See SkParticleSerialization.h for example visitors that perform serialization to and from JSON.
 */
class SkFieldVisitor {
public:
    virtual ~SkFieldVisitor() {}

    // Visit functions for primitive types, to be implemented by derived visitors.
    virtual void visit(const char*, float&) = 0;
    virtual void visit(const char*, int&) = 0;
    virtual void visit(const char*, bool&) = 0;
    virtual void visit(const char*, SkString&) = 0;

    virtual void visit(const char*, SkPoint&) = 0;
    virtual void visit(const char*, SkColor4f&) = 0;

    // Accommodation for enums, where caller can supply a value <-> string map
    struct EnumStringMapping {
        int         fValue;
        const char* fName;
    };
    virtual void visit(const char*, int&, const EnumStringMapping*, int count) = 0;

    // Specific virtual signature for SkCurve, to allow for heavily customized UI in SkGuiVisitor.
    virtual void visit(const char* name, SkCurve& c);

    // Default visit function for structs with no special behavior. It is assumed that any such
    // struct implements visitFields(SkFieldVisitor*) to recursively visit each of its fields.
    template <typename T>
    void visit(const char* name, T& value) {
        this->enterObject(name);
        value.visitFields(this);
        this->exitObject();
    }

    // Specialization for SkTArrays. In conjunction with the enterArray/exitArray virtuals, this
    // allows visitors to resize an array (for deserialization), and apply a single edit operation
    // (remove or move a single element). Each element of the array is visited as normal.
    template <typename T, bool MEM_MOVE>
    void visit(const char* name, SkTArray<T, MEM_MOVE>& arr) {
        arr.resize_back(this->enterArray(name, arr.count()));
        for (int i = 0; i < arr.count(); ++i) {
            this->visit(nullptr, arr[i]);
        }
        this->exitArray().apply(arr);
    }

    // Specialization for sk_sp pointers to types derived from SkReflected. Those types are known
    // to implement visitFields. This allows the visitor to modify the contents of the object, or
    // even replace it with an entirely new object. The virtual function uses SkReflected as a
    // common type, but uses SkReflected::Type to communicate the required base-class. In this way,
    // the new object can be verified to match the type of the original (templated) pointer.
    template <typename T>
    void visit(const char* name, sk_sp<T>& obj) {
        this->enterObject(name);

        sk_sp<SkReflected> newObj = obj;
        this->visit(newObj, T::GetType());
        if (newObj != obj) {
            if (!newObj || newObj->isOfType(T::GetType())) {
                obj.reset(static_cast<T*>(newObj.release()));
            } else {
                obj.reset();
            }
        }

        if (obj) {
            obj->visitFields(this);
        }
        this->exitObject();
    }

protected:
    // Helper struct to allow exitArray to specify a single operation performed on the array.
    struct ArrayEdit {
        enum class Verb {
            kNone,
            kRemove,
            kMoveForward,
        };

        Verb fVerb = Verb::kNone;
        int fIndex = 0;

        template <typename T, bool MEM_MOVE>
        void apply(SkTArray<T, MEM_MOVE>& arr) const {
            switch (fVerb) {
            case Verb::kNone:
                break;
            case Verb::kRemove:
                for (int i = fIndex; i < arr.count() - 1; ++i) {
                    arr[i] = arr[i + 1];
                }
                arr.pop_back();
                break;
            case Verb::kMoveForward:
                if (fIndex > 0 && fIndex < arr.count()) {
                    std::swap(arr[fIndex - 1], arr[fIndex]);
                }
                break;
            }
        }
    };

    static const char* EnumToString(int value, const EnumStringMapping* map, int count) {
        for (int i = 0; i < count; ++i) {
            if (map[i].fValue == value) {
                return map[i].fName;
            }
        }
        return nullptr;
    }
    static int StringToEnum(const char* str, const EnumStringMapping* map, int count) {
        for (int i = 0; i < count; ++i) {
            if (0 == strcmp(str, map[i].fName)) {
                return map[i].fValue;
            }
        }
        return -1;
    }

    virtual void enterObject(const char* name) = 0;
    virtual void exitObject() = 0;

    virtual int enterArray(const char* name, int oldCount) = 0;
    virtual ArrayEdit exitArray() = 0;

    virtual void visit(sk_sp<SkReflected>&, const SkReflected::Type* baseType) = 0;
};

#endif // SkReflected_DEFINED
