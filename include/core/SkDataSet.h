/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDataSet_DEFINED
#define SkDataSet_DEFINED

#include "SkData.h"
#include "SkFlattenable.h"

class SkStream;
class SkWStream;

class SkDataSet : public SkFlattenable {
public:
    /**
     *  Returns a new empty dataset. Note: since SkDataSet is immutable, this
     *  "new" set may be the same one that was returned before, but each
     *  returned object must have its reference-count balanced regardless.
     *
     *  SkDataSet* empty = SkDataSet::NewEmpty();
     *  ...
     *  empty->unref();
     */
    static SkDataSet* NewEmpty();

    struct Pair {
        const char* fKey;
        SkData*     fValue;
    };

    SkDataSet(const char key[], SkData* value);
    SkDataSet(const Pair[], int count);
    virtual ~SkDataSet();

    bool isEmpty() const { return 0 == fCount; }
    int count() const { return fCount; }
    SkData* find(const char name[]) const;

    class Iter {
    public:
        Iter(const SkDataSet& ds) {
            fPair = ds.fPairs;
            fStop = ds.fPairs + ds.fCount;
        }

        const char* key() const {
            SkASSERT(!this->done());
            return fPair->fKey;
        }

        SkData* value() const {
            SkASSERT(!this->done());
            return fPair->fValue;
        }

        bool done() const { return fPair >= fStop; }
        void next() {
            SkASSERT(!this->done());
            fPair += 1;
        }

    private:
        const SkDataSet::Pair* fPair;
        const SkDataSet::Pair* fStop;
    };

    explicit SkDataSet(SkStream*);
    void writeToStream(SkWStream*) const;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDataSet)

protected:
    SkDataSet(SkFlattenableReadBuffer&);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

private:
    int32_t     fCount;
    uint32_t    fKeySize;
    Pair*       fPairs;

    typedef SkFlattenable INHERITED;
};

#endif
