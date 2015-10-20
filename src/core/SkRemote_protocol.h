/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRemote_protocol_DEFINED
#define SkRemote_protocol_DEFINED

// ATTENTION!  Changes to this file can break protocol compatibility.  Tread carefully.

namespace SkRemote {

    // It is safe to append to this enum without breaking protocol compatibility.
    // Resorting, deleting, or inserting anywhere but the end will break compatibility.
    enum class Type : uint8_t {
        kNone,

        kMatrix,
        kMisc,
        kPath,
        kStroke,
        kXfermode,
    };

    class ID {
    public:
        explicit ID(Type type = Type::kNone) : fVal((uint64_t)type << 56) {}
        ID(Type type, uint64_t val) {
            fVal = (uint64_t)type << 56 | val;
            SkASSERT(this->type() == type && this->val() == val);
        }

        Type    type() const { return (Type)(fVal >> 56); }
        uint64_t val() const { return fVal & ~((uint64_t)0xFF << 56); }

        bool operator==(ID o) const { return fVal == o.fVal; }
        ID operator++() {
            ++fVal;
            SkASSERT(this->val() != 0);  // Overflow is particularly bad as it'd change our Type.
            return *this;
        }

    private:
        // High 8 bits hold a Type.  Low 56 bits are unique within that Type.
        // Any change to this format will break protocol compatibility.
        uint64_t fVal;
    };

}  // namespace SkRemote

#endif//SkRemote_protocol_DEFINED
