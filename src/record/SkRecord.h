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
        this->mutate(destroyer);
    }

    unsigned count() const { return fCount; }

    // Accepts a visitor functor with this interface:
    //   template <typename T>
    //   void operator()(const T& record) { ... }
    // This operator() must be defined for at least all SkRecords::*; your compiler will help you
    // get this right.
    template <typename F>
    void visit(unsigned i, F& f) const {
        SkASSERT(i < this->count());
        fRecords[i].visit(fTypes[i], f);
    }

    // As above.  f will be called on each recorded canvas call in the order they were append()ed.
    template <typename F>
    void visit(F& f) const {
        for (unsigned i = 0; i < fCount; i++) {
            this->visit(i, f);
        }
    }

    // Accepts a visitor functor with this interface:
    //   template <typename T>
    //   void operator()(T* record) { ... }
    // This operator() must be defined for at least all SkRecords::*; again, your compiler will help
    // you get this right.
    template <typename F>
    void mutate(unsigned i, F& f) {
        SkASSERT(i < this->count());
        fRecords[i].mutate(fTypes[i], f);
    }

    // As above.  f will be called on each recorded canvas call in the order they were append()ed.
    template <typename F>
    void mutate(F& f) {
        for (unsigned i = 0; i < fCount; i++) {
            this->mutate(i, f);
        }
    }

    // Allocate contiguous space for count Ts, to be destroyed (not just freed) when the SkRecord is
    // destroyed.  For classes with constructors, placement new into this array.  Throws on failure.
    // Here T can really be any class, not just those from SkRecords.
    template <typename T>
    T* alloc(unsigned count = 1) {
        return (T*)fAlloc.allocThrow(sizeof(T) * count);
    }

    // Allocate space to record a canvas call of type T at the end of this SkRecord.  You are
    // expected to placement new an object of type T onto this pointer.
    template <typename T>
    T* append() {
        if (fCount == fReserved) {
            fReserved = SkTMax(kFirstReserveCount, fReserved*2);
            fRecords.realloc(fReserved);
            fTypes.realloc(fReserved);
        }

        fTypes[fCount] = T::kType;
        return fRecords[fCount++].alloc<T>(this);
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
    // We pull one final sneaky trick in the implementation.  When recording canvas calls that need
    // to store less than a pointer of data, we don't go through the usual path of allocating the
    // draw command in fAlloc and a pointer to it in fRecords; instead, we ignore fAlloc and
    // directly allocate the object in the space we would have put the pointer in fRecords.  This is
    // why you'll see uintptr_t instead of void* in Record below.
    //
    // The cost of appending a single record into this structure is then:
    //   - 1 + sizeof(void*) + sizeof(T) if sizeof(T) >  sizeof(void*)
    //   - 1 + sizeof(void*)             if sizeof(T) <= sizeof(void*)


    // A mutator that calls destructors of all the canvas calls we've recorded.
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

    // Logically a void* to some bytes in fAlloc, but maybe has the bytes stored immediately
    // instead.  This is also the main interface for devirtualized polymorphic dispatch: see visit()
    // and mutate(), which essentially do the work of the missing vtable.
    struct Record {
    public:

        // Allocate space for a T, perhaps using the SkRecord to allocate that space.
        template <typename T>
        T* alloc(SkRecord* record) {
            if (IsLarge<T>()) {
                fRecord = (uintptr_t)record->alloc<T>();
            }
            return this->ptr<T>();
        }

        // Visit this record with functor F (see public API above) assuming the record we're
        // pointing to has this type.
        template <typename F>
        void visit(Type8 type, F& f) const {
        #define CASE(T) case SkRecords::T##_Type: return f(*this->ptr<SkRecords::T>());
            switch(type) { SK_RECORD_TYPES(CASE) }
        #undef CASE
        }

        // Mutate this record with functor F (see public API above) assuming the record we're
        // pointing to has this type.
        template <typename F>
        void mutate(Type8 type, F& f) {
        #define CASE(T) case SkRecords::T##_Type: return f(this->ptr<SkRecords::T>());
            switch(type) { SK_RECORD_TYPES(CASE) }
        #undef CASE
        }

    private:
        template <typename T>
        T* ptr() const { return (T*)(IsLarge<T>() ? (void*)fRecord : &fRecord); }

        // Is T too big to fit directly into a uintptr_t, neededing external allocation?
        template <typename T>
        static bool IsLarge() { return sizeof(T) > sizeof(uintptr_t); }

        uintptr_t fRecord;
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
