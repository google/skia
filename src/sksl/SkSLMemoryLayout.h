/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKIASL_MEMORYLAYOUT
#define SKIASL_MEMORYLAYOUT

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
        ABORT("unreachable");
    }

    /**
     * Returns a type's required alignment when used as a standalone variable.
     */
    size_t alignment(const Type& type) const {
        // See OpenGL Spec 7.6.2.2 Standard Uniform Block Layout
        switch (type.kind()) {
            case Type::kScalar_Kind:
                return this->size(type);
            case Type::kVector_Kind:
                return vector_alignment(this->size(type.componentType()), type.columns());
            case Type::kMatrix_Kind:
                return this->roundUpIfNeeded(vector_alignment(this->size(type.componentType()),
                                                              type.rows()));
            case Type::kArray_Kind:
                return this->roundUpIfNeeded(this->alignment(type.componentType()));
            case Type::kStruct_Kind: {
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
                ABORT("cannot determine size of type %s", type.name().c_str());
        }
    }

    /**
     * For matrices and arrays, returns the number of bytes from the start of one entry (row, in
     * the case of matrices) to the start of the next.
     */
    size_t stride(const Type& type) const {
        switch (type.kind()) {
            case Type::kMatrix_Kind: {
                size_t base = vector_alignment(this->size(type.componentType()), type.rows());
                return this->roundUpIfNeeded(base);
            }
            case Type::kArray_Kind: {
                int align = this->alignment(type.componentType());
                int stride = this->size(type.componentType()) + align - 1;
                stride -= stride % align;
                return this->roundUpIfNeeded(stride);
            }
            default:
                ABORT("type does not have a stride");
        }
    }

    /**
     * Returns the size of a type in bytes.
     */
    size_t size(const Type& type) const {
        switch (type.kind()) {
            case Type::kScalar_Kind:
                if (type.name() == "bool") {
                    return 1;
                }
                // FIXME need to take precision into account, once we figure out how we want to
                // handle it...
                return 4;
            case Type::kVector_Kind:
                if (fStd == kMetal_Standard && type.columns() == 3) {
                    return 4 * this->size(type.componentType());
                }
                return type.columns() * this->size(type.componentType());
            case Type::kMatrix_Kind: // fall through
            case Type::kArray_Kind:
                return type.columns() * this->stride(type);
            case Type::kStruct_Kind: {
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
                ABORT("cannot determine size of type %s", type.name().c_str());
        }
    }

    const Standard fStd;
};

} // namespace

#endif
