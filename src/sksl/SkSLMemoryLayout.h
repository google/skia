/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKIASL_MEMORYLAYOUT
#define SKIASL_MEMORYLAYOUT

#include <algorithm>

#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

class MemoryLayout {
public:
    enum Standard {
        k140_Standard,
        k430_Standard,
        kMetal_Standard
    };

    MemoryLayout(Standard std)
    : fStd(std) {}

    static size_t vector_alignment(size_t componentSize, int columns) {
        return componentSize * (columns + columns % 2);
    }

    /**
     * Rounds up to the nearest multiple of 16 if in std140, otherwise returns the parameter
     * unchanged (std140 requires various things to be rounded up to the nearest multiple of 16,
     * std430 does not).
     */
    size_t roundUpIfNeeded(size_t raw) const {
        switch (fStd) {
            case k140_Standard: return (raw + 15) & ~15;
            case k430_Standard: return raw;
            case kMetal_Standard: return raw;
        }
        SkUNREACHABLE;
    }

    /**
     * Returns a type's required alignment when used as a standalone variable.
     */
    size_t alignment(const Type& type) const {
        // See OpenGL Spec 7.6.2.2 Standard Uniform Block Layout
        switch (type.typeKind()) {
            case Type::TypeKind::kScalar:
                return this->size(type);
            case Type::TypeKind::kVector:
                return vector_alignment(this->size(type.componentType()), type.columns());
            case Type::TypeKind::kMatrix:
                return this->roundUpIfNeeded(vector_alignment(this->size(type.componentType()),
                                                              type.rows()));
            case Type::TypeKind::kArray:
                return this->roundUpIfNeeded(this->alignment(type.componentType()));
            case Type::TypeKind::kStruct: {
                size_t result = 0;
                for (const auto& f : type.fields()) {
                    size_t alignment = this->alignment(*f.fType);
                    if (alignment > result) {
                        result = alignment;
                    }
                }
                return this->roundUpIfNeeded(result);
            }
            default:
                SK_ABORT("cannot determine size of type %s", String(type.name()).c_str());
        }
    }

    /**
     * For matrices and arrays, returns the number of bytes from the start of one entry (row, in
     * the case of matrices) to the start of the next.
     */
    size_t stride(const Type& type) const {
        switch (type.typeKind()) {
            case Type::TypeKind::kMatrix: {
                size_t base = vector_alignment(this->size(type.componentType()), type.rows());
                return this->roundUpIfNeeded(base);
            }
            case Type::TypeKind::kArray: {
                int stride = this->size(type.componentType());
                if (stride > 0) {
                    int align = this->alignment(type.componentType());
                    stride += align - 1;
                    stride -= stride % align;
                    stride = this->roundUpIfNeeded(stride);
                }
                return stride;
            }
            default:
                SK_ABORT("type does not have a stride");
        }
    }

    /**
     * Returns the size of a type in bytes.
     */
    size_t size(const Type& type) const {
        switch (type.typeKind()) {
            case Type::TypeKind::kScalar:
                if (type.isBoolean()) {
                    return 1;
                }
                // FIXME need to take precision into account, once we figure out how we want to
                // handle it...
                return 4;
            case Type::TypeKind::kVector:
                if (fStd == kMetal_Standard && type.columns() == 3) {
                    return 4 * this->size(type.componentType());
                }
                return type.columns() * this->size(type.componentType());
            case Type::TypeKind::kMatrix: // fall through
            case Type::TypeKind::kArray:
                return type.columns() * this->stride(type);
            case Type::TypeKind::kStruct: {
                size_t total = 0;
                for (const auto& f : type.fields()) {
                    size_t alignment = this->alignment(*f.fType);
                    if (total % alignment != 0) {
                        total += alignment - total % alignment;
                    }
                    SkASSERT(total % alignment == 0);
                    total += this->size(*f.fType);
                }
                size_t alignment = this->alignment(type);
                SkASSERT(!type.fields().size() ||
                       (0 == alignment % this->alignment(*type.fields()[0].fType)));
                return (total + alignment - 1) & ~(alignment - 1);
            }
            default:
                SK_ABORT("cannot determine size of type %s", String(type.name()).c_str());
        }
    }

    /**
     * Not all types are compatible with memory layout.
     */
    static size_t LayoutIsSupported(const Type& type) {
        switch (type.typeKind()) {
            case Type::TypeKind::kScalar:
            case Type::TypeKind::kVector:
            case Type::TypeKind::kMatrix:
                return true;

            case Type::TypeKind::kArray:
                return LayoutIsSupported(type.componentType());

            case Type::TypeKind::kStruct:
                return std::all_of(
                        type.fields().begin(), type.fields().end(),
                        [](const Type::Field& f) { return LayoutIsSupported(*f.fType); });

            default:
                return false;
        }
    }

    const Standard fStd;
};

}  // namespace SkSL

#endif
