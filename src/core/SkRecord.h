/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecord_DEFINED
#define SkRecord_DEFINED

#include "SkChunkAlloc.h"
#include "SkRecords.h"
#include "SkTLogic.h"
#include "SkTemplates.h"

// SkRecord (REC-ord) represents a sequence of SkCanvas calls, saved for future use.
// These future uses may include: replay, optimization, serialization, or combinations of those.
//
// Though an enterprising user may find calling alloc(), append(), visit(), and mutate() enough to
// work with SkRecord, you probably want to look at SkRecorder which presents an SkCanvas interface
// for creating an SkRecord, and SkRecordDraw which plays an SkRecord back into another SkCanvas.
//
// SkRecord often looks like it's compatible with any type T, but really it's compatible with any
// type T which has a static const SkRecords::Type kType.  That is to say, SkRecord is compatible
// only with SkRecords::* structs defined in SkRecords.h.  Your compiler will helpfully yell if you
// get this wrong.

class SkRecord : SkNoncopyable {
public:
    SkRecord(size_t chunkBytes = 4096, unsigned firstReserveCount = 64 / sizeof(void*))
        : fAlloc(chunkBytes), fCount(0), fReserved(0), kFirstReserveCount(firstReserveCount) {}

    ~SkRecord() {
        Destroyer destroyer;
        for (unsigned i = 0; i < this->count(); i++) {
            this->mutate<void>(i, destroyer);
        }
    }

    // Returns the number of canvas commands in this SkRecord.
    unsigned count() const { return fCount; }

    // Visit the i-th canvas command with a functor matching this interface:
    //   template <typename T>
    //   R operator()(const T& record) { ... }
    // This operator() must be defined for at least all SkRecords::*.
    template <typename R, typename F>
    R visit(unsigned i, F& f) const {
        SkASSERT(i < this->count());
        return fRecords[i].visit<R>(fTypes[i], f);
    }

    // Mutate the i-th canvas command with a functor matching this interface:
    //   template <typename T>
    //   R operator()(T* record) { ... }
    // This operator() must be defined for at least all SkRecords::*.
    template <typename R, typename F>
    R mutate(unsigned i, F& f) {
        SkASSERT(i < this->count());
        return fRecords[i].mutate<R>(fTypes[i], f);
    }
    // TODO: It'd be nice to infer R from F for visit and mutate if we ever get std::result_of.

    // Allocate contiguous space for count Ts, to be freed when the SkRecord is destroyed.
    // Here T can be any class, not just those from SkRecords.  Throws on failure.
    template <typename T>
    T* alloc(unsigned count = 1) {
        return (T*)fAlloc.allocThrow(sizeof(T) * count);
    }

    // Add a new command of type T to the end of this SkRecord.
    // You are expected to placement new an object of type T onto this pointer.
    template <typename T>
    T* append() {
        if (fCount == fReserved) {
            fReserved = SkTMax(kFirstReserveCount, fReserved*2);
            fRecords.realloc(fReserved);
            fTypes.realloc(fReserved);
        }

        fTypes[fCount] = T::kType;
        return fRecords[fCount++].set(this->allocCommand<T>());
    }

    // Replace the i-th command with a new command of type T.
    // You are expected to placement new an object of type T onto this pointer.
    // References to the original command are invalidated.
    template <typename T>
    T* replace(unsigned i) {
        SkASSERT(i < this->count());

        Destroyer destroyer;
        this->mutate<void>(i, destroyer);

        fTypes[i] = T::kType;
        return fRecords[i].set(this->allocCommand<T>());
    }

    // Replace the i-th command with a new command of type T.
    // You are expected to placement new an object of type T onto this pointer.
    // You must show proof that you've already adopted the existing command.
    template <typename T, typename Existing>
    T* replace(unsigned i, const SkRecords::Adopted<Existing>& proofOfAdoption) {
        SkASSERT(i < this->count());

        SkASSERT(Existing::kType == fTypes[i]);
        SkASSERT(proofOfAdoption == fRecords[i].ptr<Existing>());

        fTypes[i] = T::kType;
        return fRecords[i].set(this->allocCommand<T>());
    }

private:
    // Implementation notes!
    //
    // Logically an SkRecord is structured as an array of pointers into a big chunk of memory where
    // records representing each canvas draw call are stored:
    //
    // fRecords:  [*][*][*]...
    //             |  |  |
    //             |  |  |
    //             |  |  +---------------------------------------+
    //             |  +-----------------+                        |
    //             |                    |                        |
    //             v                    v                        v
    //   fAlloc:  [SkRecords::DrawRect][SkRecords::DrawPosTextH][SkRecords::DrawRect]...
    //
    // In the scheme above, the pointers in fRecords are void*: they have no type.  The type is not
    // stored in fAlloc either; we just write raw data there.  But we need that type information.
    // Here are some options:
    //   1) use inheritance, virtuals, and vtables to make the fRecords pointers smarter
    //   2) store the type data manually in fAlloc at the start of each record
    //   3) store the type data manually somewhere with fRecords
    //
    // This code uses approach 3).  The implementation feels very similar to 1), but it's
    // devirtualized instead of using the language's polymorphism mechanisms.  This lets us work
    // with the types themselves (as SkRecords::Type), a sort of limited free RTTI; it lets us pay
    // only 1 byte to store the type instead of a full pointer (4-8 bytes); and it leads to better
    // decoupling between the SkRecords::* record types and the operations performed on them in
    // visit() or mutate().  The recorded canvas calls don't have to have any idea about the
    // operations performed on them.
    //
    // We store the types in a parallel fTypes array, mainly so that they can be tightly packed as
    // single bytes.  This has the side effect of allowing very fast analysis passes over an
    // SkRecord looking for just patterns of draw commands (or using this as a quick reject
    // mechanism) though there's admittedly not a very good API exposed publically for this.
    //
    // The cost to append a T into this structure is 1 + sizeof(void*) + sizeof(T).

    // A mutator that can be used with replace to destroy canvas commands.
    struct Destroyer {
        template <typename T>
        void operator()(T* record) { record->~T(); }
    };

    // Logically the same as SkRecords::Type, but packed into 8 bits.
    struct Type8 {
    public:
        // This intentionally converts implicitly back and forth.
        Type8(SkRecords::Type type) : fType(type) { SkASSERT(*this == type); }
        operator SkRecords::Type () { return (SkRecords::Type)fType; }

    private:
        uint8_t fType;
    };

    // No point in allocating any more than one of an empty struct.
    // We could just return NULL but it's sort of confusing to return NULL on success.
    template <typename T>
    SK_WHEN(SkTIsEmpty<T>, T*) allocCommand() {
        static T singleton = {};
        return &singleton;
    }

    template <typename T>
    SK_WHEN(!SkTIsEmpty<T>, T*) allocCommand() { return this->alloc<T>(); }

    // An untyped pointer to some bytes in fAlloc.  This is the interface for polymorphic dispatch:
    // visit() and mutate() work with the parallel fTypes array to do the work of a vtable.
    struct Record {
    public:
        // Point this record to its data in fAlloc.  Returns ptr for convenience.
        template <typename T>
        T* set(T* ptr) {
            fPtr = ptr;
            return ptr;
        }

        // Get the data in fAlloc, assuming it's of type T.
        template <typename T>
        T* ptr() const { return (T*)fPtr; }

        // Visit this record with functor F (see public API above) assuming the record we're
        // pointing to has this type.
        template <typename R, typename F>
        R visit(Type8 type, F& f) const {
        #define CASE(T) case SkRecords::T##_Type: return f(*this->ptr<SkRecords::T>());
            switch(type) { SK_RECORD_TYPES(CASE) }
        #undef CASE
            SkDEBUGFAIL("Unreachable");
            return R();
        }

        // Mutate this record with functor F (see public API above) assuming the record we're
        // pointing to has this type.
        template <typename R, typename F>
        R mutate(Type8 type, F& f) {
        #define CASE(T) case SkRecords::T##_Type: return f(this->ptr<SkRecords::T>());
            switch(type) { SK_RECORD_TYPES(CASE) }
        #undef CASE
            SkDEBUGFAIL("Unreachable");
            return R();
        }

    private:
        void* fPtr;
    };

    // fAlloc needs to be a data structure which can append variable length data in contiguous
    // chunks, returning a stable handle to that data for later retrieval.
    //
    // fRecords and fTypes need to be data structures that can append fixed length data, and need to
    // support efficient forward iteration.  (They don't need to be contiguous or indexable.)

    SkChunkAlloc fAlloc;
    SkAutoTMalloc<Record> fRecords;
    SkAutoTMalloc<Type8> fTypes;
    // fCount and fReserved measure both fRecords and fTypes, which always grow in lock step.
    unsigned fCount;
    unsigned fReserved;
    const unsigned kFirstReserveCount;
};

#endif//SkRecord_DEFINED
