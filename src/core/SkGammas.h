/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGammas_DEFINED
#define SkGammas_DEFINED

#include "SkColorSpace.h"
#include "SkData.h"
#include "SkTemplates.h"

struct SkGammas : SkRefCnt {

    // There are four possible representations for gamma curves.  kNone_Type is used
    // as a placeholder until the struct is initialized.  It is not a valid value.
    enum class Type {
        kNone_Type,
        kNamed_Type,
        kValue_Type,
        kTable_Type,
        kParam_Type,
    };

    // Contains information for a gamma table.
    struct Table {
        size_t fOffset;
        int    fSize;

        const float* table(const SkGammas* base) const {
            return SkTAddOffset<const float>(base, sizeof(SkGammas) + fOffset);
        }
    };

    // Contains the actual gamma curve information.  Should be interpreted
    // based on the type of the gamma curve.
    union Data {
        Data() : fTable{0, 0} {}

        SkGammaNamed fNamed;
        float        fValue;
        Table        fTable;
        size_t       fParamOffset;

        const SkColorSpaceTransferFn& params(const SkGammas* base) const {
            return *SkTAddOffset<const SkColorSpaceTransferFn>(base,
                                                               sizeof(SkGammas) + fParamOffset);
        }
    };

    bool allChannelsSame() const {
        // All channels are the same type?
        Type type = this->type(0);
        for (int i = 1; i < this->channels(); i++) {
            if (type != this->type(i)) {
                return false;
            }
        }

        // All data the same?
        auto& first = this->data(0);
        for (int i = 1; i < this->channels(); i++) {
            auto& data = this->data(i);
            switch (type) {
                case Type:: kNone_Type:                                                    break;
                case Type::kNamed_Type: if (first.fNamed != data.fNamed) { return false; } break;
                case Type::kValue_Type: if (first.fValue != data.fValue) { return false; } break;
                case Type::kTable_Type:
                    if (first.fTable.fOffset != data.fTable.fOffset) { return false; }
                    if (first.fTable.fSize   != data.fTable.fSize  ) { return false; }
                    break;
                case Type::kParam_Type:
                    if (0 != memcmp(&first.params(this), &data.params(this),
                                    sizeof(SkColorSpaceTransferFn))) {
                        return false;
                    }
                    break;
            }
        }
        return true;
    }

    bool isNamed     (int i) const { return Type::kNamed_Type == this->type(i); }
    bool isValue     (int i) const { return Type::kValue_Type == this->type(i); }
    bool isTable     (int i) const { return Type::kTable_Type == this->type(i); }
    bool isParametric(int i) const { return Type::kParam_Type == this->type(i); }

    const Data& data(int i) const {
        SkASSERT(i >= 0 && i < fChannels);
        return fData[i];
    }

    const float* table(int i) const {
        SkASSERT(this->isTable(i));
        return this->data(i).fTable.table(this);
    }

    int tableSize(int i) const {
        SkASSERT(this->isTable(i));
        return this->data(i).fTable.fSize;
    }

    const SkColorSpaceTransferFn& params(int i) const {
        SkASSERT(this->isParametric(i));
        return this->data(i).params(this);
    }

    Type type(int i) const {
        SkASSERT(i >= 0 && i < fChannels);
        return fType[i];
    }

    int channels() const { return fChannels; }

    SkGammas(int channels) : fChannels(channels) {
        SkASSERT(channels <= (int)SK_ARRAY_COUNT(fType));
        for (Type& t : fType) {
            t = Type::kNone_Type;
        }
    }

    // These fields should only be modified when initializing the struct.
    int  fChannels;
    Data fData[4];
    Type fType[4];

    // Objects of this type are sometimes created in a custom fashion using
    // sk_malloc_throw and therefore must be sk_freed.  We overload new to
    // also call sk_malloc_throw so that memory can be unconditionally released
    // using sk_free in an overloaded delete. Overloading regular new means we
    // must also overload placement new.
    void* operator new(size_t size) { return sk_malloc_throw(size); }
    void* operator new(size_t, void* p) { return p; }
    void operator delete(void* p) { sk_free(p); }
};

#endif
