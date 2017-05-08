/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecord_DEFINED
#define SkRecord_DEFINED

#include "SkRecords.h"
#include "SkTLogic.h"
#include "SkTemplates.h"
#include "SkVarAlloc.h"

// SkRecord represents a sequence of SkCanvas calls, saved for future use.
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

class SkRecord : public SkRefCnt {
public:
    SkRecord();
    ~SkRecord();

    // Returns the number of canvas commands in this SkRecord.
    int count() const { return fCount; }

    // Visit the i-th canvas command with a functor matching this interface:
    //   template <typename T>
    //   R operator()(const T& record) { ... }
    // This operator() must be defined for at least all SkRecords::*.
    template <typename F>
    auto visit(int i, F&& f) const -> decltype(f(SkRecords::NoOp())) {
        return fRecords[i].visit(f);
    }

    // Mutate the i-th canvas command with a functor matching this interface:
    //   template <typename T>
    //   R operator()(T* record) { ... }
    // This operator() must be defined for at least all SkRecords::*.
    template <typename F>
    auto mutate(int i, F&& f) -> decltype(f((SkRecords::NoOp*)nullptr)) {
        return fRecords[i].mutate(f);
    }

    // Allocate contiguous space for count Ts, to be freed when the SkRecord is destroyed.
    // Here T can be any class, not just those from SkRecords.  Throws on failure.
    template <typename T>
    T* alloc(size_t count = 1) {
        return (T*)fAlloc.alloc(sizeof(T) * count);
    }

    // Add a new command of type T to the end of this SkRecord.
    // You are expected to placement new an object of type T onto this pointer.
    template <typename T>
    T* append() {
        if (fCount == fReserved) {
            this->grow();
        }
        return fRecords[fCount++].set(this->allocCommand<T>());
    }

    // Replace the i-th command with a new command of type T.
    // You are expected to placement new an object of type T onto this pointer.
    // References to the original command are invalidated.
    template <typename T>
    T* replace(int i) {
        SkASSERT(i < this->count());

        Destroyer destroyer;
        this->mutate(i, destroyer);

        return fRecords[i].set(this->allocCommand<T>());
    }

    // Replace the i-th command with a new command of type T.
    // You are expected to placement new an object of type T onto this pointer.
    // You must show proof that you've already adopted the existing command.
    template <typename T, typename Existing>
    T* replace(int i, const SkRecords::Adopted<Existing>& proofOfAdoption) {
        SkASSERT(i < this->count());

        SkASSERT(Existing::kType == fRecords[i].type());
        SkASSERT(proofOfAdoption == fRecords[i].ptr());

        return fRecords[i].set(this->allocCommand<T>());
    }

    // Does not return the bytes in any pointers embedded in the Records; callers
    // need to iterate with a visitor to measure those they care for.
    size_t bytesUsed() const;

    // Rearrange and resize this record to eliminate any NoOps.
    // May change count() and the indices of ops, but preserves their order.
    void defrag();

private:
    // An SkRecord is structured as an array of pointers into a big chunk of memory where
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
    // We store the types of each of the pointers alongside the pointer.
    // The cost to append a T to this structure is 8 + sizeof(T) bytes.

    // A mutator that can be used with replace to destroy canvas commands.
    struct Destroyer {
        template <typename T>
        void operator()(T* record) { record->~T(); }
    };

    template <typename T>
    SK_WHEN(std::is_empty<T>::value, T*) allocCommand() {
        static T singleton = {};
        return &singleton;
    }

    template <typename T>
    SK_WHEN(!std::is_empty<T>::value, T*) allocCommand() { return this->alloc<T>(); }

    void grow();

    // A typed pointer to some bytes in fAlloc.  visit() and mutate() allow polymorphic dispatch.
    struct Record {
        // On 32-bit machines we store type in 4 bytes, followed by a pointer.  Simple.
        // On 64-bit machines we store a pointer with the type slotted into two top (unused) bytes.
        // FWIW, SkRecords::Type is tiny.  It can easily fit in one byte.
        uint64_t fTypeAndPtr;
        static const int kTypeShift = sizeof(void*) == 4 ? 32 : 48;

        // Point this record to its data in fAlloc.  Returns ptr for convenience.
        template <typename T>
        T* set(T* ptr) {
            fTypeAndPtr = ((uint64_t)T::kType) << kTypeShift | (uintptr_t)ptr;
            SkASSERT(this->ptr() == ptr && this->type() == T::kType);
            return ptr;
        }

        SkRecords::Type type() const { return (SkRecords::Type)(fTypeAndPtr >> kTypeShift); }
        void* ptr() const { return (void*)(fTypeAndPtr & ((1ull<<kTypeShift)-1)); }

        // Visit this record with functor F (see public API above).
        template <typename F>
        auto visit(F&& f) const -> decltype(f(SkRecords::NoOp())) {
        #define CASE(T) case SkRecords::T##_Type: return f(*(const SkRecords::T*)this->ptr());
            switch(this->type()) { SK_RECORD_TYPES(CASE) }
        #undef CASE
            SkDEBUGFAIL("Unreachable");
            return f(SkRecords::NoOp());
        }

        // Mutate this record with functor F (see public API above).
        template <typename F>
        auto mutate(F&& f) -> decltype(f((SkRecords::NoOp*)nullptr)) {
        #define CASE(T) case SkRecords::T##_Type: return f((SkRecords::T*)this->ptr());
            switch(this->type()) { SK_RECORD_TYPES(CASE) }
        #undef CASE
            SkDEBUGFAIL("Unreachable");
            return f((SkRecords::NoOp*)nullptr);
        }
    };

    // fRecords needs to be a data structure that can append fixed length data, and need to
    // support efficient random access and forward iteration.  (It doesn't need to be contiguous.)
    int fCount, fReserved;
    SkAutoTMalloc<Record> fRecords;

    // fAlloc needs to be a data structure which can append variable length data in contiguous
    // chunks, returning a stable handle to that data for later retrieval.
    SkVarAlloc fAlloc;
};

#endif//SkRecord_DEFINED
