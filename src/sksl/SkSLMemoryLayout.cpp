/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLMemoryLayout.h"

#include "src/sksl/SkSLIRGenerator.h"

namespace SkSL {

size_t MemoryLayout::VectorAlignment(size_t componentSize, int columns) {
    return componentSize * (columns + columns % 2);
}

size_t MemoryLayout::roundUpIfNeeded(size_t raw) const {
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
size_t MemoryLayout::alignment(const Type& type) const {
    // See OpenGL Spec 7.6.2.2 Standard Uniform Block Layout
    switch (type.kind()) {
        case Type::kScalar_Kind:
            return this->size(type);
        case Type::kVector_Kind:
            return VectorAlignment(this->size(type.componentType().typeNode()), type.columns());
        case Type::kMatrix_Kind:
            return this->roundUpIfNeeded(VectorAlignment(this->size(type.componentType().typeNode()),
                                                         type.rows()));
        case Type::kArray_Kind:
            return this->roundUpIfNeeded(this->alignment(type.componentType().typeNode()));
        case Type::kStruct_Kind: {
            size_t result = 0;
            for (const auto& f : type.fields()) {
                size_t alignment = this->alignment(f.fType.typeNode());
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
size_t MemoryLayout::stride(const Type& type) const {
    switch (type.kind()) {
        case Type::kMatrix_Kind: {
            size_t base = VectorAlignment(this->size(type.componentType().typeNode()), type.rows());
            return this->roundUpIfNeeded(base);
        }
        case Type::kArray_Kind: {
            int align = this->alignment(type.componentType().typeNode());
            int stride = this->size(type.componentType().typeNode()) + align - 1;
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
size_t MemoryLayout::size(const Type& type) const {
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
                return 4 * this->size(type.componentType().typeNode());
            }
            return type.columns() * this->size(type.componentType().typeNode());
        case Type::kMatrix_Kind: // fall through
        case Type::kArray_Kind:
            return type.columns() * this->stride(type);
        case Type::kStruct_Kind: {
            size_t total = 0;
            for (const auto& f : type.fields()) {
                size_t alignment = this->alignment(f.fType.typeNode());
                if (total % alignment != 0) {
                    total += alignment - total % alignment;
                }
                SkASSERT(total % alignment == 0);
                total += this->size(f.fType.typeNode());
            }
            size_t alignment = this->alignment(type);
            SkASSERT(!type.fields().size() ||
                   (0 == alignment % this->alignment(type.fields()[0].fType.typeNode())));
            return (total + alignment - 1) & ~(alignment - 1);
        }
        default:
            ABORT("cannot determine size of type %s", type.name().c_str());
    }
}

} // namespace
