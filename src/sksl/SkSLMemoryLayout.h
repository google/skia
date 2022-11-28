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
    enum class Standard {
        // GLSL std140 layout as described in OpenGL Spec v4.5, 7.6.2.2.
        k140,

        // GLSL std430 layout. This layout is like std140 but with optimizations. This layout can
        // ONLY be used with shader storage blocks.
        k430,

        // MSL memory layout.
        kMetal,

        // WebGPU Shading Language buffer layout constraints for the uniform address space.
        kWGSLUniform,

        // WebGPU Shading Language buffer layout constraints for the storage address space.
        kWGSLStorage,
    };

    MemoryLayout(Standard std)
    : fStd(std) {}

    bool isWGSL() const { return fStd == Standard::kWGSLUniform || fStd == Standard::kWGSLStorage; }

    bool isMetal() const { return fStd == Standard::kMetal; }

    /**
     * WGSL and std140 require various types of variables (structs, arrays, and matrices) in the
     * uniform address space to be rounded up to the nearest multiple of 16. This function performs
     * the rounding depending on the given `type` and the current memory layout standard.
     *
     * (For WGSL, see https://www.w3.org/TR/WGSL/#address-space-layout-constraints).
     */
    size_t roundUpIfNeeded(size_t raw, Type::TypeKind type) const {
        if (fStd == Standard::k140) {
            return roundUp16(raw);
        }
        // WGSL uniform matrix layout is simply the alignment of the matrix columns and
        // doesn't have a 16-byte multiple alignment constraint.
        if (fStd == Standard::kWGSLUniform && type != Type::TypeKind::kMatrix) {
            return roundUp16(raw);
        }
        return raw;
    }

    /**
     * Rounds up the integer `n` to the smallest multiple of 16 greater than `n`.
     */
    size_t roundUp16(size_t n) const { return (n + 15) & ~15; }

    /**
     * Returns a type's required alignment when used as a standalone variable.
     */
    size_t alignment(const Type& type) const {
        // See OpenGL Spec 7.6.2.2 Standard Uniform Block Layout
        switch (type.typeKind()) {
            case Type::TypeKind::kScalar:
            case Type::TypeKind::kAtomic:
                return this->size(type);
            case Type::TypeKind::kVector:
                return GetVectorAlignment(this->size(type.componentType()), type.columns());
            case Type::TypeKind::kMatrix:
                return this->roundUpIfNeeded(
                        GetVectorAlignment(this->size(type.componentType()), type.rows()),
                        type.typeKind());
            case Type::TypeKind::kArray:
                return this->roundUpIfNeeded(this->alignment(type.componentType()),
                                             type.typeKind());
            case Type::TypeKind::kStruct: {
                size_t result = 0;
                for (const auto& f : type.fields()) {
                    size_t alignment = this->alignment(*f.fType);
                    if (alignment > result) {
                        result = alignment;
                    }
                }
                return this->roundUpIfNeeded(result, type.typeKind());
            }
            default:
                SK_ABORT("cannot determine alignment of type %s", type.displayName().c_str());
        }
    }

    /**
     * For matrices and arrays, returns the number of bytes from the start of one entry (row, in
     * the case of matrices) to the start of the next.
     */
    size_t stride(const Type& type) const {
        switch (type.typeKind()) {
            case Type::TypeKind::kMatrix:
                return this->alignment(type);
            case Type::TypeKind::kArray: {
                int stride = this->size(type.componentType());
                if (stride > 0) {
                    int align = this->alignment(type.componentType());
                    stride += align - 1;
                    stride -= stride % align;
                    stride = this->roundUpIfNeeded(stride, type.typeKind());
                }
                return stride;
            }
            default:
                SK_ABORT("type does not have a stride");
        }
    }

    /**
     * Returns the size of a type in bytes. Returns 0 if the given type is not supported.
     */
    size_t size(const Type& type) const {
        switch (type.typeKind()) {
            case Type::TypeKind::kScalar:
                if (type.isBoolean()) {
                    if (this->isWGSL()) {
                        return 0;
                    }
                    return 1;
                }
                if ((this->isMetal() || this->isWGSL()) && !type.highPrecision() &&
                    type.isNumber()) {
                    return 2;
                }
                return 4;
            case Type::TypeKind::kAtomic:
                // Our atomic types (currently atomicUint) always occupy 4 bytes.
                return 4;
            case Type::TypeKind::kVector:
                if (this->isMetal() && type.columns() == 3) {
                    return 4 * this->size(type.componentType());
                }
                return type.columns() * this->size(type.componentType());
            case Type::TypeKind::kMatrix: // fall through
            case Type::TypeKind::kArray:
                return type.isUnsizedArray() ? 0 : (type.columns() * this->stride(type));
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
                SK_ABORT("cannot determine size of type %s", type.displayName().c_str());
        }
    }

    /**
     * Not all types are compatible with memory layout.
     */
    size_t isSupported(const Type& type) const {
        switch (type.typeKind()) {
            case Type::TypeKind::kAtomic:
                return true;

            case Type::TypeKind::kScalar:
                // bool and short are not host-shareable in WGSL.
                return !this->isWGSL() ||
                       (!type.isBoolean() && (type.isFloat() || type.highPrecision()));

            case Type::TypeKind::kVector:
            case Type::TypeKind::kMatrix:
            case Type::TypeKind::kArray:
                return this->isSupported(type.componentType());

            case Type::TypeKind::kStruct:
                return std::all_of(
                        type.fields().begin(), type.fields().end(), [this](const Type::Field& f) {
                            return this->isSupported(*f.fType);
                        });

            default:
                return false;
        }
    }

private:
    static size_t GetVectorAlignment(size_t componentSize, int columns) {
        return componentSize * (columns + columns % 2);
    }

    const Standard fStd;
};

}  // namespace SkSL

#endif
